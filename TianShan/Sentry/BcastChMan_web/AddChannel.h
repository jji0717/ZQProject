#ifndef __BcastWebPage_AddChannel_H__
#define __BcastWebPage_AddChannel_H__

#include "BasePage.h"

namespace BcastWebPage
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

#endif // __BcastWebPage_AddChannel_H__
