#ifndef __TianShanS1_GetParamRequest_H__
#define __TianShanS1_GetParamRequest_H__

#include "RequestHandler.h"

namespace TianShanS1
{
	class FixupGetParam : public FixupRequest
	{
	public: 
		typedef IceUtil::Handle<FixupGetParam> Ptr;

		FixupGetParam(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~FixupGetParam();
		virtual bool process();

	};

	class HandleGetParam : public ContentHandler
	{
	public: 
		typedef IceUtil::Handle<HandleGetParam> Ptr;
		HandleGetParam(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~HandleGetParam();
		virtual bool process();

	protected:
		std::string _returnContentBody;

	};

	class GetParamResponse : public FixupResponse
	{
	public: 
		typedef IceUtil::Handle<GetParamResponse> Ptr;
		GetParamResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse);
		virtual ~GetParamResponse();
		virtual bool process();

	};

} // namespace TianShanS1

#endif // #define __TianShanS1_GetParamRequest_H__

