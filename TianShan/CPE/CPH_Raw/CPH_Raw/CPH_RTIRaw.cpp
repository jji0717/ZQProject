
#include <list>
#include <math.h>

#ifdef ZQ_OS_LINUX
	#include <time.h>
#endif

#include "BaseClass.h"
#include "ErrorCode.h"
#include "HostToIP.h"

#include "RTIRawSource.h"
#include "CPH_RTIRawCfg.h"
#include "RTIRawTargetFac.h"
#include "TargetFactoryI.h"
#include "RTIRawFileIoFactory.h"
#include "CPH_RTIRaw.h"
#include "RTIRawTarget.h"

#include "WPCapThread.h"

using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define CPH_RTIRaw			"CPH_RTIRaw"
#define MOLOG					(glog)


std::auto_ptr<ZQTianShan::ContentProvision::FileIoFactory> _pFileIoFac;


ZQTianShan::ContentProvision::BaseCPHelper* RTIRawHelper::_theHelper =NULL;

extern "C" __EXPORT bool InitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return false;
	
	// set log handler
	ZQ::common::setGlogger(pEngine->getLogger());
	_gCPHCfg.setLogger(&glog);

	// load configurations
	std::string strCfgDir;
	strCfgDir = pEngine->getConfigDir();
	if (!_gCPHCfg.loadInFolder(strCfgDir.c_str()))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFilePath().c_str());	
		return false;		
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "Load configuration from [%s] successfully"),
			_gCPHCfg.getConfigFilePath().c_str());
		_gCPHCfg.snmpRegister("");
	}	
	//
	// do some module initialize
	//
	RTIRawFileIoFactory* pFactory = new RTIRawFileIoFactory();
	//pFactory->setRootDir(_gCPHCfg.outputDir.c_str());
	

	_pFileIoFac.reset(pFactory);

	_pFileIoFac->setLog(&glog);
	if (!_pFileIoFac->initialize())
	{
		std::string strErr;
		int nErrCode;
		_pFileIoFac->getLastError(strErr, nErrCode);
		printf("Failed to initialize fileio factory with error: %s, code: %d\n", strErr.c_str(), nErrCode);
		return false;
	}
	RTIRawTargetFac * pTargetFac = new RTIRawTargetFac(_pFileIoFac.get());
	TargetFactoryI::setInstance(pTargetFac);
	if (!RTIRawHelper::_theHelper)
		RTIRawHelper::_theHelper = new RTIRawHelper(pEngine->getThreadPool(), pEngine);
	//multicast capture initialize
	{
		WinpCapThreadInterface* pCaptureInterface;
		pCaptureInterface = new WinpCapThreadInterface();
		pCaptureInterface->setKernelBufferBytes(_gCPHCfg.winpcapKernelBufferInMB*1024*1024);
		pCaptureInterface->setMinBytesToCopy(_gCPHCfg.winpcapMinBufferToCopyInKB*1024);
		MulticastCaptureInterface::setInstance(pCaptureInterface);
		pCaptureInterface->setLog(&glog);
		std::string strLocalIp;
		if (!HostToIP::translateHostIP(_gCPHCfg.captureIp.c_str(), strLocalIp))//translate host name to ip
			strLocalIp = _gCPHCfg.captureIp;
		pCaptureInterface->addNIC(strLocalIp,_gCPHCfg.totalBandwidth);
		/*for(unsigned int i=0;i<_gCPHCfg.nInterface.size();i++)
		{
			std::string strLocalIp;
			if (!HostToIP::translateHostIP(_gCPHCfg.nInterface[i].strIp.c_str(), strLocalIp))//translate host name to ip
				strLocalIp = _gCPHCfg.nInterface[i].strIp;

			pCaptureInterface->addNIC(strLocalIp, _gCPHCfg.nInterface[i].totalBandwidth);
		}*/

		if (!pCaptureInterface->init())
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "Failed to initialize capture interface with error: %s"),
				pCaptureInterface->getLastError().c_str());
			return false;
		}
	}
	for (std::vector< ZQ::common::Config::Holder< Method > >::iterator iter = _gCPHCfg.methods.begin();iter != _gCPHCfg.methods.end();iter++)
	{
		if (iter->maxBandwidth && iter->maxSession)
		{
			pEngine->registerHelper((*iter).methodName.c_str(), RTIRawHelper::_theHelper, pCtx);
			pEngine->registerLimitation((*iter).methodName.c_str(), (*iter).maxSession,(*iter).maxBandwidth, GROUP_RTI, 0);
			MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "%s Helper registered"),(*iter).methodName.c_str());
		}
		else
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "%s Helper disabled"),(*iter).methodName.c_str());
		}
	}

	return true;
}

