#ifndef __CodWebPage_EditItem_H__
#define __CodWebPage_EditItem_H__

#include "BasePage.h"

namespace CodWebPage
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

#endif // __CodWebPage_EditItem_H__

