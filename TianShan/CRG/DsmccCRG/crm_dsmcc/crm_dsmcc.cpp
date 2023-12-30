#include "crm_dsmcc.h"
#include "clientrequest.h"
#include "crmDmsccCfg.h"
#include "EventSink.h"
#include <IceLog.h>

#define MODULE_NAME "crm_dsmcc" 
ZQ::common::Log* dsmcclog = NULL;

//ZQ::common::Log* ZQ::common::pGlog = NULL;

#define SELFLOG (mMainLogger)

ZQ::common::Config::Loader< CRMDmsccCfg >_CRMDmsccConfig("");
//
RequestHandler::RequestHandler(DSMCC_Environment& env , RequestPtr req, TianShanIce::ClientRequest::SessionPrx sess, const char* methodName)
: mEnv(env), _clientSession(sess), _req(req), _connId(0), _svrSess(NULL), _stream(NULL)
{
	//hlog(ZQ::common::Log::L_INFO, HLOGFMT("basic class RequestHandler construct")) ;
	hlog(ZQ::common::Log::L_INFO,CLOGFMT(RequestHandler,"basic class RequestHandler construct"));
	_startTime = ZQTianShan::now() ;
	if (NULL != methodName)
		_method = methodName;
	try {
		if (_req)
			_connId = _req->getConnectionId();

		if (_clientSession)
			_clientSessId = _clientSession->getSessId();
	}
	catch(...) {}
}

RequestHandler::~RequestHandler()
{
	_usedTime = ZQTianShan::now() - _startTime ;
	hlog(ZQ::common::Log::L_INFO,CLOGFMT(RequestHandler,"request processed (%lldms)"), _usedTime) ;
}
//
TianShanIce::Streamer::StreamPrx RequestHandler::openAttachedStream()
{
	if (_stream)
		return _stream;

	try {
		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("openAttachedStream()"));
		if (!_clientSession)
		{
			// read the session id from _req _sessionID
			lsc::StringMap metadatas = _req->getMessage()->getProperties() ; 
			ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_LscStreamHandle,"0",_streamHandle);
			uint32  tpSessionID;
			sscanf(_streamHandle.c_str(),"%u",&tpSessionID) ;
			std::string strSession1 = mEnv.mGateway->streamHandle2SessionId(tpSessionID);
			//open a gateway session by session id	
			_clientSession = mEnv.mGateway->openSession(strSession1) ;
			// _clientSession = call gw to opensession(sessid) 
		}
		
		_stream = _clientSession->getStreamSession();
		if (_stream)
			_streamId = _stream->getIdent().name;
	}
	catch (TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("openAttachedStream() failed, excp[%s] %s(%d): %s"),
			ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
	}
	catch (Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("openAttachedStream() failed, excp[%s]"),
			ex.ice_name().c_str());
	}
	catch(...)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("openAttachedStream() failed, caught exception"));
	}

	return _stream;
}

TianShanIce::SRM::SessionPrx RequestHandler::openAttachedServerSession()
{
	if (_svrSess )
		return _svrSess;

	try {
		hlog(ZQ::common::Log::L_DEBUG, HLOGFMT("openAttachedStream()"));

	/*	lsc::StringMap metadatas = _req->getMessage()->getProperties() ; 
		ZQTianShan::Util::getPropertyDataWithDefault(metadatas,CRMetaData_LscStreamHandle,"",_streamHandle);			
		uint32  tpSessionID;
		sscanf(_streamHandle.c_str(),"%u",&tpSessionID) ;
		std::string strSession1 = mEnv.mGateway->streamHandle2SessionId(tpSessionID);
		_clientSession = mEnv.mGateway->openSession(strSession1) ;*/

		_svrSess = _clientSession->getWeiwooSession();
		if (_svrSess)
			_svrSessId = _svrSess->getId();
	}
	catch (TianShanIce::BaseException& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("openAttachedServerSession() failed, excp[%s] %s(%d): %s"),
			ex.ice_name().c_str(), ex.category.c_str(), ex.errorCode, ex.message.c_str());
	}
	catch (Ice::Exception& ex)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("openAttachedServerSession() failed, excp[%s]"),
			ex.ice_name().c_str());
	}
	catch(...)
	{
		hlog(ZQ::common::Log::L_ERROR, HLOGFMT("openAttachedServerSession() failed, caught exception"));
	}

	return _svrSess;
}

