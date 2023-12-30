
#include <ZQ_common_conf.h>
#include <TimeUtil.h>
#include <DataCommunicatorUnite.h>
#include <TianShanIceHelper.h>
#include "environment.h"
#include "gatewayconfig.h"
#include "requestimpl.h"
#include "datadialog.h"


namespace ZQ{	namespace CLIENTREQUEST	{

extern Config::GateWayConfig gwConfig;
RequestImpl::RequestImpl( Environment& env , ZQ::DataPostHouse::IDataCommunicatorPtr comm )
:mEnv(env),
mComm(comm)
{
	mInitTime			= ZQ::common::now();
	mStartRunningTime	= 0;
	mResponse			= new ResponseImpl( env , comm );
}
RequestImpl::~RequestImpl()
{
	mResponse = 0;
}

int64 RequestImpl::getConnectionId( ) const
{
	if(!mComm)
		return -1;
	return mComm->getCommunicatorId();
}
void RequestImpl::updateLocalInfo( const std::string& ip , const std::string& port )
{
	mLocalIp = ip;
	mLocalPort = port;
}

void RequestImpl::updatePeerInfo( const std::string& ip, const std::string& port )
{
	mPeerIp = ip;
	mPeerPort = port;
	ResponseImpl* resp = (ResponseImpl*)mResponse.get();
	resp->updatePeerInfo( mPeerIp , mPeerPort );
}

bool RequestImpl::getLocalInfo( std::string& ip, std::string& port ) const
{
	ip = mLocalIp;
	port = mLocalPort;
	return true;
}

bool RequestImpl::getPeerInfo( std::string& ip, std::string& port ) const 
{
	ip = mPeerIp;
	port = mPeerPort;
	return true;
}

int64 RequestImpl::getRequestInitTime( ) const
{
	return mInitTime;
}

int64 RequestImpl::getRequestStartRunningTime( ) const
{
	return mStartRunningTime;
}


ResponsePtr RequestImpl::getResponse( )
{
	return mResponse;
}

void RequestImpl::attachMessage( MessageImplPtr msg )
{
	mMsg = msg;
	const std::map<std::string,std::string>& metaData = msg->getProperties();
	std::string sessId;
	ZQTianShan::Util::getPropertyDataWithDefault(metaData,CRMetaData_SessionId,"",sessId);
	setSessionId(sessId);
	ResponseImplPtr resp = ResponseImplPtr::dynamicCast(mResponse);
	resp->setSessionId(sessId);
	mMsg->setProperties(metaData);	
}

MessagePtr RequestImpl::getMessage( )
{
	return mMsg;
}

TianShanIce::ClientRequest::SessionPrx RequestImpl::getSession( )
{
	return mSession;
}

void RequestImpl::attachSession( TianShanIce::ClientRequest::SessionPrx sess )
{
	mSession = sess;
}

void RequestImpl::setStartRunningTime( int64 t )
{
	mStartRunningTime = t;
	ResponseImplPtr resp = ResponseImplPtr::dynamicCast(mResponse);
	resp->updateStartTime(mInitTime,t);
}

const std::string& RequestImpl::getSessionId( ) const
{
	return mSessId;
}

void RequestImpl::setSessionId( const std::string& sessId )
{
	mSessId = sessId;
}

//////////////////////////////////////////////////////////////////////////
ResponseImpl::ResponseImpl( Environment& env , ZQ::DataPostHouse::IDataCommunicatorPtr comm )
:MessageSender(env),
mEnv(env),
mComm(comm)
{
	mMsg = new MessageImpl(env);
}
ResponseImpl::~ResponseImpl()
{
}

bool ResponseImpl::complete(uint32 resultCode)
{
	if (!postMessage(mMsg, mComm))
		return false;

	int64 current = ZQ::common::now();
	int64 duration = current - mInitTime;
	uint32 uDuration = (uint32)duration;

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(ResponseImpl,"complete() session[%s/%d], took[%lld/%lld], %umsec"),
		mSessId.c_str() , mMsg->getCommand(), duration, current - mStartRunningTime, uDuration);

