#include "PushItem.h"


namespace BcastWebPage
{

	PushItem::PushItem(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	PushItem::~PushItem()
	{
	}

	bool PushItem::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		// get channel name which has been setted into url
		std::string chnlName = _varMap[ChannelNameKey];
		if (chnlName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "No channel name<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(PushItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		// write page's functionality
		snprintf(szBuf, sizeof(szBuf) - 1, "<H2>Push item on channel: [%s]</H2>", chnlName.c_str());
		responser<<szBuf;

		// write <form id="", method="" action="">
		url.clear();
		url.setPath(PushItemPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(ChannelNameKey, chnlName.c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id=\"PushItem\" method=\"post\" action=\"%s\">", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		responser<<"<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" >";

		// write table header line
		responser<<"<tr>";
		responser<<"	<td>Item name</td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type=\"text\" id=\"itemName\" name=\"itemName\"/></td>");
		responser<<szBuf;
		responser<<"	<td></td>";
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>Broadcast time</td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type=\"text\" id=\"broadcast\" name=\"broadcast\"/></td>");
		responser<<szBuf;
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><cite>[YYYY-MM-HHThh:mm:ss]</cite></td>");
		responser<<szBuf;
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>Expiration</td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type=\"text\" id=\"expiration\" name=\"expiration\"/></td>");
		responser<<szBuf;
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><cite>[YYYY-MM-HHThh:mm:ss]</cite></td>");
		responser<<szBuf;
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>Playable</td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type=\"text\" id=\"playable\" name=\"playable\" value=\"1\"/></td>");
		responser<<szBuf;
		responser<<"	<td></td>";
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>ForceNormalSpeed</td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type=\"text\" id=\"forceNormalSpeed\" name=\"forceNormalSpeed\" value=\"0\"/></td>");
		responser<<szBuf;
		responser<<"	<td></td>";
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>InTimeOffset</td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type=\"text\" id=\"inTimeOffset\" name=\"inTimeOffset\" value=\"0\"/></td>");
		responser<<szBuf;
		responser<<"	<td></td>";
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>OutTimeOffset</td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type=\"text\" id=\"outTimeOffset\" name=\"outTimeOffset\" value=\"0\"/></td>");
		responser<<szBuf;
		responser<<"	<td></td>";
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>SpliceIn</td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type=\"text\" id=\"spliceIn\" name=\"spliceIn\" value=\"0\"/></td>");
		responser<<szBuf;
		responser<<"	<td></td>";
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>SpliceOut</td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type=\"text\" id=\"spliceOut\" name=\"spliceOut\" value=\"0\"/></td>");
		responser<<szBuf;
		responser<<"	<td></td>";
		responser<<"</tr>";

		// write end table flag
		responser<<"</table>";
		responser<<"<br>";

/*
		responser<<"<fieldset>";
//		responser<<"	<legend>Item name</legend>";
		responser<<"	<label for=\"itemName\">Item name:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"itemName\" name=\"itemName\"/>");
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>Broadcast time [YYYY-MM-HHThh:mm:ss]</legend>";
		responser<<"	<label for=\"broadcast\">Broadcast time:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"broadcast\" name=\"broadcast\"/>");
		responser<<szBuf;
		snprintf(szBuf, sizeof(szBuf) - 1, "<cite>[YYYY-MM-HHThh:mm:ss]</cite>");
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>Expiration [YYYY-MM-HHThh:mm:ss]</legend>";
		responser<<"	<label for=\"expiration\">Expiration:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"expiration\" name=\"expiration\"/>");
		responser<<szBuf;
		snprintf(szBuf, sizeof(szBuf) - 1, "<cite>[YYYY-MM-HHThh:mm:ss]</cite>");
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>Playable</legend>";
		responser<<"	<label for=\"playable\">Playable:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"playable\" name=\"playable\" value=\"1\"/>");
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>ForceNormalSpeed</legend>";
		responser<<"	<label for=\"forceNormalSpeed\">ForceNormalSpeed:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"forceNormalSpeed\" name=\"forceNormalSpeed\" value=\"0\"/>");
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>InTimeOffset</legend>";
		responser<<"	<label for=\"inTimeOffset\">InTimeOffset:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"inTimeOffset\" name=\"inTimeOffset\" value=\"0\"/>");
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>OutTimeOffset</legend>";
		responser<<"	<label for=\"outTimeOffset\">OutTimeOffset:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"outTimeOffset\" name=\"outTimeOffset\" value=\"0\"/>");
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>SpliceIn</legend>";
		responser<<"	<label for=\"spliceIn\">SpliceIn:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"spliceIn\" name=\"spliceIn\" value=\"0\"/>");
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>SpliceOut</legend>";
		responser<<"	<label for=\"spliceOut\">SpliceOut:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"spliceOut\" name=\"spliceOut\" value=\"0\"/>");
		responser<<szBuf;
		responser<<"<BR>";
		responser<<"</fieldset>";
*/
		// write submit button
		responser<<"<input type=\"submit\" value=\"Push item\"/>";

		// write </form>
		responser<<"</form>";

		LinkToShowChannelPageRight;
		LinkSpace;
		LinkToBcastMainPageRight;

		return true;
	}

	bool PushItem::post()
	{
		IHttpResponse& responser = _reqCtx->Response();

		// get channel name which has been setted into url
		std::string chnlName = _varMap[ChannelNameKey];
		if (chnlName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "No channel name<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(PushItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToBcastMainPageError;
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
			snprintf(szBuf, sizeof(szBuf) - 1, "input item name is empty<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(PushItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		TianShanIce::Application::PublishPointPrx pubPrx = NULL;
		TianShanIce::Application::Broadcast::BcastPublishPointPrx pubBcastPrx = NULL;
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
			pubBcastPrx = ::TianShanIce::Application::Broadcast::BcastPublishPointPrx::checkedCast(pubPrx);
			pubBcastPrx->appendItem(chnlItem);
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "push [%s#%s] caught %s:%s<br>", 
				chnlName.c_str(), content.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(PushItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "push [%s#%s] caught %s:%s<br>", 
				chnlName.c_str(), content.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(PushItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "push [%s#%s] caught %s<br>", 
				chnlName.c_str(), content.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(PushItem, "%s"), getLastError());
			LinkToShowChannelPageError;
			LinkSpaceError;
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		// redirect page to show channel page
		RedirectToShowChannelPage;

		return true;
	}

} 

