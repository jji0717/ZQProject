
#include "BaseClass.h"
#include "CPH_RTI.h"
#include "NTFSSource.h"
#include "NTFSTarget.h"
#include "IPushTrigger.h"
#include "PushSource.h"
#include "BaseClass.h"
#include "FilesetTarget.h"
#include "CPH_RTICfg.h"
#include "RTFProc.h"
#include "McastCapsrc.h"
#include "ErrorCode.h"
#include "HostToIP.h"
#include <list>
#include <math.h>
#include "TargetFac.h"
#include "TargetFactoryI.h"
#include "NtfsFileIoFactory.h"
#include "VstrmFileIoFactory.h"
#include "WPCapThread.h"


using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define CPH_RTI			"CPH_RTI"
#define MOLOG					(glog)

NetworkIFSelector*                                      _nNetSelector = NULL;
std::auto_ptr<ZQTianShan::ContentProvision::FileIoFactory> _pFileIoFac;

ZQTianShan::ContentProvision::BaseCPHelper* RTIHelper::_theHelper =NULL;

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
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFilePath().c_str());	
		return FALSE;		
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "Load configuration from [%s] successfully"),
			_gCPHCfg.getConfigFilePath().c_str());
		
		_gCPHCfg.snmpRegister("");
	}	

	//
	// do some module initialize
	//
	std::string errmsg;
	if (_gCPHCfg.enableTestNTFS)
	{
		NtfsFileIoFactory* pFactory = new NtfsFileIoFactory();
		pFactory->setRootDir(_gCPHCfg.szNTFSOutputDir);
		_pFileIoFac.reset(pFactory);
	}
	else
	{
		VstrmFileIoFactory* pFactory = new VstrmFileIoFactory();
		pFactory->setBandwidthManageClientId(_gCPHCfg.vstrmBwClientId);
		pFactory->setDisableBufDrvThrottle(_gCPHCfg.vstrmDisableBufDrvThrottle);
		pFactory->setEnableCacheForIndex(_gCPHCfg.enableCacheForIndex);
		pFactory->setEnableRAID1ForIndex(_gCPHCfg.enableRAID1ForIndex);
		_pFileIoFac.reset(pFactory);
	}

	_pFileIoFac->setLog(&glog);
	if (!_pFileIoFac->initialize())
	{
		std::string strErr;
		int nErrCode;
		_pFileIoFac->getLastError(strErr, nErrCode);
		printf("Failed to initialize fileio factory with error: %s, code: %d\n", strErr.c_str(), nErrCode);
		return false;
	}

	TargetFac * pTargetFac = new TargetFac(_pFileIoFac.get());
	TargetFactoryI::setInstance(pTargetFac);	

	RTFProcess::initRTFLib(_gCPHCfg.rtfMaxSessionNum, &glog, _gCPHCfg.rtfMaxInputBufferBytes, 
		_gCPHCfg.rtfMaxInputBuffersPerSession, _gCPHCfg.rtfSessionFailThreshold);
	std::string errstr;
	
	if (!RTIHelper::_theHelper)
		RTIHelper::_theHelper = new RTIHelper(pEngine->getThreadPool(), pEngine);

	_nNetSelector = new NetworkIFSelector(glog);
	if(!_nNetSelector)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "Failed to new NetIfSelector object."));
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
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "failed to load config of NetworkInterface for Capture, please check CPH_RTI.xml"));
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
			MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "Failed to initialize capture interface with error: %s"),
				pCaptureInterface->getLastError().c_str());
			return false;
		}
	}

	for (std::vector< ZQ::common::Config::Holder< Method > >::iterator iter = _gCPHCfg.methods.begin();iter != _gCPHCfg.methods.end();iter++)
	{
		if (/*iter->enableFlag && */iter->maxBandwidth && iter->maxSession)
		{
			pEngine->registerHelper((*iter).methodName.c_str(), RTIHelper::_theHelper, pCtx);
			pEngine->registerLimitation((*iter).methodName.c_str(), (*iter).maxSession,(*iter).maxBandwidth, GROUP_RTI, 0);
			MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "%s Helper registered"),(*iter).methodName.c_str());
		}
		else
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "%s Helper disabled"),(*iter).methodName.c_str());
		}
	}

	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_RTI, "maxAllocSampleCount = [%d]"), _gCPHCfg.maxAllocSampleCount);	
	return TRUE;
}

