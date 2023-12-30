
#include "BaseClass.h"
#include "CPH_RDS.h"
#include "NTFSSource.h"
#include "NTFSTarget.h"
#include "TrickImportUser.h"
#include "IPushTrigger.h"
#include "PushSource.h"
#include "BaseClass.h"
#include "VstrmFilesetTarget.h"
#include "CPH_RDSCfg.h"
#include "ErrorCode.h"


using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define CPH_RDS			"CPH_RDS"
#define MOLOG					(glog)

ZQTianShan::ContentProvision::BaseCPHelper* RDSHelper::_theHelper =NULL;

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
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFileName().c_str());	
		return FALSE;
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RDS, "Load configuration from [%s] successfully"),
			_gCPHCfg.getConfigFileName().c_str());

		_gCPHCfg.snmpRegister("");
	}	

	if (!RDSHelper::_theHelper)
		RDSHelper::_theHelper = new RDSHelper(pEngine->getThreadPool(), pEngine);

	std::vector< ZQ::common::Config::Holder< Method > >::iterator iter = _gCPHCfg.methods.begin();
	if(iter != _gCPHCfg.methods.end())
		pEngine->registerLimitation((*iter).methodName.c_str(), (*iter).maxSession,(*iter).maxBandwidth, GROUP_RDS, 0);

	pEngine->registerHelper(METHODTYPE_RDSVSVSTRM, RDSHelper::_theHelper, pCtx);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RDS, "RDS Helper registered"));

	//
	// do some module initialize
	//
	std::string errmsg;
	if (!VstrmFsTarget::initVstrm(_gCPHCfg.vstrmBwClientId, errmsg))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "Failed to init vstrm class with error %s"),errmsg.c_str());
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RDS, "initVstrm() successful"));
	}

	return TRUE;
}

extern "C" __declspec(dllexport) void UninitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return;
	
	pEngine->unregisterHelper(METHODTYPE_RDSVSVSTRM, pCtx);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RDS, "RDS Helper unregistered"));

	if (RDSHelper::_theHelper)
	{
		try
		{
			delete RDSHelper::_theHelper;
		}
		catch(...){};

		RDSHelper::_theHelper = NULL;
	}

	//
	//do some module uninitialize
	//
	VstrmFsTarget::uninitVstrm();

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

RDSSess::~RDSSess()
{
	cleanup();
}

void RDSSess::cleanup()
{
	if (_bCleaned)
		return;

	BaseGraph::Close();
	_bCleaned = true;
}


