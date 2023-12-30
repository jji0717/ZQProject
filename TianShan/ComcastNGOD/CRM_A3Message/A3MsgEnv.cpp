#include "A3MsgEnv.h"
#include "ZQResource.h"
#include "A3Config.h"
#include "A3MessageEventSinkI.h"
#include "TianShanDefines.h"

ZQ::common::Config::Loader< A3MessageCfg > _A3Config("");
namespace CRM
{
	namespace A3Message
	{
		
		ZQ::common::LRUMap<std::string, ContentInfo> _contentInfos(2000);

		A3MsgEnv::A3MsgEnv(void):_evtAdap(NULL)
		{
          _pIceLog = NULL;
		  _clPrx = NULL;
		  _pConnectEventChannel = NULL;
		  _evtAdap = NULL;
		  _cpWrapper = NULL;
		  _pCdmiFuse = NULL;
		  _bUpdate = false;
		}

		A3MsgEnv::~A3MsgEnv(void)
		{
		
		}
		static void fixPath(std::string& strPath)
		{
			size_t nlen = strPath.length();
			if(strPath[nlen-1] != FNSEPC )
				strPath += FNSEPS;
		}
		bool A3MsgEnv::loadConfig(const std::string& strCfgPath)
		{
			if(_mgr)
			{
              _A3Config.setLogger(&(_mgr->superLogger()));
			}
			if(!_A3Config.load(strCfgPath.c_str()))
			{
				return false;
			}
			return true;
		}
		bool A3MsgEnv::doInit(const std::string& strLogPath, const std::string& strCfgPath)
		{
			std::string strPathOfLog = strLogPath;
			std::string strPathOfCfg = strCfgPath;	
			fixPath(strPathOfLog);
			fixPath(strPathOfCfg);

			strPathOfCfg += "CRM_A3.xml";
			if (!loadConfig(strPathOfCfg))
			{
				return false;
			}

			try
			{
				_log.open((strPathOfLog +_A3Config.logfilename).c_str(), 7);
			}
			catch (ZQ::common::FileLogException& ex)
			{
				ZQ::common::Log& log = _mgr->superLogger();
				log(ZQ::common::Log::L_ERROR, "doInit() : create log [%s] caught %s", strPathOfLog.c_str(), ex.getString());
				return false;
			}
			_log.setFileSize(_A3Config.lLogFileSize);
			_log.setFileCount(_A3Config.llogFileCount);
			_log.setLevel(_A3Config.lLogLevel);
			_log.setBufferSize(_A3Config.lLogBufferSize);

			/// initializa ice Communicator  and adapter
			try
			{
				_pIceLog = new ZQ::common::FileLog((strPathOfLog + _A3Config.Icelogfilename).c_str(),
					_A3Config.lIceLogLevel,
					_A3Config.lIcelogFileCount,
					_A3Config.lIceLogFileSize,
					_A3Config.lIceLogBufferSize,
					_A3Config.lIceLogWriteTimteout);

				int i = 0;
				_iceLog = new ::TianShanIce::common::IceLogI(_pIceLog);
#if ICE_INT_VERSION / 100 >= 303

				::Ice::InitializationData initData;
				initData.properties = Ice::createProperties(i, NULL);
				initWithConfig(initData.properties);

				initData.logger = _iceLog;
				_communicator = Ice::initialize(initData);
#else
				Ice::PropertiesPtr proper = Ice::createProperties(i,NULL);
				initWithConfig(proper);	

				_communicator = Ice::initializeWithPropertiesAndLogger(i,NULL,proper, _iceLog);
#endif // ICE_INT_VERSION
			}
			catch(::Ice::Exception &ex)
			{
				(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgEnv, "initialize ICE communicator failed, exception(%s)"), ex.ice_name().c_str());
				return false;
			}
			catch(...)
			{
				(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgEnv, "initialize ICE communicator caught unknown exception(%d)"), SYS::getLastErr());
				return false;
			}

