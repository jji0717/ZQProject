#ifndef __BcastWebPage_RemoveChannel_H__
#define __BcastWebPage_RemoveChannel_H__

#include "BasePage.h"

namespace BcastWebPage
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

#endif // __BcastWebPage_RemoveChannel_H__