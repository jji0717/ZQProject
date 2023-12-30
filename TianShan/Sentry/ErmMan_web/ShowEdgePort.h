#ifndef __ErmWebPage_ShowEdgePort_H__
#define __ErmWebPage_ShowEdgePort_H__

#include "BasePage.h"

namespace ErmWebPage
{
	class ShowEdgePort : public BasePage
	{
	public: 
		ShowEdgePort(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~ShowEdgePort();

	protected: 
		virtual bool get();
		virtual bool post();
		bool showPortInfo(IHttpResponse& responser, std::string devName);
		bool showPortByServiceGroup(IHttpResponse& responser, std::string routeName);
		bool show(IHttpResponse& responser);

	private:
		bool bGet;
	};
}

#endif // __ErmWebPage_ShowEdgePort_H__