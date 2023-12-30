#include "ShowChannel.h"
#include "DataTypes.h"
#include "EdgeRM.h"
#include "TsEdgeResource.h"

namespace ErmWebPage
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

		const char* isenable = _reqCtx->GetRequestVar("isenable");
		if (isenable)
		{
			const char* names = _reqCtx->GetRequestVar("ChannelNames");
			TianShanIce::StrValues channelName;
			splitString(channelName, names, ";");
			bool bEnable = ((strcmp(isenable, "1") == 0) ? true : false);
			enableChannel(channelName, bEnable);
		}

		std::string devName; 
		devName = _varMap[DeviceNameKey];

		url.clear();
		url.setPath(ShowChannelPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());		
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id='ShowChannel' method='post' action='%s'><br>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		responser<< "<script type='text/javascript'>";
		responser << "function enableChannels(isEnable)\n"
			<< "{\n"
			<<      "var ChannelNames=\"\";\n"
			<<      "var obj=document.getElementsByName(\"enableChannel\");\n"
			<<      "for(var i=0;i<obj.length;i++)\n"
			<<      "{\n"
			<<           "if(obj[i].checked == true)\n"
			<<           "{\n"
			<<              "ChannelNames+=obj[i].value;\n"
			<<              "ChannelNames+=\";\";\n"
			<<           "}\n"
			<<      "}\n"
			<<      "document.getElementById(\"isenable\").value=isEnable;\n"
			<<      "document.getElementById(\"ChannelNames\").value=ChannelNames;\n"
			//<<      "alert(ChannelNames);\n"
			<<      "document.getElementById(\"ShowChannel\").submit();\n"
			<< "}\n";
		responser << "function selectAll()\n"
			<< "{\n"
			<<      "var all=document.getElementById(\"all\");\n"
			<<      "var obj=document.getElementsByName(\"enableChannel\");\n"
			<<      "for(var i=0;i<obj.length;i++)\n"
			<<      "{\n"
			<<          "obj[i].checked=all.checked"
			<<      "}\n"
			<< "}\n";
		responser << "function isEnableAll()\n"
			<< "{\n"
			<<      "var count=0;\n"
			<<      "var obj=document.getElementsByName(\"enableChannel\");\n"
			// <<      "alert(obj.length);\n"
			<<      "for(var i=0;i<obj.length;i++)\n"
			<<      "{\n"
			<<          "if (obj[i].checked==true)\n"
			<<          "{\n"
			<<              "count++;\n"
			<<          "}\n"
			<<      "}\n"
			<<      "if(count!=obj.length)\n"
			<<      "{\n"
			<<           "document.getElementById(\"all\").checked=false;\n"
			<<      "}\n"
			<<       "else\n"
			<<      "{\n"
			<<           "document.getElementById(\"all\").checked=true;\n"
			<<      "}\n"
			<< "}\n";
//		responser << "document.getElementById('toolbar').innerHTML = toolbar.build();\n";
		responser << "</script>\n";


		responser << " <select name=\"search_type\" size=\"1\" >";
		responser << "	<option value=\"device\" selected>Device";
		responser << " </select>";

		TianShanIce::EdgeResource::EdgeDeviceInfos deviceInfos;
		TianShanIce::StrValues expectedMetaData;

		if (!_varMap[DeviceNameKey].empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "	<input type='text' id='search_value' name='search_value' value='%s'/>", _varMap[DeviceNameKey].c_str());
			responser<<szBuf;
		}
		else
		{
			try
			{
				deviceInfos = _ERM->listDevices(expectedMetaData);
			}
			catch (const TianShanIce::BaseException& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list device caught %s:%s<br>", 
					ex.ice_name().c_str(), ex.message.c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			catch (const Ice::Exception& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list device caught %s<br>", 
					ex.ice_name().c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			if(deviceInfos.size())
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "	<input type='text' id='search_value' name='search_value' value='%s'/>", deviceInfos[0].ident.name.c_str());
				responser<<szBuf;
			}
			else
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "	<input type='text' id='search_value' name='search_value' value='%s'/>", _varMap[DeviceNameKey].c_str());
				responser<<szBuf;
			}
		}

		// the submit button
		responser<<"	<input type='submit' value='search'/>&nbsp&nbsp\n";
		responser<<"    <input type=\"checkbox\" name=\"all\" id=\"all\" value=\"all\" onclick=\"selectAll()\"/>all&nbsp&nbsp\n"
			<< "<input type=\"button\" value=\"enable\" onclick=\"enableChannels(1)\"/>&nbsp&nbsp\n"
			<< "<input type=\"button\" value=\"disable\" onclick=\"enableChannels(0)\"/>\n"
			<< "<input type=\"hidden\" name=\"isenable\" id=\"isenable\" value=\"\"/>\n"
			<< "<input type=\"hidden\" name=\"ChannelNames\" id=\"ChannelNames\"/>\n";

