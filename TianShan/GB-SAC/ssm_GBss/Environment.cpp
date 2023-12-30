// File Name : Enviroment.cpp

#include "Environment.h"

// project configuration
#include "GBssConfig.h"

#include "StreamersConfig.h"

// project string utils
#include "stroprt.h"

// manager ice database
#include "CRGSessionManager.h"
#include "CRGSessionImpl.h"

// streamer selection
#include "GBssResourceManager.h"

// RTSP Method Handler
#include "SetupHandler.h"
#include "PlayHandler.h"
#include "PauseHandler.h"
#include "TeardownHandler.h"
#include "GetParameterHandler.h"
#include "DescribeHandler.h"

// stream event announce message
#include "StreamEventSinkI.h"

// generated from LAMFacade.ice
#include "LAMFacade.h"

// TianShan Common
#include "IceLog.h"

#include "TimeUtil.h"

#ifdef ZQ_OS_LINUX
#include <arpa/inet.h>
#endif//ZQ_OS_LINUX



#define ISVODLOG _fileLog

ZQ::common::Config::Loader<GBss::GBssConfig> _GBssConfig("");

namespace GBss
{

Environment::Environment()
:_site(NULL), _sessionManager(NULL), _sessionWatchDog(NULL), _iceStromClient(NULL), 
_selectionEnv(NULL), _resourceManager(NULL), _pEventAdapter(NULL), 
_pCommunicator(NULL), _pEventChannel(NULL), _bInit(false),_globalSequence(1)
{
}

Environment::~Environment()
{
}

bool Environment::loadConfig(const char* pConfigPath)
{
	ISVODLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "**************** [Load %s] ****************"), ZQ_FILE_NAME);
	std::string configPath = pConfigPath;
#ifdef ZQ_OS_MSWIN
	configPath += "\\ssm_GBss.xml";
#else
    configPath += "/ssm_GBss.xml";
#endif
	if (!_GBssConfig.load(configPath.c_str()))
	{
		ISVODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "loadConfig() failed to load config[%s]"), configPath.c_str());
		return false;
	}
	if (!_streamersConfig.load(_GBssConfig._sourceStreamers._fileName.c_str()))
	{
		ISVODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "loadConfig() failed to load config[%s]"), _GBssConfig._sourceStreamers._fileName.c_str());
		return false;
	}

	// adjust streaming resource configuration
	bool bAdjust = false;
	if (_streamersConfig._streamingResource._retryCount > 100)
	{
		bAdjust = true;
		_streamersConfig._streamingResource._retryCount = 100;
	}
	if (_streamersConfig._streamingResource._retryCount < 0 )
	{
		bAdjust = true;
		_streamersConfig._streamingResource._retryCount = 0;
	}
	
	if (_streamersConfig._streamingResource._maxPenaltyValue > 10000)
	{
		bAdjust = true;
		_streamersConfig._streamingResource._maxPenaltyValue = 10000;
	}
	if (_streamersConfig._streamingResource._maxPenaltyValue < 0)
	{
		bAdjust = true;
		_streamersConfig._streamingResource._maxPenaltyValue = 0;
	}
	if (bAdjust)
	{
		ISVODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "retryCount=%d maxPenaltyValue=%d"), _streamersConfig._streamingResource._retryCount, _streamersConfig._streamingResource._maxPenaltyValue);
	}
	if (0 == _GBssConfig._announce._SRMEnabled)
	{
		ISVODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "disable to send announce message to SRM"));
	}
	if (0 == _GBssConfig._announce._STBEnabled)
	{
		ISVODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "disable to send announce message to STB"));
	}

	// adjust streamer configuration
	std::vector<StreamerHolder>::iterator itStreamer = _streamersConfig._streamingResource._streamerDatas.begin();
	for (; itStreamer != _streamersConfig._streamingResource._streamerDatas.end(); itStreamer++)
	{
		//adjust streamer endpoint
		std::string& streamerEndpoint = itStreamer->_serviceEndpoint;
		if (std::string::npos == streamerEndpoint.find(":"))
		{
			streamerEndpoint = "StreamSmith:" + streamerEndpoint;			
		}
		ISVODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "Streamer netId=%s, serviceEndpoint=%s"), (itStreamer->_netId).c_str(), streamerEndpoint.c_str());
	} // end for streamers
	if( _streamersConfig._streamingResource._streamerDatas.empty())
	{
		ISVODLOG(ZQ::common::Log::L_ERROR,CLOGFMT(Environment,"No streamers is found ,please check the configuratio file"));
		return false;
	}

	return true;
}

