#include "RemoveChannel.h"


namespace ErmWebPage
{

	RemoveChannel::RemoveChannel(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	RemoveChannel::~RemoveChannel()
	{
	}

	bool RemoveChannel::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		std::string chnlName;
		chnlName = _varMap[ChannelNameKey];
		if (chnlName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "No channel name<br>");
			setLastError(szBuf);
			responser.SetLastError(getLastError());
			return false;
		}

//		removeChannel(chnlName, CHANNEL_SAFESTORE);
		
		// redirect page to show channel page
		// <script language="javascript">
		//	document.location.href="http://www.cctv.com"
		// </script>
		url.clear();
		url.setPath(ShowChannelPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(ChannelNameKey, chnlName.c_str());
		responser<<"<script language=\"javascript\">";
		snprintf(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
		responser<<"</script>";

		return true;
		return true;
	}

	bool RemoveChannel::post()
	{
		return false;
	}

} // namespace ErmWebPage

