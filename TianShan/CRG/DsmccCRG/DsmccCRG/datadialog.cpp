
#include "environment.h"
#include "gatewayconfig.h"
#include "DsmccMsg.h"
#include <lsc_common.h>
#include <lsc_parser.h>
#include <TianShanIceHelper.h>
#include "datadialog.h"
#include "gatewaycenter.h"


namespace ZQ{ namespace CLIENTREQUEST {

extern Config::GateWayConfig gwConfig;

GatewayDialogFactory::GatewayDialogFactory(Environment& env,GatewayCenter& center)
:mEnv(env),
mGatewayCenter(center),
mbQuit(false)
{
}

GatewayDialogFactory::~GatewayDialogFactory()
{
}

void GatewayDialogFactory::onReleaseDataDialog( ZQ::DataPostHouse::IDataDialogPtr dialog , ZQ::DataPostHouse::IDataCommunicatorPtr communicator )
{
	if( !communicator)
		return;
	ZQ::common::MutexGuard gd(mLocker);
	mCommsMap.erase(communicator->getCommunicatorId());
}

void GatewayDialogFactory::stop( )
{
	mbQuit = true;
	mSem.post();
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(GatewayDialogFactory,"stop() trying to stop dialog factory"));
	waitHandle(-1);
	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(GatewayDialogFactory,"stop() dialog factory stopped"));
}

int GatewayDialogFactory::run()
{
	mbQuit = false;
	int32 maxIdleTime = gwConfig.sockserver.connIdleTimeout;
	maxIdleTime = MAX(maxIdleTime,60*1000);
	maxIdleTime	= MIN( maxIdleTime, 10 * 24 * 3600 * 1000 );

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayDialogFactory,"running..."));
	while( !mbQuit )
	{
		mSem.timedWait( 5 * 1000 );
		std::map<int64 , ZQ::DataPostHouse::IDataCommunicatorPtr> conns;
		{
			ZQ::common::MutexGuard gd(mLocker);
			conns = mCommsMap;
		}
		std::map<int64 , ZQ::DataPostHouse::IDataCommunicatorPtr>::iterator it = conns.begin();
		for( ; it != conns.end(); it ++ )
		{
			ZQ::DataPostHouse::IDataCommunicatorPtr c = it->second;
			if(!c)	continue;
			uint32 idleTime = c->getIdleTime();

			DialogUserDataPtr userdata = DialogUserDataPtr::dynamicCast( c->getUserData() );

			if(!userdata)
				continue;
			if( stricmp(userdata->type.c_str() , "tcp") != 0 )
				continue;
			if( idleTime >= maxIdleTime )
			{
				MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(GatewayDialogFactory,"run() destroy comm[%lld] due to idletime[%u] >= maxIdleTime[%d]"),
					c->getCommunicatorId(), idleTime,maxIdleTime);
				c->close();
			}
		}
	}
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(GatewayDialogFactory,"quiting..."));
	return 0;
}

ZQ::DataPostHouse::IDataDialogPtr GatewayDialogFactory::onCreateDataDialog( ZQ::DataPostHouse::IDataCommunicatorPtr comm )
{
	DialogUserDataPtr userdata = DialogUserDataPtr::dynamicCast( comm->getUserData() );
	if( !userdata )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayDialogFactory,"onCreateDataDialog() no user data is found, refuse to create data dialog"));
		return 0;
	}

	ZQ::common::MutexGuard gd(mLocker);
	if( (gwConfig.sockserver.maxConnection > 0) && (mCommsMap.size() > (size_t)gwConfig.sockserver.maxConnection) )
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayDialogFactory,"onCreateDataDialog() reach to the max connection limit [%d], refuse to create new connection"),
			gwConfig.sockserver.maxConnection);
		return 0;
	}
	
	ZQ::DataPostHouse::IDataDialogPtr dialog = 0;
	if( stricmp( userdata->type.c_str(), "tcp" ) == 0  )
	{
		if( stricmp( userdata->protocol.c_str() ,"dsmcc" ) == 0 )
		{
			dialog = new DsmccDialogTcp(mEnv,mGatewayCenter);
		}
		else if( stricmp( userdata->protocol.c_str(), "lscp") == 0)
		{
			dialog = new LscpDialogTcp(mEnv,mGatewayCenter);
		}
		else
		{
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(GatewayCenter,"setupSocketserver() unknown protocl[%s], only [dsmcc] and [dsmcc] are accepted"),
				userdata->protocol.c_str() );
		}
	}
	else if( stricmp(userdata->type.c_str(), "udp") == 0 )
	{
		if( stricmp( userdata->protocol.c_str() ,"dsmcc" ) == 0 )
		{
			dialog = new DsmccDialogUdp(mEnv,mGatewayCenter);
		}
		else if( stricmp(userdata->protocol.c_str(), "lscp") == 0)
		{
			dialog = new LscpDialogUdp(mEnv,mGatewayCenter);
		}
		else
		{
			MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(GatewayCenter,"setupSocketserver() unknown protocl[%s], only [dsmcc] and [dsmcc] are accepted"),
				userdata->protocol.c_str() );
		}
	}
	else
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(GatewayCenter,"setupSocketserver() unknown type[%s], only [tcp] and [udp] are accepted"),
			userdata->type.c_str() );
	}

	if( dialog )
	{		
		mCommsMap[comm->getCommunicatorId()] = comm;
	}
	return dialog;
}

