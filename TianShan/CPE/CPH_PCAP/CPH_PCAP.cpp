
#include "BaseClass.h"
#include "CPH_PCAP.h"
#include "BaseClass.h"
#include "CPH_PCAPCfg.h"
#include "RTFProc.h"
#include "McastCapSrc.h"
#include "ErrorCode.h"
#include "HostToIP.h"
#include <list>
#include <math.h>
#include "TargetFac.h"
#include "TargetFactoryI.h"
#include "CStdFileIoFactory.h"
#include "WPCapThread.h"
#include "SystemUtils.h"
#include "Guid.h"
#ifdef ZQ_OS_LINUX
#include "CDNFileSetTarget.h"
#else
#include "FilesetTarget.h"
#endif

using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define CPH_PCAP			"CPH_PCAP"
#define MOLOG					(glog)

NetworkIFSelector*                                      _nNetSelector = NULL;
std::auto_ptr<ZQTianShan::ContentProvision::FileIoFactory> PCAPSess::_pFileIoFac;
#ifdef ZQ_OS_LINUX
PacedIndexFactory*       PCAPSess::_pPacedIndexFac=NULL;
void*   PCAPSess::_pPacedIndexDll=NULL;
#endif

ZQTianShan::ContentProvision::BaseCPHelper* PCAPHelper::_theHelper =NULL;

extern "C" __EXPORT bool InitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
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
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFilePath().c_str());	
		return FALSE;		
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "Load configuration from [%s] successfully"),
			_gCPHCfg.getConfigFilePath().c_str());
		
		_gCPHCfg.snmpRegister("");
	}	

	//
	// do some module initialize
	//
#ifdef ZQ_OS_MSWIN
	std::string errmsg;
	if (_gCPHCfg.enableTestNTFS)
	{
		CStdFileIoFactory* pFactory = new CStdFileIoFactory();
		//pFactory->setRootDir(_gCPHCfg.szNTFSOutputDir);
		PCAPSess::_pFileIoFac.reset(pFactory);
	}
#else
	CStdFileIoFactory* pFactory = new CStdFileIoFactory();
	pFactory->setDirectIO(_gCPHCfg.directIO.enable);
	pFactory->setFileSync(_gCPHCfg.filesync.maxSyncTimeout,_gCPHCfg.filesync.bytesToSync);
	PCAPSess::_pFileIoFac.reset(pFactory);
#endif

	PCAPSess::_pFileIoFac->setLog(&glog);
	if (!PCAPSess::_pFileIoFac->initialize())
	{
		std::string strErr;
		int nErrCode;
		PCAPSess::_pFileIoFac->getLastError(strErr, nErrCode);
		printf("Failed to initialize fileio factory with error: %s, code: %d\n", strErr.c_str(), nErrCode);
		return false;
	}

	TargetFac * pTargetFac = new TargetFac(PCAPSess::_pFileIoFac.get());
	TargetFactoryI::setInstance(pTargetFac);	

	RTFProcess::initRTFLib(_gCPHCfg.rtfMaxSessionNum, &glog, _gCPHCfg.rtfMaxInputBufferBytes,
		_gCPHCfg.rtfMaxInputBuffersPerSession, _gCPHCfg.rtfSessionFailThreshold);
	
	if (!PCAPHelper::_theHelper)
		PCAPHelper::_theHelper = new PCAPHelper(pEngine->getThreadPool(), pEngine);

	_nNetSelector = new NetworkIFSelector(glog);
	if(!_nNetSelector)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "Failed to new NetIfSelector object."));
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
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "failed to load config of NetworkInterface for Capture, please check CPH_PCAP.xml"));
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

		for(size_t i=0;i<_gCPHCfg.nInterface.size();i++)
		{
			std::string strLocalIp;
			if (!HostToIP::translateHostIP(_gCPHCfg.nInterface[i].strIp.c_str(), strLocalIp))//translate host name to ip
				strLocalIp = _gCPHCfg.nInterface[i].strIp;

			pCaptureInterface->addNIC(strLocalIp, _gCPHCfg.nInterface[i].totalBandwidth);
		}

		if (!pCaptureInterface->init())
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "Failed to initialize capture interface with error: %s"),
				pCaptureInterface->getLastError().c_str());
			return false;
		}
	}

	for (std::vector< ZQ::common::Config::Holder< Method > >::iterator iter = _gCPHCfg.methods.begin();iter != _gCPHCfg.methods.end();iter++)
	{
		if (iter->maxBandwidth && iter->maxSession)
		{
			pEngine->registerHelper((*iter).methodName.c_str(), PCAPHelper::_theHelper, pCtx);
			pEngine->registerLimitation((*iter).methodName.c_str(), (*iter).maxSession,(*iter).maxBandwidth, GROUP_PCAP, 0);
			MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "%s Helper registered"),(*iter).methodName.c_str());
		}
		else
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "%s Helper disabled"),(*iter).methodName.c_str());
		}
	}

