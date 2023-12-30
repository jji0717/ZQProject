#include "ShowChannel.h"


namespace CodWebPage
{

	ShowChannel::ShowChannel(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	ShowChannel::~ShowChannel()
	{
	}

	bool ShowChannel::get()
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

		std::string onDmdName, desc, netId;
		TianShanIce::StrValues netIds;
		Ice::Int maxBit;
		TianShanIce::Application::PublishPointPrx pubPrx = NULL;
		TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx chnlPPT = NULL;
		TianShanIce::StrValues items;
		TianShanIce::Application::ChannelItem chnlItem;
		try
		{
			pubPrx = chnlPub->open(chnlName);
			chnlPPT = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
			items = chnlPPT->getItemSequence();
			onDmdName = chnlPPT->getOnDemandName();
			desc = chnlPPT->getDesc();
			netIds = chnlPPT->listReplica();
			maxBit = chnlPPT->getMaxBitrate();
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "show channel [%s] caught %s:%s<br>", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowChannel, "%s"), getLastError());
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "show channel [%s] caught %s:%s<br>", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowChannel, "%s"), getLastError());
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			_snprintf(szBuf, sizeof(szBuf) - 1, "show channel [%s] caught %s<br>", 
				chnlName.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowChannel, "%s"), getLastError());
			LinkToCodMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		// write page's functionality
		_snprintf(szBuf, sizeof(szBuf) - 1, "<H2>Channel: [%s]</H2>", chnlName.c_str());
		responser<<szBuf;
		responser<<"<fieldset>";
		responser<<"<legend>Properties</legend>";
		_snprintf(szBuf, sizeof(szBuf) - 1, "<B>OnDemandName:</B> %s<br>", onDmdName.c_str());
		responser<<szBuf;
		_snprintf(szBuf, sizeof(szBuf) - 1, "<B>Description:</B> %s<br>", desc.c_str());
		responser<<szBuf;
		_snprintf(szBuf, sizeof(szBuf) - 1, "<B>MaxBitrates:</B> %d<br>", maxBit);
		responser<<szBuf;
		netId.clear();
		unsigned int i, count;
		for (i = 0, count = netIds.size(); i < count; i ++)
		{
			netId += netIds[i] + SplitNetIdChar;
		}
		if (netId.size() > 0)
			netId.resize(netId.size() - 1);
		_snprintf(szBuf, sizeof(szBuf) - 1, "<B>Restriction on:</B> %s<br>", netId.c_str());
		responser<<szBuf;
		responser<<"</fieldset><br>";

		// write start table flag
		responser<<"<table class='listTable'>";

		// write table header line
		responser<<"<tr class='heading'>";
		responser<<"	<th>ItemName</th>";
		responser<<"	<th>Edit</th>";
		responser<<"	<th>Remove</th>";
		responser<<"	<th>Insert</th>";
		responser<<"	<th>BroadcastTime</th>";
		responser<<"</tr>";

		// write table data line
		for (i = 0, count = items.size(); i < count; i ++)
		{
			responser<<"<tr>";

			// show item name
			url.clear();
			url.setPath(ShowItemPage);
			url.setVar(ChannelNameKey, chnlName.c_str());
			url.setVar(ItemNameKey, items[i].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			_snprintf(szBuf, sizeof(szBuf) - 1, "<td><center><a href=\"%s\">%s</a></center></td>", String::getRightStr(url.generate(), "/", false).c_str(), items[i].c_str());
			responser<<szBuf;

			// show a link to edit item page
			url.clear();
			url.setPath(EditItemPage);
			url.setVar(ChannelNameKey, chnlName.c_str());
			url.setVar(ItemNameKey, items[i].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			_snprintf(szBuf, sizeof(szBuf) - 1, "<td><center><a href=\"%s\">Edit</a></center></td>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;

			// show a link to remove item page
			url.clear();
			url.setPath(RemoveItemPage);
			url.setVar(ChannelNameKey, chnlName.c_str());
			url.setVar(ItemNameKey, items[i].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			_snprintf(szBuf, sizeof(szBuf) - 1, "<td><center><a href=\"%s\">Remove</a></center></td>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;

			// show a link to insert item page
			url.clear();
			url.setPath(InsertItemPage);
			url.setVar(ChannelNameKey, chnlName.c_str());
			url.setVar(ItemNameKey, items[i].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			_snprintf(szBuf, sizeof(szBuf) - 1, "<td><center><a href=\"%s\">Insert</a></center></td>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;

			try
			{
				chnlItem = chnlPPT->findItem(items[i]);
				_snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", chnlItem.broadcastStart.c_str());
				responser<<szBuf;
			}
			catch (const TianShanIce::InvalidParameter& ex)
			{
			}
			catch (const TianShanIce::BaseException& ex)
			{
			}
			catch (const Ice::Exception& ex)
			{
			}

			responser<<"</tr>";
		}

		// write end table flag
		responser<<"</table>";
		responser<<"<br>";

		// show a link to push item page
		url.clear();
		url.setPath(PushItemPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(ChannelNameKey, chnlName.c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		_snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Push item</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		LinkSpace;
		url.clear();
		url.setPath(EditChannelPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(ChannelNameKey, chnlName.c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		_snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Edit this channel</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		LinkSpace;
		LinkToCodMainPageRight;

		LinkSpace;
		url.clear();
		url.setPath(ShowChannelPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(ChannelNameKey, chnlName.c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		_snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Refresh</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		return true;
	}

	bool ShowChannel::post()
	{
		return true;
	}

} // namespace CodWebPage