void GatewayDialogFactory::onClose( CommunicatorS& comms )
{
	//do nothing
}
ZQ::DataPostHouse::IDataCommunicatorPtr GatewayDialogFactory::findCommunicator( int64 id ) const
{
	ZQ::common::MutexGuard gd(mLocker);
	std::map<int64 , ZQ::DataPostHouse::IDataCommunicatorPtr>::const_iterator it = mCommsMap.find(id);
	if( it == mCommsMap.end() )
		return 0;
	return it->second;
}

//////////////////////////////////////////////////////////////////////////
void GateWayDialogBase::initConnInfo( ZQ::DataPostHouse::IDataCommunicatorPtr comm )
{
	if(!comm)
		return;
	char buffer[512]; buffer[sizeof(buffer)-1] = 0;
	comm->getLocalAddress(mLocalIp,mLocalPort);
	comm->getRemoteAddress(mPeerIp,mPeerPort);
	snprintf(buffer,sizeof(buffer)-1,"COMM[%lld/%s] local[%s:%s] <<<=== peer[%s:%s] ",
		comm->getCommunicatorId() ,
		comm->getCommunicatorType() == ZQ::DataPostHouse::COMM_TYPE_TCP ? "tcp":"udp",
		mLocalIp.c_str(), mLocalPort.c_str(),
		mPeerIp.c_str() , mPeerPort.c_str() );
	mConnInfo = buffer;
}
GateWayDialogBase::GateWayDialogBase( Environment& env, GatewayCenter& center )
:mEnv(env),
mGatewayCenter(center)
{
}

//////////////////////////////////////////////////////////////////////////
GATEWAYCOMMAND		fromDsmccCommand( uint16 msgId )
{
	switch( msgId )
	{
	case ZQ::DSMCC::MsgID_SetupRequest:	return COMMAND_SETUP;
	case ZQ::DSMCC::MsgID_SetupConfirm:	return COMMAND_SETUP_RESPONSE;
	case ZQ::DSMCC::MsgID_ReleaseRequest:	return COMMAND_DESTROY;
	case ZQ::DSMCC::MsgID_ReleaseConfirm:	return COMMAND_DESTROY_RESPONSE;
	case ZQ::DSMCC::MsgID_ReleaseIndication: return COMMAND_RELEASE_INDICATION;
	case ZQ::DSMCC::MsgID_ReleaseResponse:	return COMMAND_RELEASE_RESPONSE;
	case ZQ::DSMCC::MsgID_ProceedingIndication: return COMMAND_PROCEEDING_INDICATION;
	case ZQ::DSMCC::MsgID_InProgressRequest: return COMMAND_SESS_IN_PROGRESS;
	default:					return COMMAND_NULL;
	}
}
GATEWAYCOMMAND	fromLscpCommand( uint8 msgId )
{
	switch (msgId)
	{
	case lsc::LSC_PAUSE:		return COMMAND_PAUSE;
	case lsc::LSC_PAUSE_REPLY:	return COMMAND_PAUSE_RESPONSE;
	case lsc::LSC_RESUME:		return COMMAND_RESUME;
	case lsc::LSC_RESUME_REPLY:	return COMMAND_RESUME_RESPONSE;
	case lsc::LSC_STATUS:		return COMMAND_STATUS;
	case lsc::LSC_STATUS_REPLY:	return COMMAND_STATUS_RESPONSE;
	case lsc::LSC_JUMP:			return COMMAND_JUMP;
	case lsc::LSC_JUMP_REPLY:	return COMMAND_JUMP_RESPONSE;
	case lsc::LSC_PLAY:			return COMMAND_PLAY;
	case lsc::LSC_PLAY_REPLY:	return COMMAND_PLAY_RESPONSE;
	case lsc::LSC_RESET:		return COMMAND_RESET;
	case lsc::LSC_RESET_REPLY:	return COMMAND_RESET_RESPONSE;
	case lsc::LSC_DONE:			return COMMAND_DONE_RESPONSE;
	default:					return COMMAND_NULL;
	}
}

