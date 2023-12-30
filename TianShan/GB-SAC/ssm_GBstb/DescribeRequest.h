#ifndef __TianShanS1_DescribeRequest_H__
#define __TianShanS1_DescribeRequest_H__

#include "RequestHandler.h"

namespace TianShanS1
{
	class FixupDescribe : public FixupRequest
	{
	public: 
		typedef IceUtil::Handle<FixupDescribe> Ptr;
		FixupDescribe(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~FixupDescribe();
		virtual bool process();

	};

	class HandleDescribe : public ContentHandler
	{
	public: 
		typedef IceUtil::Handle<HandleDescribe> Ptr;
		HandleDescribe(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse);
		virtual ~HandleDescribe();
		virtual bool process();

	protected:
		std::string _returnContentBody;

	};

	class DescribeResponse : public FixupResponse
	{
	public: 
		typedef IceUtil::Handle<DescribeResponse> Ptr;
		DescribeResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse);
		virtual ~DescribeResponse();
		virtual bool process();

	};

} // namespace TianShanS1

#endif // #define __TianShanS1_DescribeRequest_H__

