#ifndef  __TRIPTHREADREQUEST_SERVER_DIALOG_HEADER_H__
#define  __TRIPTHREADREQUEST_SERVER_DIALOG_HEADER_H__

#include  "NativeThreadPool.h"
#include  "NativeThread.h"
#include  "DataCommunicator.h"
#include  "TripServerDialog.h"
#include  "ERRPMsg.h"
#include  "ERRPMsgDefine.h"


class  TripThreadRequest:  public  ZQ::common:: ThreadRequest,public ZQ::DataPostHouse::SharedObject
{
    friend class SlaveThread;
    friend class NativeThreadPool;

public:
    TripThreadRequest( ZQ::DataPostHouse::IDataCommunicatorPtr aIDataComm, ZQ::common::NativeThreadPool& aPool, 
                       ZQ::ERRP::StringMap & aMetadataRef, 
                       ZQ::ERRP::UpdataRequest::AttrbuteValue & aAttributeValueRef, ZQ::common::Log& aLog);


    virtual ~TripThreadRequest();

protected:


    virtual int run();

    virtual void final();


private:

     //ZQ::DataPostHouse::IDataDialogPtr               mUsrDataDialogPtr;
     ZQ::ERRP::StringMap &                           mMetadataRef;
     ZQ::ERRP::UpdataRequest::AttrbuteValue &        mAttributeValueRef;
     ZQ::common::Log&								 mLog;
	 ZQ::DataPostHouse::IDataCommunicatorPtr		 mIDataCommPtr;
};
///////////////////////////////////////////////////////////////////

typedef  ZQ::DataPostHouse::ObjectHandle<TripThreadRequest> TripThreadRequestPtr;
#endif