extern "C" __declspec(dllexport) void UninitCPH(ZQTianShan::ContentProvision::ICPHManager* pEngine, void* pCtx)
{
	if (NULL == pEngine)
		return;
	
	pEngine->unregisterHelper(METHODTYPE_RTIVSVSTRM, pCtx);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "RTI Helper unregistered"));

	pEngine->unregisterHelper(METHODTYPE_RTIH264VSVSTRM, pCtx);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "RTI H264 Helper unregistered"));

	if (RTIHelper::_theHelper)
	{
		try
		{
			delete RTIHelper::_theHelper;
		}
		catch(...){};
		
		RTIHelper::_theHelper = NULL;
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

	if (_pFileIoFac.get())
	{
		_pFileIoFac->uninitialize();
		_pFileIoFac.reset(0);
	}

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

RTISess::~RTISess()
{
	_nNetSelector->freeInterface(_strFileName);
	cleanup();
}

void RTISess::cleanup()
{
	if (_bCleaned)
		return;

	BaseGraph::Close();
	_bCleaned = true;
}


bool RTISess::preLoad()
{
	 std::string multicastIp;

	if (!_sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "provision session is 0"));
		setErrorInfo(ERRCODE_NULL_SESSION, "provision session is 0");			
		return false;
	}

	std::string strMethod = _sess->methodType;
	_strMethod = strMethod;
	if(stricmp(strMethod.c_str(), METHODTYPE_RTIVSVSTRM) && 
		stricmp(strMethod.c_str(), METHODTYPE_RTIH264VSVSTRM))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] unsupported method %s"), _sess->ident.name.c_str(),
			strMethod.c_str());
		setErrorInfo(ERRCODE_UNSUPPORT_METHOD, "unsupported method");			
		return false;
	}

	if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] could not find resource URI"), _sess->ident.name.c_str());
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find resource URI");			
		return false;
	}

	TianShanIce::ValueMap& resURI = _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource " CPHPM_FILENAME);			
		return false;
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource: " CPHPM_FILENAME);			
		return false;
	}

	std::string strFilename = var.strs[0];
	_strFileName = strFilename;

	TianShanIce::ValueMap& resBw = _sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] could not find ProvisionBandwidth resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}


	int nBandwidth = (int)var1.lints[0];
	_nBandwidth = nBandwidth;

	TianShanIce::ValueMap& resMI= _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resMI.end() == resMI.find(CPHPM_SOURCEURL))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] could not find multicast resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_SOURCEURL);			
		return false;
	}
	
	TianShanIce::Variant& var2 = resMI[CPHPM_SOURCEURL];
	if (var2.type != TianShanIce::vtStrings || var2.bRange|| var2.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_SOURCEURL);			
		return false;
	}
	std::string multicastUrl = var2.strs[0];

	multicastIp = multicastUrl.substr(multicastUrl.find_first_of(':')+3,multicastUrl.find_last_of(':')-6);
	std::string strmulticastPort = multicastUrl.substr(multicastUrl.find_last_of(':')+1,multicastUrl.size()-multicastUrl.find_last_of(':')-1);
	int multicastPort = atoi(strmulticastPort.c_str());
	
	bool enableNoTrickSpeed = false;

	TianShanIce::Properties prop = _sess->props;
	TianShanIce::Properties::const_iterator it = prop.find(CPHPM_NOTRICKSPEEDS);
	if (it != prop.end())
	{
		int major = 0, minor = 0;
		RTFProcess::getCTFLibVersion(major, minor);
		if(major < 3)
		{
			MOLOG(Log::L_WARNING, CLOGFMT(CPH_RTI, "[%s] CTFLib Verion support No TrickSpeed File must >= 3.0 "), _sess->ident.name.c_str());
		}
		enableNoTrickSpeed = true;
	}

	std::list<float> trickspeed;

	::TianShanIce::ContentProvision::TrickSpeedCollection trickcol = _sess->trickSpeeds;
	if (trickcol.size() == 0)
		trickcol.push_back(7.5);
	for (::TianShanIce::ContentProvision::TrickSpeedCollection::iterator iterTrick = trickcol.begin();iterTrick != trickcol.end();iterTrick++)
	{
		if ((*iterTrick) > 60)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] can't set trickspeed larger than 60."), _sess->ident.name.c_str());
			return false;
		}
		trickspeed.push_back((*iterTrick));
	}

	SetLog(_helper.getLog());
	SetMemAlloc(_helper.getMemoryAllocator());
	SetLogHint(strFilename.c_str());

	int nMaxBandwidth = nBandwidth;
	
	DWORD timeoutInterval = _gCPHCfg.timeoutInterval;
	std::string localIp;
	if (!_nNetSelector->allocInterface(nMaxBandwidth,localIp,strFilename))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] Failed to allocate proper network card"), _strLogHint.c_str());
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
	int outPutNum = 2 + exMap.size();

	if(enableNoTrickSpeed)
		outPutNum = 2;

	if (outPutNum < 2)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] Not specify trick speed"), _strLogHint.c_str());
		return false;
	}

	RTFProcess* pProcess = (RTFProcess*)ProcessFactory::Create(PROCESS_TYPE_RTF, &_helper._pool);
	AddFilter(pProcess);
	if(!enableNoTrickSpeed)
	{
		pProcess->setTrickFile(exMap);
		pProcess->settrickSpeedNumerator(trickspeed);
	}
	pProcess->setRetryCount(_gCPHCfg.retryCaptureCount);
	pProcess->setUnifiedTrickFile(_gCPHCfg.unifiedtrickfile.enable);

	if (bH264Type)
	{
		pProcess->setTrickGenParam(CTF_INDEX_TYPE_VV2, CTF_VIDEO_CODEC_TYPE_H264);
	}
	
	{
		FilesetTarget* pTarget = (FilesetTarget*)TargetFactoryI::instance()->create(TARGET_TYPE_FILESET);
		if(!AddFilter(pTarget))
			return false;

		pTarget->setCacheDirectory(_gCPHCfg.szCacheDir);
		pTarget->setBandwidth(nMaxBandwidth);
		pTarget->setFilename(strFilename.c_str());
		pTarget->enablePacingTrace(_gCPHCfg.enablePacingTrace);
		pTarget->enableProgressEvent(_gCPHCfg.enableProgEvent);
		pTarget->enableMD5(_gCPHCfg.enableMD5);
		if(!enableNoTrickSpeed)
		{
			pTarget->setTrickFile(exMap);
			pTarget->setTrickSpeed(trickspeed);
		}
		pTarget->setEnableRAID1ForIndex(_gCPHCfg.enableRAID1ForIndex);
		pTarget->setTargetDeletion(_gCPHCfg.deleteTargetFileCapFail);
		pTarget->setWriteLatencyWarning(_gCPHCfg.warningDiskWriteLongLatency);
		if (bH264Type)
		{
			bool bPacing = true;
			pTarget->setTypeH264();
			pTarget->enablePacing(bPacing);
			pTarget->enableStreamableEvent(false);
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

	SetMediaSampleSize(_gCPHCfg.mediaSampleSize);

	if (!Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		setErrorInfo(_nLastErrCode, (std::string("Failed to initialize graph with error: ") + _strLastErr).c_str());			
		return false;
	}

	BaseCPHSession::preLoad();
	_nSchedulePlayTime = (_sess->scheduledEnd - _sess->scheduledStart);

	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] preload successful"), _strLogHint.c_str());
	return true;
}

