
#include "BaseClass.h"
#include "CPH_RTINAS.h"
#include "NTFSSource.h"
#include "NTFSTarget.h"
#include "IPushTrigger.h"
#include "PushSource.h"
#include "BaseClass.h"
#include "CPH_RTINASCfg.h"
#include "RTFProc.h"
#include "ErrorCode.h"
#include "FTPSource.h"
#include "NasFilesetTarget.h"
#include "McastCapsrc.h"
#include "HostToIP.h"
#include "WPCapThread.h"



using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define CPH_RTINAS			"CPH_RTINAS"
#define MOLOG					(glog)

ZQTianShan::ContentProvision::BaseCPHelper* RTINasHelper::_theHelper =NULL;

bool checkPacingReg()
{
    const char* pathbuff = {"SOFTWARE\\SeaChange\\PacedIndex\\VVX"};
    HKEY            hKey;
    LONG            status;
    DWORD           dwDisposition;
	
	status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
		pathbuff,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hKey,
		&dwDisposition);
	if (ERROR_SUCCESS != status)
	{
		char szErr1[MAX_PATH+64];
		sprintf(szErr1, "RegOpenKeyEx(HKEY_LOCAL_MACHINE, %s, ...) failed with status %d", pathbuff, status);
		return false;
    }
	
	//	RegOpenKeyEx(hkey, "PacingModule", )
	char szValue[512] = {0};
	DWORD dwType = REG_SZ;
	DWORD dwSize = sizeof(szValue) - 1;
	status = RegQueryValueEx(hKey, "PacingModule",		
		NULL, &dwType,
		(unsigned char*)szValue,
		&dwSize);
	if (ERROR_SUCCESS != status || !szValue[0])
	{
		const char* szVa = "PacedVVX.dll";
		RegSetValueEx(hKey, "PacingModule",	0, REG_SZ, (const unsigned char*)szVa, strlen(szVa));
	}
	
	dwSize = sizeof(szValue) - 1;
	szValue[0] = '\0';
	status = RegQueryValueEx(hKey, "PacingIntervalMs",		
		NULL, &dwType,
		(unsigned char*)szValue,
		&dwSize);
	if (ERROR_SUCCESS != status)
	{
		DWORD PacingIntervalMs = 1000;
		RegSetValueEx(hKey, "PacingIntervalMs",	0, REG_DWORD, (unsigned char*)&PacingIntervalMs, sizeof(PacingIntervalMs));
	}
	
	dwSize = sizeof(szValue) - 1;
	szValue[0] = '\0';
	status = RegQueryValueEx(hKey, "PaceMainFileOnly",		
		NULL, &dwType,
		(unsigned char*)szValue,
		&dwSize);
	if (ERROR_SUCCESS != status)
	{
		DWORD PaceMainFileOnly = 1;
		RegSetValueEx(hKey, "PaceMainFileOnly",	0, REG_DWORD, (unsigned char*)&PaceMainFileOnly, sizeof(PaceMainFileOnly));
	}

	return true;
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
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFileName().c_str());	
		return FALSE;
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "Load configuration from [%s] successfully"),
			_gCPHCfg.getConfigFileName().c_str());

		_gCPHCfg.snmpRegister("");
	}	

	//
	// do some module initialize
	//
	RTFProcess::initRTFLib(_gCPHCfg.rtfMaxSessionNum, &glog, _gCPHCfg.rtfMaxInputBufferBytes,
		_gCPHCfg.rtfMaxInputBuffersPerSession, _gCPHCfg.rtfSessionFailThreshold);

	// check pacing registery settings
	checkPacingReg();

	std::string errstr;

	//multicast capture initialize
	{
		WinpCapThreadInterface* pCaptureInterface;
		pCaptureInterface = new WinpCapThreadInterface();
		pCaptureInterface->setKernelBufferBytes(_gCPHCfg.winpcapKernelBufferInMB*1024*1024);
		pCaptureInterface->setMinBytesToCopy(_gCPHCfg.winpcapMinBufferToCopyInKB*1024);

		MulticastCaptureInterface::setInstance(pCaptureInterface);
		pCaptureInterface->setLog(&glog);

		{
			std::string strLocalIp;
			if (!HostToIP::translateHostIP(_gCPHCfg.szlocalIp, strLocalIp))//translate host name to ip
				strLocalIp = _gCPHCfg.szlocalIp;

			pCaptureInterface->addNIC(strLocalIp);
		}

		if (!pCaptureInterface->init())
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "Failed to initialize capture interface with error: %s"),
				pCaptureInterface->getLastError().c_str());
			return false;
		}
	}

	if (!RTINasHelper::_theHelper)
		RTINasHelper::_theHelper = new RTINasHelper(pEngine->getThreadPool(), pEngine);

	std::vector< ZQ::common::Config::Holder< Method > >::iterator iter = _gCPHCfg.methods.begin();
	if(iter != _gCPHCfg.methods.end())
		pEngine->registerLimitation((*iter).methodName.c_str(), (*iter).maxSession,(*iter).maxBandwidth, GROUP_RTINAS, 0);

	pEngine->registerHelper(METHODTYPE_RTIVSNAS, RTINasHelper::_theHelper, pCtx);

    MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "RTINAS Helper registered"));

	return TRUE;
}