/*
	MAPSET(Properties, loc_resposedata, CRMetaData_LscOpCode,        "129"); //0x81

	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseCurrentNpt,  "6720");//x01a 0x40
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseNumerator,   "1");
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseDenominator, "1");
	MAPSET(Properties, loc_resposedata, CRMetaData_ResponseMode,        "1");//?
*/
void RequestHandler::postResponse(GATEWAYCOMMAND cmd, uint32 resultCode)
{
	std::string strTempData;
	std::map<std::string,std::string>::iterator itorMd;
	for (itorMd = loc_resposedata.begin(); itorMd != loc_resposedata.end(); itorMd++)
		strTempData+= itorMd->first.substr(4) + "[" + itorMd->second+ "] ";

	_req->getResponse()->getMessage()->setCommand(cmd);
	_req->getResponse()->getMessage()->setProperties(loc_resposedata);

	//response.
	if (!_req->getResponse()->complete(resultCode))
	{
		hlog(ZQ::common::Log::L_WARNING, HLOGFMT("%s::postResponse() failed to send response(%d): %s"), _method.c_str(), resultCode, strTempData.c_str());
		return;
	}

	hlog(ZQ::common::Log::L_INFO, HLOGFMT("%s::postResponse() sent response(%d), took %dms: %s"), _method.c_str(), resultCode, (int)(ZQ::common::now() - _startTime), strTempData.c_str());
}

//
DSMCC_Environment::DSMCC_Environment()
:mLastEventActiveTime(0)
,mStreamEventReceiver(*this),
mStreamingInfoCache(*this)
{
	bQuited = false;
	_sysLog.open("dsmccProxy", ZQ::common::Log::L_WARNING);
	
}
#ifdef ZQ_OS_MSWIN
void DSMCC_Environment::reportvents(LPTSTR pszSrcName,DWORD dwEventID,WORD wCategory, WORD cInserts, LPCTSTR *szMsg)
{
	HANDLE hEvent; 
	hEvent = RegisterEventSource(NULL, pszSrcName);           
	if (NULL == hEvent) 
	{
		printf("Could not register the event source."); 
		return;
	}
	if (!::ReportEvent(hEvent,EVENTLOG_ERROR_TYPE,wCategory,dwEventID,NULL,cInserts,0,szMsg,NULL))	
	{
		printf("Could not report the event."); 
	}
	
	DeregisterEventSource(hEvent); 
	return;
}
#endif
void  DSMCC_Environment::setErroMsg( const char* fmt , ... )
{
#ifdef ZQ_OS_MSWIN
	char szLocalBuffer[1024];
	va_list args;
	va_start(args, fmt);
	int nCount = _vsnprintf( szLocalBuffer, sizeof(szLocalBuffer)-1, fmt, args );
	va_end(args);
	if(nCount == -1)
	{
		szLocalBuffer[ sizeof(szLocalBuffer) - 1 ] = 0;
	}
	else
	{
		szLocalBuffer[nCount] = '\0';
	}

	mErrMsg = szLocalBuffer;

#endif
}
bool  DSMCC_Environment::initLogger(const char* logfolder)
{
	if( !( logfolder && logfolder[0] != 0 ) )
	{
		setErroMsg("null log folder passed in");
		return false;
	}

	try
	{
		std::string  path = ZQTianShan::Util::fsConcatPath(logfolder,"crm_dsmcc.log") ;
		//mMainLogger.open( path.c_str() ,ZQ::common::Log::L_DEBUG,10,50000000,8192);
		mMainLogger.open( path.c_str() ,_CRMDmsccConfig.logFileLevel,_CRMDmsccConfig.logFileCount,_CRMDmsccConfig.logFileSize,_CRMDmsccConfig.logFileBufferSize);
	}
	catch(const ZQ::common::FileLogException& ex)
	{
		setErroMsg("failed to open main log due to [%s]", ex.what() );
		return false;
	}
	
	try 
	{
		std::string path = ZQTianShan::Util::fsConcatPath(logfolder,"crm_dsmcc_events.log") ;
		mEventLogger.open( path.c_str() , _CRMDmsccConfig.logFileLevel ,_CRMDmsccConfig.logFileCount,_CRMDmsccConfig.logFileSize,_CRMDmsccConfig.logFileBufferSize );
	}
	catch(const ZQ::common::FileLogException& ex)
	{
		setErroMsg("failed to open events log due to [%s]",ex.what() );
		return false;
	}
	try
	{
		std::string  path =  ZQTianShan::Util::fsConcatPath(logfolder,"crm_dsmcc_icetrace.log");
		mIceLogger.open(path.c_str(),_CRMDmsccConfig.logFileLevel,_CRMDmsccConfig.logFileCount,_CRMDmsccConfig.logFileSize,_CRMDmsccConfig.logFileBufferSize) ;
	}
	catch(ZQ::common::FileLogException& ex)
	{
		setErroMsg("failed to open ice log due to [%s]",ex.what() );
		return false;
	}

	return true ;

}
#define STRVALID(x) (x && x[0] != 0 )
bool DSMCC_Environment::loadConfig( const char* confPath )
{
	if (!STRVALID(confPath))
	{
		SELFLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"null configutation path passed in ")) ;
		return false ;
	}
    
	//
	std::string dsmccConfPath = ZQTianShan::Util::fsConcatPath(confPath,"crm_Dsmcc.xml") ;

	//to load configuration-file 
	if(!_CRMDmsccConfig.load(dsmccConfPath.c_str()))
	{
		SELFLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"failed to load configuration from [%s]"),dsmccConfPath.c_str() );
		return false;
	}
	
	//SELFLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"load configuration [%s] successfully."),dsmccConfPath.c_str() );
	//SELFLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"logfile info:logFileCount[%d],logFileSize[%ld],logFileBufferSize[%d]"),_CRMDmsccConfig.logFileCount,_CRMDmsccConfig.logFileSize,_CRMDmsccConfig.logFileBufferSize );
	return true ;
}