uint16 toProtocolCommand( GATEWAYCOMMAND cmd )
{
	switch(cmd)
	{
	case COMMAND_SETUP:                      	return ZQ::DSMCC::MsgID_SetupRequest;
	case COMMAND_SETUP_RESPONSE:             	return ZQ::DSMCC::MsgID_SetupConfirm;
	case COMMAND_DESTROY:                    	return ZQ::DSMCC::MsgID_ReleaseRequest;
	case COMMAND_DESTROY_RESPONSE:           	return ZQ::DSMCC::MsgID_ReleaseConfirm;
	case COMMAND_RELEASE_INDICATION:         	return ZQ::DSMCC::MsgID_ReleaseIndication;
	case COMMAND_RELEASE_RESPONSE:           	return ZQ::DSMCC::MsgID_ReleaseResponse;
	case COMMAND_PROCEEDING_INDICATION:      	return ZQ::DSMCC::MsgID_ProceedingIndication;
	case COMMAND_SESS_IN_PROGRESS:           	return ZQ::DSMCC::MsgID_InProgressRequest;
	case COMMAND_PAUSE:                      	return lsc::LSC_PAUSE;
	case COMMAND_PAUSE_RESPONSE:             	return lsc::LSC_PAUSE_REPLY;
	case COMMAND_RESUME:                     	return lsc::LSC_RESUME;
	case COMMAND_RESUME_RESPONSE:            	return lsc::LSC_RESUME_REPLY;
	case COMMAND_STATUS:                     	return lsc::LSC_STATUS;
	case COMMAND_STATUS_RESPONSE:            	return lsc::LSC_STATUS_REPLY;
	case COMMAND_JUMP:                       	return lsc::LSC_JUMP;
	case COMMAND_JUMP_RESPONSE:              	return lsc::LSC_JUMP_REPLY;
	case COMMAND_PLAY:                       	return lsc::LSC_PLAY;
	case COMMAND_PLAY_RESPONSE:              	return lsc::LSC_PLAY_REPLY;
	case COMMAND_RESET:                      	return lsc::LSC_RESET;
	case COMMAND_RESET_RESPONSE:             	return lsc::LSC_RESET_REPLY;
	case COMMAND_DONE_RESPONSE:					return lsc::LSC_DONE;

	default:									return 0;
	}
}

const char* command2Str( const GATEWAYCOMMAND& cmd )
{
	switch(cmd)
	{
	case COMMAND_SETUP:                      	return "setup";
	case COMMAND_SETUP_RESPONSE:             	return "setup_response";
	case COMMAND_DESTROY:                    	return "destroy";
	case COMMAND_DESTROY_RESPONSE:           	return "destroy_response";
	case COMMAND_RELEASE_INDICATION:         	return "release_indication";
	case COMMAND_RELEASE_RESPONSE:           	return "release_response";
	case COMMAND_PROCEEDING_INDICATION:      	return "proceeding_indication";
	case COMMAND_SESS_IN_PROGRESS:           	return "sess_in_progress";
	case COMMAND_PAUSE:                      	return "pause";
	case COMMAND_PAUSE_RESPONSE:             	return "pause_reply";
	case COMMAND_RESUME:                     	return "resume";
	case COMMAND_RESUME_RESPONSE:            	return "resume_reply";
	case COMMAND_STATUS:                     	return "status";
	case COMMAND_STATUS_RESPONSE:            	return "status_reply";
	case COMMAND_JUMP:                       	return "jump";
	case COMMAND_JUMP_RESPONSE:              	return "jump_reply";
	case COMMAND_PLAY:                       	return "play";
	case COMMAND_PLAY_RESPONSE:              	return "play_reply";
	case COMMAND_RESET:                      	return "reset";
	case COMMAND_RESET_RESPONSE:             	return "reset_reply";
	case COMMAND_DONE_RESPONSE:					return "lsc_done";
	default:									return "unknown";
	}
}

