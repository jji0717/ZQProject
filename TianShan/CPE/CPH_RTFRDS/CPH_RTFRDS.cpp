
#include "BaseClass.h"
#include "CPH_RTFRDS.h"
#include "NTFSSource.h"
#include "NTFSTarget.h"
#include "IPushTrigger.h"
#include "PushSource.h"
#include "BaseClass.h"
#include "FilesetTarget.h"
#include "CPH_RTFRDSCfg.h"
#include "RTFProc.h"
#include "ErrorCode.h"
#include "FTPSource.h"
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

using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define CPH_RTFRDS			"CPH_RTFRDS"
#define MOLOG					(glog)

std::auto_ptr<ZQTianShan::ContentProvision::FileIoFactory> RTFRDSSess::_pFileIoFac;
ZQTianShan::ContentProvision::BaseCPHelper* RTFRDSHelper::_theHelper =NULL;
SelectPort* RTFRDSSess::_pSelectPort = NULL;

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
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFileName().c_str());	
		return FALSE;
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "Load configuration from [%s] successfully"),
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
		RTFRDSSess::_pFileIoFac.reset(pFactory);
	}
	else
	{
		VstrmFileIoFactory* pFactory = new VstrmFileIoFactory();
		pFactory->setLog(&glog);
		pFactory->setBandwidthManageClientId(_gCPHCfg.vstrmBwClientId);
		pFactory->setDisableBufDrvThrottle(_gCPHCfg.vstrmDisableBufDrvThrottle);
		pFactory->setEnableCacheForIndex(_gCPHCfg.enableCacheForIndex);
		pFactory->setEnableRAID1ForIndex(_gCPHCfg.enableRAID1ForIndex);

		RTFRDSSess::_pFileIoFac.reset(pFactory);
	}

	if (!RTFRDSSess::_pFileIoFac->initialize())
	{
		std::string strErr;
		int nErrCode;
		RTFRDSSess::_pFileIoFac->getLastError(strErr, nErrCode);
		printf("Failed to initialize fileio factory with error: %s, code: %d\n", strErr.c_str(), nErrCode);
		return false;
	}

	TargetFac * pTargetFac = new TargetFac(RTFRDSSess::_pFileIoFac.get());
	TargetFactoryI::setInstance(pTargetFac);	

	RTFProcess::initRTFLib(_gCPHCfg.rtfMaxSessionNum, &glog, _gCPHCfg.rtfMaxInputBufferBytes,
		_gCPHCfg.rtfMaxInputBuffersPerSession, _gCPHCfg.rtfSessionFailThreshold);

	//
	// helper
	//
	if (!RTFRDSHelper::_theHelper)
		RTFRDSHelper::_theHelper = new RTFRDSHelper(pEngine->getThreadPool(), pEngine);

	//
	// register methods
	//
	for (std::vector< ZQ::common::Config::Holder< Method > >::iterator iter = _gCPHCfg.methods.begin();iter != _gCPHCfg.methods.end();iter++)
	{
		if (iter->enableFlag && iter->maxBandwidth && iter->maxSession)
		{
			pEngine->registerHelper((*iter).methodName.c_str(), RTFRDSHelper::_theHelper, pCtx);
			pEngine->registerLimitation((*iter).methodName.c_str(), (*iter).maxSession,(*iter).maxBandwidth, GROUP_RTFRDS, 0);
			MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "%s Helper registered"),(*iter).methodName.c_str());
		}
		else
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "%s Helper disabled"),(*iter).methodName.c_str());
		}
	}

	//set ftp port range		
	if (_gCPHCfg.ranges.size() && _gCPHCfg.ranges[0].portEnable)
	{
		RTFRDSSess::_pSelectPort = new SelectPort();
		std::vector<int> portVec;
		for (int i = _gCPHCfg.ranges[0].portMin; i <= _gCPHCfg.ranges[0].portMax; i++)
			portVec.push_back(i);
		RTFRDSSess::_pSelectPort->setPortRange(portVec);	
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "Set ftp port range[%d-%d]"),_gCPHCfg.ranges[0].portMin,_gCPHCfg.ranges[0].portMax);
	}
	else
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "Unenable ftp port range setting"));
	
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
			MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "%s Helper unregistered"),(*iter).methodName.c_str());
		}
	}

	if (RTFRDSHelper::_theHelper)
	{
		try
		{
			delete RTFRDSHelper::_theHelper;
		}
		catch(...){};
		
		RTFRDSHelper::_theHelper = NULL;
	}
	
	if (_gCPHCfg.ranges.size() && _gCPHCfg.ranges[0].portEnable)
	{
		if (RTFRDSSess::_pSelectPort)
			delete RTFRDSSess::_pSelectPort;
	}
	//
	//do some module uninitialize
	//
	RTFProcess::uninitRTFLib();

	if (TargetFactoryI::instance())
	{
		TargetFactoryI::destroyInstance();
	}

	if (RTFRDSSess::_pFileIoFac.get())
	{
		RTFRDSSess::_pFileIoFac->uninitialize();
		RTFRDSSess::_pFileIoFac.reset(0);
	}

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


