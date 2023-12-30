
#include "BaseClass.h"
#include "CPH_FSCP.h"
#include "NTFSTarget.h"
#include "BaseClass.h"
#include "VstrmFilesetTarget.h"
#include "CPH_FSCPCfg.h"
#include "RTFProc.h"
#include "NTFSFileSetSource .h"


using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define CPH_FSCP			"CPH_FSCP"
#define MOLOG					(glog)

#define FSCP_PROVISION_ERRCODE		1

ZQTianShan::ContentProvision::BaseCPHelper* FSCPHelper::_theHelper =NULL;

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
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFilePath().c_str());		
		return FALSE;
	}
	else
	{
 		MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "Load configuration from [%s] successfully"),
 			_gCPHCfg.getConfigFilePath().c_str());
		_gCPHCfg.snmpRegister("");
	}	

	//
	// do some module initialize
	//

	std::string errmsg;
	if (!VstrmFsTarget::initVstrm(_gCPHCfg.vstrmBwClientId, errmsg))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "Failed to init vstrm class with error %s"),errmsg.c_str());
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "initVstrm() successful"));
	}

	if (!FSCPHelper::_theHelper)
		FSCPHelper::_theHelper = new FSCPHelper(pEngine->getThreadPool(), pEngine);

	pEngine->registerHelper(METHODTYPE_FSCOPYVSVSTRM, FSCPHelper::_theHelper, pCtx);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "FSCP Helper registered"));

	return TRUE;
}

extern "C" __declspec(dllexport) void UninitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return;
	
	pEngine->unregisterHelper(METHODTYPE_FSCOPYVSVSTRM, pCtx);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "FSCP Helper unregistered"));

	if (FSCPHelper::_theHelper)
	{
		try
		{
			delete FSCPHelper::_theHelper;
		}
		catch(...){};
		
		FSCPHelper::_theHelper = NULL;
	}
	
	//
	//do some module uninitialize
	//
	VstrmFsTarget::uninitVstrm();	

	if (pEngine->getLogger() == ZQ::common::getGlogger())
		ZQ::common::setGlogger(NULL);
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

FSCPSess::~FSCPSess()
{
	cleanup();
}

void FSCPSess::cleanup()
{
	if (_bCleaned)
		return;

	BaseGraph::Close();
	_bCleaned = true;
}


