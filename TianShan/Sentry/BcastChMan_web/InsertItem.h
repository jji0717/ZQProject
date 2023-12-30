#ifndef __BcastWebPage_InsertItem_H__
#define __BcastWebPage_InsertItem_H__

#include "BasePage.h"

namespace BcastWebPage
{
	class InsertItem : public BasePage
	{
	public: 
		InsertItem(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~InsertItem();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __BcastWebPage_InsertItem_H__