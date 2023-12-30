#ifndef __CodWebPage_RemoveChannel_H__
#define __CodWebPage_RemoveChannel_H__

#include "BasePage.h"

namespace CodWebPage
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

#endif // __CodWebPage_RemoveChannel_H__

