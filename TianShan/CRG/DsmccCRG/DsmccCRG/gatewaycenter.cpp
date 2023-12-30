
#include <ZQ_common_conf.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

#include <TianShanIceHelper.h>
#include <DataCommunicatorUnite.h>

#include "environment.h"
#include "gatewaycenter.h"
#include "requestimpl.h"
#include "clientrequestsession.h"
#include "gatewayconfig.h"


namespace ZQ { namespace CLIENTREQUEST{

extern Config::GateWayConfig gwConfig;

static std::string lastLoadedModuleName ;

class PhaseRunner : public ZQ::common::ThreadRequest
{
public:
	PhaseRunner( Environment& env, GatewayCenter& gw, ZQ::common::NativeThreadPool& pool, RequestPtr req , RequestProcessPhase phase )
		:ZQ::common::ThreadRequest(pool),
		mEnv(env),
		mGateway(gw),		
		mNextPhase(phase)
	{
		mReq = RequestImplPtr::dynamicCast(req);
		assert( mReq != 0 );
		mSessId = mReq->getSessionId();
	}
	virtual ~PhaseRunner(){}

protected:
	void final(int retcode /* =0 */, bool bCancelled /* =false */)
	{
		delete this;
	}
	int run();

	void fixupRequest( );
	void contentHandler( );
	void fixupResponse( );
	void initRequest( RequestImplPtr req );

	Environment&							mEnv;
	GatewayCenter&							mGateway;
	RequestImplPtr							mReq;
	RequestProcessPhase						mNextPhase;
	TianShanIce::ClientRequest::SessionPrx	mSession;
	std::string								mSessId;
};

void PhaseRunner::initRequest( RequestImplPtr req )
{
	req->setStartRunningTime( ZQ::common::now() );

	SessionDatabase& db = mGateway.getDatabase();
	switch( req->getMessage()->getCommand())
	{
	case COMMAND_STATUS:
		return;
	default:
		break;
	}

	ZQTianShan::Util::TimeSpan sp;sp.start();	
	if( !mSessId.empty() )
	{
		req->attachSession( db.openSession( mSessId ) );
	}

	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(PhaseRunner,"initRequest() open session [%s] in [%lld]ms"),mSessId.c_str(),sp.stop() );
}
int PhaseRunner::run()
{	
	while( mNextPhase != PHASE_NULL )
	{
		switch( mNextPhase )
		{
		case PHASE_FIXUP_REQUEST:
			{
				initRequest( mReq );
				fixupRequest();
			}
			break;
		case PHASE_CONTENT_HANDLER:
			contentHandler();
			break;
		case PHASE_FIXUP_RESPONSE:
			fixupResponse();
			break;
		default:
			return 0;
		}
	}
	return 0;
}

void PhaseRunner::fixupRequest( )
{
	mNextPhase = PHASE_CONTENT_HANDLER;
	const std::vector< GatewayCenter::FixupRequestInfo >& stack = mGateway.getFixupReqeustStack();
	std::vector< GatewayCenter::FixupRequestInfo >::const_iterator it = stack.begin();
	for( ; it != stack.end(); it ++ )
	{
		try
		{
			MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(PhaseRunner,"fixupRequest() session[%s] enter fixupRequest in [%s]"),
				mSessId.c_str(),it->name.c_str() );
			ProcessResult rc = (it->proc)( mReq );
			MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(PhaseRunner,"fixupRequest() session[%s] leave fixupRequest from [%s]"),
				mSessId.c_str(), it->name.c_str() );
			switch( rc )
			{				
			case RESULT_PHASE_DONE:
				mNextPhase = PHASE_CONTENT_HANDLER; //run to next phase
				return;
				break;
			case RESULT_DONE:
				mNextPhase = PHASE_NULL;
				return;
				break;
			case RESULT_PROCESSED:					
			default:
				//run to next proc in current phase
				break;
			}
		}
		catch( const std::exception& ex)
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(PhaseRunner,"fixupRequest() caught std exception [%s], session[%s], module[%s]"),
				ex.what(), mSessId.c_str(),it->name.c_str() );
		}
		catch( ... )
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(PhaseRunner,"fixupRequest() caught unknown exception, session[%s], module[%s]"),
				mSessId.c_str(), it->name.c_str() );
		}		
	}
}

