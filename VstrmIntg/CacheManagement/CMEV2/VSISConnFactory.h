#ifndef _VSISCONNFACTORY_H_
#define _VSISCONNFACTORY_H_

#include <map>

#include "DataPostHouseEnv.h"
#include "DataCommunicatorUnite.h"
#include "DataCommunicator.h"

namespace CacheManagement {

class VSISCacheIO;

class VSISConnDialog : public ZQ::DataPostHouse::IDataDialog
{
public:
	VSISConnDialog(uint32 cfgRecvBuffSize);
	virtual ~VSISConnDialog();

public:
	virtual void onCommunicatorSetup(ZQ::DataPostHouse::IDataCommunicatorPtr communicator);
	virtual	void onCommunicatorDestroyed(ZQ::DataPostHouse::IDataCommunicatorPtr communicator);
	virtual	bool onRead(const int8* buffer, size_t bufSize);
	virtual	void onWritten(size_t bufSize);
	virtual	void onError();

protected:
	ZQ::DataPostHouse::AClientSocketTcpPtr	_clientSocket;

	uint32		_cfgRecvBuffSize;

	static uint64   _recvdataNo;
};

class VSISConnFactory: public ZQ::DataPostHouse::IDataDialogFactory
{
public:
	VSISConnFactory();
	virtual ~VSISConnFactory();

//
//	socket connection related parameters and functions
//
protected:
	typedef std::map<ZQ::DataPostHouse::IDataCommunicatorPtr, ZQ::DataPostHouse::IDataDialogPtr> DialogMap;
	DialogMap			_dlgs;
	volatile long		_connectionCount;
	ZQ::common::Mutex	_dlgLock;
	
	uint32				_readBuffSize;

	ZQ::DataPostHouse::DataPostHouseEnv* _postHouseEnv;
	ZQ::DataPostHouse::DataPostDak* _postDak;

	// callbacks from IDataDialogFactory to handle the communication events 
	virtual ZQ::DataPostHouse::IDataDialogPtr onCreateDataDialog(ZQ::DataPostHouse::IDataCommunicatorPtr communicator);
	virtual void onReleaseDataDialog(ZQ::DataPostHouse::IDataDialogPtr dialog, ZQ::DataPostHouse::IDataCommunicatorPtr communicator);
	virtual	void onClose(ZQ::DataPostHouse::IDataDialogFactory::CommunicatorS& comms);

public:
	void setRecvBuffSize(uint32  readBuffSize) { _readBuffSize = readBuffSize; };

};

typedef ZQ::DataPostHouse::ObjectHandle<VSISConnFactory> VSISConnFactoryPtr;

}
#endif
