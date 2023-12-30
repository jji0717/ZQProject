#include "GBVSSSvc.h"
#include "ServantFactory.h"
#include "GBVSSCfgLoader.h"
#ifdef ZQ_OS_MSWIN
#include <MiniDump.h>
#endif
#include "TianShanIceHelper.h"
#include "IceLog.h"

namespace ZQ{
    namespace StreamService{
        ZQ::StreamService::SsServiceImpl* pServiceInstance = NULL;
        SsEnvironment*	gSsEnvironment = NULL;
    }}


GBVSSService					gGBVSSServiceInstance;
ZQ::common::BaseZQServiceApplication		*Application=&gGBVSSServiceInstance;
ZQ::common::Config::Loader<ZQTianShan::GBVSS::GBVSSCfg> gGBVSSConfig("GBVSS.xml");

ZQ::common::Config::ILoader					*configLoader = &gGBVSSConfig;
ZQTianShan::GBVSS::GBVSSBaseConfig::GBVSSHolder	*pGBVSSBaseConfig = NULL;

ZQ::common::NativeThreadPool* gPool = NULL;

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;

static ZQ::common::MiniDump			_crashDump;
void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress);

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

bool GBVSSService::initializeCrashDumpLocation( )
{
#ifdef ZQ_OS_MSWIN
    try
    {
        if(! ZQTianShan::Util::fsCreatePath( gGBVSSConfig._crashDump.path) )
        {
            glog(ZQ::common::Log::L_ERROR,CLOGFMT(GBVSSService,"can't create path [%s] for mini dump "),
                gGBVSSConfig._crashDump.path);
            return false;
        }	
        _crashDump.setDumpPath((char *)gGBVSSConfig._crashDump.path.c_str());
        _crashDump.enableFullMemoryDump(gGBVSSConfig._crashDump.enabled);
        _crashDump.setExceptionCB(CrashExceptionCallBack);
    }
    catch (...)
    {
        glog(ZQ::common::Log::L_ERROR, CLOGFMT(GBVSSService, "unexpected exception caught when initializeCrashDumpLocation"));
        return false;
    }
#else
    DUMP_PATH = gGBVSSConfig._crashDump.path.c_str();
#endif
    return true;
}

std::string	GBVSSService::getLoggerFolder( ) const
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

void GBVSSService::uninitializeLogger( )
{

}