void PhaseRunner::contentHandler( )
{
	mNextPhase = PHASE_FIXUP_RESPONSE;
	GatewayCenter::ContentHandlerInfo info = mGateway.getContentHandler( "" );
	if( !info.proc )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(PhaseRunner,"contentHandler() no appropriate handler available"));
		return;
	}
	try
	{
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(PhaseRunner,"contentHandler() session[%s] enter contentHandler in [%s]"),
			mSessId.c_str(), info.name.c_str() );
		ProcessResult rc = (info.proc)( mReq , mSession );
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(PhaseRunner,"contentHandler() session[%s] leave contentHandler from [%s]"),
			mSessId.c_str(), info.name.c_str() );
		switch( rc )
		{		
		case RESULT_DONE:
			mNextPhase = PHASE_NULL;			
			break;
		case RESULT_PHASE_DONE:
		case RESULT_PROCESSED:					
		default:
			mNextPhase = PHASE_FIXUP_RESPONSE;
			break;
		}
	}
	catch( const std::exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(PhaseRunner,"contentHandler() caught std exception [%s], session[%s], module[%s]"),
			ex.what(), mSessId.c_str(), info.name.c_str() );
	}
	catch( ... )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(PhaseRunner,"contentHandler() caught unknown exception, session[%s], module[%s]"),
			mSessId.c_str(), info.name.c_str() );
	}
}

void PhaseRunner::fixupResponse( )
{
	mNextPhase = PHASE_NULL;
	const std::vector< GatewayCenter::FixupResponseInfo >& stack = mGateway.getFixupResponseStack();
	std::vector< GatewayCenter::FixupResponseInfo >::const_iterator it = stack.begin();
	for( ; it != stack.end(); it ++ )
	{
		try
		{
			MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(PhaseRunner,"fixupResponse() session[%s] enter fixupResponse in [%s]"), 
				mSessId.c_str(), it->name.c_str() );
			ProcessResult rc = (it->proc)( mReq , mSession );
			MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(PhaseRunner,"fixupResponse() session[%s] leave fixupResponse from [%s]"), 
				mSessId.c_str(), it->name.c_str() );
			switch( rc )
			{				
			case RESULT_PHASE_DONE:				
			case RESULT_DONE:
				mNextPhase = PHASE_NULL;
				return;
				break;
			case RESULT_PROCESSED:					
			default:
				//run to next proc in current phase
				break;
			}
		}
		catch( const std::exception& ex)
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(PhaseRunner,"fixupResponse() caught std exception [%s], session[%s], module[%s]"),
				ex.what(), mSessId.c_str(), it->name.c_str() );
		}
		catch( ... )
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(PhaseRunner,"fixupResponse() caught unknown exception, session[%s] module[%s]"),
				mSessId.c_str(), it->name.c_str() );
		}		
	}
}

//////////////////////////////////////////////////////////////////////////
class TimerRunner : public ZQ::common::ThreadRequest
{
public:
	TimerRunner( Environment& env, GatewayCenter& center,ZQ::common::NativeThreadPool& pool, ONTIMER proc , const std::string& sessId )
		:ZQ::common::ThreadRequest(pool),
		mEnv(env),
		mCenter(center),
		mTimerProc(proc),
		mSessId(sessId)
	{
	}
	virtual ~TimerRunner(){}
protected:
	int			run(void);
	void		final(int retcode /* =0 */, bool bCancelled /* =false */)
	{
		delete this;
	}
private:
	Environment&	mEnv;
	GatewayCenter&	mCenter;
	ONTIMER			mTimerProc;
	std::string		mSessId;
};

int TimerRunner::run(void)
{
	if(!mTimerProc || mSessId.empty() )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(TimerRunner,"run() no OnTimer procedure or no sessionId, refuse to run it"));
		return 0;
	}
	try
	{
		TianShanIce::ClientRequest::SessionPrx sess = mCenter.getDatabase().openSession( mSessId );
		if(!sess)
		{
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(TimerRunner,"no session opened with id[%s]"), mSessId.c_str() );
			return 0;
		}
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(TimerRunner,"session[%s] on timer"), mSessId.c_str() );
		(mTimerProc)(sess);
	}
	catch( const std::exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(TimerRunner,"run() caught [%s] while run timer thread"),
			ex.what());
	}
	catch( ... )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(TimerRunner,"run() caught unknown exception while run timer thread") );
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
GatewayCenter::GatewayCenter(Environment& env)
:mEnv(env),
mbRunning(false),
mOnTimerProc(0),
mDb(env),
mDak(0),
mTimerWatchDog(env,*this),
mStreamHandleMap(env,*this)
{
}

GatewayCenter::~GatewayCenter()
{
}

void GatewayCenter::registerFixupRequestStage( FIXUPREQUEST proc )
{
	FixupRequestInfo info;
	info.proc	= proc;
	info.name	= lastLoadedModuleName;

	mFixupRequestStack.push_back( info );
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"registerFixupRequestStage() module[%s] register fixuprequest proc[%p]"),
		lastLoadedModuleName.c_str() , proc );
}