bool DSMCC_Environment::initEventSink( )
{
	bool bOk = false;

	mEventSink = new EventSink(*this);
	
 	bOk = mStreamEventReceiver.addEventHandler( TianShanIce::Streamer::StreamEventSinkPtr::dynamicCast(mEventSink)  );
 	if( !bOk )	
	{
		SELFLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"failed to initialize event sinker"));
		return false;
	}
	
	mPlaylistEventSink = new  PlaylistEventSinkI(*this);
	bOk = mStreamEventReceiver.addEventHandler( TianShanIce::Streamer::PlaylistEventSinkPtr::dynamicCast(mPlaylistEventSink)  );
	if( !bOk )	
	{
		SELFLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"failed to initialize playlist event sinker"));
		return false;
	}

	mPauseTimeoutEventSink = new PauseTimeoutEventSinkI(*this);
	bOk = mStreamEventReceiver.addEventHandler( TianShanIce::Events::GenericEventSinkPtr::dynamicCast(mPauseTimeoutEventSink) ,
		                                       TianShanIce::Events::TopicStreamPauseTimeoutEvent  );
	if( !bOk )	
	{
		SELFLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"failed to initialize PauseTimeout event sinker"));
		return false;
	}
	return true;
}

bool DSMCC_Environment::doInit(Gateway& gw, ZQADAPTER_DECLTYPE objAdapter, const char* configpath, const char* loggerpath)
{
	//load configuration 
	if (!loadConfig(configpath) )
	{
#ifdef ZQ_OS_MSWIN
		LPCTSTR lpszError[] = {_T(" loading failed,please check the fields,properties of CRM_Dsmcc.xml ,and the CRM_DSMCC.dll can't run.")};
		reportvents(_T("CRM_Dsmcc.xml "),0,0,1,lpszError);
#endif
		return false;
	}

	if (!initLogger(loggerpath))
	{
#ifdef ZQ_OS_MSWIN
		LPCTSTR lpszError[] = {_T(" initializing failed,the CRM_DSMCC.dll can't run now. please check  codes of function initLogger")};
		reportvents(_T("crm_dsmcc.log "),0,0,1,lpszError);
#endif
		return false ;
	}
	SELFLOG(ZQ::common::Log::L_INFO,CLOGFMT(DSMCC_Environment,"======================CRM_DSMCC  service is starting====================")) ;
	

	try 
	{
		//	_communicator = objAdapter->getCommunicator();
		Ice::InitializationData		iceInitData;
		int i = 0;
		iceInitData.properties =Ice::createProperties( i , NULL );
		_communicator =	Ice::initialize( i , NULL , iceInitData );
		iceInitData.logger = new TianShanIce::common::IceLogI( &mIceLogger );
		assert( iceInitData.logger );

		//	_objAdapter = objAdapter;
		if (!_communicator)
		{
			SELFLOG(ZQ::common::Log::L_ERROR, CLOGFMT(DSMCC_Environment, "Null communicator"));
			return false;
		}

		_objAdapter = _communicator->createObjectAdapterWithEndpoints("CRM_DSMCCEvent", _CRMDmsccConfig.ListenEventEndPoint);

		if (!_objAdapter)
		{
			SELFLOG(ZQ::common::Log::L_ERROR, CLOGFMT(DSMCC_Environment, "Null objAdapter"));
			return false;
		}
		_objAdapter->activate();
	}
	catch(Ice::Exception& ex) 
	{
		SELFLOG(ZQ::common::Log::L_ERROR, CLOGFMT(DSMCC_Environment, "create adapter for CRM_DSMCC caught exception: %s"), ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		SELFLOG(ZQ::common::Log::L_ERROR, CLOGFMT(DSMCC_Environment, "create adapter for CRM_DSMCC caught unkonwn exception"));
		return false;
	}

	if(!initEventSink())
		return false;
	int64 timeoutinterval = _CRMDmsccConfig.heartbeatInterval / 3;
	if( timeoutinterval > 300 * 1000 )
	{
		timeoutinterval = 300 * 1000;
	}
	mStreamingInfoCache.init( timeoutinterval );//timeout 10s
	//
	mGateway = &gw; 
	mStreamEventReceiver.start( _CRMDmsccConfig.EventChannelEndPoint );
	SELFLOG(ZQ::common::Log::L_INFO, CLOGFMT(DSMCC_Environment,"start event sinker, EventChannelEndpoint[%s]"),
		_CRMDmsccConfig.EventChannelEndPoint.c_str() );
	
	std::vector<float>& forwardSpeeds = _CRMDmsccConfig.forwardSpeeds;
	std::string strForwardSpeed = _CRMDmsccConfig._fixedSpeedSet.strForwardSpeeds;
	std::vector<std::string> temps;
	ZQ::common::stringHelper::SplitString(strForwardSpeed,temps," "," ","","");
	for( std::vector<std::string>::const_iterator it = temps.begin() ; it != temps.end() ; it ++ )
		forwardSpeeds.push_back((float)atof(it->c_str()));

	std::vector<float>& backwardSpeeds = _CRMDmsccConfig.backwardSpeeds;
	std::string strBackwardSpeed = _CRMDmsccConfig._fixedSpeedSet.strBackwardSpeeds;
	temps.clear();
	ZQ::common::stringHelper::SplitString(strBackwardSpeed,temps," "," ","","");
	for( std::vector<std::string>::const_iterator it = temps.begin() ; it != temps.end() ; it ++ )
		backwardSpeeds.push_back((float)atof(it->c_str()));

	std::vector<float>& inputFFs = _CRMDmsccConfig.inputFFs;
	std::string strInputFF = _CRMDmsccConfig._fixedSpeedSet.strInputFF;
	temps.clear();
	ZQ::common::stringHelper::SplitString(strInputFF,temps," "," ","","");
	for( std::vector<std::string>::const_iterator it = temps.begin() ; it != temps.end() ; it ++ )
		inputFFs.push_back((float)atof(it->c_str()));

	std::vector<float>& inputREWs = _CRMDmsccConfig.inputREWs;
	std::string strInputREW = _CRMDmsccConfig._fixedSpeedSet.strInputREW;
	temps.clear();
	ZQ::common::stringHelper::SplitString(strInputREW,temps," "," ","","");
	for( std::vector<std::string>::const_iterator it = temps.begin() ; it != temps.end() ; it ++ )
		inputREWs.push_back((float)atof(it->c_str()));

	return true ;
}

void DSMCC_Environment::doUninit()
{
	try
	{
		if(_objAdapter)
			_objAdapter->deactivate();
	}
	catch (...)
	{

	}

	mStreamEventReceiver.stop();
	mEventSink = NULL;
	mPlaylistEventSink = NULL;
	mPauseTimeoutEventSink = NULL;

	try
	{
		if(_communicator)
			_communicator->destroy();
	}
	catch (...)
	{

	}
	_objAdapter = NULL;
	_communicator=NULL ;
	SELFLOG(ZQ::common::Log::L_INFO,CLOGFMT(DSMCC_Environment,"crm_dsmcc service is stopped"));
	return ;
}

//
ProcessResult DSMCC_Environment::doFixupRequest( RequestPtr pReq )
{
	//mMainLogger(ZQ::common::Log::L_INFO,CLOGFMT(DSMCC_Environment,"doFixupRequest fun entry") ) ;
	if(NULL ==  pReq)
	{
		mMainLogger(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"doFixupRequest parameter pReq is null") ) ;
		throw "pReq is null !" ;
	}
    
	RequestHandler::Ptr pRequestHandler = NULL; 
	// step 1 read the request type from request
	switch( pReq->getMessage()->getCommand() )
	{
	case   COMMAND_NULL:  
		break;
	case   COMMAND_SETUP:
		pRequestHandler = new  clientses_SetupRHandler(*this,pReq,NULL) ;
		break;
	case   COMMAND_DESTROY:
		pRequestHandler = new  clientses_ReleaseRHandler(*this,pReq,NULL);
		break; 
	case   COMMAND_RESUME:
	case   COMMAND_PLAY:
		pRequestHandler = new  lscp_PlayHandler(*this ,pReq,NULL) ;
		break;
	case   COMMAND_PAUSE:
		pRequestHandler = new  lscp_PauseHandler(*this,pReq,NULL);
		break;
	case   COMMAND_STATUS:
		pRequestHandler = new lscp_StatusHandler(*this,pReq,NULL) ;
		break;
	case   COMMAND_SESS_IN_PROGRESS:
		pRequestHandler = new clientses_InProgressRHandler(*this,pReq,NULL) ;
		break;						
	case   COMMAND_RELEASE_RESPONSE:
		pRequestHandler = new clientses_ReleaseResponseHandler(*this,pReq,NULL); 
		break;
	case   COMMAND_MAXCOMMAND:
		break;
	default:
		break;
	}

	if (NULL == pRequestHandler)
	{
		mMainLogger(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"doFixupRequest locvar pRequestHandler is null") ) ;
		return RESULT_DONE ;
	}
    
   //	mMainLogger(ZQ::common::Log::L_INFO,CLOGFMT(DSMCC_Environment,"doFixupRequest fun exited") ) ;
	pRequestHandler ->doFixupRequest();
    
	return  RESULT_PROCESSED;
}