bool GBVSSService::initializeLogger( )
{
#ifdef ZQ_OS_MSWIN
    ::std::string strLogBaseFolder = m_wsLogFolder;
    std::string svc_name = servname;
#else
    ::std::string strLogBaseFolder = _logDir;
    std::string svc_name = getServiceName();
#endif
    ::std::string strGBVSSLog	= strLogBaseFolder + FNSEPS + svc_name + ".log";

    try
    {
        // 		mSessionLogger.open(gGBVSSConfig._logFile.path.c_str(),
        // 			gGBVSSConfig._logFile.level,
        // 			ZQLOG_DEFAULT_FILENUM,
        // 			gGBVSSConfig._logFile.size,
        // 			gGBVSSConfig._logFile.bufferSize,
        // 			gGBVSSConfig._logFile.flushTimeout);
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

        mSessionLogger.open(strGBVSSLog.c_str(),
            iLogLevel,
            iLogMaxCount,
            iLogSize,
            iLogBufferSize,
            iLogFlushTimeout);

        if( gGBVSSConfig._iceTrace.enabled >= 1 )
        {
            strGBVSSLog = strLogBaseFolder + FNSEPS + svc_name + "_ice.log";
            mIceFileLogger.open( strGBVSSLog.c_str() ,
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
            CLOGFMT(GBVSSService,"failed to open log file[%s] because [%s]"),
            strGBVSSLog.c_str(),
            ex.what() );
        return false;
    }
    return true;
}

#define ADAPTERNAME_STREAM_SERVICE "StreamService"

#define DEFAULT_BINDPORT_GBVSS				10800
#define ADAPTER_NAME_GBVSS				        ADAPTER_NAME(GBVSS)
#define DEFAULT_ENDPOINT_GBVSS			        DEFAULT_ENDPOINT(GBVSS)
bool GBVSSService::InitializeIceRunTime( )
{
    try
    {
        Ice::InitializationData		iceInitData;
        int i = 0;
        iceInitData.properties =Ice::createProperties( i , NULL );

        std::stringstream ss;
        ss << pGBVSSBaseConfig->_bind.dispatchSize;
        iceInitData.properties->setProperty( ADAPTER_NAME_GBVSS".ThreadPool.Size", ss.str());
        ss << pGBVSSBaseConfig->_bind.dispatchMax;
        iceInitData.properties->setProperty( ADAPTER_NAME_GBVSS".ThreadPool.SizeMax",ss.str());
        glog(ZQ::common::Log::L_INFO,CLOGFMT(GBVSSService,"set dispatchSize/dispatchSizeMax to [%d/%d]"),
            pGBVSSBaseConfig->_bind.dispatchSize,
            pGBVSSBaseConfig->_bind.dispatchMax);
        //StreamService

        //set ice properties
        ::ZQTianShan::GBVSS::IceProperties::props::const_iterator it = pGBVSSBaseConfig->_iceProperty._props.begin();
        for( ; it != pGBVSSBaseConfig->_iceProperty._props.end() ; it ++ )
        {
            iceInitData.properties->setProperty( it->name , it->value );
            glog(ZQ::common::Log::L_INFO,CLOGFMT(GBVSSService,"Set ice property [%s] \t\t\t= [%s]"),
                it->name.c_str() , it->value.c_str() );
        }
        if( gGBVSSConfig._iceTrace.enabled >= 1 )
        {
            iceInitData.logger = new TianShanIce::common::IceLogI( &mIceFileLogger );
            assert( iceInitData.logger );
        }

        mIc	=	Ice::initialize( i , NULL , iceInitData );
        return ( mIc );
    }
    catch( const Ice::Exception& ex )
    {
        glog(ZQ::common::Log::L_ERROR, CLOGFMT(GBVSSService, "[%s] caught when InitializeIceRunTime"), ex.ice_name().c_str());
        return false;
    }
    catch(...)
    {
        glog(ZQ::common::Log::L_ERROR, CLOGFMT(GBVSSService, "unexpected exception caught when InitializeIceRunTime"));
        return false;
    }
}

bool GBVSSService::initializeServiceParameter( )
{
    //
    // init adapter
    //
	if( pGBVSSBaseConfig->_videoServer.enableMessageBinaryDump )
		ZQ::common::RTSPClient::setVerboseFlags( RTSP_VERBOSEFLG_RECV_HEX | RTSP_VERBOSEFLG_SEND_HEX );

    try
    {
        mainAdapter = ZQADAPTER_CREATE( mIc,ADAPTER_NAME_GBVSS ,pGBVSSBaseConfig->_bind.endPoint.c_str(), *m_pReporter);
        assert(mainAdapter);
    }
    catch(Ice::Exception& ex)
    {
        glog(ZQ::common::Log::L_ERROR, CLOGFMT(GBVSSService, "Create adapter failed with endpoint=%s and exception is %s"),
            pGBVSSBaseConfig->_bind.endPoint.c_str(), ex.ice_name().c_str());
        return false;
    }
/* CS
    //
    // init contentstore
    //
#ifdef ZQ_OS_MSWIN
    mNGODCSEnv.setServiceName(servname);
#else
    mNGODCSEnv.setServiceName(getServiceName().c_str());
#endif
    mNGODCSEnv.setLogPath(getLoggerFolder( ).c_str());
    mNGODCSEnv.setDataPath(gGBVSSConfig._dataBase.path.c_str());
    mNGODCSEnv.setConfig(pGBVSSBaseConfig);	
    mNGODCSEnv.setThreadPool(gPool);
    mNGODCSEnv.setIceAdapter(mainAdapter);
    if (!mNGODCSEnv.initEnv())
    {
        glog(ZQ::common::Log::L_ERROR, CLOGFMT(GBVSSService, "Failed to initialize ContentStore"));
        return false;
    }
    */
    glog(ZQ::common::Log::L_INFO, CLOGFMT(GBVSSService, "ContentStore initialized successfully"));

    //
    // init streamservice
    //
//    mpGBVSSEnv.mainAdapter		    = mainAdapter;
//    mpGBVSSEnv.mainLogger			= *m_pReporter;
//    mpGBVSSEnv.sessLogger			= mSessionLogger;
    _pGBVSSEnv->iceCommunicator	= mIc;
	_pGBVSSEnv->mainAdapter	= mainAdapter;

    try
    {
        _pGBVSSEnv->init();
    }
    catch( const Ice::Exception& ex )
    {
        glog(ZQ::common::Log::L_ERROR, CLOGFMT(GBVSSService, "[%s] caught when initializeServiceParameter"), ex.ice_name().c_str());
        return false;
    }
    catch(...)
    {
        glog(ZQ::common::Log::L_ERROR, CLOGFMT(GBVSSService, "unexpected exception caught when initializeServiceParameter"));
        return false;
    }

    glog(ZQ::common::Log::L_INFO, CLOGFMT(GBVSSService, "Stream Service initialized successfully"));

    return true;
}

void GBVSSService::UninitializeIceRunTime()
{
    try
    {
       _pGBVSSEnv->iceCommunicator = NULL;
        if(mIc)
        {
            mIc->destroy();
            mIc = NULL;
        }
    }
    catch(...)	{}
}


GBVSSService::GBVSSService()
: _pGBVSSEnv(NULL)
{
#ifdef ZQ_OS_MSWIN
    strcpy(servname, "GBVSS");
    strcpy(prodname, "TianShan");
#endif
    mIc				=	NULL;
    mpServiceImpl	=	NULL;
}

GBVSSService::~GBVSSService()
{
}

HRESULT GBVSSService::OnInit( )
{
//    ZQ::common::RTSPSession::startWatchdog();
//    ZQ::StreamService::gSsEnvironment	=	&mpGBVSSEnv;

    //get regedit argument
    std::string strNetId = "";
    if(m_argc == 1 && strlen(m_argv[0]))
    {
        strNetId = m_argv[0];
    }	

//    mpGBVSSEnv.setNetId(strNetId);
//    glog(ZQ::common::Log::L_INFO, CLOGFMT(GBVSSService, "netID='%s'"), strNetId.c_str());

    for (::ZQTianShan::GBVSS::GBVSSBaseConfig::GBVSSHolderVec::iterator iter = gGBVSSConfig._GBVSSBaseConfig.GBVSSVec.begin(); iter != gGBVSSConfig._GBVSSBaseConfig.GBVSSVec.end(); iter++)
    {
        if (strNetId == (*iter).netId)
        {
            glog(ZQ::common::Log::L_INFO, CLOGFMT(GBVSSService, "find NetID=%s"), strNetId.c_str());
            pGBVSSBaseConfig = &(*iter);
        }
    }
    if (NULL == pGBVSSBaseConfig)
    {
        glog(ZQ::common::Log::L_ERROR, CLOGFMT(GBVSSService, "couldn't find specified netID(%s)"), strNetId.c_str());
        return S_FALSE;
    }

    gPool = new ZQ::common::NativeThreadPool();
    assert( gPool != NULL );
    gPool->resize(pGBVSSBaseConfig->_bind.threadPoolSize);

	_thrdPoolRTSP.resize(max(pGBVSSBaseConfig->_bind.threadPoolSize/4, 20));

    // mpGBVSSEnv.mainThreadPool	= *gPool;

    if( !initializeCrashDumpLocation() )
    {
#ifdef ZQ_OS_MSWIN
        logEvent(ZQ::common::Log::L_ERROR,_T("failed to initialize crash dump location"));
#endif
        return S_FALSE;
    }

    if(!initializeLogger())
    {
#ifdef ZQ_OS_MSWIN
        logEvent( ZQ::common::Log::L_ERROR,_T("failed to open log"));
#endif
        return S_FALSE;
    }
    //initialize ice run time
    if( !InitializeIceRunTime() )
    {
#ifdef ZQ_OS_MSWIN
        logEvent(ZQ::common::Log::L_ERROR,_T("failed to initialize ice run time"));
#endif
        return S_FALSE;
    }

	_pGBVSSEnv = new ZQ::StreamService::GBVSSEnv(*m_pReporter, mSessionLogger,_thrdPoolRTSP, *gPool);
    assert(_pGBVSSEnv != NULL );
    ZQ::StreamService::gSsEnvironment	=	_pGBVSSEnv;
    _pGBVSSEnv->setNetId(strNetId);
    glog(ZQ::common::Log::L_INFO, CLOGFMT(GBVSSService, "netID='%s'"), strNetId.c_str());

	if(!initializeServiceParameter())
    {
#ifdef ZQ_OS_MSWIN
        logEvent(ZQ::common::Log::L_ERROR,_T("failed to initialize service parameter"));
#endif
        return S_FALSE;
    }

	mpServiceImpl = new ZQ::StreamService::SsServiceImpl(_pGBVSSEnv ,"GBVSS" );
    ZQ::StreamService::pServiceInstance	= mpServiceImpl;

    return ZQ::common::BaseZQServiceApplication::OnInit();
}

HRESULT GBVSSService::OnStart()
{
    if(!mpServiceImpl->start( gGBVSSConfig._dataBase.path , gGBVSSConfig._iceStorm.endPoint,
        "", "",
        "GBVSS"))
    {
        glog(ZQ::common::Log::L_EMERG,CLOGFMT(GBVSSService,"failed to start service instance"));
        return -1;
    }

//    mpGBVSSEnv.start();
//    mpGBVSSEnv.getMainAdapter()->activate();

	ZQ::common::RTSPSession::startDefaultSessionManager();

	_pGBVSSEnv->start();
    _pGBVSSEnv->getMainAdapter()->activate();

    return ZQ::common::BaseZQServiceApplication::OnStart();
}

HRESULT GBVSSService::OnStop()
{
    if( mpServiceImpl )
    {
        mpServiceImpl->stop();
        //should I delete service instance or just leave it to Ice runtime ?
        mpServiceImpl = NULL;

        glog(ZQ::common::Log::L_INFO, CLOGFMT(GBVSSService,"StreamService uninitialized"));
    }
/* CS
    mNGODCSEnv.uninitEnv();
    glog(ZQ::common::Log::L_INFO, CLOGFMT(GBVSSService,"ContentStore uninitialized"));
*/

    if( _pGBVSSEnv->mainAdapter )
        _pGBVSSEnv->mainAdapter->deactivate();	
    glog(ZQ::common::Log::L_INFO, CLOGFMT(GBVSSService,"Adapter deactivated"));

//    mpGBVSSEnv.mainThreadPool.stop();
    glog(ZQ::common::Log::L_INFO, CLOGFMT(GBVSSService,"ThreadPool stopped"));

    return ZQ::common::BaseZQServiceApplication::OnStop();
}

HRESULT GBVSSService::OnUnInit()
{
    UninitializeIceRunTime();
    glog(ZQ::common::Log::L_INFO, CLOGFMT(GBVSSService, "Ice runtime destroyed"));

    if (_pGBVSSEnv)
	{
		_pGBVSSEnv->uninit();
		delete _pGBVSSEnv;
		_pGBVSSEnv = NULL;
	}

    glog(ZQ::common::Log::L_INFO, CLOGFMT(GBVSSService, "GBVSS enviroment uninitialized"));

//    if( mpGBVSSEnv.mainThreadPool.stop )
//    {		
//        delete mpGBVSSEnv.mainThreadPool;
//        mpGBVSSEnv.mainThreadPool = NULL;
//        glog(ZQ::common::Log::L_INFO, CLOGFMT(GBVSSService,"ThreadPool uninitialized"));
//    }

    uninitializeLogger();
    //ZQ::common::RTSPSession::stopWatchdog();
    return ZQ::common::BaseZQServiceApplication::OnUnInit();
}