std::string Environment::getProxyString(const Ice::ObjectPrx& objectPrx) const
{
	return _pCommunicator->proxyToString(objectPrx);
}

int Environment::doInit(const char* pConfigPath, const char* pLogDir, IStreamSmithSite* pSite)
{
	doUninit();
	if (NULL == pConfigPath || strlen(pConfigPath) <= 0)
	{
		_sysLog(ZQ::common::Log::L_EMERG, CLOGFMT(Environment, "configuration directory is empty"));
		return 1;
	}

	if (NULL == pLogDir || strlen(pLogDir) <= 0)
	{
		_sysLog(ZQ::common::Log::L_EMERG, CLOGFMT(Environment, "log directory is empty"));
		return 1;
	}

	if (NULL == pSite)
	{
		_sysLog(ZQ::common::Log::L_EMERG, CLOGFMT(Environment, "Stream Smith Sitesi is empty"));
		return 1;
	}

	// open plugin log file
	std::string logDir = pLogDir;
#ifdef ZQ_OS_MSWIN
	logDir += "\\ssm_GBss";
#else
    logDir += "/ssm_GBss";
#endif
	std::string strFileLog = logDir + ".log";
	_fileLog.open(strFileLog.c_str(), ZQ::common::Log::L_DEBUG);

	// load configuration
	_GBssConfig.setLogger(&_fileLog);
	if (!loadConfig(pConfigPath))
	{
        _fileLog.flush();
		return 1;
	}

	// set file log properties from configuration
	_fileLog.setLevel(_GBssConfig._pluginLog._level);
	_fileLog.setFileSize(_GBssConfig._pluginLog._size);
	_fileLog.setFileCount(_GBssConfig._pluginLog._maxCount);
	_fileLog.setBufferSize(_GBssConfig._pluginLog._bufferSize);

	// open ice trace log
	std::string strIceTraceLog = logDir + "_iceTrace.log";
	_iceLog.open(strIceTraceLog.c_str(), _GBssConfig._iceTrace._level, _GBssConfig._iceTrace._maxCount, 
		_GBssConfig._iceTrace._size);

	// create global session
	_site = pSite;
	IClientSession* pSession = pSite->createClientSession(NULL, "rtsp://defaultSite/EventISVODInterface5?asset=1");
	if (NULL == pSession || NULL == pSession->getSessionID())
	{
		ISVODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(Environment, "failed to create global session"));
        _fileLog.flush();
		return 1;
	}
	_globalSession = pSession->getSessionID();

	// initial ICE Run Time
	if (!initIceRunTime())
	{
		ISVODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "failed to initialize Ice run-time"));
        _fileLog.flush();
		return 1;
	}

	// start sesssion watch dog to monitor session
	_sessionWatchDogPool.resize(_GBssConfig._rtspSession._monitorThreads);
	try
	{
		_sessionWatchDog = new ZQTianShan::TimerWatchDog(_fileLog, _sessionWatchDogPool, _pEventAdapter);
		_iceStromClient = new IceStormClient(*this);
	}
	catch (const std::bad_alloc& ex)
	{
		ISVODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "caught [%s] when create session watch dog or Ice Strorm Client"), ex.what());
        _fileLog.flush();
		return 1;
	}
	_sessionWatchDog->start();
	_iceStromClient->start();

	// 
	if (!initResourceManager())
	{
        _fileLog.flush();
		return 1;
	}

	// open database for session contexts
	try
	{
		_sessionManager = new CRGSessoionManager(*this, _fileLog, _pCommunicator, _pEventAdapter);
	}
	catch (const std::bad_alloc& ex)
	{
		ISVODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "caught [%s] when created session manager"), ex.what());
        _fileLog.flush();
		return 1;
	}
	if (!_sessionManager->openDB(_GBssConfig._database._path, _GBssConfig._rtspSession._cacheSize))
	{
		ISVODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "failed to open ICE Freeze database"));
        _fileLog.flush();
		return 1;
	}
	_sessionManager->start();

	_bInit = true;
	ISVODLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "successed to initialize EventIS VOD enviroment"));
	return 0;
}