			switch(_A3Config.backStoreType)
			{
			case backContentLib:
				_backStoreType = backContentLib;
				break;
			case backCacheStore:
				_backStoreType = backCacheStore;
				break;
			case backAuqaServer:
				_backStoreType = backAuqaServer;
				break;
			default:
				(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgEnv, "backStoreType is invalid"));
				return false;
			}

			(_log)(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgEnv, "backStoreType is %s"), backStoreTypeStr(_backStoreType));

			// create listen event adapter
			try 
			{
				_evtAdap = _communicator->createObjectAdapterWithEndpoints("CRMA3MessageEventAdapter", _A3Config.ListenEventEndPoint);
				_evtAdap->activate();
				_pConnectEventChannel =  new ConnectEventChannelThread(*this);
				if(!_pConnectEventChannel)
					return false;
				_pConnectEventChannel->start();
			}
			catch(Ice::Exception& ex) 
			{
				(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgEnv, "create adapter for listen event caught %s, endpoint is %s"), 
					ex.ice_name().c_str(), _A3Config.ListenEventEndPoint.c_str());
				return false;
			}

			/// create A3MessageHandler
			_A3MsgHandler = new CRM::A3Message::A3MessageHandler(*this);
			if (!_A3MsgHandler)
			{
				(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgEnv, "doInit() : Fail to create A3 Message handler"));
				return false;
			}
			/// create A3Client
			_A3Client = new CRM::A3Message::A3Client(*this);
			if (!_A3Client)
			{
				(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgEnv, "doInit(): Fail to create A3 Client handler"));
				return false;
			}
			_A3Config.snmpRegister("CRMA3Message");
            
			/// connect contentlib service	
			if(_backStoreType == backContentLib)
			{
			   connectToContentLib();
			}
			else  if(_backStoreType == backCacheStore)/// connect ContentStroe service	
			{
	           connectToCacheStore();
			}
			else if(_backStoreType == backAuqaServer)
			{
				if(_A3Config.mainFilePath.size() > 0)
				{
					if(_A3Config.mainFilePath[_A3Config.mainFilePath.size() -1] != FNSEPC)
						_A3Config.mainFilePath+= FNSEPS;
				}
				_cpWrapper =  new ContentProvisionWrapper(_log, *this);

				if(NULL == _cpWrapper)
				{
					(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgEnv, "doInit(): Fail to create ContentProvisionWrapper object"));

					return false;
				}

				if (!_cpWrapper->init(_communicator, _A3Config.cpcEndPoint, _A3Config.registerInterval)) 
				{		
					(_log)(ZQ::common::Log::L_DEBUG, CLOGFMT(A3MsgEnv, "doInit()init CPC failed"));
					return false;
				}

				TianShanIce::StrValues strtok;
				ZQTianShan::tokenize(strtok, _A3Config.strTrickSpeeds.c_str(), " \t\n\r");
				for (TianShanIce::StrValues::iterator it = strtok.begin(); it < strtok.end(); it++)
				{
					if (it->empty())
						continue;

					float val = atof(it->c_str());
					_A3Config.trickSpeedCollection.push_back(val);
				}

				_cpWrapper->setTrickSpeeds(_A3Config.trickSpeedCollection);

				_thrdPool.resize(_A3Config.maxThreadPoolSize);
				_pCdmiFuse = new CdmiFuseOps(_log, _thrdPool, _A3Config.rootUrl, _A3Config.userDomain, _A3Config.homeContainer, _A3Config.flags,FuseOpsConf(),_A3Config.bindIp);
				if(NULL == _pCdmiFuse)
				{
					(_log)(ZQ::common::Log::L_DEBUG, CLOGFMT(A3MsgEnv, "doInit()failed to create CdmiFuseOps object"));
					return false;
				}

				_aquaContentMdata = new A3AquaContentMetadata(_log, *_pCdmiFuse, _A3Config.mainFileExtension, _A3Config.contentNameFormat);
				if(NULL == _aquaContentMdata)
				{
					(_log)(ZQ::common::Log::L_DEBUG, CLOGFMT(A3MsgEnv, "doInit()failed to create A3AquaContentMetadata object"));
					return false;
				}
				_cpeSessionMgr = new A3CPESessionMgr(_log, *_pCdmiFuse);
				if(NULL == _cpeSessionMgr)
				{

					(_log)(ZQ::common::Log::L_DEBUG, CLOGFMT(A3MsgEnv, "doInit()failed to create A3CPESessionMgr object"));
					return false;
				}

				TianShanIce::Properties sessionLists, sessinonOklists;
				sessionLists.clear();
				_cpeSessionMgr->getSessionList(sessionLists);
				for(TianShanIce::Properties::iterator itorCpeProxy = sessionLists.begin(); itorCpeProxy != sessionLists.end(); ++itorCpeProxy)
				{
					std::string& cpeSessionProxy = itorCpeProxy->second;

					try {
						::TianShanIce::ContentProvision::ProvisionSessionPrx provisionSess;
						provisionSess = ::TianShanIce::ContentProvision::ProvisionSessionPrx::uncheckedCast(_communicator->stringToProxy(cpeSessionProxy));
						provisionSess->ice_ping();
						_log(ZQ::common::Log::L_INFO, CLOGFMT(A3MsgEnv, "doInit() contentName[%s] alive CPE ProvisionSession[%s]"), (itorCpeProxy->first).c_str(), cpeSessionProxy.c_str());
					}
					catch(const IceUtil::NullHandleException&)
					{
						_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3MsgEnv,"doInit()null CPE ProvisionSession[%s]"), cpeSessionProxy.c_str());
						continue;
					}
					catch(const Ice::ObjectNotExistException&)
					{
						_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3MsgEnv,"doInit() CPE ProvisionSession has gone[%s]"), cpeSessionProxy.c_str());
						continue;;
					}
					catch(const Ice::Exception& ex)
					{
						_log(ZQ::common::Log::L_DEBUG, CLOGFMT(A3MsgEnv,"doInit() check CPE ProvisionSession [%s]caugh exception: %s"), cpeSessionProxy.c_str(), ex.ice_name().c_str());
					}
					catch(...)
					{
						_log(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgEnv, "doInit() check CPE ProvisionSession [%s]caugh unknown exception"), cpeSessionProxy.c_str());
					}

					MAPSET(TianShanIce::Properties, sessinonOklists, itorCpeProxy->first, itorCpeProxy->second);
				}
				_cpeSessionMgr->updateSessionList(sessinonOklists);