RTFRDSSess::~RTFRDSSess()
{
	cleanup();
}

void RTFRDSSess::cleanup()
{
	if (_bCleaned)
		return;

	BaseGraph::Close();
	_bCleaned = true;
}


bool RTFRDSSess::preLoad()
{
	if (!_sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "provision session is 0"));
		setErrorInfo(ERRCODE_NULL_SESSION, "provision session is 0");			
		return false;
	}

	_strMethod = _sess->methodType;
	if(stricmp(_strMethod.c_str(), METHODTYPE_RTFRDSVSVSTRM) 
		&& stricmp(_strMethod.c_str(), METHODTYPE_RTFRDSH264VSVSTRM)
		&& stricmp(_strMethod.c_str(),METHODTYPE_NTFSRTFVSVSTRM)
		&& stricmp(_strMethod.c_str(),METHODTYPE_FTPRTFVSVSTRM)
		&& stricmp(_strMethod.c_str(),METHODTYPE_FTPRTFH264VSVSTRM)
		&& stricmp(_strMethod.c_str(),METHODTYPE_NTFSRTFH264VSVSTRM)
		&& stricmp(_strMethod.c_str(),METHODTYPE_FTPPropagation))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] unsupported method %s"), _sess->ident.name.c_str(),
			_strMethod.c_str());
		setErrorInfo(ERRCODE_UNSUPPORT_METHOD, "unsupported method");			
		return false;
	}

	bool bAudioFlag = false;
	std::string strFilename;
	std::string strSrcUrl;
	if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] could not find resource URI"), _sess->ident.name.c_str());
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find resource URI");			
		return false;
	}

	TianShanIce::ValueMap& resURI = _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource " CPHPM_FILENAME);			
		return false;
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource: " CPHPM_FILENAME);			
		return false;
	}

	strFilename = var.strs[0];

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
			bAudioFlag = (bool)atoi(var3.strs[0].c_str());
		}
	}

	TianShanIce::ValueMap& resBw = _sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] could not find ProvisionBandwidth resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}
    int nBandwidth = (int)var1.lints[0];
    _nBandwidth = nBandwidth;

	TianShanIce::Properties prop = _sess->props;
	TianShanIce::Properties::const_iterator it = prop.find(CPHPM_INDEXTYPE);

	int augmentationPidCount = 0;
	uint16 augmentationPids[ MAX_AUGMENTATION_PIDS ];
	memset((void*) augmentationPids, 0 , sizeof(augmentationPids));
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
		::TianShanIce::StrValues  strAugmentationPids;
		::ZQ::common::stringHelper::SplitString(strPIDs, strAugmentationPids, ";,");
		unsigned int i;
		for(i = 0; i < strAugmentationPids.size() && i < MAX_AUGMENTATION_PIDS; ++i)
		{
			augmentationPids[i] = atoi(strAugmentationPids[i].c_str());
		}
		augmentationPidCount = strAugmentationPids.size();
	}

	bool enableNoTrickSpeed = false;

	it = prop.find(CPHPM_NOTRICKSPEEDS);
	if (it != prop.end())
	{
		int major = 0, minor = 0;
		RTFProcess::getCTFLibVersion(major, minor);
		if(major < 3)
		{
			MOLOG(Log::L_WARNING, CLOGFMT(CPH_RTFRDS, "[%s] CTFLib Verion support No TrickSpeed File must >= 3.0 "), _sess->ident.name.c_str());
		}
		enableNoTrickSpeed = true;
	}
	std::list<float> trickspeed;

	::TianShanIce::ContentProvision::TrickSpeedCollection trickcol = _sess->trickSpeeds;
	if (trickcol.size() == 0)
		trickcol.push_back(7.5);
	for (::TianShanIce::ContentProvision::TrickSpeedCollection::iterator iterTrick = trickcol.begin();iterTrick != trickcol.end();iterTrick++)
	{
		trickspeed.push_back((*iterTrick));
	}
    
	SetLog(_helper.getLog());
	SetMemAlloc(_helper.getMemoryAllocator());
	SetLogHint(strFilename.c_str());

	int nMaxBandwidth;
	if (_gCPHCfg.bandwidthLimitRate)
	{
		nMaxBandwidth = int(((float)nBandwidth) * _gCPHCfg.bandwidthLimitRate / 100);
	}
	else
		nMaxBandwidth = nBandwidth;
	
	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_RTFRDS, "[%s] bandwidth[%d], max_limit_rate[%d], maxbandwidth[%d]"), _strLogHint.c_str(), 
		nBandwidth, _gCPHCfg.bandwidthLimitRate, nMaxBandwidth);

	bool bH264Type = false;
	_bPushTrigger = false;
	if (!stricmp(_strMethod.c_str(), METHODTYPE_FTPPropagation))
	{
		std::string url = strSrcUrl;
		std::string strPath = _gCPHCfg.szNTFSOutputDir;
		if(!( strPath[strPath.length()-1]=='\\' || strPath[strPath.length()-1]=='/'))
			strPath+="\\";
		std::string strFile = strPath + strFilename;

		FTPFilesetSource* ftpsource = (FTPFilesetSource*)SourceFactory::Create(SOURCE_TYPE_FTPFileset);
		AddFilter(ftpsource);
		ftpsource->setURL(url.c_str());
		ftpsource->setFilename(strFilename.c_str());
		ftpsource->setMaxBandwidth(nMaxBandwidth);
		ftpsource->setCacheDir(_gCPHCfg.szCacheDir);
		ftpsource->setTargetDir(strPath.c_str());
		ftpsource->setMode(_gCPHCfg.enableTestNTFS);
		ftpsource->enableMD5(_gCPHCfg.enableMD5);
		ftpsource->setIOFactory(_pFileIoFac.get());
		ftpsource->setDecodeSourceURL(_gCPHCfg.decodeSourceURL);
	}
	else
	{
		BaseSource *pSource= NULL;
		if(!stricmp(_strMethod.c_str(), METHODTYPE_RTFRDSVSVSTRM)||(bH264Type=!stricmp(_strMethod.c_str(),METHODTYPE_RTFRDSH264VSVSTRM)))
		{
			_bPushTrigger = true;
			PushSource* pushSource = (PushSource*)SourceFactory::Create(SOURCE_TYPE_PUSHSRC, &_helper._pool);
			AddFilter(pushSource);		//only after this, the log handle will be parsed in
			pushSource->setMaxBandwidth(nMaxBandwidth);
			pSource = pushSource;
		}
		else if(!stricmp(_strMethod.c_str(),METHODTYPE_NTFSRTFVSVSTRM)|| (bH264Type=!stricmp(_strMethod.c_str(),METHODTYPE_NTFSRTFH264VSVSTRM)))
		{
			char utf8[]="utf-8";
			std::string srcFilename = strSrcUrl;
			bool enableUtfFlag = false;
			char target[MAX_PATH];

			if (URLStr::decode(srcFilename.c_str(),(void*)target, MAX_PATH))
				srcFilename = target;
			else
			{
				MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] Failed to decode url %s"), _strLogHint.c_str(),srcFilename.c_str());
				return false;
			}
			
			if (stricmp(_gCPHCfg.szUrlEncode,utf8)== 0)
			{
				enableUtfFlag = true;
			}

			url2UNC(srcFilename);

			NTFSIOSource* ntfsSource = (NTFSIOSource*)SourceFactory::Create(SOURCE_TYPE_NTFSSRC, &_helper._pool);
			AddFilter(ntfsSource);    //only after this, the log handle will be parsed in
			ntfsSource->setMaxBandwidth(nMaxBandwidth);	
			ntfsSource->setFileName(srcFilename.c_str());
			ntfsSource->setUtfFlag(enableUtfFlag);
			pSource = ntfsSource;
		}
		else if(!stricmp(_strMethod.c_str(),METHODTYPE_FTPRTFVSVSTRM)|| (bH264Type=!stricmp(_strMethod.c_str(),METHODTYPE_FTPRTFH264VSVSTRM)))
		{
			std::string url = strSrcUrl;

			FTPIOSource* ftpsource = (FTPIOSource*)SourceFactory::Create(SOURCE_TYPE_FTP, &_helper._pool);
			AddFilter(ftpsource);    //only after this, the log handle will be parsed in
			ftpsource->setLocalNetworkInterface(_gCPHCfg.szLocalNetIf);
			ftpsource->setURL(url.c_str());
			ftpsource->setMaxBandwidth(nMaxBandwidth);
			ftpsource->setConnectionMode(_gCPHCfg.enableFtpPassiveMode);
			ftpsource->setConnectionInterval(_gCPHCfg.ftpConnectionInterval);
			ftpsource->setDecodeSourceURL(_gCPHCfg.decodeSourceURL);
			pSource = ftpsource;
		}	

		std::map<std::string, int> exMap;
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

		if(enableNoTrickSpeed)
			outPutNum = 2;
		
		if ( !enableNoTrickSpeed && outPutNum < 2 && !bAudioFlag)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] Not specify trick speed"), _strLogHint.c_str());
			return false;
		}

		RTFProcess* pProcess = (RTFProcess*)ProcessFactory::Create(PROCESS_TYPE_RTF, &_helper._pool);
		AddFilter(pProcess);
		if(!enableNoTrickSpeed)
		{
			pProcess->setTrickFile(exMap);
			pProcess->settrickSpeedNumerator(trickspeed);
		}

		pProcess->setUnifiedTrickFile(_gCPHCfg.unifiedtrickfile.enable);
		if (bAudioFlag)
			pProcess->setAudioOnly(_gCPHCfg.enableLegacyAudioOnly);
		if (bH264Type)
		{
			pProcess->setTrickGenParam(CTF_INDEX_TYPE_VV2, CTF_VIDEO_CODEC_TYPE_H264);
		}
		if (!stricmp(_strMethod.c_str(), METHODTYPE_FTPRTFVSVSTRM) || !stricmp(_strMethod.c_str(), METHODTYPE_NTFSRTFVSVSTRM))
		{
			pProcess->setAugmentationPids(augmentationPids ,augmentationPidCount);
		}

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
			pTarget->setBandwidth(nMaxBandwidth);
			pTarget->enableProgressEvent(true);
			pTarget->enableMD5(_gCPHCfg.enableMD5);
			if(!enableNoTrickSpeed)
			{
				pTarget->setTrickFile(exMap);
				pTarget->setTrickSpeed(trickspeed);
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

			_pMainTarget = pTarget;
			
			InitPins();

			if (!ConnectTo(pSource, 0, pProcess, 0))
				return false;

			for (int i = 0; i < outPutNum; i++)
				if (!ConnectTo(pProcess, i, pTarget, i))
					return false;
		}
	}

	SetMediaSampleSize(_gCPHCfg.mediaSampleSize);

	if (!Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		setErrorInfo(_nLastErrCode, (std::string("Failed to initialize graph with error: ") + _strLastErr).c_str());			
		return false;
	}

	BaseCPHSession::preLoad();

	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] preload successful"), _strLogHint.c_str());
	return true;
}

