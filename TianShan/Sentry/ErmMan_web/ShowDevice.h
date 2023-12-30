#ifndef __ErmWebPage_ShowDevice_H__
#define __ErmWebPage_ShowDevice_H__

#include "BasePage.h"

namespace ErmWebPage
{
	class ShowDevice : public BasePage
	{
	public: 
		ShowDevice(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~ShowDevice();

	protected: 
		virtual bool get();
		virtual bool post();
		bool enableDevice(TianShanIce::StrValues& deviceNames, bool bEnable);
		bool destroy(TianShanIce::StrValues& deviceNames);
	};
}

#endif // __ErmWebPage_ShowDevice_H__