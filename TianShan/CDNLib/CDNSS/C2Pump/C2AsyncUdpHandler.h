#pragma once
#ifndef  _C2ASYNC_UDP_HANDLER_H
#define  _C2ASYNC_UDP_HANDLER_H

#include <udpsocket.h>
#include <Log.h>
#include <Pointer.h>
#include <HttpEngineInterface.h>

namespace C2Streamer{
class C2AsyncUdpHandler : public LibAsync::UDPSocket
{
public:
	C2AsyncUdpHandler(LibAsync::EventLoop& loop, ZQ::common::Log& log);
	
	~C2AsyncUdpHandler();
	typedef ZQ::common::Pointer<C2AsyncUdpHandler> Ptr;

	bool setPeerAddr(std::string& ip, unsigned short port);
	bool setLocalAddr(std::string& ip, unsigned short port);

	virtual bool addContent(const char* data, size_t len);

	/// new method addBody does the same thing as addContent
	/// but here's some different between them
	/// 1. addBody do not guarantee the whole data will be transfered. if not, error code retuned
	virtual int addBody(const char* data, size_t len);
	/// register an event for writing data
	/// return false if failed to register
	virtual bool registerWrite(ZQHttp::IChannelWritable::Ptr cb);
protected:

	virtual void onWritable();
	virtual	void onSocketRecved(size_t size, std::string ip, unsigned short port);
	virtual	void	onSocketError(int err);
	virtual void	onSocketSent(size_t size);
private:
	ZQHttp::IChannelWritable::Ptr 			mWritableCB;
	ZQ::common::Log&                       mLog;
};

}//namespace C2Streamer
#endif // ! _C2ASYNC_UDP_HANDLER_H