void RTFRDSSess::terminate(bool bProvisionSuccess)
{
	// not started, just constructed
	if (getStatus() == NativeThread::stDeferred)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] to terminate() session while in init status"), _strLogHint.c_str());
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
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] to terminate() session with status[%s]"), _strLogHint.c_str(),
			bProvisionSuccess?"success":"failure");
		_bQuit = true;

		if (!bProvisionSuccess && !BaseGraph::IsErrorOccurred())
		{
			BaseGraph::SetLastError("User canceled provision", ERRCODE_USER_CANCELED);
		}

		BaseGraph::Stop();
	}
}

bool RTFRDSSess::getProgress(::Ice::Long& offset, ::Ice::Long& total)
{	
	total = _filesize;
	offset = getProcessBytes();
	return true;
} 

int RTFRDSSess::run(void)
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

		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] provision done, status[failure], error[%s]"), 
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

		if (!stricmp(_strMethod.c_str(), METHODTYPE_FTPPropagation))
		{
			FTPFilesetSource* pSource = (FTPFilesetSource*)getSourceFilter();
			pSource->getSupportFileSize(supportFileSize);
			pSource->getMD5(md5);
			pSource->getOrigFileSize(_llTotalBytes);
		}
	}

	char tmp[64];
	if (!_llTotalBytes)
	{
		_llTotalBytes = _llProcBytes;
	}
	sprintf(tmp, "%lld", _llTotalBytes);
	params[EVTPM_TOTOALSIZE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] Filesize[%s]"), _strLogHint.c_str(), tmp);

	sprintf(tmp, "%lld", supportFileSize);
	params[EVTPM_SUPPORTFILESIZE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] SupportFilesize[%s]"), _strLogHint.c_str(), tmp);

	params[EVTPM_MD5CHECKSUM] = md5;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] md5[%s]"), _strLogHint.c_str(), md5.c_str());

	if (_bitrate)
	{
		if (!stricmp(_strMethod.c_str(), METHODTYPE_FTPPropagation))
			sprintf(tmp, "%lld", _llTotalBytes*8000/_bitrate);
		else
		sprintf(tmp, "%lld", _llProcBytes*8000/_bitrate);
		params[EVTPM_PLAYTIME] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] playtime[%s]"), _strLogHint.c_str(), tmp);
	}
	else 
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] bitrate [0] , can't get playtime"), _strLogHint.c_str());
	}
	if (_pRTFProc  && (!stricmp(_strMethod.c_str(), METHODTYPE_FTPRTFVSVSTRM) || !stricmp(_strMethod.c_str(), METHODTYPE_NTFSRTFVSVSTRM)))
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
			MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] AugmentationPids[%s]"), _strLogHint.c_str(),augmentionpids.c_str());

			pRtfprocess->getPreEncryptBitRate(augmentedBitRate, originalBitRate);

			sprintf(tmp, "%d", originalBitRate);
			params[EVTPM_ORIGINALBITRATE] = tmp;
			MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] OrignalBiterate[%s]"), _strLogHint.c_str(),tmp);

			sprintf(tmp, "%d", augmentedBitRate);
			params[EVTPM_AUGMENTATEDBITRATE] = tmp;
			MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] AugmentatedBitarate[%s]"), _strLogHint.c_str(),tmp);

			sprintf(tmp, "1");
			params[EVTPM_PREENCRYPTION] = tmp;
			MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] PreEncryption[%s]"), _strLogHint.c_str(),tmp);
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
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] provision done, status[success], spent[%d]ms, actualspeed[%d]bps"), 
		_strLogHint.c_str(), nTimeSpentMs, nActualBps);
	return 0;
}

