#include "ShowEdgePort.h"
#include "DataTypes.h"
#include "strHelper.h"

namespace ErmWebPage
{
	ShowEdgePort::ShowEdgePort(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
		bGet = true;
	}

	ShowEdgePort::~ShowEdgePort()
	{
	}

	bool ShowEdgePort::get()
	{
		IHttpResponse& responser = _reqCtx->Response();
		bGet = true;
		show(responser);
		return true;
	}

	bool ShowEdgePort::post()
	{
		IHttpResponse& responser = _reqCtx->Response();
		bGet = false;
		show(responser);	
		return true;
	}

	bool ShowEdgePort::show(IHttpResponse& responser)
	{
		std::string devName; 
		devName = _varMap[DeviceNameKey];

		url.clear();
		url.setPath(ShowEdgePortPage);
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());		
		snprintf(szBuf, sizeof(szBuf) - 1, "<form id='ShowEdgePort' method='post' action='%s'><br>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		if(bGet)
		{
			if(!_varMap[RouteNamesKey].empty())
			{
				responser << " <select name=\"search_type\" size=\"1\" >";
				responser << "	<option value=\"device\">Device";
				responser << "	<option value=\"servicegroup\" selected>ServiceGroup";
				responser << " </select>";

				snprintf(szBuf, sizeof(szBuf) - 1, "	<input type='text' id='search_value' name='search_value' value='%s'/>", _varMap[RouteNamesKey].c_str());
				responser<<szBuf;
			}
			else
			{
				responser << " <select name=\"search_type\" size=\"1\" >";
				responser << "	<option value=\"device\" selected>Device";
				responser << "	<option value=\"servicegroup\">ServiceGroup";
				responser << " </select>";
				if (!_varMap[DeviceNameKey].empty())
				{
					snprintf(szBuf, sizeof(szBuf) - 1, "	<input type='text' id='search_value' name='search_value' value='%s'/>", _varMap[DeviceNameKey].c_str());
					responser<<szBuf;
				}
				else
				{
					TianShanIce::EdgeResource::EdgeDeviceInfos deviceInfos;
					TianShanIce::StrValues expectedMetaData;
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
						snprintf(szBuf, sizeof(szBuf) - 1, "	<input type='text' id='search_value' name='search_value' value=''/>");
						responser<<szBuf;
					}
				}
			}

		}
		else
		{
			responser << " <select name=\"search_type\" size=\"1\" >";
			if(_varMap["search_type"] == "device")
			{
				responser << "	<option value=\"device\" selected>Device";
				responser << "	<option value=\"servicegroup\">ServiceGroup";
			}
			if(_varMap["search_type"] == "servicegroup")
			{
				responser << "	<option value=\"device\">Device";
				responser << "	<option value=\"servicegroup\" selected>ServiceGroup";
			}
			responser << " </select>";

			snprintf(szBuf, sizeof(szBuf) - 1, "	<input type='text' id='search_value' name='search_value' value='%s'/>", _varMap["search_value"].c_str());
			responser<<szBuf;
		}

		// the submit button
		responser<< "	<input type='submit' value='search'/>&nbsp&nbsp\n";
		responser<< "	<input type=\"hidden\" name=\"DeviceNames\" id=\"DeviceNames\"/>\n";

// 		url.clear();
// 		url.setPath(ShowEdgePortPage);
// 		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
// 		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
// 		url.setVar(DeviceNameKey, devName.c_str());
// 		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Refresh</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
// 		responser<<szBuf;

		// write </form>
		responser<<"</form>";

		// write start table flag
		responser<<"<table class='listTable'>";

		// write table header line
		responser<<"<tr>";
		responser<<"	<th><center>Port ID</center></th>";
		responser<<"	<th><center>Power Level<br>(dBmv)</center></th>";
		responser<<"	<th><center>Modulation<br>Format</center></th>";
//		responser<<"	<th><center>Symbol<br>Rate</center></th>";
		responser<<"	<th><center>InterLeaver<br>Mode</center></th>";
		responser<<"	<th><center>FEC</center></th>";
		responser<<"	<th><center>DeviceIP</center></th>";
		responser<<"	<th><center>DeviceGroup</center></th>";
		responser<<"	<th><center>RouteNames</center></th>";
		responser<<"	<th><center>Channels</center></th>";
		responser<<"</tr>";

