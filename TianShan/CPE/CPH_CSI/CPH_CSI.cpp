
#include "BaseClass.h"
#include "CPH_CSI.h"
#include "NTFSSource.h"
#include "NTFSTarget.h"
#include "BaseClass.h"
#include "FilesetTarget.h"
#include "RTFProc.h"
#include "McastCapsrc.h"
#include "ErrorCode.h"
#include "FTPSource.h"
#include "HostToIP.h"
#include "FTPFilesetSource.h"
#include "VstrmBase.h"
#include "EncodeConvert.h"
#include <math.h>
#include <list>
#include "TargetFac.h"
#include "TargetFactoryI.h"
#include "NtfsFileIoFactory.h"
#include "VstrmFileIoFactory.h"
#include "SelectPort.h"
#include "WPCapThread.h"
#include "CSILibAPI.h"
#include "CSILibErrors.h"

using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;
using namespace CSILib;

#define CPH_CSI			"CPH_CSI"
#define MOLOG					(glog)

NetworkIFSelector*                                      _nNetSelector = NULL;
std::auto_ptr<ZQTianShan::ContentProvision::FileIoFactory> CSISess::_pFileIoFac;
ZQTianShan::ContentProvision::BaseCPHelper* CSIHelper::_theHelper =NULL;
SelectPort* CSISess::_pSelectPort = NULL;

void logCB(HANDLE hSession, const char *pClassString,  char *pMessageString )
{

}
extern "C" __declspec(dllexport) BOOL InitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return FALSE;

	// set log handler
	ZQ::common::setGlogger(pEngine->getLogger());
	_gCPHCfg.setLogger(&glog);

	// load configurations
	std::string strCfgDir;
	strCfgDir = pEngine->getConfigDir();
	if (!_gCPHCfg.loadInFolder(strCfgDir.c_str()))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFileName().c_str());	
		return FALSE;
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "Load configuration from [%s] successfully"),
			_gCPHCfg.getConfigFileName().c_str());

		_gCPHCfg.snmpRegister("");
	}	

	//
	// do some module initialize
	//
	std::string errmsg;
	if (_gCPHCfg.enableTestNTFS)
	{
		NtfsFileIoFactory* pFactory = new NtfsFileIoFactory();
		pFactory->setLog(&glog);
		pFactory->setRootDir(_gCPHCfg.szNTFSOutputDir);
		CSISess::_pFileIoFac.reset(pFactory);
	}
	else
	{
		VstrmFileIoFactory* pFactory = new VstrmFileIoFactory();
		pFactory->setLog(&glog);
		pFactory->setBandwidthManageClientId(_gCPHCfg.vstrmBwClientId);
		pFactory->setDisableBufDrvThrottle(_gCPHCfg.vstrmDisableBufDrvThrottle);
		pFactory->setEnableCacheForIndex(_gCPHCfg.enableCacheForIndex);
		pFactory->setEnableRAID1ForIndex(_gCPHCfg.enableRAID1ForIndex);

		CSISess::_pFileIoFac.reset(pFactory);
	}

	if (!CSISess::_pFileIoFac->initialize())
	{
		std::string strErr;
		int nErrCode;
		CSISess::_pFileIoFac->getLastError(strErr, nErrCode);
		printf("Failed to initialize fileio factory with error: %s, code: %d\n", strErr.c_str(), nErrCode);
		return false;
	}

	TargetFac * pTargetFac = new TargetFac(CSISess::_pFileIoFac.get());
	TargetFactoryI::setInstance(pTargetFac);	

	RTFProcess::initRTFLib(_gCPHCfg.rtfMaxSessionNum, &glog, _gCPHCfg.rtfMaxInputBufferBytes,
		_gCPHCfg.rtfMaxInputBuffersPerSession, _gCPHCfg.rtfSessionFailThreshold);

	//
	// helper
	//
	if (!CSIHelper::_theHelper)
		CSIHelper::_theHelper = new CSIHelper(pEngine->getThreadPool(), pEngine);

	_nNetSelector = new NetworkIFSelector(glog);
	if(!_nNetSelector)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "Failed to new NetIfSelector object."));
		return false;
	}
	for (std::vector< ZQ::common::Config::Holder< NetInterface > >::iterator iter = _gCPHCfg.nInterface.begin();
		iter != _gCPHCfg.nInterface.end(); iter++)
	{
		_nNetSelector->addInterface((*iter).strIp,(*iter).totalBandwidth);
	}

	NetworkIFSelector::InterfaceInfoList infoList;
	_nNetSelector->listInterfaceInfo(infoList);
	if (infoList.size() == 0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "failed to load config of NetworkInterface for Capture, please check CPH_CSI.xml"));
		return false;
	}

	//multicast capture initialize
	{
		WinpCapThreadInterface* pCaptureInterface;
		pCaptureInterface = new WinpCapThreadInterface();
		pCaptureInterface->setKernelBufferBytes(_gCPHCfg.winpcapKernelBufferInMB*1024*1024);
		pCaptureInterface->setMinBytesToCopy(_gCPHCfg.winpcapMinBufferToCopyInKB*1024);

		MulticastCaptureInterface::setInstance(pCaptureInterface);
		pCaptureInterface->setLog(&glog);

		for(unsigned int i=0;i<_gCPHCfg.nInterface.size();i++)
		{
			std::string strLocalIp;
			if (!HostToIP::translateHostIP(_gCPHCfg.nInterface[i].strIp.c_str(), strLocalIp))//translate host name to ip
				strLocalIp = _gCPHCfg.nInterface[i].strIp;

			pCaptureInterface->addNIC(strLocalIp, _gCPHCfg.nInterface[i].totalBandwidth);
		}

		if (!pCaptureInterface->init())
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "Failed to initialize capture interface with error: %s"),
				pCaptureInterface->getLastError().c_str());
			return false;
		}
	}

	//
	// register methods
	//
	for (std::vector< ZQ::common::Config::Holder< Method > >::iterator iter = _gCPHCfg.methods.begin();iter != _gCPHCfg.methods.end();iter++)
	{
		if (iter->enableFlag && iter->maxBandwidth && iter->maxSession)
		{
			pEngine->registerHelper((*iter).methodName.c_str(), CSIHelper::_theHelper, pCtx);
			pEngine->registerLimitation((*iter).methodName.c_str(), (*iter).maxSession,(*iter).maxBandwidth, GROUP_CSI, 0);
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "%s Helper registered"),(*iter).methodName.c_str());
		}
		else
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "%s Helper disabled"),(*iter).methodName.c_str());
		}
	}

	//set ftp port range		
	if (_gCPHCfg.ranges.size() && _gCPHCfg.ranges[0].portEnable)
	{
		CSISess::_pSelectPort = new SelectPort();
		std::vector<int> portVec;
		for (int i = _gCPHCfg.ranges[0].portMin; i <= _gCPHCfg.ranges[0].portMax; i++)
			portVec.push_back(i);
		CSISess::_pSelectPort->setPortRange(portVec);	
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "Set ftp port range[%d-%d]"),_gCPHCfg.ranges[0].portMin,_gCPHCfg.ranges[0].portMax);
	}
	else
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "Unenable ftp port range setting"));

	DWORD logLevel = 7;
	CSI_ERROR error = csiInitializeLibrary( 10, logCB, logLevel);
	if(error != CSI_MSG_SUCCESS)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI,"failed to Initialize Library"));
		return 0;
	}
	
	return TRUE;
}