#ifdef ZQ_OS_LINUX
	PCAPSess::_pPacedIndexDll = dlopen(_gCPHCfg.szPaceDllPath,RTLD_LAZY);
	if (!PCAPSess::_pPacedIndexDll)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "%s failed to load"),_gCPHCfg.szPaceDllPath);
		return false;
	}
	typedef bool (*FunCreatePacedIndexFactory)(PacedIndexFactory**);
	FunCreatePacedIndexFactory _create;
	_create = (FunCreatePacedIndexFactory)dlsym(PCAPSess::_pPacedIndexDll,"CreatePacedIndexFactory");
	if (_create != NULL)
	{
		_create(&PCAPSess::_pPacedIndexFac);
		if (!PCAPSess::_pPacedIndexFac)
		{
			dlclose(PCAPSess::_pPacedIndexDll);
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "failed to create pacingFactory"));
			return false;
		}
		PCAPSess::_pPacedIndexFac->setLog(&glog);
		//PCAPSess::_pPacedIndexFac->setConfig("name","value");
		MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "Successfully load %s"),_gCPHCfg.szPaceDllPath);
	}
	else
	{
		std::string strErr =  SYS::getErrorMessage(SYS::RTLD);
		dlclose(PCAPSess::_pPacedIndexDll);
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "failed to get CreatePacedIndexFactory entry error[%s]"), strErr.c_str());
		return false;
	}
#endif

	return TRUE;
}

extern "C" __EXPORT void UninitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return;
	
	for (std::vector< ZQ::common::Config::Holder< Method > >::iterator iter = _gCPHCfg.methods.begin();iter != _gCPHCfg.methods.end();iter++)
	{
		if ((*iter).enableFlag == 1)
		{
			pEngine->unregisterHelper((*iter).methodName.c_str(), pCtx);
			MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "%s Helper unregistered"),(*iter).methodName.c_str());
		}
	}

	if (PCAPHelper::_theHelper)
	{
		try
		{
			delete PCAPHelper::_theHelper;
		}
		catch(...){};
		
		PCAPHelper::_theHelper = NULL;
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

	RTFProcess::uninitRTFLib();

	if (TargetFactoryI::instance())
	{
		TargetFactoryI::destroyInstance();
	}

	if (PCAPSess::_pFileIoFac.get())
	{
		PCAPSess::_pFileIoFac->uninitialize();
		PCAPSess::_pFileIoFac.reset(0);
	}

#ifdef ZQ_OS_LINUX
	typedef bool (*DestroyPacedIndexFactory)(PacedIndexFactory*);
	DestroyPacedIndexFactory _pdestry = (DestroyPacedIndexFactory)dlsym(PCAPSess::_pPacedIndexDll,"DestroyPacedIndexFactory");
	if (_pdestry)
	{
		_pdestry(PCAPSess::_pPacedIndexFac);
	}
	else
	{
		std::string strErr =  SYS::getErrorMessage(SYS::RTLD);
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "failed to get DestroyPacedIndexFactory entry with error[%s]"),strErr.c_str());
	}
	if (PCAPSess::_pPacedIndexDll)
	{
		dlclose(PCAPSess::_pPacedIndexDll);
		MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "Successfully unload %s"),_gCPHCfg.szPaceDllPath);
	}
#endif

//	ZQ::common::setGlogger(NULL);
}

///////////////
/*
#include "urlstr.h"

static bool fixpath(std::string& path, bool bIsLocal = true)
{
	char* pathbuf = new char[path.length() +2];
	if (NULL ==pathbuf)
		return false;

	strcpy(pathbuf, path.c_str());
	pathbuf[path.length()] = '\0';

	for (char* p = pathbuf; *p; p++)
	{
		if ('\\' == *p || '/' == *p)
			*p = FNSEPC;
	}

	if (!bIsLocal && ':' == pathbuf[1])
		pathbuf[1] = '$';
	else if (bIsLocal && '$' == pathbuf[1])
		pathbuf[1] = ':';

	static const char* szProtocol[] = {
		"file:",
		"cifs:",
		"nfs:"
	};

	static int nProto = sizeof(szProtocol)/sizeof(const char*);

	int i;

	for(i=0;i<nProto;i++)
	{
#ifdef ZQ_OS_MSWIN
		if (!strnicmp(pathbuf, szProtocol[i], strlen(szProtocol[i])))
#else
		if (!strncasecmp(pathbuf, szProtocol[i], strlen(szProtocol[i])))
#endif
		{
			path = pathbuf + strlen(szProtocol[i]);
			break;
		}
	}
	if (i>=nProto)
	{
		//not found
		path = pathbuf;
	}
	return true;    	
}
*/