		std::string serviceGroup; 
		serviceGroup = _varMap[RouteNamesKey];
		if(!serviceGroup.empty())
		{
			showPortByServiceGroup(responser, serviceGroup);
			return true;
		}

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
				snprintf(szBuf, sizeof(szBuf) - 1, "list port caught %s:%s<br>", 
					ex.ice_name().c_str(), ex.message.c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			catch (const Ice::Exception& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "list port caught %s<br>", 
					ex.ice_name().c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
				responser.SetLastError(getLastError());
				return false;
			}
			if(bGet)
			{
				if(deviceInfos.size())
					showPortInfo(responser, deviceInfos[0].ident.name);
			}
			else
			{
				// write table data line
				for (DeviceInfos_iter it = deviceInfos.begin(); it != deviceInfos.end(); it++)
				{
					EdgeDeviceInfo deviceInfo = *it;
					showPortInfo(responser, deviceInfo.ident.name);
				}
			}

		}
		else
		{
			showPortInfo(responser, devName);
		}

		// write end table flag
		responser<<"</table>";
		responser<<"<br>";

		if(!devName.empty())
		{
			url.clear();
			url.setPath(ShowDevicePage);
//			url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
			url.setVar(TemplateKey, neighborLayout(_varMap[TemplateKey].c_str(), "LOCAL_ERM_DEVICE").c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(DeviceNameKey, devName.c_str());
			snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Back to Device</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
			responser<<szBuf;
		}
		return true;
	}

