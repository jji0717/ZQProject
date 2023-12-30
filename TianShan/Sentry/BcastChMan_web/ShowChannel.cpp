#include "ShowChannel.h"

namespace BcastWebPage
{
	struct SunChannelInfo
	{
		std::string name;
		std::string dest;
		std::string uptime;
	};

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
			snprintf(szBuf, sizeof(szBuf) - 1, "No channel name<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(InsertItem, "%s"), getLastError());
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		std::string desc, netId, type, destIP;
		TianShanIce::StrValues netIds;
		Ice::Int maxBit, destPort;
		TianShanIce::Application::PublishPointPrx pubPrx = NULL;
		::TianShanIce::Application::Broadcast::BcastPublishPointPrx pubBcastPrx = NULL;
		TianShanIce::StrValues items;
		TianShanIce::SRM::ResourceMap resourceRequirement;
		TianShanIce::SRM::Resource clientResource;
		TianShanIce::Variant var;
		TianShanIce::Application::ChannelItem chnlItem;
		try
		{
			pubPrx = chnlPub->open(chnlName);
			pubBcastPrx = ::TianShanIce::Application::Broadcast::BcastPublishPointPrx::checkedCast(pubPrx);
			items = pubBcastPrx->getItemSequence();
			desc = pubBcastPrx->getDesc();
			maxBit = pubBcastPrx->getMaxBitrate();
			netIds = pubBcastPrx->listReplica();
			type = pubBcastPrx->getType();
			resourceRequirement = pubBcastPrx->getResourceRequirement();
			clientResource = resourceRequirement[TianShanIce::SRM::rtEthernetInterface];
			var = clientResource.resourceData["destIP"];
			destIP = var.strs[0];
			var = clientResource.resourceData["destPort"];
			destPort = var.ints[0];
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "show channel [%s] caught %s:%s<br>", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowChannel, "%s"), getLastError());
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::ServerError& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "show channel [%s] caught %s:%s<br>", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowChannel, "%s"), getLastError());
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "show channel [%s] caught %s:%s<br>", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowChannel, "%s"), getLastError());
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "show channel [%s] caught %s<br>", 
				chnlName.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowChannel, "%s"), getLastError());
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
/*
		// write page's functionality
		snprintf(szBuf, sizeof(szBuf) - 1, "<H2>Channel: [%s]</H2>", chnlName.c_str());
		responser<<szBuf;
		responser<<"<fieldset>";
		responser<<"<legend>Properties</legend>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<B>Description:</B> %s<br>", desc.c_str());
		responser<<szBuf;
		snprintf(szBuf, sizeof(szBuf) - 1, "<B>MaxBitrates:</B> %d<br>", maxBit);
		responser<<szBuf;
		netId.clear();
		unsigned int i, count;
		for (i = 0, count = netIds.size(); i < count; i ++)
		{
			netId += netIds[i] + SplitNetIdChar;
		}
		if (netId.size() > 0)
			netId.resize(netId.size() - 1);
		snprintf(szBuf, sizeof(szBuf) - 1, "<B>Restriction on:</B> %s<br>", netId.c_str());
		responser<<szBuf;
		responser<<"</fieldset><br>";
*/
		unsigned int i, count;
		url.clear();
		url.setPath(ShowChannelPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(ChannelNameKey, chnlName.c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id=\"EditChannel\" method=\"post\" action=\"%s\">", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		// write start table flag
		responser<<"<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" >";

		// write table header line
		responser<<"<tr>";
		responser<<"	<td>MaxBitrates</td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input onkeyup=\"value=value.replace(/[^\\d]/g,'') \"onbeforepaste=\"clipboardData.setData('text',clipboardData.getData('text').replace(/[^\\d]/g,''))\" type=\"text\" id=\"maxBitrates\" name=\"maxBitrates\" value=\"%d\"/></td>", maxBit);
		responser<<szBuf;
		responser<<"	<td></td>";
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>DestIP</td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type=\"text\" id=\"destIP\" name=\"destIP\" value=\"%s\"/></td>", destIP.c_str());
		responser<<szBuf;
		responser<<"	<td></td>";
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>DestPort</td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input onkeyup=\"value=value.replace(/[^\\d]/g,'') \"onbeforepaste=\"clipboardData.setData('text',clipboardData.getData('text').replace(/[^\\d]/g,''))\" type=\"text\" id=\"destPort\" name=\"destPort\" value=\"%d\"/></td>", destPort);
		responser<<szBuf;
		responser<<"	<td></td>";
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>Description</td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type=\"text\" id=\"description\" name=\"description\" value=\"%s\"/></td>", desc.c_str());
		responser<<szBuf;
		responser<<"	<td></td>";
		responser<<"</tr>";

		netId.clear();
		for (i = 0, count = netIds.size(); i < count; i ++)
		{
			netId += netIds[i] + SplitNetIdChar;
		}
		if (netId.size() > 0)
			netId.resize(netId.size() - 1);

		responser<<"<tr>";
		responser<<"	<td>ContentStores</td>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type=\"text\" id=\"restriction\" name=\"restriction\" value=\"%s\"/></td>", netId.c_str());
		responser<<szBuf;
		responser<<"	<td><i>use \'" SplitNetIdChar "\' as delimiter</i></td>";
		responser<<"</tr>";

		// write end table flag
		responser<<"</table>";
		responser<<"<br>";
