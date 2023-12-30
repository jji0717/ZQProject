
#ifndef _c2client_http_session_header_file_banana__
#define _c2client_http_session_header_file_banana__

#include <DataPostHouse/DataCommunicatorUnite.h>
#include <Locks.h>
#include <boost/function.hpp>
#include "HttpProtocol.h"
#include "SessionDataRecorder.h"
#include <set>

class HttpSessionFactory;
class SimpleHttpSession : public ZQ::DataPostHouse::SharedObject
{
public:
	SimpleHttpSession( SessionDataRecorder& recorder ,HttpSessionFactory& fac);
	virtual ~SimpleHttpSession();
	
	virtual void onCreated( ZQ::DataPostHouse::IDataCommunicatorPtr comm ) ;

	virtual void onHeadersComplete( HttpMessagePtr msg ) ;

	virtual bool onBodyContent( const char* data , size_t size ,bool bComplete ) ;

	virtual void onMessageComplete( ) ;

	void		 onBadBodyContent( const char* data , size_t size );

private:
	
	bool				mbFirstData;
	ZQ::DataPostHouse::IDataCommunicatorPtr	mComm;
	HttpMessagePtr		mMsg;
	SessionDataRecorder&	mRecorder;
	DataRecord*				mDataRecorder;
	HttpSessionFactory&		mSessFac;
};

typedef ZQ::DataPostHouse::ObjectHandle<SimpleHttpSession> SimpleHttpSessionPtr;

typedef boost::function< void (void ) > SimpleSessionEvent ;

class HttpSessionFactory : public ZQ::DataPostHouse::SharedObject
{
public:
	HttpSessionFactory( );
	virtual ~HttpSessionFactory();
	virtual SimpleHttpSessionPtr		createSession() ;

	SessionDataRecorder&				getRecorder( );
	
	uint64								getTotalBytes( ) const;

	void								registerEventHandler( SimpleSessionEvent startEvent , SimpleSessionEvent completeEvent );

	void								resetTotalBytes( );

protected:

	friend class SimpleHttpSession;
	void								updateTotalByte( uint64 byteInc );

	void								onSessionComplete( SimpleHttpSessionPtr sess );

	void								onSessionCreated( SimpleHttpSessionPtr sess );

private:
	SessionDataRecorder					mRecorder;
	uint64								mTotalBytes;
	ZQ::common::Mutex					mMutex;
	typedef std::set<SimpleHttpSessionPtr>	SessionSET;
	SessionSET							mSessions;
	SimpleSessionEvent					mSessionStartEvent;
	SimpleSessionEvent					mSessionCompleteEvent;
};

typedef ZQ::DataPostHouse::ObjectHandle<HttpSessionFactory> HttpSessionFactoryPtr;

class HttpSessionUserData : public ZQ::DataPostHouse::SharedObject
{
public:
	virtual ~HttpSessionUserData(){}
	virtual std::string getTag( ) const = 0;
};

typedef ZQ::DataPostHouse::ObjectHandle<HttpSessionUserData> HttpSessionUserDataPtr;


#endif//_c2client_http_session_header_file_banana__

