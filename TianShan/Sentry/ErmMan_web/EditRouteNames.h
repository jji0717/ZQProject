#ifndef __ErmWebPage_EditRouteNames_H__
#define __ErmWebPage_EditRouteNames_H__

#include "BasePage.h"

namespace ErmWebPage
{
	class EditRouteNames : public BasePage
	{
	public: 
		EditRouteNames(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~EditRouteNames();

	protected: 
		virtual bool get();
		virtual bool post();
		bool show(IHttpResponse& responser);
		bool link(IHttpResponse& responser, std::string devName, std::string portId, std::string routeName, std::string& freqs);
		bool unlink(IHttpResponse& responser, std::string devName, std::string portId, const TianShanIce::StrValues& routeNames);
	};
}

#endif // __ErmWebPage_EditServiceGroup_H__

