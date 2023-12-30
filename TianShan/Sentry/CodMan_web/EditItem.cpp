#include "EditItem.h"


namespace CodWebPage
{

	EditItem::EditItem(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	EditItem::~EditItem()
	{
	}

	bool EditItem::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		// get channel name which has been setted into url
		std::string chnlName = _varMap[ChannelNameKey];
		std::string itemName = _varMap[ItemNameKey];
		if (chnlName.empty() || itemName.empty())
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "No channel name or item name<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditItem, "%s"), getLastError());
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
			_snprintf(szBuf, sizeof(szBuf) - 1, "edit item page [%s#%s] caught %s:%s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "edit item page [%s#%s] caught %s:%s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "edit item page [%s#%s] caught %s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		// write page's functionality
		_snprintf(szBuf, sizeof(szBuf) - 1, "<H2>Edit item: [%s#%s]</H2>", chnlName.c_str(), itemName.c_str());
		responser<<szBuf;

		// write <form id="", method="" action="">
		url.clear();
		url.setPath(EditItemPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(ChannelNameKey, chnlName.c_str());
		url.setVar(ItemNameKey, itemName.c_str());
		_snprintf(szBuf, sizeof(szBuf) - 1, "<form id=\"EditItem\" method=\"post\" action=\"%s\">", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		responser<<"<fieldset>";
		responser<<"	<legend>Item name</legend>";
		responser<<"	<label for=\"itemName\">Item name:</label>";
		_snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"itemName\" name=\"itemName\" value=\"%s\"/>", chnlItem.contentName.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>Broadcast time [YYYY-MM-HHThh:mm:ss]</legend>";
		responser<<"	<label for=\"broadcast\">Broadcast time:</label>";
		_snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"broadcast\" name=\"broadcast\" value=\"%s\"/>", chnlItem.broadcastStart.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>Expiration [YYYY-MM-HHThh:mm:ss]</legend>";
		responser<<"	<label for=\"expiration\">Expiration:</label>";
		_snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"expiration\" name=\"expiration\" value=\"%s\"/>", chnlItem.expiration.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>Playable</legend>";
		responser<<"	<label for=\"playable\">Playable:</label>";
		_snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"playable\" name=\"playable\" value=\"%d\"/>", chnlItem.playable);
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>ForceNormalSpeed</legend>";
		responser<<"	<label for=\"forceNormalSpeed\">ForceNormalSpeed:</label>";
		_snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"forceNormalSpeed\" name=\"forceNormalSpeed\" value=\"%d\"/>", chnlItem.forceNormalSpeed);
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>InTimeOffset</legend>";
		responser<<"	<label for=\"inTimeOffset\">InTimeOffset:</label>";
		_snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"inTimeOffset\" name=\"inTimeOffset\" value=\"%lld\"/>", chnlItem.inTimeOffset);
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>OutTimeOffset</legend>";
		responser<<"	<label for=\"outTimeOffset\">OutTimeOffset:</label>";
		_snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"outTimeOffset\" name=\"outTimeOffset\" value=\"%lld\"/>", chnlItem.outTimeOffset);
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>SpliceIn</legend>";
		responser<<"	<label for=\"spliceIn\">SpliceIn:</label>";
		_snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"spliceIn\" name=\"spliceIn\" value=\"%d\"/>", chnlItem.spliceIn);
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>SpliceOut</legend>";
		responser<<"	<label for=\"spliceOut\">SpliceOut:</label>";
		_snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"spliceOut\" name=\"spliceOut\" value=\"%d\"/>", chnlItem.spliceOut);
		responser<<szBuf;
		responser<<"</fieldset>";

		// write submit button
		responser<<"<input type=\"submit\" value=\"Update\"/>";

		// write </form>
		responser<<"</form>";

		LinkToShowChannelPageRight;
		LinkSpace;
		LinkToCodMainPageRight;

		return true;
	}

	bool EditItem::post()
	{
		IHttpResponse& responser = _reqCtx->Response();

		// get channel name which has been setted into url
		std::string chnlName = _varMap[ChannelNameKey];
		std::string itemName = _varMap[ItemNameKey];
		if (chnlName.empty() || itemName.empty())
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "No channel name or item name<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		// get input parameter
		std::string content = _varMap["itemName"];
		std::string broadcast = _varMap["broadcast"];
		std::string expiration = _varMap["expiration"];
		std::string playable = _varMap["playable"];
		std::string forceNormalSpeed = _varMap["forceNormalSpeed"];
		std::string inTimeOffset = _varMap["inTimeOffset"];
		std::string outTimeOffset = _varMap["outTimeOffset"];
		std::string spliceIn = _varMap["spliceIn"];
		std::string spliceOut = _varMap["spliceOut"];
		String::trimAll(content);
		String::trimAll(broadcast);
		String::trimAll(expiration);
		String::trimAll(playable);
		String::trimAll(forceNormalSpeed);
		String::trimAll(inTimeOffset);
		String::trimAll(outTimeOffset);
		String::trimAll(spliceIn);
		String::trimAll(spliceOut);
		if (content.empty())
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "input item name is empty<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		TianShanIce::Application::PublishPointPrx pubPrx = NULL;
		TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx chnlPPT  = NULL;
		TianShanIce::Application::ChannelItem chnlItem;
		chnlItem.contentName = content;
		chnlItem.broadcastStart = broadcast;
		chnlItem.expiration = expiration;
		chnlItem.playable = atoi(playable.c_str());
		chnlItem.forceNormalSpeed = atoi(forceNormalSpeed.c_str());
		chnlItem.inTimeOffset = atol(inTimeOffset.c_str());
		chnlItem.outTimeOffset = atol(outTimeOffset.c_str());
		chnlItem.spliceIn = atoi(spliceIn.c_str());
		chnlItem.spliceOut = atoi(spliceOut.c_str());
		try
		{
			pubPrx = chnlPub->open(chnlName);
			chnlPPT = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
			chnlPPT->replaceItem(itemName, chnlItem);
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "edit [%s#%s] caught %s:%s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "edit [%s#%s] caught %s:%s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "edit [%s#%s] caught %s<br>", 
				chnlName.c_str(), itemName.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		// redirect to show item page
		url.clear();
		url.setPath(ShowItemPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(ChannelNameKey, chnlName.c_str());
		url.setVar(ItemNameKey, content.c_str());
		responser<<"<script language=\"javascript\">";
		_snprintf(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
		responser<<"</script>";
		

		return true;
	}

} // namespace CodWebPage