/*
static unsigned long timeval()
{
	unsigned long rettime = 1;

	int64 ntime = ZQTianShan::now();
	ntime &= 0xFFFFFFFF;//keep only the low part
	rettime = (unsigned long)ntime;
	return rettime;
}
*/

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

std::string replaceString(const std::string& inStr)
{
	std::string str = inStr;
	size_t pos = str.find('.');
	while ( std::string::npos != pos)
	{
		str[pos] = '#';
		pos = str.find('.');
	}
	return str;
}

std::string formatSpeed(std::list<float> trickspeeds)
{
	std::string strResult = "";
	for(std::list<float>::iterator itorSpeed = trickspeeds.begin();itorSpeed != trickspeeds.end(); itorSpeed++)
	{
		char buf[128]= "";
		sprintf(buf, "%.2f,", *itorSpeed);
		strResult += buf;
	}
	return strResult;
}
std::string formatSpeed(std::vector<float> trickspeeds)
{
	std::string strResult = "";
	for(std::vector<float>::iterator itorSpeed = trickspeeds.begin();itorSpeed != trickspeeds.end(); itorSpeed++)
	{
		char buf[128]= "";
		sprintf(buf, "%.2f,", *itorSpeed);
		strResult += buf;
	}
	return strResult;
}
PCAPSess::~PCAPSess()
{
	_nNetSelector->freeInterface(_strFileName);
	cleanup();
}

void PCAPSess::cleanup()
{
	if (_bCleaned)
		return;

	BaseGraph::Close();
	_bCleaned = true;
}


