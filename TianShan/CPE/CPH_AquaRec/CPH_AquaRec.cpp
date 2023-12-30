
#include "CPH_AquaRec.h"
#include "CPH_AquaRecCfg.h"
#include  "AquaRecLeadSess.h"
#include  "AquaRecLeadSessCol.h"
#include "AquaRecVirtualSess.h"
#include "AquaRecVirtualSessFac.h"
#include "ErrorCode.h"
#include "AquaRecWrapper.h"
#include "AquaRecLeadSessColI.h"
#include "../../ContentStore/ContentStore.h"
#include "../../ContentStore/ContentUser.h"
#include "../../ContentStore/ContentSysMD.h"
using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define CPH_AquaRec			"CPH_AquaRec"
#define MOLOG					(glog)

ZQTianShan::ContentProvision::BaseCPHelper* AquaRecHelper::_theHelper =NULL;
AquaRecWrapper*											_AquaRecWrapper = NULL;

extern "C" __EXPORT bool InitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return false;

	// set log handler
	ZQ::common::setGlogger(pEngine->getLogger());
	_gCPHCfg.setLogger(&glog);

	_AquaRecWrapper = new AquaRecWrapper();
	_AquaRecWrapper->setLog(&glog);
	_AquaRecWrapper->setConfigPath( pEngine->getConfigDir());
	_AquaRecWrapper->setThreadPool(&pEngine->getThreadPool());

	if (!_AquaRecWrapper->initialize())
	{
		return false;
	}

	AquaRecHelper* pAquaRecHelper = new AquaRecHelper(pEngine->getThreadPool(), pEngine);
	if (!AquaRecHelper::_theHelper)
		AquaRecHelper::_theHelper = pAquaRecHelper;


	std::vector< ZQ::common::Config::Holder< Method > >::iterator iter = _gCPHCfg.methods.begin();
	if(iter != _gCPHCfg.methods.end())
		pEngine->registerLimitation((*iter).methodName.c_str(), (*iter).maxSession,(*iter).maxBandwidth, GROUP_AQUAREC, 0);

	pEngine->registerHelper(METHODTYPE_AQUAREC, AquaRecHelper::_theHelper, pCtx);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaRec, "AquaRec Helper registered"));

	return true;
}

extern "C" __EXPORT void UninitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return;

	pEngine->unregisterHelper(METHODTYPE_AQUAREC, pCtx);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaRec, "AquaRec Helper unregistered"));


	if (AquaRecHelper::_theHelper)
	{
		try
		{
			delete AquaRecHelper::_theHelper;
		}
		catch(...){};

		AquaRecHelper::_theHelper = NULL;
	}

	//
	//do some module uninitialize
	//	
	if (_AquaRecWrapper)
	{
		_AquaRecWrapper->uninitialize();
		delete _AquaRecWrapper;
		_AquaRecWrapper = NULL;
	}
}

///////////////
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

	path = pathbuf;

	return true;

}
AquaRecSess::~AquaRecSess()
{
	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_AquaRec, "[%s] ~AquaRecSess()"), _strLogHint.c_str());
}

int randomProgressInterval(int nInterval)
{
	srand((unsigned int )SYS::getTickCount());
	return (nInterval/2 + int(nInterval * (rand()/(RAND_MAX+1.0)) / 2)); 
}

