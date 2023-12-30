#ifndef __CodWebPage_RemoveItem_H__
#define __CodWebPage_RemoveItem_H__

#include "BasePage.h"

namespace CodWebPage
{
	class RemoveItem : public BasePage
	{
	public: 
		RemoveItem(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~RemoveItem();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __CodWebPage_RemoveItem_H__

