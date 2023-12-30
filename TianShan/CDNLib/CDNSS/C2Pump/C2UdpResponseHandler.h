#pragma once
#ifndef _C2UDP_RESPONSE_HANDLER_H
#define _C2UDP_RESPONSE_HANDLER_H

#include <C2StreamerLib.h>
#include <Log.h>
#include <C2AsyncUdpHandler.h>

namespace C2Streamer {
	class C2UdpResponseHandler : public C2ResponseHandler
	{
	public:
		C2UdpResponseHandler(std::string ip, unsigned short port, ZQ::common::Log& log);
		virtual ~C2UdpResponseHandler();
	public:
		virtual int		addBodyContent(const char* data, size_t size);
		virtual bool	registerWrite(ZQHttp::IChannelWritable::Ptr cb);
		virtual std::string  lastError() const;
		virtual LibAsync::EventLoop*    getLoop() const { return &(mUdpHandler->getLoop());}
		virtual bool setLocalAddr(const std::string& ip, unsigned short port = 0);
		virtual bool	complete() { // take it as success, but how to really close a udp socket ?
			return true;
		}
		virtual void   setCommOption(int opt, int value);

	protected:
		ZQ::common::Log&           mLog;
		C2AsyncUdpHandler::Ptr     mUdpHandler;
		LibAsync::AsyncBuffer      mRecvBuffer;
	};
}
#endif