bool AquaRecSess::preLoad()
{
	if (!_sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "provision session is 0"));
		setErrorInfo(ERRCODE_NULL_SESSION, "provision session is 0");			
		return false;
	}
	std::string strMethod = _sess->methodType;
	_strMethod = strMethod;
	if(stricmp(strMethod.c_str(), METHODTYPE_AQUAREC))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] unsupported method %s"), _sess->ident.name.c_str(),
			strMethod.c_str());
		setErrorInfo(ERRCODE_UNSUPPORT_METHOD, "unsupported method");			
		return false;
	}

	if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find resource URI"), _sess->ident.name.c_str());
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find resource URI");			
		return false;
	}

	TianShanIce::ValueMap& resURI = _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource " CPHPM_FILENAME);			
		return false;
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource: " CPHPM_FILENAME);			
		return false;
	}

	std::string contentId = var.strs[0];
	
	if (contentId[0] == '/' || contentId[0] == '\\')
	{
		std::string fileName = contentId.substr(1,contentId.length()-1);
		contentId = fileName;
	}
	if (contentId.length() > 0)
	{
		std::string::iterator iter = contentId.end() -1;
		while((*iter) == '/' || (*iter) == '\\' || (*iter) == ' ')
		{
			contentId.erase(iter);
			iter = contentId.end() -1;
		}
	}
	_strLogHint = contentId;
	TianShanIce::ValueMap& resBw = _sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find ProvisionBandwidth resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}

	int nBandwidth = (int)var1.lints[0];
	_nBandwidth = nBandwidth;

	TianShanIce::ValueMap& resMI= _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resMI.end() == resMI.find(CPHPM_SOURCEURL))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find multicast resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_SOURCEURL);			
		return false;
	}

	TianShanIce::Variant& var2 = resMI[CPHPM_SOURCEURL];
	if (var2.type != TianShanIce::vtStrings || var2.bRange|| var2.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_SOURCEURL);			
		return false;
	}

	std::string chName = var2.strs[0];
	if (chName[0] == '/' || chName[0] == '\\')
	{
		std::string fileName = chName.substr(1,chName.length()-1);
		chName = fileName;
	}
	if (chName.length() > 0)
	{
		std::string::iterator iter = chName.end() -1;
		while((*iter) == '/' || (*iter) == '\\' || (*iter) == ' ')
		{
			chName.erase(iter);
			iter = chName.end() -1;
		}
	}
	std::string startTimeUTC, endTimeUTC, providerId, providerAssetId;

	TianShanIce::Properties props = _sess->props;

	/*if(props.end() == props.find(CPHPM_STARTTIME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find properties[CPHPM_STARTTIME]%s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find properties: " CPHPM_STARTTIME);			
		return false;
	}
	startTimeUTC = props[CPHPM_STARTTIME];
	if(props.end() == props.find(CPHPM_ENDTIME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find properties[CPHPM_ENDTIME]%s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find properties: " CPHPM_ENDTIME);			
		return false;
	}
	endTimeUTC = props[CPHPM_ENDTIME];*/
	char buffer[64] = {0};
	ZQ::common::TimeUtil::TimeToUTC(_sess->scheduledStart,buffer,sizeof(buffer) - 1);
	startTimeUTC = buffer;
	ZQ::common::TimeUtil::TimeToUTC(_sess->scheduledEnd,buffer,sizeof(buffer) - 1);
	endTimeUTC = buffer;
	MOLOG (Log::L_DEBUG,CLOGFMT(CPH_AquaRec,"[%s] preLoad SourceChannel [%s],startTime [%s] ,endTime [%s]"),_strLogHint.c_str(),chName.c_str(),startTimeUTC.c_str(),endTimeUTC.c_str());

	if(props.end() == props.find(CPHPM_PROVIDERID))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find properties[CPHPM_PROVIDERID]%s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find properties: " CPHPM_PROVIDERID);			
		return false;
	}
	providerId = props[CPHPM_PROVIDERID];

	if(props.end() == props.find(CPHPM_PROVIDERASSETID))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find properties[CPHPM_PROVIDERASSETID]%s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find properties: " CPHPM_PROVIDERASSETID);			
		return false;
	}
	providerAssetId = props[CPHPM_PROVIDERASSETID];

	std::string sessId = _sess->ident.name;

	AquaRecVirtualSessI*  pVirtualSess = _AquaRecWrapper->generateVirtualSession(chName, contentId, sessId, startTimeUTC, endTimeUTC, _nBandwidth);
	if (!pVirtualSess)
	{
		MOLOG(Log::L_ERROR,CLOGFMT(CPH_AquaRec,"[%s] could not generateVirtualSession on channel [%s]"),_strLogHint.c_str(),chName.c_str());
		setErrorInfo(ERRCODE_AQUAREC_FAILTOGENERATEVIRSESS,"could not generateVirtualSession");
		return false;
	}
	_pAquaRecSess.reset(pVirtualSess);

	BaseAquaRecSession::preLoad();	

	MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaRec, "[%s] preload successful"), _strLogHint.c_str());

	setIdentity(_sess->ident);
	return true;
}