//////////////////////////////////////////////////////////////////////////
///
GatewayDsmccDialog::GatewayDsmccDialog(Environment& env,GatewayCenter& center)
:GateWayDialogBase(env,center)
{
}

WritableMessagePtr GatewayDsmccDialog::toMessage(const char *buf, size_t &size)
{
	size_t byteprocessed = 0;
	ZQ::DSMCC::DsmccMsg::Ptr msg =  ZQ::DSMCC::DsmccMsg::parseMessage((const uint8*)buf,size,byteprocessed);
	size = size - byteprocessed;
	if( msg )
	{
		WritableMessagePtr p = new MessageImpl(mEnv);
		p->setCommand( fromDsmccCommand(msg->getMessageId()) );
		ZQ::DSMCC::StringMap smap;
		msg->toMetaData(smap);
		p->setProperties(smap);		
		return p;
	}
	else
	{
		return 0;
	}
}

bool GatewayDsmccDialog::fromMessage( WritableMessagePtr message, char *buf, size_t& size)
{
	MessageImplPtr msg = MessageImplPtr::dynamicCast( message );

	if(!msg ||!buf || size <= 0 )
		return false;
	uint16 msgId = toProtocolCommand(msg->getCommand());

	ZQ::DSMCC::DsmccMsg::Ptr pMsg = 0;
	ZQ::DSMCC::DsmccMsg::HardHeader tmpHeader;
	switch(msgId)
	{
	case ZQ::DSMCC::MsgID_SetupRequest:
		pMsg = new ZQ::DSMCC::ClientSessionSetupRequest(tmpHeader);
		break;
	case ZQ::DSMCC::MsgID_SetupConfirm:
		pMsg = new ZQ::DSMCC::ClientSessionSetupConfirm(tmpHeader);
		break;
	case ZQ::DSMCC::MsgID_ReleaseRequest:
		pMsg = new ZQ::DSMCC::ClientSessionReleaseRequest(tmpHeader);
		break;
	case ZQ::DSMCC::MsgID_ReleaseConfirm:    
		pMsg = new ZQ::DSMCC::ClientSessionReleaseConfirm(tmpHeader);
		break;
	case ZQ::DSMCC::MsgID_ReleaseIndication:    
		pMsg = new ZQ::DSMCC::ClientSessionReleaseIndication(tmpHeader);
		break;
	case ZQ::DSMCC::MsgID_ReleaseResponse:   
		pMsg = new ZQ::DSMCC::ClientSessionReleaseResponse(tmpHeader);
		break;
	case ZQ::DSMCC::MsgID_ProceedingIndication: 
		pMsg = new ZQ::DSMCC::ClientSessionProceedingIndication(tmpHeader);
		break;
	case ZQ::DSMCC::MsgID_InProgressRequest: 
		pMsg = new ZQ::DSMCC::ClientSessionInProgressRequest(tmpHeader);
		break;
	default:
		pMsg = 0;
	}
	if(!pMsg)
		return false;
	
	pMsg->readMetaData(msg->getProperties());
	if(msgId == ZQ::DSMCC::MsgID_SetupConfirm)
		pMsg->readResource(msg->getResources());

	int sizeUsed = pMsg->toMessage((uint8*)buf,size);
	size = sizeUsed;
	return true;
}

GatewayLscpDialog::GatewayLscpDialog(Environment& env,GatewayCenter& center)
:GateWayDialogBase(env,center)
{
}

