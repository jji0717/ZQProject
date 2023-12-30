#ifndef __ErmWebPage_RemoveDevice_H__
#define __ErmWebPage_RemoveDevice_H__

#include "BasePage.h"

namespace ErmWebPage
{
	class RemoveDevice : public BasePage
	{
	public: 
		RemoveDevice(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~RemoveDevice();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __ErmWebPage_RemoveDevice_H__

