/*
#ifndef __ErmWebPage_EditServiceGroup_H__
#define __ErmWebPage_EditServiceGroup_H__

#include "BasePage.h"

namespace ErmWebPage
{
	class EditServiceGroup : public BasePage
	{
	public: 
		EditServiceGroup(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~EditServiceGroup();

	protected: 
		virtual bool get();
		virtual bool post();
		bool show(IHttpResponse& responser);
		bool link(IHttpResponse& responser, std::string devName, std::string portId, std::string routeName, std::string& freqs);
		bool unlink(IHttpResponse& responser, std::string devName, std::string portId, const TianShanIce::StrValues& routeNames);
	};
}

#endif // __ErmWebPage_EditServiceGroup_H__

*/