extern "C" __declspec(dllexport) void UninitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return;
	
	pEngine->unregisterHelper(METHODTYPE_RTIVSNAS, pCtx);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "RTINAS Helper unregistered"));

	if (RTINasHelper::_theHelper)
	{
		try
		{
			delete RTINasHelper::_theHelper;
		}
		catch(...){};
		
		RTINasHelper::_theHelper = NULL;
	}
	
	//
	//do some module uninitialize
	//
	MulticastCaptureInterface::destroyInstance();
	RTFProcess::uninitRTFLib();

//	ZQ::common::setGlogger(NULL);
}

///////////////
#include "UrlStr.h"

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

	if (!strnicmp(pathbuf, "file:", strlen("file:")))
	{
		path = pathbuf + strlen("file:");
	}
	else
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

RTINasSess::~RTINasSess()
{
	cleanup();
}

void RTINasSess::cleanup()
{
	if (_bCleaned)
		return;

	BaseGraph::Close();
	_bCleaned = true;
}


bool RTINasSess::preLoad()
{
	 std::string multicastIp;

	if (!_sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "provision session is 0"));
		setErrorInfo(ERRCODE_NULL_SESSION, "provision session is 0");			
		return false;
	}

	std::string strMethod = _sess->methodType;
	_strMethod = strMethod;
	if(stricmp(strMethod.c_str(), METHODTYPE_RTIVSNAS))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] unsupported method %s"), _sess->ident.name.c_str(),
			strMethod.c_str());
		setErrorInfo(ERRCODE_UNSUPPORT_METHOD, "unsupported method");			
		return false;
	}

	if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find resource URI"), _sess->ident.name.c_str());
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find resource URI");			
		return false;
	}

	TianShanIce::ValueMap& resURI = _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource " CPHPM_FILENAME);			
		return false;
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource: " CPHPM_FILENAME);			
		return false;
	}

	std::string strFilename = var.strs[0];

	TianShanIce::ValueMap& resBw = _sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find ProvisionBandwidth resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}
    int nBandwidth = (int)var1.lints[0];
	_nBandwidth = nBandwidth;


	TianShanIce::ValueMap& resMI= _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resMI.end() == resMI.find(CPHPM_SOURCEURL))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find multicast resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_SOURCEURL);			
		return false;
	}

	TianShanIce::Variant& var2 = resMI[CPHPM_SOURCEURL];
	if (var2.type != TianShanIce::vtStrings || var2.bRange|| var2.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_SOURCEURL);			
		return false;
	}
	std::string multicastUrl = var2.strs[0];

	TianShanIce::ValueMap& resURl = _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURl.end() == resURl.find(CPHPM_OUTPUTURL))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_OUTPUTURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_OUTPUTURL);	
		return false;
	}

	TianShanIce::Variant& varUrl = resURl[CPHPM_OUTPUTURL];
	if (varUrl.type != TianShanIce::vtStrings || varUrl.bRange || varUrl.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_OUTPUTURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_OUTPUTURL);	
		return false;
	}
	std::string nasroot = varUrl.strs[0];

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
	
	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_RTINAS, "[%s] bandwidth[%d], max_limit_rate[%d], maxbandwidth[%d]"), _strLogHint.c_str(), 
		nBandwidth, _gCPHCfg.bandwidthLimitRate, nMaxBandwidth);

	
	multicastIp = multicastUrl.substr(multicastUrl.find_first_of(':')+3,multicastUrl.find_last_of(':')-6);
	std::string strmulticastPort = multicastUrl.substr(multicastUrl.find_last_of(':')+1,multicastUrl.size()-multicastUrl.find_last_of(':')-1);
	int multicastPort = atoi(strmulticastPort.c_str());

	DWORD timeoutInterval = _gCPHCfg.timeoutInterval;
	std::string localIp;
	if (!HostToIP::translateHostIP(_gCPHCfg.szlocalIp,localIp))//translate host name to ip
		localIp = _gCPHCfg.szlocalIp;
	McastCapSource* pSource = (McastCapSource*)SourceFactory::Create(SOURCE_TYPE_MCASTCAPSRC, &_helper._pool);	
	AddFilter(pSource);
	pSource->setInspectPara(multicastIp,multicastPort,timeoutInterval,localIp);
	//dumper parameters
	pSource->enableDump(_gCPHCfg.enableDump);
	pSource->setDumpPath(_gCPHCfg.szDumpPath);
	pSource->deleteDumpOnSuccess(_gCPHCfg.deleteDumpOnSuccess);

	std::map<std::string, int> exMap;
	exMap.insert(std::make_pair(std::string(".FF"),0));
	exMap.insert(std::make_pair(std::string(".FR"),0));

	RTFProcess* pProcess = (RTFProcess*)ProcessFactory::Create(PROCESS_TYPE_RTF, &_helper._pool);
	AddFilter(pProcess);
	pProcess->setTrickFile(exMap);

	fixpath(strFilename);
	fixpath(nasroot);
	std::string filepath;
	std::string absoluteFilename;
	if (strFilename.find('\\') != -1)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] Need assemble the multilevel directory"),_strLogHint.c_str());
		absoluteFilename = strFilename.substr(strFilename.find_last_of('\\')+1,strFilename.size()-strFilename.find_last_of('\\')-1);
		std::string absolutePath = strFilename.substr(0,strFilename.find_last_of('\\'));
		if (*absolutePath.begin() == '\\')
		{
			std::string temp = absolutePath.substr(1,absolutePath.size()-1);
			absolutePath = temp;
		}

		if (*(nasroot.end()-1) != '\\')
			filepath = nasroot + '\\'+ absolutePath;
		else
			filepath = nasroot + absolutePath;	
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] File path is %s and absolute name is %s"),_strLogHint.c_str(),filepath.c_str(),absoluteFilename.c_str());

	}
	else
	{
		absoluteFilename= strFilename;
		filepath = nasroot;
	}
	
	NasFsTarget* pTarget = (NasFsTarget*)TargetFactory::Create(TARGET_TYPE_NAS, &_helper._pool);
	if(!AddFilter(pTarget))
		return false;
	pTarget->setFilename(absoluteFilename.c_str());
	pTarget->setCacheDirectory(_gCPHCfg.szCacheDir);
	pTarget->setFilePath(filepath.c_str());
	pTarget->setBandwidth(nMaxBandwidth);

	pTarget->enablePacing(true);
	pTarget->enableStreamableEvent(_gCPHCfg.enableStreamEvent);
	pTarget->enableProgressEvent(_gCPHCfg.enableProgEvent);
	pTarget->enableMD5(_gCPHCfg.enableMD5);
	_pMainTarget = pTarget;
	
	InitPins();
	if (!ConnectTo(pSource, 0, pProcess, 0))
		return false;
	if (!ConnectTo(pProcess, 0, pTarget, 0))
		return false;
	if (!ConnectTo(pProcess, 1, pTarget, 1))
		return false;
	if (!ConnectTo(pProcess, 2, pTarget, 2))
		return false;
	if (!ConnectTo(pProcess, 3, pTarget, 3))
		return false;

	SetMediaSampleSize(_gCPHCfg.mediaSampleSize);

	if (!Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		setErrorInfo(_nLastErrCode, (std::string("Failed to initialize graph with error: ") + _strLastErr).c_str());			
		return false;
	}

	BaseCPHSession::preLoad();

	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] preload successful"), _strLogHint.c_str());
	return true;
}

