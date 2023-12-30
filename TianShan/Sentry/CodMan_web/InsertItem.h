#ifndef __CodWebPage_InsertItem_H__
#define __CodWebPage_InsertItem_H__

#include "BasePage.h"

namespace CodWebPage
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

#endif // __CodWebPage_InsertItem_H__