extern "C" __declspec(dllexport) void UninitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return;
	
	for (std::vector< ZQ::common::Config::Holder< Method > >::iterator iter = _gCPHCfg.methods.begin();iter != _gCPHCfg.methods.end();iter++)
	{
		if ((*iter).enableFlag == 1)
		{
			pEngine->unregisterHelper((*iter).methodName.c_str(), pCtx);
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "%s Helper unregistered"),(*iter).methodName.c_str());
		}
	}

	if (CSIHelper::_theHelper)
	{
		try
		{
			delete CSIHelper::_theHelper;
		}
		catch(...){};
		
		CSIHelper::_theHelper = NULL;
	}
	
	if (_gCPHCfg.ranges.size() && _gCPHCfg.ranges[0].portEnable)
	{
		if (CSISess::_pSelectPort)
			delete CSISess::_pSelectPort;
	}


	if (_nNetSelector)
	{
		delete _nNetSelector;
		_nNetSelector = NULL;
	}
	//
	//do some module uninitialize
	//
	MulticastCaptureInterface::destroyInstance();

	//
	//do some module uninitialize
	//
	RTFProcess::uninitRTFLib();

	if (TargetFactoryI::instance())
	{
		TargetFactoryI::destroyInstance();
	}

	if (CSISess::_pFileIoFac.get())
	{
		CSISess::_pFileIoFac->uninitialize();
		CSISess::_pFileIoFac.reset(0);
	}

	csiCloseLibrary();

//	ZQ::common::setGlogger(NULL);
}

///////////////
#include "UrlStr.h"

//
// for "file:", "cifs:", "nfs:" protocol etc. to UNC format
// 
static bool url2UNC(std::string& path)
{
	char pathbuf[2048];
	const char* szProtocolTag = "://";
	const char* p1 = strstr((char*)path.c_str(), szProtocolTag);
	if (p1)
	{
		pathbuf[0] = FNSEPC;
		pathbuf[1] = FNSEPC;

		const char* p2 = strstr((char*)path.c_str(), "@");
		if (p2)
		{
			strcpy(pathbuf+2, p2 + 1);
		}
		else
		{
			strcpy(pathbuf+2, p1 + strlen(szProtocolTag));
		}
	}
	else
	{
		strcpy(pathbuf, path.c_str());
	}

	for (char* p = pathbuf; *p; p++)
	{
		if ('\\' == *p || '/' == *p) 
			*p = FNSEPC;
	}

	path = pathbuf;
	return true;    
}

static unsigned long timeval()
{
	unsigned long rettime = 1;
	
	FILETIME systemtimeasfiletime;
	LARGE_INTEGER litime;
	
	GetSystemTimeAsFileTime(&systemtimeasfiletime);
	memcpy(&litime,&systemtimeasfiletime,sizeof(LARGE_INTEGER));
	litime.QuadPart /= 10000;  //convert to milliseconds
	litime.QuadPart &= 0xFFFFFFFF;    //keep only the low part
	rettime = (unsigned long)(litime.QuadPart);
	
	return rettime;
}

void getUnifiedTrickExt(int speedNo, char* ext)
{
	if(0 == speedNo)
	{
		sprintf(ext, ".FFR");
	}
	else
	{
		sprintf(ext, ".FFR%d", speedNo);
	}		
}

void getTrickExt(int speedNo, char* ext1, char* ext2)
{
	if(0 == speedNo)
	{
		sprintf(ext1, ".FF");
		sprintf(ext2, ".FR");
	}
	else
	{
		sprintf(ext1, ".FF%d", speedNo);
		sprintf(ext2, ".FR%d", speedNo);
	}		
}


CSISess::~CSISess()
{
	_nNetSelector->freeInterface(_strFileName);
	cleanup();
}

void CSISess::cleanup()
{
	if (_bCleaned)
		return;

	BaseGraph::Close();
	_bCleaned = true;
}


