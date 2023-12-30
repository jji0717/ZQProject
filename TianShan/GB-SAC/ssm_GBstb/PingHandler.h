#ifndef __TianShanS1_PingRequest_H__
#define __TianShanS1_PingRequest_H__

#include "./RequestHandler.h"

namespace TianShanS1
{
	
class FixupPing : public FixupRequest
{
public: 
	typedef IceUtil::Handle<FixupPing> Ptr;
	FixupPing(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		:FixupRequest(env, pSite, pReq, pResponse)
	{
	}
	virtual ~FixupPing()
	{
	}
	virtual bool process() {return true;}
};

class HandlePing : public ContentHandler
{
public: 
	typedef IceUtil::Handle<HandlePing> Ptr;
	HandlePing(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: ContentHandler(env, pSite, pReq, pResponse)
	{
	}
	virtual ~HandlePing()
	{
	}
	virtual bool process();	
};

class PingResponse : public FixupResponse
{
public: 
	typedef IceUtil::Handle<PingResponse> Ptr;
	PingResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse)
		:FixupResponse(env,pSite,pReq,pResponse)
	{
	}
	virtual ~PingResponse()
	{		
	}
	virtual bool process() {return true;}

};

} // namespace TianShanS1

#endif // #define __TianShanS1_PingRequest_H__

