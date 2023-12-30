#ifndef __ErmWebPage_ShowRouteNames_H__
#define __ErmWebPage_ShowRouteNames_H__

#include "BasePage.h"

namespace ErmWebPage
{
	class ShowRouteNames : public BasePage
	{
	public: 
		ShowRouteNames(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~ShowRouteNames();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __ErmWebPage_ShowServiceGroup_H__