bool PCAPSess::preLoad()
{
	 std::string multicastIp;

	if (!_sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "provision session is 0"));
		setErrorInfo(ERRCODE_NULL_SESSION, "provision session is 0");			
		return false;
	}

	std::string strMethod = _sess->methodType;
	_strMethod = strMethod;
	if(stricmp(strMethod.c_str(), METHODTYPE_RTIVSVSTRM) && 
		stricmp(strMethod.c_str(), METHODTYPE_RTIH264VSVSTRM))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] unsupported method %s"), _sess->ident.name.c_str(),
			strMethod.c_str());
		setErrorInfo(ERRCODE_UNSUPPORT_METHOD, "unsupported method");			
		return false;
	}

	if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] could not find resource URI"), _sess->ident.name.c_str());
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find resource URI");			
		return false;
	}

	TianShanIce::ValueMap& resURI = _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource " CPHPM_FILENAME);			
		return false;
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource: " CPHPM_FILENAME);			
		return false;
	}

	std::string strFilename = var.strs[0];
	_strFileName = strFilename;

	TianShanIce::ValueMap& resBw = _sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] could not find ProvisionBandwidth resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}


	int nBandwidth = (int)var1.lints[0];
	_nBandwidth = nBandwidth;

	TianShanIce::ValueMap& resMI= _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resMI.end() == resMI.find(CPHPM_SOURCEURL))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] could not find multicast resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_SOURCEURL);			
		return false;
	}
	
	TianShanIce::Variant& var2 = resMI[CPHPM_SOURCEURL];
	if (var2.type != TianShanIce::vtStrings || var2.bRange|| var2.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_SOURCEURL);			
		return false;
	}
	std::string multicastUrl = var2.strs[0];

	multicastIp = multicastUrl.substr(multicastUrl.find_first_of(':')+3,multicastUrl.find_last_of(':')-6);
	std::string strmulticastPort = multicastUrl.substr(multicastUrl.find_last_of(':')+1,multicastUrl.size()-multicastUrl.find_last_of(':')-1);
	int multicastPort = atoi(strmulticastPort.c_str());
	
	std::list<float> trickspeed;
	std::list<float> trickspeedHD;

	::TianShanIce::ContentProvision::TrickSpeedCollection trickcol = _sess->trickSpeeds;

	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_PCAP, "trickSpeed from session [%s]"), formatSpeed(trickcol).c_str());

	if (trickcol.size() == 0 || (trickcol.size() == 1 && fabs(trickcol[0]) <=1e-6 )) //add compare folat 
		trickcol.push_back(7.5);

	bool bFoundHD = false;
	for (::TianShanIce::ContentProvision::TrickSpeedCollection::iterator iterTrick = trickcol.begin();iterTrick != trickcol.end();iterTrick++)
	{
		if ((*iterTrick) > 60)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] can't set trickspeed larger than 60."), _sess->ident.name.c_str());
			return false;
		}
		else if(fabs(*iterTrick) <=1e-6)
		{
			bFoundHD = true;
			continue;
		}
		if(bFoundHD)
		{
			trickspeedHD.push_back((*iterTrick));
		}
		else
			trickspeed.push_back((*iterTrick));		
	}

	if(trickspeed.empty() && trickspeedHD.size() > 0)
	{
		trickspeed.assign(trickspeedHD.begin(), trickspeedHD.end());
	}
	if(!trickspeed.empty() && !trickspeedHD.empty() && trickspeed.size() != trickspeedHD.size())
	{
		trickspeed.sort();
		trickspeedHD.sort();

		if(trickspeed.size() < trickspeedHD.size())
		{
			int npos = trickspeedHD.size() - trickspeed.size();
			std::list<float>::iterator itorSpeed =  trickspeedHD.begin();
			for(uint i = 0; i < trickspeedHD.size() - npos ; i++)
			{
				itorSpeed++;
			}
			for(uint i = 0; i < npos ; i++)
			{
				trickspeed.push_back(*itorSpeed);
				itorSpeed++;
			}
		}
		else
		{
			int npos = trickspeed.size() - trickspeedHD.size();
			std::list<float>::iterator itorSpeed =  trickspeed.begin();
			for(uint i = 0; i < trickspeed.size() - npos ; i++)
			{
				itorSpeed++;
			}
			for(uint i = 0; i < npos ; i++)
			{
				trickspeedHD.push_back(*itorSpeed);
				itorSpeed++;
			}
		}
		trickspeed.sort();
		trickspeedHD.sort();
	}

	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s]SD trickspeed [%s], HD trickSpeed[%s]"),
		_sess->ident.name.c_str(), formatSpeed(trickspeed).c_str(), formatSpeed(trickspeedHD).c_str());


	bool bIndexVVC=false;
	std::string providerId,providerAssetId;
	TianShanIce::Properties prop = _sess->props;
	TianShanIce::Properties::const_iterator it = prop.find(CPHPM_INDEXTYPE);
	if(it != prop.end())
	{
		if(stricmp(it->second.c_str(),"VVC") == 0)
		{
			bIndexVVC = true;
		}
	}

	it = prop.find(CPHPM_PROVIDERID);
		if (it!=prop.end())
			providerId = it->second;

	it = prop.find(CPHPM_PROVIDERASSETID);
		if (it!=prop.end())
			providerAssetId = it->second;
	SetLog(_helper.getLog());
	SetMemAlloc(_helper.getMemoryAllocator());
	SetLogHint(strFilename.c_str());

	int nMaxBandwidth = nBandwidth;
	
	DWORD timeoutInterval = _gCPHCfg.timeoutInterval;
	std::string localIp;
	if (!_nNetSelector->allocInterface(nMaxBandwidth,localIp,strFilename))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] Failed to allocate proper network card"), _strLogHint.c_str());
		return false;
	}
	if (!HostToIP::translateHostIP(localIp.c_str(),_strLocalIp))//translate host name to ip
		_strLocalIp = localIp;
	McastCapSource* pSource = (McastCapSource*)SourceFactory::Create(SOURCE_TYPE_MCASTCAPSRC, &_helper._pool);
	AddFilter(pSource);
	pSource->setInspectPara(multicastIp,multicastPort,timeoutInterval,_strLocalIp);

	//dumper parameters
	pSource->enableDump(_gCPHCfg.enableDump);
	pSource->setDumpPath(_gCPHCfg.szDumpPath);
	pSource->deleteDumpOnSuccess(_gCPHCfg.deleteDumpOnSuccess);

	bool bH264Type = false;
	if (!stricmp(strMethod.c_str(), METHODTYPE_RTIH264VSVSTRM))
	{
		bH264Type = true;
	}

	ZQ::common::MD5ChecksumUtil md5;
	md5.checksum(strFilename.c_str(), strFilename.length());
	std::string strMd5 = md5.lastChecksum();

	unsigned short uFileCiscoExt = atoi(strMd5.substr(14, 2).c_str());
	uFileCiscoExt &= 0xfff0;

	transform (strMd5.begin(), strMd5.end(), strMd5.begin(), (int(*)(int))toupper);
	std::string strFileCiscoExt = strMd5.substr(0, 14);

	char buf[64] = "";

	memset(buf, 0, 64);
	snprintf(buf, sizeof(buf)-2, ".0x%s%02X\0", strFileCiscoExt.c_str(), uFileCiscoExt);
	std::string mainFileExt="";
	
	std::string replacePAID = replaceString(providerAssetId);
	std::string replacePID = replaceString(providerId);
	
	if( 1 == _gCPHCfg.ciscofileext.mode )
	{
		mainFileExt = std::string(buf);
	}
	else if ( 2 == _gCPHCfg.ciscofileext.mode )
	{
		memset(buf, 0, 64);
		snprintf(buf, sizeof(buf) - 2, ".00%s_%s\0", replacePAID.c_str(), replacePID.c_str());
		mainFileExt =  std::string(buf);
	}

	FileExtensions exMap;
	FileExtensions::iterator iter;
	std::string key="";
	std::map<std::string , int> exMapOutPutfile;
	trickspeed.sort();
	int index = 0;

	for (int i = 0; i < (int)trickspeed.size(); i++)
	{
		char ex[10]={0};
		char exr[10] ={0};

		FileExtension fileExt;
		if (bH264Type && _gCPHCfg.unifiedtrickfile.enable)
		{
			memset(buf, 0, 64);
			getUnifiedTrickExt(index,ex);
			key = std::string(ex);
			if( 1 == _gCPHCfg.ciscofileext.mode && bIndexVVC)
			{
				++uFileCiscoExt;
				snprintf(buf, sizeof(buf)-2, ".0x%s%02X\0", strFileCiscoExt.c_str(), uFileCiscoExt);
				exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
				key = std::string(buf);
			}
			else if( 2 == _gCPHCfg.ciscofileext.mode && bIndexVVC )
			{
				memset(buf, 0, 64);
				snprintf(buf, sizeof(buf)-2, ".B%X%s_%s\0", i, replacePAID.c_str(), replacePID.c_str());
				exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
				key = std::string(buf);
			}
			else
			{
				exMapOutPutfile.insert(std::make_pair(std::string(ex), i));
			}
			fileExt.ext = ex;
			fileExt.extForCisco = buf;
			fileExt.position = i;
			exMap.insert(std::make_pair(key, fileExt));
		}
		else
		{
			getTrickExt(index,ex,exr);
			key = std::string(ex);
			memset(buf, 0, 64);
			if( 1 == _gCPHCfg.ciscofileext.mode && bIndexVVC)
			{
				++uFileCiscoExt;
				snprintf(buf, sizeof(buf)-2, ".0x%s%02X\0", strFileCiscoExt.c_str(), uFileCiscoExt);

				exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
				key = std::string(buf);
			}
			else if( 2 == _gCPHCfg.ciscofileext.mode && bIndexVVC )
			{
				memset(buf, 0, 64);
				snprintf(buf, sizeof(buf)-2, ".F%X%s_%s\0", i, replacePAID.c_str(), replacePID.c_str());
				exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
				key = std::string(buf);
			}
			else
				exMapOutPutfile.insert(std::make_pair(std::string(ex), i));
			fileExt.ext = ex;
			fileExt.extForCisco = buf;
			fileExt.position = i;
			exMap.insert(std::make_pair(key, fileExt));


			key = std::string(exr);
			memset(buf, 0, 64);
			if( 1 == _gCPHCfg.ciscofileext.mode && bIndexVVC )
			{
				++uFileCiscoExt;
				snprintf(buf, sizeof(buf)-2, ".0x%s%02X\0", strFileCiscoExt.c_str(), uFileCiscoExt);
				exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
				key = std::string(buf);
			}
			else if ( 2 == _gCPHCfg.ciscofileext.mode && bIndexVVC )
			{
				memset(buf, 0, 64);
				snprintf(buf, sizeof(buf)-2, ".R%X%s_%s\0", i, replacePAID.c_str(), replacePID.c_str());
				exMapOutPutfile.insert(std::make_pair(std::string(buf), i));
				key = std::string(buf);
			}
			else
				exMapOutPutfile.insert(std::make_pair(std::string(exr), i));
			fileExt.ext = exr;
			fileExt.extForCisco = buf;
			fileExt.position = i;
			exMap.insert(std::make_pair(key, fileExt));
		}
		index++;
	}


	int outPutNum = 2 + exMap.size();

	if (outPutNum < 2)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] Not specify trick speed"), _strLogHint.c_str());
		return false;
	}

	RTFProcess* pProcess = (RTFProcess*)ProcessFactory::Create(PROCESS_TYPE_RTF, &_helper._pool);
	AddFilter(pProcess);
	pProcess->setTrickFileEx(exMap);
	pProcess->settrickSpeedNumerator(trickspeed);
	pProcess->settrickSpeedNumeratorHD(trickspeedHD);
	pProcess->setRetryCount(_gCPHCfg.retryCaptureCount);
    pProcess->setUnifiedTrickFile(_gCPHCfg.unifiedtrickfile.enable);
    if (0 != _gCPHCfg.ciscofileext.mode )
		pProcess->setCsicoFileExt(1);
	pProcess->setCsicoMainFileExt(mainFileExt);
	if (bH264Type)
	{
		pProcess->setTrickGenParam(CTF_INDEX_TYPE_VV2, CTF_VIDEO_CODEC_TYPE_H264);
	}