void AquaRecSess::terminate(bool bProvisionSuccess)
{
	if (_sessState == SSTATE_NONE)
	{
		// in this state, do not need to send stop event at here, upper layer will do it
		MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaRec, "[%s] to terminate() session while in init status"), _strLogHint.c_str());
		if (_pAquaRecSess.get() && !_pAquaRecSess->isError())
		{
			_pAquaRecSess->setLastError("User canceled provision", ERRCODE_USER_CANCELED);
		}

		destroy();
	}
	else
	{
		//
		// execute() called
		//
		MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaRec, "to terminate() session with status[%s]"),
			bProvisionSuccess?"success":"failure");
		
		if (!bProvisionSuccess && _pAquaRecSess.get() && !_pAquaRecSess->isError())
		{
			_pAquaRecSess->setLastError("User canceled provision", ERRCODE_USER_CANCELED);
		}

		_sessState = SSTATE_STOP;
		createTimer(0);
	}
}

bool AquaRecSess::getProgress(::Ice::Long& offset, ::Ice::Long& total)
{	
	if (!_pAquaRecSess.get())
		return false;

	_pAquaRecSess->getProgress(offset,total);
	return true;
}

bool AquaRecSess::sendStartEvent()
{
	MediaInfo mInfo;
	if (!_pAquaRecSess->getMediaInfo(mInfo))
	{	
		// not ready
		return false;
	}

	_mediaInfo = mInfo;
	_bitrate = mInfo.bitrate;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaRec, "[%s] getMediaInfo() ready"), _strLogHint.c_str());

	::TianShanIce::Properties params;	

	{
		char tmp[40];
		sprintf(tmp, "%d", mInfo.bitrate);
		params[EVTPM_MPEGBITRATE] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaRec, "[%s] bitrate[%s]"), _strLogHint.c_str(),tmp);
		sprintf(tmp, "%d", mInfo.videoResolutionV);
		params[EVTPM_VIDEOHEIGHT] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaRec, "[%s] videoResolutionV[%s]"), _strLogHint.c_str(), tmp);
		sprintf(tmp, "%d", mInfo.videoResolutionH);
		params[EVTPM_VIDEOWIDTH] =  tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaRec, "[%s] videoResolutionH[%s]"), _strLogHint.c_str(), tmp);
		sprintf(tmp, "%d", mInfo.videoBitrate);
		params[EVTPM_VIDEOBITRATE] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaRec, "[%s] videoBitrate[%s]"), _strLogHint.c_str(), tmp);
		sprintf(tmp,"%.2f",mInfo.framerate);
		params[EVTPM_FRAMERATE] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaRec, "[%s] framerate[%s]"), _strLogHint.c_str(),tmp);
	}

	notifyStarted(params);
	return true;
}

void AquaRecSess::sendProgressEvent()
{
	_pAquaRecSess->getProgress(_processed, _filesize);
	updateProgress(_processed, _filesize);
}

void AquaRecSess::sendStopEvent()
{
	if (!_pAquaRecSess.get())
		return;

	::TianShanIce::Properties params;	

	bool bSessionFail = false;
	AquaRecVirtualSessI::SessionState st = _pAquaRecSess->getState();
	if (st == AquaRecVirtualSessI::StateSuccess)
	{
		bSessionFail = false;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaRec, "[%s] provision status [success]"), _strLogHint.c_str());
	}
	else if (st == AquaRecVirtualSessI::StateFailure)
	{
		std::string strErr;
		int nErrorCode;
	
		bSessionFail = true;
		_pAquaRecSess->getLastError(strErr, nErrorCode);
		setErrorInfo(nErrorCode, (std::string("Provisioning failed with error: ") + strErr).c_str());			

		MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaRec, "[%s] provision status[failure], error[%s]"), 
			_strLogHint.c_str(), strErr.c_str());
	}

	notifyStopped(bSessionFail, params);
	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_AquaRec, "[%s] notifyStopped() called"), _strLogHint.c_str());
}

int randomPreload(int nPreload)
{
	srand((unsigned int) SYS::getTickCount());
	return (nPreload/2 + int(nPreload * (rand()/(RAND_MAX+1.0)))); 
}