bool CSISess::preLoad()
{
	if (!_sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "provision session is 0"));
		setErrorInfo(ERRCODE_NULL_SESSION, "provision session is 0");			
		return false;
	}

	_strMethod = _sess->methodType;
	if(stricmp(_strMethod.c_str(), METHODTYPE_CSI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] unsupported method %s"), _sess->ident.name.c_str(),
			_strMethod.c_str());
		setErrorInfo(ERRCODE_UNSUPPORT_METHOD, "unsupported method");			
		return false;
	}

	_bAudioFlag = false;
	std::string strFilename;
	std::string strSrcUrl;
	if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] could not find resource URI"), _sess->ident.name.c_str());
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find resource URI");			
		return false;
	}

	TianShanIce::ValueMap& resURI = _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource " CPHPM_FILENAME);			
		return false;
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource: " CPHPM_FILENAME);			
		return false;
	}

	strFilename = var.strs[0];
	_strFileName = strFilename;

	if (resURI.end() != resURI.find(CPHPM_SOURCEURL))
	{
		TianShanIce::Variant& var2 = resURI[CPHPM_SOURCEURL];
		if (var2.type == TianShanIce::vtStrings && var2.strs.size()>0)
		{
			strSrcUrl = var2.strs[0];
		}
	}
	if (resURI.end() != resURI.find(CPHPM_SUBTYPE))
	{
		TianShanIce::Variant& var3 = resURI[CPHPM_SUBTYPE];
		if (var3.type == TianShanIce::vtStrings && var3.strs.size()>0)
		{
			_bAudioFlag = (bool)atoi(var3.strs[0].c_str());
		}
	}

	TianShanIce::ValueMap& resBw = _sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] could not find ProvisionBandwidth resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}
    int nBandwidth = (int)var1.lints[0];
    _nBandwidth = nBandwidth;

	TianShanIce::Properties prop = _sess->props;
	TianShanIce::Properties::const_iterator it = prop.find(CPHPM_INDEXTYPE);

	memset((void*) _augmentationPids, 0 , sizeof(_augmentationPids));
	_augmentationPidCount = 0;

	std::string strPIDs = "";
	it = prop.find(CPHPM_AUGMENTATIONPIDS);
	if (it!=prop.end())
	{
		strPIDs = it->second;
	}
	else
	{
		strPIDs = _gCPHCfg.strAugmentationPids;
	}

	if(!strPIDs.empty())
	{
		TianShanIce::StrValues  strAugmentationPids;
		::ZQ::common::stringHelper::SplitString(strPIDs, strAugmentationPids, ";,");
		unsigned int i;
		for(i = 0; i < strAugmentationPids.size() && i < MAX_AUGMENTATION_PIDS; ++i)
		{
			_augmentationPids[i] = atoi(strAugmentationPids[i].c_str());
		}
		_augmentationPidCount = strAugmentationPids.size();
	}

	_enableNoTrickSpeed = false;

	it = prop.find(CPHPM_NOTRICKSPEEDS);
	if (it != prop.end())
	{
		int major = 0, minor = 0;
		RTFProcess::getCTFLibVersion(major, minor);
		if(major < 3)
		{
			MOLOG(Log::L_WARNING, CLOGFMT(CPH_CSI, "[%s] CTFLib Verion support No TrickSpeed File must >= 3.0 "), _sess->ident.name.c_str());
		}
		_enableNoTrickSpeed = true;
	}

	::TianShanIce::ContentProvision::TrickSpeedCollection trickcol = _sess->trickSpeeds;
	if (trickcol.size() == 0)
		trickcol.push_back(7.5);
	for (::TianShanIce::ContentProvision::TrickSpeedCollection::iterator iterTrick = trickcol.begin();iterTrick != trickcol.end();iterTrick++)
	{
		_trickspeed.push_back((*iterTrick));
	}
    
	SetLog(_helper.getLog());
	SetMemAlloc(_helper.getMemoryAllocator());
	SetLogHint(strFilename.c_str());

	if (_gCPHCfg.bandwidthLimitRate)
	{
		_nMaxBandwidth = int(((float)nBandwidth) * _gCPHCfg.bandwidthLimitRate / 100);
	}
	else
		_nMaxBandwidth = nBandwidth;
	
	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_CSI, "[%s] bandwidth[%d], max_limit_rate[%d], maxbandwidth[%d]"), _strLogHint.c_str(), 
		nBandwidth, _gCPHCfg.bandwidthLimitRate, _nMaxBandwidth);

	ZQ::common::URLStr src(strSrcUrl.c_str());
	const char* proto = src.getProtocol();

	if(proto == NULL)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] could not find Protocol in sourceUrl %s"), strSrcUrl.c_str(), 
			CPHPM_SOURCEURL);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_CSI, 0, " could not find Protocol in sourceUrl " CPHPM_SOURCEURL);
	}

     if (!stricmp(proto, "file") || !stricmp(proto, "cifs"))
//	if(!stricmp(_strMethod.c_str(),METHODTYPE_NTFSRTFVSVSTRM)|| (bH264Type=!stricmp(_strMethod.c_str(),METHODTYPE_NTFSRTFH264VSVSTRM)))
	{
		char utf8[]="utf-8";
		std::string srcFilename = strSrcUrl;
		bool enableUtfFlag = false;
		char target[MAX_PATH];

		if (URLStr::decode(srcFilename.c_str(),(void*)target, MAX_PATH))
			srcFilename = target;
		else
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] Failed to decode url %s"), _strLogHint.c_str(),srcFilename.c_str());
			return false;
		}

		if (stricmp(_gCPHCfg.szUrlEncode,utf8)== 0)
		{
			enableUtfFlag = true;
		}

		url2UNC(srcFilename);

		NTFSIOSource* ntfsSource = (NTFSIOSource*)SourceFactory::Create(SOURCE_TYPE_NTFSSRC, &_helper._pool);
		AddFilter(ntfsSource);    //only after this, the log handle will be parsed in
		ntfsSource->setMaxBandwidth(_nMaxBandwidth);	
		ntfsSource->setFileName(srcFilename.c_str());
		ntfsSource->setUtfFlag(enableUtfFlag);
		_pSource = ntfsSource;
		_sourceURL = srcFilename;
	}
	else if (!stricmp(proto, "ftp"))
