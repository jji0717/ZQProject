#include "ShowItem.h"


namespace CodWebPage
{

	ShowItem::ShowItem(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	ShowItem::~ShowItem()
	{
	}

	bool ShowItem::get()
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
		TianShanIce::Application::ChannelItem chnlItem;
		try
		{
			pubPrx = chnlPub->open(chnlName);
			chnlPPT = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
			chnlItem = chnlPPT->findItem(itemName);
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "show [%s#%s] caught %s:%s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "show [%s#%s] caught %s:%s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "show [%s#%s] caught %s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		// write page's functionality
		_snprintf(szBuf, sizeof(szBuf) - 1, "<H2>Channel: %s, Item: %s</H2>", chnlName.c_str(), itemName.c_str());
		responser<<szBuf;

		responser<<"<fieldset>";
		responser<<"<legend>Properties</legend>";
		_snprintf(szBuf, sizeof(szBuf) - 1, "<B>Broadcast time:</B> %s<br>", chnlItem.broadcastStart.c_str());
		responser<<szBuf;
		_snprintf(szBuf, sizeof(szBuf) - 1, "<B>Expiration:</B> %s<br>", chnlItem.expiration.c_str());
		responser<<szBuf;
		_snprintf(szBuf, sizeof(szBuf) - 1, "<B>Playable:</B> %d<br>", chnlItem.playable);
		responser<<szBuf;
		_snprintf(szBuf, sizeof(szBuf) - 1, "<B>ForceNormalSpeed:</B> %d<br>", chnlItem.forceNormalSpeed);
		responser<<szBuf;
		_snprintf(szBuf, sizeof(szBuf) - 1, "<B>InTimeOffset:</B> %lld<br>", chnlItem.inTimeOffset);
		responser<<szBuf;
		_snprintf(szBuf, sizeof(szBuf) - 1, "<B>OutTimeOffset:</B> %lld<br>", chnlItem.outTimeOffset);
		responser<<szBuf;
		_snprintf(szBuf, sizeof(szBuf) - 1, "<B>SpliceIn:</B> %d<br>", chnlItem.spliceIn);
		responser<<szBuf;
		_snprintf(szBuf, sizeof(szBuf) - 1, "<B>SpliceOut:</B> %d<br>", chnlItem.spliceOut);
		responser<<szBuf;
		responser<<"</fieldset><br>";

		// a link to add edit item page
		url.clear();
		url.setPath(EditItemPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(ChannelNameKey, chnlName.c_str());
		url.setVar(ItemNameKey, itemName.c_str());
		_snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Edit this item</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		// show channel page
		LinkSpace;
		LinkToShowChannelPageRight;

		// main page link
		LinkSpace;
		LinkToCodMainPageRight;

		// refresh link
		LinkSpace;
		url.clear();
		url.setPath(ShowItemPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(ChannelNameKey, chnlName.c_str());
		url.setVar(ItemNameKey, itemName.c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Refresh</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
		


		return true;
	}

	bool ShowItem::post()
	{
		return true;
	}

} // namespace CodWebPage

