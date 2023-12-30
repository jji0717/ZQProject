#include "BaseClass.h"
#include "CPH_C2Propagation.h"
#include "CPH_C2PropagationCfg.h"
#include "ErrorCode.h"
#include "EncodeConvert.h"
#include <math.h>
#include <list>
#include "CStdFileIoFactory.h"
#include "HTTPClientFactory.h"

#ifdef ZQ_OS_MSWIN
#include "VstrmFileIoFactory.h"
#endif

#include "HTTPPropSource.h"
//#include "CIFSSource.h"
#include "SystemUtils.h"
#include "NormalizeSparseFile.h"
using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#ifdef ZQ_OS_LINUX
#ifndef stricmp
#define stricmp strcasecmp
#endif
#endif


#define CPH_C2PROPAGATION		"CPH_C2Propagation"
#define MOLOG					(glog)

std::auto_ptr<ZQTianShan::ContentProvision::FileIoFactory> C2PropagationSess::_pFileIoFac;
ZQTianShan::ContentProvision::BaseCPHelper* C2PropagationHelper::_theHelper =NULL;
HTTPClientFactory* C2PropagationSess::_pHttpClientFactory = NULL;
int64 SparseFileSize = 20000000000;

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
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFileName().c_str());	
		return false;
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "Load configuration from [%s] successfully"),
			_gCPHCfg.getConfigFileName().c_str());

		_gCPHCfg.snmpRegister("");
	}	
#ifdef ZQ_OS_MSWIN
	//
	// do some module initialize
	//
	std::string errmsg;
	if (_gCPHCfg.enableTestNTFS)
	{
		CStdFileIoFactory* pFactory = new CStdFileIoFactory();
		pFactory->setRootDir(_gCPHCfg.szNTFSOutputDir);
		C2PropagationSess::_pFileIoFac.reset(pFactory);
	}
	else
	{
		VstrmFileIoFactory* pFactory = new VstrmFileIoFactory();
		pFactory->setBandwidthManageClientId(_gCPHCfg.vstrmBwClientId);
		pFactory->setDisableBufDrvThrottle(_gCPHCfg.vstrmDisableBufDrvThrottle);
		C2PropagationSess::_pFileIoFac.reset(pFactory);
	}
#else
	CStdFileIoFactory* pFactory = new CStdFileIoFactory();
	//	pFactory->setRootDir(_gCPHCfg.szTargetDir);
	C2PropagationSess::_pFileIoFac.reset(pFactory);
#endif
	C2PropagationSess::_pFileIoFac->setLog(&glog);
	if (!C2PropagationSess::_pFileIoFac->initialize())
	{
		std::string strErr;
		int nErrCode;
		C2PropagationSess::_pFileIoFac->getLastError(strErr, nErrCode);
		printf("Failed to initialize fileio factory with error: %s, code: %d\n", strErr.c_str(), nErrCode);
		return false;
	}

	C2PropagationSess::_pHttpClientFactory = new HTTPClientFactory();
	if (!C2PropagationSess::_pHttpClientFactory)
	{
		return false;	
	}
	C2PropagationSess::_pHttpClientFactory->setLog(&glog);
	//
	// helper
	//
	if (!C2PropagationHelper::_theHelper)
		C2PropagationHelper::_theHelper = new C2PropagationHelper(pEngine->getThreadPool(), pEngine);

	//
	// register methods
	//
	for (std::vector< ZQ::common::Config::Holder< Method > >::iterator iter = _gCPHCfg.methods.begin();iter != _gCPHCfg.methods.end();iter++)
	{
		if (iter->enableFlag && iter->maxBandwidth && iter->maxSession)
		{
			pEngine->registerHelper((*iter).methodName.c_str(), C2PropagationHelper::_theHelper, pCtx);
			pEngine->registerLimitation((*iter).methodName.c_str(), (*iter).maxSession,(*iter).maxBandwidth, GROUP_C2PROPAGATION, 0);
			MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "%s Helper registered"),(*iter).methodName.c_str());
		}
		else
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "%s Helper disabled"),(*iter).methodName.c_str());
		}
	}
	if(_gCPHCfg.sparsefile.sparseFilesize)
		SparseFileSize = (int64)_gCPHCfg.sparsefile.sparseFilesize * 1000;

	return true;
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
			MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "%s Helper unregistered"),(*iter).methodName.c_str());
		}
	}

	if (C2PropagationHelper::_theHelper)
	{
		try
		{
			delete C2PropagationHelper::_theHelper;
		}
		catch(...){};

		C2PropagationHelper::_theHelper = NULL;
	}

	//
	//do some module uninitialize
	//

	if (C2PropagationSess::_pHttpClientFactory)
	{
		try
		{
			delete C2PropagationSess::_pHttpClientFactory;
		}
		catch(...){};

		C2PropagationSess::_pHttpClientFactory = NULL;
	}

	if (C2PropagationSess::_pFileIoFac.get())
	{
		C2PropagationSess::_pFileIoFac->uninitialize();
		C2PropagationSess::_pFileIoFac.reset(0);
	}

