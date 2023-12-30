#include "FileLog.h"
#include <stdio.h>
#include "IceLog.h"

#include "CDNCSService.h"
#include "MCCSCfg.h"
#include "ContentImpl.h"
#include "FileSystemOp.h"


#define CONFIGURATION_XML	"CDNCS.xml"

using namespace ZQ::common;

ZQTianShan::ContentStore::CDNCSEngineSvc g_server;
::ZQ::common::NativeThreadPool*	g_pMCCSThreadPool=NULL;
::ZQTianShan::ContentStore::ContentStoreImpl::Ptr g_pStore;

ZQADAPTER_DECLTYPE g_adapter;

Ice::CommunicatorPtr	g_ic;					/*<!-ice communicator*/	
ZQ::common::ZQDaemon *Application = &g_server;

Config::Loader<ConfigGroup> configGroup(CONFIGURATION_XML);
ZQ::common::Config::ILoader		*configLoader=&configGroup;

extern const char* DUMP_PATH;

#define MOLOG		glog
#define CDNCS		"CDNCS"	

namespace ZQTianShan{
	namespace ContentStore{

		CDNCSEngineSvc::CDNCSEngineSvc()			
		{
		}

		CDNCSEngineSvc::~CDNCSEngineSvc()
		{

		}

		bool CDNCSEngineSvc::OnInit()
		{
            DUMP_PATH = configGroup.dumpPath.c_str();

			std::string strLogFolder = _logDir;
			if (strLogFolder.at(strLogFolder.length()-1)!=FNSEPC) 
			{
				strLogFolder+=FNSEPS;
			}

			_EventLogFile=new ZQ::common::FileLog( (strLogFolder+ getServiceName() + "_events.log").c_str(),
				configGroup.iceTraceLevel ,
				ZQLOG_DEFAULT_FILENUM,
				configGroup.iceTraceSize);

			g_pMCCSThreadPool=new ZQ::common::NativeThreadPool();
			if ( !g_pMCCSThreadPool) 
			{
				MOLOG(Log::L_ERROR, CLOGFMT(CDNCS, "Can't create threadpool instance"));
				return false;
			}

			std::string strNetId="";
printf("argc: %d: argv[0]: %s\n", _argc, _argv[0]);
			if(_argc == 1 && _argv[0]) {
				strNetId = _argv[0];
			}

			MOLOG(Log::L_INFO, CLOGFMT(CDNCS, "Content Store NetId %s"),strNetId.c_str());

			if (!configGroup.setContentStoreNetId(strNetId))
			{
				if (!strNetId.empty())
				{
					MOLOG(Log::L_ERROR, CLOGFMT(CDNCS, "NetId %s does not exist"),strNetId.c_str());
				}
				return false;
			}

			{
				int i=0;
				Ice::InitializationData iceInitData;
				iceInitData.properties =Ice::createProperties(i,NULL);
				iceInitData.properties->setProperty("Ice.ThreadPool.Client.Size","5");
				iceInitData.properties->setProperty("Ice.ThreadPool.Client.SizeMax","10");
				iceInitData.properties->setProperty("Ice.ThreadPool.Server.Size","5");
				iceInitData.properties->setProperty("Ice.ThreadPool.Server.SizeMax","10");
				
				std :: map<std::string, std::string>::const_iterator it = configGroup.mccsConfig.iceProp.begin();
				for( ; it != configGroup.mccsConfig.iceProp.end(); it++ )
				{		
					iceInitData.properties->setProperty( it->first , it->second );
				}

				///initialize ice communicator
				if( configGroup.iceTraceEnabled)
				{
/*
					if (_iceLogFile != NULL)
					{
						delete _iceLogFile;
						_iceLogFile= NULL;
					}

					_iceLogFile=new ZQ::common::FileLog((strLogFolder+ getServiceName() + ".IceTrace.log").c_str(),
						configGroup.iceTraceLevel,
						ZQLOG_DEFAULT_FILENUM,
						configGroup.iceTraceSize);
*/

//					iceInitData.logger = new TianShanIce::common::IceLogI(_iceLogFile);
					iceInitData.logger = new TianShanIce::common::IceLogI(_logger);
				}

				g_ic=Ice::initialize(i,NULL,iceInitData);
			}

			g_pMCCSThreadPool->resize(configGroup.mccsConfig.workerThreadSize);

			try
			{	
				g_adapter = ZQADAPTER_CREATE(g_ic, ADAPTER_NAME_ContentStore, configGroup.mccsConfig.clusterEndpoint.c_str(), *m_pReporter);
			}
			catch(Ice::Exception& ex)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CDNCS, "Create g_adapter failed with endpoint=%s and exception is %s"),
					configGroup.mccsConfig.clusterEndpoint.c_str(), ex.ice_name().c_str());
				return false;
			}


			//
			// publish log to sentry
			//
			std::vector<MonitoredLog>::iterator iter;
			for (iter = configGroup.mccsConfig.monitoredLogs.begin(); iter != configGroup.mccsConfig.monitoredLogs.end(); ++iter) 
			{			
				if (!g_adapter->publishLogger(iter->name.c_str(), iter->syntax.c_str(), iter->syntaxKey.c_str()))				
				{				
					MOLOG(ZQ::common::Log::L_ERROR, "Failed to publish logger name[%s] synax[%s] key[%s]", 					
						iter->name.c_str(), iter->syntax.c_str(), iter->syntaxKey.c_str());				
				}			
				else				
				{				
					MOLOG(ZQ::common::Log::L_INFO, "Publish logger name[%s] synax[%s] key[%s] successful", 					
						iter->name.c_str(), iter->syntax.c_str(), iter->syntaxKey.c_str());				
				}			
			}	

			try
			{
				std::string	strDbPath ;
				strDbPath = configGroup.dbPath;
				if (!FS::createDirectory(strDbPath))
				{
					MOLOG(Log::L_ERROR, CLOGFMT(CDNCS, "Data db path %s error"), strDbPath.c_str());
					return false;
				}

				if (FNSEPC != strDbPath[strDbPath.length()-1])
					strDbPath += FNSEPS;
				strDbPath = strDbPath + getServiceName();
				if (!FS::createDirectory(strDbPath))
				{
					MOLOG(Log::L_ERROR, CLOGFMT(CDNCS, "Data db path %s error"), strDbPath.c_str());
					return false;
				}

				g_pStore = new ::ZQTianShan::ContentStore::ContentStoreImpl(*m_pReporter, *_EventLogFile, *g_pMCCSThreadPool, g_adapter, strDbPath.c_str());
				if (!g_pStore)
				{
					MOLOG(Log::L_ERROR, CLOGFMT(CDNCS, "create content store object failed."));
					return false;
				}

				g_pStore->_replicaGroupId = configGroup.mccsConfig.csStrReplicaGroupId;
				g_pStore->_replicaId = configGroup.mccsConfig.csStrReplicaId;
				g_pStore->_replicaPriority = configGroup.mccsConfig.csIReplicaPriority;
				g_pStore->_netId = configGroup.mccsConfig.netId;
				g_pStore->_contentEvictorSize = configGroup.mccsConfig.contentEvictorSize;
				g_pStore->_volumeEvictorSize = configGroup.mccsConfig.volumeEvictorSize;
				g_pStore->_replicaTimeout = configGroup.mccsConfig.csReplicaTimeout;
				g_pStore->_cacheable = configGroup.mccsConfig.isCacheMode;
				g_pStore->_cacheLevel = configGroup.mccsConfig.cacheLevel & 0xff;
				g_pStore->_exposeStreamService = 0;
				g_pStore->_contentSavePeriod = configGroup.mccsConfig.contentSavePeriod;
				g_pStore->_contentSaveSizeTrigger = configGroup.mccsConfig.contentSaveSizeTrigger;
				g_pStore->_storeAggregate = 1;
				g_pStore->_enableInServiceCheck = configGroup.mccsConfig.enalbeInServiceCheck;

				g_pStore->_storeType = "SeaChange.MediaCluster";
				if (g_pStore->_cacheable)
					g_pStore->_storeType += ".cache";

				g_pStore->_autoFileSystemSync = false;
				if (!g_pStore->initializeContentStore())
				{
					MOLOG(Log::L_ERROR, CLOGFMT(CDNCS, "initializeContentStore() failed"));
					return false;
				}
			}
			catch(Ice::Exception& ex)
			{
				MOLOG(Log::L_ERROR, CLOGFMT(CDNCS, "create content store object failed and exception is %s"), ex.ice_name().c_str());
				return false;
			}
			
			return ZQDaemon::OnInit();
		}

		bool CDNCSEngineSvc::OnStart()
		{
			try
			{
				g_adapter->activate();
			}
			catch(const Ice::Exception& ex)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CDNCS, "activate g_adapter caught exception: %s"), ex.ice_name().c_str());
			}

			return ZQDaemon ::OnStart();
		}

		void CDNCSEngineSvc::OnUnInit()
		{
			try
			{
				if (g_pStore)
				{
					g_pStore= NULL;
				}
			}
			catch (...)
			{
			}

			try
			{
				g_adapter->destroy();
				g_ic->shutdown();
				g_ic->destroy();
			}
			catch (...)
			{
			}	

/*
			try
			{
				if (_iceLogFile)
				{
					delete _iceLogFile;
					_iceLogFile= NULL;
				}
			}
			catch (...)
			{
				_iceLogFile= NULL;
			}
*/
			try
			{
				if (_EventLogFile)
				{
					delete _EventLogFile;
					_EventLogFile= NULL;
				}
			}
			catch (...)
			{
				_EventLogFile= NULL;
			}

			if(g_pMCCSThreadPool)
			{
				try
				{
					delete g_pMCCSThreadPool;
					g_pMCCSThreadPool=NULL;
				}
				catch(...){ }
			}

			return ZQDaemon ::OnUnInit();	
		}

		void CDNCSEngineSvc::OnStop()
		{
			return ZQDaemon::OnStop();	
		}
}}
