#ifndef __BcastWebPage_PushItem_H__
#define __BcastWebPage_PushItem_H__

#include "BasePage.h"

namespace BcastWebPage
{
	class PushItem : public BasePage
	{
	public: 
		PushItem(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~PushItem();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __BcastWebPage_PushItem_H__