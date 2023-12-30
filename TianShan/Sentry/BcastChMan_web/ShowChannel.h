#ifndef __BcastWebPage_ShowChannel_H__
#define __BcastWebPage_ShowChannel_H__

#include "BasePage.h"

namespace BcastWebPage
{
	class ShowChannel : public BasePage
	{
	public: 
		ShowChannel(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~ShowChannel();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __BcastWebPage_ShowChannel_H__
