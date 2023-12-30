#ifndef __BcastWebPage_EditItem_H__
#define __BcastWebPage_EditItem_H__

#include "BasePage.h"

namespace BcastWebPage
{
	class EditItem : public BasePage
	{
	public: 
		EditItem(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~EditItem();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __BcastWebPage_EditItem_H__