void RTISess::terminate(bool bProvisionSuccess)
{
	// not started, just constructed
	if (getStatus() == NativeThread::stDeferred)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] to terminate() session while in init status"), _strLogHint.c_str());
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
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] to terminate() session with status[%s]"), _strLogHint.c_str(),
			bProvisionSuccess?"success":"failure");
			_bQuit = true;

		if (!bProvisionSuccess && !BaseGraph::IsErrorOccurred())
		{
			BaseGraph::SetLastError("User canceled provision", ERRCODE_USER_CANCELED);
		}

		BaseGraph::Stop();
	}
}

bool RTISess::getProgress(::Ice::Long& offset, ::Ice::Long& total)
{	
	total = _filesize;
	offset = getProcessBytes();
	return true;
}

int RTISess::run(void)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] run() called"), _strLogHint.c_str());

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
		notifyStopped(true, params);

		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] notifyStopped() called, status[failure], error[%s]"), 
			_strLogHint.c_str(), _strLastErr.c_str());

		Close();
		return 0;
	}

	Close();
	LONGLONG supportFileSize=0;
	std::string md5;
//	if (_gCPHCfg.enableTestNTFS)
	{
		FilesetTarget* pTarget = (FilesetTarget*)_pMainTarget;
		supportFileSize = pTarget->getSupportFileSize();
		pTarget->getMD5(md5);
	}

	char tmp[64];
	_llTotalBytes = _llProcBytes;		//use the real process bytes
	sprintf(tmp, "%lld", _llTotalBytes);
	params[EVTPM_TOTOALSIZE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] Filesize[%s]"), _strLogHint.c_str(), tmp);

	sprintf(tmp, "%lld", supportFileSize);
	params[EVTPM_SUPPORTFILESIZE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] SupportFilesize[%s]"), _strLogHint.c_str(), tmp);

	params[EVTPM_MD5CHECKSUM] = md5;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] md5[%s]"), _strLogHint.c_str(), md5.c_str());

	if (_bitrate)
	{
		sprintf(tmp, "%lld", _llProcBytes*8000/_bitrate);
		params[EVTPM_PLAYTIME] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] playtime[%s]"), _strLogHint.c_str(), tmp);
	}
	else {
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] bitrate [0] , can't get playtime"), _strLogHint.c_str());
	}

	if (!_bStartEventSent)
	{
		notifyStarted(params);
	}
	notifyStopped(false, params);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] notifyStopped() called, status[success]"), _strLogHint.c_str());
	return 0;
}