ProcessResult DSMCC_Environment::doContentHandler( RequestPtr pReq, TianShanIce::ClientRequest::SessionPrx sess )
{
	//mMainLogger(ZQ::common::Log::L_INFO,CLOGFMT(DSMCC_Environment,"doContentHandler fun entry") ) ;
	RequestHandler::Ptr pRequestHandler = NULL;
	
	// step 1 read the request type from request
	switch( pReq->getMessage()->getCommand() )
	{
	case  COMMAND_NULL:
		break; ;
	case COMMAND_SETUP : 
		pRequestHandler = new clientses_SetupRHandler(*this,pReq, sess); // refer to RTSP SETUP
		break;

	case COMMAND_DESTROY:
		pRequestHandler = new clientses_ReleaseRHandler(*this, pReq, sess); // refer to RTSP TEARDOWN
		break;

	case COMMAND_SESS_IN_PROGRESS:
		pRequestHandler = new clientses_InProgressRHandler(*this, pReq, sess); // refer to RTSP PING
		break;

	case COMMAND_RELEASE_RESPONSE:
		pRequestHandler =  new  clientses_ReleaseResponseHandler(*this, pReq, sess) ;
		break;

		// about LSCP

	case COMMAND_PLAY://LSC_PLAY    LSC_RESUME
	case COMMAND_RESUME:
		pRequestHandler = new  lscp_PlayHandler(*this ,pReq,sess) ;
		break;
	case COMMAND_PAUSE: //LSC_PAUSE
		pRequestHandler = new  lscp_PauseHandler(*this,pReq,sess);
		break;

	case COMMAND_STATUS: //LSC_STATUS
		pRequestHandler = new lscp_StatusHandler(*this,pReq,sess) ;
		break;
	default:
		break;
	}

	if (NULL == pRequestHandler)
	{
		mMainLogger(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"doContentHandler locvar pRequestHandler is null") ) ;
		return RESULT_PROCESSED ;
	}

	pRequestHandler ->doContentHandler();
	//mMainLogger(ZQ::common::Log::L_INFO,CLOGFMT(DSMCC_Environment,"doContentHandler fun exited") ) ;
	return  RESULT_PROCESSED;

}

