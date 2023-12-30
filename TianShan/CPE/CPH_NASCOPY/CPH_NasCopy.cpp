
#include "BaseClass.h"
#include "CPH_NasCopy.h"
#include "NTFSSource.h"
#include "BaseClass.h"
#include "VstrmFilesetTarget.h"
#include "CPH_NasCopyCfg.h"
#include "ErrorCode.h"
#include "NasCopySource .h"
#include "TsStorage.h"
#include "VstrmBase.h"

using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define CPH_NasCopy			"CPH_NasCopy"
#define MOLOG					(glog)

ZQTianShan::ContentProvision::BaseCPHelper* NasCyHelper::_theHelper =NULL;

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
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFilePath().c_str());	
		return FALSE;
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "Load configuration from [%s] successfully"),
			_gCPHCfg.getConfigFilePath().c_str());
		
		_gCPHCfg.snmpRegister("");
	}

	//
	// do some module initialize
	//
	std::string errmsg;
	if (!VstrmBaseFunc::init(_gCPHCfg.vstrmBwClientId, errmsg))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "Failed to VstrmBaseFunc::init() with error %s"), errmsg.c_str());
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "VstrmBaseFunc::init() successful"));
	}

	if (!NasCyHelper::_theHelper)
		NasCyHelper::_theHelper = new NasCyHelper(pEngine->getThreadPool(), pEngine);

	std::vector< ZQ::common::Config::Holder< Method > >::iterator iter = _gCPHCfg.methods.begin();
	if(iter != _gCPHCfg.methods.end())
		pEngine->registerLimitation((*iter).methodName.c_str(), (*iter).maxSession,(*iter).maxBandwidth, GROUP_NASCOPY, 0);

	pEngine->registerHelper(METHODTYPE_NASCOPYVSVSTRM, NasCyHelper::_theHelper, pCtx);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "Nas copy Helper registered"));

	return TRUE;
}

extern "C" __declspec(dllexport) void UninitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return;
	
	pEngine->unregisterHelper(METHODTYPE_NASCOPYVSVSTRM, pCtx);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "Nascopy Helper unregistered"));

	if (NasCyHelper::_theHelper)
	{
		try
		{
			delete NasCyHelper::_theHelper;
		}
		catch(...){};
		
		NasCyHelper::_theHelper = NULL;
	}
	
	//
	//do some module uninitialize
	//
	VstrmBaseFunc::unInit();

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
	
	static const char* szProtocol[] = {
		"file:",
		"cifs:",
		"nfs:"
	};
	static int nProto = sizeof(szProtocol)/sizeof(const char*);

	int i;
	for(i=0;i<nProto;i++)
	{
		if (!strnicmp(pathbuf, szProtocol[i], strlen(szProtocol[i])))
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

NasCySess::~NasCySess()
{
	cleanup();
}

void NasCySess::cleanup()
{
	if (_bCleaned)
		return;

	BaseGraph::Close();
	_bCleaned = true;
}


bool NasCySess::preLoad()
{
	if (!_sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "provision session is 0"));
		setErrorInfo(ERRCODE_NULL_SESSION, "provision session is 0");			
		return false;
	}

	std::string strMethod = _sess->methodType;
	_strMethod = strMethod;
	if(stricmp(strMethod.c_str(), METHODTYPE_NASCOPYVSVSTRM))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] unsupported method %s"), _sess->ident.name.c_str(),
			strMethod.c_str());
		setErrorInfo(ERRCODE_UNSUPPORT_METHOD, "unsupported method");			
		return false;
	}

	if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] could not find resource URI"), _sess->ident.name.c_str());
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find resource URI");			
		return false;
	}

	TianShanIce::ValueMap& resURI = _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource " CPHPM_FILENAME);			
		return false;
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource: " CPHPM_FILENAME);			
		return false;
	}
	std::string strFilename = var.strs[0];

	TianShanIce::ValueMap& resBw = _sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] could not find bandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find bandwidth resource " CPHPM_BANDWIDTH);			
		return false;
	}
	
	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] could not find bandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find bandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}
    ULONG64   bitrate = var1.lints[0];
	_nBandwidth = bitrate;

	TianShanIce::ValueMap& resDes = _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resDes.end() == resDes.find(CPHPM_SOURCEURL))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] could not find source filename resource %s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find source filename resource " CPHPM_SOURCEURL);			
		return false;
	}
	
	TianShanIce::Variant& var2 = resDes[CPHPM_SOURCEURL];
	if (var2.type != TianShanIce::vtStrings || var2.bRange || var2.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] could not find source filename resource %s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find source filename resource: " CPHPM_SOURCEURL);			
		return false;
	}

	std::string srcFilename = var2.strs[0];	

	//
	// convert the file name if need
	//
	char target[MAX_PATH];
	if (URLStr::decode(srcFilename.c_str(),(void*)target, MAX_PATH))
		srcFilename = target;
	else
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] Failed to decode url %s"), _strLogHint.c_str(),srcFilename.c_str());
		return false;
	}

	fixpath(srcFilename, false);
	
	SetLog(_helper.getLog());
	SetMemAlloc(_helper.getMemoryAllocator());
	SetLogHint(strFilename.c_str());

	NasCopySource* pSource = (NasCopySource*)SourceFactory::Create(SOURCE_TYPE_NASCOPY);//new NasCopySource();
	if (!pSource)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] Failed to initialize NasCopySource object"), _strLogHint.c_str(), _strLastErr.c_str());
	    return false;
	}
	AddFilter(pSource);

	pSource->setSourceFileName(srcFilename.c_str());
	pSource->setDestFileName(strFilename.c_str());
	pSource->setBandwith(bitrate);
	pSource->setVstrmBwClientId(_gCPHCfg.vstrmBwClientId);	
	pSource->setDisableBitrateLimit(_gCPHCfg.bDisableBitrateLimit);

	if (!Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		setErrorInfo(_nLastErrCode, (std::string("Failed to initialize graph with error: ") + _strLastErr).c_str());			
		return false;
	}

	BaseCPHSession::preLoad();

	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] preload successful"), _strLogHint.c_str());
	return true;
}

