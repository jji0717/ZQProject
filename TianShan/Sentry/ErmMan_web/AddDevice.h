#ifndef __ErmWebPage_AddDevice_H__
#define __ErmWebPage_AddDevice_H__

#include "BasePage.h"

namespace ErmWebPage
{
	class AddDevice : public BasePage
	{
	public: 
		AddDevice(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~AddDevice();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __ErmWebPage_AddDevice_H__

