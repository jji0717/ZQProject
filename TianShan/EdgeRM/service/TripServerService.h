#ifndef __ECHO_SERVICE_HEADER_FILE_H__
#define __ECHO_SERVICE_HEADER_FILE_H__

#include <DataCommunicatorUnite.h>
#include "TripServerDialog.h"

using namespace ZQ::DataPostHouse;

// *************************************************
// TripServerDilalogEnvironment * _gTripServerDialogEnvPtr;
// ************************************************
class DialogFactory : public ZQ::DataPostHouse::IDataDialogFactory
{
public:
 //   TripServerDilalogEnvironment&  mEnv;
	DialogFactory(ZQ::common::NativeThreadPool& procThPool, ZQ::common::Log& aLog)
		: mLog(aLog),mProcThPool(procThPool){}
   
    // *********************For ERRMessage ****************
	/*
     TripServerDilalogEnvironment & GetTripServerDialogEnvironment()
     {
        return  mEnv;
     }
	 */
     // ***************************************************
protected:
	virtual	void					onClose( CommunicatorS& comms );

	virtual IDataDialogPtr			onCreateDataDialog( IDataCommunicatorPtr communicator ) ;

    virtual void					onReleaseDataDialog( ZQ::DataPostHouse::IDataDialogPtr dialog , 
        ZQ::DataPostHouse::IDataCommunicatorPtr communicator ) ;
private:
    
	ZQ::common::NativeThreadPool& mProcThPool;
    ZQ::common::Log& mLog;
};

typedef ZQ::DataPostHouse::ObjectHandle<DialogFactory> DialogFactoryPtr;

class TripSocketServer
{
public:
	TripSocketServer(ZQ::common::Log& aLog,int recvPoolSize ,int procPoolSize);
	virtual ~TripSocketServer(void);
	
	bool	addListener( const std::string& ip, const std::string& port );

	bool	start();

	void	stop();

    // *******************************
    
	/*
   TripServerDilalogEnvironment&  GetNativeThreadPoolPtr()
   {
       return  mEnv;
   }
   */

    // *******************************

private:	
	ZQ::DataPostHouse::DataPostHouseEnv			mDakEnv; // 整个server 过程中只能有一块儿 Env.
	ZQ::DataPostHouse::DataPostDak*				mDak;
	DialogFactoryPtr							mDialogFactory;
	std::vector<ZQ::DataPostHouse::AServerSocketTcpPtr>	mListeners;

    ZQ::common::Log& mLog;
	int	 mRecvPoolSize;
	ZQ::common::NativeThreadPool& mProcThPool;
};

#endif//__ECHO_SERVICE_HEADER_FILE_H__