//	else if(!stricmp(_strMethod.c_str(),METHODTYPE_FTPRTFVSVSTRM)|| (bH264Type=!stricmp(_strMethod.c_str(),METHODTYPE_FTPRTFH264VSVSTRM)))
	{
		std::string url = strSrcUrl;

		FTPIOSource* ftpsource = (FTPIOSource*)SourceFactory::Create(SOURCE_TYPE_FTP, &_helper._pool);
		AddFilter(ftpsource);    //only after this, the log handle will be parsed in
		ftpsource->setLocalNetworkInterface(_gCPHCfg.szLocalNetIf);
		ftpsource->setURL(url.c_str());
		ftpsource->setMaxBandwidth(_nMaxBandwidth);
		ftpsource->setConnectionMode(_gCPHCfg.enableFtpPassiveMode);
		ftpsource->setConnectionInterval(_gCPHCfg.ftpConnectionInterval);
		ftpsource->setDecodeSourceURL(_gCPHCfg.decodeSourceURL);
		_pSource = ftpsource;
		_sourceURL = url;
	}	
    else if (!stricmp(proto, "udp"))
	{
		std::string multicastUrl = strSrcUrl;

		 std::string multicastIp = multicastUrl.substr(multicastUrl.find_first_of(':')+3,multicastUrl.find_last_of(':')-6);
		std::string strmulticastPort = multicastUrl.substr(multicastUrl.find_last_of(':')+1,multicastUrl.size()-multicastUrl.find_last_of(':')-1);
		int multicastPort = atoi(strmulticastPort.c_str());

		DWORD timeoutInterval = _gCPHCfg.timeoutInterval;
		std::string localIp;
		if (!_nNetSelector->allocInterface(_nMaxBandwidth,localIp,strFilename))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] Failed to allocate proper network card"), _strLogHint.c_str());
			return false;
		}
		if (!HostToIP::translateHostIP(localIp.c_str(),_strLocalIp))//translate host name to ip
			_strLocalIp = localIp;
		McastCapSource* mcastSource = (McastCapSource*)SourceFactory::Create(SOURCE_TYPE_MCASTCAPSRC, &_helper._pool);

		AddFilter(mcastSource);
		mcastSource->setInspectPara(multicastIp,multicastPort,timeoutInterval,_strLocalIp);

		//dumper parameters
		mcastSource->enableDump(_gCPHCfg.enableDump);
		mcastSource->setDumpPath(_gCPHCfg.szDumpPath);
		mcastSource->deleteDumpOnSuccess(_gCPHCfg.deleteDumpOnSuccess);
		_pSource = mcastSource;
		_sourceURL = strSrcUrl;

	}
	_protocol = proto;

//////////////////////// 移至RTFProcess run /////////////////////////
/*	std::map<std::string, int> exMap;
	std::map<std::string, int>::iterator iter;
	trickspeed.sort();
	int index = 0;
	for (unsigned int i = 0; i < trickspeed.size(); i++)
	{
		char ex[10]={0};
		char exr[10] ={0};

		if (bH264Type && _gCPHCfg.unifiedtrickfile.enable)
		{
			getUnifiedTrickExt(index,ex);

			exMap.insert(std::make_pair(std::string(ex),i));		
		}
		else
		{
			getTrickExt(index,ex,exr);

			exMap.insert(std::make_pair(std::string(ex),i));
			exMap.insert(std::make_pair(std::string(exr),i));
		}	
		index++;
	}

	if (bAudioFlag)
		exMap.clear();

	int outPutNum = 2 + exMap.size();

	if (outPutNum < 2 && !bAudioFlag)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] Not specify trick speed"), _strLogHint.c_str());
		return false;
	}
*/
////////////////////////////////////////////////////////////////////////////////////////////////

	RTFProcess* pProcess = (RTFProcess*)ProcessFactory::Create(PROCESS_TYPE_RTF, &_helper._pool);
	AddFilter(pProcess);
////////////// 移至RTFProcess run /////////////////////
//	pProcess->setTrickFile(exMap);
//////////////////////////////////////////////////////
	if(!_enableNoTrickSpeed)
		pProcess->settrickSpeedNumerator(_trickspeed);

	pProcess->setUnifiedTrickFile(_gCPHCfg.unifiedtrickfile.enable);
	if (_bAudioFlag)
		pProcess->setAudioOnly(_gCPHCfg.enableLegacyAudioOnly);

////////////// 移至RTFProcess run /////////////////////
/*	if (bH264Type)
	{
		pProcess->setTrickGenParam(CTF_INDEX_TYPE_VV2, CTF_VIDEO_CODEC_TYPE_H264);
	}
	if (!stricmp(_strMethod.c_str(), METHODTYPE_FTPRTFVSVSTRM) || !stricmp(_strMethod.c_str(), METHODTYPE_NTFSRTFVSVSTRM))
	{
		pProcess->setAugmentationPids(augmentationPids ,augmentationPidCount);
	}
*/
////////////////////////////////////////////////////
	_pRTFProc = pProcess;

	{
		FilesetTarget* pTarget = (FilesetTarget*)TargetFactoryI::instance()->create(TARGET_TYPE_FILESET);
		if(!AddFilter(pTarget))
			return false;
		pTarget->setFilename(strFilename.c_str());
		pTarget->setCacheDirectory(_gCPHCfg.szCacheDir);
		pTarget->enablePacingTrace(_gCPHCfg.enablePacingTrace);
		pTarget->traceIndexWrite(_gCPHCfg.traceIndexWrite);
		pTarget->setEnableRAID1ForIndex(_gCPHCfg.enableRAID1ForIndex);
		pTarget->setBandwidth(_nMaxBandwidth);
		pTarget->enableProgressEvent(true);
		pTarget->enableMD5(_gCPHCfg.enableMD5);

		if(!_enableNoTrickSpeed)
			pTarget->setTrickSpeed(_trickspeed);

////////////// 移至RTFProcess run /////////////////////
/*		pTarget->setTrickFile(exMap);
		if (bH264Type)
		{
			pTarget->setTypeH264();
			pTarget->enablePacing(true);

			pTarget->enableStreamableEvent(_gCPHCfg.enableStreamEvent);
		}
		else
		{
			pTarget->enablePacing(true);
			pTarget->enableStreamableEvent(_gCPHCfg.enableStreamEvent);
		}*/
//////////////////////////////////////////////////////

		_pMainTarget = pTarget;

////////////// 移至RTFProcess run /////////////////////
//		InitPins();


//		if (!ConnectTo(_pSource, 0, pProcess, 0))
//			return false;

//		for (int i = 0; i < outPutNum; i++)
//			if (!ConnectTo(pProcess, i, pTarget, i))
//				return false;

//////////////////////////////////////////////////////
	}

	SetMediaSampleSize(_gCPHCfg.mediaSampleSize);

