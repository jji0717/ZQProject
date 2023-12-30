#ifndef __CodWebPage_ShowChannel_H__
#define __CodWebPage_ShowChannel_H__

#include "BasePage.h"

namespace CodWebPage
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

#endif // __CodWebPage_ShowChannel_H__