void GatewayCenter::registerContentHandlerStage( CONTENTHANDLER proc, const std::string& handlername )
{
	ContentHandlerInfo info;
	info.proc	= proc;
	info.name	= lastLoadedModuleName;

	if( gwConfig.requestHandler.defaultHandler == handlername )
	{
		mDefaultHandler = info;
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"registerContentHandlerStage() set default handler to [%p] with uri[%s] in module[%s]"),
			proc, handlername.c_str() , lastLoadedModuleName.c_str() );
	}
	const std::map<std::string,std::string>& handlerMap = gwConfig.requestHandler.handlermap;

	std::map<std::string,std::string>::const_iterator it = handlerMap.begin();
	bool bFind = false;
	for( ; it != handlerMap.end() ; it ++ )
	{
		if( it->second == handlername )
		{
			bFind = true;
			mHandlerMap[ it->first ] = info;
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"registerContentHandlerStage() set content handler map, proc [%p] path[%s] handler[%s] module[%s]"),
				proc, it->first.c_str() , handlername.c_str() , lastLoadedModuleName.c_str() );
		}
	}
	if(!bFind)
	{
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"registerContentHandlerStage() handler[%s] is not configured from module[%s]"),
			handlername.c_str() , lastLoadedModuleName.c_str() );
	}
}

void GatewayCenter::registerFixupResponseStage( FIXUPRESPONSE proc )
{
	FixupResponseInfo info;
	info.proc	= proc;
	info.name	= lastLoadedModuleName;

	mFixupResponseStack.push_back(info);
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"registerFixupResponseStage() module[%s] register fixupresponse proc[%p]"),
		lastLoadedModuleName.c_str() , proc );
}

void GatewayCenter::unregisterFixupRequestStage( FIXUPREQUEST proc )
{
	std::vector<FixupRequestInfo>::iterator it = mFixupRequestStack.begin();
	for( ; it != mFixupRequestStack.end() ; it ++ )
	{
		if( it->proc == proc )
		{
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"unregisterFixupRequestStage() fixuprequest proc[%p] removed"), proc );
			mFixupRequestStack.erase(it);
			return;
		}
	}
}

void GatewayCenter::unregisterContentHandlerStage( CONTENTHANDLER proc, const std::string& handler )
{
	if( proc == mDefaultHandler.proc )
		mDefaultHandler.proc = 0;

	std::vector<std::string> names;
	std::map<std::string,ContentHandlerInfo>::iterator it = mHandlerMap.begin();
	for( ; it != mHandlerMap.end() ; it ++ )
	{
		if( it->second.name == handler )
		{
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"content handler proc[%p] with path[%s], handler[%s] removed"),
				proc, it->first.c_str(), handler.c_str()  );
			names.push_back(it->first);
		}
	}
	std::vector<std::string>::const_iterator itName = names.begin();
	for( ; itName != names.end() ; itName++ )
	{
		mHandlerMap.erase(*itName);
	}
}

void GatewayCenter::unregisterFixupResponseStage( FIXUPRESPONSE proc )
{
	std::vector<FixupResponseInfo>::iterator it = mFixupResponseStack.begin();
	for( ; it != mFixupResponseStack.end() ; it ++ )
	{
		if( it->proc == proc )
		{
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"fixupresponse proc[%p] in module[%s] removed"), 
				proc , it->name.c_str() );
			mFixupResponseStack.erase(it);
			return;
		}
	}
}

void GatewayCenter::registerOntimerProc( ONTIMER proc )
{
	mOnTimerProc = proc;
}

TianShanIce::ClientRequest::SessionPrx GatewayCenter::createSession( const std::string& sessId , const std::string& clientId )
{
	ZQTianShan::Util::TimeSpan sp;sp.start();
	TianShanIce::ClientRequest::SessionPtr p = new ClientRequestSession(mEnv,*this,sessId,clientId);
	assert(p);
	if(!mDb.addSession(p))
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayCenter,"createSession() failed to create session with sessid[%s] clientId[%s], maybe this session already exist"),
			sessId.c_str() , clientId.c_str() );
		return false;
	}
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"createSession() created session[%s] clientId[%s], using [%lld]ms"),
		sessId.c_str() , clientId.c_str(), sp.stop() );
	return mDb.openSession(sessId);
}

TianShanIce::ClientRequest::SessionPrx GatewayCenter::openSession( const std::string& id )
{	
	TianShanIce::ClientRequest::SessionPrx sess = mDb.openSession(id);
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(GatewayCenter,"openSession() session %s be opened by id[%s]"),
		sess ? "" :"do not", id.c_str() );
	return sess;
}

std::vector<TianShanIce::ClientRequest::SessionPrx> GatewayCenter::findSessionsByClient( const std::string& clientId )
{
	return mDb.findSessionByClient( clientId );
}

void GatewayCenter::postRequest( RequestPtr req, RequestProcessPhase phase)
{
	if(!req)
		return;
	if(!mbRunning)
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(GatewayCenter,"postRequest() service is stopping, deny any request"));
		return;
	}
	
	if( phase <= PHASE_FIXUP_REQUEST && checkIfBusy(req) )
	{
		return;
	}

	RequestImplPtr cliReq = RequestImplPtr::dynamicCast(req);

	
	PhaseRunner *p = new PhaseRunner(mEnv,*this,mThreadPool,req,phase);
	p->start();

}

