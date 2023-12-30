// File Name : Enviroment.cpp

#include "Environment.h"

// project configuration
#include "OpenVBOConfig.h"

#include "StreamersConfig.h"

// project string utils
#include "stroprt.h"

// manager ice database
#include "CRGSessionManager.h"
#include "CRGSessionImpl.h"

// streamer selection
#include "OpenVBOResourceManager.h"

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

extern "C" {
#ifdef ZQ_OS_LINUX
#include <arpa/inet.h>
#endif
}


#define ISVODLOG _fileLog

ZQ::common::Config::Loader<EventISVODI5::OpenVBOConfig> _openVBOConfig("");

namespace EventISVODI5
{

Environment::Environment()
:_site(NULL), _sessionManager(NULL), _sessionWatchDog(NULL), _iceStromClient(NULL), _updateSnmpTimer(NULL),
_selectionEnv(NULL), _resourceManager(NULL), _pEventAdapter(NULL), 
_mmib(_fileLog, 1000, 4),
_snmpSA(_fileLog, _mmib, 5000),
_pCommunicator(NULL), _pEventChannel(NULL), _bInit(false),_globalSequence(1),_eventDispatch(new StreamEventDispatcher(_fileLog, *this))
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
	configPath += "\\ssm_OpenVBO.xml";
#else
    configPath += "/ssm_OpenVBO.xml";
#endif
	if (!_openVBOConfig.load(configPath.c_str()))
	{
		ISVODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "loadConfig() failed to load config[%s]"), configPath.c_str());
		return false;
	}
	if (!_streamersConfig.load(_openVBOConfig._sourceStreamers._fileName.c_str()))
	{
		ISVODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "loadConfig() failed to load config[%s]"), _openVBOConfig._sourceStreamers._fileName.c_str());
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
	if (0 == _openVBOConfig._announce._SRMEnabled)
	{
		ISVODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "disable to send announce message to SRM"));
	}
	if (0 == _openVBOConfig._announce._STBEnabled)
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
			ISVODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "Streamer netId=%s, serviceEndpoint=%s"), (itStreamer->_netId).c_str(), streamerEndpoint.c_str());
		}
	} // end for streamers

	//adjust LAM Server
	std::string& lamServerEndpoint = _openVBOConfig._lam._lamServer._endpoint;
	if (lamServerEndpoint.find(":") == std::string::npos)
	{
		lamServerEndpoint = "LAMFacade:" + lamServerEndpoint;
		ISVODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "LAMServer serviceEndpoint=%s"), lamServerEndpoint.c_str());
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
	logDir += "\\ssm_OpenVBO";
#else
    logDir += "/ssm_OpenVBO";
#endif
	std::string strFileLog = logDir + ".log";
	_fileLog.open(strFileLog.c_str(), ZQ::common::Log::L_DEBUG);

	// load configuration
	_openVBOConfig.setLogger(&_fileLog);
	if (!loadConfig(pConfigPath))
	{
        _fileLog.flush();
		return 1;
	}

//	_openVBOConfig.snmpRegister("");
	// set file log properties from configuration
	_fileLog.setLevel(_openVBOConfig._pluginLog._level);
	_fileLog.setFileSize(_openVBOConfig._pluginLog._size);
	_fileLog.setFileCount(_openVBOConfig._pluginLog._maxCount);
	_fileLog.setBufferSize(_openVBOConfig._pluginLog._bufferSize);

	// open ice trace log
	std::string strIceTraceLog = logDir + "_iceTrace.log";
	_iceLog.open(strIceTraceLog.c_str(), _openVBOConfig._iceTrace._level, _openVBOConfig._iceTrace._maxCount, 
		_openVBOConfig._iceTrace._size);

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
	ISVODLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "initialized global session[%s]"), _globalSession.c_str());

	// initial ICE Run Time
	if (!initIceRunTime())
	{
		ISVODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "failed to initialize Ice run-time"));
        _fileLog.flush();
		return 1;
	}

	// start sesssion watch dog to monitor session
	_sessionWatchDogPool.resize(_openVBOConfig._rtspSession._monitorThreads);
	try
	{
		_sessionWatchDog = new ZQTianShan::TimerWatchDog(_fileLog, _sessionWatchDogPool, _pEventAdapter);
		_iceStromClient = new IceStormClient(*this);
        _updateSnmpTimer = new UpdateSNMPTableTimer(*this);
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

	registerSnmp(pSite);

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
	if (!_sessionManager->openDB(_openVBOConfig._database._runtimePath, _openVBOConfig._rtspSession._cacheSize))
	{
		ISVODLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "failed to open ICE Freeze database"));
        _fileLog.flush();
		return 1;
	}
	_sessionManager->start();
    _updateSnmpTimer->start();

	_pEventAdapter->activate();

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

    if (_updateSnmpTimer)
    {
        _updateSnmpTimer->stop();
        ISVODLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "successed to uninitalize update snmp client"));
    }

	//if (_vboSnmpAgent)
	//{
	//	_vboSnmpAgent->stop();
	//	delete _vboSnmpAgent;
	//	_vboSnmpAgent = NULL;
	//}

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
			ISVODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(Environment, "unsupported verb[%d]"), pReq->getVerb());
			pRequestHandler = new RequestHandler(_fileLog, *this, pSite, pReq);
			break;
		}

		if (pRequestHandler)
			return pRequestHandler->process();
	}
	catch (const std::bad_alloc& ex)
	{
		ISVODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "caught [%s] when created request handler"), ex.what());
		return RequestError;
	}
	
	return RequestError;
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
			std::vector<EventISVODI5::IceProperty::IcePropertyHolder>::iterator itIceProp;
			itIceProp = _openVBOConfig._iceProps._propDatas.begin();
			for (; itIceProp != _openVBOConfig._iceProps._propDatas.end(); itIceProp ++)
			{
				props->setProperty(itIceProp->_name, itIceProp->_value);
			}
		}

		// DO: create communicator
		Ice::InitializationData idt;
		idt.properties = props;
		if (_openVBOConfig._iceTrace._enabled > 0)
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
		_pEventAdapter = ZQADAPTER_CREATE(_pCommunicator, _openVBOConfig._bind._serviceName.c_str(), _openVBOConfig._bind._endpoint.c_str(), ISVODLOG);