// 		url.clear();
// 		url.setPath(ShowChannelPage);
// 		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
// 		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
// 		url.setVar(DeviceNameKey, devName.c_str());
// 		url.setVar(EdgePortKey, _varMap[EdgePortKey].c_str());
// 		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Refresh</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
// 		responser<<szBuf;

		// write </form>
		responser<<"</form>";



		// write start table flag
		responser<<"<table class='listTable'>";

		// write table header line
		responser<<"<tr>";
		responser<<"	<th><center>QAM<br>Channel</center></th>";
		responser<<"	<th><center>State</center></th>";
		responser<<"	<th><center>Enabled</center></th>";
		responser<<"	<th><center>RF Freq<br>(Khz)</center></th>";
		responser<<"	<th><center>Symbol<br>Rate</center></th>";
// 		responser<<"	<th><center>Power Level<br>(dBmv)</center></th>";
// 		responser<<"	<th><center>Modulation<br>Format</center></th>";
// 		responser<<"	<th><center>InterLeaver<br>Mode</center></th>";
// 		responser<<"	<th><center>FEC</center></th>";
// 		responser<<"	<th><center>Symbol<br>Rate</center></th>";
// 		responser<<"	<th><center>Device Name</center></th>";
// 		responser<<"	<th><center>Device IP</center></th>";
// 		responser<<"	<th><center>Device Group</center></th>";
		responser<<"	<th><center>TSID</center></th>";
		responser<<"	<th><center>PAT<br>Interval</center></th>";
		responser<<"	<th><center>PMT<br>Interval</center></th>";
		responser<<"	<th><center>NITPID</center></th>";
		responser<<"	<th><center>Start</br>UDP Port</center></th>";
		responser<<"	<th><center>Start<br>Program Number</center></th>";
		responser<<"	<th><center>Max Sessions</center></th>";
		responser<<"	<th><center>Low</br>Bandwidth Utilization</center></th>";
		responser<<"	<th><center>High</br>Bandwidth Utilization</center></th>";
		responser<<"	<th><center>Last Updated</center></th>";
		responser<<"	<th><center>Allocation</center></th>";
		responser<<"</tr>";

		if (devName.empty())
		{
// 			TianShanIce::EdgeResource::EdgeDeviceInfos deviceInfos;
// 			TianShanIce::StrValues expectedMetaData;
// 			expectedMetaData.push_back(Zone);
// 			expectedMetaData.push_back(Vendor);
// 			expectedMetaData.push_back(Model);
// 			expectedMetaData.push_back(Description);
// 			expectedMetaData.push_back(TFTP);
// 			expectedMetaData.push_back(AdminUrl);
// 			try
// 			{
// 				deviceInfos = _ERM->listDevices(expectedMetaData);
// 			}
// 			catch (const TianShanIce::BaseException& ex)
// 			{
// 				snprintf(szBuf, sizeof(szBuf) - 1, "list device caught %s:%s<br>", 
// 					ex.ice_name().c_str(), ex.message.c_str());
// 				setLastError(szBuf);
// 				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
// 				responser.SetLastError(getLastError());
// 				return false;
// 			}
// 			catch (const Ice::Exception& ex)
// 			{
// 				snprintf(szBuf, sizeof(szBuf) - 1, "list device caught %s<br>", 
// 					ex.ice_name().c_str());
// 				setLastError(szBuf);
// 				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
// 				responser.SetLastError(getLastError());
// 				return false;
// 			}
			// write table data line
// 			for (DeviceInfos_iter it = deviceInfos.begin(); it != deviceInfos.end(); it++)
// 			{
// 				EdgeDeviceInfo deviceInfo = *it;
// 				showChannelInfo(responser, deviceInfo.ident.name);
// 			}
			if(deviceInfos.size())
				showChannelInfo(responser, deviceInfos[0].ident.name);
		}
		else
		{
			showChannelInfo(responser, devName);
		}


		// write end table flag
		responser<<"</table>";
		responser<<"<br>";
		return true;
	}

	bool ShowChannel::post()
	{
		IHttpResponse& responser = _reqCtx->Response();

		const char* isenable = _reqCtx->GetRequestVar("isenable");
		if (isenable)
		{
			const char* names = _reqCtx->GetRequestVar("ChannelNames");
			TianShanIce::StrValues channelsName;
			splitString(channelsName, names, ";");
			bool bEnable = ((strcmp(isenable, "1") == 0) ? true : false);
			enableChannel(channelsName, bEnable);
		}

		std::string devName; 
		if(_varMap["search_type"] == "device")
			devName = _varMap["search_value"];

		url.clear();
		url.setPath(ShowChannelPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());		
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id='ShowChannel' method='post' action='%s'><br>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		responser<< "<script type='text/javascript'>";
		responser << "function enableChannels(isEnable)\n"
			<< "{\n"
			<<      "var ChannelNames=\"\";\n"
			<<      "var obj=document.getElementsByName(\"enableChannel\");\n"
			<<      "for(var i=0;i<obj.length;i++)\n"
			<<      "{\n"
			<<           "if(obj[i].checked == true)\n"
			<<           "{\n"
			<<              "ChannelNames+=obj[i].value;\n"
			<<              "ChannelNames+=\";\";\n"
			<<           "}\n"
			<<      "}\n"
			<<      "document.getElementById(\"isenable\").value=isEnable;\n"
			<<      "document.getElementById(\"ChannelNames\").value=ChannelNames;\n"
			//<<      "alert(ChannelNames);\n"
			<<      "document.getElementById(\"ShowChannel\").submit();\n"
			<< "}\n";
		responser << "function selectAll()\n"
			<< "{\n"
			<<      "var all=document.getElementById(\"all\");\n"
			<<      "var obj=document.getElementsByName(\"enableChannel\");\n"
			<<      "for(var i=0;i<obj.length;i++)\n"
			<<      "{\n"
			<<          "obj[i].checked=all.checked"
			<<      "}\n"
			<< "}\n";
		responser << "function isEnableAll()\n"
			<< "{\n"
			<<      "var count=0;\n"
			<<      "var obj=document.getElementsByName(\"enableChannel\");\n"
			// <<      "alert(obj.length);\n"
			<<      "for(var i=0;i<obj.length;i++)\n"
			<<      "{\n"
			<<          "if (obj[i].checked==true)\n"
			<<          "{\n"
			<<              "count++;\n"
			<<          "}\n"
			<<      "}\n"
			<<      "if(count!=obj.length)\n"
			<<      "{\n"
			<<           "document.getElementById(\"all\").checked=false;\n"
			<<      "}\n"
			<<       "else\n"
			<<      "{\n"
			<<           "document.getElementById(\"all\").checked=true;\n"
			<<      "}\n"
			<< "}\n";
//		responser << "document.getElementById('toolbar').innerHTML = toolbar.build();\n";
		responser << "</script>\n";


		responser << " <select name=\"search_type\" size=\"1\" >";
		responser << "	<option value=\"device\" selected>Device";
		responser << " </select>";

		snprintf(szBuf, sizeof(szBuf) - 1, "	<input type='text' id='search_value' name='search_value' value='%s'/>", _varMap["search_value"].c_str());
		responser<<szBuf;

		// the submit button
		responser<<"	<input type='submit' value='search'/>&nbsp&nbsp\n";
		responser<<"    <input type=\"checkbox\" name=\"all\" id=\"all\" value=\"all\" onclick=\"selectAll()\"/>all&nbsp&nbsp\n"
			<< "<input type=\"button\" value=\"enable\" onclick=\"enableChannels(1)\"/>&nbsp&nbsp\n"
			<< "<input type=\"button\" value=\"disable\" onclick=\"enableChannels(0)\"/>\n"
			<< "<input type=\"hidden\" name=\"isenable\" id=\"isenable\" value=\"\"/>\n"
			<< "<input type=\"hidden\" name=\"ChannelNames\" id=\"ChannelNames\"/>\n";

		// write </form>
		responser<<"</form>";

		// write start table flag
		responser<<"<table class='listTable'>";

		// write table header line
		responser<<"<tr>";
		responser<<"	<th><center>QAM<br>Channel</center></th>";
/*		responser<<"	<th><center>Admin<br>State</center></th>";*/
		responser<<"	<th><center>State</center></th>";
		responser<<"	<th><center>Enabled</center></th>";
		responser<<"	<th><center>RF Freq<br>(Khz)</center></th>";
		responser<<"	<th><center>Symbol<br>Rate</center></th>";
		responser<<"	<th><center>TSID</center></th>";
		responser<<"	<th><center>PAT<br>Interval</center></th>";
		responser<<"	<th><center>PMT<br>Interval</center></th>";
		responser<<"	<th><center>NITPID</center></th>";
		responser<<"	<th><center>Start</br>UDP Port</center></th>";
		responser<<"	<th><center>Start<br>Program Number</center></th>";
		responser<<"	<th><center>Max Sessions</center></th>";
		responser<<"	<th><center>Low</br>Bandwidth Utilization</center></th>";
		responser<<"	<th><center>High</br>Bandwidth Utilization</center></th>";
/*		responser<<"	<th><center>Enabled</center></th>";*/
		responser<<"	<th><center>Last Updated</center></th>";
		responser<<"	<th><center>Allocation</center></th>";
		responser<<"</tr>";

		if (devName.empty())
		{
			TianShanIce::EdgeResource::EdgeDeviceInfos deviceInfos;
			TianShanIce::StrValues expectedMetaData;
			expectedMetaData.push_back(Zone);
			expectedMetaData.push_back(Vendor);
			expectedMetaData.push_back(Model);
			expectedMetaData.push_back(Description);
			expectedMetaData.push_back(TFTP);
			expectedMetaData.push_back(AdminUrl);
			try
			{
				deviceInfos = _ERM->listDevices(expectedMetaData);
			}
			catch (const TianShanIce::BaseException& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list device caught %s:%s<br>", 
					ex.ice_name().c_str(), ex.message.c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			catch (const Ice::Exception& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list device caught %s<br>", 
					ex.ice_name().c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			// write table data line
			for (DeviceInfos_iter it = deviceInfos.begin(); it != deviceInfos.end(); it++)
			{
				EdgeDeviceInfo deviceInfo = *it;
				showChannelInfo(responser, deviceInfo.ident.name);
			}
		}
		else
		{
			showChannelInfo(responser, devName);
		}


		// write end table flag
		responser<<"</table>";
		responser<<"<br>";
		/*		
		url.clear();
		url.setPath(AddChannelPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Add</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;
		*/
		return true;
	}

	bool ShowChannel::showChannelInfo(IHttpResponse& responser, std::string devName)
	{

		std::string edgePort; 
		edgePort = _varMap[EdgePortKey];
		int port = atoi(edgePort.c_str());
		TianShanIce::EdgeResource::EdgePortInfos  portInfos;
		if(port <= 0)
		{
			try
			{
				TianShanIce::EdgeResource::EdgeDevicePrx devicePrx; 
				devicePrx = _ERM->openDevice(devName);
				portInfos = devicePrx->listEdgePorts();
			}
			catch (const TianShanIce::BaseException& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list edge port caught %s:%s<br>", 
					ex.ice_name().c_str(), ex.message.c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			catch (const Ice::Exception& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list edge port caught %s<br>", 
					ex.ice_name().c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			for (PortInfos_iter it = portInfos.begin(); it != portInfos.end(); it++)
			{
				showChannelInfo(responser, devName, it->Id);
			}
		}
		else
		{
			showChannelInfo(responser, devName, port);
		}

		return true;
	}

	bool ShowChannel::showChannelInfo(IHttpResponse& responser, std::string devName, short port)
	{
		TianShanIce::EdgeResource::EdgeChannelInfos channelInfos;
		TianShanIce::EdgeResource::EdgePortInfos portInfos;

		try
		{
			TianShanIce::EdgeResource::EdgeDevicePrx devicePrx; 
			devicePrx = _ERM->openDevice(devName);
			portInfos = devicePrx->listEdgePorts();
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "list edge port caught %s:%s<br>", 
				ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowEdgePort, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "list edge port caught %s<br>", 
				ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowEdgePort, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		std::string sPower = "";
		std::string sModulation = "";
		std::string sMode = "";
		std::string sFec = "";
		std::string sSymbolRate = "";
		std::string sEdgeDeviceName = "";
		std::string sEdgeDeviceIP = "";
		std::string sEdgeDeviceGroup = "";
		TianShanIce::ValueMap::const_iterator vMap_itor; // value map iterator

		for (PortInfos_iter it = portInfos.begin(); it != portInfos.end(); it++)
		{
			TianShanIce::EdgeResource::EdgePort portInfo = *it;
			if(portInfo.Id == port)
			{

				int intFormat = -1;
				vMap_itor = portInfo.resAtscModulationMode.resourceData.find(Modulation);
				if (portInfo.resAtscModulationMode.resourceData.end() != vMap_itor)
				{
					const TianShanIce::Variant& formatVar = vMap_itor->second;
					if (TianShanIce::vtBin == formatVar.type && formatVar.bin.size() > 0)
						intFormat = formatVar.bin[0];
				}

				int depth = -1;
				vMap_itor = portInfo.resAtscModulationMode.resourceData.end();
				vMap_itor = portInfo.resAtscModulationMode.resourceData.find(Depth);
				if (portInfo.resAtscModulationMode.resourceData.end() != vMap_itor)
				{
					const TianShanIce::Variant& depthVar = vMap_itor->second;
					if (TianShanIce::vtBin == depthVar.type && depthVar.bin.size() > 0)
						depth = depthVar.bin[0];
				}

				int symbolRate = -1;
				vMap_itor = portInfo.resAtscModulationMode.resourceData.end();
				vMap_itor = portInfo.resAtscModulationMode.resourceData.find(SymRate);
				if (portInfo.resAtscModulationMode.resourceData.end() != vMap_itor)
				{
					const TianShanIce::Variant& rateVar = vMap_itor->second;
					if (TianShanIce::vtInts == rateVar.type && rateVar.ints.size() > 0)
						symbolRate = rateVar.ints[0];
				}

				int md = -1;
				vMap_itor = portInfo.resAtscModulationMode.resourceData.end();
				vMap_itor = portInfo.resAtscModulationMode.resourceData.find(Mode);
				if (portInfo.resAtscModulationMode.resourceData.end() != vMap_itor)
				{
					const TianShanIce::Variant& modeVar = vMap_itor->second;
					if (TianShanIce::vtBin == modeVar.type && modeVar.bin.size() > 0)
						md = modeVar.bin[0];
				}

				int fec = -1;
				vMap_itor = portInfo.resAtscModulationMode.resourceData.end();
				vMap_itor = portInfo.resAtscModulationMode.resourceData.find(Fec);
				if (portInfo.resAtscModulationMode.resourceData.end() != vMap_itor)
				{
					const TianShanIce::Variant& fecVar = vMap_itor->second;
					if (TianShanIce::vtBin == fecVar.type && fecVar.bin.size() > 0)
						fec = fecVar.bin[0];
				}

				std::string edgeDeviceName;
				vMap_itor = portInfo.resPhysicalChannel.resourceData.find(EDeviceName);
				if (portInfo.resPhysicalChannel.resourceData.end() != vMap_itor)
				{
					const TianShanIce::Variant& dnVar = vMap_itor->second;
					if (TianShanIce::vtStrings == dnVar.type && dnVar.strs.size() > 0)
						edgeDeviceName = dnVar.strs[0];
				}

				std::string edgeDeviceIP;
				vMap_itor = portInfo.resPhysicalChannel.resourceData.find(EDeviceIP);
				if (portInfo.resPhysicalChannel.resourceData.end() != vMap_itor)
				{
					const TianShanIce::Variant& ipVar = vMap_itor->second;
					if (TianShanIce::vtStrings == ipVar.type && ipVar.strs.size() > 0)
						edgeDeviceIP = ipVar.strs[0];
				}

				std::string edgeDeviceGroup;
				vMap_itor = portInfo.resPhysicalChannel.resourceData.find(EDeviceGroup);
				if (portInfo.resPhysicalChannel.resourceData.end() != vMap_itor)
				{
					const TianShanIce::Variant& dgVar = vMap_itor->second;
					if (TianShanIce::vtStrings == dgVar.type && dgVar.strs.size() > 0)
						edgeDeviceGroup = dgVar.strs[0];
				}

				char temp[256] = {0};
				sPower = portInfo.powerLevel;
				sModulation = switchModulation(intFormat);
				sMode = switchMode(md);
				sFec = switchFec(fec);
				itoa(symbolRate, temp, 10);
				sSymbolRate = temp;
				sEdgeDeviceGroup = edgeDeviceName;
				sEdgeDeviceIP = edgeDeviceIP;
				sEdgeDeviceName = edgeDeviceName;
				break;
			}
		}

		TianShanIce::State state;
		TianShanIce::StrValues expectedMetaData;
		expectedMetaData.push_back(RF);
		expectedMetaData.push_back(SYS_PROP(symbolRate));
		expectedMetaData.push_back(TSID);
		expectedMetaData.push_back(PAT_Interval);
		expectedMetaData.push_back(PMT_Interval);
		expectedMetaData.push_back(LastUpdated);
		expectedMetaData.push_back(NITPID);
		expectedMetaData.push_back(StartUDP);
		expectedMetaData.push_back(UdpSBP);
		expectedMetaData.push_back(PN);
		expectedMetaData.push_back(MaxSessions);
		expectedMetaData.push_back(LBandWidth);
		expectedMetaData.push_back(HBandWidth);
		expectedMetaData.push_back(Enabled);
		TianShanIce::EdgeResource::EdgeDevicePrx devicePrx;
		try
		{ 
			devicePrx = _ERM->openDevice(devName);
			channelInfos = devicePrx->listChannels(port, expectedMetaData, false);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "list channel caught %s:%s<br>", 
				ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "list channel caught %s<br>", 
				ex.ice_name().c_str());
			setLastError(szBuf);
			responser.SetLastError(getLastError());
			return false;
		}

		// write table data line
		for (ChannelInfos_iter it = channelInfos.begin(); it != channelInfos.end(); it++)
		{
			responser<<"<tr>";

			EdgeChannelInfo channelInfo = *it;

			TianShanIce::EdgeResource::AllocationInfos allocationInfos;
			TianShanIce::StrValues expectedMetaData;

			short portNum = 0;
			short chNum = 0;
			string chName = channelInfo.ident.name;
			chName = chName.substr(chName.find_last_of('/')+1, chName.find_last_of('.')-chName.find_last_of('/'));
			portNum = atoi(chName.c_str());
			string cn = channelInfo.ident.name;
			cn = cn.substr(cn.find_last_of('.')+1, cn.size());
			chNum = atoi(cn.c_str());
			try
			{ 
				TianShanIce::EdgeResource::EdgeChannelPrx channelPrx;
				channelPrx = devicePrx->openChannel(port, chNum);
				state = channelPrx->getState();
				allocationInfos = _ERM->listAllocations(devName, port, chNum, expectedMetaData);
			}
			catch (const TianShanIce::BaseException& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list edge port caught %s:%s<br>", 
					ex.ice_name().c_str(), ex.message.c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			catch (const Ice::Exception& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list edge port caught %s<br>", 
					ex.ice_name().c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}

			// show channel name
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><input type='checkbox' name='enableChannel' id='enableChannel' onclick='isEnableAll()' value='%s' style='width:auto'>", channelInfo.ident.name.c_str());
			responser<<szBuf;
			// show a link to detail
			url.clear();
			url.setPath(ChannelDetailPage);
			url.setVar(ChannelNameKey, channelInfo.ident.name.c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(DeviceNameKey, devName.c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\">%s</a></center></td>", String::getRightStr(url.generate(), "/", false).c_str(), channelInfo.ident.name.c_str());
			responser<<szBuf;


			// show admin state
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", switchState(state).c_str());
			responser<<szBuf;

			// show Enabled
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", channelInfo.props[Enabled].c_str());
			responser<<szBuf;

			// show RF Freq
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", channelInfo.props[RF].c_str());
			responser<<szBuf;

			// show RF SymRate
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", channelInfo.props[SYS_PROP(symbolRate)].c_str());
			responser<<szBuf;
			/*
			// show power level
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", sPower.c_str());
			responser<<szBuf;

			// show modulation format
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", sModulation.c_str());
			responser<<szBuf;

			// show interleaver mode
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", sMode.c_str());
			responser<<szBuf;

			// show FEC
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", sFec.c_str());
			responser<<szBuf;

			// show SymbolRate
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", sSymbolRate.c_str());
			responser<<szBuf;

			// show EdgeDeviceName
			//			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", sEdgeDeviceName.c_str());
			//			responser<<szBuf;
			// show a link to EdgeDeviceName
			url.clear();
			url.setPath(ShowDevicePage);
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(EDeviceName, _varMap[EDeviceName].c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><a href=\"%s\">%s</a></center></td>", String::getRightStr(url.generate(), "/", false).c_str(), sEdgeDeviceName.c_str());
			responser<<szBuf;

			// show EdgeDeviceIP
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", sEdgeDeviceIP.c_str());
			responser<<szBuf;

			// show EdgeDeviceGroup
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", sEdgeDeviceGroup.c_str());
			responser<<szBuf;
			*/
			// show TSID
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", channelInfo.props[TSID].c_str());
			responser<<szBuf;

			// show PAT interval
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", channelInfo.props[PAT_Interval].c_str());
			responser<<szBuf;

			// show PMT interval
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", channelInfo.props[PMT_Interval].c_str());
			responser<<szBuf;

			// show NITPID
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", channelInfo.props[NITPID].c_str());
			responser<<szBuf;

			// show UDP Port
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", channelInfo.props[StartUDP].c_str());
			responser<<szBuf;

			// show Start Program Number
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", channelInfo.props[PN].c_str());
			responser<<szBuf;

			// show Max Sessions
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", channelInfo.props[MaxSessions].c_str());
			responser<<szBuf;

			// show Low Bandwidth Utilization
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%lld</center></td>", _atoi64(channelInfo.props[LBandWidth].c_str())/1000);
			responser<<szBuf;

			// show High Bandwidth Utilization
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%lld</center></td>", _atoi64(channelInfo.props[HBandWidth].c_str())/1000);
			responser<<szBuf;

			// show Last Updated
			char timeBuffer[128];
			memset(timeBuffer,'\0',sizeof(timeBuffer));
			ZQ::common::TimeUtil::TimeToUTC(_atoi64(channelInfo.props[LastUpdated].c_str()),timeBuffer,sizeof(timeBuffer),true);
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", timeBuffer);
			responser<<szBuf;
			/*
			// show a link to edit item page
			url.clear();
			url.setPath(EditChannelPage);
			url.setVar(ChannelNameKey, channelInfo.ident.name.c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center><a href=\"%s\">Edit</a>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;

			// show a link to delete item page
			url.clear();
			url.setPath(RemoveChannelPage);
			url.setVar(ChannelNameKey, channelInfo.ident.name.c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<br><a href=\"%s\">Remove</a></center>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;
			*/
			// show a link to show allocation
			url.clear();
			url.setPath(ShowAllocationPage);
			url.setVar(ChannelNameKey, channelInfo.ident.name.c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(TemplateKey, neighborLayout(_varMap[TemplateKey].c_str(), "LOCAL_ERM_ALLOCATION").c_str());
			url.setVar(DeviceNameKey, devName.c_str()); 
			char temp[64] = {0};
			snprintf(temp, sizeof(temp), "%d", port);
			url.setVar(EdgePortKey, temp);
			snprintf(temp, sizeof(temp), "%d", chNum);
			url.setVar(ChannelNumberKey, temp);
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><a href=\"%s\">%d</a></center></td>", String::getRightStr(url.generate(), "/", false).c_str(), allocationInfos.size());
			responser<<szBuf;

			responser<<"</tr>";
		}
		
		return true;
	}

	bool ShowChannel::enableChannel(TianShanIce::StrValues& channelNames, bool bEnable)
	{
		if(channelNames.size() <= 0)
			return false;
		for(int i=0; i<channelNames.size(); i++)
		{
			try
			{
				TianShanIce::EdgeResource::EdgeRMPrx edgeRM = TianShanIce::EdgeResource::EdgeRMPrx::uncheckedCast(_ERM);
				TianShanIce::EdgeResource::EdgeChannelPrx channelPrx = edgeRM->openChannel(channelNames[i]);
				channelPrx->enable(bEnable);
			}
			catch (const Ice::Exception& ex)
			{
				continue;
			}
		}

		return true;
	}

} // namespace ErmWebPage

