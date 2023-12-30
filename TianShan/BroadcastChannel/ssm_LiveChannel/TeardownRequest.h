#ifndef __TianShanS1_TeardownRequest_H__
#define __TianShanS1_TeardownRequest_H__

#include "RequestHandler.h"

namespace LiveChannel
{
	class FixupTeardown : public FixupRequest
	{
	public: 
		typedef IceUtil::Handle<FixupTeardown> Ptr;
		FixupTeardown(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~FixupTeardown();
		virtual bool process();

	};

	class HandleTeardown : public ContentHandler
	{
	public: 
		typedef IceUtil::Handle<HandleTeardown> Ptr;
		HandleTeardown(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~HandleTeardown();
		virtual bool process();

	};

	class TeardownResponse : public FixupResponse
	{
	public: 
		typedef IceUtil::Handle<TeardownResponse> Ptr;
		TeardownResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse);
		virtual ~TeardownResponse();
		virtual bool process();

	};

}

#endif // define __TianShanS1_TeardownRequest_H__

