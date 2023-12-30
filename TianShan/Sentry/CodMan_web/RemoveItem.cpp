#include "RemoveItem.h"


namespace CodWebPage
{

	RemoveItem::RemoveItem(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	RemoveItem::~RemoveItem()
	{
	}

	bool RemoveItem::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		std::string chnlName, itemName;
		chnlName = _varMap[ChannelNameKey];
		itemName = _varMap[ItemNameKey];
		if (chnlName.empty() || itemName.empty())
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "No channel name or item name<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(InsertItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		TianShanIce::Application::PublishPointPrx pubPrx = NULL;
		TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx chnlPPT  = NULL;
		try
		{
			pubPrx = chnlPub->open(chnlName);
			chnlPPT = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
			chnlPPT->removeItem(itemName);
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "remove [%s#%s] caught %s:%s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(RemoveItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "remove [%s#%s] caught %s:%s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(RemoveItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "remove [%s#%s] caught %s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(RemoveItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		// redirect page to show item page
		RedirectToShowChannelPage;

		return true;
	}

	bool RemoveItem::post()
	{
		return true;
	}

} // namespace CodWebPage