ProcessResult DSMCC_Environment::doFixupResponse( RequestPtr pReq, TianShanIce::ClientRequest::SessionPrx sess )
{
	//mMainLogger(ZQ::common::Log::L_INFO,CLOGFMT(DSMCC_Environment,"doFixupResponse fun entry ") ) ;
	RequestHandler::Ptr pRequestHandler = NULL;

	// step 1 read the request type from request
	switch( pReq->getMessage()->getCommand() )
	{
	case  COMMAND_NULL:
	break; ;

	// about DSM-CC
	case COMMAND_SETUP : 
		pRequestHandler = new clientses_SetupRHandler(*this,pReq, sess); // refer to RTSP SETUP
		break;
	case COMMAND_DESTROY:
		pRequestHandler = new clientses_ReleaseRHandler(*this, pReq, sess); // refer to RTSP TEARDOWN
		break;
	case COMMAND_SESS_IN_PROGRESS:
		pRequestHandler = new clientses_InProgressRHandler(*this, pReq, sess); // refer to RTSP PING
		break;
	case COMMAND_RELEASE_RESPONSE:
	    pRequestHandler =  new  clientses_ReleaseResponseHandler(*this, pReq, sess) ;
		break;

	// about LSCP
	case COMMAND_RESUME:
	case COMMAND_PLAY://LSC_PLAY    LSC_RESUME
		pRequestHandler = new  lscp_PlayHandler(*this ,pReq,NULL) ;
		break;
	case COMMAND_PAUSE: //LSC_PAUSE
	   pRequestHandler = new  lscp_PauseHandler(*this,pReq,NULL);
		break;
	case COMMAND_STATUS: //LSC_STATUS
		pRequestHandler = new lscp_StatusHandler(*this,pReq,NULL) ;
		break;
	default:
	break;
	}

	if (NULL == pRequestHandler)
	{
		mMainLogger(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"doFixupResponse locvar pRequestHandler is null") ) ;
		return RESULT_PROCESSED ;
	}

	pRequestHandler->doFixupRespone() ;
   //	mMainLogger(ZQ::common::Log::L_INFO,CLOGFMT(DSMCC_Environment,"doFixupResponse fun exited ") ) ;
	return  RESULT_PROCESSED;

}

