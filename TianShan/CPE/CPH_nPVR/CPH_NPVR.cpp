
#include "CPH_NPVR.h"
#include "CPH_NPVRCfg.h"
#include  "LeadSess.h"
#include  "LeadSessCol.h"
#include "VirtualSess.h"
#include "VirtualSessFac.h"
#include "RTFProc.h"
#include "HostToIP.h"
#include "McastCapSrc.h"
#include "ErrorCode.h"
#include "NtfsFileIoFactory.h"
#include "NPVRTargetFactory.h"
#include "NPVRWrapper.h"
#include "LeadSessColI.h"
#include "..\..\ContentStore\ContentStore.h"
#include "..\..\ContentStore\ContentUser.h"
#include "..\..\ContentStore\ContentSysMD.h"



using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define CPH_NPVR			"CPH_NPVR"
#define MOLOG					(glog)


ZQTianShan::ContentProvision::BaseCPHelper* NPVRHelper::_theHelper =NULL;
NPVRWrapper*											_nPVRWrapper = NULL;
NetworkIFSelector*                                      _nNetSelector = NULL;

extern "C" __declspec(dllexport) BOOL InitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return FALSE;

	// set log handler
	ZQ::common::setGlogger(pEngine->getLogger());
	_gCPHCfg.setLogger(&glog);

	_nPVRWrapper = new NPVRWrapper();
	_nPVRWrapper->setLog(&glog);
	_nPVRWrapper->setMemoryAllocator(pEngine->getMemoryAllocator());
	_nPVRWrapper->setConfigPath( pEngine->getConfigDir());
	_nPVRWrapper->setThreadPool(&pEngine->getThreadPool());

	if (!_nPVRWrapper->initialize())
	{
		return false;
	}

	NPVRHelper* pNPVRHelper = new NPVRHelper(pEngine->getThreadPool(), pEngine);
	if (!NPVRHelper::_theHelper)
		NPVRHelper::_theHelper = pNPVRHelper;

    _nNetSelector = new NetworkIFSelector(glog);
	if(!_nNetSelector)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "Failed to new NetIfSelector object."));
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
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "failed to load config of NetworkInterface for Capture, please check CPH_NPVR.xml"));
		return false;
	}

	std::vector< ZQ::common::Config::Holder< Method > >::iterator iter = _gCPHCfg.methods.begin();
	if(iter != _gCPHCfg.methods.end())
		pEngine->registerLimitation((*iter).methodName.c_str(), (*iter).maxSession,(*iter).maxBandwidth, GROUP_NPVR, 0);

	pEngine->registerHelper(METHODTYPE_NPVRVSVSTRM, NPVRHelper::_theHelper, pCtx);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "NPVR Helper registered"));

	return TRUE;
}

extern "C" __declspec(dllexport) void UninitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return;

	pEngine->unregisterHelper(METHODTYPE_NPVRVSVSTRM, pCtx);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "NPVR Helper unregistered"));


	if (NPVRHelper::_theHelper)
	{
		try
		{
			delete NPVRHelper::_theHelper;
		}
		catch(...){};

		NPVRHelper::_theHelper = NULL;
	}

	//
	//do some module uninitialize
	//
	if (pEngine->getLogger() == ZQ::common::getGlogger())
		ZQ::common::setGlogger(NULL);

	if (_nPVRWrapper)
	{
		_nPVRWrapper->uninitialize();
		delete _nPVRWrapper;
	}
	_nPVRWrapper = NULL;

	if (_nNetSelector)
		delete _nNetSelector;
	_nNetSelector = NULL;
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

NPVRSess::~NPVRSess()
{
	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_NPVR, "[%s] ~NPVRSess()"), _strLogHint.c_str());
}

int randomProgressInterval(int nInterval)
{
	srand(GetTickCount());
	return (nInterval/2 + int(nInterval * (rand()/(RAND_MAX+1.0)) / 2)); 
}