bool AquaRecHelper::validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
throw ( ::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource)
{
	if (!sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "provision session is 0"));
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_AquaRec, 0, "provision session is 0");
	}

	std::string strMethod = sess->methodType;
	if(stricmp(strMethod.c_str(), METHODTYPE_AQUAREC))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] unsupported method %s"), sess->ident.name.c_str(),
			strMethod.c_str());
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_AquaRec, 0, "unsupported method %s", strMethod.c_str());
	}

	if (sess->resources.end() == sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find resource URI"), sess->ident.name.c_str());
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_AquaRec, 0, "could not find resource URI");
	}

	TianShanIce::ValueMap& resURI = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_AquaRec, 0, "could not find URI resource " CPHPM_FILENAME);
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_AquaRec, 0, "could not find URI resource " CPHPM_FILENAME);
	}

	std::string strFilename = var.strs[0];

	TianShanIce::ValueMap& resBw = sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_AquaRec, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_AquaRec, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}

	TianShanIce::ValueMap& resMI= sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resMI.end() == resMI.find(CPHPM_SOURCEURL))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find multicast resource: %s"), sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_AquaRec, 0, "could not find multicast resource " CPHPM_SOURCEURL);
	}

	TianShanIce::Variant& var2 = resMI[CPHPM_SOURCEURL];
	if (var2.type != TianShanIce::vtStrings || var2.bRange||  var2.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_AquaRec, "[%s] could not find multicast resource %s"), sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_AquaRec, 0, "could not find multicast resource" CPHPM_SOURCEURL);
	}

	sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduledRestorable);
	sess->preload = randomPreload(_gCPHCfg.preloadTime);

	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_AquaRec, "[%s] set preload=%d ms"), sess->ident.name.c_str(), sess->preload);
	return true;
}

bool AquaRecSess::prime()
{
	if (!BaseAquaRecSession::prime())
		return false;

	if (!_pAquaRecSess.get())
		return false;

	_pAquaRecSess->execute();	
	MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaRec, "[%s] prime() successful"), _strLogHint.c_str());
	return true;
}

bool AquaRecSess::_process()
{
	switch (_sessState)
	{
	case SSTATE_INIT:
		doInit();
		break;
	case SSTATE_PROCESSING:
		doProcessing();
		break;
	case SSTATE_STOP:
		doStop();
		break;
	case SSTATE_DESTROY:
		doDestroy();
		break;
	default:
		;
	}
	
	return true;
}

bool AquaRecSess::process()
{
	Lock sync(*this);

	return _process();
}

bool AquaRecSess::doInit()
{
	do
	{
		if (_sessState != SSTATE_INIT)
			break;

		AquaRecVirtualSessI::SessionState st = _pAquaRecSess->getState();
		if (st != AquaRecVirtualSessI::StateProcessing)
		{
			_sessState = SSTATE_STOP;
			MOLOG(Log::L_DEBUG, CLOGFMT(CPH_AquaRec, "[%s] session state to stop"), _strLogHint.c_str());
			break;
		}	

		if (!sendStartEvent())
			break;

		_sessState = SSTATE_PROCESSING;
	}while(0);

	createTimer(500);
	return true;
}

bool AquaRecSess::doProcessing()
{
	if (_sessState != SSTATE_PROCESSING)
	{
		createTimer(1000);
		return true;
	}

	sendProgressEvent();

	if (!_bStreamable)
	{
		if (checkStreamable())
		{
			sendStreamableEvent();
			_bStreamable = true;
		}
	}

	AquaRecVirtualSessI::SessionState st = _pAquaRecSess->getState();
	if (st != AquaRecVirtualSessI::StateProcessing)
	{
		_sessState = SSTATE_STOP;
		createTimer(0);
		MOLOG(Log::L_DEBUG, CLOGFMT(CPH_AquaRec, "[%s] session state to stop"), _strLogHint.c_str());
	}	
	else
	{
		createTimer(_bStreamable?randomProgressInterval(_gCPHCfg.progressSendInterval):1000);
	}
	return true;
}