#ifndef ZQ_OS_MSWIN
        else if(bIndexVVC)    
	{
	    pProcess->setTrickGenParam(CTF_INDEX_TYPE_VVC, CTF_VIDEO_CODEC_TYPE_MPEG2);
        }
#else
       else
       {
           pProcess->setTrickGenParam(CTF_INDEX_TYPE_VVX, CTF_VIDEO_CODEC_TYPE_MPEG2);
       }
#endif
	_pRTFProc = pProcess;
	
	{
#ifdef ZQ_OS_LINUX
		CDNFilesetTarget* pTarget = (CDNFilesetTarget*)TargetFactoryI::instance()->create(TARGET_TYPE_FILESET);
		pTarget->setPacingFactory(_pPacedIndexFac);
#else
		FilesetTarget* pTarget = (FilesetTarget*)TargetFactoryI::instance()->create(TARGET_TYPE_FILESET);
#endif
		if(!AddFilter(pTarget))
			return false;
		pTarget->setFilename(strFilename.c_str());
		pTarget->setBandwidth(nMaxBandwidth);
		pTarget->enableProgressEvent(_gCPHCfg.enableProgEvent);
		pTarget->setTrickFile(exMapOutPutfile);
		pTarget->setCsicoMainFileExt(mainFileExt);
		pTarget->setTargetDeletion(_gCPHCfg.deleteTargetFileCapFail);
		pTarget->setWriteLatencyWarning(_gCPHCfg.warningDiskWriteLongLatency);
        pTarget->setTrickSpeed(trickspeed);
		if (bH264Type)
		{
			pTarget->setTypeH264();
			pTarget->enablePacing(true);
			pTarget->enableStreamableEvent(_gCPHCfg.enableStreamEvent);
		}
		else
		{
            pTarget->setIndexType(bIndexVVC);
			pTarget->enablePacing(false);
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

	SetMediaSampleSize(_gCPHCfg.mediaSampleSize);

	if (!Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		setErrorInfo(_nLastErrCode, (std::string("Failed to initialize graph with error: ") + _strLastErr).c_str());			
		return false;
	}

	BaseCPHSession::preLoad();

	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] preload successful"), _strLogHint.c_str());
	return true;
}

