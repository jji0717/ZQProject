#include "C2AsyncUdpHandler.h"

namespace C2Streamer {
	C2AsyncUdpHandler::C2AsyncUdpHandler(LibAsync::EventLoop& loop, ZQ::common::Log& log)
		: UDPSocket(loop), Socket(loop), mWritableCB(NULL), mLog(log)
	{

	}


	C2AsyncUdpHandler::~C2AsyncUdpHandler()
	{
		if (NULL != mWritableCB )
			mWritableCB = NULL;
	}

	bool C2AsyncUdpHandler::setPeerAddr(std::string& ip, unsigned short port)
	{
		return peer(ip, port);
	}

	bool C2AsyncUdpHandler::setLocalAddr(std::string&ip, unsigned short port)
	{
		return local(ip, port);
	}

	bool C2AsyncUdpHandler::registerWrite(ZQHttp::IChannelWritable::Ptr cb)
	{
		mWritableCB = cb;
		return Socket::registerWrite();
	}

	bool C2AsyncUdpHandler::addContent(const char* data, size_t len)
	{
		//mLog(ZQ::common::Log::L_DEBUG, CLOGFMT(C2AsyncUdpHandler, "addContent() try to send data[%d] to [%s:%u]."), len, mPeerAddr.c_str(), mPeerPort);
		LibAsync::AsyncBuffer sendBuf;
		sendBuf.base = (char*) data;
		sendBuf.len = len;
		return sendto(sendBuf);
	}

	int C2AsyncUdpHandler::addBody(const  char* data, size_t len)
	{
		//mLog(ZQ::common::Log::L_DEBUG, CLOGFMT(C2AsyncUdpHandler, "addBody() try to send data[%d] to [%s:%u]."), len, mPeerAddr.c_str(), mPeerPort);
		LibAsync::AsyncBuffer sendBuf;
		sendBuf.base = (char*) data;
		sendBuf.len = len;
		return sendDirect(sendBuf);
	}

	void C2AsyncUdpHandler::onWritable()
	{
		mWritableCB->onWritable();
		return;
	}

	void C2AsyncUdpHandler::onSocketRecved(size_t size, std::string ip, unsigned short port)
	{
		mLog(ZQ::common::Log::L_DEBUG, CLOGFMT(C2AsyncUdpHandler, "onSocketRecved() recv data[%d] from [%s:%u]."), size, ip.c_str(), port);
		return;
	}

	void C2AsyncUdpHandler::onSocketSent(size_t size)
	{
		mLog(ZQ::common::Log::L_DEBUG, CLOGFMT(C2AsyncUdpHandler, "onSocketSent() send size[%d]."), size);
		return;
	}

	void C2AsyncUdpHandler::onSocketError(int err)
	{
		mLog(ZQ::common::Log::L_DEBUG, CLOGFMT(C2AsyncUdpHandler, "onSocketerror() with error[%d]."), err);
		Socket::close();
		return;
	}
}//namespace C2Streamer{
