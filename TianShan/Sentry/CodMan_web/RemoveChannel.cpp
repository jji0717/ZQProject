#include "RemoveChannel.h"


namespace CodWebPage
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
			_snprintf(szBuf, sizeof(szBuf) - 1, "No channel name<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(InsertItem, "%s"), getLastError());
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		TianShanIce::Application::PublishPointPrx pubPrx = NULL;
		TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx chnlPPT = NULL;
		try
		{
			pubPrx = chnlPub->open(chnlName);
			chnlPPT = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
			chnlPPT->destroy();
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "remove channel [%s] caught %s:%s<br>", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(RemoveChannel, "%s"), getLastError());
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "remove channel [%s] caught %s:%s<br>", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(RemoveChannel, "%s"), getLastError());
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "remove channel [%s] caught %s<br>", 
				chnlName.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(RemoveChannel, "%s"), getLastError());
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		RedirectToCodMainPage;

		return true;
	}

	bool RemoveChannel::post()
	{
		return false;
	}

} // namespace CodWebPage