void PCAPSess::terminate(bool bProvisionSuccess)
{
	// not started, just constructed
	if (getStatus() == NativeThread::stDeferred)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] to terminate() session while in init status"), _strLogHint.c_str());
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
		MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] to terminate() session with status[%s]"), _strLogHint.c_str(),
			bProvisionSuccess?"success":"failure");
			_bQuit = true;

		if (!bProvisionSuccess && !BaseGraph::IsErrorOccurred())
		{
			BaseGraph::SetLastError("User canceled provision", ERRCODE_USER_CANCELED);
		}

		BaseGraph::Stop();
	}
}

bool PCAPSess::getProgress(::Ice::Long& offset, ::Ice::Long& total)
{	
	total = _filesize;
	offset = getProcessBytes();
	return true;
}

int PCAPSess::run(void)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] run() called"), _strLogHint.c_str());

	_helper.increaseLoad(_strMethod,_nBandwidth);
	bool bRet = Run();
	_helper.decreaseLoad(_strMethod,_nBandwidth);
	_nNetSelector->freeInterface(_strFileName);
	::TianShanIce::Properties params;	
	if (!bRet)
	{
		setErrorInfo(_nLastErrCode, (std::string("Provisioning failed with error: ") + _strLastErr).c_str());			

		if (!_bStartEventSent)
		{
			notifyStarted(params);
		}

		char tmp[40];
		sprintf(tmp, "False");
		params[EVTPM_OPENFORWRITE] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s]OpenForWrite [%s]"), _strLogHint.c_str(),tmp);

		notifyStopped(true, params);

		MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] notifyStopped() called, status[failure], error[%s]"), 
			_strLogHint.c_str(), _strLastErr.c_str());

		Close();
		return 0;
	}
	// query the available range info, and fill in params
	if (_pRTFProc)
	{
		RTFProcess* pProcess = (RTFProcess*)_pRTFProc;
		// get the range info from 
		std::map<std::string, std::string> fileinfomap;
		std::map<std::string, std::string>::iterator iter;
		std::string extCol;

		pProcess->getOutputFileInfo(fileinfomap);
		pProcess->getOutputFileExtCol(extCol);
		// set to params
		for (iter = fileinfomap.begin();iter != fileinfomap.end();iter++)
		{
			params[iter->first.c_str()] = iter->second;
			//MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] File[%s] range[%s]"), _strLogHint.c_str(),iter->first.c_str(),iter->second.c_str());
			params[EVTPM_MEMBERFILEEXTS] = extCol;
			//MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] FileExts[%s]"), _strLogHint.c_str(),extCol.c_str());
		}
	}
	Close();
	int64 supportFileSize=0;
	std::string md5;
	if (!_gCPHCfg.enableTestNTFS)
	{
#ifdef ZQ_OS_LINUX
		CDNFilesetTarget* pTarget = (CDNFilesetTarget*)_pMainTarget;
#else
		FilesetTarget* pTarget = (FilesetTarget*)_pMainTarget;
#endif
		supportFileSize = pTarget->getSupportFileSize();
		pTarget->getMD5(md5);
	}

	char tmp[64];
	_llTotalBytes = _llProcBytes;
	sprintf(tmp, FMT64, _llTotalBytes);
	params[EVTPM_TOTOALSIZE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] Filesize-%s"), _strLogHint.c_str(), tmp);

	sprintf(tmp, FMT64, supportFileSize);
	params[EVTPM_SUPPORTFILESIZE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] SupportFilesize-%s"), _strLogHint.c_str(), tmp);

	params[EVTPM_MD5CHECKSUM] = md5;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] md5-%s"), _strLogHint.c_str(), md5.c_str());

	if (_bitrate)
	{
		sprintf(tmp, FMT64, _llProcBytes*8000/_bitrate);
		params[EVTPM_PLAYTIME] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] playtime-%s"), _strLogHint.c_str(), tmp);
	}
	else {
		MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] bitrate - 0 , can't get playtime"), _strLogHint.c_str());
	}

	sprintf(tmp, "False");
	params[EVTPM_OPENFORWRITE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] OpenForWrite[%s]"), _strLogHint.c_str(),tmp);

	if (!_bStartEventSent)
	{
		notifyStarted(params);
	}
	notifyStopped(false, params);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] notifyStopped() called, status[success]"), _strLogHint.c_str());
	return 0;
}