/*	if (!Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		setErrorInfo(_nLastErrCode, (std::string("Failed to initialize graph with error: ") + _strLastErr).c_str());			
		return false;
	}
*/
	BaseCPHSession::preLoad();

	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] preload successful"), _strLogHint.c_str());
	return true;
}
bool CSISess::checkFileFormat(const std::string& protocol, const std::string& sourceUrl, bool& bH264)
{
	std::string strFileNameTemp = _strFileName + ".checkFF";
	bH264 = false;
	bool bChecked = false;
	BaseSource* pSource = NULL;
	if (!stricmp((char*)protocol.c_str(), "file") || !stricmp((char*)protocol.c_str(), "cifs"))
	{
		char utf8[]="utf-8";
		bool enableUtfFlag = false;
		if (stricmp(_gCPHCfg.szUrlEncode,utf8)== 0)
		{
			enableUtfFlag = true;
		} 
		NTFSIOSource* ntfsSource = (NTFSIOSource*)SourceFactory::Create(SOURCE_TYPE_NTFSSRC, &_helper._pool);
		if(ntfsSource == NULL)
			return false;
		ntfsSource->setFileName(sourceUrl.c_str());
		ntfsSource->setUtfFlag(enableUtfFlag);
		ntfsSource->SetLogHint(_strFileName.c_str());
		pSource = ntfsSource;
	}
	else if (!stricmp((char*)protocol.c_str(), "ftp"))
	{

		FTPIOSource* ftpsource = (FTPIOSource*)SourceFactory::Create(SOURCE_TYPE_FTP, &_helper._pool);
		if(ftpsource == NULL)
			return false;
		ftpsource->setLocalNetworkInterface(_gCPHCfg.szLocalNetIf);
		ftpsource->setURL(sourceUrl.c_str());
		ftpsource->setConnectionMode(_gCPHCfg.enableFtpPassiveMode);
		ftpsource->setConnectionInterval(_gCPHCfg.ftpConnectionInterval);
		ftpsource->setDecodeSourceURL(_gCPHCfg.decodeSourceURL);
		ftpsource->SetLogHint(_strFileName.c_str());
		pSource = ftpsource;

	}	
	else if(!stricmp((char*)protocol.c_str(), "udp"))
	{
		std::string multicastUrl = sourceUrl;

		std::string multicastIp = multicastUrl.substr(multicastUrl.find_first_of(':')+3,multicastUrl.find_last_of(':')-6);
		std::string strmulticastPort = multicastUrl.substr(multicastUrl.find_last_of(':')+1,multicastUrl.size()-multicastUrl.find_last_of(':')-1);
		int multicastPort = atoi(strmulticastPort.c_str());

		DWORD timeoutInterval = _gCPHCfg.timeoutInterval;
		std::string localIp;
		if (!_nNetSelector->allocInterface(_nMaxBandwidth,localIp,strFileNameTemp))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] Failed to allocate proper network card"), _strLogHint.c_str());
			return false;
		}
		if (!HostToIP::translateHostIP(localIp.c_str(),_strLocalIp))//translate host name to ip
			_strLocalIp = localIp;
		McastCapSource* mcastSource = (McastCapSource*)SourceFactory::Create(SOURCE_TYPE_MCASTCAPSRC, &_helper._pool);
		if(mcastSource == NULL)
			return false;
		mcastSource->SetLogHint(_strFileName.c_str());
	    mcastSource->SetLog(&glog);
		mcastSource->setInspectPara(multicastIp,multicastPort,timeoutInterval,_strLocalIp);
		pSource = mcastSource;
		//dumper parameters
	}
	else
	{
		return false;
	}
	pSource->SetGraph(this);
    pSource->SetLog(&glog);
	pSource->InitPins();
	if(!pSource->Init())
	{
		delete pSource;
		return false;
	}
	pSource->Start();

	MediaSample* pSample = pSource->GetData();
	if (NULL == pSample)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] checkFileFormat() GetData return 0"), _strLogHint.c_str());
		pSource->Stop();
		pSource->Close();
		delete pSource;
		return false;
	}

	HANDLE phSession = NULL;
	CSI_ERROR error =  csiAddSession(&phSession );
	if(error != CSI_MSG_SUCCESS)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s]failed to Add Session"), _strLogHint.c_str());
		pSource->Stop();
		pSource->Close();
		delete pSource;
		return false;
	}

	do 
	{
		pSample->addRef();

		CSI_STREAM_INFORMATION streamInfo;
		memset((void*)&streamInfo, 0, sizeof(CSI_STREAM_INFORMATION));

		error = csiProcessBuffer(phSession, (BYTE *)pSample->getPointer(),pSample->getBufSize(), &streamInfo);

		if(! (streamInfo.flags &  CSI_INFO_VCODEC))
		{
			freeMediaSample(pSample);
			pSample = pSource->GetData();
			continue;
		}
		else
		{
			MOLOG(Log::L_DEBUG, CLOGFMT(CPH_CSI,"[%s]checkFileFormat() flags(%d), length(%d), videoCodec(%s), TBD(%s)"), _strLogHint.c_str(), streamInfo.flags, streamInfo.length, streamInfo.videoCodec, streamInfo.TBD);
			if(!stricmp(streamInfo.videoCodec,"H.264"))
			{
				bH264 = true;
				_sourceType = TianShanIce::Storage::ctH264;	
			}
			else if(!stricmp(streamInfo.videoCodec,"Mpeg2"))
			{
				bH264 = false;
				_sourceType = TianShanIce::Storage::ctMPEG2TS;	
			}
			else
			{
				MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI,"[%s]checkFileFormat() unkonwn source file format"), _strLogHint.c_str());
				break;
			} 
			bChecked = true;

			MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI,"[%s]checkFileFormat() source contentType =[%s]"), _strLogHint.c_str(), streamInfo.videoCodec);
			break;

		}

	} while(pSample != NULL);


	if(pSample)
	{
		freeMediaSample(pSample);
		pSample = NULL;
	}

	if(phSession)
		csiCloseSession(phSession);

	if(!stricmp((char*)protocol.c_str(), "udp"))
	{
		_nNetSelector->freeInterface(strFileNameTemp);
	}
	
	pSource->Stop();
	pSource->Close();
	delete pSource;
	return true;
}
bool  CSISess::setSpeedInfo(bool bH264Type)
{
	std::map<std::string, int> exMap;
	std::map<std::string, int>::iterator iter;
	_trickspeed.sort();
	int index = 0;
	for (unsigned int i = 0; i < _trickspeed.size(); i++)
	{
		char ex[10]={0};
		char exr[10] ={0};

		if (bH264Type && _gCPHCfg.unifiedtrickfile.enable)
		{
			getUnifiedTrickExt(index,ex);

			exMap.insert(std::make_pair(std::string(ex),i));		
		}
		else
		{
			getTrickExt(index,ex,exr);

			exMap.insert(std::make_pair(std::string(ex),i));
			exMap.insert(std::make_pair(std::string(exr),i));
		}	
		index++;
	}

	if (_bAudioFlag)
		exMap.clear();

	int outPutNum = 2 + exMap.size();

	if(_enableNoTrickSpeed)
		outPutNum = 2;

	if ( !_enableNoTrickSpeed && outPutNum < 2 && !_bAudioFlag)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] Not specify trick speed"), _strLogHint.c_str());
		return false;
	}

	if(NULL == _pRTFProc)
	{
		return false;
	}

	RTFProcess* pProcess = (RTFProcess*)_pRTFProc;

	if(!_enableNoTrickSpeed)
	{
		pProcess->setTrickFile(exMap);
	}
	if (bH264Type)
	{
		pProcess->setTrickGenParam(CTF_INDEX_TYPE_VV2, CTF_VIDEO_CODEC_TYPE_H264);
	}
	if (!_gCPHCfg.enableTestNTFS)
	{
		pProcess->setAugmentationPids(_augmentationPids ,_augmentationPidCount);
	}

	if(NULL == _pMainTarget)
	{
		return false;
	}

	FilesetTarget* pTarget = (FilesetTarget*)_pMainTarget;

	if(!_enableNoTrickSpeed)
	{
		pTarget->setTrickFile(exMap);
	}
	if (bH264Type)
	{
		pTarget->setTypeH264();
		pTarget->enablePacing(true);

		pTarget->enableStreamableEvent(_gCPHCfg.enableStreamEvent);
	}
	else
	{
		pTarget->enablePacing(true);
		pTarget->enableStreamableEvent(_gCPHCfg.enableStreamEvent);
	}

	InitPins();


	if (!ConnectTo(_pSource, 0, pProcess, 0))
		return false;

	for (int i = 0; i < outPutNum; i++)
		if (!ConnectTo(pProcess, i, pTarget, i))
			return false;

	if (!Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		setErrorInfo(_nLastErrCode, (std::string("Failed to initialize graph with error: ") + _strLastErr).c_str());			
		return false;
	}

	return true;

}