/*
		responser<<"<fieldset>";
//		responser<<"	<legend>Max Bitrates</legend>";
		responser<<"	<label for=\"maxBitrates\">MaxBitrates:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input onkeyup=\"value=value.replace(/[^\\d]/g,'') \"onbeforepaste=\"clipboardData.setData('text',clipboardData.getData('text').replace(/[^\\d]/g,''))\" type=\"text\" id=\"maxBitrates\" name=\"maxBitrates\" value=\"%d\"/>", maxBit);
		responser<<szBuf;
		responser<<"<BR>";
		responser<<"</fieldset>";

		responser<<"<fieldset>";
//		responser<<"	<legend>IP</legend>";
		responser<<"	<label for=\"destIP\">IP:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"destIP\" name=\"destIP\" value=\"%s\"/>", destIP.c_str());
		responser<<szBuf;
		responser<<"<BR>";
		responser<<"</fieldset>";

		responser<<"<fieldset>";
//		responser<<"	<legend>Port</legend>";
		responser<<"	<label for=\"destPort\">Port:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input onkeyup=\"value=value.replace(/[^\\d]/g,'') \"onbeforepaste=\"clipboardData.setData('text',clipboardData.getData('text').replace(/[^\\d]/g,''))\" type=\"text\" id=\"destPort\" name=\"destPort\" value=\"%d\"/>", destPort);
		responser<<szBuf;
		responser<<"<BR>";
		responser<<"</fieldset>";

		responser<<"<fieldset>";
//		responser<<"	<legend>Description</legend>";
		responser<<"	<label for=\"description\">Description:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"description\" name=\"description\" value=\"%s\"/>", desc.c_str());
		responser<<szBuf;
		responser<<"<BR>";
		responser<<"</fieldset>";

		netId.clear();
		for (i = 0, count = netIds.size(); i < count; i ++)
		{
			netId += netIds[i] + SplitNetIdChar;
		}
		if (netId.size() > 0)
			netId.resize(netId.size() - 1);
		responser<<"<fieldset>";
//		responser<<"	<legend>Restriction(use \'" SplitNetIdChar "\' to seperate multi net-id)</legend>";
		responser<<"	<label for=\"restriction\">ContentStores:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<input type=\"text\" id=\"restriction\" name=\"restriction\" value=\"%s\"/>", netId.c_str());
		responser<<szBuf;
		snprintf(szBuf, sizeof(szBuf) - 1, "<cite>use \'" SplitNetIdChar "\' as delimiter</cite>");
		responser<<szBuf;
		responser<<"<BR>";
		responser<<"</fieldset>";
*/
		// write submit button
		responser<<"<input type=\"submit\" value=\"Update\"/>";

		// write </form>
		responser<<"</form>";

		//NOVD Channel to show suppelmental channels
		if(type == NVODChannel_Type)
		{
			std::vector<SunChannelInfo> spChInfos;
			char mainChUptime[40];
			try
			{
				TianShanIce::Application::Broadcast::NVODSupplementalChannels supplementalChannels;
				TianShanIce::Application::Broadcast::NVODSupplementalChannels::const_iterator supplementalChannels_itor;
				TianShanIce::Application::Broadcast::NVODChannelPublishPointPrx pubVVODPrx = NULL;
				pubVVODPrx = ::TianShanIce::Application::Broadcast::NVODChannelPublishPointPrx::checkedCast(pubPrx);
				supplementalChannels = pubVVODPrx->getSupplementalChannels();
				Ice::Long  uptime =  pubVVODPrx->getUpTime();
				uptime = uptime/1000;
				Ice::Long hour = uptime/3600;
				Ice::Long minute = (uptime - hour*3600)/60;
				Ice::Long second = uptime - hour*3600 - minute*60;
				snprintf(mainChUptime, sizeof(mainChUptime) - 1, "%4d:%02d:%02d", (int)hour, (int)minute, (int)second);
				for(supplementalChannels_itor = supplementalChannels.begin(); supplementalChannels_itor != supplementalChannels.end(); supplementalChannels_itor++ )
				{
					SunChannelInfo scInfo;
					::TianShanIce::Application::BroadcastPublishPointPrx supplementChannel = ::TianShanIce::Application::BroadcastPublishPointPrx::checkedCast(*supplementalChannels_itor);
					uptime =  supplementChannel->getUpTime();
					scInfo.name = supplementChannel->getName();
					uptime = uptime/1000;
					hour = uptime/3600;
					minute = (uptime - hour*3600)/60;
					second = uptime - hour*3600 - minute*60;
					char time[40];
					snprintf(time, sizeof(time) - 1, "%4d:%02d:%02d", (int)hour, (int)minute, (int)second);
					scInfo.uptime = time;

					resourceRequirement = supplementChannel->getResourceRequirement();
					clientResource = resourceRequirement[TianShanIce::SRM::rtEthernetInterface];
					var = clientResource.resourceData["destIP"];
					destIP = var.strs[0];
					var = clientResource.resourceData["destPort"];
					destPort = var.ints[0];
					char destination[256];
					snprintf(destination, sizeof(destination) - 1, "%s:%d", destIP.c_str(), destPort);
					scInfo.dest = destination;
			

					spChInfos.push_back(scInfo);
				}
			}
			catch (const TianShanIce::InvalidParameter& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "get supplemental channel [%s] caught %s:%s<br>", 
					chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowChannel, "%s"), getLastError());
				LinkToBcastMainPageError;
				responser.SetLastError(getLastError());
				return false;
			}
			catch (const TianShanIce::ServerError& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "get supplemental channel [%s] caught %s:%s<br>", 
					chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowChannel, "%s"), getLastError());
				LinkToBcastMainPageError;
				responser.SetLastError(getLastError());
				return false;
			}
			catch (const TianShanIce::BaseException& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "get supplemental channel [%s] caught %s:%s<br>", 
					chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowChannel, "%s"), getLastError());
				LinkToBcastMainPageError;
				responser.SetLastError(getLastError());
				return false;
			}
			catch (const Ice::Exception& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "get supplemental channel [%s] caught %s<br>", 
					chnlName.c_str(), ex.ice_name().c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowChannel, "%s"), getLastError());
				LinkToBcastMainPageError;
				responser.SetLastError(getLastError());
				return false;
			}
			// write start table flag
			responser<<"<table class='listTable'>";

			// write table header line
			responser<<"<tr class='heading'>";
			responser<<"	<th>MainChannel</th>";
			responser<<"	<th>SubChannel</th>";
			responser<<"	<th>Destination</th>";
			responser<<"	<th>Uptime</th>";
			responser<<"</tr>";

			for (std::vector<SunChannelInfo>::const_iterator spchinfo_itor = spChInfos.begin(); spchinfo_itor != spChInfos.end(); spchinfo_itor++)
			{
				responser<<"<tr>";
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", mainChUptime);
				responser<<szBuf;
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", spchinfo_itor->name.c_str());
				responser<<szBuf;
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", spchinfo_itor->dest.c_str());
				responser<<szBuf;
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", spchinfo_itor->uptime.c_str());
				responser<<szBuf;
				responser<<"</tr>";
			}

			// write end table flag
			responser<<"</table>";
			responser<<"<br>";
		}

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
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center><a href=\"%s\">%s</a></center></td>", String::getRightStr(url.generate(), "/", false).c_str(), items[i].c_str());
			responser<<szBuf;

			// show a link to edit item page
			url.clear();
			url.setPath(EditItemPage);
			url.setVar(ChannelNameKey, chnlName.c_str());
			url.setVar(ItemNameKey, items[i].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center><a href=\"%s\">Edit</a></center></td>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;

			// show a link to remove item page
			url.clear();
			url.setPath(RemoveItemPage);
			url.setVar(ChannelNameKey, chnlName.c_str());
			url.setVar(ItemNameKey, items[i].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center><a href=\"%s\">Remove</a></center></td>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;

			// show a link to insert item page
			url.clear();
			url.setPath(InsertItemPage);
			url.setVar(ChannelNameKey, chnlName.c_str());
			url.setVar(ItemNameKey, items[i].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center><a href=\"%s\">Insert</a></center></td>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;

			try
			{
				chnlItem = pubBcastPrx->findItem(items[i]);
				snprintf(szBuf, sizeof(szBuf) - 1, "<td>%s</td>", chnlItem.broadcastStart.c_str());
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
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Push item</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
/*
		LinkSpace;
		url.clear();
		url.setPath(EditChannelPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(ChannelNameKey, chnlName.c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Edit this channel</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
*/
		LinkSpace;
		LinkToBcastMainPageRight;

		LinkSpace;
		url.clear();
		url.setPath(ShowChannelPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(ChannelNameKey, chnlName.c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Refresh</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		return true;
	}

	bool ShowChannel::post()
	{
		IHttpResponse& responser = _reqCtx->Response();

		std::string chnlName;
		chnlName = _varMap[ChannelNameKey];
		if (chnlName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "No channel name<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(InsertItem, "%s"), getLastError());
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		// get input values
		std::string desc = _varMap["description"];
		std::string netId = _varMap["restriction"];
		std::string maxBit = _varMap["maxBitrates"];
		std::string destIP = _varMap["destIP"];
		std::string destPort = _varMap["destPort"];
		String::trimAll(desc);
		String::trimAll(netId);
		String::trimAll(maxBit);
		std::vector<std::string> netIds;
		String::splitStr(netId, SplitNetIdChar, netIds);

		TianShanIce::Application::PublishPointPrx pubPrx = NULL;
		::TianShanIce::Application::Broadcast::BcastPublishPointPrx pubBcastPrx = NULL;
		TianShanIce::SRM::ResourceMap resourceRequirement;
		TianShanIce::SRM::Resource clientResource;
		TianShanIce::Variant var;
		TianShanIce::ValueMap valuemap;
		TianShanIce::LValues vtLnts;
		TianShanIce::IValues vtInts;
		TianShanIce::StrValues strvauls;
		try
		{
			pubPrx = chnlPub->open(chnlName);
			pubBcastPrx = ::TianShanIce::Application::Broadcast::BcastPublishPointPrx::checkedCast(pubPrx);
			pubBcastPrx->setDesc(desc);
			pubBcastPrx->restrictReplica(netIds);
			pubBcastPrx->setMaxBitrate(atoi(maxBit.c_str()));
			resourceRequirement = pubBcastPrx->getResourceRequirement();
			clientResource = resourceRequirement[TianShanIce::SRM::rtEthernetInterface];
			
			//add rtEthernetInterface  destIp
			var.bRange = false;
			var.type = ::TianShanIce::vtStrings;
			strvauls.clear();
			strvauls.push_back(destIP);
			var.strs = strvauls;	
			clientResource.resourceData["destIP"] = var;
			var.strs.clear();
			//add rtEthernetInterface  destPort
			var.bRange = false;
			var.type = ::TianShanIce::vtInts;
			vtInts.clear();
			vtInts.push_back(atoi(destPort.c_str()));
			var.ints = vtInts;	
			clientResource.resourceData["destPort"] = var;
			var.ints.clear();
			
			pubBcastPrx->requireResource(TianShanIce::SRM::rtEthernetInterface, clientResource);

		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "update channel [%s] caught %s:%s<br>", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditChannel, "%s"), getLastError());
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::ServerError& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "update channel [%s] caught %s:%s<br>", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditChannel, "%s"), getLastError());
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "update channel [%s] caught %s:%s<br>", 
				chnlName.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditChannel, "%s"), getLastError());
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "update channel [%s] caught %s<br>", 
				chnlName.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(EditChannel, "%s"), getLastError());
			LinkToBcastMainPageError;
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
		snprintf(szBuf, sizeof(szBuf) - 1, "document.location.href=\"%s\"", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
		responser<<"</script>";

		return true;
	}
}