#include "AddChannel.h"


namespace CodWebPage
{
#define PostBackAddChannelPath ""

	AddChannel::AddChannel(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	AddChannel::~AddChannel()
	{
	}

	bool AddChannel::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		// write page's functionality
		responser<<"<H2>New channel</H2>";

		// write <form id="", method="" action="">
		url.clear();
		url.setPath(AddChannelPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());		
		_snprintf(szBuf, sizeof(szBuf) - 1, "<form id='AddChannel' method='post' action='%s'><i>* required fields</i><br>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		responser<<"<fieldset><legend>Channel Information</legend>";
		responser<<"<label for='channelName'>ChannelName:</label><input type='text' id='channelName' name='channelName'/>*\n";
		responser<<"<label for='onDemandName'>OnDemandName:</label><input type='text' id='onDemandName' name='onDemandName'/>\n";
		responser<<"<label for='maxBitrates'>MaxBitrates:</label><input type='text' id='maxBitrates' name='maxBitrates' value='4000000'/>*\n";
		responser<<"<label for='description'>Description:</label><input type='text' id='description' name='description' width=80% />\n";
		responser<<"</fieldset>\n";

		responser<<"<label for='restriction'>Store Restriction:</label><input type='text' id='restriction' name='restriction' width=80% />\n";
		responser<<"<br><i>delimited by \'" SplitNetIdChar "\'</i>\n";

		// the submit button
		responser<<"<input type='submit' value='Add channel'/>";

		// write </form>
		responser<<"</form>";

		// a link to add main page
		LinkToCodMainPageRight;

		return true;
	}

	bool AddChannel::post()
	{
		IHttpResponse& responser = _reqCtx->Response();

		// TODO: get channel information from http request
		std::string chnlName, onDmdName, desc, maxBit, netId;
		chnlName = _varMap["channelName"];
		onDmdName = _varMap["onDemandName"];
		desc = _varMap["description"];
		maxBit = _varMap["maxBitrates"];
		netId = _varMap["restriction"];
		String::trimAll(chnlName);
		String::trimAll(onDmdName);
		String::trimAll(desc);
		String::trimAll(maxBit);
		String::trimAll(netId);
		if (chnlName.empty())
			chnlName = onDmdName;
		if (onDmdName.empty())
			onDmdName = chnlName;
		if (chnlName.empty())
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "ChannelName is empty<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(AddChannel, "%s"), getLastError());
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		ChannelInfo chnlInfo;
		chnlInfo.name = chnlName;
		chnlInfo.onDmdName = onDmdName;
		chnlInfo.maxBit = atoi(maxBit.c_str());
		chnlInfo.desc = desc;
		chnlInfo.netIds.clear();
		String::splitStr(netId, SplitNetIdChar, chnlInfo.netIds);

		TianShanIce::Application::PublishPointPrx pubPrx = NULL;
		TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx chnlPPT = NULL;
		try
		{
			pubPrx = chnlPub->publishEx(chnlInfo.name, chnlInfo.onDmdName, chnlInfo.maxBit, TianShanIce::Properties(), chnlInfo.desc);
			chnlPPT = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
			chnlPPT->restrictReplica(chnlInfo.netIds);
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "publish channel [%s] caught %s:%s<br>", 
				chnlInfo.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(AddChannel, "%s"), getLastError());
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "publish channel [%s] caught %s:%s<br>", 
				chnlInfo.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(AddChannel, "%s"), getLastError());
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "publish channel [%s] caught %s<br>", 
				chnlInfo.name.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(AddChannel, "%s"), getLastError());
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		// redirect page to CodMainPage
		RedirectToCodMainPage;

		return true;
	}

} // namespace CodWebPage