void DSMCC_Environment::doExpired(TianShanIce::ClientRequest::SessionPrx sess)
{
	if (!sess)
		return;
	std::string sessId = sess->getSessId();
	SELFLOG(ZQ::common::Log::L_INFO,CLOGFMT(DSMCC_Environment,"doExpired() session[%s] timer expired, trying to destroy server session and client session"), sessId.c_str() );
	if( !mStreamingInfoCache.isSessionExpired(sessId))
	{
		SELFLOG(ZQ::common::Log::L_INFO,CLOGFMT(DSMCC_Environment,"doExpired() session[%s] is not expired, update timer"), sessId.c_str() );
		sess->updateTimer(_CRMDmsccConfig.heartbeatInterval);
		return;
	}
	//destroy weiwoo session
	bool weiwooSessDestroyed = false;
	TianShanIce::SRM::SessionPrx weiwooSess = 0;
	try
	{
		weiwooSess = sess->getWeiwooSession();
		if(weiwooSess)
			weiwooSess->destroy();
		weiwooSessDestroyed = true;
	}
	catch( const TianShanIce::BaseException& ex)
	{
		SELFLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"doExpired() session[%s] caught [%s] while destroy weiwoo session[%s]"),
			sessId.c_str(), ex.message.c_str(),_communicator->proxyToString(weiwooSess).c_str());
	}
	catch( const Ice::ObjectNotExistException& ex)
	{
		SELFLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"doExpired() session[%s] caught [%s] while destroy weiwoo session[%s]"),
			sessId.c_str(), ex.ice_name().c_str(),_communicator->proxyToString(weiwooSess).c_str());
	}
	catch( const Ice::Exception& ex)
	{
		SELFLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"doExpired() session[%s] caught [%s] while destroy weiwoo session[%s]"),
			sessId.c_str(), ex.ice_name().c_str(),_communicator->proxyToString(weiwooSess).c_str());
	}
	catch( const std::exception& ex)
	{
		SELFLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"doExpired() session[%s] caught [%s] while destroy weiwoo session[%s]"),
			sessId.c_str(), ex.what(),_communicator->proxyToString(weiwooSess).c_str());
	}
	catch( ... )
	{
		SELFLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"doExpired() session[%s] caught [%s] while destroy weiwoo session[%s]"),
			sessId.c_str(), "unknown exception",_communicator->proxyToString(weiwooSess).c_str());
	}

