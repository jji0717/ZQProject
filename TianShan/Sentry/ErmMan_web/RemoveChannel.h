#ifndef __ErmWebPage_RemoveChannel_H__
#define __ErmWebPage_RemoveChannel_H__

#include "BasePage.h"

namespace ErmWebPage
{
	class RemoveChannel : public BasePage
	{
	public: 
		RemoveChannel(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~RemoveChannel();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __ErmWebPage_RemoveChannel_H__