void RTINasSess::terminate(bool bProvisionSuccess)
{
	// not started, just constructed
	if (getStatus() == NativeThread::stDeferred)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] to terminate() session while in init status"), _strLogHint.c_str());
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

bool RTINasSess::getProgress(::Ice::Long& offset, ::Ice::Long& total)
{	
	total = _filesize;
	offset = getProcessBytes();
	return true;
}

int RTINasSess::run(void)
{	
	_helper.increaseLoad(_strMethod,_nBandwidth);
	bool bRet = Run();
	_helper.decreaseLoad(_strMethod,_nBandwidth);
	::TianShanIce::Properties params;	
	if (!bRet)
	{
		setErrorInfo(_nLastErrCode, (std::string("Provisioning failed with error: ") + _strLastErr).c_str());			
		Close();

		if (!_bStartEventSent)
		{
			notifyStarted(params);
		}
		notifyStopped(true, params);

		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] notifyStopped() called, status[failure], error[%s]"), 
			_strLogHint.c_str(), _strLastErr.c_str());
		return 0;
	}

	LONGLONG supportFileSize=0;
	NasFsTarget* pTarget = (NasFsTarget*)_pMainTarget;
	pTarget->getSupportFileSize(supportFileSize);
	std::string md5;
	pTarget->getMD5(md5);
	char tmp[64];
	if (!_llTotalBytes)
	{
		_llTotalBytes = _llProcBytes;
	}
	sprintf(tmp, "%lld", _llTotalBytes);
	params[EVTPM_TOTOALSIZE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] Filesize[%s]"), _strLogHint.c_str(), tmp);

	sprintf(tmp, "%lld", supportFileSize);
	params[EVTPM_SUPPORTFILESIZE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] SupportFilesize[%s]"), _strLogHint.c_str(), tmp);

	params[EVTPM_MD5CHECKSUM] = md5;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] md5[%s]"), _strLogHint.c_str(), md5.c_str());

	if (_bitrate)
	{
		sprintf(tmp, "%lld", _llProcBytes*8000/_bitrate);
		params[EVTPM_PLAYTIME] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] playtime[%s]"), _strLogHint.c_str(), tmp);
	}
	else {
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] bitrate [0] , can't get playtime"), _strLogHint.c_str());
	}
	
	Close();

	if (!_bStartEventSent)
	{
		notifyStarted(params);
	}
	notifyStopped(false, params);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] notifyStopped() called, status[success]"), _strLogHint.c_str());
	return 0;
}