void NasCySess::terminate(bool bProvisionSuccess)
{
	// not started, just constructed
	if (getStatus() == NativeThread::stDeferred)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] to terminate() session while in init status"), _strLogHint.c_str());
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

bool NasCySess::getProgress(::Ice::Long& offset, ::Ice::Long& total)
{	
	total = _filesize;
	offset = getProcessBytes();
	return true;
}

int NasCySess::run(void)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] run() called"), _strLogHint.c_str());
	::TianShanIce::Properties params;
	
	_helper.increaseLoad(_strMethod,_nBandwidth);
	bool bRet = Run();
	_helper.decreaseLoad(_strMethod,_nBandwidth);
	if (!bRet)
	{
		setErrorInfo(_nLastErrCode, (std::string("Provisioning failed with error: ") + _strLastErr).c_str());			
		
		notifyStopped(true, params);
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] notifyStopped() called, status[failure], error[%s]"), 
			_strLogHint.c_str(), _strLastErr.c_str());
		Close();
		return 0;
	}

	char tmp[64];
	sprintf(tmp, "%lld", _llTotalBytes);
	params[EVTPM_TOTOALSIZE] = tmp;
	if (_bitrate)
	{
		sprintf(tmp, "%lld", (_llTotalBytes*8000/_bitrate));
		params[EVTPM_PLAYTIME] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] playtime[%s]"), _strLogHint.c_str(),tmp);
	}
	else
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] bitrate [0] , can't get playtime"), _strLogHint.c_str());
	Close();

	OnStreamable(true);
	notifyStopped(false, params);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] notifyStopped() called, status[success]"), _strLogHint.c_str());
	return 0;
}

void NasCySess::final(int retcode, bool bCancelled)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] final() ret=%d, cancelled=%c; calling doCleanup()"), 
		_strLogHint.c_str(),retcode, bCancelled?'Y':'N');
	
	cleanup();
	try
	{
		delete this;
	}catch(...)
	{
	}
}


