#include "MCUSSSvc.h"
#include "IceLog.h"

#include <string>

#ifdef ZQ_OS_MSWIN
#include <MiniDump.h>
#endif

namespace ZQ{
	namespace StreamService{
		ZQ::StreamService::SsServiceImpl* pServiceInstance = NULL;
		SsEnvironment*	gSsEnvironment = NULL;
	}}

ZQTianShan::MCUSS::MCUSSService					gMCUSSServiceInstance;
ZQ::common::BaseZQServiceApplication		*Application=&gMCUSSServiceInstance;

ZQ::common::Config::Loader<ZQTianShan::MCUSS::MCUSSCfg> gMCUSSConfig("MCUSS.xml");
ZQ::common::Config::ILoader					*configLoader = &gMCUSSConfig;

ZQTianShan::MCUSS::ST_MCUSS::MCUSSHolder	*pMCUSSBaseConfig = NULL;

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

static ZQ::common::MiniDump			_crashDump;
//void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);
void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();

	glog( ZQ::common::Log::L_CRIT, 
		_T("Crash exception callback called,ExceptionCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x"),
		ExceptionCode, ExceptionAddress, dwThreadID);
	glog.flush();	
}
#else
extern const char* DUMP_PATH;
#endif


namespace ZQTianShan{
	namespace MCUSS{
		MCUSSService::MCUSSService(void)
		{

#ifdef ZQ_OS_MSWIN
			strcpy(servname, "MCUSS");
			strcpy(prodname, "TianShan");
#endif
			mIc				=	NULL;
			mpServiceImpl	=	NULL;
			nssSnmpAgent    =   NULL;
		}

		MCUSSService::~MCUSSService(void)
		{
		}