void RTINasSess::final(int retcode, bool bCancelled)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] final() ret=%d, cancelled=%c; calling doCleanup()"), 
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
	srand(GetTickCount());
	return (nPreload/2 + int(nPreload * (rand()/(RAND_MAX+1.0)))); 
}

bool RTINasHelper::validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource)
{
	if (!sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "provision session is 0"));
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTINAS, 0, "provision session is 0");
	}
	
	std::string strMethod = sess->methodType;
	if(stricmp(strMethod.c_str(), METHODTYPE_RTIVSNAS))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] unsupported method %s"), sess->ident.name.c_str(),
			strMethod.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTINAS, 0, "unsupported method %s", strMethod.c_str());
	}
	
	if (sess->resources.end() == sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find resource URI"), sess->ident.name.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTINAS, 0, "could not find resource URI");
	}

	TianShanIce::ValueMap& resURI = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTINAS, 0, "could not find URI resource " CPHPM_FILENAME);
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTINAS, 0, "could not find URI resource " CPHPM_FILENAME);
	}

	TianShanIce::ValueMap& resURl = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURl.end() == resURl.find(CPHPM_OUTPUTURL))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTINAS, 0, "could not find URI resource " CPHPM_OUTPUTURL);
	}

	TianShanIce::Variant& varUrl = resURl[CPHPM_OUTPUTURL];
	if (varUrl.type != TianShanIce::vtStrings || varUrl.bRange || varUrl.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTINAS, 0, "could not find URI resource " CPHPM_OUTPUTURL);
	}

	TianShanIce::ValueMap& resBw = sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTINAS, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTINAS, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}

	TianShanIce::ValueMap& resMI= sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resMI.end() == resMI.find(CPHPM_SOURCEURL))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find multicast resource: %s"), sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTINAS, 0, "could not find multicast resource " CPHPM_SOURCEURL);
	}

	TianShanIce::Variant& var2 = resMI[CPHPM_SOURCEURL];
	if (var2.type != TianShanIce::vtStrings || var2.bRange|| var2.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] could not find multicast resource %s"), sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTINAS, 0, "could not find multicast resource" CPHPM_SOURCEURL);
	}
	std::string mulicastUrl = var2.strs[0];
	std::string strpro = mulicastUrl.substr(0,6);
	if (_stricmp(strpro.c_str(),"udp://")!= 0)
