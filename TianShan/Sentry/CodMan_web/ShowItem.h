#ifndef __CodWebPage_ShowItem_H__
#define __CodWebPage_ShowItem_H__

#include "BasePage.h"

namespace CodWebPage
{
	class ShowItem : public BasePage
	{
	public: 
		ShowItem(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~ShowItem();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __CodWebPage_ShowItem_H__