//	ZQ::common::setGlogger(NULL);
}

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

	delete []pathbuf;
	return true;

}

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


C2PropagationSess::~C2PropagationSess()
{
	cleanup();
}

void C2PropagationSess::cleanup()
{
	if (_bCleaned)
		return;

	BaseGraph::Close();
	_bCleaned = true;
}


bool C2PropagationSess::preLoad()
{
	if (!_sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "provision session is 0"));
		setErrorInfo(ERRCODE_NULL_SESSION, "provision session is 0");			
		return false;
	}

	_strMethod = _sess->methodType;
	if(stricmp(_strMethod.c_str(), METHODTYPE_CDN_HTTPPropagation))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "[%s] unsupported method %s"), _sess->ident.name.c_str(),
			_strMethod.c_str());
		setErrorInfo(ERRCODE_UNSUPPORT_METHOD, "unsupported method");			
		return false;
	}
	std::string strFilename;
	std::string strSrcUrl;
	if (_sess->resources.end() == _sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "[%s] could not find resource URI"), _sess->ident.name.c_str());
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find resource URI");			
		return false;
	}

	TianShanIce::ValueMap& resURI = _sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find URI resource " CPHPM_FILENAME);			
		return false;
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "[%s] could not find URI resource %s"), _sess->ident.name.c_str(), 
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

	TianShanIce::ValueMap& resBw = _sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "[%s] could not find ProvisionBandwidth resource: %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "[%s] could not find ProvisionBandwidth resource %s"), _sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		setErrorInfo(ERRCODE_PARAMS_MISSING, "could not find ProvisionBandwidth resource: " CPHPM_BANDWIDTH);			
		return false;
	}
	int nBandwidth = (int)var1.lints[0];
	_nBandwidth = nBandwidth;

	std::string providerId,providerAssetId;
	TianShanIce::Properties prop = _sess->props;
	TianShanIce::Properties::const_iterator it;

	it = prop.find(CPHPM_PROVIDERID);
	if (it!=prop.end())
		providerId = it->second;

	it = prop.find(CPHPM_PROVIDERASSETID);
	if (it!=prop.end())
		providerAssetId = it->second;

	if(strFilename.size() == 0)
	{
		strFilename = providerAssetId + providerId;
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

	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_C2PROPAGATION, "[%s] bandwidth[%d], max_limit_rate[%d], maxbandwidth[%d]"), _strLogHint.c_str(), 
		nBandwidth, _gCPHCfg.bandwidthLimitRate, nMaxBandwidth);

	bool bVstreamIO = false;
	HTTPPropSource* pSource = new HTTPPropSource(_pHttpClientFactory,_pFileIoFac.get());
	if (!pSource)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "[%s] create HTTP source failed"), _strLogHint.c_str());
		return false;
	}
	AddFilter(pSource);
	pSource->setURL(strSrcUrl.c_str());
	pSource->setLog(_pLog);
	pSource->setLocalNetIf(_gCPHCfg.szLocalNetIf);
	pSource->setCacheDir(_gCPHCfg.szCacheDir);
	pSource->setFilename(strFilename.c_str());
	pSource->setMaxBandwidth(nMaxBandwidth);
	pSource->setEnableResumeDownload(_gCPHCfg.enableResumeForDownload);
	//pSource->setBandwidthCtrlInterval(_gCPHCfg.bandwidthCtrlInterval);
	pSource->setWaitTimeForGrowing(_gCPHCfg.timeoutForGrowing);
	pSource->enableMD5(true);	
	pSource->setPIDAndPAID(providerId, providerAssetId);
	pSource->setIngressCapcaity(_gCPHCfg.ingressCapcaity * 1000);
	pSource->setLocateFileIP(_gCPHCfg.bindip, _gCPHCfg.transferip);
	pSource->setTransferServerPort(_gCPHCfg.transferPort);
	pSource->setTransferDelay(_gCPHCfg.transferdelay);
	pSource->setSpeed(_gCPHCfg.nspeed);
	pSource->setTimeout(_gCPHCfg.timeout);
	pSource->setSleeptime(_gCPHCfg.sleeptime.timeInterval);