//				TianShanIce::Properties props;
//				_cpeSessionMgr->getMetadataInfo("cdntest1234567890000001","xor1", props);
			}

//			_contentInfos._capacity = 10000;
			if(!_A3Config.exposeURL.empty() && _A3Config.exposeURL[_A3Config.exposeURL.size() - 1] != FNSEPC)
				_A3Config.exposeURL += FNSEPS;
			
            return true;
		}

		/// this method must be called no matter if doInit() success or fail
		void A3MsgEnv::doUninit()
		{
			try
			{ 
				if(_pConnectEventChannel)
				{
					_pConnectEventChannel->toExit();
					delete _pConnectEventChannel;
				}

				_eventChannel = NULL;

				if(_A3Client)
				{
					delete _A3Client;
					_A3Client = NULL;
				}
				if(_A3MsgHandler)
					
				{
					delete _A3MsgHandler;
					_A3MsgHandler = NULL;
				}

				_aquaContentMdata = NULL;
				_cpeSessionMgr = NULL;

				if(_cpWrapper)
				{
					_cpWrapper->unInit();
					_cpWrapper = NULL;
				}
				if(_pCdmiFuse)
				{
					delete _pCdmiFuse;
					_pCdmiFuse = NULL;
				}
				
			}
			catch (...){}

			try{
				if(_evtAdap)
				{
					_evtAdap->deactivate();
					_evtAdap->destroy();
				}

				if (_communicator )
				{
					_communicator->destroy();
					(_log)(ZQ::common::Log::L_NOTICE, CLOGFMT(A3MsgEnv, "Communicator->destroy() successfully"));
				}

				if(_pIceLog)
					delete _pIceLog;
			}
			catch (const Ice::Exception& ex)
			{
				(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgEnv, "destory communicator caught(%s)"), ex.ice_name().c_str());
			}
          
		   _pIceLog = NULL;
		   _A3Client = NULL;
		   _A3MsgHandler = NULL;
		   _iceLog = NULL;
		   _communicator = NULL;
		   _pConnectEventChannel = NULL;
		   _evtAdap = NULL;
		}
		void A3MsgEnv::initWithConfig(Ice::PropertiesPtr proper )
		{
			proper->setProperty("Ice.ThreadPool.Client.Size","5");
			proper->setProperty("Ice.ThreadPool.Client.SizeMax","10");
			proper->setProperty("Ice.ThreadPool.Server.Size","5");
			proper->setProperty("Ice.ThreadPool.Server.SizeMax","10");

			std::map<std::string, std::string>::iterator iter = _A3Config.icePropMap.begin();
			for (; iter != _A3Config.icePropMap.end(); ++iter) 
			{
				proper->setProperty(iter->first, iter->second);
				(_log)(::ZQ::common::Log::L_INFO, CLOGFMT(A3MsgEnv, "Set ICE Property %s=%s"), (*iter).first.c_str(), (*iter).second.c_str());			
			}
		}
		TianShanIce::Repository::ContentLibPrx A3MsgEnv::connectToContentLib()
		{
			std::string clEndpoint = "ContentLibApp:" + _A3Config.clibEndpoint;
			try {

				if(_clPrx)
					return _clPrx;

				_clPrx = TianShanIce::Repository::ContentLibPrx::checkedCast(_communicator->stringToProxy(clEndpoint.c_str()));
				(_log)(::ZQ::common::Log::L_INFO, CLOGFMT(A3MsgEnv, "connected contentlib service with (%s)"), clEndpoint.c_str());
			}
			catch(const Ice::Exception& ex)
			{
				_clPrx = NULL;
				(_log)(::ZQ::common::Log::L_WARNING, CLOGFMT(A3MsgEnv, "failed connect to contentlib service with (%s) caught ice exception (%s)"), 
					clEndpoint.c_str(), ex.ice_name().c_str());
			}
			return _clPrx;
		}
		TianShanIce::Storage::CacheStorePrx A3MsgEnv::connectToCacheStore()
		{
//#pragma message ( __MSGLOC__ "TODO: impl here")
			std::string csEndpoint = SERVICE_NAME_CacheStore + std::string(":") + _A3Config.csEndpoint;
			try {

				if(_csPrx)
					return _csPrx;

				_csPrx = TianShanIce::Storage::CacheStorePrx::checkedCast(_communicator->stringToProxy(csEndpoint.c_str()));
				(_log)(::ZQ::common::Log::L_INFO, CLOGFMT(A3MsgEnv, "connected cachestore service with (%s)"), csEndpoint.c_str());
			}
			catch(const Ice::Exception& ex) 
			{
				_csPrx = NULL;
				(_log)(::ZQ::common::Log::L_WARNING, CLOGFMT(A3MsgEnv, "failed connect to cachestore service with (%s) caught ice exception (%s)"), 
					csEndpoint.c_str(), ex.ice_name().c_str());
			} 
           return _csPrx;
		}
		bool A3MsgEnv::connectToEventChannel()
		{
			(_log)(ZQ::common::Log::L_DEBUG, CLOGFMT(A3MsgEnv, "do connectEventChannel()"));
			try
			{
				_eventChannel = new TianShanIce::Events::EventChannelImpl(_evtAdap, _A3Config.EventEndpoint.c_str());
				TianShanIce::Events::GenericEventSinkPtr a3EventPtr = new CRM::A3Message::A3MessageEventSinkI(*this);
				TianShanIce::Properties qos;
				_eventChannel->sink(a3EventPtr, qos);
				_eventChannel->start();
			}
			catch (const TianShanIce::BaseException& ex)
			{
				(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgEnv, "failed to connect to EventChannel(%s) caught(%s: %s)"),
					_A3Config.EventEndpoint.c_str(), ex.ice_name().c_str(), ex.message.c_str());
				return false;
			}
			catch (const Ice::Exception& ex)
			{
				(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgEnv, "failed to connect to EventChannel(%s) caught(%s)"), 
					_A3Config.EventEndpoint.c_str(), ex.ice_name().c_str());
				return false;
			}
            catch(...)
			{
				(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(A3MsgEnv, "failed to connect to EventChannel(%s) caught unknown exception(%d)"),
					_A3Config.EventEndpoint.c_str(), SYS::getLastErr());
				return false;
			}
			(_log)(ZQ::common::Log::L_NOTICE, CLOGFMT(A3MsgEnv, "connect to EventChannel(%s) successfully"),_A3Config.EventEndpoint.c_str());
			return true;
		}

		bool A3MsgEnv::addContentInfo(const std::string& contentName, ContentInfo& contentInfo)
		{
			ZQ::common::MutexGuard gd(_lockContentInfos);
			_contentInfos[contentName] = contentInfo;
			return true;
		}
		bool A3MsgEnv::removeContentInfo(const std::string& contentName)
		{
			ZQ::common::MutexGuard gd(_lockContentInfos);
			ZQ::common::LRUMap<std::string, ContentInfo>::iterator itor = _contentInfos.find(contentName);
			if(itor != _contentInfos.end())
				_contentInfos.erase(itor->first);
			return true;
		}
		ContentInfo A3MsgEnv::getContentInfo(const std::string&contentName)
		{
			/// for backstore = AquaServer, and  enableRaw=1
			/// CPE method: METHODTYPE_RTIRAW write file to disk, contentKey.content = _A3Config.mainFilePath  + strContentName;
			std::string strTempContentName = contentName;
			/*if(_A3Config.enableRaw && !_A3Config.mainFilePath.empty())
			{
              if(contentName.size() >  _A3Config.mainFilePath.size() && contentName.substr(0, _A3Config.mainFilePath.size()) ==_A3Config.mainFilePath)
				  strTempContentName = contentName.substr(_A3Config.mainFilePath.size());
			}*/
			ContentInfo contentInfo;
			ZQ::common::MutexGuard gd(_lockContentInfos);
			ZQ::common::LRUMap<std::string, ContentInfo>::iterator itor = _contentInfos.find(strTempContentName);
			if(itor != _contentInfos.end())
				contentInfo = itor->second;
			return contentInfo;
		}
		////////////////////////////////////////
		//////////////class ConnectIceStrom//////
		/////////////////////////////////////////
		ConnectEventChannelThread::ConnectEventChannelThread(CRM::A3Message::A3MsgEnv& env)
			: _env(env), _bExit(false)
		{
		}

		ConnectEventChannelThread::~ConnectEventChannelThread()
		{
		}

		bool ConnectEventChannelThread::init()
		{
			return true;
		}

		int ConnectEventChannelThread::run(void)
		{
			while (!_bExit)
			{
				if (_env.connectToEventChannel())
					_bExit = true;
				else 
					_event.wait(2000);
			}
			return 0;
		}
		void ConnectEventChannelThread::toExit(void)
		{
			_bExit = true;
			_event.signal();
		}
	}///end namespace A3Server
}///end namespace CRM