bool NPVRSess::preLoad()
{
	std::string multicastIp;
	bool bH264Type = FALSE;

	if (!_sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "provision session is 0"));
		setErrorInfo(ERRCODE_NULL_SESSION, "provision session is 0");			
		return false;
	}

	std::string strMethod = _sess->methodType;
	_strMethod = strMethod;
	if(stricmp(strMethod.c_str(), METHODTYPE_NPVRVSVSTRM))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] unsupported method %s"), _sess->ident.name.c_str(),
			strMethod.c_str());
		setErrorInfo(ERRCODE_UNSUPPORT_METHOD, "unsupported method");			
		return false;
	}

	if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] could not find resource URI"), _sess->ident.name.c_str());
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find resource URI");			
		return false;
	}

	TianShanIce::ValueMap& resURI = _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource " CPHPM_FILENAME);			
		return false;
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource: " CPHPM_FILENAME);			
		return false;
	}

	std::string strFilename = var.strs[0];

	TianShanIce::ValueMap& resBw = _sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] could not find ProvisionBandwidth resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}


	int nBandwidth = (int)var1.lints[0];
	_nBandwidth = nBandwidth;

	TianShanIce::ValueMap& resMI= _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resMI.end() == resMI.find(CPHPM_SOURCEURL))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] could not find multicast resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_SOURCEURL);			
		return false;
	}

	TianShanIce::Variant& var2 = resMI[CPHPM_SOURCEURL];
	if (var2.type != TianShanIce::vtStrings || var2.bRange|| var2.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_SOURCEURL);			
		return false;
	}
	std::string multicastUrl = var2.strs[0];

	multicastIp = multicastUrl.substr(multicastUrl.find_first_of(':')+3,multicastUrl.find_last_of(':')-6);
	std::string strmulticastPort = multicastUrl.substr(multicastUrl.find_last_of(':')+1,multicastUrl.size()-multicastUrl.find_last_of(':')-1);
	int multicastPort = atoi(strmulticastPort.c_str());
	
	if (strFilename[0] == '/' || strFilename[0] == '\\')
	{
		std::string fileName = strFilename.substr(1,strFilename.size()-1);
		strFilename = fileName;
	}
	_strLogHint = strFilename;

	VirtualSessI*  pVirtualSess = _nPVRWrapper->generateVirtualSession(nBandwidth, strFilename, multicastIp, multicastPort, bH264Type);
	if (!pVirtualSess)
	{
		return false;
	}
	_pNPVRSess.reset(pVirtualSess);

	if (!setContentProperty())
		return false;

	BaseNPVRSession::preLoad();	

	MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] preload successful"), _strLogHint.c_str());

	setIdentity(_sess->ident);
	return true;
}

void NPVRSess::terminate(bool bProvisionSuccess)
{
	if (_sessState == SSTATE_NONE)
	{
		// in this state, do not need to send stop event at here, upper layer will do it
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] to terminate() session while in init status"), _strLogHint.c_str());
		if (_pNPVRSess.get() && !_pNPVRSess->isError())
		{
			_pNPVRSess->setLastError("User canceled provision", ERRCODE_USER_CANCELED);
		}

		destroy();
	}
	else
	{
		//
		// execute() called
		//
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "to terminate() session with status[%s]"),
			bProvisionSuccess?"success":"failure");
		
		if (!bProvisionSuccess && _pNPVRSess.get() && !_pNPVRSess->isError())
		{
			_pNPVRSess->setLastError("User canceled provision", ERRCODE_USER_CANCELED);
		}

		_sessState = SSTATE_STOP;
		createTimer(0);
	}
}

bool NPVRSess::getProgress(::Ice::Long& offset, ::Ice::Long& total)
{	
	if (!_pNPVRSess.get())
		return false;

	_pNPVRSess->getProgress(offset,total);
	return true;
}

bool NPVRSess::sendStartEvent()
{
	MediaInfo mInfo;
	if (!_pNPVRSess->getMediaInfo(mInfo))
	{	
		// not ready
		return false;
	}

	_mediaInfo = mInfo;
	_bitrate = mInfo.bitrate;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] getMediaInfo() ready"), _strLogHint.c_str());

	::TianShanIce::Properties params;	

	{
		char tmp[40];
		sprintf(tmp, "%d", mInfo.bitrate);
		params[EVTPM_MPEGBITRATE] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] bitrate[%s]"), _strLogHint.c_str(),tmp);
		sprintf(tmp, "%d", mInfo.videoResolutionV);
		params[EVTPM_VIDEOHEIGHT] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] videoResolutionV[%s]"), _strLogHint.c_str(), tmp);
		sprintf(tmp, "%d", mInfo.videoResolutionH);
		params[EVTPM_VIDEOWIDTH] =  tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] videoResolutionH[%s]"), _strLogHint.c_str(), tmp);
		sprintf(tmp, "%d", mInfo.videoBitrate);
		params[EVTPM_VIDEOBITRATE] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] videoBitrate[%s]"), _strLogHint.c_str(), tmp);
		sprintf(tmp,"%.2f",mInfo.framerate);
		params[EVTPM_FRAMERATE] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] framerate[%s]"), _strLogHint.c_str(),tmp);
	}

	notifyStarted(params);
	return true;
}

