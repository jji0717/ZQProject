#ifndef __ErmWebPage_AddChannel_H__
#define __ErmWebPage_AddChannel_H__

#include "BasePage.h"

namespace ErmWebPage
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

#endif // __ErmWebPage_AddChannel_H__

