#include "C2UdpResponseHandler.h"
#include <InetAddr.h>
#include <http.h>


namespace C2Streamer {
	C2UdpResponseHandler::C2UdpResponseHandler(std::string ip, unsigned short port, ZQ::common::Log& log)
		:mLog(log)
	{
		mUdpHandler = new C2AsyncUdpHandler(LibAsync::getLoopCenter().getLoop(), mLog);
		assert(NULL != mUdpHandler && "failed to create new udp handler");
		if(!mUdpHandler->peer(ip, port)) {
			mLog.error(CLOGFMT(C2UdpResponseHandler,"failed to set target addr[%s:%hu]"), ip.c_str(), port);
		}
		mLog(ZQ::common::Log::L_DEBUG, CLOGFMT(C2UdpResponseHandler, "C2UdpResponseHandler() successfully set target addr[%s:%u]."), ip.c_str(), port);
	}


	C2UdpResponseHandler::~C2UdpResponseHandler()
	{
		if (mUdpHandler)
			mUdpHandler = NULL;
	}

	bool C2UdpResponseHandler::setLocalAddr(const std::string& ip, unsigned short port/* = 0*/)
	{
		return mUdpHandler->local(ip, port);
	
	}

	int C2UdpResponseHandler::addBodyContent(const char* data, size_t size)
	{
		assert(NULL != mUdpHandler && "failed to create new udp handler");
		//mUdpHandler->recv()
		return mUdpHandler->addBody(data, size);
	}

	bool C2UdpResponseHandler::registerWrite(ZQHttp::IChannelWritable::Ptr cb)
	{
		assert(NULL != mUdpHandler && "failed to create new udp handler");
		return mUdpHandler->registerWrite(cb);
	}

	std::string  C2UdpResponseHandler::lastError() const 
	{ 
		return ""; 
	}
    
    void C2UdpResponseHandler::setCommOption(int opt, int value)
    {
        assert(NULL != mUdpHandler && "failed to create new udp handler");
        if (opt == ZQHttp_OPT_WriteBufSize) //write
        {
            mUdpHandler->setSendBufSize(value);
        }
		else if (opt == ZQHttp_OPT_TTL) //ttl
		{
			mUdpHandler->setTTL(value);
		}
    }


}