ServerRequestPtr GatewayCenter::createServerRequest( int64 connId, TianShanIce::ClientRequest::SessionPrx sess )
{
	ZQ::DataPostHouse::IDataCommunicatorPtr comm = mDialogFactory->findCommunicator(connId);
	if( !comm )
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(GatewayCenter,"createServerRequest() failed to create server request with connId[%lld]"),
			connId);
		return 0;
	}
	ServerRequestImplPtr pRequest = new ServerRequestImpl(mEnv,comm);
	if( comm->getCommunicatorType() == ZQ::DataPostHouse::COMM_TYPE_UDP )
	{
		if(sess)
		{
			try
			{
				std::string ip = sess->getProperty( SESS_PROP_REMOTE_IP );
				std::string port = sess->getProperty( SESS_PROP_REMOTE_PORT );
				pRequest->updatePeerInfo(ip,port);
			}
			catch(const Ice::Exception&  )
			{
				//TODO: record log message here
			}
		}
	}
	return pRequest;
}

void GatewayCenter::destroySession( const std::string& id )
{
	TianShanIce::ClientRequest::SessionPrx sess = mDb.openSession(id);
	if(!sess)
	{
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"destroySession() session[%s] does not exist"), id.c_str() );
		return;
	}
	try
	{
		sess->destroy();
	}
	catch( const Ice::ObjectNotExistException&)
	{
	}
	catch( const Ice::Exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayCenter,"destroySession() caught [%s] while destroy session [%s]"),
			ex.ice_name().c_str() , id.c_str());
	}
	catch( const std::exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayCenter,"destroySession() caught [%s] while destroy session [%s]"),
			ex.what() , id.c_str());
	}
	catch( ... )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayCenter,"destroySession() caught unknown exception while destroy session [%s]"),
			id.c_str());
	}
}

void GatewayCenter::updateTimer( const std::string& sessionId, int64 interval )
{
// 	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(GatewayCenter,"updateTimer() session[%s] update timer[%lld]"),
// 		sessionId.c_str(), interval );
	mTimerWatchDog.updateTimer( sessionId , interval );
}

uint32 GatewayCenter::sessionId2StreamHandle( const std::string& sessId ) const
{
	return mStreamHandleMap.findStreamHandle(sessId);
}

std::string GatewayCenter::streamSessId2ClientSessionId( const std::string& streamSessId ) const
{
	return mStreamHandleMap.findClientSessId(streamSessId);
}

std::string GatewayCenter::streamHandle2SessionId( uint32 handle ) const
{
	return mStreamHandleMap.findSessionForStrmHandle( handle );
}

SessionDatabase& GatewayCenter::getDatabase( )
{
	return mDb;
}

GatewayCenter::ContentHandlerInfo GatewayCenter::getContentHandler( const std::string& uri )
{
	std::map<std::string,ContentHandlerInfo>::iterator it = mHandlerMap.find(uri);
	if( it == mHandlerMap.end() )
		return mDefaultHandler;
	else
		return it->second;
}

const std::vector<GatewayCenter::FixupRequestInfo>& GatewayCenter::getFixupReqeustStack( ) const
{
	return mFixupRequestStack;
}

const std::vector<GatewayCenter::FixupResponseInfo>& GatewayCenter::getFixupResponseStack( ) const
{
	return mFixupResponseStack;
}

bool GatewayCenter::checkIfBusy( RequestPtr q )
{
	static  int totalThreadCount = mThreadPool.size();
	int runningThreadCount = mThreadPool.activeCount();
	int pendingCount = mThreadPool.pendingRequestSize();
	
	if( gwConfig.maxPendingRequest > 0 )
	{
		RequestImplPtr req = RequestImplPtr::dynamicCast(q);
		const std::string& sessId = req->getSessionId();

		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(GatewayCenter,"sess[%s], thread[%d/%d] pending[%d/%d]"),
			sessId.c_str(), runningThreadCount, totalThreadCount, pendingCount,gwConfig.maxPendingRequest);
		if( pendingCount >= gwConfig.maxPendingRequest )
		{	
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"sess[%s], thread[%d/%d] pending[%d] maxPending[%d], reject"),
				sessId.c_str(), runningThreadCount, totalThreadCount, pendingCount,gwConfig.maxPendingRequest);
			return true;
		}
	}
	return false;
}

bool GatewayCenter::start(  const char* loggerpath , const char* dbPath )
{
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(GatewayCenter,"start() trying to start gateway center"));
	int threadCount = MAX( gwConfig.processThreadCount, 3 );
	mThreadPool.resize( threadCount );
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"start() service thread count was resize to %d"),threadCount );
	
	mbRunning = true;

	mTimerWatchDog.start();

	mEnv.getIc()->addObjectFactory( new SessionFactory(mEnv,*this), TianShanIce::ClientRequest::Session::ice_staticId());
	
	if(!mDb.openDB(dbPath,true,mEnv.getAdapter()) )
		return false;

  	if(!loadPlugin(loggerpath))
  		return false;

	if(!setupSocketServer())
		return false;

	restoreDb();

	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(GatewayCenter,"start() gateway center started"));
	return true;
}

