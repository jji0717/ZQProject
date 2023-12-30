#include "EditChannel.h"


namespace CodWebPage
{
#define PostBackEditChannelPath ""

	EditChannel::EditChannel(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	EditChannel::~EditChannel()
	{
	}

	bool EditChannel::get()
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

		// write page's functionality
		_snprintf(szBuf, sizeof(szBuf) - 1, "<H2>Edit channel: [%s]</H2>", chnlName.c_str());
		responser<<szBuf;

		std::string onDmdName, desc, netId;
		TianShanIce::StrValues netIds;
		Ice::Int maxBit;
		TianShanIce::Application::PublishPointPrx pubPrx = NULL;
		TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx chnlPPT = NULL;
		try
		{
			pubPrx = chnlPub->open(chnlName);
			chnlPPT = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
			onDmdName = chnlPPT->getOnDemandName();
			desc = chnlPPT->getDesc();
			netIds = chnlPPT->listReplica();
			maxBit = chnlPPT->getMaxBitrate();
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "retrieve channel [%s] caught %s:%s<br>", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditChannel, "%s"), getLastError());
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "retrieve channel [%s] caught %s:%s<br>", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditChannel, "%s"), getLastError());
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "retrieve channel [%s] caught %s<br>", 
				chnlName.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditChannel, "%s"), getLastError());
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		url.clear();
		url.setPath(EditChannelPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(ChannelNameKey, chnlName.c_str());
		_snprintf(szBuf, sizeof(szBuf) - 1, "<form id=\"EditChannel\" method=\"post\" action=\"%s\">", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
		
		// write html text like the following
		//		<fieldset>
		//			<legend>用户名:</legend>
		//			<label for="username">用户名:</label>
		//			<input type="text" id="username" name="username" />
		//		</fieldset>

		responser<<"<fieldset>";
		responser<<"<legend>On Demand Name</legend>";
		responser<<"<label for=\"onDemandName\">OnDemandName:</label>";
		_snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"onDemandName\" name=\"onDemandName\" value=\"%s\"/>", onDmdName.c_str());
		responser<<szBuf;
		responser<<"<font color=red>Use Channel's name if left empty</font>";
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>Max Bitrates</legend>";
		responser<<"	<label for=\"maxBitrates\">MaxBitrates:</label>";
		_snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"maxBitrates\" name=\"maxBitrates\" value=\"%d\"/>", maxBit);
		responser<<szBuf;
		responser<<"</fieldset>";

		responser<<"<fieldset>";
		responser<<"	<legend>Description</legend>";
		responser<<"	<label for=\"description\">Description:</label>";
		_snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"description\" name=\"description\" value=\"%s\"/>", desc.c_str());
		responser<<szBuf;
		responser<<"</fieldset>";

		netId.clear();
		for (unsigned int i = 0, count = netIds.size(); i < count; i ++)
		{
			netId += netIds[i] + SplitNetIdChar;
		}
		if (netId.size() > 0)
			netId.resize(netId.size() - 1);
		responser<<"<fieldset>";
		responser<<"	<legend>Restriction(use \'" SplitNetIdChar "\' to seperate multi net-id)</legend>";
		responser<<"	<label for=\"restriction\">Restriction:</label>";
		_snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"restriction\" name=\"restriction\" value=\"%s\"/>", netId.c_str());
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

	bool EditChannel::post()
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

		// get input values
		std::string onDmdName = _varMap["onDemandName"];
		std::string desc = _varMap["description"];
		std::string netId = _varMap["restriction"];
		std::string maxBit = _varMap["maxBitrates"];
		String::trimAll(onDmdName);
		String::trimAll(desc);
		String::trimAll(netId);
		String::trimAll(maxBit);
		std::vector<std::string> netIds;
		String::splitStr(netId, SplitNetIdChar, netIds);

		if (onDmdName.empty())
			onDmdName = chnlName;
		if (chnlName.empty())
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "ChannelName is empty<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditChannel, "%s"), getLastError());
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
			chnlPPT->setOnDemandName(onDmdName);
			chnlPPT->setDesc(desc);
			chnlPPT->restrictReplica(netIds);
			chnlPPT->setMaxBitrate(atoi(maxBit.c_str()));
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "update channel [%s] caught %s:%s<br>", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditChannel, "%s"), getLastError());
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "update channel [%s] caught %s:%s<br>", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditChannel, "%s"), getLastError());
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "update channel [%s] caught %s<br>", 
				chnlName.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditChannel, "%s"), getLastError());
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

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
		_snprintf(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
		responser<<"</script>";

		return true;
	}

} // namespace CodWebPage

