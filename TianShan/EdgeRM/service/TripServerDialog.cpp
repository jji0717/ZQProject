#include "TripServerDialog.h"
#include  "ERRPMsg.h"
#include   "TripThreadRequest.h"

 
using namespace ZQ::ERRP;
char* strMsgType[5] = {"OPEN","UPDATE","NOTIFICATION","KEEPALIVE","UNKOWN"};
TripServerDialog::TripServerDialog(ZQ::common::NativeThreadPool& procThPool, ZQ::common::Log& aLog) 
: _log(aLog),mProcThPool(procThPool),mMetaData(*(new ZQ::ERRP::StringMap)),mAttributeValue(*(new ZQ::ERRP::UpdataRequest::AttrbuteValue))
{
}

TripServerDialog::~TripServerDialog(void)
{
	if(&mMetaData)
		delete &mMetaData;
	if(&mAttributeValue)
		delete &mAttributeValue;
}

void TripServerDialog::onCommunicatorSetup( ZQ::DataPostHouse::IDataCommunicatorPtr communicator )
{
	mComm = communicator;
}

void TripServerDialog::onCommunicatorDestroyed( ZQ::DataPostHouse::IDataCommunicatorPtr communicator ) 
{
	mComm = 0;
}

bool TripServerDialog::onRead( const int8* buffer, size_t bufSize )
{
	int64 stampBegin = ZQ::common::TimeUtil::now();// for log
	(_log)(ZQ::common::Log::L_INFO,CLOGFMT(TripServerDialog,"received data size:%d"),bufSize);
	//_log.hexDump(ZQ::common::Log::L_INFO,(void*)buffer,(int)bufSize,"TripServerReceived");

	//printf("This is in TripServerDialog::onRead(), 1\n");
	if(!buffer || bufSize <= 0)
		return true;// no data passed in, reject

	size_t msgProcessed = 0;
	uint16 msgLength = 0;

	uint16 size = 0;
	size_t bytesProcessed = 0;

	while(msgProcessed < bufSize)
	{
		//splitting the combined messages,only effect update message
		memcpy(&msgLength,buffer,sizeof(msgLength));
		msgLength = ntohs(msgLength);
		ZQ::ERRP::ERRPMsg::Ptr pMsg = ZQ::ERRP::ERRPMsg::parseMessage((uint8*)buffer, msgLength, bytesProcessed);

		buffer += msgLength;
		msgProcessed += msgLength;

		if(!pMsg)
		{
			(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(TripServerDialog, "parse message failed"));
			
			return false;
		}
		else 
		{
			uint8 messageType = pMsg->getMessageType();
			switch(messageType)
			{
			case MsgType_UPDATE:
				{
					ZQ::ERRP::ERRPMsg * pTempMsg = pMsg.get();  // 从智能指针里提取出真实结构体的指针。
					ZQ::ERRP::UpdataRequest* pTemp = dynamic_cast<ZQ::ERRP::UpdataRequest*>(pTempMsg);
					pMsg->toMetaData(mMetaData);
					if (NULL != pTemp)
					{
						mAttributeValue = pTemp->getRouteAttributes();// 解析好的message 里面的 Attributes 提取出来。
					}
					break;
				}
			case MsgType_OPEN:
				{
					uint32 size = pMsg->toMetaData(mMetaData);
					if(size == 0)
					{
						(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(TripServerDialog, "toMetaData failed"));
						return 0;
					}
					ZQ::ERRP::StringMap::iterator iter = mMetaData.find(ERRPMD_OpenHoldTime);
					if(iter == mMetaData.end())
						return 0;
					mTimeOut = atoi(iter->second.c_str())*1000;//second
					if(mTimeOut/3 < 5000)
						mTimeOut = 5000;
					//printf("timeOut:%d\n",mTimeOut);
					mLastUpdate = ZQ::common::TimeUtil::now();

					break;
				}
			case MsgType_KEEPALIVE:
				{
					uint32 size = pMsg->toMetaData(mMetaData);
					if(size == 0)
					{
						(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(TripServerDialog, "toMetaData failed"));
						return 0;
					}
					mLastUpdate = ZQ::common::TimeUtil::now();
					break;
				}
			case MsgType_NOTIFICATION:
				{
					mComm->close();
				}
			default:
				{
					(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(TripServerDialog, "Unknow message type"));
					//create notification metadata
					MAPSET(ZQ::ERRP::StringMap,mMetaData,ERRPMD_MsgType,"3");
					MAPSET(ZQ::ERRP::StringMap,mMetaData,ERRPMD_NoErrorCode,"1");
					MAPSET(ZQ::ERRP::StringMap,mMetaData,ERRPMD_NoErrorSubCode,"2");
					MAPSET(ZQ::ERRP::StringMap,mMetaData,ERRPMD_NoErrorData,"Unknow message type");
				}
			}
		}
		TripThreadRequest*  aTripThreadPtr = new TripThreadRequest(mComm, mProcThPool, mMetaData, mAttributeValue,_log);
		aTripThreadPtr->start();
		int64 processTime = ZQ::common::TimeUtil::now() - stampBegin;
		glog(ZQ::common::Log::L_INFO,CLOGFMT(TripServerDialog,"Session[%lld] receive message in %lldms"),mComm->getCommunicatorId(),processTime);
	}

	return true;
}

void TripServerDialog::onWritten( size_t bufSize )
{
	// 这个由于windows和linux对于发送事件的解释是完全不一样的。
	// 而且我们基本不用异步发送，所以这个函数一般是不管的
}

void TripServerDialog::onError( )
{
	//通常情况下，这里面不需要有任何实现。当然，可以写一些日志之类的东西
}