bool GatewayLscpDialog::fromMessage( WritableMessagePtr pMsg, char *buf, size_t &size)
{
	MessageImplPtr msg = MessageImplPtr::dynamicCast(pMsg);
	if(!msg || !buf ||size <= 0 )
		return false;
	uint8_t msgId = (uint8)toProtocolCommand( msg->getCommand() );
	lsc::lscMessage* lscMsg = 0 ;
	switch( msgId )
	{
	case lsc::LSC_PAUSE:	lscMsg = new lsc::lscPause();	break;
	case lsc::LSC_RESUME:	lscMsg = new lsc::lscResume();	break;
	case lsc::LSC_RESET:	lscMsg = new lsc::lscReset( ); break;
	case lsc::LSC_JUMP:		lscMsg = new lsc::lscJump( );	break;
	case lsc::LSC_PLAY:		lscMsg = new lsc::lscPlay( ); break;	

	case lsc::LSC_PAUSE_REPLY:
	case lsc::LSC_RESUME_REPLY:
	case lsc::LSC_STATUS_REPLY:
	case lsc::LSC_RESET_REPLY:
	case lsc::LSC_JUMP_REPLY:
	case lsc::LSC_PLAY_REPLY:
	case lsc::LSC_DONE:
		lscMsg = new lsc::lscResponse();	break;
	default:
		lscMsg = new lsc::lscMessage(); break;
	}
	lscMsg->readMetaData( msg->getProperties() );
//	lscMsg->hton();
	
	memcpy( buf , &(lscMsg->GetLscMessageContent()) , MIN(lscMsg->getMsgSize(),size ) );
	
	size = MIN(lscMsg->getMsgSize(),size );

	delete lscMsg;
	return true;	
}

WritableMessagePtr GatewayLscpDialog::toMessage( const char* buf , size_t& size )
{
	lsc::lscMessage* newMsg = 0;
	void* pBuf = (void*)buf;
	int newsize = (int)size;
	
	lsc::lscMessage* msg = 0;
	try
	{
		msg = lsc::ParseMessage( pBuf,newsize, newMsg );
	}
	catch( const std::exception& ex)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayLscpDialog,"toMessage() caught [%s] while parsing lscp data"),
			ex.what());
		return 0;
	}
	if(!msg)
	{
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayLscpDialog,"toMessage() failed to parse lscp message into a structure"));
		return 0;
	}
	lsc::lscMessage* pRealMsg = 0;
	switch( msg->GetLscMessageContent().jump.header.opCode)
	{
	case	lsc::LSC_PAUSE:				pRealMsg = new lsc::lscPause();		break;
	case	lsc::LSC_PAUSE_REPLY:		pRealMsg = new lsc::lscResponse();	break;
	case	lsc::LSC_RESUME:			pRealMsg = new lsc::lscResume();	break;
	case	lsc::LSC_RESUME_REPLY:		pRealMsg = new lsc::lscResponse();	break;
	case	lsc::LSC_STATUS:			pRealMsg = new lsc::lscStatus();	break;
	case	lsc::LSC_STATUS_REPLY:		pRealMsg = new lsc::lscResponse();	break;
	case	lsc::LSC_RESET:				pRealMsg = new lsc::lscReset();		break;
	case	lsc::LSC_RESET_REPLY:		pRealMsg = new lsc::lscResponse();	break;
	case	lsc::LSC_JUMP:				pRealMsg = new lsc::lscJump();		break;
	case	lsc::LSC_JUMP_REPLY:		pRealMsg = new lsc::lscResponse();	break;
	case	lsc::LSC_PLAY:				pRealMsg = new lsc::lscPlay();		break;
	case	lsc::LSC_PLAY_REPLY:		pRealMsg = new lsc::lscResponse();	break;
	case	lsc::LSC_DONE:				pRealMsg = new lsc::lscResponse();	break;
	default:
		pRealMsg = 0;	break;
	}

	if(!pRealMsg)
	{
		delete msg;
		msg =0;
		MLOG(ZQ::common::Log::L_ERROR,CLOGFMT(GatewayLscpDialog,"toMessage() failed to parse lscp messgae"));
		return 0;
	}
	
	pRealMsg->setMessageContent(msg->GetLscMessageContent());
	pRealMsg->setOpCode(msg->GetLscMessageOpCode());
	
	delete msg;
	msg =0;

	size = newsize;
	
	lsc::StringMap smap;
	pRealMsg->toMetaData(smap);
	int64 strmHandle = 0 ;
	ZQTianShan::Util::getPropertyDataWithDefault(smap,CRMetaData_LscStreamHandle,0,strmHandle);
	std::string sessId = mGatewayCenter.streamHandle2SessionId( (uint32)strmHandle );
	smap[CRMetaData_SessionId] = sessId;
	
	MessageImpl* p = new MessageImpl(mEnv);
	p->setProperties( smap );	
	p->setCommand(  fromLscpCommand(pRealMsg->GetLscMessageOpCode()));
	
	delete pRealMsg;
	pRealMsg = 0;

	return p;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