void RTFRDSSess::final(int retcode, bool bCancelled)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] final() ret=%d, cancelled=%c; calling doCleanup()"), 
		_strLogHint.c_str(),retcode, bCancelled?'Y':'N');
	
	cleanup();
	try
	{
		delete this;
	}catch(...)
	{
	}
}


bool RTFRDSHelper::validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource)
{
	if (!sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "provision session is 0"));
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTFRDS, 0, "provision session is 0");
	}
	
	std::string strMethod = sess->methodType;
	if(stricmp(strMethod.c_str(), METHODTYPE_RTFRDSVSVSTRM) 
		&& stricmp(strMethod.c_str(), METHODTYPE_RTFRDSH264VSVSTRM)
		&& stricmp(strMethod.c_str(),METHODTYPE_NTFSRTFVSVSTRM)
		&& stricmp(strMethod.c_str(),METHODTYPE_FTPRTFVSVSTRM)
		&& stricmp(strMethod.c_str(),METHODTYPE_FTPRTFH264VSVSTRM)
		&& stricmp(strMethod.c_str(),METHODTYPE_NTFSRTFH264VSVSTRM)
		&& stricmp(strMethod.c_str(),METHODTYPE_FTPPropagation))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] unsupported method %s"), sess->ident.name.c_str(),
			strMethod.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTFRDS, 0, "unsupported method %s", strMethod.c_str());
	}
	
	if (sess->resources.end() == sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] could not find resource URI"), sess->ident.name.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTFRDS, 0, "could not find resource URI");
	}

	TianShanIce::ValueMap& resURI = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTFRDS, 0, "could not find URI resource " CPHPM_FILENAME);
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTFRDS, 0, "could not find URI resource " CPHPM_FILENAME);
	}

	TianShanIce::ValueMap& resBw = sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTFRDS, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTFRDS, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}

	if(stricmp(strMethod.c_str(), METHODTYPE_RTFRDSVSVSTRM)==0 || stricmp(strMethod.c_str(), METHODTYPE_RTFRDSH264VSVSTRM) == 0)
	{
		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stPushTrigger);
	}
	else if(stricmp(strMethod.c_str(),METHODTYPE_NTFSRTFVSVSTRM)==0
		|| stricmp(strMethod.c_str(),METHODTYPE_NTFSRTFH264VSVSTRM)==0)
	{	
		TianShanIce::ValueMap& resSrc = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
		if (resSrc.end() == resSrc.find(CPHPM_SOURCEURL))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTFRDS, 0, "could not find URI resource " CPHPM_SOURCEURL);
		}

		TianShanIce::Variant& var2 = resSrc[CPHPM_SOURCEURL];
		if (var2.type != TianShanIce::vtStrings || var2.bRange || var2.strs.size() <=0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTFRDS, 0, "could not find URI resource " CPHPM_SOURCEURL);
		}

		std::string sourceUrl = var2.strs[0];
		std::string srcFilename;
		char target[MAX_PATH];

		if (URLStr::decode(sourceUrl.c_str(),(void*)target, MAX_PATH))
			srcFilename = target;
		else
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] Failed to decode url[%s]"), sess->ident.name.c_str(), sourceUrl.c_str());
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
				MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] sourceurl[%s], could not open file [%S] because of \"%s\""), sess->ident.name.c_str(), 
					srcFilename.c_str(), unifilename.c_str(),errmsg.c_str());
				ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTFRDS, TianShanIce::Storage::csexpInvalidSourceURL, "could not open file %S ", unifilename.c_str());
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
				MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] could not open file %s because of \"%s\""), sess->ident.name.c_str(), 
					srcFilename.c_str(),errmsg.c_str());
				ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTFRDS, TianShanIce::Storage::csexpInvalidSourceURL, "could not open file %s ", srcFilename.c_str());
			}
			CloseHandle(hFile);
		}

		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	}
	else if(stricmp(strMethod.c_str(),METHODTYPE_FTPRTFVSVSTRM)==0 
		|| stricmp(strMethod.c_str(),METHODTYPE_FTPRTFH264VSVSTRM)==0
		|| stricmp(strMethod.c_str(),METHODTYPE_FTPPropagation)==0
		)
	{
		TianShanIce::ValueMap& resSrc = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
		if (resSrc.end() == resSrc.find(CPHPM_SOURCEURL))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTFRDS, 0, "could not find URI resource " CPHPM_SOURCEURL);
		}

		TianShanIce::Variant& var2 = resSrc[CPHPM_SOURCEURL];
		if (var2.type != TianShanIce::vtStrings || var2.bRange || var2.strs.size() <=0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTFRDS, 0, "could not find URI resource " CPHPM_SOURCEURL);
		}

		std::string url = var2.strs[0];
		std::string strpro = url.substr(0,6);
		if (_stricmp(strpro.c_str(),"ftp://")!= 0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] Can't find the FTP protocol from url %s"), sess->ident.name.c_str(), url.c_str());
			ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTFRDS, TianShanIce::Storage::csexpInvalidSourceURL, "Can't find the FTP protocol from url" ,url.c_str());
		}

		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	}
	else
	{
		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
		MOLOG(Log::L_WARNING, CLOGFMT(CPH_RTFRDS, "[%s] Unknown method[%s], setSessionType() with default type"), sess->ident.name.c_str(), strMethod.c_str());
	}

	return true;
}

