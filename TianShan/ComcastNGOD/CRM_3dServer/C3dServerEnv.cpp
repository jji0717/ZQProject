#include "C3dServerEnv.h"
#include "ZQResource.h"
#include "C3dServerConfig.h"
#include "TianShanDefines.h"
#include "C3dServerMsgHandler.h"

#include "CdmiClientBase.h"
ZQ::common::Config::Loader< C3dServerCfg > _3dServerConfig("");
namespace CRM
{
	namespace C3dServer
	{
		C3dServerEnv::C3dServerEnv(void)
		{
          _pIceLog = NULL;
		  _pCdmiClient = NULL;
		}

		C3dServerEnv::~C3dServerEnv(void)
		{
			delete _cdmiClientPool;
		}
		static void fixPath(std::string& strPath)
		{
			size_t nlen = strPath.length();
			if(strPath[nlen-1] != FNSEPC )
				strPath += FNSEPS;
		}
		bool C3dServerEnv::loadConfig(const std::string& strCfgPath)
		{
			if(_mgr)
			{
              _3dServerConfig.setLogger(&(_mgr->superLogger()));
			}
			if(!_3dServerConfig.load(strCfgPath.c_str()))
			{
				return false;
			}
            
			if(!_3dServerConfig.contentFolder.empty() && _3dServerConfig.contentFolder[_3dServerConfig.contentFolder.length()-1] != '/')
				_3dServerConfig.contentFolder += "/";

			if(!_3dServerConfig.sourceFolder.empty() && _3dServerConfig.sourceFolder[_3dServerConfig.sourceFolder.length()-1] != '/')
				_3dServerConfig.sourceFolder += "/";
			return true;
		}
		bool C3dServerEnv::doInit(const std::string& strLogPath, const std::string& strCfgPath)
		{
			std::string strPathOfLog = strLogPath;
			std::string strPathOfCfg = strCfgPath;	
			fixPath(strPathOfLog);
			fixPath(strPathOfCfg);

			strPathOfCfg += "CRM_3dServer.xml";
			if (!loadConfig(strPathOfCfg))
			{
				return false;
			}

			try
			{
				_log.open((strPathOfLog +_3dServerConfig.logfilename).c_str(), 7);
			}
			catch (ZQ::common::FileLogException& ex)
			{
				ZQ::common::Log& log = _mgr->superLogger();
				log(ZQ::common::Log::L_ERROR, "doInit() : create log [%s] caught %s", strPathOfLog.c_str(), ex.getString());
				return false;
			}

			_log.setFileSize(_3dServerConfig.lLogFileSize);
			_log.setFileCount(_3dServerConfig.llogFileCount);
			_log.setLevel(_3dServerConfig.lLogLevel);
			_log.setBufferSize(_3dServerConfig.lLogBufferSize);

			/// initializa ice Communicator  and adapter
			try
			{
				_pIceLog = new ZQ::common::FileLog((strPathOfLog + _3dServerConfig.Icelogfilename).c_str(),
					_3dServerConfig.lIceLogLevel,
					_3dServerConfig.lIcelogFileCount,
					_3dServerConfig.lIceLogFileSize,
					_3dServerConfig.lIceLogBufferSize,
					_3dServerConfig.lIceLogWriteTimteout);

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
				(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerEnv, "initialize ICE communicator failed, exception(%s)"), ex.ice_name().c_str());
				return false;
			}
			catch(...)
			{
				(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerEnv, "initialize ICE communicator caught unknown exception(%d)"), SYS::getLastErr());
				return false;
			}

			///init _pCdmiClient
			_cdmiClientPool = new ZQ::common::NativeThreadPool(_3dServerConfig.maxThreadPoolSize);
			_pCdmiClient = new CdmiClientBase(_log,*_cdmiClientPool,_3dServerConfig.rootUrl,_3dServerConfig.homeContainer);
			if(!_pCdmiClient)
			{
				(_log)(ZQ::common::Log::L_ERROR,CLOGFMT(C3dServerEnv,"doInit():faild to create cdmiClientBase"));
				return false;
			}
			ZQ::common::CURLClient::startCurlClientManager();

			/// create 3dServerMsgHandler
			_3dServerMsgHandler = new CRM::C3dServer::C3dServerMsgHandler(*this);
			if (!_3dServerMsgHandler)
			{
				(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerEnv, "doInit() : Fail to create A3 Message handler"));
				return false;
			}
			_pCdmiClient->setTimeout(_3dServerConfig.connectTimeout, _3dServerConfig.timeout);
			_3dServerConfig.snmpRegister("C3dServer");
            
            return true;
		}

		/// this method must be called no matter if doInit() success or fail
		void C3dServerEnv::doUninit()
		{
			try
			{ 
				if(_3dServerMsgHandler) 
					delete _3dServerMsgHandler;

				_3dServerMsgHandler  = NULL;
				
			}
			catch (...){}

			try{
				if (_communicator )
				{
					_communicator->destroy();
					(_log)(ZQ::common::Log::L_NOTICE, CLOGFMT(C3dServerEnv, "Communicator->destroy() successfully"));
				}

				if(_pIceLog)
					delete _pIceLog;
			}
			catch (const Ice::Exception& ex)
			{
				(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(C3dServerEnv, "destory communicator caught(%s)"), ex.ice_name().c_str());
			}
          
			if(_pCdmiClient)
			{
				delete _pCdmiClient;
				_pCdmiClient = NULL;
			}
		   _pIceLog = NULL;
		   _iceLog = NULL;
		   _communicator = NULL;
		   ZQ::common::CURLClient::stopCurlClientManager();

		}
		void C3dServerEnv::initWithConfig(Ice::PropertiesPtr proper )
		{
			proper->setProperty("Ice.ThreadPool.Client.Size","5");
			proper->setProperty("Ice.ThreadPool.Client.SizeMax","10");
			proper->setProperty("Ice.ThreadPool.Server.Size","5");
			proper->setProperty("Ice.ThreadPool.Server.SizeMax","10");

			std::map<std::string, std::string>::iterator iter = _3dServerConfig.icePropMap.begin();
			for (; iter != _3dServerConfig.icePropMap.end(); ++iter) 
			{
				proper->setProperty(iter->first, iter->second);
				(_log)(::ZQ::common::Log::L_INFO, CLOGFMT(C3dServerEnv, "Set ICE Property %s=%s"), (*iter).first.c_str(), (*iter).second.c_str());			
			}
		}

		TianShanIce::ContentProvision::ContentProvisionServicePrx C3dServerEnv::getCPEProxy()
		{
			std::string cpeEndpoint = std::string(SERVICE_NAME_ContentProvisionService ":") + _3dServerConfig.EndpointOfCPE;
			try {

				if(_cpePrx)
					return _cpePrx;

				_cpePrx = TianShanIce::ContentProvision::ContentProvisionServicePrx::checkedCast(_communicator->stringToProxy(cpeEndpoint));
				(_log)(::ZQ::common::Log::L_INFO, CLOGFMT(C3dServerEnv, "connected CPE service with (%s)"), cpeEndpoint.c_str());
			}
			catch(const Ice::Exception& ex) 
			{
				_cpePrx = NULL;
				(_log)(::ZQ::common::Log::L_WARNING, CLOGFMT(C3dServerEnv, "failed connect to CPE service with (%s) caught ice exception (%s)"), 
					cpeEndpoint.c_str(), ex.ice_name().c_str());
			} 
			return _cpePrx;
		}

	}///end namespace 3dServer
}///end namespace CRM
