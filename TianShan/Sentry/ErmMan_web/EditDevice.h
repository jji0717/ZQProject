#ifndef __ErmWebPage_EditDevice_H__
#define __ErmWebPage_EditDevice_H__

#include "BasePage.h"

namespace ErmWebPage
{
	class EditDevice : public BasePage
	{
	public: 
		EditDevice(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~EditDevice();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __ErmWebPage_EditDevice_H__
