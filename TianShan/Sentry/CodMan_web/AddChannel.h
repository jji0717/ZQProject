#ifndef __CodWebPage_AddChannel_H__
#define __CodWebPage_AddChannel_H__

#include "BasePage.h"

namespace CodWebPage
{
	class AddChannel : public BasePage
	{
	public: 
		AddChannel(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~AddChannel();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __CodWebPage_AddChannel_H__