void PCAPSess::final(int retcode, bool bCancelled)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] final() ret=%d, cancelled=%c; calling doCleanup()"), 
		_strLogHint.c_str(),retcode, bCancelled?'Y':'N');
	
	cleanup();
	try
	{
		delete this;
	}catch(...)
	{
	}
}


int randomPreload(int nPreload)
{
	srand(SYS::getTickCount());
	return (nPreload/2 + int(nPreload * (rand()/(RAND_MAX+1.0)))); 
}

bool PCAPHelper::validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource)
{
	if (!sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "provision session is 0"));
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(CPH_PCAP, 0, "provision session is 0");
	}
	
	std::string strMethod = sess->methodType;
	if(stricmp(strMethod.c_str(), METHODTYPE_RTIVSVSTRM)&&
		stricmp(strMethod.c_str(), METHODTYPE_RTIH264VSVSTRM))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] unsupported method %s"), sess->ident.name.c_str(),
			strMethod.c_str());
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(CPH_PCAP, 0, "unsupported method %s", strMethod.c_str());
	}
	
	if (sess->resources.end() == sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] could not find resource URI"), sess->ident.name.c_str());
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(CPH_PCAP, 0, "could not find resource URI");
	}
	
	TianShanIce::ValueMap& resURI = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(CPH_PCAP, 0, "could not find URI resource " CPHPM_FILENAME);
	}
	
	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(CPH_PCAP, 0, "could not find URI resource " CPHPM_FILENAME);
	}
	
	TianShanIce::ValueMap& resBw = sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(CPH_PCAP, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}
	
	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(CPH_PCAP, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}
	
	TianShanIce::ValueMap& resMI= sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resMI.end() == resMI.find(CPHPM_SOURCEURL))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] could not find multicast resource: %s"), sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(CPH_PCAP, 0, "could not find multicast resource " CPHPM_SOURCEURL);
	}

	TianShanIce::Variant& var2 = resMI[CPHPM_SOURCEURL];
	if (var2.type != TianShanIce::vtStrings || var2.bRange||  var2.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] could not find multicast resource %s"), sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(CPH_PCAP, 0, "could not find multicast resource" CPHPM_SOURCEURL);
	}
	std::string mulicastUrl = var2.strs[0];
	std::string strpro = mulicastUrl.substr(0,6);
	if (stricmp(strpro.c_str(),"udp://")!= 0)
	//if (strstr(mulicastUrl.c_str(),"udp://")==NULL)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_PCAP, "[%s] Can't find the udp protocol from url %s"), sess->ident.name.c_str(), mulicastUrl.c_str());
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(CPH_PCAP, TianShanIce::Storage::csexpInvalidSourceURL, "Can't find the udp header from url[%s]" ,mulicastUrl.c_str());
	}

	sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	sess->preload = randomPreload(_gCPHCfg.preloadTime);

	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_PCAP, "[%s] set preload=%d ms"), sess->ident.name.c_str(), sess->preload);
	return true;
}

bool PCAPSess::prime()
{
        if (!Start())
	{
		setErrorInfo(_nLastErrCode, (std::string("Failed to start graph with error: ") + _strLastErr).c_str());			
		MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] failed to start graph withe error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}
	if (!BaseCPHSession::prime())
		return false;

	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] prime() successful"), _strLogHint.c_str());
	return true;
}

