#ifndef __CLIENT_REQUEST_HANDLE_H__
#define __CLIENT_REQUEST_HANDLE_H__

#include  "NativeThreadPool.h"
#include  "NativeThread.h"
#include  "DataCommunicator.h"
#include  "ClientDialog.h"
#include  "VrepMessage.h"
//process the data come from onread()
class ClientRequestHandle:public ZQ::common::ThreadRequest
{
public:
	ClientRequestHandle(ClientDialog& clientDialog,ZQ::DataPostHouse::IDataCommunicatorPtr comm,ZQ::common::NativeThreadPool& thPool,
								const uint8* buf,size_t bufSize,ZQ::common::Log& log,std::string& lastMsgRecved);
	virtual ~ClientRequestHandle();

protected:
	virtual int run();
	virtual void final();
	

private:
	ClientDialog&							mClientDialog;
	ZQ::DataPostHouse::IDataCommunicatorPtr mComm;
	const uint8*							mBuf;
	size_t									mBufSize;
	ZQ::common::Log							mLog;
	std::string&							mLastMsgRecved;//last received message type,it will decide which message will be sent
};
typedef ZQ::DataPostHouse::ObjectHandle<ClientRequestHandle>	ClientRequestHandlePtr;

#endif //__CLIENT_REQUEST_HANDLE_H__