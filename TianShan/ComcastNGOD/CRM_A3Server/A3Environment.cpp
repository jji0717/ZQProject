// FileName : A3Enviroment.cpp
// Author   : Zheng Junming
// Date     : 2009-05
// Desc     : A3Environment mainly load configuration, create log and initial ice runtime

#include "A3Environment.h"
#include "ZQResource.h"
#include "A3Config.h"
#include "A3MsgHandler.h"
#include "A3Client.h"
#include "FileSystemOp.h"

#include <Freeze/Evictor.h>
#include <IceStorm/IceStorm.h>

ZQ::common::Config::Loader<CRG::Plugin::A3Server::HttpA3Cfg> _httpA3Config("");

using namespace CRG::Plugin::A3Server;

Environment::Environment()
:_A3MsgHandler(NULL), _pAdapter(NULL), _iceLogger(NULL), _pCommunicator(NULL), _a3Content(NULL),
_assetIdx(NULL), _volumeIdx(NULL), _factory(NULL), _eventChannel(NULL), _a3Client(NULL), _a3FacedeIPtr(NULL)
{
}

Environment::~Environment()
{
}

bool Environment::loadConfig(const std::string& strCfgPath)
{
	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "loadConfig() : **************** (Load %s) ****************"), ZQ_PRODUCT_NAME);
	_httpA3Config.setLogger(&glog);
	if(!_httpA3Config.load(strCfgPath.c_str()))
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "loadConfig() : load config %s failed"), strCfgPath.c_str());
		return false;
	}
	return true;
}

static void fixPath(std::string& strPath)
{
	size_t nlen = strPath.length();
	if(strPath[nlen-1] != FNSEPC )
		strPath += FNSEPS;
}

bool Environment::doInit(const std::string& strLogPath,const std::string& strCfgPath)
{
	// open log file and load config
	_sysLog.open(ZQ_PRODUCT_NAME, ZQ::common::Log::L_WARNING);

	std::string strPathOfLog = strLogPath;
	std::string strPathOfCfg = strCfgPath;	
	fixPath(strPathOfLog);
	fixPath(strPathOfCfg);

	strPathOfLog += "CRM_A3Server.log";
	try
	{
		_fileLog.open(strPathOfLog.c_str(), 7);
	}
	catch (ZQ::common::FileLogException& ex)
	{
		_sysLog(ZQ::common::Log::L_ERROR, "doInit() : create log [%s] caught %s", strPathOfLog.c_str(), ex.getString());
		return false;
	}
	
	ZQ::common::setGlogger(&_fileLog);
	strPathOfCfg += "CRM_A3Server.xml";
	if (!loadConfig(strPathOfCfg))
	{
		return false;
	}
	_fileLog.setFileSize(_httpA3Config._logFile.size);
	_fileLog.setFileCount(_httpA3Config._logFile.maxCount);
	_fileLog.setLevel(_httpA3Config._logFile.level);
	_fileLog.setBufferSize(_httpA3Config._logFile.bufferSize);
	
	if (!initIceRuntime(strLogPath, _pCommunicator, _pAdapter) || 
		!connectContentStore(_contentStoreProxies))
	{
		return false;
	}

	_a3Client = new (std::nothrow) A3Client();
	if (!_a3Client)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "doInit() : Fail to create A3 Http Client"));
		return false;
	}

	// add A3FacedeI
	TianShanIce::Events::GenericEventSinkPtr a3EventPtr; 
	a3EventPtr = new (std::nothrow) A3FacedeI(_pAdapter, _a3Content, _assetIdx, _volumeIdx, _a3Client,
		_contentStoreProxies);
	if (!a3EventPtr)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "doInit() : Fail to create A3 FacedeI"));
		return false;
	}
	connectIceStorm(a3EventPtr);

	_a3FacedeIPtr = A3FacedeIPtr::dynamicCast(a3EventPtr);
	_a3FacedeIPtr->updateA3ContentLib();
	_pAdapter->ZQADAPTER_ADD(_pCommunicator, _a3FacedeIPtr, _httpA3Config._bind.serviceName);
	/*if (_httpA3Config._bind.enabled == 1)
	{
		_a3FacedeIPtr->start();
		glog(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "doInit() : Success to start A3 Faceade thread to sync with content store active"));
	}*/
	_A3MsgHandler = new (std::nothrow) A3MsgHandler(_a3FacedeIPtr, _a3Client);
	if (!_A3MsgHandler)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "doInit() : Fail to create A3 Message handler"));
		return false;
	}
	_httpA3Config.snmpRegister("HttpA3Server");
	return true;
}

