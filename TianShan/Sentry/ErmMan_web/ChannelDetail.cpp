#include "ChannelDetail.h"
#include "DataTypes.h"

namespace ErmWebPage
{
	ChannelDetail::ChannelDetail(IHttpRequestCtx* pHttpRequestCtx) : BasePage(pHttpRequestCtx)
	{
	}

	ChannelDetail::~ChannelDetail()
	{
	}

	bool ChannelDetail::get()
	{
		IHttpResponse& responser = _reqCtx->Response();

		std::string chnlName;
		chnlName = _varMap[ChannelNameKey];
		if (chnlName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "No channel name<br>");
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(InsertItem, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		std::string devName; 
		devName = _varMap[DeviceNameKey];
		if (devName.empty())
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "No device name<br>");
			setLastError(szBuf);
			responser.SetLastError(getLastError());
			return false;
		}

		short portNum = 0;
		short chNum = 0;
		string chName = chnlName;
			chName = chName.substr(chName.find_last_of('/')+1, chName.find_last_of('.')-chName.find_last_of('/'));
		portNum = atoi(chName.c_str());
		std::string strPort= chName;
		string cn = chnlName;
			cn = cn.substr(cn.find_last_of('.')+1, cn.size());
		chNum = atoi(cn.c_str());

		TianShanIce::EdgeResource::EdgePortInfos  portInfos;
		TianShanIce::EdgeResource::EdgeDevicePrx devicePrx;
		try
		{ 
			devicePrx = _ERM->openDevice(devName);
			portInfos = devicePrx->listEdgePorts();
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "open device caught %s:%s<br>", 
				ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "open device caught %s<br>", 
				ex.ice_name().c_str());
			setLastError(szBuf);
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
			if(portInfo.Id == portNum)
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
				sMode = switchMode(depth);
				sFec = switchFec(fec);
				itoa(symbolRate, temp, 10);
				sSymbolRate = temp;
				sEdgeDeviceGroup = edgeDeviceName;
				sEdgeDeviceIP = edgeDeviceIP;
				sEdgeDeviceName = edgeDeviceName;
				break;
			}
		}

		EdgeChannelInfo channelInfo;
		TianShanIce::State state;
		TianShanIce::StrValues expectedMetaData;
		expectedMetaData.push_back(RF);
		expectedMetaData.push_back(TSID);
		expectedMetaData.push_back(SYS_PROP(symbolRate));
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

		try
		{ 
			TianShanIce::EdgeResource::EdgeChannelPrx channelPrx;
			channelPrx = devicePrx->openChannel(portNum, chNum);
			state = channelPrx->getState();
			channelInfo = channelPrx->getInfo(expectedMetaData);
		}
		catch (const TianShanIce::BaseException& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "open channel caught %s:%s<br>", 
				ex.ice_name().c_str(), ex.message.c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}
		catch (const Ice::Exception& ex)
		{
			snprintf(szBuf, sizeof(szBuf) - 1, "open channel caught %s<br>", 
				ex.ice_name().c_str());
			setLastError(szBuf);
			glog(ErrorLog, CLOGFMT(ShowDevice, "%s"), getLastError());
			responser.SetLastError(getLastError());
			return false;
		}

		// write page's functionality
		snprintf(szBuf, sizeof(szBuf) - 1, "<H2>Channel Detail [%s]</H2>", chnlName.c_str());
		responser<<szBuf;

		responser<<"<fieldset>";