bool RTFRDSSess::prime()
{
	if (!BaseCPHSession::prime())
		return false;

 	if(_bPushTrigger)
	{
		if (!_pPushSource)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTFRDS, "[%s] can not find Push Interface"), _strLogHint.c_str());
			setErrorInfo(ERRCODE_PUSHSOURCE_MISSING, ("Provisioning failed with error: can not find Push Interface"));			
			return false;
		}

		PushSource* pSource = (PushSource*)getSourceFilter();
		if (!pSource)
		{
			//should not go here
			setErrorInfo(ERRCODE_INVALID_SRCURL, ("Source file is invalid."));			
			return false;
		}

		pSource->setPushSrcI(_pPushSource);
	}

	if (!Start())
	{
		setErrorInfo(_nLastErrCode, (std::string("Failed to start graph with error: ") + _strLastErr).c_str());			
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] failed to start graph withe error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}

	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] prime() successful"), _strLogHint.c_str());
	return true;
}

void RTFRDSSess::OnProgress(LONGLONG& prcvBytes)
{
	BaseGraph::OnProgress(prcvBytes);
	::TianShanIce::Properties params;
	updateProgress(_llProcBytes, _llTotalBytes,params);
}

void RTFRDSSess::OnStreamable(bool bStreamable)
{
	BaseGraph::OnStreamable(bStreamable);
	notifyStreamable(bStreamable);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void RTFRDSSess::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());

	char tmp[40];
	::TianShanIce::Properties params;
	sprintf(tmp, "%d", mInfo.bitrate);
	params[EVTPM_MPEGBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] bitrate[%s]"), _strLogHint.c_str(),tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionV);
	params[EVTPM_VIDEOHEIGHT] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] videoResolutionV[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionH);
	params[EVTPM_VIDEOWIDTH] =  tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] videoResolutionH[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoBitrate);
	params[EVTPM_VIDEOBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] videoBitrate[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp,"%.2f",mInfo.framerate);
	params[EVTPM_FRAMERATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] framerate[%s]"), _strLogHint.c_str(),tmp);

	_bitrate = mInfo.bitrate;
	
	notifyStarted(params);
	_bStartEventSent = true;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTFRDS, "[%s] notifyStarted() called"), _strLogHint.c_str());

	if (_pMainTarget)
	{
		_pMainTarget->setStreamableBytes(mInfo.bitrate*_gCPHCfg.streamReqSecs/8);
	}
}