void RTISess::final(int retcode, bool bCancelled)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] final() ret=%d, cancelled=%c; calling doCleanup()"), 
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

bool RTIHelper::validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource)
{
	if (!sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "provision session is 0"));
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTI, 0, "provision session is 0");
	}
	
	std::string strMethod = sess->methodType;
	if(stricmp(strMethod.c_str(), METHODTYPE_RTIVSVSTRM)&&
		stricmp(strMethod.c_str(), METHODTYPE_RTIH264VSVSTRM))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] unsupported method %s"), sess->ident.name.c_str(),
			strMethod.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTI, 0, "unsupported method %s", strMethod.c_str());
	}
	
	if (sess->resources.end() == sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] could not find resource URI"), sess->ident.name.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTI, 0, "could not find resource URI");
	}
	
	TianShanIce::ValueMap& resURI = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTI, 0, "could not find URI resource " CPHPM_FILENAME);
	}
	
	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTI, 0, "could not find URI resource " CPHPM_FILENAME);
	}
	
	TianShanIce::ValueMap& resBw = sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTI, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}
	
	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTI, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}
	
	TianShanIce::ValueMap& resMI= sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resMI.end() == resMI.find(CPHPM_SOURCEURL))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] could not find multicast resource: %s"), sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTI, 0, "could not find multicast resource " CPHPM_SOURCEURL);
	}

	TianShanIce::Variant& var2 = resMI[CPHPM_SOURCEURL];
	if (var2.type != TianShanIce::vtStrings || var2.bRange||  var2.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] could not find multicast resource %s"), sess->ident.name.c_str(), 
			CPHPM_SOURCEURL);
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTI, 0, "could not find multicast resource" CPHPM_SOURCEURL);
	}
	std::string mulicastUrl = var2.strs[0];
	std::string strpro = mulicastUrl.substr(0,6);
	if (_stricmp(strpro.c_str(),"udp://")!= 0)
	//if (strstr(mulicastUrl.c_str(),"udp://")==NULL)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] Can't find the udp protocol from url %s"), sess->ident.name.c_str(), mulicastUrl.c_str());
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter>(CPH_RTI, TianShanIce::Storage::csexpInvalidSourceURL, "Can't find the udp header from url" ,mulicastUrl.c_str());
	}

	sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	sess->preload = randomPreload(_gCPHCfg.preloadTime);

	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_RTI, "[%s] set preload=%d ms"), sess->ident.name.c_str(), sess->preload);
	return true;
}