void Environment::doUninit()
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "doUninit() : do doUninit()"));
	try{
		if(_pAdapter)
		{
			_pAdapter->deactivate();
			_pAdapter->closeBarker();
		}
	}catch(...){}

	if (_A3MsgHandler)
	{
		delete _A3MsgHandler;
		_A3MsgHandler = NULL;
	}

	if (_a3Client)
	{
		delete _a3Client;
		_a3Client = NULL;
	}

	closeFreezeEvictor();
	uninitIceRuntime();
}

bool Environment::connectContentStore(std::map<std::string, TianShanIce::Storage::ContentStorePrx>& contentStoreProxies)
{
	bool bSuccess = false;
	TianShanIce::Storage::ContentStorePrx contentStoreProxy;
	std::string strNetId;
	std::string strEndpoint;
	ContentStoreList::contentStores::iterator contentStoreIt;
	for ( contentStoreIt = _httpA3Config._contentStoreList._contentStores.begin();
		contentStoreIt != _httpA3Config._contentStoreList._contentStores.end();
		contentStoreIt++)
	{
		try
		{
			strEndpoint = contentStoreIt->endpoint;
			Ice::ObjectPrx base = _pCommunicator->stringToProxy(strEndpoint);
			contentStoreProxy = TianShanIce::Storage::ContentStorePrx::checkedCast(base);
			if (!contentStoreProxy)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "connectContentStore() : Fail to get content store [%s] proxy"), contentStoreIt->endpoint.c_str());
				continue;
			}
			strNetId = contentStoreProxy->getNetId();
		}
		catch(const Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "connectContentStore() : get content store [%s] proxy caught an exception[%s]"), contentStoreIt->endpoint.c_str(), ex.ice_name().c_str());
			continue;
		}
		bSuccess = true;
		contentStoreProxies.insert(std::make_pair(strNetId, contentStoreProxy));
		glog(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "connectContentStore() : connect with endpoint [%s], net Id [%s]"),contentStoreIt->endpoint.c_str(), strNetId.c_str());
	}
	return bSuccess;
}

bool Environment::initIceRuntime(const std::string& strLogPath, Ice::CommunicatorPtr& pCommunicator, 
								 ZQADAPTER_DECLTYPE& pAdapter)
{
	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "do initIceRuntime()"));
	try
	{
		// DO: initialize properties for ice run-time
		int i=0;
		Ice::PropertiesPtr props = Ice::createProperties(i, NULL);
		if ( 0 != props)
		{
			IceProperties::props::iterator it;
			for (it = _httpA3Config._iceProperties._props.begin(); it != _httpA3Config._iceProperties._props.end(); it++)
			{
				props->setProperty(it->name.c_str(), it->value.c_str());
			}
		}

		// DO: create communicator
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(Environment, "do create communicator"));
		Ice::InitializationData idt;
		idt.properties = props;

		std::string iceLogDir = strLogPath;
		fixPath(iceLogDir);
		iceLogDir += "CRM_A3Server.icetrace.log";

		if (_httpA3Config._iceTrace.enabled >= 1)
		{
			try
			{
				_iceFileLog.open(iceLogDir.c_str(),
					_httpA3Config._iceTrace.level,
					_httpA3Config._iceTrace.maxCount,
					_httpA3Config._iceTrace.size);
				_iceLogger = new TianShanIce::common::IceLogI(&_iceFileLog);
				idt.logger = _iceLogger;
			}
			catch(const ZQ::common::FileLogException& ex)
			{
				glog(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "failed to create ice trace file log [%s]"), ex.getString());
			}
		}
		pCommunicator=Ice::initialize(idt);
		glog(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "communicator created successfully"));
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "create communicator caught(%s)"), ex.ice_name().c_str());
		return false;
	}

	try
	{
		pAdapter = ZQADAPTER_CREATE(pCommunicator, "A3Adapter", _httpA3Config._bind.endpoint.c_str(), glog);
		if (!initFreezeEvictor(_factory, _assetIdx, _volumeIdx, _a3Content))
		{
			return false;
		}
		pAdapter->activate();
	}
	catch(const Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "create adapter(%s) caught(%s)"), _httpA3Config._bind.endpoint.c_str(), ex.ice_name().c_str());
		return false;
	}

	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "leave initIceRuntime()"));
	return true;
}

