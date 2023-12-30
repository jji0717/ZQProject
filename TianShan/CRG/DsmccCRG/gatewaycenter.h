#ifndef __zq_dsmcc_gateway_center_header_file_h__
#define __zq_dsmcc_gateway_center_header_file_h__

#include <string>
#include <map>
#include <set>
#include <Locks.h>
#include <NativeThreadPool.h>
#include <DynSharedObj.h>

#include "gateway_interface.h"
#include "sessdb.h"
#include <DataCommunicatorUnite.h>
#include "datadialog.h"


namespace ZQ { namespace CLIENTREQUEST{

class Environment;

class GatewayCenter;

class SessionFactory:public Ice::ObjectFactory
{
public:
	SessionFactory(Environment& env, GatewayCenter& center );	
	virtual ::Ice::ObjectPtr create(const ::std::string&);
	virtual void destroy(){}
private:
	Environment&	mEnv;
	GatewayCenter&	mGatewayCenter;
};

class TimerWatchdog : public ZQ::common::NativeThread
{
public:
	TimerWatchdog( Environment& env , GatewayCenter& center );
	virtual ~TimerWatchdog();

public:

	bool	start( );

	void	stop( );

	void	updateTimer( const std::string& sessId, int64 interval );

	void	removeTimer( const std::string& sessId );

	size_t	sessionCount() const;

protected:

	int		run(void);

private:
	Environment&		mEnv;
	GatewayCenter&		mGatewayCenter;
	ZQ::common::Mutex	mLocker;
	struct TimerKey 
	{
		std::string		sessid;
		int64			targettime;
		TimerKey():targettime(0){}
		bool operator<( const TimerKey& b) const
		{
			if( targettime < b.targettime )
				return true;
			else if(targettime == b.targettime)
				return strcmp(sessid.c_str(),b.sessid.c_str()) < 0 ;
			else
				return false;
		}
	};

	std::map<std::string, TimerKey> mTimerLookupMap;
	std::set<TimerKey>	mTimers;

	bool				mbRunning;
	ZQ::common::Semaphore mSem;
	int64				mNextWakeup;
};

class MemorySessionIndexMap
{
public:
	MemorySessionIndexMap(Environment& env, GatewayCenter& center );
	virtual ~MemorySessionIndexMap();
	
	uint32		findStreamHandle( const std::string& clientSessionId ) const;
	uint32		createStreamHandle( );
	std::string	findSessionForStrmHandle( uint32 handle )const;
	void		updateStreamHandleInfo( const std::string& clientSessionId, uint32 handle);
	void		removeStreamHandleInfo( const std::string& clientSessionId );
	void		updateStreamSessionInfo( const std::string& clientSessionId, const std::string& streamSessId );
	void		removeStreamSessionInfo( const std::string& clientSessionId );
	std::string	findStreamSessId( const std::string& clientSessionId ) const;
	std::string findClientSessId( const std::string& streamSessId ) const;
private:
	Environment&					mEnv;
	GatewayCenter&					mCenter;
	std::map<uint32,std::string>	mHandle2Sess;
	std::map<std::string,uint32>	mSess2Handle;

	std::map<std::string,std::string> mStrm2SessMap;
	std::map<std::string,std::string> mSess2StrmMap;

	ZQ::common::Mutex				mLocker;
	uint32							mLastIndex;
};

class GatewayCenter : public Gateway
{
public:
	GatewayCenter(Environment& env);
	virtual ~GatewayCenter(void);

public:
	
	/// start gateway
	bool		start(  const char* loggerpath , const char* dbpath);

	/// stop gateway
	void		stop();

	void		postTimerRequest( const std::string& sessId );

	MemorySessionIndexMap&	getStrmHandleMap( );

	ZQ::common::NativeThreadPool& getThreadPool()
	{
		return mThreadPool; 
	}

	inline uint32 activeCount()
	{
		return (uint32)mThreadPool.activeCount();
	}
	inline uint32 pendingRequestSize()
	{
		return (uint32)mThreadPool.pendingRequestSize();
	}
	inline uint32 size()
	{
		return (uint32)mThreadPool.size();
	}
	uint32		sessionCount()
	{
		return (uint32)mTimerWatchDog.sessionCount();
	}

public:

	virtual TianShanIce::ClientRequest::SessionPrx	createSession( const std::string& sessId , const std::string& clientId );

	virtual TianShanIce::ClientRequest::SessionPrx	openSession( const std::string& id );

	virtual std::vector<TianShanIce::ClientRequest::SessionPrx>	findSessionsByClient( const std::string& clientId );

	virtual void				destroySession( const std::string& id );

	virtual void				updateTimer( const std::string& sessionId, int64 interval );

	virtual uint32				sessionId2StreamHandle( const std::string& sessId ) const;

	virtual std::string			streamHandle2SessionId( uint32 streamHandle ) const ;

	virtual std::string			streamSessId2ClientSessionId( const std::string& streamSessId ) const;

	virtual ServerRequestPtr	createServerRequest( int64 connId , TianShanIce::ClientRequest::SessionPrx sess );

	virtual void				postRequest( RequestPtr req , RequestProcessPhase stage );

	virtual void				registerFixupRequestStage( FIXUPREQUEST proc );
	virtual void				registerContentHandlerStage( CONTENTHANDLER proc, const std::string& handlername );
	virtual void				registerFixupResponseStage( FIXUPRESPONSE proc );

	virtual void				unregisterFixupRequestStage( FIXUPREQUEST proc );
	virtual void				unregisterContentHandlerStage( CONTENTHANDLER proc, const std::string& uri );
	virtual void				unregisterFixupResponseStage( FIXUPRESPONSE proc );

	virtual void				registerOntimerProc( ONTIMER proc );

public:

	struct ContentHandlerInfo
	{
		CONTENTHANDLER	proc;
		std::string		name;
		ContentHandlerInfo()
			:proc(0)
		{
		}
	};

	struct FixupRequestInfo
	{
		FIXUPREQUEST	proc;
		std::string		name;
		FixupRequestInfo()
			:proc(0)
		{
		}
	};

	struct FixupResponseInfo
	{
		FIXUPRESPONSE	proc;
		std::string		name;
		FixupResponseInfo()
			:proc(0)
		{
		}
	};

	ContentHandlerInfo					getContentHandler( const std::string& uri );

	const std::vector<FixupRequestInfo>&	getFixupReqeustStack( ) const;
	const std::vector<FixupResponseInfo>&	getFixupResponseStack( ) const;

	SessionDatabase&					getDatabase( );

protected:
	
	bool					checkIfBusy( RequestPtr req );
	void					serverBusyResponse( RequestPtr req );

	bool					loadPlugin( const char* loggerpath );
	void					unloadPlugin( );

	bool					setupSocketServer( );
	void					destroySocketServer( );
	void					restoreDb( );

private:
	std::map<std::string,ContentHandlerInfo> mHandlerMap;

	Environment&					mEnv;

	ContentHandlerInfo				mDefaultHandler;

	bool							mbRunning;

	std::vector<FixupRequestInfo>	mFixupRequestStack;

	std::vector<FixupResponseInfo>	mFixupResponseStack;

	ONTIMER							mOnTimerProc;

	ZQ::common::Mutex				mLocker;

	SessionDatabase					mDb;
	
	ZQ::common::NativeThreadPool	mThreadPool;

	struct PluginInfo 
	{
		std::string					name;
		ZQ::common::DynSharedObj*	plugin;
		PluginInfo()
			:plugin(0)
		{
		}
	};
	std::vector<PluginInfo>	mPlugins;

	ZQ::DataPostHouse::DataPostHouseEnv			mDakEnv;
	ZQ::DataPostHouse::DataPostDak*				mDak;
	GatewayDialogFactoryPtr						mDialogFactory;
	std::vector<ZQ::DataPostHouse::ASocketPtr>	mListeners;

	TimerWatchdog					mTimerWatchDog;
	MemorySessionIndexMap			mStreamHandleMap;

};

}}//namespace ZQ::DSMCC

#endif//__zq_dsmcc_gateway_center_header_file_h__