void NPVRSess::sendProgressEvent()
{
	_pNPVRSess->getProgress(_processed, _filesize);
	updateProgress(_processed, _filesize);
}

void NPVRSess::sendStopEvent()
{
	if (!_pNPVRSess.get())
		return;

	::TianShanIce::Properties params;	

	bool bSessionFail;
	VirtualSessI::SessionState st = _pNPVRSess->getState();
	if (st == VirtualSessI::StateSuccess)
	{
		bSessionFail = false;
		
		LONGLONG llProgress = 0;
		LONGLONG llTotal = 0;
		LONGLONG supportFileSize = 0;
		std::string md5;
		int nPlayTime = 0;

		// get data
		{
			_pNPVRSess->getProgress(llProgress, llTotal);
			supportFileSize = _pNPVRSess->getSupportFileSize();
			md5 = _pNPVRSess->getMD5Sum();

			if (_mediaInfo.bitrate)
			{
				nPlayTime = int(llProgress*8000/_mediaInfo.bitrate);
			}
		}

		// prepare event parameters
		{
			char tmp[64];
			sprintf(tmp, "%lld", llProgress);
			params[EVTPM_TOTOALSIZE] = tmp;
			MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] Filesize[%s]"), _strLogHint.c_str(), tmp);

			sprintf(tmp, "%lld", supportFileSize);
			params[EVTPM_SUPPORTFILESIZE] = tmp;
			MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] SupportFilesize[%s]"), _strLogHint.c_str(), tmp);

			params[EVTPM_MD5CHECKSUM] = md5;
			MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] md5[%s]"), _strLogHint.c_str(), md5.c_str());

			sprintf(tmp, "%d", nPlayTime);
			params[EVTPM_PLAYTIME] = tmp;
			MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] playtime[%s]"), _strLogHint.c_str(), tmp);
		}

		MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] provision status [success]"), _strLogHint.c_str());
	}
	else if (st == VirtualSessI::StateFailure)
	{
		std::string strErr;
		int nErrorCode;
	
		bSessionFail = true;
		_pNPVRSess->getLastError(strErr, nErrorCode);
		setErrorInfo(nErrorCode, (std::string("Provisioning failed with error: ") + strErr).c_str());			

		MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] provision status[failure], error[%s]"), 
			_strLogHint.c_str(), strErr.c_str());
	}

	notifyStopped(bSessionFail, params);
	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_NPVR, "[%s] notifyStopped() called"), _strLogHint.c_str());
}

int randomPreload(int nPreload)
{
	srand(GetTickCount());
	return (nPreload/2 + int(nPreload * (rand()/(RAND_MAX+1.0)))); 
}

bool NPVRHelper::validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource)
{
	if (!sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "provision session is 0"));
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NPVR, 0, "provision session is 0");
	}

	std::string strMethod = sess->methodType;
	if(stricmp(strMethod.c_str(), METHODTYPE_NPVRVSVSTRM))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] unsupported method %s"), sess->ident.name.c_str(),
			strMethod.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NPVR, 0, "unsupported method %s", strMethod.c_str());
	}

	if (sess->resources.end() == sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] could not find resource URI"), sess->ident.name.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NPVR, 0, "could not find resource URI");
	}

	TianShanIce::ValueMap& resURI = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NPVR, 0, "could not find URI resource " CPHPM_FILENAME);
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NPVR, 0, "could not find URI resource " CPHPM_FILENAME);
	}

	std::string strFilename = var.strs[0];

	TianShanIce::ValueMap& resBw = sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NPVR, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NPVR, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}

	TianShanIce::ValueMap& resMI= sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resMI.end() == resMI.find(CPHPM_SOURCEURL))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] could not find multicast resource: %s"), sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NPVR, 0, "could not find multicast resource " CPHPM_SOURCEURL);
	}

	TianShanIce::Variant& var2 = resMI[CPHPM_SOURCEURL];
	if (var2.type != TianShanIce::vtStrings || var2.bRange||  var2.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] could not find multicast resource %s"), sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NPVR, 0, "could not find multicast resource" CPHPM_SOURCEURL);
	}
	std::string mulicastUrl = var2.strs[0];
	std::string strpro = mulicastUrl.substr(0,6);
	if (_stricmp(strpro.c_str(),"udp://")!= 0)
		//if (strstr(mulicastUrl.c_str(),"udp://")==NULL)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] Can't find the udp protocol from url %s"), sess->ident.name.c_str(), mulicastUrl.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NPVR, TianShanIce::Storage::csexpInvalidSourceURL, "Can't find the udp header from url" ,mulicastUrl.c_str());
	}

	if (!validateContentNPVRProperty(sess))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] Can't find content property [ProviderId/ProviderAssetId/SubscriberId]"), sess->ident.name.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NPVR, TianShanIce::Storage::csexpInvalidSourceURL, "Can't find content property [ProviderId/ProviderAssetId/SubscriberId]");
	}

	sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	sess->preload = randomPreload(_gCPHCfg.preloadTime);

	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_NPVR, "[%s] set preload=%d ms"), sess->ident.name.c_str(), sess->preload);
	return true;
}

