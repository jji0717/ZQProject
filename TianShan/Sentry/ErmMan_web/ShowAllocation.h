#ifndef __ErmWebPage_ShowAllocation_H__
#define __ErmWebPage_ShowAllocation_H__

#include "BasePage.h"

namespace ErmWebPage
{
	class ShowAllocation : public BasePage
	{
	public: 
		ShowAllocation(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~ShowAllocation();

	protected: 
		virtual bool get();
		virtual bool post();
		bool showAllocationInfo(IHttpResponse& responser, std::string devName);
		bool showAllocationInfo(IHttpResponse& responser, std::string devName, short port);
		bool showAllocationInfo(IHttpResponse& responser, std::string devName, short port, short chNum);
	private:
		TianShanIce::EdgeResource::AllocationInfos AllInfos;
		unsigned int _sessCount;
		unsigned int _pageCount;
		unsigned int _sessionNumberPerPage;
		unsigned int _linkNumberPerPage;
	};
}

#endif // __ErmWebPage_ShowAllocation_H__