#ifdef ZQ_OS_MSWIN
	if(!_gCPHCfg.enableTestNTFS)
		bVstreamIO = true;
#endif

	pSource->setVstreamIO(bVstreamIO);
//  pSource->setTestCsicoPara(_gCPHCfg.testforcisco.nspeed, _gCPHCfg.testforcisco.transferdelay,
// 		_gCPHCfg.testforcisco.bandwidth, _gCPHCfg.testforcisco.range);
// 	pSource->setTimeout(_gCPHCfg.testforcisco.timeout);
// 	pSource->setLocatePos(_gCPHCfg.testforcisco.locateBeginpos, _gCPHCfg.testforcisco.locateEndpos);

	SetMediaSampleSize(_gCPHCfg.mediaSampleSize);

	if (!Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		setErrorInfo(_nLastErrCode, (std::string("Failed to initialize graph with error: ") + _strLastErr).c_str());			
		return false;
	}

	BaseCPHSession::preLoad();

	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] preload successful"), _strLogHint.c_str());
	return true;
}

void C2PropagationSess::terminate(bool bProvisionSuccess)
{
	// not started, just constructed
	if (getStatus() == NativeThread::stDeferred)
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] to terminate() session while in init status"), _strLogHint.c_str());
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
		MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] to terminate() session with status[%s]"), _strLogHint.c_str(),
			bProvisionSuccess?"success":"failure");
		_bQuit = true;

		if (!bProvisionSuccess && !BaseGraph::IsErrorOccurred())
		{
			BaseGraph::SetLastError("User canceled provision", ERRCODE_USER_CANCELED);
		}

		BaseGraph::Stop();
	}
}

bool C2PropagationSess::getProgress(::Ice::Long& offset, ::Ice::Long& total)
{	
	total = _filesize;
	offset = getProcessBytes();
	return true;
} 