int Environment::doUninit()
{
	ISVODLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "do uninitialization [%s]"), ZQ_PRODUCT_NAME);
	if (!_bInit)
	{
		return 1;
	}

	if (_sessionManager)
	{
		_sessionManager->closeDB();
		_sessionManager->stop();
		delete _sessionManager;
		_sessionManager = NULL;
		ISVODLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "successed to uninitalize session manager"));
	}

	if (_sessionWatchDog)
	{
		_sessionWatchDog->quit();
		delete _sessionWatchDog;
		_sessionWatchDog = NULL;
		ISVODLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "successed to uninitalize session watch dog"));
	}
	_sessionWatchDogPool.stop();

	if (_iceStromClient)
	{
		_iceStromClient->stop();
		delete _iceStromClient;
		_iceStromClient = NULL;
		ISVODLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "successed to uninitalize ice storm client"));
	}

	if (_resourceManager)
	{
		_resourceManager->stop();
		delete _resourceManager;
		_resourceManager = NULL;
		ISVODLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "successed to uninitalize resource manager"));
	}

	if (_selectionEnv)
	{
		delete _selectionEnv;
		_selectionEnv = NULL;
	}

	if (_pEventChannel)
	{
		_pEventChannel->clean();
	}

	uninitIceRunTime();

	_pEventChannel = NULL;

	// destroy global session
	if (_site)
	{
		_site->destroyClientSession(_globalSession.c_str());
		_site = NULL;
	}
	ISVODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "uninitialization [%s] successfully"), ZQ_PRODUCT_NAME);

	// flush log before quit
	_sysLog.flush();
	_fileLog.flush();
	_iceLog.flush();
	_bInit = false;
	return 0;
}

RequestProcessResult Environment::doFixupRequest(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	// translate response to request
	if (pReq && pReq->getVerb() == RTSP_MTHD_RESPONSE)
	{
		char szBuf[2048];
		memset(szBuf, 0, sizeof(szBuf));
		pReq->getStartline(szBuf, sizeof(szBuf) - 1);
		std::vector<std::string> starts;
		ZQ::StringOperation::splitStr(szBuf, " \r\n\t", starts);
		if (starts.size() >= 2 && (strcmp(starts[1].c_str(), "200") == 0 || strcmp(starts[1].c_str(), "454") == 0))
		{
			if (strcmp(starts[1].c_str(), "200") == 0)
			{
				pReq->setArgument(RTSP_MTHD_GET_PARAMETER, "*", starts[0].c_str());
			}
			else
			{
				pReq->setArgument(RTSP_MTHD_TEARDOWN, "*", starts[0].c_str());
			}
		}
		pReq->setHeader("NeedResponse", "no");
	}
	return RequestProcessed;
}

RequestProcessResult Environment::doContentHandler(IStreamSmithSite* pSite, IClientRequestWriter* pReq)
{
	try
	{
		RequestHandler::Ptr pRequestHandler = NULL;
		switch (pReq->getVerb())
		{
		case RTSP_MTHD_SETUP:
			pRequestHandler = new SetupHandler(_fileLog, *this, pSite, pReq);
			break;
		case RTSP_MTHD_TEARDOWN:
			pRequestHandler = new TeardownHandler(_fileLog, *this, pSite, pReq);
			break;
		case RTSP_MTHD_GET_PARAMETER:
			pRequestHandler = new GetParameterHandler(_fileLog, *this, pSite, pReq);
			break;
		case RTSP_MTHD_PLAY:
			pRequestHandler = new PlayHandler(_fileLog, *this, pSite, pReq);
			break;
		case RTSP_MTHD_PAUSE:
			pRequestHandler = new PauseHandler(_fileLog, *this, pSite, pReq);
			break;
		case RTSP_MTHD_DESCRIBE:
			pRequestHandler = new DescribeHandler(_fileLog, *this, pSite, pReq);
			break;
		default:
			pRequestHandler = new RequestHandler(_fileLog, *this, pSite, pReq);
			break;
		}
		return pRequestHandler->process();
	}
	catch (const std::bad_alloc& ex)
	{
		ISVODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "caught [%s] when created request handler"), ex.what());
		return RequestError;
	}
}