		bool MCUSSService::initializeCrashDumpLocation( )
		{
#ifdef ZQ_OS_MSWIN
			try
			{
				if(! ZQTianShan::Util::fsCreatePath( gMCUSSConfig._szCrashDumpPath) )
				{
					glog(ZQ::common::Log::L_ERROR,CLOGFMT(MCUSSService,"can't create path [%s] for mini dump "),
						gMCUSSConfig._szCrashDumpPath);
					return false;
				}	
				_crashDump.setDumpPath((char *)gMCUSSConfig._szCrashDumpPath);
				_crashDump.enableFullMemoryDump(gMCUSSConfig._crashDumpEnabled);
				_crashDump.setExceptionCB(CrashExceptionCallBack);
			}
			catch (...)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(MCUSSService, "unexpected exception caught when initializeCrashDumpLocation"));
				return false;
			}
#else
			DUMP_PATH = gMCUSSConfig._szCrashDumpPath;
#endif
			return true;
		}

		std::string	MCUSSService::getLoggerFolder( ) const
		{
#ifdef ZQ_OS_MSWIN
			std::string folderPath = m_wsLogFolder;
#else
			std::string folderPath = _logDir;
#endif
			if ( folderPath.at(folderPath.length()-1) != FNSEPC)
			{
				folderPath += FNSEPS;
			}
			return folderPath;
		}

		bool MCUSSService::initializeLogger( )
		{
#ifdef ZQ_OS_MSWIN
			::std::string strLogBaseFolder = m_wsLogFolder;
			std::string svc_name = servname;
#else
			::std::string strLogBaseFolder = _logDir;
			std::string svc_name = getServiceName();
#endif
			::std::string strNSSLog	= strLogBaseFolder + FNSEPS + svc_name + ".sess.log";

			try
			{
#ifdef ZQ_OS_MSWIN
				int32 iLogLevel			= m_dwLogLevel;
				int32 iLogMaxCount		= m_dwLogFileCount;
				int32 iLogSize			= m_dwLogFileSize;
				int32 iLogBufferSize	= m_dwLogBufferSize;
				int32 iLogFlushTimeout	= m_dwLogWriteTimeOut;
#else
				int32 iLogLevel			= _logLevel;
				int32 iLogMaxCount		= ZQLOG_DEFAULT_FILENUM;
				int32 iLogSize			= _logSize;
				int32 iLogBufferSize	= _logBufferSize;
				int32 iLogFlushTimeout	= _logTimeout;
#endif

				mSessionLogger.open(strNSSLog.c_str(),
					iLogLevel,
					iLogMaxCount,
					iLogSize,
					iLogBufferSize,
					iLogFlushTimeout);

				if( gMCUSSConfig._iceTraceLogEnabled >= 1 )
				{
					strNSSLog = strLogBaseFolder + FNSEPS + svc_name + "_ice.log";
					mIceFileLogger.open( strNSSLog.c_str() ,
						iLogLevel,
						iLogMaxCount,
						iLogSize,
						iLogBufferSize,
						iLogFlushTimeout);
				}
			}
			catch( const ZQ::common::FileLogException& ex)
			{
				glog(ZQ::common::Log::L_ERROR,
					CLOGFMT(MCUSSService,"failed to open log file[%s] because [%s]"),
					strNSSLog.c_str(),
					ex.what() );
				return false;
			}
			return true;
		}

		void MCUSSService::uninitializeLogger( )
		{

		}

		bool MCUSSService::InitializeIceRunTime( )
		{
			try
			{
				Ice::InitializationData		iceInitData;
				int i = 0;
				iceInitData.properties =Ice::createProperties( i , NULL );

				std::stringstream ss;
				ss << pMCUSSBaseConfig->_bindDispatchSize;
				iceInitData.properties->setProperty( ADAPTER_NAME_NSS".ThreadPool.Size", ss.str());
				ss << pMCUSSBaseConfig->_bindDispatchMax;
				iceInitData.properties->setProperty( ADAPTER_NAME_NSS".ThreadPool.SizeMax",ss.str());
				glog(ZQ::common::Log::L_INFO,CLOGFMT(MCUSSService,"set dispatchSize/dispatchSizeMax to [%d/%d]"),
					pMCUSSBaseConfig->_bindDispatchSize,
					pMCUSSBaseConfig->_bindDispatchMax);
				//StreamService

				//set ice properties

				std::map<std::string, std::string>::const_iterator it = pMCUSSBaseConfig->_icePropertiesMap.begin();
				for(; it != pMCUSSBaseConfig->_icePropertiesMap.end(); it++)
				{
					iceInitData.properties->setProperty(it->first, it->second);
					glog(ZQ::common::Log::L_INFO,CLOGFMT(MCUSSService,"Set ice property [%s] \t\t\t= [%s]"),
						it->first.c_str() , it->second.c_str() );

				}
				if( gMCUSSConfig._iceTraceLogEnabled >= 1 )
				{
					iceInitData.logger = new TianShanIce::common::IceLogI( &mIceFileLogger );
					assert( iceInitData.logger );
				}

				mIc	=	Ice::initialize( i , NULL , iceInitData );
				return ( mIc );
			}
			catch( const Ice::Exception& ex )
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(MCUSSService, "[%s] caught when InitializeIceRunTime"), ex.ice_name().c_str());
				return false;
			}
			catch(...)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(MCUSSService, "unexpected exception caught when InitializeIceRunTime"));
				return false;
			}
		}

		void MCUSSService::UninitializeIceRunTime()
		{
			try
			{
				mpMCUSSEnv->iceCommunicator = NULL;
				if(mIc)
				{
					mIc->destroy();
					mIc = NULL;
				}
			}
			catch(...)	{}
		}

		bool MCUSSService::initializeServiceParameter( )
		{

			//
			// init adapter
			//	
			try
			{
				mainAdapter = ZQADAPTER_CREATE( mIc,"MCUSS" ,pMCUSSBaseConfig->_bindEndpoint.c_str(), *m_pReporter);
				assert(mainAdapter);
			}
			catch(Ice::Exception& ex)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(MCUSSService, "Create adapter failed with endpoint=%s and exception is %s"),
					pMCUSSBaseConfig->_bindEndpoint.c_str(), ex.ice_name().c_str());
				return false;
			}

#ifdef ZQ_OS_MSWIN
			mpMCUSSEnv->setServiceName(servname);