int C2PropagationSess::run(void)
{	
	bool bRet;

	Ice::Long llStart = ZQTianShan::now();
	_helper.increaseLoad(_strMethod,_nBandwidth);
	if (!stricmp(_strMethod.c_str(), METHODTYPE_CDN_HTTPPropagation))
	{
		HTTPPropSource* pSource = (HTTPPropSource*)getSourceFilter();
		if (!pSource)
		{
			setErrorInfo(ERRCODE_INVALID_SRCURL, ("Source file is invalid."));			
			return false;
		}
		bRet = pSource->Run();

	}
	else
		bRet = Run();
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

		char tmp[40];
		sprintf(tmp, "False");
		params[EVTPM_OPENFORWRITE] = tmp;
		MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s]OpenForWrite [%s]"), _strLogHint.c_str(),tmp);

		notifyStopped(true, params);

		HTTPPropSource* pSource = (HTTPPropSource*)getSourceFilter();
		pSource->deleteOutput(_gCPHCfg.deleteOnFail);

		MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] provision done, status[failure], error[%s]"), 
			_strLogHint.c_str(), _strLastErr.c_str());
		return 0;
	}

	if(!stricmp(_strMethod.c_str(), METHODTYPE_CDN_HTTPPropagation))
	{
		HTTPPropSource* pSource = (HTTPPropSource*)getSourceFilter();
		// get the range info from 
		std::map<std::string, std::string> fileinfomap;
		std::map<std::string, std::string>::iterator iter;
		std::string extCol;

		pSource->getOutputFileInfo(fileinfomap);
		pSource->getOutputFileExtCol(extCol);
		// set to params
		for (iter = fileinfomap.begin();iter != fileinfomap.end();iter++)
		{
			params[iter->first.c_str()] = iter->second;
			//MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] File[%s] range[%s]"), _strLogHint.c_str(),iter->first.c_str(),iter->second.c_str());
			params[EVTPM_MEMBERFILEEXTS] = extCol;
			//MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] FileExts[%s]"), _strLogHint.c_str(),extCol.c_str());
		}
	}
	Close();

	int64 supportFileSize=0;
	std::string md5;
	std::vector <std::string> outputfilelist;
	std::string cacherDir = "";
	int indexType, outputfilecount;

#ifdef ZQ_OS_MSWIN
	if (_gCPHCfg.enableTestNTFS)
#endif
	{
		if (!stricmp(_strMethod.c_str(), METHODTYPE_CDN_HTTPPropagation))
		{
			HTTPPropSource* pSource = (HTTPPropSource*)getSourceFilter();
			pSource->getSupportFileSize(supportFileSize);
			pSource->getMD5(md5);
			_llProcBytes = pSource->getProcessSize();
			_llTotalBytes = pSource->getTotalSize();
			pSource->getOutputFiles(outputfilelist, outputfilecount, indexType);
		}
	}

	bool bVstreamIO = false;
#ifdef ZQ_OS_MSWIN
	if(!_gCPHCfg.enableTestNTFS)
		bVstreamIO = true;
#endif

	if(!bVstreamIO && _gCPHCfg.enableNSF && outputfilelist.size() > 0)
	{
		CStdFileIoFactory* pFactory = (CStdFileIoFactory*)C2PropagationSess::_pFileIoFac.get();
		cacherDir = pFactory->getRootDir();
		for(unsigned int i = 0 ; i < outputfilelist.size(); i ++)
		{
			outputfilelist[i] =  cacherDir +  outputfilelist[i];
		}
		NormalizeSparseFile normallize(_pLog, outputfilecount, outputfilelist, indexType, false);
		int bRet = normallize.normalizeSparseFileSet();
		if(bRet)
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] provision done, status[failure], error[failed to normalize sparse file]"), 
				_strLogHint.c_str());
			return 0;
		}
	}

	char tmp[64];
	if (!_llTotalBytes)
	{
		_llTotalBytes = _llProcBytes;
	}
	sprintf(tmp, FMT64, _llTotalBytes);
	params[EVTPM_TOTOALSIZE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] Filesize[%s]"), _strLogHint.c_str(), tmp);

	sprintf(tmp, FMT64, supportFileSize);
	params[EVTPM_SUPPORTFILESIZE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] SupportFilesize[%s]"), _strLogHint.c_str(), tmp);

	params[EVTPM_MD5CHECKSUM] = md5;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] md5[%s]"), _strLogHint.c_str(), md5.c_str());

	if (_bitrate)
	{
		if (!stricmp(_strMethod.c_str(), METHODTYPE_CDN_HTTPPropagation))
		{
			sprintf(tmp, FMT64, _llTotalBytes*8000/_bitrate);
			params[EVTPM_PLAYTIME] = tmp;
		}
		else
		{
			sprintf(tmp, FMT64, _llProcBytes*8000/_bitrate);
			params[EVTPM_PLAYTIME] = tmp;
		}
		MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] playtime[%s]"), _strLogHint.c_str(), tmp);
	}
	else 
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] bitrate [0] , can't get playtime"), _strLogHint.c_str());
	}

	sprintf(tmp, "False");
	params[EVTPM_OPENFORWRITE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] OpenForWrite[%s]"), _strLogHint.c_str(),tmp);

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
	MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] provision done, status[success], spent[%d]ms, actualspeed[%d]bps"), 
		_strLogHint.c_str(), nTimeSpentMs, nActualBps);
	return 0;
}