RequestProcessResult Environment::doFixupResponse(IStreamSmithSite* pSite, IClientRequest* pReq)
{
	return RequestDone;
}

bool Environment::initIceRunTime()
{
	try
	{
		// DO: initialize properties for ice run-time
		int i=0;
		Ice::PropertiesPtr props = Ice::createProperties(i, NULL);
		if (NULL != props)
		{
			std::vector<GBss::IceProperty::IcePropertyHolder>::iterator itIceProp;
			itIceProp = _GBssConfig._iceProps._propDatas.begin();
			for (; itIceProp != _GBssConfig._iceProps._propDatas.end(); itIceProp ++)
			{
				props->setProperty(itIceProp->_name, itIceProp->_value);
			}
		}

		// DO: create communicator
		Ice::InitializationData idt;
		idt.properties = props;
		if (_GBssConfig._iceTrace._enabled > 0)
		{
			idt.logger = new TianShanIce::common::IceLogI(&_iceLog);
		}
		_pCommunicator=Ice::initialize(i, NULL, idt);
	}
	catch (const std::bad_alloc&ex)
	{
		ISVODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(Environment, "caught [%s] when initial Ice Runtime"), ex.what());
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		ISVODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(Environment, "caught [%s] when created Ice Communicator"), ex.ice_name().c_str());
		return false;
	}

	try
	{
		_pEventAdapter = ZQADAPTER_CREATE(_pCommunicator, _GBssConfig._bind._serviceName.c_str(), _GBssConfig._bind._endpoint.c_str(), ISVODLOG);
#ifndef _INDEPENDENT_ADAPTER
		std::vector<PublishLogHolder>& logDatas = _GBssConfig._publishLogs._logDatas;
		std::vector<PublishLogHolder>::const_iterator iter = logDatas.begin();
		for (; iter != logDatas.end(); iter++)
		{
			if (!_pEventAdapter->publishLogger(iter->_path.c_str(), iter->_syntax.c_str(), iter->_key.c_str()))
			{
				ISVODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "failed to publish logger name[%s] synax[%s] key[%s]"), iter->_path.c_str(), iter->_syntax.c_str(),iter->_key.c_str());
			}
			else
			{
				ISVODLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "successed to publish logger name[%s] synax[%s] key[%s]"), iter->_path.c_str(), iter->_syntax.c_str() , iter->_key.c_str());
			}
		}
#endif
		_pEventAdapter->activate();

		GBssServantImplPtr servantImplPtr = new GBssServantImpl(_fileLog, *this);
		_pEventAdapter->ZQADAPTER_ADD(_pCommunicator, servantImplPtr, "ReplicaSubscriber");
	}
	catch (const std::bad_alloc& ex)
	{
		ISVODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(Environment, "caught [%s] when initial Ice Runtime"), ex.what());
		return false;
	}
	catch (const Ice::Exception& ex) 
	{
		ISVODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(Environment, "caught [%s] when created ICE adapter: [%s] "), _GBssConfig._bind._endpoint.c_str(), ex.ice_name().c_str());
		return false;
	}
	
	return true;
}

void Environment::uninitIceRunTime()
{
	try
	{
		if (NULL != _pEventAdapter)
		{
			_pEventAdapter->deactivate();
			_pEventAdapter = NULL;
		}
	}
	catch (const Ice::Exception& ex)
	{
		ISVODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(Environment, "deactive adapters caught [%s]"), ex.ice_name().c_str());
	}
	try
	{
		if (NULL != _pCommunicator)
		{
			_pCommunicator->destroy();
			_pCommunicator = NULL;
		}
	}
	catch (const Ice::Exception& ex)
	{
		ISVODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(Environment, "destroy communicator caught [%s]"), ex.ice_name().c_str());
	}
}

std::string Environment::getAnnounceSequence()
{
	ZQ::common::MutexGuard lk(_sequenceLock);
	if (_globalSequence == UINT_MAX)
	{
		_globalSequence = 1;
	}
	char buff[20];
	buff[sizeof(buff) - 1] = '\0';
	snprintf(buff, sizeof(buff) - 1, "%d", _globalSequence++);
	return buff;
}