bool CSISess::Start()
{
	bool bH264 = false;
	if(!checkFileFormat(_protocol, _sourceURL, bH264))
	{
      return false;
	}
	if(!setSpeedInfo(bH264))
	{
		return false;
	}
	std::vector<BaseFilter*>::iterator it;
	for(it=_filters.begin();it!=_filters.end();it++)
	{
		if (*it!=NULL)
		{
			if (!(*it)->Start())
			{
				MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] (%s) failed to start"), _strLogHint.c_str(), (*it)->GetName());
				return false;
			}			

			MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] (%s) started successfully"), _strLogHint.c_str(), (*it)->GetName());
		}
	}

	MOLOG(Log::L_INFO, CLOGFMT(CSISess, "[%s] Graph started"), _strLogHint.c_str());

	return true;
}
void CSISess::terminate(bool bProvisionSuccess)
{
	// not started, just constructed
	if (getStatus() == NativeThread::stDeferred)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] to terminate() session while in init status"), _strLogHint.c_str());
		if (!BaseGraph::IsErrorOccurred())
		{
			BaseGraph::SetLastError("User canceled provision", ERRCODE_USER_CANCELED);
		}
		cleanup();
		try
		{
			delete this;
		}
		catch (...)
		{			
		}
	}
	else
	{
		//
		// thread already started, let final to delete this
		//
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] to terminate() session with status[%s]"), _strLogHint.c_str(),
			bProvisionSuccess?"success":"failure");
		_bQuit = true;

		if (!bProvisionSuccess && !BaseGraph::IsErrorOccurred())
		{
			BaseGraph::SetLastError("User canceled provision", ERRCODE_USER_CANCELED);
		}

		BaseGraph::Stop();
	}
}

bool CSISess::getProgress(::Ice::Long& offset, ::Ice::Long& total)
{	
	total = _filesize;
	offset = getProcessBytes();
	return true;
} 