#ifndef _INDEPENDENT_ADAPTER
		std::vector<PublishLogHolder>& logDatas = _openVBOConfig._publishLogs._logDatas;
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

		OpenVBOServantImplPtr servantImplPtr = new OpenVBOServantImpl(_fileLog, *this);
		_pEventAdapter->ZQADAPTER_ADD(_pCommunicator, servantImplPtr, "ReplicaSubscriber");
	}
	catch (const std::bad_alloc& ex)
	{
		ISVODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(Environment, "caught [%s] when initial Ice Runtime"), ex.what());
		return false;
	}
	catch (const Ice::Exception& ex) 
	{
		ISVODLOG(ZQ::common::Log::L_EMERG, CLOGFMT(Environment, "caught [%s] when created ICE adapter: [%s] "), _openVBOConfig._bind._endpoint.c_str(), ex.ice_name().c_str());
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
		_sEvent = new StreamEventSinkI(_fileLog, *this, *_eventDispatch);
		TianShanIce::Streamer::PlaylistEventSinkPtr _playlistEvent = new PlayListEventSinkI(_fileLog, *this, *_eventDispatch);
		_pEventChannel=new TianShanIce::Events::EventChannelImpl(_pEventAdapter, endpoint.c_str());
		_gEvent = new PauseTimeoutSinkI(_fileLog, *this, *_eventDispatch);
		TianShanIce::Properties qos;
		_pEventChannel->sink(_sEvent, qos);
		_pEventChannel->sink(_playlistEvent, qos);
		_pEventChannel->sink(_gEvent, qos, TianShanIce::Events::TopicStreamPauseTimeoutEvent);

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
	LAMFacadePrx lamProxy;
	try
	{
		Ice::ObjectPrx objectPrx = _pCommunicator->stringToProxy(_openVBOConfig._lam._lamServer._endpoint);
		if (_openVBOConfig._lam._enableWarmup > 0 &&  0 == _openVBOConfig._lam._lamTestMode._enabled)
		{
			lamProxy = LAMFacadePrx::checkedCast(objectPrx);

		}
		else
		{
			lamProxy = LAMFacadePrx::uncheckedCast(objectPrx);
		}
		ISVODLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "successed to get LAM Server[%s] proxy"), _openVBOConfig._lam._lamServer._endpoint.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		ISVODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(Environment, "got LAM Server [%s] failed with exception[%s]"), _openVBOConfig._lam._lamServer._endpoint.c_str(),ex.ice_name().c_str());
		lamProxy = NULL;
	}

	_selectionEnv->mLogger = &_fileLog;
	_selectionEnv->lamProxy = lamProxy;
	_selectionEnv->mMaxPenaltyValue = _streamersConfig._streamingResource._maxPenaltyValue;
	_selectionEnv->mReplicaUpdateInterval = _streamersConfig._streamingResource._replicaUpdateInterval*1000;//convert second to millisecond

	_selectionEnv->mbContentTestMode = _openVBOConfig._lam._lamTestMode._enabled > 0;
	_selectionEnv->mbStreamerReplicaTestMode = _streamersConfig._streamingResource._replicaUpdateEnable <= 0;
	if (_openVBOConfig._lam._lamTestMode._enabled > 0)
	{
		_selectionEnv->mbStreamerReplicaTestMode = true;
		_selectionEnv->mbIcReplicaTestMode = true;
		ISVODLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "Lam test mode is enbaled"));
	}
	LAMTestMode::LAMSubTestSetHolderS& testData = _openVBOConfig._lam._lamTestMode._subTests;
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
		_resourceManager = new OpenVBOResourceManager(*_selectionEnv);
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
		resourceStreamerAttr.sourceAddr = itStreamer->_source;
		resourceStreamerAttr.sourceIP = inet_addr(resourceStreamerAttr.sourceAddr.c_str());
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

		resourceStreamerAttr.volume = itStreamer->_volume; //parse volume into NetId and volume-name
		std::string::size_type pos = resourceStreamerAttr.volume.find("/");
		if( pos != std::string::npos )
		{
			resourceStreamerAttr.volumeNetId = resourceStreamerAttr.volume.substr(0, pos);
			std::string strTemp = resourceStreamerAttr.volume.substr(pos + 1);
			if( strTemp.find("*") != std::string::npos )
				resourceStreamerAttr.bAllPartitionAvail = true;
			else
				(resourceStreamerAttr.availPartition).insert(strTemp);
		} // end if ( pos != std::string::npos )
		streamersMap.insert(std::make_pair(itStreamer->_netId, resourceStreamerAttr));
		_IdToSourceMap.insert(std::make_pair(itStreamer->_netId, itStreamer->_source));
	} // end for 

	sops.insert(std::make_pair("test", streamersMap));
	_resourceManager->updateSopData(sops);

	// import ImportChannel
	ResourceImportChannelAttrMap importChannelMap;
	std::vector<ImportChannelHolder>::iterator itImportChannel;
	itImportChannel = _openVBOConfig._passThruStreaming._importChannelDatas.begin();
	for (; itImportChannel != _openVBOConfig._passThruStreaming._importChannelDatas.end(); itImportChannel++)
	{
		ResourceImportChannelAttr importChannelAttr;
		importChannelAttr.netId = itImportChannel->_name;
		importChannelAttr.confMaxBw = ((int64)itImportChannel->_bandwidth)*1000;//kbps
		importChannelAttr.confMaxSessCount = itImportChannel->_maxImport;
		importChannelMap.insert(std::make_pair(itImportChannel->_name, importChannelAttr));
	}
	_resourceManager->updateImportChannelData(importChannelMap);

	// import volume data
	ResourceVolumeAttrMap volumeMap;
	ContentVolumeHolderS::iterator itVolume = _openVBOConfig._lam._lamServer._contentVolumes.begin();
	for (; itVolume != _openVBOConfig._lam._lamServer._contentVolumes.end(); itVolume++)
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