bool NPVRSess::prime()
{
	if (!BaseNPVRSession::prime())
		return false;

	if (!_pNPVRSess.get())
		return false;

	_pNPVRSess->execute();	
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] prime() successful"), _strLogHint.c_str());
	return true;
}

/// set the NPVR content property 
///@return	if content proxy is empty, return true; if not empty and set property successful, then return true; else return false
bool NPVRSess::setContentProperty()
{
	if (!_pNPVRSess.get())
		return false;

	// get a property for NPVR session
	std::string strLeadSessPath = _pNPVRSess->getIndexPathName();

	return setContentPropertyLeadCopy(strLeadSessPath);
}

bool NPVRSess::setContentPropertyLeadCopy( const std::string& strLeadCopy)
{
	if (!_sess->primaryContent)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] no content proxy, skip setting the nPVR content property"), _strLogHint.c_str());
		return true;
	}

	MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] nPVR lead session copy [%s]"), 
		_strLogHint.c_str(), strLeadCopy.c_str());

	::TianShanIce::Storage::UnivContentPrx uniContent = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(_sess->primaryContent);
	TianShanIce::Properties metaData;
	metaData[METADATA_nPVRLeadCopy] = strLeadCopy;
	try
	{
		uniContent->setMetaData(metaData);
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] content metadata [nPVRLeadCopy] set successful"), _strLogHint.c_str());
		return true;
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] failed to set content  metadata [nPVRLeadCopy], caught exception[ObjectNotExistException]"), _strLogHint.c_str());
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] failed to set content  metadata [nPVRLeadCopy], caught exception[%s]: %s"), _strLogHint.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] failed to set content  metadata [nPVRLeadCopy], caught exception[%s]"),  _strLogHint.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] failed to set content  metadata [nPVRLeadCopy], caught unknown exception"), _strLogHint.c_str());
	}

	return false;
}

bool NPVRSess::_process()
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

bool NPVRSess::process()
{
	Lock sync(*this);

	return _process();
}

bool NPVRSess::doInit()
{
	do
	{
		if (_sessState != SSTATE_INIT)
			break;

		VirtualSessI::SessionState st = _pNPVRSess->getState();
		if (st != VirtualSessI::StateProcessing)
		{
			_sessState = SSTATE_STOP;
			MOLOG(Log::L_DEBUG, CLOGFMT(CPH_NPVR, "[%s] session state to stop"), _strLogHint.c_str());
			break;
		}	

		if (!sendStartEvent())
			break;

		_sessState = SSTATE_PROCESSING;
	}while(0);

	createTimer(500);
	return true;
}

bool NPVRSess::doProcessing()
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

	VirtualSessI::SessionState st = _pNPVRSess->getState();
	if (st != VirtualSessI::StateProcessing)
	{
		_sessState = SSTATE_STOP;
		createTimer(0);
		MOLOG(Log::L_DEBUG, CLOGFMT(CPH_NPVR, "[%s] session state to stop"), _strLogHint.c_str());
	}	
	else
	{
		createTimer(_bStreamable?randomProgressInterval(_gCPHCfg.progressSendInterval):1000);
	}
	return true;
}

bool NPVRSess::doStop()
{
	if (_sessState == SSTATE_STOP)
	{
		MOLOG(Log::L_DEBUG, CLOGFMT(CPH_NPVR, "[%s] doStop enter"), _strLogHint.c_str());
		_pNPVRSess->uninitialize();
		sendStopEvent();
		
		_helper.decreaseLoad(_strMethod, _nBandwidth);

		_sessState = SSTATE_DESTROY;	
	}

	createTimer(10);
	return true;
}

