#include "AddFilterItem.h"

namespace BcastWebPage
{

	AddFilterItem::AddFilterItem(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	AddFilterItem::~AddFilterItem()
	{
	}

	bool AddFilterItem::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		// write <form id="", method="" action="">
		url.clear();
		url.setPath(AddFilterItemPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
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

		// write submit button
		responser<<"<input type=\"submit\" value=\"Add item\"/>";

		// write </form>
		responser<<"</form>";

		LinkToBcastMainPageRight;

		return true;
	}

	bool AddFilterItem::post()
	{
		IHttpResponse& responser = _reqCtx->Response();

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
			glog(ErrorLog, CLOGFMT(AddFilterItem, "%s"), getLastError());
			LinkSpaceError;
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

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

		if(chnlItem.inTimeOffset < 0 ||  chnlItem.inTimeOffset >= chnlItem.outTimeOffset)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "InTimeOffset must be larger than 0, and OutTimeOffset larger than InTimeOffset");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(AddFilterItem, "%s"), getLastError());
			LinkSpaceError;
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		try
		{
			chnlPub->addFilterItem(chnlItem);
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "add [filter#%s] caught %s:%s<br>", 
				content.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(AddFilterItem, "%s"), getLastError());
			LinkSpaceError;
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "push [filter#%s] caught %s:%s<br>", 
				content.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(AddFilterItem, "%s"), getLastError());
			LinkSpaceError;
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "push [filter#%s] caught %s<br>", 
				content.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(AddFilterItem, "%s"), getLastError());
			LinkSpaceError;
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		// redirect page to show channel page
		url.clear();
		url.setPath(BcastMainPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TabKey, AddFilterItemPage);
		responser<<"<script language=\"javascript\">";
		snprintf(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
		responser<<"</script>";

		return true;
	}

} 

