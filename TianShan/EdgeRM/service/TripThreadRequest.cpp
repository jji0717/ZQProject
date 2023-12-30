#include  "TripThreadRequest.h"
//#include <WinBase.h>

TripThreadRequest::TripThreadRequest(ZQ::DataPostHouse::IDataCommunicatorPtr aIDataComm,ZQ::common::NativeThreadPool& aPool, ZQ::ERRP::StringMap & aMetadataRef, 
										ZQ::ERRP::UpdataRequest::AttrbuteValue & aAttributeValueRef, ZQ::common::Log& aLog)
:ZQ::common::ThreadRequest(aPool),mMetadataRef(aMetadataRef),mAttributeValueRef(aAttributeValueRef),mLog(aLog),mIDataCommPtr(aIDataComm)
{
}

TripThreadRequest::~TripThreadRequest()
{

}

int TripThreadRequest::run() 
{   
    //printf("This is in TripThreadRequest::run()!\n");
	int64 stampBegin = ZQ::common::TimeUtil::now();
	ZQ::DataPostHouse::IDataCommunicatorPtr		 mIDataCommPtrTemp = mIDataCommPtr;
    ZQ::ERRP::ERRPMsg::HardHeader head = {0};//  零初始化一个消息头结构体。
    ZQ::ERRP::ERRPMsg::Ptr pToMsg = NULL;    //  指向封装回复消息的智能指针。
    uint8 RetWrite = 0;
    uint8 buf[5120] = "";
    memset(buf,0,sizeof(buf));
    int  size = 0;
    int  SendSize = 0;
    ZQ::ERRP::StringMap metadata;
    std::string messagetype;// record the message type, just for log
    ZQ::ERRP::StringMap::iterator itorMd = mMetadataRef.find(ERRPMD_MsgType);
    if(itorMd == mMetadataRef.end())
    {
		(mLog)(ZQ::common::Log::L_ERROR,CLOGFMT(TripThreadRequest,"did not found message type in metaDatas"));
        return 0;
    }

    switch(atoi(itorMd->second.c_str()))
    {
    case ZQ::ERRP::MsgType_OPEN:
		{
			ZQ::ERRP::StringMap::iterator iter;

			
			iter = mMetadataRef.find(ERRPMD_OpenVersion);
			if(iter == mMetadataRef.end())
				return 0;
			std::string strOpenVersion = iter->second;
			
			iter = mMetadataRef.find(ERRPMD_OpenReserved);
			if(iter == mMetadataRef.end())
				return 0 ;
			std::string strOpenReserved = iter->second;

			iter = mMetadataRef.find(ERRPMD_OpenAdddressDomain);
			if (iter == mMetadataRef.end())
				return 0;
			std::string strOpenAddr = iter->second;
			std::string ip,port;
			mIDataCommPtr->getLocalAddress(ip,port);

			MAPSET(ZQ::ERRP::StringMap,metadata,ERRPMD_MsgType,"1");
			MAPSET(ZQ::ERRP::StringMap,metadata,ERRPMD_OpenVersion,strOpenVersion.c_str());
			MAPSET(ZQ::ERRP::StringMap,metadata,ERRPMD_OpenReserved,strOpenReserved.c_str());
			MAPSET(ZQ::ERRP::StringMap,metadata,ERRPMD_OpenHoldTime,"0");
			MAPSET(ZQ::ERRP::StringMap,metadata,ERRPMD_OpenAdddressDomain,strOpenAddr.c_str());
			MAPSET(ZQ::ERRP::StringMap,metadata,ERRPMD_OpenErrpIdentifier,ip.c_str());// 构建 Open Message meta.

			pToMsg = new ZQ::ERRP::OpenRequest(head);//  new 一个 reply open message head.
			messagetype = "OPEN";
			pToMsg->readMetaData(metadata);
			size = pToMsg->toMessage(buf,sizeof(buf));

			(mLog)(ZQ::common::Log::L_INFO, CLOGFMT(TripServerDialog, "Session[%lld] [Received:%s] Send Response Message: OPEN (%d)"),mIDataCommPtr->getCommunicatorId(),messagetype.c_str(),size);
			break;
		}
    case ZQ::ERRP::MsgType_UPDATE:
		{
			/// send keepalive message
			MAPSET(ZQ::ERRP::StringMap,metadata,ERRPMD_MsgType,"4");
			messagetype = "UPDATE";
			pToMsg = new ZQ::ERRP::KeepAliveRequest(head);
			pToMsg->readMetaData(metadata);
			size = pToMsg->toMessage(buf,sizeof(buf));

			(mLog)(ZQ::common::Log::L_INFO, CLOGFMT(TripServerDialog, "Session[%lld] [Received:%s] Send Response Message: KEEPALIVE (%d)"),mIDataCommPtr->getCommunicatorId(),messagetype.c_str(),size);
			break;
		}

    case ZQ::ERRP::MsgType_KEEPALIVE:
		{
			/// send keepalive message
			MAPSET(ZQ::ERRP::StringMap,metadata,ERRPMD_MsgType,"4");
			messagetype = "KEEPALIVE";
			pToMsg = new ZQ::ERRP::KeepAliveRequest(head);
			pToMsg->readMetaData(metadata);
			size = pToMsg->toMessage(buf,sizeof(buf));

			(mLog)(ZQ::common::Log::L_INFO, CLOGFMT(TripServerDialog, "Session[%lld] [Received:%s] Send Response Message: KEEPALIVE (%d)"), mIDataCommPtr->getCommunicatorId(),messagetype.c_str(),size);
			break;
		}

    case ZQ::ERRP::MsgType_NOTIFICATION:
		{
			// close the socket
			// ...
			mIDataCommPtr->close();
			messagetype = "NOTIFICATION";
			glog(ZQ::common::Log::L_NOTICE,CLOGFMT(TripServerDialog,"Session[%lld] [Received:%s] closed."),mIDataCommPtr->getCommunicatorId(),messagetype.c_str());

			return 1;
		}

    default:
		{
			//send a unknow type notification message
			glog(ZQ::common::Log::L_ERROR,CLOGFMT(TripServerDialog,"Session[%lld] received unknow message type"),mIDataCommPtr->getCommunicatorId());

			return 0;
		}

    }
	if(!size)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(TripThreadRequest,"to message failed"));
		return 0;
	}
    RetWrite = mIDataCommPtrTemp->write((int8 *)buf, size);
    if(RetWrite != size)
    {
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(TripThreadRequest,"write message failed"));
		return 0;
    }
    //printf("This is in TripThreadRequest::run(),after temp->write(), *(buf+2) = %u,   MessageSize = %d\n",*(buf+2),RetWrite);
    //    temp->write("Message process ok!\n", 20);// Just for telnet can echo character .

	int64 processedTime = ZQ::common::TimeUtil::now() - stampBegin;
	//printf("processedTime:%lldms\n",processedTime);
	glog(ZQ::common::Log::L_INFO,CLOGFMT(TripThreadRequest,"Session[%lld] send message in %lldms"),mIDataCommPtr->getCommunicatorId(),processedTime);

    return 1;	
}

void   TripThreadRequest::final() 
{
	if(&mMetadataRef)
		delete &mMetadataRef;
	if(&mAttributeValueRef)
		delete &mAttributeValueRef;
    delete   this;
}
