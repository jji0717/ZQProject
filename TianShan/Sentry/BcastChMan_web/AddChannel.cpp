#include "AddChannel.h"

namespace BcastWebPage
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

		//different types of channel
		responser << "<SCRIPT language=\"javascript\">"; 
		responser << "	function ss()  \n"; 
		responser << "{  \n"; 
		responser << "	if(document.getElementById(\"channelType\").value==\"Bcast\") \n";  
		responser << "	{  \n"; 
		responser << "		document.getElementById(\"interval\").value = 'Not Required';\n"; 
		responser << "		document.getElementById(\"iteration\").value = 'Not Required';\n"; 
		responser << "		document.getElementById(\"channelName\").value = 'Bcast_';\n"; 
		responser << "		document.getElementById(\"interval\").disabled = 'true';\n"; 
		responser << "		document.getElementById(\"iteration\").disabled = 'true';\n"; 
		responser << "	}\n"; 
		responser << "	if(document.getElementById(\"channelType\").value==\"NVOD\")  \n"; 
		responser << "	{   \n";
		responser << "		document.getElementById(\"interval\").value = '5000';\n"; 
		responser << "		document.getElementById(\"iteration\").value = '5'; \n"; 
		responser << "		document.getElementById(\"channelName\").value = 'NVOD_';\n";
		responser << "		document.getElementById(\"interval\").disabled = 0;\n"; 
		responser << "		document.getElementById(\"iteration\").disabled = 0;\n"; 
		responser << "	}\n"; 
		responser << "}  \n"; 
		responser << "</SCRIPT>  \n"; 

		// write <form id="", method="" action="">
		url.clear();
		url.setPath(AddChannelPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());		
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id='AddChannel' method='post' action='%s'><i>* required fields</i><br>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		responser << "<TH>Channle Type ";
		responser << "<select name=\"channelType\" size=\"1\" onchange=\"ss();\">";
		responser << "	<option value=\"Bcast\">Bcast";
		responser << "	<option value=\"NVOD\" selected>NVOD";
		responser << " </select>";
		responser << "</TH>\n";

		responser << "<BR>\n";

		// write start table flag
		responser<<"<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" >";

		// write table header line
		responser<<"<tr>";
		responser<<"	<td width=\"80\">ChannelName</td>";
		responser<<"	<td width=\"125\"><input type='text' id='channelName' name='channelName' value='NVOD_'/></td>";
		responser<<"	<td>*</td>";
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>MaxBitrates</td>";
		responser<<"	<td><input onkeyup=\"value=value.replace(/[^\\d]/g,'') \"onbeforepaste=\"clipboardData.setData('text',clipboardData.getData('text').replace(/[^\\d]/g,''))\" type='text' id='maxBitrates' name='maxBitrates' value='4000000'/></td>";
		responser<<"	<td>*</td>";
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>Description</td>";
		responser<<"	<td><input type='text' id='description' name='description'/></td>";
		responser<<"	<td></td>";
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>DestMac</td>";
		responser<<"	<td><input type='text' id='destMac' name='destMac'/></td>";
		responser<<"	<td></td>";
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>DestIP</td>";
		responser<<"	<td><input type='text' id='destIP' name='destIP' value='10.0.0.0'/></td>";
		responser<<"	<td>*</td>";
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>DestPort</td>";
		responser<<"	<td><input onkeyup=\"value=value.replace(/[^\\d]/g,'') \"onbeforepaste=\"clipboardData.setData('text',clipboardData.getData('text').replace(/[^\\d]/g,''))\" type='text' id='destPort' name='destPort' value='1000'/></td>";
		responser<<"	<td>*</td>";
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>Interval</td>";
		responser<<"	<td><input onkeyup=\"value=value.replace(/[^\\d]/g,'') \"onbeforepaste=\"clipboardData.setData('text',clipboardData.getData('text').replace(/[^\\d]/g,''))\" type='text' id='interval' name='interval' value='5000'/></td>";
		responser<<"	<td>*</td>";
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>Iteration</td>";
		responser<<"	<td><input onkeyup=\"value=value.replace(/[^\\d]/g,'') \"onbeforepaste=\"clipboardData.setData('text',clipboardData.getData('text').replace(/[^\\d]/g,''))\" type='text' id='iteration' name='iteration' value='5'/></td>";
		responser<<"	<td>*</td>";
		responser<<"</tr>";

		responser<<"<tr>";
		responser<<"	<td>ContentStores</td>";
		responser<<"	<td><input type='text' id='restriction' name='restriction'/></td>";
		responser<<"	<td><i>delimited by \'" SplitNetIdChar "\'</i></td>";
		responser<<"</tr>";

		// write end table flag
		responser<<"</table>";
		responser<<"<br>";