int CSISess::run(void)
{	
	Ice::Long llStart = ZQTianShan::now();
    _helper.increaseLoad(_strMethod,_nBandwidth);
	bool bRet = Run();
	_helper.decreaseLoad(_strMethod,_nBandwidth);
	::TianShanIce::Properties params;	
	if (!bRet)
	{
		setErrorInfo(_nLastErrCode, (std::string("Provisioning failed with error: ") + _strLastErr).c_str());			
		
		if (!_bStartEventSent)
		{
			notifyStarted(params);
		}
		notifyStopped(true, params);

		MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] provision done, status[failure], error[%s]"), 
			_strLogHint.c_str(), _strLastErr.c_str());
		Close();
		return 0;
	}

	Close();

	LONGLONG supportFileSize=0;
	std::string md5;

	if (!_gCPHCfg.enableTestNTFS)
	{
		if (_pMainTarget)
		{
			FilesetTarget* pTarget = (FilesetTarget*)_pMainTarget;
			supportFileSize = pTarget->getSupportFileSize();
			pTarget->getMD5(md5);
		}
	}

	char tmp[64];
	if (!_llTotalBytes)
	{
		_llTotalBytes = _llProcBytes;
	}
	sprintf(tmp, "%lld", _llTotalBytes);
	params[EVTPM_TOTOALSIZE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] Filesize[%s]"), _strLogHint.c_str(), tmp);

	sprintf(tmp, "%lld", supportFileSize);
	params[EVTPM_SUPPORTFILESIZE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] SupportFilesize[%s]"), _strLogHint.c_str(), tmp);

	params[EVTPM_MD5CHECKSUM] = md5;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] md5[%s]"), _strLogHint.c_str(), md5.c_str());

	if (_bitrate)
	{
		sprintf(tmp, "%lld", _llProcBytes*8000/_bitrate);
		params[EVTPM_PLAYTIME] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] playtime[%s]"), _strLogHint.c_str(), tmp);
	}
	else 
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] bitrate [0] , can't get playtime"), _strLogHint.c_str());
	}
	if (_pRTFProc  && _gCPHCfg.enableTestNTFS)
	{
		RTFProcess* pRtfprocess = (RTFProcess*)_pRTFProc;
		bool bEncrypt = pRtfprocess->getPreEncrypt();
		int augmentedBitRate;
		int originalBitRate;
		std::string augmentionpids;
		if(bEncrypt)
		{
			pRtfprocess->getAugmentationPids(augmentionpids);	
			params[EVTPM_AUGMENTATIONPIDS] = augmentionpids;
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] AugmentationPids[%s]"), _strLogHint.c_str(),augmentionpids.c_str());

			pRtfprocess->getPreEncryptBitRate(augmentedBitRate, originalBitRate);

			sprintf(tmp, "%d", originalBitRate);
			params[EVTPM_ORIGINALBITRATE] = tmp;
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] OrignalBiterate[%s]"), _strLogHint.c_str(),tmp);

			sprintf(tmp, "%d", augmentedBitRate);
			params[EVTPM_AUGMENTATEDBITRATE] = tmp;
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] AugmentatedBitarate[%s]"), _strLogHint.c_str(),tmp);

			sprintf(tmp, "1");
			params[EVTPM_PREENCRYPTION] = tmp;
			MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] PreEncryption[%s]"), _strLogHint.c_str(),tmp);
		}
	}
	if (!_bStartEventSent)
	{
		notifyStarted(params);
	}
	notifyStopped(false, params);

	int nTimeSpentMs = int(ZQTianShan::now() - llStart);
	int nActualBps;
	if (nTimeSpentMs)
	{
		nActualBps = int(_llProcBytes*8000/nTimeSpentMs);
	}
	else
	{
		nActualBps = 0;
	}
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] provision done, status[success], spent[%d]ms, actualspeed[%d]bps"), 
		_strLogHint.c_str(), nTimeSpentMs, nActualBps);
	return 0;
}

void CSISess::final(int retcode, bool bCancelled)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] final() ret=%d, cancelled=%c; calling doCleanup()"), 
		_strLogHint.c_str(),retcode, bCancelled?'Y':'N');
	
	cleanup();
	try
	{
		delete this;
	}catch(...)
	{
	}
}


bool CSIHelper::validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource)
{
	if (!sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "provision session is 0"));
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_CSI, 0, "provision session is 0");
	}
	
	std::string strMethod = sess->methodType;
	if(stricmp(strMethod.c_str(), METHODTYPE_CSI ))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] unsupported method %s"), sess->ident.name.c_str(),
			strMethod.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_CSI, 0, "unsupported method %s", strMethod.c_str());
	}
	
	if (sess->resources.end() == sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] could not find resource URI"), sess->ident.name.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_CSI, 0, "could not find resource URI");
	}

	TianShanIce::ValueMap& resURI = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_CSI, 0, "could not find URI resource " CPHPM_FILENAME);
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_CSI, 0, "could not find URI resource " CPHPM_FILENAME);
	}

	TianShanIce::ValueMap& resBw = sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_CSI, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_CSI, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}


	TianShanIce::ValueMap& resSrc = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resSrc.end() == resSrc.find(CPHPM_SOURCEURL))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_CSI, 0, "could not find URI resource " CPHPM_SOURCEURL);
	}

	TianShanIce::Variant& var2 = resSrc[CPHPM_SOURCEURL];
	if (var2.type != TianShanIce::vtStrings || var2.bRange || var2.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_CSI, 0, "could not find URI resource " CPHPM_SOURCEURL);
	}

	std::string sourceUrl = var2.strs[0];


	ZQ::common::URLStr src(sourceUrl.c_str());
	const char* proto = src.getProtocol();

	if(proto == NULL)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] could not find Protocol in sourceUrl %s"), sourceUrl.c_str(), 
			CPHPM_SOURCEURL);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_CSI, 0, " could not find Protocol in sourceUrl " CPHPM_SOURCEURL);
	}
    if (!stricmp(proto, "file") || !stricmp(proto, "cifs"))
	{	
		std::string srcFilename;
		char target[MAX_PATH];

		if (URLStr::decode(sourceUrl.c_str(),(void*)target, MAX_PATH))
			srcFilename = target;
		else
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] Failed to decode url[%s]"), sess->ident.name.c_str(), sourceUrl.c_str());
			srcFilename = sourceUrl;
		}

		url2UNC(srcFilename);

		const char* utf8="utf-8";
		if (stricmp(_gCPHCfg.szUrlEncode,utf8)== 0)
		{
			std::wstring unifilename;
			
			//EncodeConvert::utf8_to_ansi(srcFilename, tempFilename);
			EncodeConvert::utf8_to_unicode(srcFilename,unifilename);

			HANDLE hFile = CreateFileW(unifilename.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				std::string errmsg;
				getSystemErrorText(errmsg);
				MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] sourceurl[%s], could not open file [%S] because of \"%s\""), sess->ident.name.c_str(), 
					srcFilename.c_str(), unifilename.c_str(),errmsg.c_str());
				ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_CSI, TianShanIce::Storage::csexpInvalidSourceURL, "could not open file %S ", unifilename.c_str());
			}
			CloseHandle(hFile);
		}
		else
		{
			HANDLE hFile = CreateFile(srcFilename.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				std::string errmsg;
				getSystemErrorText(errmsg);
				MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] could not open file %s because of \"%s\""), sess->ident.name.c_str(), 
					srcFilename.c_str(),errmsg.c_str());
				ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_CSI, TianShanIce::Storage::csexpInvalidSourceURL, "could not open file %s ", srcFilename.c_str());
			}
			CloseHandle(hFile);
		}

		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	}
	else if (!stricmp(proto, "ftp"))
	{
		std::string url = sourceUrl;
		std::string strpro = url.substr(0,6);
		if (_stricmp(strpro.c_str(),"ftp://")!= 0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_CSI, "[%s] Can't find the FTP protocol from url %s"), sess->ident.name.c_str(), url.c_str());
			ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_CSI, TianShanIce::Storage::csexpInvalidSourceURL, "Can't find the FTP protocol from url" ,url.c_str());
		}

		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	}
	else if (!stricmp(proto, "udp"))
	{

	}
	else
	{
		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
		MOLOG(Log::L_WARNING, CLOGFMT(CPH_CSI, "[%s] Unknown method[%s], setSessionType() with default type"), sess->ident.name.c_str(), strMethod.c_str());
	}

	return true;
}

