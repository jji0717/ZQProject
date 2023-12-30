#ifndef __CodWebPage_PushItem_H__
#define __CodWebPage_PushItem_H__

#include "BasePage.h"

namespace CodWebPage
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

#endif // __CodWebPage_PushItem_H__