/*
		responser<<"<fieldset><legend>Channel Information</legend>";
		responser<<"<label for='channelName'>ChannelName:</label><input type='text' id='channelName' name='channelName' value='NVOD_'/>*\n";
		responser << "<BR>\n";
//		responser<<"<label for='bandwidth'>Bandwidth:</label><input type='text' id='bandwidth' name='bandwidth' value='4000'/>*\n";
//		responser << "<BR>\n";
		responser<<"<label for='maxBitrates'>MaxBitrates:</label><input onkeyup=\"value=value.replace(/[^\\d]/g,'') \"onbeforepaste=\"clipboardData.setData('text',clipboardData.getData('text').replace(/[^\\d]/g,''))\" type='text' id='maxBitrates' name='maxBitrates' value='4000000'/>\n";
		responser << "<BR>\n";
		responser<<"<label for='description'>Description:</label><input type='text' id='description' name='description' width=80% />\n";
		responser << "<BR>\n";
		responser<<"<label for='destMac'>DestMac:</label><input type='text' id='destMac' name='destMac'/>\n";
		responser << "<BR>\n";
		responser<<"<label for='destIP'>DestIP:</label><input type='text' id='destIP' name='destIP' value='10.0.0.0'/>*\n";
		responser << "<BR>\n";
		responser<<"<label for='destPort'>DestPort:</label><input onkeyup=\"value=value.replace(/[^\\d]/g,'') \"onbeforepaste=\"clipboardData.setData('text',clipboardData.getData('text').replace(/[^\\d]/g,''))\" type='text' id='destPort' name='destPort' value='1000'/>*\n";
		responser << "<BR>\n";
		responser<<"<label for='interval'>Interval:</label><input onkeyup=\"value=value.replace(/[^\\d]/g,'') \"onbeforepaste=\"clipboardData.setData('text',clipboardData.getData('text').replace(/[^\\d]/g,''))\" type='text' id='interval' name='interval' value='5000'/>*\n";
		responser << "<BR>\n";
		responser<<"<label for='iteration'>Iteration:</label><input onkeyup=\"value=value.replace(/[^\\d]/g,'') \"onbeforepaste=\"clipboardData.setData('text',clipboardData.getData('text').replace(/[^\\d]/g,''))\" type='text' id='iteration' name='iteration' value='5'/>*\n";
		responser << "<BR>\n";
		responser<<"</fieldset>\n";

		responser<<"<label for='restriction'>ContentStores:</label><input type='text' id='restriction' name='restriction' width=80% />\n";
		responser<<"<br><i>delimited by \'" SplitNetIdChar "\'</i>\n";
*/

		responser<<"<BR>";
		// the submit button
		responser<<"<input type='submit' value='Add channel'/>";

		// write </form>
		responser<<"</form>";

		// a link to add main page
		LinkToBcastMainPageRight;

		return true;
	}

	bool AddChannel::post()
	{
		IHttpResponse& responser = _reqCtx->Response();

		// TODO: get channel information from http request
		std::string chnlName, bandwidth, desc, maxBit, netId, type, destIP, destMac, destPort, Interval, Iteration;
		chnlName = _varMap["channelName"];
//		bandwidth = _varMap["bandwidth"];
		desc = _varMap["description"];
		maxBit = _varMap["maxBitrates"];
		netId = _varMap["restriction"];
		type = _varMap["channelType"];
		destIP = _varMap["destIP"];
		destMac = _varMap["destMac"];
		destPort = _varMap["destPort"];
		Interval = _varMap["interval"];
		Iteration = _varMap["iteration"];
		String::trimAll(chnlName);
		String::trimAll(bandwidth);
		String::trimAll(desc);
		String::trimAll(maxBit);
		String::trimAll(netId);
		String::trimAll(type);
		if (chnlName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "ChannelName is empty<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(AddChannel, "%s"), getLastError());
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		ChannelInfo chnlInfo;
		chnlInfo.name = chnlName;
//		chnlInfo.bandwidth = bandwidth;
		chnlInfo.maxBit = atoi(maxBit.c_str());
		chnlInfo.desc = desc;
		chnlInfo.netIds.clear();
		String::splitStr(netId, SplitNetIdChar, chnlInfo.netIds);
	
		try
		{
			TianShanIce::Properties props;
			TianShanIce::SRM::ResourceMap resourceRequirement;
			TianShanIce::SRM::Resource clientResource;
			TianShanIce::Variant var;
			TianShanIce::ValueMap valuemap;
			TianShanIce::LValues vtLnts;
			TianShanIce::IValues vtInts;
			TianShanIce::StrValues strvauls;

			if(type.compare("Bcast") == 0)
			{
				TianShanIce::Application::PublishPointPrx pubPrx = NULL;
/*
				clientResource.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
				clientResource.status = TianShanIce::SRM::rsRequested;
				var.bRange = false;
				var.type = ::TianShanIce::vtLongs;
				vtLnts.clear();
				vtLnts.push_back(atol(bandwidth.c_str()));
				var.lints = vtLnts;	
				clientResource.resourceData["bandwidth"] = var;
				resourceRequirement[TianShanIce::SRM::ResourceType::rtTsDownstreamBandwidth] = clientResource;
				var.lints.clear();
				clientResource.resourceData.clear();
*/
				//add rtEthernetInterface  destIP
				clientResource.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
				clientResource.status = TianShanIce::SRM::rsRequested;
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

				//add rtEthernetInterface  destMac
				var.bRange = false;
				var.type = ::TianShanIce::vtStrings;
				strvauls.clear();
				strvauls.push_back(destMac);
				var.strs = strvauls;	
				clientResource.resourceData["destMac"] = var;		
				var.strs.clear();

				resourceRequirement[TianShanIce::SRM::rtEthernetInterface] = clientResource;

				pubPrx = chnlPub->createBcastPublishPoint(chnlInfo.name, resourceRequirement, props, chnlInfo.desc);

				pubPrx->restrictReplica(chnlInfo.netIds);
				pubPrx->setMaxBitrate(atol(maxBit.c_str()));
				
			}
			if(type.compare("NVOD") == 0)
			{
				TianShanIce::Application::Broadcast::NVODChannelPublishPointPrx pubPrx = NULL;
/*
				clientResource.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
				clientResource.status = TianShanIce::SRM::rsRequested;
				var.bRange = false;
				var.type = ::TianShanIce::vtLongs;
				vtLnts.clear();
				vtLnts.push_back(atol(bandwidth.c_str()));
				var.lints = vtLnts;	
				clientResource.resourceData["bandwidth"] = var;
				resourceRequirement[TianShanIce::SRM::ResourceType::rtTsDownstreamBandwidth] = clientResource;
				var.lints.clear();
				clientResource.resourceData.clear();
*/
				//add rtEthernetInterface  destIP
				clientResource.attr = TianShanIce::SRM::raMandatoryNonNegotiable;
				clientResource.status = TianShanIce::SRM::rsRequested;
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

				//add rtEthernetInterface  destMac
				var.bRange = false;
				var.type = ::TianShanIce::vtStrings;
				strvauls.clear();
				strvauls.push_back(destMac);
				var.strs = strvauls;	
				clientResource.resourceData["destMac"] = var;		
				var.strs.clear();

				resourceRequirement[TianShanIce::SRM::rtEthernetInterface] = clientResource;
				pubPrx = chnlPub->createNVODPublishPoint(chnlName, resourceRequirement,atoi(Iteration.c_str()), atoi(Interval.c_str()), props, desc);

				pubPrx->restrictReplica(chnlInfo.netIds);
				pubPrx->setMaxBitrate(atol(maxBit.c_str()));

			}
		}
		catch (const TianShanIce::InvalidParameter& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "publish channel [%s] caught %s:%s<br>", 
				chnlInfo.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(AddChannel, "%s"), getLastError());
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "publish channel [%s] caught %s:%s<br>", 
				chnlInfo.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(AddChannel, "%s"), getLastError());
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "publish channel [%s] caught %s<br>", 
				chnlInfo.name.c_str(), ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(AddChannel, "%s"), getLastError());
			LinkToBcastMainPageError;
			responser.SetLastError(getLastError());
			return false;
		}

		// redirect page to BcastMainPage
		RedirectToBcastMainPage;

		return true;
	}
}