extern "C" __EXPORT void UninitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return;

	pEngine->unregisterHelper(METHODTYPE_AQUAREC, pCtx);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "Helper unregistered"));
	if (RTIRawHelper::_theHelper)
	{
		try
		{
			delete RTIRawHelper::_theHelper;
		}
		catch(...){};

		RTIRawHelper::_theHelper = NULL;
	}
	//
	//do some module uninitialize
	//
	MulticastCaptureInterface::destroyInstance();
	if (TargetFactoryI::instance())
	{
		TargetFactoryI::destroyInstance();
	}
	if (_pFileIoFac.get())
	{
		_pFileIoFac->uninitialize();
		_pFileIoFac.reset(0);
	}

	//	ZQ::common::setGlogger(NULL);
}
RTIRawSess::~RTIRawSess()
{
	/*_nNetSelector->freeInterface(_strFileName);*/
	//cleanup();
}

void RTIRawSess::cleanup()
{
	if (_bCleaned)
		return;
	BaseGraph::Close();
	_bCleaned = true;
}


bool RTIRawSess::preLoad()
{
	std::string multicastIp;
	if (!_sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "provision session is 0"));
		setErrorInfo(ERRCODE_NULL_SESSION, "provision session is 0");			
		return false; 
	}
	std::string strMethod = _sess->methodType;
	_strMethod = strMethod;
	if(stricmp(strMethod.c_str(), METHODTYPE_RTIRAW))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] unsupported method %s"), _sess->ident.name.c_str(),strMethod.c_str());
		setErrorInfo(ERRCODE_UNSUPPORT_METHOD, "unsupported method");			
		return false;
	}

	if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] could not find resource URI"), _sess->ident.name.c_str());
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find resource URI");			
		return false;
	}

	TianShanIce::ValueMap& resURI = _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource " CPHPM_FILENAME);			
		return false;
	}
	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource: " CPHPM_FILENAME);			
		return false;
	}
	std::string strFilename = var.strs[0];

	MOLOG(Log::L_INFO,CLOGFMT(CPH_RTIRaw,"%s preload() get filename[%s]."), _sess->ident.name.c_str(),strFilename.c_str());

 	_strFileName.clear();
   	//_strFileName +="C:/TianShan/RTIRawData";
   	//_strFileName += "/cetstrest";
	_strFileName += strFilename;
	TianShanIce::ValueMap& resBw = _sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] could not find ProvisionBandwidth resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}


	int nBandwidth = (int)var1.lints[0];
	_nBandwidth = nBandwidth;

	TianShanIce::ValueMap& resMI= _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resMI.end() == resMI.find(CPHPM_SOURCEURL))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] could not find multicast resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_SOURCEURL);			
		return false;
	}

	TianShanIce::Variant& var2 = resMI[CPHPM_SOURCEURL];
	if (var2.type != TianShanIce::vtStrings || var2.bRange|| var2.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_SOURCEURL);			
		return false;
	}
	std::string multicastUrl = var2.strs[0];

	multicastIp = multicastUrl.substr(multicastUrl.find_first_of(':')+3,multicastUrl.find_last_of(':')-6);
	std::string strmulticastPort = multicastUrl.substr(multicastUrl.find_last_of(':')+1,multicastUrl.size()-multicastUrl.find_last_of(':')-1);
	int multicastPort = atoi(strmulticastPort.c_str());

	SetLog(_helper.getLog());
	SetMemAlloc(_helper.getMemoryAllocator());
	SetLogHint(strFilename.c_str());
	int nMaxBandwidth = nBandwidth;

	unsigned long timeoutInterval = _gCPHCfg.timeoutInterval;
	std::string localIp;
	if (!HostToIP::translateHostIP(_gCPHCfg.captureIp.c_str(), localIp))//translate host name to ip
		localIp = _gCPHCfg.captureIp;
	RTIRawTarget* pTarget = (RTIRawTarget*)TargetFactoryI::instance()->create(TARGET_TYPE_RTIRAW);
	if(!AddFilter(pTarget))
		return false;
	pTarget->setFilename(_strFileName.c_str());
	pTarget->setCacheDirectory(_gCPHCfg.szCacheDir);
	pTarget->enableProgressEvent(_gCPHCfg.enableProgEvent);
	_pMainTarget = pTarget;

	_nSchedulePlayTime = (_sess->scheduledEnd - _sess->scheduledStart);
	RTIRawSource* pSource = (RTIRawSource*)SourceFactory::Create(SOURCE_TYPE_RTIRAW);
	if(!AddFilter(pSource))
		return false;