void C2PropagationSess::final(int retcode, bool bCancelled)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] final() ret=%d, cancelled=%c; calling doCleanup()"), 
		_strLogHint.c_str(),retcode, bCancelled?'Y':'N');

	cleanup();
	try
	{
		delete this;
	}catch(...)
	{
	}
}


bool C2PropagationHelper::validateSetup(::TianShanIce::ContentProvision::ProvisionSessionExPtr sess)
throw (::TianShanIce::InvalidParameter, ::TianShanIce::SRM::InvalidResource)
{
	if (!sess)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "provision session is 0"));
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_C2PROPAGATION, 0, "provision session is 0");
	}

	std::string strMethod = sess->methodType;
	if(stricmp(strMethod.c_str(), METHODTYPE_CDN_HTTPPropagation))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "[%s] unsupported method %s"), sess->ident.name.c_str(),
			strMethod.c_str());
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_C2PROPAGATION, 0, "unsupported method %s", strMethod.c_str());
	}

	if (sess->resources.end() == sess->resources.find(::TianShanIce::SRM::rtURI))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "[%s] could not find resource URI"), sess->ident.name.c_str());
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_C2PROPAGATION, 0, "could not find resource URI");
	}

	TianShanIce::ValueMap& resURI = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
	if (resURI.end() == resURI.find(CPHPM_FILENAME))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_C2PROPAGATION, 0, "could not find URI resource " CPHPM_FILENAME);
	}

	TianShanIce::Variant& var = resURI[CPHPM_FILENAME];
	if (var.type != TianShanIce::vtStrings || var.bRange || var.strs.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
			CPHPM_FILENAME);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_C2PROPAGATION, 0, "could not find URI resource " CPHPM_FILENAME);
	}

	TianShanIce::ValueMap& resBw = sess->resources[::TianShanIce::SRM::rtProvisionBandwidth].resourceData;
	if (resBw.end() == resBw.find(CPHPM_BANDWIDTH))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_C2PROPAGATION, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}

	TianShanIce::Variant& var1 = resBw[CPHPM_BANDWIDTH];
	if (var1.type != TianShanIce::vtLongs || var1.lints.size() <=0)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "[%s] could not find ProvisionBandwidth resource %s"), sess->ident.name.c_str(), 
			CPHPM_BANDWIDTH);
		ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_C2PROPAGATION, 0, "could not find ProvisionBandwidth resource " CPHPM_BANDWIDTH);
	}

	if(stricmp(strMethod.c_str(), METHODTYPE_CDN_HTTPPropagation)==0)
	{
		TianShanIce::ValueMap& resSrc = sess->resources[::TianShanIce::SRM::rtURI].resourceData;
		if (resSrc.end() == resSrc.find(CPHPM_SOURCEURL))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_C2PROPAGATION, 0, "could not find URI resource " CPHPM_SOURCEURL);
		}

		TianShanIce::Variant& var2 = resSrc[CPHPM_SOURCEURL];
		if (var2.type != TianShanIce::vtStrings || var2.bRange || var2.strs.size() <=0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "[%s] could not find URI resource %s"), sess->ident.name.c_str(), 
				CPHPM_SOURCEURL);
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_C2PROPAGATION, 0, "could not find URI resource " CPHPM_SOURCEURL);
		}

		std::string url = var2.strs[0];
		std::string strpro = url.substr(0,9);
		if (stricmp(strpro.c_str(),"c2http://")!= 0)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(CPH_C2PROPAGATION, "[%s] Can't find the c2 HTTP protocol from url %s"), sess->ident.name.c_str(), url.c_str());
			ZQTianShan::_IceThrow< ::TianShanIce::InvalidParameter>(CPH_C2PROPAGATION, TianShanIce::Storage::csexpInvalidSourceURL, "Can't find the c2 HTTP protocol from url[%s]" ,url.c_str());
		}

		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
	}
	else
	{
		sess->setSessionType(TianShanIce::ContentProvision::ptCatcher, TianShanIce::ContentProvision::stScheduled);
		MOLOG(Log::L_WARNING, CLOGFMT(CPH_C2PROPAGATION, "[%s] Unknown method[%s], setSessionType() with default type"), sess->ident.name.c_str(), strMethod.c_str());
	}

	return true;
}