	bool ShowEdgePort::showPortInfo(IHttpResponse& responser, std::string devName)
	{

		if(_varMap["search_type"] == "device" && !_varMap["search_value"].empty())
		{
			if(devName != _varMap["search_value"])
				return true;
		}

		TianShanIce::ValueMap::const_iterator vMap_itor; // value map iterator
		TianShanIce::EdgeResource::EdgePortInfos  portInfos;
		TianShanIce::EdgeResource::EdgeDevicePrx devicePrx;
		try
		{
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
		catch (...)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "list edge port caught unknown error<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowEdgePort, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		// write table data line
		for (PortInfos_iter it = portInfos.begin(); it != portInfos.end(); it++)
		{
//			std::string edgeServiceGroup;
			std::map<std::string, int> mapSG;
			::TianShanIce::EdgeResource::RoutesMap routes = devicePrx->getRoutesRestriction(it->Id);
			for(::TianShanIce::EdgeResource::RoutesMap::const_iterator iter = routes.begin(); iter != routes.end(); iter++)
			{
				/*char buf[256] = {0};
				snprintf(buf, sizeof(buf)-1, "%d", iter->first);*/
				mapSG.insert(make_pair( iter->first.c_str(), 1));
// 				if((iter + 1) != groups.end())
// 					edgeServiceGroup = edgeServiceGroup + buf  + ";";
// 				else
// 					edgeServiceGroup = edgeServiceGroup + buf;
			}

			if(_varMap["search_type"] == "servicegroup" && !_varMap["search_value"].empty())
			{
				if(mapSG.find(_varMap["search_value"]) == mapSG.end())
					continue;
			}

			responser<<"<tr>";

			TianShanIce::EdgeResource::EdgePort portInfo = *it;

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

			int mode = -1;
			vMap_itor = portInfo.resAtscModulationMode.resourceData.end();
			vMap_itor = portInfo.resAtscModulationMode.resourceData.find(Mode);
			if (portInfo.resAtscModulationMode.resourceData.end() != vMap_itor)
			{
				const TianShanIce::Variant& modeVar = vMap_itor->second;
				if (TianShanIce::vtBin == modeVar.type && modeVar.bin.size() > 0)
					mode = modeVar.bin[0];
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

			TianShanIce::StrValues expectedMetaData;
			TianShanIce::EdgeResource::EdgeChannelInfos channelInfos;
			channelInfos = devicePrx->listChannels(it->Id, expectedMetaData, false);

			// show RF port id
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s/%d</center></td>", edgeDeviceName.c_str(), portInfo.Id);
			responser<<szBuf;

			// show powerLevel
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%d</center></td>", portInfo.powerLevel);
			responser<<szBuf;

			// show modulationFormat
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", switchModulation(intFormat).c_str());
			responser<<szBuf;

			// show interleaverLevel
			//snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", switchLevel(portInfo.interleaverLevel).c_str());
			//responser<<szBuf;

			// show symbol rate
//			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%d</center></td>", symbolRate);
//			responser<<szBuf;

			// show interleaverMode
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", switchMode(depth).c_str());
			responser<<szBuf;

			// show FEC
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", switchFec(fec).c_str());
			responser<<szBuf;

			// show device ip
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", edgeDeviceIP.c_str());
			responser<<szBuf;

			// show device group
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", edgeDeviceGroup.c_str());
			responser<<szBuf;

			char id[256] = {0};
			itoa(portInfo.Id, id, 10);

			url.clear();
			url.setPath(EditRouteNamesPage);
			url.setVar(TemplateKey, neighborLayout(_varMap[TemplateKey].c_str(), "LOCAL_ERM_ROUTENAMES").c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(DeviceNameKey, devName.c_str());
			url.setVar(EdgePortKey, id);
			// show a link to restrict service group
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center><a href=\"%s\">%d</a></td>", String::getRightStr(url.generate(), "/", false).c_str(), routes.size());
			responser<<szBuf;

			url.clear();
			url.setPath(ShowChannelPage);
			url.setVar(TemplateKey, neighborLayout(_varMap[TemplateKey].c_str(), "LOCAL_ERM_CHANNEL").c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(DeviceNameKey, devName.c_str());
			url.setVar(EdgePortKey, id);
			if(portInfo.Id >= 0)
			{
				itoa(portInfo.powerLevel, id, 10);
				url.setVar(Power, id);
				url.setVar(Modulation, switchModulation(intFormat).c_str());
				url.setVar(Mode, switchMode(mode).c_str());
				url.setVar(Fec, switchFec(fec).c_str());
				itoa(symbolRate, id, 10);
				url.setVar(SymRate, id);
				url.setVar(EDeviceName, edgeDeviceName.c_str());
				url.setVar(EDeviceIP, edgeDeviceIP.c_str());
				url.setVar(EDeviceGroup, edgeDeviceGroup.c_str());
			}

			// show a link to list channels
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center><a href=\"%s\">%d</a></td>", String::getRightStr(url.generate(), "/", false).c_str(), channelInfos.size());
			responser<<szBuf;

			responser<<"</tr>";
		}
		return true;
	}

	bool ShowEdgePort::showPortByServiceGroup(IHttpResponse& responser, std::string routeName)
	{

		TianShanIce::EdgeResource::EdgePortInfos  portInfos;
		TianShanIce::ValueMap::const_iterator vMap_itor; // value map iterator
		try
		{
			portInfos = _ERM->findRFPortsByRouteName(routeName);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "find RF port by Service Group caught %s:%s<br>", 
				ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowEdgePort, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "find RF port by Service Group caught %s<br>", 
				ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowEdgePort, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		catch (...)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "find RF port by Service Group caught unknown error<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowEdgePort, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		// write table data line
		for (PortInfos_iter it = portInfos.begin(); it != portInfos.end(); it++)
		{
			responser<<"<tr>";

			TianShanIce::EdgeResource::EdgePort portInfo = *it;

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

			int mode = -1;
			vMap_itor = portInfo.resAtscModulationMode.resourceData.end();
			vMap_itor = portInfo.resAtscModulationMode.resourceData.find(Mode);
			if (portInfo.resAtscModulationMode.resourceData.end() != vMap_itor)
			{
				const TianShanIce::Variant& modeVar = vMap_itor->second;
				if (TianShanIce::vtBin == modeVar.type && modeVar.bin.size() > 0)
					mode = modeVar.bin[0];
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

			TianShanIce::EdgeResource::EdgeDevicePrx devicePrx;
			TianShanIce::StrValues expectedMetaData;
			TianShanIce::EdgeResource::EdgeChannelInfos channelInfos;
			::TianShanIce::EdgeResource::RoutesMap routes;
			try
			{
				devicePrx = _ERM->openDevice(edgeDeviceName);
				channelInfos = devicePrx->listChannels(it->Id, expectedMetaData, false);
				routes = devicePrx->getRoutesRestriction(it->Id);
			}
			catch (const TianShanIce::BaseException& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "open Device caught %s:%s<br>", 
					ex.ice_name().c_str(), ex.message.c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowEdgePort, "%s"), getLastError());
				responser.SetLastError(getLastError());
				continue;
			}
			catch (const Ice::Exception& ex)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "open Device caught %s<br>", 
					ex.ice_name().c_str());
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowEdgePort, "%s"), getLastError());
				responser.SetLastError(getLastError());
				continue;
			}
			catch (...)
			{
				snprintf(szBuf, sizeof(szBuf) - 1, "open Device caught unknown error<br>");
				setLastError(szBuf);
				glog(ErrorLog, CLOGFMT(ShowEdgePort, "%s"), getLastError());
				responser.SetLastError(getLastError());
				continue;
			}	