bool FSCPSess::preLoad()
{
	if (!_sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "provision session is 0"));
		setErrorInfo(FSCP_PROVISION_ERRCODE, "provision session is 0");			
		return false;
	}

	std::string strMethod = _sess->methodType;
	if(stricmp(strMethod.c_str(), METHODTYPE_FSCOPYVSVSTRM))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "[%s] unsupported method %s"), _sess->ident.name.c_str(),
			strMethod.c_str());
		setErrorInfo(FSCP_PROVISION_ERRCODE, "unsupported method");			
		return false;
	}

	if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "[%s] could not find resource URI"), _sess->ident.name.c_str());
		setErrorInfo(FSCP_PROVISION_ERRCODE, "could not find resource URI");			
		return false;
	}

	TianShanIce::ValueMap& resURI = _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(FSCP_PROVISION_ERRCODE, "could not find URI resource " CPHPM_FILENAME);			
		return false;
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(FSCP_PROVISION_ERRCODE, "could not find URI resource: " CPHPM_FILENAME);			
		return false;
	}

	std::string strFilename = var.strs[0];

	TianShanIce::ValueMap& resBw = _sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "[%s] could not find ProvisionBandwidth resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(FSCP_PROVISION_ERRCODE, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(FSCP_PROVISION_ERRCODE, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}
	int nBandwidth = (int)var1.lints[0];

	SetLog(_helper._pCPELogger);
	SetMemAlloc(this);
	SetLogHint(strFilename.c_str());

	int nMaxBandwidth;
 	if (_gCPHCfg.bandwidthLimitRate)
 	{
 		nMaxBandwidth = int(((float)nBandwidth) * _gCPHCfg.bandwidthLimitRate / 100);
 	}
 	else
		nMaxBandwidth = nBandwidth;
	
 	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_FSCP, "[%s] bandwidth[%d], max_limit_rate[%d], maxbandwidth[%d]"), _strLogHint.c_str(), 
 		nBandwidth, _gCPHCfg.bandwidthLimitRate, nMaxBandwidth);

	fixpath(strFilename);
	std::string sourcedir = strFilename.substr(0, strFilename.find_last_of('\\'));
	strFilename = strFilename.substr(strFilename.find_last_of('\\')+1, strFilename.size()-strFilename.find_last_of('\\')-1);
    if (sourcedir.compare("") == 0)
    {
		MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] Failed to get source directory."), _strLogHint.c_str());
		setErrorInfo(_nLastErrCode, "Failed to get source directory.");
		return false;
    }

	std::string cachdir = _gCPHCfg.szCacheDir;
	if (cachdir.compare("") == 0)
    {
		MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] Can't read out Cache directory from xml."), _strLogHint.c_str());
		setErrorInfo(_nLastErrCode, "Can't read out Cache directory from xml.");
		return false;
    }

	NTFSFileSetSource* pSource =  (NTFSFileSetSource*)SourceFactory::Create(SOURCE_TYPE_NTFS_FILESET);
	if (pSource== NULL)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] Creating source object failed."), _strLogHint.c_str());
		return false;
	}
	AddFilter(pSource);
	pSource->setFilename(strFilename.c_str());
	pSource->setSourceDirectory(sourcedir.c_str());
	pSource->setCachDir(cachdir.c_str());
	pSource->setWaitTime( _gCPHCfg.waittime);
	pSource->setMaxBandwidth(nMaxBandwidth);
	
	
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
		
		

		if (!ConnectTo(pSource, 0, pTarget[0], 0))
			return false;
		if (!ConnectTo(pSource, 1, pTarget[1], 0))
			return false;
		if (!ConnectTo(pSource, 2, pTarget[2], 0))
			return false;
		if (!ConnectTo(pSource, 3, pTarget[3], 0))
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
		pTarget->enableStreamableEvent(_gCPHCfg.enableStreamEvent);
		pTarget->enableProgressEvent(_gCPHCfg.enableProgEvent);
		_pMainTarget = pTarget;
		
		if (!ConnectTo(pSource, 0, pTarget, 0))
			return false;
		if (!ConnectTo(pSource, 1, pTarget, 0))
			return false;
		if (!ConnectTo(pSource, 2, pTarget, 0))
			return false;
		if (!ConnectTo(pSource, 3, pTarget, 0))
			return false;
	}

	SetMediaSampleSize(_gCPHCfg.mediaSampleSize);

	if (!Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		setErrorInfo(FSCP_PROVISION_ERRCODE, (std::string("Failed to initialize graph with error: ") + _strLastErr).c_str());			
		return false;
	}

	BaseCPHSession::preLoad();

	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] preload successful"), _strLogHint.c_str());
	return true;
}

void FSCPSess::terminate(bool cleanupIncompletedOutput)
{
	// not started, just constructed
	if (getStatus() == NativeThread::stDeferred)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] to terminate() session while in init status"), _strLogHint.c_str());
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
		MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] to terminate() session"), _strLogHint.c_str());
		_bQuit = true;
		BaseGraph::Stop();
	}
}

bool FSCPSess::getProgress(::Ice::Long& offset, ::Ice::Long& total)
{	
	total = _filesize;
	offset = getProcessBytes();
	return true;
}

int FSCPSess::run(void)
{	
	bool bRet = Run();
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
		MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] notifyStopped() called"), _strLogHint.c_str());
		return 0;
	}

	LONGLONG supportFileSize=0;
	std::string md5;
	if (_gCPHCfg.enableTestNTFS)
	{
		VstrmFsTarget* pTarget = (VstrmFsTarget*)_pMainTarget;
		pTarget->getSupportFileSize(supportFileSize);
		pTarget->getMD5(md5);
	}

	char tmp[64];
	sprintf(tmp, "%lld", _llProcBytes);
	params[EVTPM_TOTOALSIZE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] Filesize-%s"), _strLogHint.c_str(), tmp);

	sprintf(tmp, "%lld", supportFileSize);
	params[EVTPM_SUPPORTFILESIZE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] SupportFilesize-%s"), _strLogHint.c_str(), tmp);

	params[EVTPM_MD5CHECKSUM] = md5;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] md5-%s"), _strLogHint.c_str(), md5.c_str());

	if (_bitrate)
	{
		sprintf(tmp, "%lld", _llProcBytes*8000/_bitrate);
		params[EVTPM_PLAYTIME] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] playtime-%s"), _strLogHint.c_str(), tmp);
	}
	else {
		MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] bitrate - 0 , can't get playtime"), _strLogHint.c_str());
	}


	Close();
	if (!_bStartEventSent)
	{
		notifyStarted(params);
	}
	notifyStopped(false, params);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] notifyStopped() called"), _strLogHint.c_str());
	return 0;
}

void FSCPSess::final(int retcode, bool bCancelled)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] final() ret=%d, cancelled=%c; calling doCleanup()"), 
		_strLogHint.c_str(),retcode, bCancelled?'Y':'N');
	
	cleanup();
	try
	{
		delete this;
	}catch(...)
	{
	}
}