void GatewayCenter::stop()
{
	mbRunning = false;
	destroySocketServer();
	mTimerWatchDog.stop();
	mThreadPool.stop();
	unloadPlugin();
	mDb.closeDB();
}

void GatewayCenter::postTimerRequest( const std::string& sessId )
{
	TimerRunner* p = new TimerRunner(mEnv,*this,mThreadPool,mOnTimerProc,sessId);
	p->start();	
}

MemorySessionIndexMap& GatewayCenter::getStrmHandleMap( )
{
	return mStreamHandleMap;
}

void GatewayCenter::restoreDb( )
{
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(GatewayCenter,"restoreDb() trying to restore session from database"));
	
	std::vector<std::string> ids =  mDb.loadAllSessionIds();

	srand((unsigned int)time(NULL));

	std::vector<std::string>::const_iterator it = ids.begin();
	for( ; it != ids.end() ; it ++ )
	{
		try
		{
			TianShanIce::ClientRequest::SessionPrx sess = mDb.openSession(*it);
			if( sess )
			{
				sess->onRestore();
			}			
		}
		catch( const Ice::Exception& )
		{
		}		
	}	

	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(GatewayCenter,"restoreDb() sessions restored from db"));
}

class PluginObj : public ZQ::common::DynSharedFacet
{
	DECLARE_DSOFACET(PluginObj, DynSharedFacet);
	DECLARE_PROC(bool, MODULEINIT, (Gateway&, ZQADAPTER_DECLTYPE, const char*, const char*));
	DECLARE_PROC(bool, MODULEUNINIT, (Gateway&));

	DSOFACET_PROC_BEGIN();
		DSOFACET_PROC_SPECIAL(MODULEINIT,"ModuleInit");
		DSOFACET_PROC_SPECIAL(MODULEUNINIT,"ModuleUninit");
	DSOFACET_PROC_END();
};


bool GatewayCenter::loadPlugin( const char* loggerpath )
{
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(GatewayCenter,"loadPlugin() trying to load plugin for gateway"));
	//read plugin configuration from conf
	const std::vector<Config::RequestHandler::RequestPluginHolder>& plugins = gwConfig.requestHandler.plugins;
	std::vector<Config::RequestHandler::RequestPluginHolder>::const_iterator it = plugins.begin();
	for( ; it != plugins.end() ; it ++ )
	{
		if( it->plugingfile.empty())
			continue;

		MLOG(ZQ::common::Log::L_INFO,CLOGFMT( GatewayCenter,"loadPlugin() trying to load plugin [%s] with config folder[%s]"),
			it->plugingfile.c_str() , it->pluginconf.c_str() );
		try
		{
			ZQ::common::DynSharedObj* obj = new ZQ::common::DynSharedObj( it->plugingfile.c_str() );
			PluginObj p(*obj);

			if(!p.isValid())
			{
				MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(GatewayCenter,"loadPlugin() target module[%s] may not be a valid gateway plugin"),
					it->plugingfile.c_str());
				continue;
			}
			
			lastLoadedModuleName = it->plugingfile;
			ZQTianShan::Util::TimeSpan sp;sp.start();

			if( !p.MODULEINIT( *this, mEnv.getAdapter(), it->pluginconf.c_str() , loggerpath ) )
			{
				MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(GatewayCenter,"loadPlugin() failed to initialize module[%s] with configuration folder[%s]"),
					it->plugingfile.c_str() , it->pluginconf.c_str() );
				p.MODULEUNINIT(*this);
				delete obj;
			}
			else
			{
				MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"loadPlugin() load module [%s] with configuration folder[%s] successfully, using [%lld] milliseconds "),
					it->plugingfile.c_str() , it->pluginconf.c_str() , sp.stop() );
				
				PluginInfo info;
				info.name = it->plugingfile;
				info.plugin = obj;

				mPlugins.push_back( info );
			}
		}
		catch( std::exception& ex )
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayCenter,"loadPlugin() caught exception[%s] while loading plugin[%s] configuration folder[%s]"),
				ex.what(), it->plugingfile.c_str() , it->pluginconf.c_str() );
		}
		catch( ... )
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayCenter,"loadPlugin() caught unknown exception while loading plugin[%s] configuration folder[%s]"),
				it->plugingfile.c_str() , it->pluginconf.c_str() );
		}
	}
	if( mPlugins.size() >= 1)
	{
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(GatewayCenter,"loadPlugin() [%u] plugins loaded"), mPlugins.size());
		return true;
	}
	else
	{
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(GatewayCenter,"loadPlugin() no plugins loaded"));
		return false;
	}
}

