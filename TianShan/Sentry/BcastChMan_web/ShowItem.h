#ifndef __BcastWebPage_ShowItem_H__
#define __BcastWebPage_ShowItem_H__

#include "BasePage.h"

namespace BcastWebPage
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

#endif // __BcastWebPage_ShowItem_H__