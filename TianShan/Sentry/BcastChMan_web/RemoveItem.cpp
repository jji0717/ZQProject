#include "RemoveItem.h"


namespace BcastWebPage
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
			snprintf(szBuf, sizeof(szBuf) - 1, "No channel name or item name<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(RemoveItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		TianShanIce::Application::PublishPointPrx pubPrx = NULL;
		TianShanIce::Application::Broadcast::BcastPublishPointPrx pubBcastPrx = NULL;
		try
		{
			pubPrx = chnlPub->open(chnlName);
			pubBcastPrx = ::TianShanIce::Application::Broadcast::BcastPublishPointPrx::checkedCast(pubPrx);
			pubBcastPrx->removeItem(itemName);
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "remove [%s#%s] caught %s:%s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(RemoveItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "remove [%s#%s] caught %s:%s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(RemoveItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "remove [%s#%s] caught %s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(RemoveItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToBcastMainPageError;
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