//	pSource->setPool(RTIRawHelper::_theHelper->_pool);
	pSource->setScheduleTime(_sess->scheduledStart + (_nSchedulePlayTime * 0.8));
	pSource->setFilter(pTarget);
	pSource->setInspectPara(multicastIp,multicastPort,timeoutInterval,localIp);
	
	SetMediaSampleSize(_gCPHCfg.mediaSampleSize);
	if (!Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		setErrorInfo(_nLastErrCode, (std::string("Failed to initialize graph with error: ") + _strLastErr).c_str());			
		return false;
	}

	BaseCPHSession::preLoad();

	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "[%s] preload successful"), _strLogHint.c_str());
	return true;
}

void RTIRawSess::terminate(bool bProvisionSuccess)
{
	// not started, just constructed
	if (getStatus() == NativeThread::stDeferred)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "[%s] to terminate() session while in init status"), _strLogHint.c_str());
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
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "[%s] to terminate() session with status[%s]"), _strLogHint.c_str(),
			bProvisionSuccess?"success":"failure");
		_bQuit = true;
		if (!bProvisionSuccess && !BaseGraph::IsErrorOccurred())
		{
			BaseGraph::SetLastError("User canceled provision", ERRCODE_USER_CANCELED);
		}
		BaseGraph::Stop();
	}
}

bool RTIRawSess::getProgress(::Ice::Long& offset, ::Ice::Long& total)
{	
	total = _filesize;
	offset = getProcessBytes();
	return true;
}

int RTIRawSess::run(void)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "[%s] run() called"), _strLogHint.c_str());

	_helper.increaseLoad(_strMethod,_nBandwidth);
	int64 sessStartTime = ZQ::common::TimeUtil::now();

#ifdef PROCESSED_BY_TIME
	_nsessStartTime = sessStartTime;
#endif

	bool bRet = Run();
	_helper.decreaseLoad(_strMethod,_nBandwidth);
	/*_nNetSelector->freeInterface(_strFileName);*/
	::TianShanIce::Properties params;	
	if (!bRet)
	{
		setErrorInfo(_nLastErrCode, (std::string("Provisioning failed with error: ") + _strLastErr).c_str());			
		if (!_bStartEventSent)
		{
			notifyStarted(params);
		}
		char tmp[64];
		_llTotalBytes = _llProcBytes;		//use the real process bytes
		sprintf(tmp, "%lld", _llTotalBytes);
		params[EVTPM_TOTOALSIZE] = tmp;
		OnProgress(_llProcBytes);
		notifyStopped(true, params);
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "[%s] notifyStopped() called, status[failure], error[%s]"), 
			_strLogHint.c_str(), _strLastErr.c_str());
		Close();
		std::remove(_strFileName.c_str());
		MOLOG(Log::L_ERROR,CLOGFMT(CPH_RTIRaw,"[%s]failed to record the data with error[%s]"),_strLogHint.c_str(), GetLastError().c_str());
		return 0;
	}
	int64 supportFileSize=0;
	char tmp[64];
	_llTotalBytes = _llProcBytes;		//use the real process bytes
	sprintf(tmp, "%lld", _llTotalBytes);
	params[EVTPM_TOTOALSIZE] = tmp;
	if (_bitrate)
	{
		sprintf(tmp, "%lld", _llProcBytes*8000/_bitrate);
		params[EVTPM_PLAYTIME] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "[%s] playtime[%s]"), _strLogHint.c_str(), tmp);
	}
	else {
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "[%s] bitrate [0] , can't get playtime"), _strLogHint.c_str());
	}
	if (!_bStartEventSent)
		notifyStarted(params);
	Close();
	int64 sessEndTime = ZQ::common::TimeUtil::now();
	int64 sessTotalTime = (sessEndTime - sessStartTime) / 1000;
	MOLOG(Log::L_INFO,CLOGFMT(CPH_RTIRaw,"[%s] record filesize[%lld]Byte,use Time[%lld]s,bitrate[%lld] bps"),_strLogHint.c_str(), _llTotalBytes, sessTotalTime, _llTotalBytes*8/sessTotalTime);
	OnProgress(_llProcBytes);
	notifyStopped(false, params);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "[%s] run() success, status[success]"), _strLogHint.c_str());
	return 0;
}

void RTIRawSess::final(int retcode, bool bCancelled)
{
 	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "[%s] final() ret=%d, cancelled=%c; calling doCleanup()"), 
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
#ifdef ZQ_OS_MSWIN
	srand(GetTickCount());