bool C2PropagationSess::prime()
{
	if (!Start())
	{
		setErrorInfo(_nLastErrCode, (std::string("Failed to start graph with error: ") + _strLastErr).c_str());			
		MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] failed to start graph withe error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}

	if (!BaseCPHSession::prime())
		return false;

	MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] prime() successful"), _strLogHint.c_str());
	return true;
}

void C2PropagationSess::OnProgress(int64& prcvBytes)
{
	BaseGraph::OnProgress(prcvBytes);

	::TianShanIce::Properties params;
	char tmp[40];

	sprintf(tmp, "True");
	params[EVTPM_OPENFORWRITE] = tmp;
	//MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] OpenForWrite[%s]"), _strLogHint.c_str(),tmp);

	updateProgress(_llProcBytes, _llTotalBytes, params);
}

void C2PropagationSess::OnStreamable(bool bStreamable)
{
	BaseGraph::OnStreamable(bStreamable);
	notifyStreamable(bStreamable);
	MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void C2PropagationSess::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());

	char tmp[40];
	::TianShanIce::Properties params;
	sprintf(tmp, "%d", mInfo.bitrate);
	params[EVTPM_MPEGBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] bitrate[%s]"), _strLogHint.c_str(),tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionV);
	params[EVTPM_VIDEOHEIGHT] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] videoResolutionV[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoResolutionH);
	params[EVTPM_VIDEOWIDTH] =  tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] videoResolutionH[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp, "%d", mInfo.videoBitrate);
	params[EVTPM_VIDEOBITRATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] videoBitrate[%s]"), _strLogHint.c_str(), tmp);
	sprintf(tmp,"%.2f",mInfo.framerate);
	params[EVTPM_FRAMERATE] = tmp;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] framerate[%s]"), _strLogHint.c_str(),tmp);

	_bitrate = mInfo.bitrate;

	notifyStarted(params);
	_bStartEventSent = true;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_C2PROPAGATION, "[%s] notifyStarted() called"), _strLogHint.c_str());

}

C2PropagationHelper::C2PropagationHelper(ZQ::common::NativeThreadPool& pool, ZQTianShan::ContentProvision::ICPHManager* mgr)
:BaseCPHelper(pool, mgr) 
{
	C2PROPAGATION::Methods::iterator it;
	for (it = _gCPHCfg.methods.begin();it != _gCPHCfg.methods.end();it++)
	{
		if (!it->maxSession || !it->maxBandwidth)
			continue;

		_methodCostList[it->methodName] = new MethodCostI(it->maxBandwidth, it->maxSession);				
	}			
}

C2PropagationHelper::~C2PropagationHelper()
{
	MethodCostList::iterator it = _methodCostList.begin();
	for (;it!=_methodCostList.end();it++)
	{
		if (it->second)
			delete it->second;
	}

	_methodCostList.clear();
}

bool C2PropagationHelper::getBandwidthLoad(const char* methodType, long& bpsAllocated, long& bpsMax, long& initCost)
{
	if (stricmp(methodType, METHODTYPE_CDN_HTTPPropagation))
		return false;

	bpsAllocated =0;

	C2PROPAGATION::Methods::iterator methodIter;
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
bool C2PropagationHelper:: getLoad(const char* methodType, uint32& allocatedKbps, uint32& maxKbps, uint& sessions, uint& maxSessins)
{
	C2PROPAGATION::Methods::iterator methodIter;
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