			// show RF port id
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s/%d</center></td>", edgeDeviceName.c_str(), portInfo.Id);
			responser<<szBuf;

			// show powerLevel
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%d</center></td>", portInfo.powerLevel);
			responser<<szBuf;

			// show modulationFormat
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", switchModulation(intFormat).c_str());
			responser<<szBuf;

			// show interleaverLevel
			//snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", switchLevel(portInfo.interleaverLevel).c_str());
			//responser<<szBuf;

			// show symbol rate
//			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%d</center></td>", symbolRate);
//			responser<<szBuf;

			// show interleaverMode
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", switchMode(depth).c_str());
			responser<<szBuf;

			// show FEC
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", switchFec(fec).c_str());
			responser<<szBuf;

			// show device ip
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", edgeDeviceIP.c_str());
			responser<<szBuf;

			// show device group
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center>%s</center></td>", edgeDeviceGroup.c_str());
			responser<<szBuf;

			char id[256] = {0};
			itoa(portInfo.Id, id, 10);

			url.clear();
			url.setPath(EditRouteNamesPage);
			url.setVar(TemplateKey, neighborLayout(_varMap[TemplateKey].c_str(), "LOCAL_ERM_ROUTENAMES").c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(DeviceNameKey, edgeDeviceName.c_str());
			url.setVar(EdgePortKey, id);
			// show a link to restrict service group
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center><a href=\"%s\">%d</a></td>", String::getRightStr(url.generate(), "/", false).c_str(), routes.size());
			responser<<szBuf;

			url.clear();
			url.setPath(ShowChannelPage);
			url.setVar(TemplateKey, neighborLayout(_varMap[TemplateKey].c_str(), "LOCAL_ERM_CHANNEL").c_str());
			url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
			url.setVar(DeviceNameKey, edgeDeviceName.c_str());
			url.setVar(EdgePortKey, id);
			if(portInfo.Id >= 0)
			{
				itoa(portInfo.powerLevel, id, 10);
				url.setVar(Power, id);
				url.setVar(Modulation, switchModulation(intFormat).c_str());
				url.setVar(Mode, switchMode(mode).c_str());
				url.setVar(Fec, switchFec(fec).c_str());
				itoa(symbolRate, id, 10);
				url.setVar(SymRate, id);
				url.setVar(EDeviceName, edgeDeviceName.c_str());
				url.setVar(EDeviceIP, edgeDeviceIP.c_str());
				url.setVar(EDeviceGroup, edgeDeviceGroup.c_str());
			}

			// show a link to list channels
			snprintf(szBuf, sizeof(szBuf) - 1, "<td><center><a href=\"%s\">%d</a></td>", String::getRightStr(url.generate(), "/", false).c_str(), channelInfos.size());
			responser<<szBuf;

			responser<<"</tr>";
		}
		return true;
	}

} // namespace ErmWebPage

