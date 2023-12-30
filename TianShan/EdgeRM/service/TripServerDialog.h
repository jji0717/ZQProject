#ifndef __ECHO_SERVER_DIALOG_HEADER_H__
#define __ECHO_SERVER_DIALOG_HEADER_H__

#include <DataCommunicatorUnite.h>
#include  "common_define.h"
#include  "NativeThreadPool.h"
#include "ERRPMsg.h"
// ****************************************
/*
class  TripServerDilalogEnvironment : public ZQ::DataPostHouse::SharedObject
{
public:
    TripServerDilalogEnvironment(ZQ::common::NativeThreadPool & aNativeThreadPoolRef):SharedObject( ),mNativeThreadPoolRef(aNativeThreadPoolRef){}
    virtual ~TripServerDilalogEnvironment(){}
    
    ZQ::common::NativeThreadPool & mNativeThreadPoolRef;
};

typedef  ZQ::DataPostHouse::ObjectHandle<TripServerDilalogEnvironment> TripServerDilalogEnvironmentPtr;
*/

// *********************************************************************
class TripServerDialog : public ZQ::DataPostHouse::IDataDialog
{
public:
	TripServerDialog(ZQ::common::NativeThreadPool& procThPool, ZQ::common::Log& aLog);
	virtual ~TripServerDialog(void);
    ZQ::common::Log& _log;
  // ***************************************************
    virtual ZQ::DataPostHouse::IDataCommunicatorPtr  getDataCommunicatorPtr()
    {
        return mComm;
    }


   // *************************************************
protected:

	virtual		void		onCommunicatorSetup( ZQ::DataPostHouse::IDataCommunicatorPtr communicator ) ;

	virtual		void		onCommunicatorDestroyed( ZQ::DataPostHouse::IDataCommunicatorPtr communicator ) ;

	virtual		bool		onRead( const int8* buffer , size_t bufSize ) ;

	virtual		void		onWritten( size_t bufSize ) ;

	virtual		void		onError( ) ;

    // ******************************************************************
	/*
    virtual    TripServerDilalogEnvironment& GetTripServerDilalogEnvironment()
    {
        return mEnv;
    }

	*/
private:

	std::string		mReservedBuffer;
	ZQ::DataPostHouse::IDataCommunicatorPtr mComm;

	ZQ::ERRP::StringMap& mMetaData;
	ZQ::ERRP::UpdataRequest::AttrbuteValue& mAttributeValue;
	uint64		mTimeOut;//millisecond
	uint64		mLastUpdate;

// *********************************
	ZQ::common::NativeThreadPool& mProcThPool;
// *********************************
};


#endif//__ECHO_SERVER_DIALOG_HEADER_H__