bool NPVRSess::doDestroy()
{
	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_NPVR, "[%s] doDestroy enter"), _strLogHint.c_str());
	removeTimer();
	destroy();
	return true;
}

void NPVRSess::execute()
{
	_helper.increaseLoad(_strMethod,_nBandwidth);
	_sessState = SSTATE_INIT;
	createTimer(1000);
	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_NPVR, "[%s] execute() called, session to init state"), _strLogHint.c_str());
}

void NPVRSess::sendStreamableEvent()
{
	notifyStreamable(true);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

bool NPVRSess::checkStreamable()
{
	if (!_bitrate)
		return false;

	LONGLONG llStreamableBytes = ((LONGLONG)_bitrate) * _gCPHCfg.streamReqSecs / 8;
	if (_processed < llStreamableBytes)
		return false;

	return true;
}

void NPVRSess::OnTimer()
{
	process();
}

int NPVRSess::getTimeoutMilisecond()
{
	return 0;
}

void NPVRSess::createTimer( int nTimeoutInMs )
{
	_helper.createTimer(this, nTimeoutInMs);
}

void NPVRSess::removeTimer()
{
	_helper.removeTimer(this);
}


/// query the current load information of a method type
///@param[in] methodType to specify the method type to query
///@param[out] allocatedKbps the current allocated bandwidth in Kbps
///@param[out] maxKbps the maximal allowed bandwidth in Kbps, -1 if unlimited
///@param[out] sessions the current running session instances
///@param[out] maxSessins the maximal allowed session instances, -1 if unlimited
///@return true if the query succeeded
bool NPVRHelper:: getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins)
{
	if (stricmp(methodType, METHODTYPE_NPVRVSVSTRM))
		return false;

	NPVRConfig::Methods::iterator methodIter;
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

unsigned int NPVRHelper::evaluateCost(unsigned int bandwidthKbps, unsigned int sessions)
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

NPVRHelper::~NPVRHelper()
{
}

bool NPVRHelper::getBandwidthLoad( const char* methodType, long& bpsAllocated, long& bpsMax, long& initCost )
{
	if (stricmp(methodType, METHODTYPE_NPVRVSVSTRM))
		return false;

	bpsAllocated =0;

	NPVRConfig::Methods::iterator methodIter;
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

NPVRHelper::NPVRHelper( ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr )
:BaseCPHelper(pool, mgr)
{
}

bool NPVRHelper::validateContentNPVRProperty( ::TianShanIce::ContentProvision::ProvisionSessionExPtr sess )
{
	if (!sess->primaryContent)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NPVR, "[%s] no content proxy, skip validating the nPVR content property"), sess->ident.name.c_str());
		return true;
	}

	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_NPVR, "[%s] validateContentNPVRProperty() enter"), sess->ident.name.c_str());

	TianShanIce::Properties metaDatas;
	try
	{
		metaDatas = sess->primaryContent->getMetaData();

		//
		// dump the NPVR parameters
		//

		::TianShanIce::Properties::const_iterator it = metaDatas.find(METADATA_ProviderId);
		if (it==metaDatas.end())
			return false;

		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "[%s] NPVR propaties: [%s] = [%s]"), 
			sess->ident.name.c_str(), it->first.c_str(), it->second.c_str());

		it = metaDatas.find(METADATA_ProviderAssetId);
		if (it==metaDatas.end())
			return false;

		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "[%s] NPVR propaties: [%s] = [%s]"), 
			sess->ident.name.c_str(), it->first.c_str(), it->second.c_str());

		it = metaDatas.find(METADATA_SubscriberId);
		if (it==metaDatas.end())
			return false;

		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(MediaClusterCS, "[%s] NPVR propaties: [%s] = [%s]"), 
			sess->ident.name.c_str(), it->first.c_str(), it->second.c_str());

		return true;
	}
	catch(const ::Ice::ObjectNotExistException&)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] failed to getMetaData from content, caught exception[ObjectNotExistException]"), sess->ident.name.c_str());
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] failed to getMetaData from content, caught exception[%s]: %s"), sess->ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] failed to getMetaData from content, caught exception[%s]"),  sess->ident.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NPVR, "[%s] failed to getMetaData from content, caught unknown exception"), sess->ident.name.c_str());
	}

	return false;
}