bool RTISess::prime()
{
	if (!BaseCPHSession::prime())
		return false;

// 	McastCapSource* pSource = (McastCapSource*)getSourceFilter();
// 	if (!pSource)
// 	{
// 		//should not go here
// 		setErrorInfo(ERRCODE_PUSHSOURCE_MISSING, ("Provisioning failed with error: not initialized"));			
// 		return false;
// 	}
// 
// 	pSource->Start();

	if (!Start())
	{
		setErrorInfo(_nLastErrCode, (std::string("Failed to start graph with error: ") + _strLastErr).c_str());		
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] failed to start graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}

	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] prime() successful"), _strLogHint.c_str());
	return true;
}

void RTISess::OnProgress(LONGLONG& prcvBytes)
{
	BaseGraph::OnProgress(prcvBytes);
	::TianShanIce::Properties params;
	updateProgress(_llProcBytes, _llTotalBytes,params);
}

void RTISess::OnStreamable(bool bStreamable)
{
	BaseGraph::OnStreamable(bStreamable);
	notifyStreamable(bStreamable);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void RTISess::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());
	
	char tmp[40];
	::TianShanIce::Properties params;
	sprintf(tmp, "%d", mInfo.bitrate);
	params[EVTPM_MPEGBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] bitrate[%s]"), _strLogHint.c_str(),tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionV);
	params[EVTPM_VIDEOHEIGHT] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] videoResolutionV[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionH);
	params[EVTPM_VIDEOWIDTH] =  tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] videoResolutionH[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoBitrate);
	params[EVTPM_VIDEOBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] videoBitrate[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp,"%.2f",mInfo.framerate);
	params[EVTPM_FRAMERATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] framerate[%s]"), _strLogHint.c_str(),tmp);

	_bitrate = mInfo.bitrate;
	
	LONGLONG llTotalBytes = (LONGLONG)((_bitrate*1.0)*_nSchedulePlayTime/8000);
	setTotalBytes(llTotalBytes);
	
	notifyStarted(params);
	_bStartEventSent = true;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] notifyStarted() called"), _strLogHint.c_str());
	
	if (_pMainTarget)
	{
		_pMainTarget->setStreamableBytes(mInfo.bitrate*_gCPHCfg.streamReqSecs/8);
	}
}

RTIHelper::RTIHelper(ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr)
:BaseCPHelper(pool, mgr) 
{
	RtiConfig::Methods::iterator it;
	for (it = _gCPHCfg.methods.begin();it != _gCPHCfg.methods.end();it++)
	{
		if (!it->maxSession || !it->maxBandwidth)
			continue;

		_methodCostList[it->methodName] = new MethodCostI(it->maxBandwidth, it->maxSession);				
	}			
}

RTIHelper::~RTIHelper()
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
bool RTIHelper:: getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins)
{
	RtiConfig::Methods::iterator methodIter;
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