bool RDSSess::preLoad()
{
	if (!_sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "provision session is 0"));
		setErrorInfo(ERRCODE_NULL_SESSION, "provision session is 0");			
		return false;
	}

	std::string strMethod = _sess->methodType;
	_strMethod = strMethod;
	if(stricmp(strMethod.c_str(), METHODTYPE_RDSVSVSTRM))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "[%s] unsupported method %s"), _sess->ident.name.c_str(),
			strMethod.c_str());
		setErrorInfo(ERRCODE_UNSUPPORT_METHOD, "unsupported method");			
		return false;
	}

	if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "[%s] could not find resource URI"), _sess->ident.name.c_str());
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find resource URI");			
		return false;
	}

	TianShanIce::ValueMap& resURI = _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource " CPHPM_FILENAME);			
		return false;
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource: " CPHPM_FILENAME);			
		return false;
	}

	std::string strFilename = var.strs[0];

	TianShanIce::ValueMap& resBw = _sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "[%s] could not find ProvisionBandwidth resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}

	int nBandwidth = (int)var1.lints[0];
	_nBandwidth = nBandwidth;

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
	
	PushSource* pSource = (PushSource*)SourceFactory::Create(SOURCE_TYPE_PUSHSRC);	
	AddFilter(pSource);
	pSource->setMaxBandwidth(nMaxBandwidth);

	CTrickImportUser* pProcess = (CTrickImportUser*)ProcessFactory::Create(PROCESS_TYPE_TRICKGEN);
	AddFilter(pProcess);
	pProcess->setMaxCodingError(_gCPHCfg.maxCodingError);
	
	if (_gCPHCfg.enableTestNTFS)
	{
		NTFSTarget* pTarget[4] = {0};
		pTarget[0]=(NTFSTarget*)TargetFactory::Create(TARGET_TYPE_NTFS);
		pTarget[1]=(NTFSTarget*)TargetFactory::Create(TARGET_TYPE_NTFS);
		pTarget[2]=(NTFSTarget*)TargetFactory::Create(TARGET_TYPE_NTFS);
		pTarget[3]=(NTFSTarget*)TargetFactory::Create(TARGET_TYPE_NTFS);
		
		if(!AddFilter(pTarget[0]))
			return false;
		if(!AddFilter(pTarget[1]))
			return false;
		if (!AddFilter(pTarget[2]))
			return false;
		if (!AddFilter(pTarget[3]))
			return false;

		std::string strPath = _gCPHCfg.szNTFSOutputDir;
		if(!( strPath[strPath.length()-1]=='\\' || strPath[strPath.length()-1]=='/'))
			strPath+="\\";
		std::string strFile = strPath + strFilename;
		
		pTarget[0]->setFilename(strFile.c_str());
		strFile = strPath + strFilename + ".ff";
		pTarget[1]->setFilename(strFile.c_str());
		strFile = strPath + strFilename + ".fr";
		pTarget[2]->setFilename(strFile.c_str());
		strFile = strPath + strFilename + ".vvx";
		pTarget[3]->setFilename(strFile.c_str());
		pTarget[0]->enableStreamableEvent(_gCPHCfg.enableStreamEvent);
		pTarget[0]->enableProgressEvent(_gCPHCfg.enableProgEvent);
		_pMainTarget = pTarget[0];		
		
		InitPins();
		if (!ConnectTo(pSource, 0, pProcess, 0))
			return false;
		if (!ConnectTo(pProcess, 0, pTarget[0], 0))
			return false;
		if (!ConnectTo(pProcess, 1, pTarget[1], 0))
			return false;
		if (!ConnectTo(pProcess, 2, pTarget[2], 0))
			return false;
		if (!ConnectTo(pProcess, 3, pTarget[3], 0))
			return false;
	}
	else
	{
		VstrmFsTarget* pTarget = (VstrmFsTarget*)TargetFactory::Create(TARGET_TYPE_VSTRMFS);
		if(!AddFilter(pTarget))
			return false;

		pTarget->setFilename(strFilename.c_str());
		pTarget->setCacheDirectory(_gCPHCfg.szCacheDir);
		pTarget->enablePacing(true);
		pTarget->enablePacingTrace(_gCPHCfg.enablePacingTrace);
		pTarget->setBandwidth(nMaxBandwidth);
		pTarget->setVstrmBwClientId(_gCPHCfg.vstrmBwClientId);	
		pTarget->disableBufDrvThrottle(_gCPHCfg.vstrmDisableBufDrvThrottle?true:false);		
		pTarget->enableStreamableEvent(_gCPHCfg.enableStreamEvent);
		pTarget->enableProgressEvent(_gCPHCfg.enableProgEvent);
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
	}

	SetMediaSampleSize(_gCPHCfg.mediaSampleSize);

	if (!Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		setErrorInfo(_nLastErrCode, (std::string("Failed to initialize graph with error: ") + _strLastErr).c_str());			
		return false;
	}

	BaseCPHSession::preLoad();

	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RDS, "[%s] preload successful"), _strLogHint.c_str());
	return true;
}

void RDSSess::terminate(bool bProvisionSuccess)
{
	// not started, just constructed
	if (getStatus() == NativeThread::stDeferred)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RDS, "[%s] to terminate() session while in init status"), _strLogHint.c_str());
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

bool RDSSess::getProgress(::Ice::Long& offset, ::Ice::Long& total)
{	
	total = _filesize;
	offset = getProcessBytes();
	return true;
}

int RDSSess::run(void)
{	
	_helper.increaseLoad(_strMethod,_nBandwidth);
	bool bRet = Run();
	_helper.decreaseLoad(_strMethod,_nBandwidth);
	::TianShanIce::Properties params;	
	if (!bRet)
	{
		setErrorInfo(_nLastErrCode, (std::string("Provisioning failed with error: ") + _strLastErr).c_str());			
		Close();
		notifyStopped(true, params);

		MOLOG(Log::L_INFO, CLOGFMT(CPH_RDS, "[%s] notifyStopped() called, status[failure], error[%s]"), 
			_strLogHint.c_str(), _strLastErr.c_str());
		return 0;
	}

	char tmp[64];
	sprintf(tmp, "%lld", _llProcBytes);
	params[EVTPM_TOTOALSIZE] = tmp;

	Close();
	notifyStopped(false, params);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RDS, "[%s] notifyStopped() called, status[success]"), _strLogHint.c_str());
	return 0;
}

