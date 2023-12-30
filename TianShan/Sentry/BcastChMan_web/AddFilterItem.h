#ifndef __BcastWebPage_AddFilterItem_H__
#define __BcastWebPage_AddFilterItem_H__

#include "BasePage.h"

namespace BcastWebPage
{
	class AddFilterItem : public BasePage
	{
	public: 
		AddFilterItem(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~AddFilterItem();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __BcastWebPage_AddFilterItem_H__