bool FSCPHelper::validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource)
{
	if (!sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "provision session is 0"));
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_FSCP, 0, "provision session is 0");
	}
	
	std::string strMethod = sess->methodType;
	if(stricmp(strMethod.c_str(), METHODTYPE_FSCOPYVSVSTRM))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "[%s] unsupported method %s"), sess->ident.name.c_str(),
			strMethod.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_FSCP, 0, "unsupported method %s", strMethod.c_str());
	}
	
	if (sess->resources.end() == sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "[%s] could not find resource URI"), sess->ident.name.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_FSCP, 0, "could not find resource URI");
	}
	
	TianShanIce::ValueMap& resURI = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_FSCP, 0, "could not find URI resource " CPHPM_FILENAME);
	}
	
	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_FSCP, 0, "could not find URI resource " CPHPM_FILENAME);
	}
	
	TianShanIce::ValueMap& resBw = sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_FSCP, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}
	
	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_FSCP, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_FSCP, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}
	
	sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduledRestorable);
	return true;
}

bool FSCPSess::prime()
{
	if (!BaseCPHSession::prime())
		return false;

	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] prime() successful"), _strLogHint.c_str());
	return true;
}

void FSCPSess::OnProgress(LONGLONG& prcvBytes)
{
	BaseGraph::OnProgress(prcvBytes);
	updateProgress(_llProcBytes, _llTotalBytes);
}

void FSCPSess::OnStreamable(bool bStreamable)
{
	BaseGraph::OnStreamable(bStreamable);
	notifyStreamable(bStreamable);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void FSCPSess::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());

	char tmp[40];
	::TianShanIce::Properties params;
	sprintf(tmp, "%d", mInfo.bitrate);
	params[EVTPM_MPEGBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] bitrate-%s"), _strLogHint.c_str(),tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionV);
	params[EVTPM_VIDEOHEIGHT] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] videoResolutionV-%s"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionH);
	params[EVTPM_VIDEOWIDTH] =  tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] videoResolutionH-%s"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoBitrate);
	params[EVTPM_VIDEOBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] videoBitrate-%s"), _strLogHint.c_str(), tmp);
	sprintf(tmp,"%.2f",mInfo.framerate);
	params[EVTPM_FRAMERATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] framerate-%s"), _strLogHint.c_str(),tmp);

	_bitrate = mInfo.bitrate;
	
	notifyStarted(params);
	_bStartEventSent = true;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_FSCP, "[%s] notifyStarted() called"), _strLogHint.c_str());

	if (_pMainTarget)
	{
		_pMainTarget->setStreamableBytes(mInfo.bitrate*_gCPHCfg.streamReqSecs/8);
	}
}

bool FSCPHelper::getBandwidthLoad(const char* methodType, long& bpsAllocated, long& bpsMax, long& initCost)
{
	if (NULL == methodType || 0 != stricmp(METHODTYPE_RTFRDSVSVSTRM, methodType))
		return false;

	bpsAllocated =0;
	bpsMax=_gCPHCfg.maxBandwidthBps;
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
bool FSCPHelper:: getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins)
{
	if (stricmp(methodType, METHODTYPE_RTIVSNAS))
		return false;

	maxKbps = _gCPHCfg.maxBandwidthBps/1024;
	maxSessins = _gCPHCfg.maxSessionNum;

	{
		ZQ::common::Guard<ZQ::common::Mutex> op(_lockSessMap);
		sessions = _sessMap.size();
	}	

	allocatedKbps = 0;
	return true;
}

/// evaluate the cost per given session count and total allocated bandwidth
///@param[in] methodType to specify the method type to query
///@param[in] bandwidthKbps to specify the allocated bandwidth in Kbps
///@param[in] sessions to specify the allocated session instances
///@return a cost in the range of [0, MAX_LOAD_VALUE] at the given load level:
///		0 - fully available
///		MAX_LOAD_VALUE - completely unavailable
uint16 FSCPHelper::evaluateCost(const char* methodType, const uint32 bandwidthKbps, const uint16 sessions)
{
	if (sessions > _gCPHCfg.maxSessionNum)
		return MAX_LOAD_VALUE;

	unsigned int maxBandwidthKBps = _gCPHCfg.maxBandwidthBps/1024;
	if (bandwidthKbps > maxBandwidthKBps)	
		return MAX_LOAD_VALUE;

	return uint16((((float)bandwidthKbps)/maxBandwidthKBps)*MAX_LOAD_VALUE);
}