void GatewayCenter::unloadPlugin( )
{
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(GatewayCenter,"unloadPlugin() trying to load plugin for gateway"));
	ZQTianShan::Util::TimeSpan sp;sp.start();
	std::vector<PluginInfo>::iterator it = mPlugins.begin();
	for( ; it != mPlugins.end() ; it ++ )
	{
		if( !it->plugin )
			continue;

		MLOG(ZQ::common::Log::L_INFO,CLOGFMT( GatewayCenter,"unloadPlugin() trying to unload plugin [%s]"),
			it->name.c_str() );
		try
		{
			ZQ::common::DynSharedObj* obj = it->plugin;
			ZQTianShan::Util::TimeSpan sp;sp.start();
			{
				PluginObj p(*obj);

				if(!p.isValid())
				{
					MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(GatewayCenter,"unloadPlugin() target module[%s] may not be a valid gateway plugin"),
						it->name.c_str());
					continue;
				}

				lastLoadedModuleName = it->name;			

				p.MODULEUNINIT(*this);	
			}
			delete obj;
			MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"unloadPlugin() module[%s] unloaded, using [%lld] milliseconds"),
				it->name.c_str() , sp.stop() );
			
		}
		catch( std::exception& ex )
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayCenter,"unloadPlugin() caught exception[%s] while unloading plugin[%s]"),
				ex.what(), it->name.c_str() );
		}
		catch( ... )
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayCenter,"unloadPlugin() caught unknown exception while unloading plugin[%s]"),
				it->name.c_str() );
		}
	}
	sp.stop();
	if( mPlugins.size() >= 1)
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(GatewayCenter,"unloadPlugin() plugins unloaded, used [%lld] milliseconds"),sp.span() );
	else
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(GatewayCenter,"unloadPlugin() no plugin unloaded, used [%lld] milliseconds"),sp.span() );
	
	mPlugins.clear();
}

#define STRSWITCH() if(0){
#define STRCASE(x)	} else if(::strncmp( it->type.c_str() , x ,strlen(x) ) == 0 ){
#define STRENDCASE() }

bool GatewayCenter::setupSocketServer( )
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(GatewayCenter, "setupSocketserver() trying to setup socket server") );
	
	mDialogFactory = new GatewayDialogFactory(mEnv,*this);
	if(mDak)
	{
		delete mDak; 
		mDak = NULL ;
	}
	mDakEnv.dataFactory = mDialogFactory;
	mDak = new ZQ::DataPostHouse::DataPostDak(mDakEnv,mDialogFactory);

	mDialogFactory->start();

	if( !mDak->startDak(gwConfig.sockserver.threadcount) )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayCenter,"setupSocketServer() failed to start data post dak"));
		return false;
	}
	else
	{
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"setupSocketServer() start socket server with [%d] threads"),
			gwConfig.sockserver.threadcount);
	}

	const std::vector< Config::SocketServer::ServerListenerHolder>& listeners = gwConfig.sockserver.listeners;
	std::vector< Config::SocketServer::ServerListenerHolder>::const_iterator it = listeners.begin();

	for( ; it != listeners.end() ; it ++ )
	{
		MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(GatewayCenter,"setupSocketServer() trying to create listener: type[%s] ip[%s] port[%s] protocol[%s], exportaddress[%s]"),
			it->type.c_str() , it->ip.c_str() , it->port.c_str() , it->protocol.c_str() , it->exportAddress.c_str() );
		if( !(stricmp(it->protocol.c_str(),"lscp") == 0 || stricmp(it->protocol.c_str(),"dsmcc") == 0)  )
		{
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(GatewayCenter,"setupSocketServer() "));
			continue;
		}
		ZQ::DataPostHouse::IDataCommunicatorPtr comm = 0 ;
		DialogUserDataPtr userdata = new DialogUserData();
		userdata->protocol	= it->protocol;
		userdata->type		= it->type;

		if( stricmp( it->type.c_str() , "tcp") == 0  )
		{
			ZQ::DataPostHouse::AServerSocketTcpPtr p = new ZQ::DataPostHouse::AServerSocketTcp(*mDak,mDakEnv);
			if( !p->startServer( it->ip , it->port , userdata ) )
			{
				MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayCenter,"setupSocketServer() failed to start tcp server at[%s:%s]"),
					it->ip.c_str() , it->port.c_str() );
				continue;
			}
			else
			{
				mListeners.push_back( p );
				comm = p;
			}
			
		}
		else if( stricmp(it->type.c_str() , "udp") == 0 )
		{
			ZQ::DataPostHouse::AServerSocketUdpPtr p = new ZQ::DataPostHouse::AServerSocketUdp(*mDak,mDakEnv);
			if( !p->startServer(it->ip, it->port,userdata) )
			{
				MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(GatewayCenter,"setupSocketServer() failed to start udp server at[%s:%s]"),
					it->ip.c_str() , it->port.c_str() );
				continue;
			}
			else
			{
				mListeners.push_back( p );
				comm = p;
			}
		}
		else
		{
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(GatewayCenter,"setupSocketServer() unknown type[%s], only [tcp] and [udp] are accepted"),
				it->type.c_str() );
			continue;
		}
		MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"setupSocketServer() server started at[%s:%s], type[%s] protocol[%s], CommId[%lld]"),
			it->ip.c_str() , it->port.c_str() , it->type.c_str() , it->protocol.c_str(),comm ? comm->getCommunicatorId():-1);
	}
	
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"setupSocketServer() %d servers created"), mListeners.size() );
	
	return mListeners.size() >= 1;
}