#else
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	srand(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif
	return (nPreload/2 + int(nPreload * (rand()/(RAND_MAX+1.0)))); 
}

bool RTIRawHelper::validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource)
{
	if (!sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "provision session is 0"));
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_RTIRaw, 0, "provision session is 0");
	}

	std::string strMethod = sess->methodType;
	if(stricmp(strMethod.c_str(), METHODTYPE_RTIRAW))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] unsupported method %s"), sess->ident.name.c_str(),
			strMethod.c_str());
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_RTIRaw, 0, "unsupported method %s", strMethod.c_str());
	}

	if (sess->resources.end() == sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] could not find resource URI"), sess->ident.name.c_str());
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_RTIRaw, 0, "could not find resource URI");
	}

	TianShanIce::ValueMap& resURI = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_RTIRaw, 0, "could not find URI resource " CPHPM_FILENAME);
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_RTIRaw, 0, "could not find URI resource " CPHPM_FILENAME);
	}

	TianShanIce::ValueMap& resBw = sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_RTIRaw, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_RTIRaw, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}

	TianShanIce::ValueMap& resMI= sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resMI.end() == resMI.find(CPHPM_SOURCEURL))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] could not find multicast resource: %s"), sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_RTIRaw, 0, "could not find multicast resource " CPHPM_SOURCEURL);
	}

	TianShanIce::Variant& var2 = resMI[CPHPM_SOURCEURL];
	if (var2.type != TianShanIce::vtStrings || var2.bRange||  var2.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] could not find multicast resource %s"), sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_RTIRaw, 0, "could not find multicast resource" CPHPM_SOURCEURL);
	}
	std::string mulicastUrl = var2.strs[0];
	std::string strpro = mulicastUrl.substr(0,6);
	if (stricmp(strpro.c_str(),"udp://")!= 0)
		//if (strstr(mulicastUrl.c_str(),"udp://")==NULL)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTIRaw, "[%s] Can't find the udp protocol from url %s"), sess->ident.name.c_str(), mulicastUrl.c_str());
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_RTIRaw, TianShanIce::Storage::csexpInvalidSourceURL, "Can't find the udp header from url" ,mulicastUrl.c_str());
	}

	sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	sess->preload = randomPreload(_gCPHCfg.preloadTime);

	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_RTIRaw, "[%s] set preload=%d ms"), sess->ident.name.c_str(), sess->preload);
	return true;
}

bool RTIRawSess::prime()
{
	if (!BaseCPHSession::prime())
		return false;
	if (!Start())
	{
		setErrorInfo(_nLastErrCode, (std::string("Failed to start graph with error: ") + _strLastErr).c_str());		
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "[%s] failed to start graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}

	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "[%s] prime() successful"), _strLogHint.c_str());
	return true;
}

void RTIRawSess::OnProgress(int64& prcvBytes)
{
	BaseGraph::OnProgress(prcvBytes);
	::TianShanIce::Properties params;
	
#ifdef PROCESSED_BY_TIME
	Ice::Long nRealPlayTime = ZQ::common::now() - _nsessStartTime;
	char tmp[64];
	
	snprintf(tmp, sizeof(tmp), "%lld", nRealPlayTime);
	params["realplaytime"] = tmp;

	
	snprintf(tmp, sizeof(tmp), "%lld",_nSchedulePlayTime);
	params["allplaytime"] = tmp;
#endif

	updateProgress(_llProcBytes, _llTotalBytes,params);
}

void RTIRawSess::OnStreamable(bool bStreamable)
{
	BaseGraph::OnStreamable(bStreamable);
	notifyStreamable(bStreamable);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void RTIRawSess::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());
	::TianShanIce::Properties params;
	notifyStarted(params);
	_bStartEventSent = true;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTIRaw, "[%s] notifyStarted() called"), _strLogHint.c_str());
}
void RTIRawSess::OnStartNotify()
{
	::TianShanIce::Properties params;
	notifyStarted(params);
	_bStartEventSent = true;
}

RTIRawHelper::RTIRawHelper(ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr)
:BaseCPHelper(pool, mgr) 
{
	RtiRawConfig::Methods::iterator it;
	for (it = _gCPHCfg.methods.begin();it != _gCPHCfg.methods.end();it++)
	{
		if (!it->maxSession || !it->maxBandwidth)
			continue;

		_methodCostList[it->methodName] = new MethodCostI(it->maxBandwidth, it->maxSession);				
	}			
}

RTIRawHelper::~RTIRawHelper()
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
bool RTIRawHelper:: getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins)
{
	RtiRawConfig::Methods::iterator methodIter;
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