void PCAPSess::OnProgress(int64& prcvBytes)
{
	BaseGraph::OnProgress(prcvBytes);

	::TianShanIce::Properties params;
	char tmp[40];
	// query the available range info, and fill in params
	if (_pRTFProc)
	{
		RTFProcess* pProcess = (RTFProcess*)_pRTFProc;
		// get the range info from 
		std::map<std::string, std::string> fileinfomap;
		std::map<std::string, std::string>::iterator iter;
		std::string extCol;

		pProcess->getOutputFileInfo(fileinfomap);
		pProcess->getOutputFileExtCol(extCol);
		// set to params
		for (iter = fileinfomap.begin();iter != fileinfomap.end();iter++)
		{
			params[iter->first.c_str()] = iter->second;
			//MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] File[%s] range[%s]"), _strLogHint.c_str(),iter->first.c_str(),iter->second.c_str());
			params[EVTPM_MEMBERFILEEXTS] = extCol;
			//MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] FileExts[%s]"), _strLogHint.c_str(),extCol.c_str());
		}
	}

	sprintf(tmp, "True");
	params[EVTPM_OPENFORWRITE] = tmp;
	//MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] %s"), _strLogHint.c_str(),tmp);
	updateProgress(_llProcBytes, _llTotalBytes,params);
}

void PCAPSess::OnStreamable(bool bStreamable)
{
	BaseGraph::OnStreamable(bStreamable);
	notifyStreamable(bStreamable);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void PCAPSess::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());
	
	char tmp[40];
	::TianShanIce::Properties params;
	sprintf(tmp, "%d", mInfo.bitrate);
	params[EVTPM_MPEGBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] bitrate-%s"), _strLogHint.c_str(),tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionV);
	params[EVTPM_VIDEOHEIGHT] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] videoResolutionV-%s"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionH);
	params[EVTPM_VIDEOWIDTH] =  tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] videoResolutionH-%s"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoBitrate);
	params[EVTPM_VIDEOBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] videoBitrate-%s"), _strLogHint.c_str(), tmp);
	sprintf(tmp,"%.2f",mInfo.framerate);
	params[EVTPM_FRAMERATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] framerate-%s"), _strLogHint.c_str(),tmp);

	_bitrate = mInfo.bitrate;
	
	if (_pRTFProc)
	{
		std::string indextype;

		RTFProcess* pProcess = (RTFProcess*)_pRTFProc;
		pProcess->getIndexType(indextype);

		params[EVTPM_INDEXEXT] = indextype;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] IndexTypeExt-%s"), _strLogHint.c_str(),indextype.c_str());

	}

	notifyStarted(params);
	_bStartEventSent = true;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_PCAP, "[%s] notifyStarted() called"), _strLogHint.c_str());
	
	if (_pMainTarget)
	{
		_pMainTarget->setStreamableBytes(mInfo.bitrate*_gCPHCfg.streamReqSecs/8);
	}
}

PCAPHelper::PCAPHelper(ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr)
:BaseCPHelper(pool, mgr) 
{
	PCAPConfig::Methods::iterator it;
	for (it = _gCPHCfg.methods.begin();it != _gCPHCfg.methods.end();it++)
	{
		if (!it->maxSession || !it->maxBandwidth)
			continue;

		_methodCostList[it->methodName] = new MethodCostI(it->maxBandwidth, it->maxSession);				
	}			
}

PCAPHelper::~PCAPHelper()
{
	MethodCostList::iterator it = _methodCostList.begin();
	for (;it!=_methodCostList.end();it++)
	{
		if (it->second)
			delete it->second;
	}

	_methodCostList.clear();
}
/// query the current load information of a method type
///@param[in] methodType to specify the method type to query
///@param[out] allocatedKbps the current allocated bandwidth in Kbps
///@param[out] maxKbps the maximal allowed bandwidth in Kbps, -1 if unlimited
///@param[out] sessions the current running session instances
///@param[out] maxSessins the maximal allowed session instances, -1 if unlimited
///@return true if the query succeeded
bool PCAPHelper:: getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins)
{
	PCAPConfig::Methods::iterator methodIter;
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

bool PCAPHelper::getShareFlag( const std::string& methodType )
{
	PCAPConfig::Methods::iterator methodIter;
	for (methodIter = _gCPHCfg.methods.begin();methodIter != _gCPHCfg.methods.end();methodIter++)
	{
		if (stricmp((*methodIter).methodName.c_str(),methodType.c_str()) == 0)
			break;
	}

	if (methodIter == _gCPHCfg.methods.end())
		return false;

	return (*methodIter).shareFlag;
}
