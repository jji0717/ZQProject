#ifndef __ErmWebPage_ChannelDetail_H__
#define __ErmWebPage_ChannelDetail_H__

#include "BasePage.h"

namespace ErmWebPage
{
	class ChannelDetail : public BasePage
	{
	public: 
		ChannelDetail(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~ChannelDetail();

	protected: 
		virtual bool get();
		virtual bool post();
	};
}

#endif // __ErmWebPage_ChannelDetail_H__