// 	if(!weiwooSessDestroyed)
// 	{
// 		Ice::Long interval = 20 * 1000;
// 		try
// 		{
// 			sess->updateTimer();
// 		}
// 		catch( const Ice::Exception&){}
// 		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"doExpired() failed to destroy weiwoo session , wait another [%lld] ms"),interval);
// 		return;
// 	}
	
	if(mEventSink)
	{
		mEventSink->issueCSRI(sess, 27);
	}

	try
	{
		mStreamingInfoCache.deleteInfo(sessId);
		sess->destroy();
	}
	catch( const Ice::Exception& ex)
	{
		SELFLOG(ZQ::common::Log::L_ERROR,CLOGFMT(DSMCC_Environment,"doExpired() caught [%s] while destroying client session[s]"),
			ex.ice_name().c_str() , sessId.c_str() );
	}
	SELFLOG(ZQ::common::Log::L_INFO,CLOGFMT(DSMCC_Environment,"doExpired() client session [%s] expired, and be destroyed now"),
		sessId.c_str());
}
void  DSMCC_Environment::sendCSIR(const ::std::string& playListId, int reasonCode)
{

	if (NULL == mGateway)
		return;

	std::string clientSessId = mGateway->streamSessId2ClientSessionId(playListId);
	TianShanIce::ClientRequest::SessionPrx clientSession = mGateway->openSession(clientSessId);

	if (!clientSession)
	{
		SELFLOG(ZQ::common::Log::L_WARNING, CLOGFMT(DSMCC_Environment, "sendCSIR(%s) unkown stream, ignore"), playListId.c_str());
		return;
	}
	if(mEventSink)
	{
		mEventSink->issueCSRI(clientSession, reasonCode);
	}
}
void DSMCC_Environment::updateLastEventRecvTime(int64 t)
{
	mLastEventActiveTime = t;
}
int64 DSMCC_Environment::getLastEventRecvTime() const
{
	return mLastEventActiveTime;
}

//////////////////////////////////////////////////////////////////////////
//SessionInfoCache
SessionInfoCache::SessionInfoCache( DSMCC_Environment& env )
:mEnv(env),
mNodes(0)
{
}
SessionInfoCache::~SessionInfoCache()
{
	if (mNodes)
	{
		delete[] mNodes;
		mNodes = 0;
	}
}

bool SessionInfoCache::init(int64 timeout  ,int nodecount )
{
	mExpireInterval = timeout;
	nodecount = MIN(4,nodecount);
	nodecount = MAX(128,nodecount);
	mNodeCount = nodecount;
	if (mNodes)
	{
		delete[] mNodes;
		mNodes = 0;
	}
	mNodes = new CacheMapNode[nodecount];
	return true;
}

size_t SessionInfoCache::sessionId2Index( const std::string& sessId )
{
	int value = 0 ;
	size_t length = sessId.length();
	length = MIN(length,5);
	for( int i = 0 ; i < length; i ++ )
	{
		value += sessId.at(i*2+1);
	}
	return value%mNodeCount;
}


void SessionInfoCache::updateInfo( const std::string& sessId, int64 npt, float speed)
{
	CacheMapNode& cacheNode = mNodes[sessionId2Index(sessId)];
	{
		ZQ::common::MutexGuard gd(cacheNode.locker);
		SESSIONINFOCACHEMAP::iterator it = cacheNode.cacheMap.find(sessId);
		if( it == cacheNode.cacheMap.end())
		{
			SessionStreamingInfo info;
			info.npt = npt;
			info.speed = speed;
			info.lasttouch = ZQ::common::now();
			info.lastupdate = info.lasttouch;
			cacheNode.cacheMap[sessId] = info;
		}
		else
		{
			SessionStreamingInfo& info = it->second;
			info.npt = npt;
			info.speed = speed;
			info.paused = false;
			info.lasttouch = ZQ::common::now();			
			info.lastupdate = info.lasttouch;
		}
	}
	hlog(ZQ::common::Log::L_DEBUG,CLOGFMT(SessionInfoCache,"updateInfo, set sess[%s] npt[%lld] speed[%f]"),
		sessId.c_str(), npt, speed );
}