bool CSISess::prime()
{
	if (!BaseCPHSession::prime())
		return false;

	if (!Start())
	{
		setErrorInfo(_nLastErrCode, (std::string("Failed to start graph with error: ") + _strLastErr).c_str());			
		MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] failed to start graph withe error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}

	MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] prime() successful"), _strLogHint.c_str());
	return true;
}

void CSISess::OnProgress(LONGLONG& prcvBytes)
{
	BaseGraph::OnProgress(prcvBytes);
	::TianShanIce::Properties params;
	updateProgress(_llProcBytes, _llTotalBytes,params);
}

void CSISess::OnStreamable(bool bStreamable)
{
	BaseGraph::OnStreamable(bStreamable);
	notifyStreamable(bStreamable);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void CSISess::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());

	char tmp[40];
	::TianShanIce::Properties params;
	sprintf(tmp, "%d", mInfo.bitrate);
	params[EVTPM_MPEGBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] bitrate[%s]"), _strLogHint.c_str(),tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionV);
	params[EVTPM_VIDEOHEIGHT] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] videoResolutionV[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionH);
	params[EVTPM_VIDEOWIDTH] =  tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] videoResolutionH[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoBitrate);
	params[EVTPM_VIDEOBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] videoBitrate[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp,"%.2f",mInfo.framerate);
	params[EVTPM_FRAMERATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] framerate[%s]"), _strLogHint.c_str(),tmp);
	params[EVTPM_SOURCETYPE] = _sourceType;
	_bitrate = mInfo.bitrate;
	
	notifyStarted(params);
	_bStartEventSent = true;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_CSI, "[%s] notifyStarted() called"), _strLogHint.c_str());

	if (_pMainTarget)
	{
		_pMainTarget->setStreamableBytes(mInfo.bitrate*_gCPHCfg.streamReqSecs/8);
	}
}

CSIHelper::CSIHelper(ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr)
:BaseCPHelper(pool, mgr) 
{
	CPHCSICfg::Methods::iterator it;
	for (it = _gCPHCfg.methods.begin();it != _gCPHCfg.methods.end();it++)
	{
		if (!it->maxSession || !it->maxBandwidth)
			continue;

		_methodCostList[it->methodName] = new MethodCostI(it->maxBandwidth, it->maxSession);				
	}			
}

CSIHelper::~CSIHelper()
{
	MethodCostList::iterator it = _methodCostList.begin();
	for (;it!=_methodCostList.end();it++)
	{
		if (it->second)
			delete it->second;
	}

	_methodCostList.clear();
}

bool CSIHelper::getBandwidthLoad(const char* methodType, long& bpsAllocated, long& bpsMax, long& initCost)
{
	if (stricmp(methodType, METHODTYPE_CSI))
		return false;
	
	bpsAllocated =0;

	CPHCSICfg::Methods::iterator methodIter;
	for (methodIter = _gCPHCfg.methods.begin();methodIter != _gCPHCfg.methods.end();methodIter++)
	{
		if (stricmp((*methodIter).methodName.c_str(),methodType) == 0)
			break;
	}
	if (methodIter == _gCPHCfg.methods.end())
		return false;

	bpsMax=(*methodIter).maxBandwidth;
	initCost =0;
	return true;
}

/// query the current load information of a method type
///@param[in] methodType to specify the method type to query
///@param[out] allocatedKbps the current allocated bandwidth in Kbps
///@param[out] maxKbps the maximal allowed bandwidth in Kbps, -1 if unlimited
///@param[out] sessions the current running session instances
///@param[out] maxSessins the maximal allowed session instances, -1 if unlimited
///@return true if the query succeeded
bool CSIHelper:: getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins)
{
	CPHCSICfg::Methods::iterator methodIter;
	for (methodIter = _gCPHCfg.methods.begin();methodIter != _gCPHCfg.methods.end();methodIter++)
	{
		if (stricmp((*methodIter).methodName.c_str(),methodType) == 0)
			break;
	}
	if (methodIter == _gCPHCfg.methods.end())
		return false;

	maxKbps = (*methodIter).maxBandwidth;
	maxSessins = (*methodIter).maxSession;
	
	getCurrentLoad(methodType,allocatedKbps,sessions);

	return true;
}