bool NasCyHelper::validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource)
{
	if (!sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "provision session is 0"));
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NasCopy, 0, "provision session is 0");
	}
	std::string srcFilename; 
	std::string strMethod = sess->methodType;
	if(stricmp(strMethod.c_str(), METHODTYPE_NASCOPYVSVSTRM))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] unsupported method %s"), sess->ident.name.c_str(),
			strMethod.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NasCopy, 0, "unsupported method %s", strMethod.c_str());
	}
	
	if (sess->resources.end() == sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] could not find resource URI"), sess->ident.name.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NasCopy, 0, "could not find resource URI");
	}
	
	TianShanIce::ValueMap& resURI = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NasCopy, 0, "could not find URI resource " CPHPM_FILENAME);
	}
	
	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NasCopy, 0, "could not find URI resource " CPHPM_FILENAME);
	}
	
	TianShanIce::ValueMap& resBw = sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NasCopy, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}
	
	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NasCopy, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}
	
 	TianShanIce::ValueMap& resDes = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
 	if (resDes.end() == resDes.find(CPHPM_SOURCEURL))
 	{
 		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] could not find source file name resource %s"), sess->ident.name.c_str(), 
 			CPHPM_SOURCEURL);
 		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NasCopy, 0, "could not find source file name resource " CPHPM_SOURCEURL);
 	}
 	
 	TianShanIce::Variant& var2 = resDes[CPHPM_SOURCEURL];
 	if (var2.type != TianShanIce::vtStrings || var2.bRange || var2.strs.size() <=0)
 	{
 		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] could not find source file resource %s"), sess->ident.name.c_str(), 
 			CPHPM_SOURCEURL);
 		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NasCopy, TianShanIce::Storage::csexpInvalidSourceURL, "could not find source file name resource " CPHPM_SOURCEURL);		
 	}

	srcFilename = var2.strs[0];

	//
	// convert the file name if need
	//
	char target[MAX_PATH];
	if (URLStr::decode(srcFilename.c_str(),(void*)target, MAX_PATH))
		srcFilename = target;
	else
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] Failed to decode url %s"), sess->ident.name.c_str(),srcFilename.c_str());
		return false;
	}
	fixpath(srcFilename);

	HANDLE hFile;
	hFile = CreateFile(srcFilename.c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		std::string errmsg;
		getSystemErrorText(errmsg);
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_NasCopy, "[%s] could not open file %s, because %s"), sess->ident.name.c_str(), 
			srcFilename.c_str(),errmsg.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_NasCopy, TianShanIce::Storage::csexpInvalidSourceURL, "could not open file %s ", srcFilename.c_str());
	}
	CloseHandle(hFile);

	sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	return true;
}

bool NasCySess::prime()
{
	if (!BaseCPHSession::prime())
		return false;
	if (!Start())
	{
		setErrorInfo(_nLastErrCode, (std::string("Failed to start graph with error: ") + _strLastErr).c_str());		
		MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] failed to start graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}

	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] prime() successful"), _strLogHint.c_str());
	return true;
}

void NasCySess::OnProgress(LONGLONG& prcvBytes)
{
	BaseGraph::OnProgress(prcvBytes);
	if (_llProcBytes>_llTotalBytes)
		_llTotalBytes = _llProcBytes;

	::TianShanIce::Properties params;
	updateProgress(_llProcBytes, _llTotalBytes,params);
}

void NasCySess::OnStreamable(bool bStreamable)
{
	BaseGraph::OnStreamable(bStreamable);
	notifyStreamable(bStreamable);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void NasCySess::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());
	
	char tmp[40];
	::TianShanIce::Properties params;
	sprintf(tmp, "%d", mInfo.bitrate);
	params[EVTPM_MPEGBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] bitrate[%s]"), _strLogHint.c_str(),tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionV);
	params[EVTPM_VIDEOHEIGHT] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] videoResolutionV[%s]"), _strLogHint.c_str(),tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionH);
	params[EVTPM_VIDEOWIDTH] =  tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] videoResolutionH[%s]"), _strLogHint.c_str(),tmp);
	sprintf(tmp, "%d", mInfo.videoBitrate);
	params[EVTPM_VIDEOBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] videoBitrate[%s]"), _strLogHint.c_str(),tmp);
	sprintf(tmp,"%.2f",mInfo.framerate);
	params[EVTPM_FRAMERATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] framerate[%s]"), _strLogHint.c_str(),tmp);

	_bitrate = mInfo.bitrate;
	
	notifyStarted(params);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_NasCopy, "[%s] notifyStarted() called"), _strLogHint.c_str());
	
	if (_pMainTarget)
	{
		_pMainTarget->setStreamableBytes(mInfo.bitrate*_gCPHCfg.streamReqSecs/8);
	}
}
NasCyHelper::NasCyHelper(ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr)
:BaseCPHelper(pool, mgr) 
{	
}

NasCyHelper::~NasCyHelper()
{
}
/// query the current load information of a method type
///@param[in] methodType to specify the method type to query
///@param[out] allocatedKbps the current allocated bandwidth in Kbps
///@param[out] maxKbps the maximal allowed bandwidth in Kbps, -1 if unlimited
///@param[out] sessions the current running session instances
///@param[out] maxSessins the maximal allowed session instances, -1 if unlimited
///@return true if the query succeeded
bool NasCyHelper:: getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins)
{
	if (stricmp(methodType, METHODTYPE_NASCOPYVSVSTRM))
		return false;

	NasCopyConfig::Methods::iterator methodIter;
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

 unsigned int NasCyHelper::evaluateCost(unsigned int bandwidthKbps, unsigned int sessions)
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