bool AquaRecSess::doStop()
{
	if (_sessState == SSTATE_STOP)
	{
		MOLOG(Log::L_DEBUG, CLOGFMT(CPH_AquaRec, "[%s] doStop enter"), _strLogHint.c_str());
		_pAquaRecSess->uninitialize();
		sendStopEvent();
		
		_helper.decreaseLoad(_strMethod, 0);

		_sessState = SSTATE_DESTROY;	
	}

	createTimer(10);
	return true;
}

bool AquaRecSess::doDestroy()
{
	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_AquaRec, "[%s] doDestroy enter"), _strLogHint.c_str());
	removeTimer();
	destroy();
	return true;
}

void AquaRecSess::execute()
{
	_helper.increaseLoad(_strMethod,0);
	_sessState = SSTATE_INIT;
	createTimer(1000);
	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_AquaRec, "[%s] execute() called, session to init state"), _strLogHint.c_str());
}

void AquaRecSess::sendStreamableEvent()
{
	notifyStreamable(true);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_AquaRec, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}
bool AquaRecSess::checkStreamable()
{
	if (!_bitrate)
		return false;

	int64 llStreamableBytes = ((int64)_bitrate) * _gCPHCfg.streamReqSecs / 8;
	if (_processed < llStreamableBytes)
		return false;

	return true;
}
void AquaRecSess::OnTimer()
{
	process();
}


void AquaRecSess::createTimer( int nTimeoutInMs )
{
	_helper.createTimer(this, nTimeoutInMs);
}

void AquaRecSess::removeTimer()
{
	_helper.removeTimer(this);
}
void AquaRecSess::updateScheduledTime(const ::std::string& startTimeUTC, const ::std::string& endTimeUTC)
{
	//MOLOG(Log::L_DEBUG,CLOGFMT(CPH_AquaRec,"[%s] updateScheduledTime startTime [%s] endTime [%s]"),_strLogHint.c_str(),startTimeUTC.c_str(),endTimeUTC.c_str());
	if (!_pAquaRecSess.get())
		return;
	_pAquaRecSess.get()->updateScheduledTime(startTimeUTC, endTimeUTC);
}

/// query the current load information of a method type
///@param[in] methodType to specify the method type to query
///@param[out] allocatedKbps the current allocated bandwidth in Kbps
///@param[out] maxKbps the maximal allowed bandwidth in Kbps, -1 if unlimited
///@param[out] sessions the current running session instances
///@param[out] maxSessins the maximal allowed session instances, -1 if unlimited
///@return true if the query succeeded
bool AquaRecHelper:: getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins)
{
	if (stricmp(methodType, METHODTYPE_AQUAREC))
		return false;

	AquaRecConfig::Methods::iterator methodIter;
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

unsigned int AquaRecHelper::evaluateCost(unsigned int bandwidthKbps, unsigned int sessions)
{
	static unsigned int maxsessionNum = _gCPHCfg.methods[0].maxSession;
	static unsigned int maxBandwidthKBps = _gCPHCfg.methods[0].maxBandwidth;

	if (sessions >maxsessionNum)
		return MAX_LOAD_VALUE + 1;

//	if(bandwidthKbps > maxBandwidthKBps)	
//		return MAX_LOAD_VALUE + 1;

//	int nCost1 = (((float)bandwidthKbps)/maxBandwidthKBps)*MAX_LOAD_VALUE; 
	int nCost2 = (int) (((float)sessions)/maxsessionNum)*MAX_LOAD_VALUE; 

//	return __max(nCost1, nCost2);
	return nCost2;
}

AquaRecHelper::~AquaRecHelper()
{
}


AquaRecHelper::AquaRecHelper( ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr )
:BaseCPHelper(pool, mgr)
{
}
bool AquaRecHelper::getBandwidthLoad( const char* methodType, long& bpsAllocated, long& bpsMax, long& initCost )
{
	if (stricmp(methodType, METHODTYPE_AQUAREC))
		return false;

	bpsAllocated =0;

	AquaRecConfig::Methods::iterator methodIter;
	for (methodIter = _gCPHCfg.methods.begin();methodIter != _gCPHCfg.methods.end();methodIter++)
	{
		if (stricmp((*methodIter).methodName.c_str(), methodType) == 0)
			break;
	}
	if (methodIter == _gCPHCfg.methods.end())
		return false;

	bpsMax=(*methodIter).maxBandwidth;
	initCost =0;
	return true;
}
