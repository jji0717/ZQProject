#ifndef __BcastWebPage_RemoveFilterItem_H__
#define __BcastWebPage_RemoveFilterItem_H__

#include "BasePage.h"

namespace BcastWebPage
{
	class RemoveFilterItem : public BasePage
	{
	public: 
		RemoveFilterItem(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~RemoveFilterItem();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __BcastWebPage_RemoveFilterItem_H__