#else
			mpMCUSSEnv->setServiceName(getServiceName().c_str());
#endif
			//
			// init streamservice
			//
			mpMCUSSEnv->mainAdapter		= mainAdapter;
			mpMCUSSEnv->iceCommunicator	= mIc;

			try
			{
				mpMCUSSEnv->init();
			}
			catch( const Ice::Exception& ex )
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(MCUSSService, "failed to initialize StreamService, caught[%s]"), ex.ice_name().c_str());
				return false;
			}
			catch(...)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(MCUSSService, "failed to initialize StreamService, caught runtime exception"));
				return false;
			}

			glog(ZQ::common::Log::L_INFO, CLOGFMT(MCUSSService, "StreamService module initialized successfully"));

			return true;
		}

		HRESULT MCUSSService::OnInit( )
		{
			//get regedit argument
			std::string strNetId = "";
			if(m_argc == 1 && strlen(m_argv[0]))
			{
				strNetId = m_argv[0];
			}	


			glog(ZQ::common::Log::L_INFO, CLOGFMT(MCUSSService, "netID='%s'"), strNetId.c_str());

			for (::ZQTianShan::MCUSS::MCUSSCfg::MCUSSList::iterator iter = gMCUSSConfig._mcussList.begin(); iter != gMCUSSConfig._mcussList.end(); iter++)
			{
				if (strNetId == iter->_netId)
				{
					glog(ZQ::common::Log::L_INFO, CLOGFMT(MCUSSService, "find NetID=%s"), strNetId.c_str());
					pMCUSSBaseConfig = &(*iter);
				}
			}

			if (NULL == pMCUSSBaseConfig)
			{
				glog(ZQ::common::Log::L_ERROR, CLOGFMT(MCUSSService, "couldn't find specified netID[%s]"), strNetId.c_str());
				return S_FALSE;
			}

			mStreamServiceThreadpool.resize( pMCUSSBaseConfig->_bindThreadPoolSize );

			_thrdPoolRTSP.resize(max(pMCUSSBaseConfig->_bindThreadPoolSize/4, 20));

			if ( !initializeCrashDumpLocation() )
			{
#ifdef ZQ_OS_MSWIN
				logEvent(ZQ::common::Log::L_ERROR,_T("failed to initialize crash dump location"));
#endif
				return S_FALSE;
			}

			if (!initializeLogger())
			{
#ifdef ZQ_OS_MSWIN
				logEvent( ZQ::common::Log::L_ERROR,_T("failed to open log"));
#endif
				return S_FALSE;
			}

			mpMCUSSEnv = new ZQTianShan::MCUSS::MCUSSEnv(*m_pReporter, mSessionLogger, _thrdPoolRTSP, mStreamServiceThreadpool);
			mpMCUSSEnv->setNetId(strNetId);

			// initialize ice run time
			if( !InitializeIceRunTime() )
			{
#ifdef ZQ_OS_MSWIN
				logEvent(ZQ::common::Log::L_ERROR, _T("failed to initialize ice run time"));
#endif
				return S_FALSE;
			}	

			if (!initializeServiceParameter())
			{
#ifdef ZQ_OS_MSWIN
				logEvent(ZQ::common::Log::L_ERROR, _T("failed to initialize service parameter"));
#endif
				return S_FALSE;
			}	

			ZQ::StreamService::gSsEnvironment	=	mpMCUSSEnv;
			mpServiceImpl = new ZQ::StreamService::SsServiceImpl( mpMCUSSEnv , "MCUSS" );	
			ZQ::StreamService::pServiceInstance	= mpServiceImpl;
			mpServiceImpl->strCheckpointPeriod	=	pMCUSSBaseConfig->_videoServerHolder.streamDbPerfTune.databaseStreamCheckpointPeriod;
			mpServiceImpl->strSavePeriod		=	pMCUSSBaseConfig->_videoServerHolder.streamDbPerfTune.databaseStreamSavePeriod;
			mpServiceImpl->strSaveSizeTrigger	=	pMCUSSBaseConfig->_videoServerHolder.streamDbPerfTune.databaseStreamSaveSizeTrigger;

			if(NULL == nssSnmpAgent)
				nssSnmpAgent = new ZQ::Snmp::Subagent(1400, 5, Application->getInstanceId());

			using namespace ZQ::Snmp;
			nssSnmpAgent->addObject(Oid("1.2"), ManagedPtr(new SimpleObject(VariablePtr(new nssSessionCount(mpServiceImpl)), AsnType_Integer, aReadWrite)));

			//init FixedSpeedSet
			std::vector<std::string> temps;
			std::vector<float>& inputFFs = pMCUSSBaseConfig->_videoServerHolder.sessionInterfaceHolder.FixedSpeedSetForwardSet;
			std::string strInputFF = pMCUSSBaseConfig->_videoServerHolder.sessionInterfaceHolder.FixedSpeedSetForward;
			temps.clear();
			ZQ::common::stringHelper::SplitString(strInputFF,temps," "," ","","");
			for( std::vector<std::string>::const_iterator it = temps.begin() ; it != temps.end() ; it ++ )
				inputFFs.push_back((float)atof(it->c_str()));

			std::vector<float>& inputREWs = pMCUSSBaseConfig->_videoServerHolder.sessionInterfaceHolder.FixedSpeedSetBackwardSet;
			std::string strInputREW = pMCUSSBaseConfig->_videoServerHolder.sessionInterfaceHolder.FixedSpeedSetBackward;
			temps.clear();
			ZQ::common::stringHelper::SplitString(strInputREW,temps," "," ","","");
			for( std::vector<std::string>::const_iterator it = temps.begin() ; it != temps.end() ; it ++ )
				inputREWs.push_back((float)atof(it->c_str()));

			return ZQ::common::BaseZQServiceApplication::OnInit();
		}

		HRESULT MCUSSService::OnStart()
		{
			if ( strlen( gMCUSSConfig._szIceRuntimeDbFolder ) <= 0)
				strcpy(gMCUSSConfig._szIceRuntimeDbFolder,gMCUSSConfig._szIceDbFolder);
			//gMCUSSConfig._szIceRuntimeDbFolder = gMCUSSConfig._szIceDbFolder;
			std::string dbPath = ZQTianShan::Util::fsConcatPath( gMCUSSConfig._szIceRuntimeDbFolder, mpMCUSSEnv->_strServiceName);

			if (!mpServiceImpl->start(dbPath, gMCUSSConfig._eventChannelEndpoint, "", mpMCUSSEnv->_netId, mpMCUSSEnv->_strServiceName))
			{
				glog(ZQ::common::Log::L_EMERG,CLOGFMT(MCUSSService,"failed to start service instance"));
				return -1;
			}
			if (NULL != nssSnmpAgent)
			{
				nssSnmpAgent->setLogger(m_pReporter);
				nssSnmpAgent->start();
				glog(ZQ::common::Log::L_INFO, CLOGFMT(MCUSSService, "start(), snmp agent succeed"));
			}
			else
				glog(ZQ::common::Log::L_INFO, CLOGFMT(MCUSSService, "start(), snmp agent failed"));

			//ZQ::common::RTSPSession::startDefaultSessionManager();

#ifndef _INDEPENDENT_ADAPTER
			{ // publish logs
				for(size_t i = 0; i < pMCUSSBaseConfig->_publishedLog._logDatas.size(); ++i) {
					const ZQTianShan::MCUSS::PublishLog& pl= pMCUSSBaseConfig->_publishedLog._logDatas[i];
					if(pl._path.empty() || pl._syntax.empty() || pl._key.empty()) {
						glog(ZQ::common::Log::L_WARNING, CLOGFMT(MCUSSService,"publishLogger() Bad logger configuration. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
						continue;
					}
					if(mainAdapter->publishLogger(pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str())) {
						glog(ZQ::common::Log::L_INFO, CLOGFMT(MCUSSService,"publishLogger() Successfully publish logger. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
					} else {
						glog(ZQ::common::Log::L_ERROR, CLOGFMT(MCUSSService,"publishLogger() Failed to publish logger. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
					}
				}
			}
#endif

			mpMCUSSEnv->start();
			mpMCUSSEnv->getMainAdapter()->activate();
			return ZQ::common::BaseZQServiceApplication::OnStart();
		}

		HRESULT MCUSSService::OnStop()
		{
			if (NULL != nssSnmpAgent)
			{
				nssSnmpAgent->stop();
				glog(ZQ::common::Log::L_INFO, CLOGFMT(MCUSSService, "OnStop() snmp agent stopped"));
			}

#ifndef _INDEPENDENT_ADAPTER
			{ // unpublish logs
				for(size_t i = 0; i < pMCUSSBaseConfig->_publishedLog._logDatas.size(); ++i) {
					const ZQTianShan::MCUSS::PublishLog& pl= pMCUSSBaseConfig->_publishedLog._logDatas[i];
					if(pl._path.empty() || pl._syntax.empty() || pl._key.empty()) {
						glog(ZQ::common::Log::L_WARNING, CLOGFMT(MCUSSService,"OnStop() Bad logger configuration. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
						continue;
					}
					if(mainAdapter->unpublishLogger(pl._path.c_str())) {
						glog(ZQ::common::Log::L_INFO, CLOGFMT(MCUSSService,"unpublishLogger() Successfully unpublish logger. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
					} else {
						glog(ZQ::common::Log::L_ERROR, CLOGFMT(MCUSSService,"unpublishLogger() Failed to unpublish logger. name[%s] synax[%s] key[%s] type[%s]"), pl._path.c_str(), pl._syntax.c_str(), pl._key.c_str(), pl._type.c_str());
					}
				}
			}
#endif

			if (mpServiceImpl )
			{
				mpServiceImpl->stop();
				//should I delete service instance or just leave it to Ice runtime ?
				glog(ZQ::common::Log::L_INFO, CLOGFMT(MCUSSService,"MCUSSService stopped"));
			}

			//mNGODCSEnv.uninitEnv();
			//glog(ZQ::common::Log::L_INFO, CLOGFMT(MCUSSService,"ContentStore uninitialized"));

			//ZQ::common::RTSPSession::stopDefaultSessionManager();

			if( mpMCUSSEnv->mainAdapter )
				mpMCUSSEnv->mainAdapter->deactivate();	
			glog(ZQ::common::Log::L_INFO, CLOGFMT(MCUSSService,"Adapter deactivated"));

			mStreamServiceThreadpool.stop();
			//	mContentStorethreadpool.stop();
			glog(ZQ::common::Log::L_INFO, CLOGFMT(MCUSSService,"ThreadPool stopped"));

			return ZQ::common::BaseZQServiceApplication::OnStop();
		}

		HRESULT MCUSSService::OnUnInit()
		{
			UninitializeIceRunTime();
			glog(ZQ::common::Log::L_INFO, CLOGFMT(MCUSSService, "Ice runtime destroyed"));

			mpMCUSSEnv->uninit();
			glog(ZQ::common::Log::L_INFO, CLOGFMT(MCUSSService, "MCUSS enviroment uninitialized"));
			if(mpMCUSSEnv)
			{		
				delete mpMCUSSEnv;
				mpMCUSSEnv = NULL;
			}

			if (NULL != nssSnmpAgent)
			{
				ZQ::Snmp::Subagent*  tmpAgent = nssSnmpAgent;
				nssSnmpAgent = NULL;
				delete tmpAgent;
				glog(ZQ::common::Log::L_INFO, CLOGFMT(MCUSSService, "snmp agent uninitialized"));
			}

			if (mpServiceImpl )
			{
				delete mpServiceImpl;
				mpServiceImpl = NULL;
				glog(ZQ::common::Log::L_INFO, CLOGFMT(MCUSSService,"MCUSSService uninitialized"));
			}

			uninitializeLogger();
			return ZQ::common::BaseZQServiceApplication::OnUnInit();
		}
	} //namespace ZQTianShan
} //namespace NssStrem