#ifndef __TianShanS1_OptionRequest_H__
#define __TianShanS1_OptionRequest_H__

#include "RequestHandler.h"

namespace TianShanS1
{
	class FixupOption : public FixupRequest
	{
	public: 
		typedef IceUtil::Handle<FixupOption> Ptr;
		FixupOption(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~FixupOption();
		virtual bool process();

	};

	class HandleOption : public ContentHandler
	{
	public: 
		typedef IceUtil::Handle<HandleOption> Ptr;
		HandleOption(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~HandleOption();
		virtual bool process();

	protected:
		std::string _returnContentBody;

	};

	class OptionResponse : public FixupResponse
	{
	public: 
		typedef IceUtil::Handle<OptionResponse> Ptr;
		OptionResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse);
		virtual ~OptionResponse();
		virtual bool process();

	};

} // namespace TianShanS1

#endif // #define __TianShanS1_OptionRequest_H__

