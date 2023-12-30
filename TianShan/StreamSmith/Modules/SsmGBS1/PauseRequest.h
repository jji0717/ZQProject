#ifndef __TianShanS1_PauseRequest_H__
#define __TianShanS1_PauseRequest_H__

#include "RequestHandler.h"

namespace TianShanS1
{
	class FixupPause : public FixupRequest
	{
	public: 
		typedef IceUtil::Handle<FixupPause> Ptr;
		FixupPause(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~FixupPause();
		virtual bool process();

	};

	class HandlePause : public ContentHandler
	{
	public: 
		typedef IceUtil::Handle<HandlePause> Ptr;
		HandlePause(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~HandlePause();
		virtual bool process();		
	protected:
		friend class pauseExAsync;
		std::string _returnScale;
		std::string _returnRange;

	};

	class PauseResponse : public FixupResponse
	{
	public: 
		typedef IceUtil::Handle<PauseResponse> Ptr;
		PauseResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse);
		virtual ~PauseResponse();
		virtual bool process();

	};

} // namespace TianShanS1

#endif // #define __TianShanS1_PauseRequest_H__