void GatewayCenter::destroySocketServer( )
{
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(GatewayCenter,"destroySocketServer() trying to stop socket server"));

	mDialogFactory->stop();

	std::vector<ZQ::DataPostHouse::ASocketPtr>::iterator it = mListeners.begin();
	
	for ( ; it != mListeners.end() ; it ++ )
	{
		DialogUserDataPtr userdata = DialogUserDataPtr::dynamicCast( (*it)->getUserData() );

		if(!userdata)
			continue;

		if( stricmp(userdata->type.c_str() , "udp") == 0 )
		{
			ZQ::DataPostHouse::AServerSocketUdpPtr p = ZQ::DataPostHouse::AServerSocketUdpPtr::dynamicCast(*it);
			if(!p)
				continue;
			p->stop();
		}
		else if( stricmp(userdata->type.c_str() , "tcp") == 0 )
		{
			ZQ::DataPostHouse::AServerSocketTcpPtr p = ZQ::DataPostHouse::AServerSocketTcpPtr::dynamicCast(*it);
			if(!p)
				continue;
			p->stop();
		}
		else
		{

		}
	}
	
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayCenter,"destroySocketServer() %d listeners stopped"),
		mListeners.size() );
	mListeners.clear();

	mDialogFactory = 0;
	if(mDak)
	{
		mDak->stopDak();
		delete mDak;
	}
}

//////////////////////////////////////////////////////////////////////////

SessionFactory::SessionFactory(Environment& env, GatewayCenter& center )
:mEnv(env),
mGatewayCenter(center)
{
}