bool Environment::ConnectIceStorm(const std::string& endpoint)
{
#if defined _DEBUG || defined DEBUG
#pragma message(__MSGLOC__"TODO : remove this line in release version")
	return false;
#endif
	try
	{
		ISVODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "do connect to EventChannel: [%s]"), endpoint.c_str());
		_sEvent = new StreamEventSinkI(_fileLog, *this);
		TianShanIce::Streamer::PlaylistEventSinkPtr _playlistEvent = new PlayListEventSinkI(_fileLog, *this);
		_pEventChannel=new TianShanIce::Events::EventChannelImpl(_pEventAdapter, endpoint.c_str());
		TianShanIce::Properties qos;
		_pEventChannel->sink(_sEvent, qos);
		_pEventChannel->sink(_playlistEvent, qos);
		ISVODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "connect to EventChannel: [%s] successfully"), endpoint.c_str());
	}
	catch (const std::bad_alloc& ex)
	{
		ISVODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(Environment, "caught [%s] when connect Ice storm"), ex.what());
		return false;
	}
	catch (const TianShanIce::BaseException& ex)
	{
		ISVODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "connect to EventChannel: [%s] caught %s:%s"), endpoint.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		ISVODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "connect to EventChannel: [%s] caught an %s"), endpoint.c_str(), ex.ice_name().c_str());
		return false;
	}
	return true;
}