//		responser<<"<legend>Admin<br>State</legend>";
		responser<<"<label for=\"adminState\">Admin State:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", switchState(state).c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>RF Freq<br>(Khz)</legend>";
		responser<<"	<label for=\"RF\">RF:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", channelInfo.props[RF].c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>Power<br>Level</legend>";
		responser<<"	<label for=\"powerLevel\">Power Level:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", sPower.c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>Modulation<br>Format</legend>";
		responser<<"	<label for=\"modulation\">Modulation Format:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", sModulation.c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>InterLeaver<br>Mode</legend>";
		responser<<"	<label for=\"mode\">InterLeaver Mode:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", sMode.c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>FEC</legend>";
		responser<<"	<label for=\"FEC\">FEC:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", sFec.c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>Symbol Rate</legend>";
		responser<<"	<label for=\"SymbolRate\">Symbol Rate:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", channelInfo.props[SYS_PROP(symbolRate)].c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>Device Name</legend>";
		responser<<"	<label for=\"DeviceName\">Device Name:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", sEdgeDeviceName.c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>Device IP</legend>";
		responser<<"	<label for=\"DeviceIP\">Device IP:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", sEdgeDeviceIP.c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>Device Group</legend>";
		responser<<"	<label for=\"DeviceGroup\">Device Group:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", sEdgeDeviceGroup.c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>TSID</legend>";
		responser<<"	<label for=\"TSID\">TSID:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", channelInfo.props[TSID].c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>PAT</legend>";
		responser<<"	<label for=\"PAT\">PAT:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", channelInfo.props[PAT_Interval].c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>PMT</legend>";
		responser<<"	<label for=\"PMT\">PMT:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", channelInfo.props[PMT_Interval].c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>NITPID</legend>";
		responser<<"	<label for=\"NITPID\">NITPID:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", channelInfo.props[NITPID].c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>Start UDP Port</legend>";
		responser<<"	<label for=\"StartUDP\">Start UDP Port:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", channelInfo.props[StartUDP].c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>Program Number</legend>";
		responser<<"	<label for=\"PN\">Program Number:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", channelInfo.props[PN].c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>Max Sessions</legend>";
		responser<<"	<label for=\"MaxSessions\">Max Sessions:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", channelInfo.props[MaxSessions].c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>Low Bandwidth Utilization</legend>";
		responser<<"	<label for=\"LBandWidth\">Low Bandwidth Utilization:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%d</th>", _atoi64(channelInfo.props[LBandWidth].c_str())/1000);
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>High Bandwidth Utilization</legend>";
		responser<<"	<label for=\"HBandWidth\">High Bandwidth Utilization:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%d</th>", _atoi64(channelInfo.props[HBandWidth].c_str())/1000);
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>Enabled</legend>";
		responser<<"	<label for=\"Enabled\">Enabled:</label>";
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", channelInfo.props[Enabled].c_str());
		responser<<szBuf;
		responser<<"<BR>";
//		responser<<"</fieldset>";

//		responser<<"<fieldset>";
//		responser<<"	<legend>Last Updated</legend>";
		responser<<"	<label for=\"LastUpdated\">Last Updated:</label>";
		char timeBuffer[128];
		memset(timeBuffer,'\0',sizeof(timeBuffer));
		ZQ::common::TimeUtil::TimeToUTC(_atoi64(channelInfo.props[LastUpdated].c_str()),timeBuffer,sizeof(timeBuffer),true);
		snprintf(szBuf, sizeof(szBuf) - 1, "<th>%s</th>", timeBuffer);
		responser<<szBuf;
		responser<<"<BR>";
		responser<<"</fieldset>";

		url.clear();
		url.setPath(ChannelDetailPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(DeviceNameKey, devName.c_str());
		url.setVar(ChannelNameKey, _varMap[ChannelNameKey].c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<br><a href=\"%s\"><B>Refresh</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;

		LinkSpace;
		url.clear();
		url.setPath(ShowChannelPage);
		url.setVar(TemplateKey, _varMap[TemplateKey].c_str());
		url.setVar(PublisherKey, _varMap[PublisherKey].c_str());
		url.setVar(DeviceNameKey, devName.c_str());
		url.setVar(ChannelNameKey, _varMap[ChannelNameKey].c_str());
		url.setVar(EdgePortKey, strPort.c_str());
		snprintf(szBuf, sizeof(szBuf) - 1, "<a href=\"%s\"><B>Back to Channel List</B></a>", String::getRightStr(url.generate(), "/", false).c_str());
		responser<<szBuf;


		return true;
	}

	bool ChannelDetail::post()
	{
		return true;
	}

} // namespace ErmWebPage


