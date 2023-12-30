#ifndef __BcastWebPage_RemoveItem_H__
#define __BcastWebPage_RemoveItem_H__

#include "BasePage.h"

namespace BcastWebPage
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

#endif // __BcastWebPage_RemoveItem_H__