	mEnv.getCollector().collect(mMsg->getCommand(), resultCode, (uint32)duration);
	return true;
}

WritableMessagePtr ResponseImpl::getMessage()
{
	return mMsg;
}

//////////////////////////////////////////////////////////////////////////
MessageSender::MessageSender(Environment& env)
:mEnv(env)
{
}
bool MessageSender::postMessage( MessageImplPtr msg , ZQ::DataPostHouse::IDataCommunicatorPtr comm )
{
	if(!comm)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(MessageSender,"postMessage() no available communicator, refuse to send message"));
		return false;
	}
	GateWayDialogBase* dialogbase = 0;
	ZQ::DataPostHouse::SharedObjectPtr uData = comm->getUserData();
	DialogUserDataPtr userData = DialogUserDataPtr::dynamicCast(uData);
	ZQ::DataPostHouse::IDataDialogPtr dialogref = 0; //I'm not sure if this work

	dialogref = comm->getDataDialog();
	if( stricmp( userData->type.c_str(),"tcp") == 0  )
	{
		if( stricmp(userData->protocol.c_str(),"dsmcc") == 0 )
		{
			DsmccDialogTcpPtr dialog = DsmccDialogTcpPtr::dynamicCast(comm->getDataDialog());			
			dialogbase = dialog.get();
		}
		else
		{
			LscpDialogTcpPtr dialog = LscpDialogTcpPtr::dynamicCast( comm->getDataDialog() );			
			dialogbase = dialog.get();
		}
	}
	else
	{
		if( stricmp(userData->protocol.c_str(),"dsmcc") == 0 )
		{
			DsmccDialogUdpPtr dialog = DsmccDialogUdpPtr::dynamicCast( comm->getDataDialog() );			
			dialogbase = dialog.get();
		}
		else
		{
			LscpDialogUdpPtr dialog = LscpDialogUdpPtr::dynamicCast( comm->getDataDialog() );			
			dialogbase = dialog.get();
		}
	}
	
	if(!dialogbase)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(MessageSender,"postMessage() logical error in gateway service"));
		return false;
	}
	char buffer[32*1024];
	size_t size = sizeof(buffer)-1;
	if(!dialogbase->fromMessage(msg,buffer,size))
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(MessageSender,"postMessage() invalid message, failed to convert it to binary data"));
		return false;
	}
	int32 writtenSize = 0;
	std::string peerIp,peerPort;
	std::string localIp,localPort;
	if( comm->getCommunicatorType() == ZQ::DataPostHouse::COMM_TYPE_UDP )
	{
		ZQ::DataPostHouse::AUdpSocketPtr udpsock = ZQ::DataPostHouse::AUdpSocketPtr::dynamicCast( comm );
		if(!udpsock)
		{
			MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(MessageSender,"postMessage() failed to convert communicator to udpsocket, logical error"));
			return false;
		}
		peerIp = getPeerIp();
		peerPort = getPeerPort();
		writtenSize = udpsock->writeTo( buffer, size, peerIp, peerPort );
		udpsock->getLocalAddress(localIp,localPort);
	}
	else
	{
		comm->getRemoteAddress(peerIp,peerPort);
		writtenSize = comm->write( buffer, size );
		comm->getLocalAddress(localIp,localPort);
	}
	
	char PeerAddrInfoBuffer[512]; PeerAddrInfoBuffer[sizeof(PeerAddrInfoBuffer)-1] = 0;
	snprintf(PeerAddrInfoBuffer,sizeof(PeerAddrInfoBuffer)-1,"COMM[%lld/%s] local[%s:%s] ===>>> peer[%s:%s]",
		comm->getCommunicatorId() , 
		comm->getCommunicatorType() == ZQ::DataPostHouse::COMM_TYPE_TCP ? "tcp":"udp",
		localIp.c_str(), localPort.c_str(),
		peerIp.c_str() , peerPort.c_str() );

	if( writtenSize != size )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(MessageSender,"postMessage() failed to post message through %s"),PeerAddrInfoBuffer);
		return false;
	}
	if( gwConfig.sockserver.hexDumpEnabled >= 1)
	{		
		MLOG.hexDump(ZQ::common::Log::L_INFO,buffer,(int)size,PeerAddrInfoBuffer );
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
MessageImpl::MessageImpl( Environment &env )
:mEnv(env),
mCommand(COMMAND_NULL)
{
}
MessageImpl::~MessageImpl()
{
}

GATEWAYCOMMAND MessageImpl::getCommand( ) const
{
	return mCommand;
}

const std::map<std::string,std::string>& MessageImpl::getProperties( ) const
{
	ZQ::common::MutexGuard gd(mLocker);
	return mProperties;
}

void MessageImpl::setProperties( const std::map<std::string,std::string>& props )
{
	ZQ::common::MutexGuard gd(mLocker);
	mProperties = props;
}

const ZQ::DSMCC::DsmccResources& MessageImpl::getResources( ) const
{
	ZQ::common::MutexGuard gd(mLocker);
	return mResources;
}

void MessageImpl::setCommand( GATEWAYCOMMAND cmd )
{
	mCommand = cmd;
}

void MessageImpl::setResources( const ZQ::DSMCC::DsmccResources& resources )
{
	ZQ::common::MutexGuard gd(mLocker);
	mResources = resources;
}

//////////////////////////////////////////////////////////////////////////
ServerRequestImpl::ServerRequestImpl( Environment& env , ZQ::DataPostHouse::IDataCommunicatorPtr comm )
:MessageSender(env),
mEnv(env),
mComm(comm)
{
	mMsg = new MessageImpl(env);
}

ServerRequestImpl::~ServerRequestImpl()
{
}

WritableMessagePtr ServerRequestImpl::getMessage( )
{
	return mMsg;
}

bool ServerRequestImpl::complete( )
{
	return postMessage(mMsg,mComm);
}
void ServerRequestImpl::updatePeerInfo( const std::string& ip, const std::string& port )
{
	mPeerIp = ip;
	mPeerPort = port;
}

}}//namespace ZQ::CLIENTREQUEST