void Environment::registerSRMConn(const std::string& SRMID, const std::string& SRMConnectionId)
{
	if (SRMID.empty())
		return;

	{
		ZQ::common::MutexGuard lk(_SRMMapLock);
		Props::iterator it = _SRM2Conn.find(SRMID);

		if (_SRM2Conn.end() != it) // clean the old mapping of the connection
			_conn2SRM.erase(it->second);

		MAPSET(Props, _SRM2Conn, SRMID, SRMConnectionId);
		MAPSET(Props, _conn2SRM, SRMConnectionId, SRMID);
	}

	ISVODLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "updated connection mapping: SRM[%s]=> conn[%s]"), SRMID.c_str(), SRMConnectionId.c_str());
}

std::string Environment::getSRMByConn(const std::string& connId)
{
	ZQ::common::MutexGuard lk(_SRMMapLock);
	Props::iterator it = _conn2SRM.find(connId);

	if (_conn2SRM.end() == it)
		return "";

	return it->second;
}

size_t Environment::listSRMConns(std::vector<std::string>& connIds, const std::string& srmId) const
{
	connIds.clear();
	ZQ::common::MutexGuard lk(_SRMMapLock);
	Props::const_iterator it = _SRM2Conn.end();
	if (srmId.empty())
	{
		// return all conns
		for (it = _SRM2Conn.begin(); it != _SRM2Conn.end(); it++)
			connIds.push_back(it->second);
	}
	else
	{
		it = _SRM2Conn.find(srmId);
		if (_SRM2Conn.end() != it)
			connIds.push_back(it->second);
	}

	return connIds.size();
}

/*
void Environment::addIntoSRMMap(const std::string& SRMID, const std::string& SRMConnectionId)
{
	{
		ZQ::common::MutexGuard lk(_SRMMapLock);
		MAPSET(Props, _SRMMap, SRMID, SRMConnectionId);
	}

	ISVODLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "updated connection mapping: SRM[%s]=> conn[%s]"), SRMID.c_str(), SRMConnectionId.c_str());
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
			return true;
	}
//	ISVODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(Environment, "unauthorized connection id [%s]"), srmConnID.c_str());
	return false;
}
*/

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
		timeout = long(_openVBOConfig._rtspSession._timeout * 1000);
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
    ZQ::common::TimeUtil::TimeToUTC(ZQ::common::now(), buff, 62);
    /*
	SYSTEMTIME systemTime;
	GetSystemTime(&systemTime);
	ZQ::common::TimeUtil::Time2Iso(systemTime, buff, sizeof(buff) - 1);
    */
    return buff;
}