void RDSSess::final(int retcode, bool bCancelled)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RDS, "[%s] final() ret=%d, cancelled=%c; calling doCleanup()"), 
		_strLogHint.c_str(),retcode, bCancelled?'Y':'N');

	cleanup();
	try
	{
		delete this;
	}catch(...)
	{
	}
}

bool RDSHelper::validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource)
{
	if (!sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "provision session is 0"));
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RDS, 0, "provision session is 0");
	}
	
	std::string strMethod = sess->methodType;
	if(stricmp(strMethod.c_str(), METHODTYPE_RDSVSVSTRM))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "[%s] unsupported method %s"), sess->ident.name.c_str(),
			strMethod.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RDS, 0, "unsupported method %s", strMethod.c_str());
	}
	
	if (sess->resources.end() == sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "[%s] could not find resource URI"), sess->ident.name.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RDS, 0, "could not find resource URI");
	}
	
	TianShanIce::ValueMap& resURI = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RDS, 0, "could not find URI resource " CPHPM_FILENAME);
	}
	
	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RDS, 0, "could not find URI resource " CPHPM_FILENAME);
	}
	
	TianShanIce::ValueMap& resBw = sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RDS, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}
	
	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RDS, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}
	
	sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stPushTrigger);
	return true;
}

bool RDSSess::prime()
{
	if (!BaseCPHSession::prime())
		return false;

	if (!_pPushSource)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RDS, "[%s] can not find Push Interface"), _strLogHint.c_str());
		setErrorInfo(ERRCODE_PUSHSOURCE_MISSING, ("Provisioning failed with error: can not find Push Interface"));			
		return false;
	}

	PushSource* pSource = (PushSource*)getSourceFilter();
	if (!pSource)
	{
		//should not go here
		setErrorInfo(ERRCODE_PUSHSOURCE_MISSING, ("Provisioning failed with error: not initialized"));			
		return false;
	}

	pSource->setPushSrcI(_pPushSource);

	if (!Start())
	{
		setErrorInfo(_nLastErrCode, (std::string("Failed to start graph with error: ") + _strLastErr).c_str());			
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RDS, "[%s] failed to start graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}

	MOLOG(Log::L_INFO, CLOGFMT(CPH_RDS, "[%s] prime() successful"), _strLogHint.c_str());
	return true;
}

void RDSSess::OnProgress(LONGLONG& prcvBytes)
{
	BaseGraph::OnProgress(prcvBytes);
	::TianShanIce::Properties params;
	updateProgress(_llProcBytes, _llTotalBytes,params);
}

void RDSSess::OnStreamable(bool bStreamable)
{
	BaseGraph::OnStreamable(bStreamable);
	notifyStreamable(bStreamable);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RDS, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void RDSSess::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RDS, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());
	
	char tmp[40];
	::TianShanIce::Properties params;
	sprintf(tmp, "%d", mInfo.bitrate);
	params[EVTPM_MPEGBITRATE] = tmp;
	sprintf(tmp, "%d", mInfo.videoResolutionV);
	params[EVTPM_VIDEOHEIGHT] = tmp;
	sprintf(tmp, "%d", mInfo.videoResolutionH);
	params[EVTPM_VIDEOWIDTH] =  tmp;
	sprintf(tmp, "%d", mInfo.videoBitrate);
	params[EVTPM_VIDEOBITRATE] = tmp;
	
	notifyStarted(params);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RDS, "[%s] notifyStarted() called"), _strLogHint.c_str());
	
	if (_pMainTarget)
	{
		_pMainTarget->setStreamableBytes(mInfo.bitrate*_gCPHCfg.streamReqSecs/8);
	}
}

RDSHelper::RDSHelper(ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr)
:BaseCPHelper(pool, mgr) 
{
		
}

RDSHelper::~RDSHelper()
{

}
/// query the current load information of a method type
///@param[in] methodType to specify the method type to query
///@param[out] allocatedKbps the current allocated bandwidth in Kbps
///@param[out] maxKbps the maximal allowed bandwidth in Kbps, -1 if unlimited
///@param[out] sessions the current running session instances
///@param[out] maxSessins the maximal allowed session instances, -1 if unlimited
///@return true if the query succeeded
bool RDSHelper:: getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins)
{
	if (stricmp(methodType, METHODTYPE_RDSVSVSTRM))
		return false;
	
	CPHRDSCfg::Methods::iterator methodIter;
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

unsigned int RDSHelper::evaluateCost(unsigned int bandwidthKbps, unsigned int sessions)
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