///DsmccDialogTcp
DsmccDialogTcp::DsmccDialogTcp( Environment& env , GatewayCenter& center)
:GatewayDsmccDialog(env,center)
{
}
DsmccDialogTcp::~DsmccDialogTcp()
{
}

void DsmccDialogTcp::onCommunicatorSetup( ZQ::DataPostHouse::IDataCommunicatorPtr communicator )
{
	initConnInfo(communicator);
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(DsmccDialogTcp,"onCommunicatorSetup() comes a new connection [%s]"),mConnInfo.c_str() );
	mComm = communicator;
}

void DsmccDialogTcp::onCommunicatorDestroyed( ZQ::DataPostHouse::IDataCommunicatorPtr communicator )
{
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(DsmccDialogTcp,"onCommunicatorDestroyed() connection destroyed [%s]"),mConnInfo.c_str() );
	mComm = 0;
}

bool DsmccDialogTcp::onRead( const int8* buffer , size_t bufSize )
{
	if( gwConfig.sockserver.hexDumpEnabled >= 1 )
	{
		MLOG.hexDump(ZQ::common::Log::L_INFO,buffer,(int)bufSize,mConnInfo.c_str() );
	}
	WritableMessagePtr msg = toMessage(buffer,bufSize);
	if(!msg)
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(DsmccDialogTcp,"onRead() %s failed to parse data as a dsmcc message"),mConnInfo.c_str());
		return true;
	}
	
	RequestImplPtr pReq = new RequestImpl(mEnv,mComm);
	
	pReq->attachMessage( MessageImplPtr::dynamicCast(msg) );
	pReq->updateLocalInfo(mLocalIp,mLocalPort);
	pReq->updatePeerInfo(mPeerIp,mPeerPort);	

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(DsmccDialogTcp,"onRead() got a new request, session[%s], command[%s]"),
		pReq->getSessionId().c_str(), command2Str( msg->getCommand() ) );
	mGatewayCenter.postRequest(pReq,PHASE_FIXUP_REQUEST);

	return true;
}

void DsmccDialogTcp::onWritten( size_t bufSize )
{//do nothing
}

void DsmccDialogTcp::onError( )
{
}

//////////////////////////////////////////////////////////////////////////
///DsmccDialogUdp
DsmccDialogUdp::DsmccDialogUdp( Environment& env,GatewayCenter& center )
:GatewayDsmccDialog(env,center)
{
}
DsmccDialogUdp::~DsmccDialogUdp()
{
}

void DsmccDialogUdp::onError( )
{

}


void DsmccDialogUdp::onData( const int8* buffer , size_t bufSize , ZQ::DataPostHouse::IDataCommunicatorPtr comm  )
{
	initConnInfo( comm );
	if( gwConfig.sockserver.hexDumpEnabled >= 1 )
	{
		MLOG.hexDump(ZQ::common::Log::L_INFO,buffer,(int)bufSize,mConnInfo.c_str() );
	}
	
	WritableMessagePtr msg = toMessage(buffer,bufSize);
	if(!msg)
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(DsmccDialogUdp,"onData() %s failed to parse data as a dsmcc message"),mConnInfo.c_str());
		return ;
	}

	RequestImplPtr pReq = new RequestImpl(mEnv,comm);
	
	MessageImplPtr pNewMsg = MessageImplPtr::dynamicCast( msg ) ;
	pReq->attachMessage( pNewMsg );
	pReq->updateLocalInfo(mLocalIp,mLocalPort);
	pReq->updatePeerInfo(mPeerIp,mPeerPort);

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(DsmccDialogUdp,"onData() got a new request, session[%s] command[%s]"),
		pReq->getSessionId().c_str(), command2Str( msg->getCommand() ) );

	mGatewayCenter.postRequest(pReq,PHASE_FIXUP_REQUEST);
	return ;
}