void SessionInfoCache::updateInfo( const std::string& sessId, bool paused )
{
	int64 npt  = 0;
	float speed = 0.0f;
	CacheMapNode& cacheNode = mNodes[sessionId2Index(sessId)];
	{
		ZQ::common::MutexGuard gd(cacheNode.locker);
		SESSIONINFOCACHEMAP::iterator it = cacheNode.cacheMap.find(sessId);
		if( it == cacheNode.cacheMap.end())
		{
			hlog(ZQ::common::Log::L_WARNING,CLOGFMT(SessionInfoCache,"updateInfo, no session is found for id[%s], paused"),sessId.c_str());
			return;
		}
		else
		{
			SessionStreamingInfo& info = it->second;
			
			info.lasttouch = info.lastupdate;
			if( !info.paused )
			{
				info.npt += (int64)( info.speed  * ( ZQ::common::now() - info.lastupdate ) );
				info.paused = paused;
			}
			npt = info.npt;
			speed = info.speed;
			info.lastupdate = ZQ::common::now();
		}
	}
	hlog(ZQ::common::Log::L_DEBUG,CLOGFMT(SessionInfoCache,"updateInfo, set sess[%s] to [%s], status : npt[%lld] speed[%f]"),
		sessId.c_str(), paused ? "paused":"streaming" , npt, speed );
}

bool SessionInfoCache::getInfo( const std::string& sessId, int64& npt, float& speed )
{
	CacheMapNode& cacheNode = mNodes[sessionId2Index(sessId)];
	bool paused = false;
	int64 timedelta = 0;
	
	{
		ZQ::common::MutexGuard gd(cacheNode.locker);
		SESSIONINFOCACHEMAP::iterator it = cacheNode.cacheMap.find(sessId);
		if( it == cacheNode.cacheMap.end())
		{
			return false;
		}
		else
		{
			SessionStreamingInfo& info = it->second;
			info.lasttouch = ZQ::common::now();
			timedelta = ZQ::common::now() - info.lastupdate;
			if( timedelta > mExpireInterval || info.lastupdate <= 0 )
				return false;
			npt = info.npt;
			speed = info.speed;
			paused = info.paused;			
		}
	}
	if(!paused)
	{
		npt += (int64)(speed *  timedelta);		
	}
	if(npt < 0 )
		npt = 0;
	hlog(ZQ::common::Log::L_DEBUG,CLOGFMT(SessionInfoCache,"getInfo, session[%s] npt[%lld] speed[%f] timedelta[%lld] "),
		sessId.c_str(), npt, speed, timedelta);
	return true;
}

bool SessionInfoCache::isSessionExpired( const std::string& sessId )
{
	CacheMapNode& cacheNode = mNodes[sessionId2Index(sessId)];
	int64 curtime = ZQ::common::now();
	int64 delta = 0;
	{
		
		ZQ::common::MutexGuard gd(cacheNode.locker);
		SESSIONINFOCACHEMAP::iterator it  = cacheNode.cacheMap.find(sessId);
		if( it == cacheNode.cacheMap.end())
		{
			return true;
		}
		SessionStreamingInfo& info = it->second;
		delta = curtime - info.lasttouch;
	}
	if( delta > _CRMDmsccConfig.heartbeatInterval )
		return true;
	else
		return false;
}

void SessionInfoCache::deleteInfo( const std::string& sessId )
{
	CacheMapNode& cacheNode = mNodes[sessionId2Index(sessId)];
	bool bDel = false;
	{
		ZQ::common::MutexGuard gd(cacheNode.locker);
		SESSIONINFOCACHEMAP::iterator it  = cacheNode.cacheMap.find(sessId);
		if( it != cacheNode.cacheMap.end())
		{
			bDel = true;
			cacheNode.cacheMap.erase(it);
		}
	}
	if(bDel)
	{
		hlog(ZQ::common::Log::L_DEBUG,CLOGFMT(SessionInfoCache,"deleteInfo, delete session[%s]'s cache information"),
			sessId.c_str() );
	}
}
