#include "stdafx.h"
#include "ClientRequestHandle.h"
#include "ClientSockets.h"

extern char* openMsg;
extern char* updateMsg;
extern char* keepAliveMsg;

extern int  openMsgSize;
extern int updateMsgSize;
extern int keepAliveMsgSize;

ClientRequestHandle::ClientRequestHandle(ClientDialog& clientDialog,ZQ::DataPostHouse::IDataCommunicatorPtr comm,ZQ::common::NativeThreadPool& thPool,
										 const uint8* buf,size_t bufSize,ZQ::common::Log& log,std::string& lastMsgRecved)
										 :mClientDialog(clientDialog),ThreadRequest(thPool),mComm(comm), mLog(log),mLastMsgRecved(lastMsgRecved)
{
	if(buf)
		mBuf = buf;
	if(bufSize)
		mBufSize = bufSize;
}

ClientRequestHandle::~ClientRequestHandle()
{
}

int ClientRequestHandle::run()
{
	uint64 stampNow = ZQ::common::TimeUtil::now();
	size_t byteProcessed = 0;
	/*
	ZQ::ERRP::ERRPMsg::Ptr pMsg = ZQ::Vrep::::parseMessage(mBuf,mBufSize,byteProcessed);
	if(!pMsg)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(ClientRequestHandle,"run():parseMessage error"));
		return 0;
	}
	if(pMsg->getMessageLength()+sizeof(ZQ::ERRP::ERRPMsg::HardHeader) != byteProcessed)
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(ClientRequestHandle,"run():message length wrong"));

	uint8 messageType = pMsg->getMessageType();
	*/
	ZQ::Vrep::VREPHeader hdr;
	if(0 > ZQ::Vrep::parseVREPHeader(hdr,(byte*)mBuf,mBufSize))
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(ClientRequestHandle,"run():parseVREPHeader error"));
		return 0;
	}
	if((size_t)hdr.length != mBufSize)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(ClientRequestHandle,"run(): message length error"));
		return false;
	}

	switch(hdr.type)
	{
	case VREP_OPEN:
		{
			if(mComm->isValid())
			{
				mComm->write(keepAliveMsg,keepAliveMsgSize);
				mLastMsgRecved = LASTMSG_OPEN;
			}

			glog(ZQ::common::Log::L_INFO,CLOGFMT(ClientRequestHandle,"Session[%lld] recieved confirm message of open request,send keepalive message"),mComm->getCommunicatorId());
			break;
		}
	case VREP_KEEPALIVE:
		{
			if(mLastMsgRecved == LASTMSG_OPEN)//this message is confirmed to open request
			{
				if(mComm->isValid())
				{
					mComm->write(updateMsg,updateMsgSize);
					mLastMsgRecved = LASTMSG_KEEPALIVE;
				}
				glog(ZQ::common::Log::L_INFO,CLOGFMT(ClientRequestHandle,"Session[%lld] recieved confirm message of open,send update message"),mComm->getCommunicatorId());
			}
			else if(mLastMsgRecved == LASTMSG_KEEPALIVE)//this message is confirmed to keepAlive,and start heart beat here
			{
				mLastMsgRecved = LASTMSG_NULL;//set mLastMsgRecved to 0,the sending flow is finished
				//start send heart beat
				ClientDialog::startHeartbeat();

				glog(ZQ::common::Log::L_INFO,CLOGFMT(ClientRequestHandle,"Session[%lld] recieved confirm message of update."),mComm->getCommunicatorId());
			}

			break;
		}
	case VREP_NOTIFICATION:
		{
			// add error processed code
			mClientDialog.stop();
			break;
		}
	default:
		{
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(ClientRequestHandle,"Session[%lld] missed message type %d"),mComm->getCommunicatorId(),hdr.type);
			return 0;
		}
	}
	uint64 processedTime = ZQ::common::TimeUtil::now() - stampNow;
	glog(ZQ::common::Log::L_INFO,CLOGFMT(ClientRequestHandle,"Session[%lld] processed in %lldms"),mComm->getCommunicatorId(),processedTime);
	return 0;
}

void ClientRequestHandle::final()
{
    delete   this;
}