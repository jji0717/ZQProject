#ifndef __zq_dsmcc_gateway_data_dialog_header_file_h__
#define __zq_dsmcc_gateway_data_dialog_header_file_h__

#include <DataCommunicatorUnite.h>
#include <Locks.h>
#include "requestimpl.h"

namespace ZQ{ namespace CLIENTREQUEST{

class Environment;
class GatewayCenter;

class GateWayDialogBase
{
public:	
	GateWayDialogBase( Environment& env, GatewayCenter& center );
	virtual ~GateWayDialogBase(){}

	void						initConnInfo( ZQ::DataPostHouse::IDataCommunicatorPtr communicator );

	virtual WritableMessagePtr	toMessage( const char* buf , size_t& size ) = 0;

	virtual bool				fromMessage( WritableMessagePtr msg, char* buf , size_t& size ) = 0;

protected:
	Environment&		mEnv;
	GatewayCenter&		mGatewayCenter;
	std::string			mConnInfo;
	std::string			mBufReserved;
	std::string			mLocalIp;
	std::string			mLocalPort;
	std::string			mPeerIp;
	std::string			mPeerPort;
};

class GatewayDsmccDialog : public GateWayDialogBase
{
public:
	GatewayDsmccDialog(Environment& env,GatewayCenter& center);

	virtual WritableMessagePtr	toMessage( const char* buf , size_t& size );

	virtual bool				fromMessage( WritableMessagePtr msg , char* buf , size_t& size );
	
};

class GatewayLscpDialog : public GateWayDialogBase
{
public:
	GatewayLscpDialog(Environment& env,GatewayCenter& center);

	virtual WritableMessagePtr	toMessage( const char* buf , size_t& size );

	virtual bool				fromMessage( WritableMessagePtr msg , char* buf , size_t& size );
};


class DsmccDialogTcp : public GatewayDsmccDialog, public ZQ::DataPostHouse::IDataDialog
{
public:	
	DsmccDialogTcp( Environment& env, GatewayCenter& center);
	virtual ~DsmccDialogTcp();

protected:	
	virtual		void		onCommunicatorSetup( ZQ::DataPostHouse::IDataCommunicatorPtr communicator );

	virtual		void		onCommunicatorDestroyed( ZQ::DataPostHouse::IDataCommunicatorPtr communicator );

	virtual		bool		onRead( const int8* buffer , size_t bufSize );

	virtual		void		onWritten( size_t bufSize );

	virtual		void		onError( );
private:
	ZQ::DataPostHouse::IDataCommunicatorPtr mComm;
};
typedef ZQ::DataPostHouse::ObjectHandle<DsmccDialogTcp> DsmccDialogTcpPtr;

class DsmccDialogUdp : public GatewayDsmccDialog, public ZQ::DataPostHouse::IDgramDialog
{
public:
	DsmccDialogUdp( Environment& env ,GatewayCenter& center);
	virtual ~DsmccDialogUdp();
	
protected:
	virtual		void		onCommunicatorSetup( ZQ::DataPostHouse::IDataCommunicatorPtr communicator ){;}

	virtual		void		onCommunicatorDestroyed( ZQ::DataPostHouse::IDataCommunicatorPtr communicator ){;}

	virtual		bool		onRead( const int8* buffer , size_t bufSize ){ return true;}

	virtual		void		onWritten( size_t bufSize ){;}

	virtual		void		onError( );

	virtual		void		onData( const int8* buffer , size_t bufSize , ZQ::DataPostHouse::IDataCommunicatorPtr comm  );
};
typedef ZQ::DataPostHouse::ObjectHandle<DsmccDialogUdp> DsmccDialogUdpPtr;

class LscpDialogTcp : public GatewayLscpDialog, public ZQ::DataPostHouse::IDataDialog
{
public:
	LscpDialogTcp( Environment& env, GatewayCenter& center );
	virtual ~LscpDialogTcp();

protected:	
	virtual		void		onCommunicatorSetup( ZQ::DataPostHouse::IDataCommunicatorPtr communicator );

	virtual		void		onCommunicatorDestroyed( ZQ::DataPostHouse::IDataCommunicatorPtr communicator );

	virtual		bool		onRead( const int8* buffer , size_t bufSize );

	virtual		void		onWritten( size_t bufSize );

	virtual		void		onError( );
private:
	ZQ::DataPostHouse::IDataCommunicatorPtr	mComm;
};
typedef ZQ::DataPostHouse::ObjectHandle<LscpDialogTcp> LscpDialogTcpPtr;

class LscpDialogUdp : public GatewayLscpDialog, public ZQ::DataPostHouse::IDgramDialog
{
public:
	LscpDialogUdp( Environment& env, GatewayCenter& center );
	virtual ~LscpDialogUdp( );

protected:
	virtual		void		onCommunicatorSetup( ZQ::DataPostHouse::IDataCommunicatorPtr communicator ){;}

	virtual		void		onCommunicatorDestroyed( ZQ::DataPostHouse::IDataCommunicatorPtr communicator ){;}

	virtual		bool		onRead( const int8* buffer , size_t bufSize ){ return true;}

	virtual		void		onWritten( size_t bufSize ){;}

	virtual		void		onError( );

	virtual		void		onData( const int8* buffer , size_t bufSize , ZQ::DataPostHouse::IDataCommunicatorPtr comm  );

};
typedef ZQ::DataPostHouse::ObjectHandle<LscpDialogUdp> LscpDialogUdpPtr;

class DialogUserData : public ZQ::DataPostHouse::SharedObject
{
public:
	DialogUserData(){}
	std::string		type;
	std::string		protocol;
};

typedef ZQ::DataPostHouse::ObjectHandle<DialogUserData> DialogUserDataPtr;


class GatewayDialogFactory : public ZQ::DataPostHouse::IDataDialogFactory , public ZQ::common::NativeThread
{
public:
	GatewayDialogFactory(Environment& env , GatewayCenter& center);
	virtual ~GatewayDialogFactory();

	ZQ::DataPostHouse::IDataCommunicatorPtr			findCommunicator( int64 id ) const;

	void											stop();
protected:

	virtual	void					onClose( CommunicatorS& comms );

	virtual ZQ::DataPostHouse::IDataDialogPtr		onCreateDataDialog( ZQ::DataPostHouse::IDataCommunicatorPtr communicator );

	virtual void					onReleaseDataDialog( ZQ::DataPostHouse::IDataDialogPtr dialog , ZQ::DataPostHouse::IDataCommunicatorPtr communicator );

	virtual int						run();

private:

	Environment&		mEnv;
	GatewayCenter&		mGatewayCenter;
	ZQ::common::Mutex	mLocker;
	std::map<int64 , ZQ::DataPostHouse::IDataCommunicatorPtr> mCommsMap;
	ZQ::common::Semaphore	mSem;
	bool				mbQuit;
};
typedef ZQ::DataPostHouse::ObjectHandle<GatewayDialogFactory> GatewayDialogFactoryPtr;

}}//namespace ZQ::DSMCC

#endif//__zq_dsmcc_gateway_data_dialog_header_file_h__

