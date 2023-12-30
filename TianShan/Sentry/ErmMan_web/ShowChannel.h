#ifndef __ErmWebPage_ShowChannel_H__
#define __ErmWebPage_ShowChannel_H__

#include "BasePage.h"

namespace ErmWebPage
{
	class ShowChannel : public BasePage
	{
	public: 
		ShowChannel(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~ShowChannel();

	protected: 
		virtual bool get();
		virtual bool post();
		bool showChannelInfo(IHttpResponse& responser, std::string devName);
		bool showChannelInfo(IHttpResponse& responser, std::string devName, short port);
		bool enableChannel(TianShanIce::StrValues& channelNames, bool bEnable);
	};
}

#endif // __ErmWebPage_ShowChannel_H__