RTFRDSHelper::RTFRDSHelper(ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr)
:BaseCPHelper(pool, mgr) 
{
	CPHRTFRDSCfg::Methods::iterator it;
	for (it = _gCPHCfg.methods.begin();it != _gCPHCfg.methods.end();it++)
	{
		if (!it->maxSession || !it->maxBandwidth)
			continue;

		_methodCostList[it->methodName] = new MethodCostI(it->maxBandwidth, it->maxSession);				
	}			
}

RTFRDSHelper::~RTFRDSHelper()
{
	MethodCostList::iterator it = _methodCostList.begin();
	for (;it!=_methodCostList.end();it++)
	{
		if (it->second)
			delete it->second;
	}

	_methodCostList.clear();
}

bool RTFRDSHelper::getBandwidthLoad(const char* methodType, long& bpsAllocated, long& bpsMax, long& initCost)
{
	if (stricmp(methodType, METHODTYPE_RTFRDSVSVSTRM) && 
		stricmp(methodType, METHODTYPE_RTFRDSH264VSVSTRM)&&
		stricmp(methodType, METHODTYPE_NTFSRTFVSVSTRM)&& 
		stricmp(methodType, METHODTYPE_FTPRTFVSVSTRM)&&
		stricmp(methodType, METHODTYPE_FTPRTFH264VSVSTRM)&& 
		stricmp(methodType, METHODTYPE_FTPPropagation)&&
		stricmp(methodType, METHODTYPE_NTFSRTFH264VSVSTRM))
		return false;
	
	bpsAllocated =0;

	CPHRTFRDSCfg::Methods::iterator methodIter;
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
bool RTFRDSHelper:: getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins)
{
	CPHRTFRDSCfg::Methods::iterator methodIter;
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