//-----------------------------------------------------------------
Environment::IceStormClient::IceStormClient(Environment& env)
 :_bQuit(false),/* _hEvent(NULL),*/ _env(env)
{

}

Environment::IceStormClient::~IceStormClient()
{

}

bool Environment::IceStormClient::init()
{
    /*
	if(_hEvent)
	{
		CloseHandle(_hEvent);
		_hEvent = NULL;
	}
	_hEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);
	return _hEvent != NULL;
    */
    return true;
}

int Environment::IceStormClient::run()
{
	uint32 dwSleepTime = 10 * 1000;
	while (!_bQuit)
	{
		if (_env.ConnectIceStorm(_openVBOConfig._iceStorm._endpoint))
		{
			_bQuit = true;
			break;
		}

		//WaitForSingleObject(_hEvent, dwSleepTime);
        _hEvent.wait(dwSleepTime);
	}
	return 1;
}

void Environment::IceStormClient::final()
{
    /*
	if(_hEvent)
	{
		CloseHandle(_hEvent);
		_hEvent = NULL;
		_bQuit = true;
	}
    */
}

void Environment::IceStormClient::stop()
{
    _bQuit = true;
    _hEvent.signal();
    waitHandle(5000);
}

//template<typename TableClass, typename TableData>
//class TableMediator: public ZQ::Snmp::IManaged
//{
//public:
//	TableMediator(ZQ::common::Log& reporter, const ZQ::Snmp::Oid subid, ZQ::Snmp::Subagent* snmpTableAgent, TableData & tableEnv)
//		:_subid(subid), _triggerSubid("1.1"), _snmpTableAgent(snmpTableAgent), _createTable(tableEnv), _reporter(reporter)
//	{
//		_inStoreTable = _createTable(_reporter);
//	};
//
//	virtual ~TableMediator(){};
//
//	virtual ZQ::Snmp::Status get(const ZQ::Snmp::Oid& subid, ZQ::Snmp::SmiValue& val)
//	{
//		if (0 == _triggerSubid.compare(0, subid.length(), subid))
//			_inStoreTable = _createTable(_reporter);//refresh table
//
//		return _inStoreTable->get(subid, val); 
//	};
//
//	virtual ZQ::Snmp::Status set(const ZQ::Snmp::Oid& subid, const ZQ::Snmp::SmiValue& val)
//	{
//		return _inStoreTable->set(subid, val);
//	};
//
//	virtual ZQ::Snmp::Status next(const ZQ::Snmp::Oid& subid, ZQ::Snmp::Oid& nextId) const
//	{
//		if (0 == _triggerSubid.compare(0, subid.length(), subid))
//		{
//			TableMediator* tempThis = const_cast<TableMediator*>(this);
//			tempThis->_inStoreTable = tempThis->_createTable(_reporter);//refresh table
//		}
//
//		return  _inStoreTable->next(subid, nextId);
//	};
//
//	virtual ZQ::Snmp::Status first(ZQ::Snmp::Oid& firstId) const
//	{
//		return _inStoreTable->first(firstId);
//	};
//
//	bool addColumn(uint32 colId, ZQ::Snmp::AsnType type, ZQ::Snmp::Access access)
//	{
//		return _inStoreTable->addColumn(colId, type, access);
//	};
//
//	bool addRowData(uint32 colId, ZQ::Snmp::Oid rowIndex, ZQ::Snmp::VariablePtr var)
//	{
//		return _inStoreTable->addRowData(colId, rowIndex, var);
//	};
//
//	ZQ::Snmp::Oid buildIndex(const std::string& idx)
//	{
//		return _inStoreTable->buildIndex(idx);
//	};
//
//	ZQ::Snmp::Oid buildIndex(uint32 idx)
//	{
//		return _inStoreTable->buildIndex(idx);
//	};
//
//private:
//	ZQ::Snmp::Oid             _subid;	
//	ZQ::Snmp::Oid             _triggerSubid;
//	ZQ::Snmp::Subagent *      _snmpTableAgent;
//	ZQ::Snmp::TablePtr        _inStoreTable;
//	ZQ::common::Log&          _reporter;
//
//	TableClass                _createTable;	
//};
//
//class VboStrTable
//{
//private:
//	enum VboStrColumn
//	{
//		VBO_NULL		    = 0,
//		VBO_STR_INDEX          ,
//		VBO_STR_NETID          ,
//		VBO_STR_SOURCE_ADDR    ,
//		VBO_STR_ENDPOINT       ,
//		VBO_STR_STATUS         ,
//		VBO_STR_PENALTY        ,
//		VBO_STR_SESSION_USED   ,
//		VBO_STR_SESSION_FAILD  ,
//		VBO_STR_USED_BANDWIDTH ,
//		VBO_STR_MAX_BANDWIDTH  ,
//		VBO_STR_ACTIVE_SESSION ,
//		VBO_STR_MAX_SESSION    ,
//		VBO_STR_LOCAL_SESSION  ,
//		VBO_STR_VOLUME         ,
//		VBO_STR_COLUNM_COUNT
//	};
//
//public:
//	VboStrTable(NgodResourceManager& resourceManager)
//		:_vboResourceManager(resourceManager)
//	{}
//
//	ZQ::Snmp::TablePtr  operator()(ZQ::common::Log& reporter)
//	{
//		typedef DECLARE_SNMP_RO_TYPE(int, int, int)                           RoVboIcInt;
//		//typedef DECLARE_SNMP_RO_TYPE(int64, int64, int64)                     RoVboIcInt64;
//		typedef DECLARE_SNMP_RO_TYPE(std::string, std::string, std::string)   RoVboIcString;
//
//		SOPS sops;
//		int  rowIndex = 0;
//		SsmOpenVBO::StreamersStatisticsWithStamp statistics;
//		ZQ::Snmp::TablePtr tbVboStr(new ZQ::Snmp::Table());
//
//		tbVboStr->addColumn(VBO_STR_INDEX         , ZQ::Snmp::AsnType_Integer, ZQ::Snmp::aReadOnly);            
//		tbVboStr->addColumn(VBO_STR_NETID         , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
//		tbVboStr->addColumn(VBO_STR_SOURCE_ADDR   , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly);
//		tbVboStr->addColumn(VBO_STR_ENDPOINT      , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
//		tbVboStr->addColumn(VBO_STR_STATUS        , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
//		tbVboStr->addColumn(VBO_STR_PENALTY       , ZQ::Snmp::AsnType_Integer, ZQ::Snmp::aReadOnly);            
//		tbVboStr->addColumn(VBO_STR_SESSION_USED  , ZQ::Snmp::AsnType_Integer, ZQ::Snmp::aReadOnly); 
//		tbVboStr->addColumn(VBO_STR_SESSION_FAILD , ZQ::Snmp::AsnType_Integer, ZQ::Snmp::aReadOnly);
//		tbVboStr->addColumn(VBO_STR_USED_BANDWIDTH, ZQ::Snmp::AsnType_Integer, ZQ::Snmp::aReadOnly); 
//		tbVboStr->addColumn(VBO_STR_MAX_BANDWIDTH , ZQ::Snmp::AsnType_Integer, ZQ::Snmp::aReadOnly); 
//		tbVboStr->addColumn(VBO_STR_ACTIVE_SESSION, ZQ::Snmp::AsnType_Integer, ZQ::Snmp::aReadOnly);            
//		tbVboStr->addColumn(VBO_STR_MAX_SESSION   , ZQ::Snmp::AsnType_Integer, ZQ::Snmp::aReadOnly); 
//		tbVboStr->addColumn(VBO_STR_LOCAL_SESSION , ZQ::Snmp::AsnType_Integer, ZQ::Snmp::aReadOnly);
//		tbVboStr->addColumn(VBO_STR_VOLUME        , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly); 
//
//		_vboResourceManager.getSopData(sops, statistics.stampMeasuredSince);
//		for (SOPS::iterator itSop = sops.begin(); itSop != sops.end(); ++itSop)
//		{
//			ResourceStreamerAttrMap& streamerInfos = itSop->second;
//			ResourceStreamerAttrMap::iterator itStreamerInfo = streamerInfos.begin();
//			for (; itStreamerInfo != streamerInfos.end(); ++itStreamerInfo)
//			{
//				++rowIndex;
//				ResourceStreamerAttr& strAttr = itStreamerInfo->second;
//				int localSession = (int)(strAttr.statisticsTotalSessCount - strAttr.statisticsRemoteSessCount);
//				std::string streamerStatus = (strAttr.bMaintainEnable > 0 ? (strAttr.bReplicaStatus > 0 ? "avail" : "unavail") : "disable");
//				int maxBandwidth  = strAttr.maxBw / 1000;
//				int usedBandwidth = strAttr.usedBw / 1000;
//
//				ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex(rowIndex);
//				tbVboStr->addRowData(VBO_STR_INDEX         , indexOid, ZQ::Snmp::VariablePtr( new RoVboIcInt(rowIndex) ));              
//				tbVboStr->addRowData(VBO_STR_NETID         , indexOid, ZQ::Snmp::VariablePtr( new RoVboIcString(strAttr.netId) ));	            
//				tbVboStr->addRowData(VBO_STR_SOURCE_ADDR   , indexOid, ZQ::Snmp::VariablePtr( new RoVboIcString(strAttr.sourceAddr) ));
//				tbVboStr->addRowData(VBO_STR_ENDPOINT      , indexOid, ZQ::Snmp::VariablePtr( new RoVboIcString(strAttr.endpoint) ));
//				tbVboStr->addRowData(VBO_STR_STATUS        , indexOid, ZQ::Snmp::VariablePtr( new RoVboIcString(streamerStatus) ));
//				tbVboStr->addRowData(VBO_STR_PENALTY       , indexOid, ZQ::Snmp::VariablePtr( new RoVboIcInt((int)strAttr.penalty) ));              
//				tbVboStr->addRowData(VBO_STR_SESSION_USED  , indexOid, ZQ::Snmp::VariablePtr( new RoVboIcInt((int)strAttr.statisticsUsedSessCount ) ));	            
//				tbVboStr->addRowData(VBO_STR_SESSION_FAILD , indexOid, ZQ::Snmp::VariablePtr( new RoVboIcInt((int)strAttr.statisticsFailedSessCount) ));
//				tbVboStr->addRowData(VBO_STR_USED_BANDWIDTH, indexOid, ZQ::Snmp::VariablePtr( new RoVboIcInt(usedBandwidth) ));
//				tbVboStr->addRowData(VBO_STR_MAX_BANDWIDTH , indexOid, ZQ::Snmp::VariablePtr( new RoVboIcInt(maxBandwidth) ));
//				tbVboStr->addRowData(VBO_STR_ACTIVE_SESSION, indexOid, ZQ::Snmp::VariablePtr( new RoVboIcInt((int)strAttr.usedSessCount ) ));              
//				tbVboStr->addRowData(VBO_STR_MAX_SESSION   , indexOid, ZQ::Snmp::VariablePtr( new RoVboIcInt((int)strAttr.maxSessCount) ));	            
//				tbVboStr->addRowData(VBO_STR_LOCAL_SESSION , indexOid, ZQ::Snmp::VariablePtr( new RoVboIcInt(localSession) ));
//				tbVboStr->addRowData(VBO_STR_VOLUME        , indexOid, ZQ::Snmp::VariablePtr( new RoVboIcString(strAttr.volumeNetId) ));
//			}
//		}
//
//		(reporter)(ZQ::common::Log::L_DEBUG, CLOGFMT(VboStrTable, "OpenVBO create tbVboStr Table end, row[%d]"), rowIndex);
//		return tbVboStr;
//	}
//
//private:
//    NgodResourceManager& _vboResourceManager;
//};
//
//class VboIcTable
//{
//public:
//	enum VboIcColumn
//	{
//		VBO_NULL		       = 0,
//		VBO_IC_INDEX              ,
//		VBO_IC_CHANNEL_NAME       ,
//		VBO_IC_USED_BANDWIDTH     ,
//		VBO_IC_TOTAL_BANDWIDTH    ,
//		VBO_IC_RUNNING_SESS_COUNT ,
//		VBO_TABLE_COLUNM_COUNT
//	};
//
//public:
//	VboIcTable(NgodResourceManager& resourceManager)
//		:_vboResourceManager(resourceManager)
//	{}
//
//	ZQ::Snmp::TablePtr  operator()(ZQ::common::Log& reporter)
//	{
//		typedef DECLARE_SNMP_RO_TYPE(int, int, int)                           RoVboIcInt;
//		typedef DECLARE_SNMP_RO_TYPE(std::string, std::string, std::string)   RoVboIcString;
//
//		ZQ::Snmp::TablePtr tbVboIc(new ZQ::Snmp::Table());
//		tbVboIc->addColumn(VBO_IC_INDEX             , ZQ::Snmp::AsnType_Integer, ZQ::Snmp::aReadOnly); 
//		tbVboIc->addColumn(VBO_IC_CHANNEL_NAME      , ZQ::Snmp::AsnType_Octets , ZQ::Snmp::aReadOnly);
//		tbVboIc->addColumn(VBO_IC_USED_BANDWIDTH    , ZQ::Snmp::AsnType_Integer, ZQ::Snmp::aReadOnly); 
//		tbVboIc->addColumn(VBO_IC_TOTAL_BANDWIDTH   , ZQ::Snmp::AsnType_Integer, ZQ::Snmp::aReadOnly); 
//		tbVboIc->addColumn(VBO_IC_RUNNING_SESS_COUNT, ZQ::Snmp::AsnType_Integer, ZQ::Snmp::aReadOnly); 
//		
//		int rowIndex = 0;
//		ResourceImportChannelAttrMap ics;
//		ResourceImportChannelAttrMap::const_iterator itIc;
//		
//		_vboResourceManager.getImportChannelData(ics);
//		for (itIc = ics.begin(); itIc != ics.end(); itIc++)
//		{
//			++rowIndex;
//			SsmOpenVBO::ImportChannelStatistics importChannelInfos;
//			const ResourceImportChannelAttr& attr = itIc->second;
//			ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex(rowIndex);
//
//			tbVboIc->addRowData(VBO_IC_INDEX             , indexOid, ZQ::Snmp::VariablePtr( new RoVboIcInt((int)rowIndex) ));              
//			tbVboIc->addRowData(VBO_IC_CHANNEL_NAME      , indexOid, ZQ::Snmp::VariablePtr( new RoVboIcString(attr.netId) ));
//			tbVboIc->addRowData(VBO_IC_USED_BANDWIDTH    , indexOid, ZQ::Snmp::VariablePtr( new RoVboIcInt((int)attr.reportUsedBW ) ));	            
//			tbVboIc->addRowData(VBO_IC_TOTAL_BANDWIDTH   , indexOid, ZQ::Snmp::VariablePtr( new RoVboIcInt((int)attr.reportMaxBW) ));
//			tbVboIc->addRowData(VBO_IC_RUNNING_SESS_COUNT, indexOid, ZQ::Snmp::VariablePtr( new RoVboIcInt((int)attr.reportUsedSessCount) ));
//		}
//
//		(reporter)(ZQ::common::Log::L_DEBUG, CLOGFMT(VboIcTable, "OpenVBO create VboIcTable Table end, row[%d]"), rowIndex);
//		return tbVboIc;
//	}
//
//private:
//	NgodResourceManager& _vboResourceManager;
//};
//
bool Environment::registerSnmp(IStreamSmithSite* pSite)
{	
	//using namespace ZQ::common;
	//ISVODLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(Environment,"registerSnmpTable() entering"));

	//using ZQ::common::Variant;
	//long instanceId = 0;
	//try
	//{
	//	Variant  snmpProcess    = pSite->getInfo(IStreamSmithSite::INFORMATION_SNMP_PROCESS);
	//	Variant  snmpInstanceId = snmpProcess["sys.snmp.instanceId"];
	//	instanceId  = (long)snmpInstanceId;
	//}
	//catch (...)
	//{
	//	ISVODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(Environment,"registerSnmp() get unknown exception"));
	//}

	//ISVODLOG(ZQ::common::Log::L_INFO, CLOGFMT(Environment,"registerSnmp() instanceId[%d]"), instanceId);
	//_vboSnmpAgent = new ZQ::Snmp::Subagent(1000, 4, instanceId);
	//if (!_vboSnmpAgent)
	//{
	//	ISVODLOG(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "failed to initial snmp agent"));
	//	return false;
	//}

	//_vboSnmpAgent->setLogger(&_fileLog);
	//_vboSnmpAgent->start();

	//typedef TableMediator<VboStrTable, NgodResourceManager >  VboStrTblType;
	//typedef TableMediator<VboIcTable, NgodResourceManager >   VboIcTblType;

	//ZQ::Snmp::ManagedPtr vboStrTbl(new VboStrTblType(_fileLog, ZQ::Snmp::Oid("1.1.1"), _vboSnmpAgent, *_resourceManager));
	//ZQ::Snmp::ManagedPtr vboIcTbl(new VboIcTblType(_fileLog, ZQ::Snmp::Oid("2.1.1"), _vboSnmpAgent, *_resourceManager));

	//int nRev1 = _vboSnmpAgent->addObject(ZQ::Snmp::Oid("1.1.1"), ZQ::Snmp::ManagedPtr(vboStrTbl));
	//int nRev2 = _vboSnmpAgent->addObject(ZQ::Snmp::Oid("2.1.1"), ZQ::Snmp::ManagedPtr(vboIcTbl));

	//ISVODLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(Environment,"registerSnmp() instanceId[%d] succeed"), instanceId);
    _mmib.addObject(new ZQ::SNMP::SNMPObjectByAPI<Environment, uint32>("vboStatReset", *this, ZQ::SNMP::AsnType_Int32, &EventISVODI5::Environment::snmp_dummyGet, &EventISVODI5::Environment::snmp_refreshVBOStrUsage));

    _snmpSA.start();

	return true;
}

