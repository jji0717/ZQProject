#ifndef __zq_dsmcc_gateway_request_response_implement_header_file_h__
#define __zq_dsmcc_gateway_request_response_implement_header_file_h__

#include "DataCommunicator.h"
#include "gateway_interface.h"
#include "datadialog.h"

namespace ZQ{	namespace CLIENTREQUEST	{

class Environment;


class MessageImpl : public WritableMessage
{
public:

	MessageImpl( Environment &env );
	virtual ~MessageImpl();

	virtual GATEWAYCOMMAND	getCommand( ) const;

	virtual const ZQ::DSMCC::DsmccResources&	getResources( ) const;

	virtual const std::map<std::string,std::string>& getProperties( ) const;

	virtual void			setCommand( GATEWAYCOMMAND cmd );

	virtual void			setProperties( const std::map<std::string,std::string>& props );

	virtual void			setResources( const ZQ::DSMCC::DsmccResources& resources );
private:
	Environment&		mEnv;
	ZQ::common::Mutex	mLocker;
	ZQ::DSMCC::DsmccResources		mResources;
	ZQ::DSMCC::StringMap			mProperties;
	GATEWAYCOMMAND		mCommand;
};
typedef IceUtil::Handle<MessageImpl> MessageImplPtr;

class RequestImpl: public Request
{
public:
	RequestImpl( Environment& env , ZQ::DataPostHouse::IDataCommunicatorPtr comm );
	virtual ~RequestImpl(void);

public:

	virtual int64		getConnectionId( ) const;

	virtual bool		getLocalInfo( std::string& ip, std::string& port ) const ;
	virtual bool		getPeerInfo( std::string& ip, std::string& port ) const ;

	virtual	int64		getRequestInitTime( ) const ;
	virtual int64		getRequestStartRunningTime( ) const ;

	virtual ResponsePtr	getResponse( );

	TianShanIce::ClientRequest::SessionPrx getSession( );

	void				attachSession( TianShanIce::ClientRequest::SessionPrx sess );

	void				setStartRunningTime( int64 t );
	
	const std::string&	getSessionId( ) const;

	void				setSessionId( const std::string& sessId );

	virtual MessagePtr	getMessage( );

	void				updateLocalInfo( const std::string& ip , const std::string& port );

	void				updatePeerInfo( const std::string& ip, const std::string& port );

	void				attachMessage( MessageImplPtr msg );

private:
	Environment&		mEnv;
	int64				mInitTime;
	int64				mStartRunningTime;
	ResponsePtr			mResponse;
	ZQ::DataPostHouse::IDataCommunicatorPtr mComm;
	MessageImplPtr		mMsg;
	TianShanIce::ClientRequest::SessionPrx mSession;
	std::string			mSessId;

	std::string			mLocalIp;
	std::string			mLocalPort;
	std::string			mPeerIp;
	std::string			mPeerPort;
};
typedef IceUtil::Handle<RequestImpl> RequestImplPtr;


class MessageSender
{
public:
	MessageSender(Environment& env);
	virtual ~MessageSender(){}
	bool postMessage( MessageImplPtr msg , ZQ::DataPostHouse::IDataCommunicatorPtr comm );
	virtual const std::string& getPeerIp( ) const = 0;
	virtual const std::string& getPeerPort( ) const = 0;
private:
	Environment&	mEnv;
};

class ResponseImpl: public Response, public MessageSender
{
public:
	ResponseImpl( Environment& env , ZQ::DataPostHouse::IDataCommunicatorPtr comm );
	virtual ~ResponseImpl();
	virtual WritableMessagePtr getMessage( );
	virtual bool complete(uint32 resultCode=0);
	virtual const std::string& getPeerIp( ) const
	{
		return mPeerIp;
	}
	virtual const std::string& getPeerPort( ) const
	{
		return mPeerPort;
	}
	void	updatePeerInfo( const std::string& peerIp, const std::string& peerPort )
	{
		mPeerIp = peerIp;
		mPeerPort = peerPort;
	}
	void updateStartTime( int64 initTime , int64 startTime )
	{
		mInitTime = initTime;
		mStartRunningTime = startTime;
	}
	void setSessionId( const std::string& sessId )
	{
		mSessId = sessId;
	}
private:
	Environment&		mEnv;
	ZQ::DataPostHouse::IDataCommunicatorPtr mComm;
	MessageImplPtr		mMsg;	
	std::string			mPeerIp;
	std::string			mPeerPort;
	int64				mInitTime;
	int64				mStartRunningTime;
	std::string			mSessId;

};

typedef IceUtil::Handle<ResponseImpl> ResponseImplPtr;


class ServerRequestImpl : public ServerRequest, public MessageSender
{
public:
	ServerRequestImpl( Environment& env , ZQ::DataPostHouse::IDataCommunicatorPtr comm );
	virtual ~ServerRequestImpl();

	virtual WritableMessagePtr getMessage( );

	virtual bool	complete();

	void			updatePeerInfo( const std::string& ip, const std::string& port );

protected:
	virtual const std::string& getPeerIp( ) const
	{
		return mPeerIp;
	}
	virtual const std::string& getPeerPort( ) const
	{
		return mPeerPort;
	}
private:
	Environment&	mEnv;
	ZQ::DataPostHouse::IDataCommunicatorPtr mComm;
	MessageImplPtr	mMsg;
	std::string		mPeerIp;
	std::string		mPeerPort;
};
typedef IceUtil::Handle<ServerRequestImpl> ServerRequestImplPtr;

}}//namespace ZQ::CLIENTREQUEST

#endif//__zq_dsmcc_gateway_request_response_implement_header_file_h__