//	if ((strstr(mulicastUrl.c_str(),"udp://")==NULL)&&(strstr(mulicastUrl.c_str(),"UDP://")==NULL))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTINAS, "[%s] Can't find the udp protocol from url %s"), sess->ident.name.c_str(), mulicastUrl.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTINAS, TianShanIce::Storage::csexpInvalidSourceURL, "Can't find the udp header from url" ,mulicastUrl.c_str());
	}

	sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	sess->preload = randomPreload(_gCPHCfg.preloadTime);

	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_RTINAS, "[%s] set preload=%d ms"), sess->ident.name.c_str(), sess->preload);
	return true;
}

bool RTINasSess::prime()
{
	if (!BaseCPHSession::prime())
		return false;

	if (!Start())
	{
		setErrorInfo(_nLastErrCode, (std::string("Failed to start graph with error: ") + _strLastErr).c_str());			
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] failed to start graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}

	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] prime() successful"), _strLogHint.c_str());
	return true;
}

void RTINasSess::OnProgress(LONGLONG& prcvBytes)
{
	BaseGraph::OnProgress(prcvBytes);
	::TianShanIce::Properties params;
	updateProgress(_llProcBytes, _llTotalBytes,params);
}

void RTINasSess::OnStreamable(bool bStreamable)
{
	BaseGraph::OnStreamable(bStreamable);
	notifyStreamable(bStreamable);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void RTINasSess::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());

	char tmp[40];
	::TianShanIce::Properties params;
	sprintf(tmp, "%d", mInfo.bitrate);
	params[EVTPM_MPEGBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] bitrate[%s]"), _strLogHint.c_str(),tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionV);
	params[EVTPM_VIDEOHEIGHT] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] videoResolutionV[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionH);
	params[EVTPM_VIDEOWIDTH] =  tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] videoResolutionH[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoBitrate);
	params[EVTPM_VIDEOBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] videoBitrate[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp,"%.2f",mInfo.framerate);
	params[EVTPM_FRAMERATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] framerate[%s]"), _strLogHint.c_str(),tmp);


	_bitrate = mInfo.bitrate;

	notifyStarted(params);
	_bStartEventSent = true;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTINAS, "[%s] notifyStarted() called"), _strLogHint.c_str());

	if (_pMainTarget)
	{
		_pMainTarget->setStreamableBytes(mInfo.bitrate*_gCPHCfg.streamReqSecs/8);
	}
}

RTINasHelper::RTINasHelper(ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr)
:BaseCPHelper(pool, mgr) 
{
	
}

RTINasHelper::~RTINasHelper()
{

}


bool RTINasHelper::getBandwidthLoad(const char* methodType, long& bpsAllocated, long& bpsMax, long& initCost)
{
	if (NULL == methodType || 0 != stricmp(METHODTYPE_RTFRDSVSVSTRM, methodType))
		return false;
	
	bpsAllocated =0;
	CPHRTINasCfg::Methods::iterator methodIter;
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
bool RTINasHelper:: getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins)
{
	if (stricmp(methodType, METHODTYPE_RTIVSNAS))
		return false;
	
	CPHRTINasCfg::Methods::iterator methodIter;
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

unsigned int RTINasHelper::evaluateCost(unsigned int bandwidthKbps, unsigned int sessions)
{
	static unsigned int maxsessionNum = _gCPHCfg.methods[0].maxSession;
	static unsigned int maxBandwidthKBps = _gCPHCfg.methods[0].maxBandwidth;

	if (sessions >maxsessionNum)
		return MAX_LOAD_VALUE + 1;

	if(bandwidthKbps > maxBandwidthKBps)	
		return MAX_LOAD_VALUE + 1;

	int nCost1 = (((float)bandwidthKbps)/maxBandwidthKBps)*MAX_LOAD_VALUE; 
	int nCost2 = (((float)sessions)/maxsessionNum)*MAX_LOAD_VALUE; 

	return __max(nCost1, nCost2);
}