bool Environment::initResourceManager()
{
	using namespace com::izq::am::facade::servicesForIce;
	try
	{
		_selectionEnv = new SelectionEnv();
	}
	catch (const std::bad_alloc& ex)
	{
		ISVODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "caught [%s] create selection enviroment"), ex.what());
		return false;
	}

	_selectionEnv->mLogger = &_fileLog;
	_selectionEnv->lamProxy = NULL;
	_selectionEnv->mMaxPenaltyValue = _streamersConfig._streamingResource._maxPenaltyValue;
	_selectionEnv->mReplicaUpdateInterval = _streamersConfig._streamingResource._replicaUpdateInterval*1000;//convert second to millisecond

	_selectionEnv->mbContentTestMode = _GBssConfig._lam._lamTestMode._enabled > 0;
	_selectionEnv->mbGBMode = true;
	_selectionEnv->mNasUrlPrefix = _GBssConfig._lam._nasurlPrefix;
	_selectionEnv->mbStreamerReplicaTestMode = _streamersConfig._streamingResource._replicaUpdateEnable <= 0;
	if (_GBssConfig._lam._lamTestMode._enabled > 0)
	{
		_selectionEnv->mbStreamerReplicaTestMode = true;
		_selectionEnv->mbIcReplicaTestMode = true;
		ISVODLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "Lam test mode is enbaled"));
	}
	LAMTestMode::LAMSubTestSetHolderS& testData = _GBssConfig._lam._lamTestMode._subTests;
	LAMTestMode::LAMSubTestSetHolderS::const_iterator subTestIter = testData.begin();
	for (; subTestIter != testData.end(); subTestIter++)
	{
		using namespace  ::com::izq::am::facade::servicesForIce;
		AEInfo3 info;
		info.name		= subTestIter->_contentName;
		info.bandWidth	= subTestIter->_bandwidth;
		info.cueIn		= subTestIter->_cueIn;
		info.cueOut		= subTestIter->_cueOut;
		info.nasUrls	= subTestIter->_urls;
		info.volumeList	= subTestIter->_volumeList;
		_selectionEnv->mTestModeContent.push_back(info);
	}

	try
	{
		_resourceManager = new GBssResourceManager(*_selectionEnv);
	}
	catch (const std::bad_alloc& ex)
	{
		ISVODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "caught [%] when created resource manager"), ex.what());
		return false;
	}

	// import Streamer Config
	SOPS sops;
	ResourceStreamerAttrMap streamersMap;
	std::vector<StreamerHolder>::iterator itStreamer = _streamersConfig._streamingResource._streamerDatas.begin();
	for (; itStreamer != _streamersConfig._streamingResource._streamerDatas.end(); itStreamer++)
	{
		ResourceStreamerAttr resourceStreamerAttr;
		resourceStreamerAttr.sourceIP = inet_addr((itStreamer->_source).c_str());
		resourceStreamerAttr.bMaintainEnable = itStreamer->_adminEnabled > 0;
		resourceStreamerAttr.netId = itStreamer->_netId;
		resourceStreamerAttr.maxBw = static_cast<int64>(itStreamer->_totalBW) * 1000;
		resourceStreamerAttr.maxSessCount = itStreamer->_maxStream;
		resourceStreamerAttr.importChannelName = itStreamer->_importChannel;
		try
		{
            resourceStreamerAttr.endpoint = itStreamer->_serviceEndpoint;
			Ice::ObjectPrx objectPrx = _pCommunicator->stringToProxy(itStreamer->_serviceEndpoint);
			if (_streamersConfig._streamingResource._enableWarmup > 0)
			{
				resourceStreamerAttr.streamServicePrx = TianShanIce::Streamer::StreamSmithAdminPrx::uncheckedCast(objectPrx);
			}
			else
			{
				resourceStreamerAttr.streamServicePrx = TianShanIce::Streamer::StreamSmithAdminPrx::uncheckedCast(objectPrx);
			}
		}
		catch (const Ice::Exception&)
		{
			// do nothing
		}

		std::string associatedVolume = itStreamer->_volume; //parse volume into NetId and volume-name
		std::string::size_type pos = associatedVolume.find("/");
		if( pos != std::string::npos )
		{
			resourceStreamerAttr.volumeNetId = associatedVolume.substr(0, pos);
			std::string strTemp = associatedVolume.substr(pos + 1);
			if( strTemp.find("*") != std::string::npos )
			{
				resourceStreamerAttr.bAllPartitionAvail = true;
			}
			else
			{
				(resourceStreamerAttr.availPartition).insert(strTemp);
			}
		} // end if ( pos != std::string::npos )
		streamersMap.insert(std::make_pair(itStreamer->_netId, resourceStreamerAttr));
		ISVODLOG(ZQ::common::Log::L_INFO,CLOGFMT(Environment,"insert a new streamer [%s] to sop[test]"),
			itStreamer->_netId.c_str());
		_IdToSourceMap.insert(std::make_pair(itStreamer->_netId, itStreamer->_source));
	} // end for 
	sops.insert(std::make_pair("test", streamersMap));
	_resourceManager->updateSopData(sops);

	// import ImportChannel
	ResourceImportChannelAttrMap importChannelMap;
	std::vector<ImportChannelHolder>::iterator itImportChannel;
	itImportChannel = _GBssConfig._passThruStreaming._importChannelDatas.begin();
	for (; itImportChannel != _GBssConfig._passThruStreaming._importChannelDatas.end(); itImportChannel++)
	{
		ResourceImportChannelAttr importChannelAttr;
		importChannelAttr.netId = itImportChannel->_name;
		importChannelAttr.confMaxBw = itImportChannel->_bandwidth;
		importChannelAttr.confMaxSessCount = itImportChannel->_maxImport;
		importChannelMap.insert(std::make_pair(itImportChannel->_name, importChannelAttr));
	}
	_resourceManager->updateImportChannelData(importChannelMap);

	// import volume data
	ResourceVolumeAttrMap volumeMap;
	ContentVolumeHolderS::iterator itVolume = _GBssConfig._lam._lamServer._contentVolumes.begin();
	for (; itVolume != _GBssConfig._lam._lamServer._contentVolumes.end(); itVolume++)
	{
		ResourceVolumeAttr volumeAttr;
		volumeAttr.netId = itVolume->_name;
		volumeAttr.level = itVolume->_cacheLevel;
		volumeAttr.bSupportNas = itVolume->_supportNasStreaming;
		volumeAttr.bAllPartitions = false;

		const std::string& storageName = itVolume->_name;
		std::string::size_type posSlash = storageName.find("/");
		if( posSlash != std::string::npos )
		{
			volumeAttr.netId = storageName.substr(0, posSlash);
			std::string strTemp = storageName.substr(posSlash + 1);
			if( strTemp.find("*") != std::string::npos )
			{
				volumeAttr.bAllPartitions = true;
			}
			else
			{
				volumeAttr.partitions.insert(strTemp);
			}
		}
		volumeMap.insert(std::make_pair(volumeAttr.netId, volumeAttr));
	}
	_resourceManager->updateVolumesData(volumeMap);

	if (!_resourceManager->initResourceManager(_streamersConfig._streamingResource._enableWarmup > 0))
	{
		ISVODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "failed to initial resource manager"));
		return false;
	}
	_resourceManager->start();
	return true;

}

