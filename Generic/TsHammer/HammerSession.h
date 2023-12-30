#ifndef __HAMMER_SESSION__
#define __HAMMER_SESSION__

#include "RTSPClient.h"

class Hammer;
class HammerSession : public ZQ::common::RTSPSession {

public:

	typedef ZQ::common::Pointer<HammerSession> Ptr;
	typedef std::map<std::string, std::string> SessionCtx;

	SessionCtx& ctx() {
		return _ctx;
	}

public:

	HammerSession(Hammer& hammer, uint seqIdx, 
				  ZQ::common::Log&, ZQ::common::NativeThreadPool& pool, 
				  const char* streamURL, const char* filePath, int timeout, const char* id, SessionCtx& ctx);

	virtual ~HammerSession();

public:

	virtual void OnResponse_GET_PARAMETER(ZQ::common::RTSPClient& rtspClient, 
									ZQ::common::RTSPRequest::Ptr& pReq, 
									ZQ::common::RTSPMessage::Ptr& pResp, 
									uint resultCode, 
									const char* resultString);

	virtual void OnResponse_SETUP(ZQ::common::RTSPClient& rtspClient, 
								  ZQ::common::RTSPRequest::Ptr& pReq, 
								  ZQ::common::RTSPMessage::Ptr& pResp, 
								  uint resultCode, const char* resultString); 

	virtual void OnResponse_PLAY(ZQ::common::RTSPClient& rtspClient, 
								  ZQ::common::RTSPRequest::Ptr& pReq, 
								  ZQ::common::RTSPMessage::Ptr& pResp, 
								  uint resultCode, const char* resultString);
	
	virtual void OnResponse_TEARDOWN(ZQ::common::RTSPClient& rtspClient, 
								  ZQ::common::RTSPRequest::Ptr& pReq, 
								  ZQ::common::RTSPMessage::Ptr& pResp, 
								  uint resultCode, const char* resultString);

public:

	virtual void OnRequestError(ZQ::common::RTSPClient& rtspClient, 
								ZQ::common::RTSPRequest::Ptr& pReq, 
								RequestError errCode, 
								const char* errDesc=0);


private:

	Hammer& _hammer;
	uint _seqIdx;
	uint _cseq;

	SessionCtx _ctx;
};

class HammerClient : public ZQ::common::RTSPClient{
public:
	HammerClient( Hammer& hammer, ZQ::common::Log& log,  ZQ::common::NativeThreadPool& thrdpool,  ZQ::common::InetHostAddress& bindAddress,
				  const std::string& baseURL, const char* userAgent = NULL,  ZQ::common::Log::loglevel_t verbosityLevel = ZQ::common::Log::L_WARNING,  ZQ::common::tpport_t bindPort=0);
	
//	HammerClient( ZQ::common::Log& log,  ZQ::common::NativeThreadPool& thrdpool,  ZQ::common::InetHostAddress& bindAddress,
//				  const std::string& baseURL, const char* userAgent = NULL,  ZQ::common::Log::loglevel_t verbosityLevel = ZQ::common::Log::L_WARNING,  ZQ::common::tpport_t bindPort=0);
	virtual ~HammerClient(); 

public:
	
	virtual void OnResponse(ZQ::common::RTSPClient& rtspClient,
							ZQ::common::RTSPRequest::Ptr& pReq,
							ZQ::common::RTSPMessage::Ptr& pResp,
							uint resultCode,
							const char* resultString);

	virtual void OnResponse_DESCRIBE(ZQ::common::RTSPClient& rtspClient, 
							ZQ::common::RTSPRequest::Ptr& pReq, 
							ZQ::common::RTSPMessage::Ptr& pResp, 
							uint resultCode,const char* resultString);

	virtual void OnResponse_OPTIONS(ZQ::common::RTSPClient& rtspClient, 
							ZQ::common::RTSPRequest::Ptr& pReq, 
							ZQ::common::RTSPMessage::Ptr& pResp, 
							uint resultCode,const char* resultString);
/*
public:
	virtual void OnRequestError(ZQ::common::RTSPClient& rtspClient, 
								ZQ::common::RTSPRequest::Ptr& pReq, 
								RequestError errCode, 
								const char* errDesc=0);
*/
private:
//	uint _cseq;
	Hammer& _hammer;
//public:
//	uint _seqIdx;

};

#endif

// vim: nu ts=4 sw=4 bg=dark