//////////////////////////////////////////////////////////////////////////
///LscpDialogTcp
LscpDialogTcp::LscpDialogTcp( Environment& env , GatewayCenter& center)
:GatewayLscpDialog(env,center)
{
}
LscpDialogTcp::~LscpDialogTcp()
{
}

void LscpDialogTcp::onCommunicatorSetup( ZQ::DataPostHouse::IDataCommunicatorPtr communicator )
{
	initConnInfo(communicator);
	mComm = communicator;
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(LscpDialogTcp,"onCommunicatorSetup() comes a new connection [%s]"),mConnInfo.c_str() );
}

void LscpDialogTcp::onCommunicatorDestroyed( ZQ::DataPostHouse::IDataCommunicatorPtr communicator )
{
	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(LscpDialogTcp,"onCommunicatorDestroyed() connection destroyed [%s]"),mConnInfo.c_str() );
	mComm = 0;
}

bool LscpDialogTcp::onRead( const int8* buffer , size_t bufSize )
{
	if( gwConfig.sockserver.hexDumpEnabled >= 1 )
	{
		MLOG.hexDump(ZQ::common::Log::L_INFO,buffer,(int)bufSize,mConnInfo.c_str() );
	}

	WritableMessagePtr msg = toMessage(buffer,bufSize);
	if(!msg)
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(LscpDialogTcp,"onRead() %s failed to parse data as a dsmcc message"),mConnInfo.c_str());
		return true;
	}
	RequestImplPtr pReq = new RequestImpl(mEnv,mComm);
	
	MessageImplPtr pNewMsg = MessageImplPtr::dynamicCast( msg );
	pReq->attachMessage( pNewMsg );
	pReq->updateLocalInfo(mLocalIp,mLocalPort);
	pReq->updatePeerInfo(mPeerIp,mPeerPort);

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(LscpDialogTcp,"onRead() got a new request, session[%s], command[%s]"),
		pReq->getSessionId().c_str(), command2Str(msg->getCommand()));
	mGatewayCenter.postRequest(pReq,PHASE_FIXUP_REQUEST);

	return true;
}

void LscpDialogTcp::onWritten( size_t bufSize )
{
}

void LscpDialogTcp::onError( )
{
}

//////////////////////////////////////////////////////////////////////////
///LscpDialogUdp
LscpDialogUdp::LscpDialogUdp( Environment& env, GatewayCenter& center )
:GatewayLscpDialog(env,center)
{
}
LscpDialogUdp::~LscpDialogUdp()
{
}

void LscpDialogUdp::onError( )
{

}

void LscpDialogUdp::onData( const int8* buffer , size_t bufSize , ZQ::DataPostHouse::IDataCommunicatorPtr comm  )
{
	initConnInfo( comm );
	if( gwConfig.sockserver.hexDumpEnabled >= 1 )
	{
		MLOG.hexDump(ZQ::common::Log::L_INFO,buffer,(int)bufSize,mConnInfo.c_str() );
	}

	WritableMessagePtr msg = toMessage(buffer,bufSize);
	if(!msg)
	{
		MLOG(ZQ::common::Log::L_WARNING,CLOGFMT(LscpDialogUdp,"onData() %s failed to parse data as a dsmcc message"),mConnInfo.c_str());
		return ;
	}

	RequestImplPtr pReq = new RequestImpl(mEnv,comm);

	pReq->attachMessage(MessageImplPtr::dynamicCast( msg) );
	pReq->updateLocalInfo(mLocalIp,mLocalPort);
	pReq->updatePeerInfo(mPeerIp,mPeerPort);

	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(LscpDialogUdp,"onData() got a new request, session[%s], command[%s]"),
		pReq->getSessionId().c_str(), command2Str(msg->getCommand()));

	mGatewayCenter.postRequest(pReq,PHASE_FIXUP_REQUEST);

	return ;
}

}}//namespace 