void Environment::snmp_refreshVBOStrUsage(const uint32& iDummy)
{
    static ZQ::SNMP::Oid subOidTbl;
    if (subOidTbl.isNil())
        _mmib.reserveTable("vboStrTable", 14, subOidTbl);
    if (subOidTbl.isNil())
    {
        ISVODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(Environment,"snmp_refreshVBOStrUsage() failed to locate vboStrTable in MIB"));
        return;
    }

    // step 1. clean up the table content
    ZQ::SNMP::Oid tmpOid(subOidTbl);
    tmpOid.append(1);
    _mmib.removeSubtree(tmpOid);

    SOPS sopRes;
    std::string strSince;
   getResourceManager().getSopData(sopRes, strSince);

    uint32 idxRow =1;
    for (SOPS::iterator it=sopRes.begin(); it != sopRes.end(); it++)
    {
        for (ResourceStreamerAttrMap::iterator j = it->second.begin(); j != it->second.end(); j++, idxRow++)
        {
            ResourceStreamerAttr& stmrdata = j->second;
            _mmib.addTableCell(subOidTbl,  1, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboStrIndex", (int32)idxRow));
            _mmib.addTableCell(subOidTbl,  2, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboStrNetId", stmrdata.netId));
            _mmib.addTableCell(subOidTbl,  3, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboStrSourceAddr", stmrdata.sourceAddr));
            _mmib.addTableCell(subOidTbl,  4, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboStrEndpoint", stmrdata.endpoint));
            _mmib.addTableCell(subOidTbl,  5, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboStrStatus", (const char*)(stmrdata.bReplicaStatus?"avail":"unavail")));
            _mmib.addTableCell(subOidTbl,  6, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboStrPenalty", (int64)stmrdata.penalty));
            _mmib.addTableCell(subOidTbl,  7, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboStrSessionUsed", stmrdata.statisticsUsedSessCount));
            _mmib.addTableCell(subOidTbl,  8, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboStrSessionFailed", stmrdata.statisticsFailedSessCount));
           // _mmib.addTableCell(subOidTbl,  9, idxRow, new ZQ::SNMP::SNMPObjectDupValue("sopErrorRate", (int32)(stmrdata.statisticsTotalSessCount? (stmrdata.statisticsFailedSessCount*100 / stmrdata.statisticsTotalSessCount) :0)));
            _mmib.addTableCell(subOidTbl, 9, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboStrUsedBandwidth", (int64) stmrdata.usedBw));
            _mmib.addTableCell(subOidTbl, 10, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboStrMaxBandwidth", (int64) stmrdata.maxBw));
            _mmib.addTableCell(subOidTbl, 11, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboStrActiveSession", (int64)stmrdata.usedSessCount));
            _mmib.addTableCell(subOidTbl, 12, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboStrMaxSession", (int64)stmrdata.maxSessCount));
            _mmib.addTableCell(subOidTbl, 13, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboStrLocalSession", (int64)(stmrdata.statisticsTotalSessCount -stmrdata.statisticsRemoteSessCount)));
            _mmib.addTableCell(subOidTbl, 14, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboStrVolume", stmrdata.volume));
        }
    }
    _mmib.addObject(new ZQ::SNMP::SNMPObjectDupValue("vboStrCount", (int32)idxRow -1));
    ISVODLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(Environment,"snmp_refreshVBOStrUsage() table refreshed: %d rows"), (int32)idxRow -1);

    snmp_refreshIcUsage(iDummy); // trigger IC refresh as currently take a single sopReset as the trigger to refresh both table
}

void Environment::snmp_refreshIcUsage(const uint32&)
{
    static ZQ::SNMP::Oid subOidTbl;
    if (subOidTbl.isNil())
        _mmib.reserveTable("vboIcTable", 5, subOidTbl);
    if (subOidTbl.isNil())
    {
        ISVODLOG(ZQ::common::Log::L_WARNING, CLOGFMT(Environment,"snmp_refreshIcUsage() failed to locate vboIcTable in MIB"));
        return;
    }

    // step 1. clean up the table content
    ZQ::SNMP::Oid tmpOid(subOidTbl);
    tmpOid.append(1);
    _mmib.removeSubtree(tmpOid);

    ResourceImportChannelAttrMap ics;
    getResourceManager().getImportChannelData(ics);

    uint32 idxRow =1;
    for (ResourceImportChannelAttrMap::iterator it=ics.begin(); it != ics.end(); it++, idxRow++)
    {
        ResourceImportChannelAttr& ic = it->second;
        _mmib.addTableCell(subOidTbl,  1, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboIcIndex",         (int32)idxRow));
        _mmib.addTableCell(subOidTbl,  2, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboIcChannelName",   ic.netId));
        _mmib.addTableCell(subOidTbl,  3, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboIcUsedBandwidth", (int64)ic.reportUsedBW));
        _mmib.addTableCell(subOidTbl,  4, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboIcTotalBandwidth", (int64)ic.reportMaxBW));
        _mmib.addTableCell(subOidTbl,  5, idxRow, new ZQ::SNMP::SNMPObjectDupValue("vboIcRunningSessCount", (int32)ic.reportUsedSessCount));
        //_mmib.addTableCell(subOidTbl,  6, idxRow, new ZQ::SNMP::SNMPObjectDupValue("icStatus",           (char*)"n/a")); // ????
    }

    _mmib.addObject(new ZQ::SNMP::SNMPObjectDupValue("vboIcCount", (int32)idxRow -1));
    ISVODLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(Environment,"snmp_refreshIcUsage() table refreshed: %d rows"), (int32)idxRow -1);
}

int UpdateSNMPTableTimer::run(){
    while (!_bQuit)
    {
        int64 now = ZQ::common::TimeUtil::now();
        if ((now - _lastUpdateTime) > 30*1000)
        {
            _env.snmp_refreshVBOStrUsage(1);
            _lastUpdateTime = now;
        }
    }
    return 1;
}

void UpdateSNMPTableTimer::stop()
{
    if (!_bQuit)
    {
        _bQuit = true;
        waitHandle(5000);
    }
}

} // end __EVENT_IS_VODI5_ENVIROMENT_H__