std::string Environment::getStreamerSource(const std::string& streamerNetId) const
{
	std::map<std::string, std::string>::const_iterator iter = _IdToSourceMap.find(streamerNetId);
	if (iter == _IdToSourceMap.end())
	{
		return "";
	}
	return iter->second;
}

void Environment::addIntoSRMMap(const std::string& SRMID, const std::string& SRMConnectionId)
{
	ZQ::common::MutexGuard lk(_SRMMapLock);
	std::map<std::string, std::string>::iterator iter = _SRMMap.find(SRMID);
	if (iter != _SRMMap.end())
	{
		iter->second = SRMConnectionId;
	}
	else
	{
		_SRMMap.insert(std::make_pair(SRMID, SRMConnectionId));
	}
	ISVODLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "authorize SRM [%s] with Connection ID[%s]"), SRMID.c_str(), SRMConnectionId.c_str());
}

void Environment::getSRMConnectionIDs(std::vector<std::string> &connections) const
{
	ZQ::common::MutexGuard lk(_SRMMapLock);
	std::map<std::string, std::string>::const_iterator iter = _SRMMap.begin();
	for (; iter != _SRMMap.end(); iter++)
	{
		connections.push_back(iter->second);
	}
}

bool Environment::authorized(const std::string &srmConnID) const
{
	ZQ::common::MutexGuard lk(_SRMMapLock);
	std::map<std::string, std::string>::const_iterator iter = _SRMMap.begin();
	for (; iter != _SRMMap.end(); iter++)
	{
		if (iter->second == srmConnID)
		{
			return true;
		}
	}
//	ISVODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(Environment, "unauthorized connection id [%s]"), srmConnID.c_str());
	return false;
}

IStreamSmithSite& Environment::getStreamSmithSite()
{
	return *_site;
}

const std::string& Environment::getGlobalSession() const
{
	return _globalSession;
}

NgodResourceManager& Environment::getResourceManager()
{
	return *_resourceManager;
}

SelectionEnv& Environment::getSelectionEnv()
{
	return *_selectionEnv;
}

CRGSessoionManager& Environment::getSessionManager()
{
	return *_sessionManager;
}

TianShanIce::Streamer::StreamEventSinkPtr& Environment::getStreamEventSink()
{
	return _sEvent;
}

void Environment::watchSession(::Ice::Identity& ident, long timeout)
{
	if (0 == timeout)
	{
		timeout = long(_GBssConfig._rtspSession._timeout * 1000);
	}
	_sessionWatchDog->watch(ident, timeout);
}

void Environment::watchSession(const std::string& sessId, long timeout)
{
	Ice::Identity ident = _sessionManager->getIdentity(sessId);
	watchSession(ident, timeout);
}

std::string Environment::getUTCTime() const
{
	char buff[64];
	buff[sizeof(buff) - 1] = '\0';
#ifdef ZQ_OS_LINUX
	struct timeval systemTime;
	gettimeofday(&systemTime,(struct timezone*)NULL);
#else
	SYSTEMTIME systemTime;
	GetSystemTime(&systemTime);
#endif
	
	ZQ::common::TimeUtil::Time2Iso(systemTime, buff, sizeof(buff) - 1);

	return buff;

}

//-----------------------------------------------------------------
Environment::IceStormClient::IceStormClient(Environment& env)
:_bQuit(false), _env(env)
{

}

Environment::IceStormClient::~IceStormClient()
{

}

bool Environment::IceStormClient::init()
{
	return true;
}

int Environment::IceStormClient::run()
{
	timeout_t dwSleepTime = 10 * 1000;
	while (!_bQuit)
	{
		if (_env.ConnectIceStorm(_GBssConfig._iceStorm._endpoint))
		{
			_bQuit = true;
			break;
		}

		_hEvent.wait(dwSleepTime);
	}
	return 1;
}

void Environment::IceStormClient::final()
{

}

void Environment::IceStormClient::stop()
{
	if(!_bQuit)
	{
		_bQuit = true;
		_hEvent.signal();
		waitHandle(5000);
	}
}

} // end __EVENT_IS_VODI5_ENVIROMENT_H__
