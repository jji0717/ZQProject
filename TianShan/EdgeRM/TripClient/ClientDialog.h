#ifndef __CLIENT_DIALOG_H__
#define __CLIENT_DIALOG_H__

#include <DataCommunicatorUnite.h>
#include  "common_define.h"
#include  "NativeThreadPool.h"
#include  "ERRPMsgDefine.h"
#include  <sys/stat.h>

// *******************
//#include "Timer.h"
#include <iostream>
#include "TimeUtil.h"
#include "NativeThread.h"
#include <vector>
#include "Locks.h"
#include <SystemUtils.h>
#include "NativeThread.h"
#include "NativeThreadPool.h"
#include <string>

#define	 LASTMSG_OPEN		"open"
#define  LASTMSG_KEEPALIVE	"keepAlive"
#define	 LASTMSG_NULL		""

class ClientDialog;

//send heartbeat to each dialog in mClientDialogPtrs
class ClientHeartbeat : public ZQ::common::NativeThread,public  ZQ::DataPostHouse::SharedObject
{
public:
	friend class ClientDialog;

	ClientHeartbeat(){ mQuit = false;mBHeartStarted = false; }
	virtual ~ClientHeartbeat(){}
	typedef std::list<ClientDialog*>  ClientDialogPtrs;

public:
	void add(ClientDialog* clientDialog);
	void remove(ClientDialog* clientDialog);
	void stop();

protected:
	virtual int run(void);

private:
	ZQ::common::Mutex      mLockSessList;
	bool                   mQuit;
	SYS::SingleObject      mWakeUpEvent;
	ClientDialogPtrs       mClientDialogPtrs;
	bool			   	   mBHeartStarted;//ensure that the heart beat thread only started one time
};
typedef  ZQ::DataPostHouse::ObjectHandle<ClientHeartbeat>   ClientHeartbeatPtr;

//class ClientDialog
//clientDialogFactory create this class instance,
//onRead() will be invoked when data arrived
class ClientDialog : public ZQ::DataPostHouse::IDataDialog
{
public:
	ClientDialog(ZQ::common::NativeThreadPool& thPool,ZQ::common::Log& log);
	ClientDialog();

	~ClientDialog(void);

	ZQ::common::Log& mLog;

	void OnTimer();

	void stop();

	static void startHeartbeat();
	static void stopHeartbeat();
	ZQ::DataPostHouse::IDataCommunicatorPtr getCommunicator(){return mComm;}// just for log

protected:
	virtual		void		onCommunicatorSetup( ZQ::DataPostHouse::IDataCommunicatorPtr communicator ) ;

	virtual		void		onCommunicatorDestroyed( ZQ::DataPostHouse::IDataCommunicatorPtr communicator ) ;

	virtual		bool		onRead( const int8* buffer , size_t bufSize ) ;

	virtual		void		onWritten( size_t bufSize ) ;

	virtual		void		onError( ) ;

private:
	ZQ::DataPostHouse::IDataCommunicatorPtr		mComm;
	ZQ::common::NativeThreadPool&				mThPool;
public:
	uint64										mDialogTimeOut;
	uint64										mLastUpdate;
	std::string								    mLastMsgRecved;
	uint64										mHoldTime;
	uint64										mTimeWasBorn;
};

#endif //__CLIENT_DIALOG__