void Environment::uninitIceRuntime()
{
	try
	{
		_eventChannel = NULL;
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "destroy event Channel adapters caught(%s)"), ex.ice_name().c_str());
	}
	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "do uninitIceRuntime()"));
	
	try{
		if(_iceLogger)
			_iceLogger = NULL;
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "ice logger  caught(%s)"), ex.ice_name().c_str());
	}
	try
	{
		if ( 0 != _pCommunicator )
		{
			glog(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "do _pCommunicator->deactivate()"));
			_pAdapter = NULL;
			_pCommunicator->destroy();
			glog(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "_pCommunicator->deactivate() successfully"));
		}
		_pCommunicator = NULL;
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "deactive adapters caught(%s)"), ex.ice_name().c_str());
	}
	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "leave uninitIceRuntime()"));
}

bool Environment::initFreezeEvictor(A3ContentFactoryPtr& factory, A3Module::AssetIdxPtr& assetIdx, 
									A3Module::VolumeIdxPtr& volumeIdx, Freeze::EvictorPtr& a3Content)
{
	closeFreezeEvictor();
	factory = new (std::nothrow) A3ContentFactory(_pAdapter);
	if (!factory)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "initFreezeEvictor() : Fail to create a3 content factory"));
		return false;
	}
	// create directory
	std::string strDataBase = _httpA3Config._dataBase.path;
	std::string strRuntimePath = _httpA3Config._dataBase.runtimePath;
	fixPath(strDataBase);
	fixPath(strRuntimePath);
	strDataBase += "A3Server";
	strRuntimePath += "A3Server";

	FS::createDirectory(strDataBase.c_str());
	FS::createDirectory(strRuntimePath.c_str());

	try
	{
		assetIdx = new A3Module::AssetIdx("AssetIndex");
		volumeIdx = new A3Module::VolumeIdx("VolumeIndex");
		std::vector<Freeze::IndexPtr> indices;
		indices.push_back(assetIdx);
		indices.push_back(volumeIdx);
#if ICE_INT_VERSION / 100 >= 303
		a3Content = ::Freeze::createBackgroundSaveEvictor(_pAdapter, strDataBase, "A3Content", 0, indices);
#else
		a3Content = Freeze::createEvictor(_pAdapter, strRuntimePath, "A3Content", 0, indices);
#endif
		a3Content->setSize(_httpA3Config._bind.evictorSize);
		_pAdapter->addServantLocator(_a3Content, "A3Content");
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(Environment,"catch an exception[%s] when create freeze evictor"),
			ex.ice_name().c_str());
		return false;
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(Environment, "Success to create freeze evictor"));
	return true;

}

void Environment::closeFreezeEvictor()
{
	_a3Content = NULL;
	_factory = NULL;
}

bool Environment::connectIceStorm(TianShanIce::Events::GenericEventSinkPtr& a3Event)
{
	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "do connectIceStorm()"));
	try
	{
		_eventChannel = new TianShanIce::Events::EventChannelImpl(_pAdapter, _httpA3Config._iceStorm.endPoint.c_str());
		TianShanIce::Properties qos;
		_eventChannel->sink(a3Event, qos);
		_eventChannel->start();
	}
	catch (const TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "connectIceStorm(%s) caught(%s: %s)")
			, _httpA3Config._iceStorm.endPoint.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "connectIceStorm(%s) caught(%s)")
			, _httpA3Config._iceStorm.endPoint.c_str(), ex.ice_name().c_str());
		return false;
	}

	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(Environment, "leave connectIceStorm()"));
	return true;
}