Ice::ObjectPtr SessionFactory::create(const ::std::string& id )
{
	if( id == TianShanIce::ClientRequest::Session::ice_staticId() )
	{
		return new ClientRequestSession(mEnv,mGatewayCenter);
	}
	else
	{
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////////
///TimerWatchdog
TimerWatchdog::TimerWatchdog( Environment& env , GatewayCenter& center )
:mEnv(env),
mGatewayCenter(center),
mbRunning(false)
{
}

TimerWatchdog::~TimerWatchdog()
{
}

bool TimerWatchdog::start()
{
	mbRunning = true;
	return ZQ::common::NativeThread::start();
}
void TimerWatchdog::stop()
{
	mbRunning = false;
	mSem.post();
	waitHandle(100*1000);
}
void TimerWatchdog::updateTimer(const std::string &sessId, int64 interval)
{
	if(sessId.empty())
		return;
	TimerKey key;
	key.sessid		= sessId;
	key.targettime	= interval + ZQ::common::now();
	bool bSignal = false;
	{
		ZQ::common::MutexGuard gd(mLocker);
		std::map<std::string, TimerKey>::iterator itLookup =  mTimerLookupMap.find(sessId);
		if( itLookup != mTimerLookupMap.end() )
		{
			mTimers.erase(itLookup->second);
		}
		mTimerLookupMap[sessId] = key;
		mTimers.insert(key);
		if( key.targettime < mNextWakeup )
			bSignal = true;
	}
	if(bSignal)
		mSem.post();
}

void TimerWatchdog::removeTimer( const std::string& sessId )
{
	if( sessId.empty())
		return;
			
	ZQ::common::MutexGuard gd(mLocker);
	std::map<std::string, TimerKey>::iterator itLookup =  mTimerLookupMap.find(sessId);
	if( itLookup != mTimerLookupMap.end())
	{
		mTimers.erase(itLookup->second);
		mTimerLookupMap.erase(itLookup);
	}
}

size_t TimerWatchdog::sessionCount() const
{
	ZQ::common::MutexGuard gd(mLocker);
	return mTimerLookupMap.size();
}

int TimerWatchdog::run(void)
{
	int64 interval = 60 * 1000;	
	int64 currentTime = 0 ;
	std::string sessid;
	while(mbRunning)
	{		
		currentTime = ZQ::common::now();
		while( true )
		{
			sessid.clear();
			{
				ZQ::common::MutexGuard gd(mLocker);
				if( mTimers.empty())					
				{
					interval = 60 * 1000;
					break;
				}
				std::set<TimerKey>::iterator it = mTimers.begin();
				if( it->targettime <= currentTime )
				{
					sessid = it->sessid;
					mTimers.erase(it);
					mTimerLookupMap.erase(sessid);
				}
				else
				{
					interval = it->targettime - currentTime;
					interval = MAX(5,interval);
					break;
				}
			}
			if( !sessid.empty() )
				mGatewayCenter.postTimerRequest( sessid );
		}
		{
			ZQ::common::MutexGuard gd(mLocker);
			mNextWakeup = interval + ZQ::common::now();
		}
		mSem.timedWait((timeout_t)interval);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
///class StrmHandleSessionMap

MemorySessionIndexMap::MemorySessionIndexMap(Environment& env, GatewayCenter& center )
:mEnv(env),
mCenter(center)
{
}
MemorySessionIndexMap::~MemorySessionIndexMap()
{
}
uint32 MemorySessionIndexMap::findStreamHandle( const std::string& sessioId ) const
{
	ZQ::common::MutexGuard gd(mLocker);
	std::map<std::string,uint32>::const_iterator it = mSess2Handle.find(sessioId);
	if( it == mSess2Handle.end())
		return (uint32)-1;
	else
		return it->second;
}
uint32 MemorySessionIndexMap::createStreamHandle( )
{
#ifdef _KWG_DEBUG
	return (uint32)3394210316;
#endif // KWG_DEBUG

	time_t timet;
	struct tm* pstm;
	uint32 id = 0;
	static uint32 sLastLWord = 0;
	ZQ::common::MutexGuard gd(mLocker);
	{
		timet = time(0);
		pstm = gmtime(&timet);
		if(pstm)
			id =(uint32)pstm->tm_hour << 27 | 
			(uint32)pstm->tm_min << 21 | 
			(uint32)pstm->tm_sec << 15;

		id = id | (pstm->tm_mday<<10 )| (sLastLWord ++);
		if(sLastLWord >= ( 1 << 10 ) )
			sLastLWord = 0;
	}
	return id;
}

std::string	MemorySessionIndexMap::findSessionForStrmHandle( uint32 handle ) const
{
	ZQ::common::MutexGuard gd(mLocker);
	std::map<uint32,std::string>::const_iterator it = mHandle2Sess.find(handle);
	if( it == mHandle2Sess.end() )
		return std::string("");
	else
		return it->second;
}
void MemorySessionIndexMap::removeStreamHandleInfo( const std::string& sessId )
{
	size_t clientSessionCount = 0;
	uint32 handle = 0 ;
	{
		ZQ::common::MutexGuard gd(mLocker);
		std::map<std::string,uint32>::iterator it = mSess2Handle.find(sessId);
		if( it == mSess2Handle.end())
			return;
		handle = it->second;
		mHandle2Sess.erase(it->second);
		mSess2Handle.erase(it);
		clientSessionCount = mSess2Handle.size();
	}
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(MemorySessionIndexMap,"removeStreamHandleInfo() remove clientSess[%s] with strmHandle[%u], current session count[%u]"),
		sessId.c_str() , handle, clientSessionCount);
}
void MemorySessionIndexMap::updateStreamHandleInfo( const std::string& sessId, uint32 handle)
{
	size_t clientSessionCount = 0;
	{
		ZQ::common::MutexGuard gd(mLocker);
		mSess2Handle[sessId] = handle;
		mHandle2Sess[handle] = sessId;
		clientSessionCount = mSess2Handle.size();
	}
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(MemorySessionIndexMap,"updateStreamHandleInfo() associate clientSess[%s] with strmHandle[%u], current session count[%u]"),
		sessId.c_str() , handle, clientSessionCount);
}

void MemorySessionIndexMap::updateStreamSessionInfo( const std::string& clientSessionId, const std::string& streamSessId )
{
	if(clientSessionId.empty() || streamSessId.empty())
		return;
	ZQ::common::MutexGuard gd(mLocker);
	mSess2StrmMap[clientSessionId] = streamSessId;
	mStrm2SessMap[streamSessId] = clientSessionId;
}

void MemorySessionIndexMap::removeStreamSessionInfo( const std::string& clientSessionId )
{
	ZQ::common::MutexGuard gd(mLocker);
	std::map<std::string, std::string>::iterator it = mSess2StrmMap.find(clientSessionId);
	if( it != mSess2StrmMap.end())
	{
		mStrm2SessMap.erase(it->second);
		mSess2StrmMap.erase(it);
	}
}

std::string MemorySessionIndexMap::findStreamSessId( const std::string& clientSessionId ) const
{
	ZQ::common::MutexGuard gd(mLocker);
	std::map<std::string, std::string>::const_iterator it = mSess2StrmMap.find(clientSessionId);
	if( it == mSess2StrmMap.end())
		return "";
	return it->second;

}

std::string MemorySessionIndexMap::findClientSessId( const std::string& streamSessId ) const
{
	ZQ::common::MutexGuard gd(mLocker);
	std::map<std::string,std::string>::const_iterator it = mStrm2SessMap.find(streamSessId);
	if( it == mStrm2SessMap.end() )
		return "";
	else
		return it->second;
}

}}//namespace ZQ::DSMCC

