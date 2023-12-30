// EdgeRMClient.cpp : Defines the entry point for the console application.
//
#include <fstream>
#include <sstream>
#include <cstdio>
#include "getopt.h"
#include "strHelper.h"
#include "TimeUtil.h"

#include "ZQResource.h"
#include "EdgeRMClient.h"
#include "DataTypes.h"
#include "RouteNameXmlParse.h"

#define TRY_BEGIN \
	try {
#define TRY_END \
	} \
	catch(const TianShanIce::InvalidParameter& e) { \
	std::cerr << e.message << std::endl; \
	} \
	catch(const TianShanIce::InvalidStateOfArt& e) { \
	std::cerr << e.message << std::endl; \
	} \
	catch(const TianShanIce::NotSupported& e) { \
	std::cerr << e.message << std::endl; \
	} \
	catch(const Ice::Exception& e) { \
	std::cerr << e.ice_name() << std::endl; \
	} \
	catch(const char* str) { \
	std::cerr << str << std::endl; \
	} \
	catch(...) { \
	std::cerr << "unknown error" << std::endl; \
	}

namespace {

	const char* BITRATE = "bitrate";
	const char* SRCTYPE = "sourceType";
	const char* STARTTIME = "startTime";
	const char* ENDTIME = "endTime";
	const char* TIMEOUT = "timeout";
	const char* LISTMD = "lookForMetaData";

	unsigned defaultTimeout = 5000;
}

EdgeRMClient::EdgeRMClient()
{
	ic_ = Ice::initialize();
}

EdgeRMClient::~EdgeRMClient() {

}

void EdgeRMClient::usage(const std::string& key) const {
	// ruler         "-------------------------------------------------------------------------------"
	std::cout << "Console client for EdgeResourceManager version: " 
		<< ZQ_PRODUCT_VER_MAJOR << "." 
		<< ZQ_PRODUCT_VER_MINOR << "." 
		<< ZQ_PRODUCT_VER_PATCH << "(build " 
		<< ZQ_PRODUCT_VER_BUILD << ")\n\n"

		<< "connect <endpoint>           ICE endpoint, eg: \"tcp -h 10.0.0.1 -p 11400\"\n"
		<< "                             refer to variable <timeout>, require a \n"
		<< "                             reconnection to take effect\n" 
		<< "close                        disconnect with EdgeResourceManager\n"
		<< "exit|quit                    exit shell\n"
		<< "clear                        clear screen\n"
		<< "help                         display this screen\n"
		<< "current                      display the current console context\n"
		<< "set [<var>=<value>]          set a variable in the context, show if no args\n"
		<< "open <deviceName>            open a device\n"
		<< "open channel <channelName>   open a channel\n"
		<< "open allocation <name>       open an allocation\n"
		<< "open port <name>             open a port\n"
		<< "list                         list devices, refer to the\n"
		<< "                             variable <lookForMetaData>\n"
		<< "list port                    list ports on current device, refer to the\n"
		<< "                             variable <lookForMetaData>\n"
		<< "list channel <port>          list channels on specified port, refer to the\n"
		<< "                             variable <lookForMetaData>\n"
		<< "list allocation              list allocations on current channel, refer to the\n"
		<< "                             variable <lookForMetaData>\n"
		<< "add  <deviceName>            add a device\n"
		<< "import <deviceXMLFilePath>	 import  devices from specified xml file\n"
		<< "export <name> <xmlDevicePath> export device name ,if name=null than export all devices\n"
		<< "                                                           if xmlDevicePath=null than export file in c:/tianshan/ex_devices.xml\n"
		<< "add port <name>              add a port on current device\n"
		<< "populate <name>              populate channels on specified port\n"
		<< "create allocation <name>     create an allocation\n"
//		<< "update                       update current device\n"
		<< "addchannel  <portId> <channelId> <frequency(KHZ)> <symbolrate(bps)> <startUDPport> <PortStep> <startPn> <maxSession> <lowBandwidth(Kbps)> <highBandwidth(Kbps)>  add a channel to device RFPort\n"
										"eg:addchannel 1 10 106000 6875000 4567 1 2 19 100 38000\n" 
		<< "update channel               update current channel(set parameter: startPort,step,startPN,nitPid,maxSession,low(Kbps),high(Kbps),intervalPAT,intervalPMT,tsId)\n"
//		<< "update port <name>           update specified port\n"
		<< "remove <deviceName>          remove specified device\n"
		<< "remove allocation <name>     remove specified allocation\n"
		<< "remove port <name>           remove specified port\n"
		<< "info                         show info of current device\n"
		<< "enable channel [true]        enable current channel, \n"
		<< "                             true to enable and false to disable\n"
		<< "link <port> <group> <freq>   link sevice group by port, eg:link pt1 \"group1\" 3750000\n"
		<< "link <port> <group> <freqs> link sevice group by port, eg:link pt1 \"group1\" \"3750000,4750000\"\n"
		<< "link <group> <freq>                link service group by all ports\n"
		<< "importRoutes <routesFilePath>    import routes\n"
		<< "exportRoutes <deviceName> <outRoutesFilePath>    export routes info for device, if devicename="", export routes info for all device\n"
		<< "unlink <port> <group>        unlink sevice group by port\n"
		<< std::endl;
}

void EdgeRMClient::prompt() const
{
	std::cout << (ctx_.deviceName_.empty() ? "EdgeResourceManager" : ctx_.deviceName_);
	if(!ctx_.channelName_.empty())
		std::cout << ":" << ctx_.channelName_;
	std::cout << "> ";
}

void EdgeRMClient::connect(const std::string& endpoint) {
	init();

	std::ostringstream oss;
	oss << "EdgeRM" << ":" << endpoint;

	std::string::size_type pos = std::string::npos;
	if((pos = endpoint.find_last_of(" -t ")) == std::string::npos) {
		oss << " -t " << defaultTimeout;
	}
	else {
		std::istringstream iss(endpoint.substr(pos, endpoint.find_first_of(' ', pos)));
		iss >> defaultTimeout;
	}

	try {
		ctx_.erm_ = EdgeResouceManagerPrx::checkedCast(ic_->stringToProxy(oss.str()));

		if (!ctx_.erm_) {
			std::cerr << "failed connecting with (" << oss.str() << ")" << std::endl;
			return;
		}
	}
	catch(const Ice::Exception& e) {
		std::cerr << e.ice_name() << std::endl;
		return;
	}

	std::cout << "connected with (" << oss.str() << ")" << std::endl;
}

void EdgeRMClient::close() {
	if(checkConnection()) {
		init();
	}
}

void EdgeRMClient::exit() {
	try{
		ic_->destroy();
	} 
	catch(const Ice::Exception& e) {
		std::cerr << e.ice_name() << std::endl;
	}
	catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
	catch(...) {
		std::cerr << "unknown error" << std::endl;  /* not necessary but a place holder */
	}

	quit_ = true;
}

bool EdgeRMClient::quit() const {
	return quit_;
}

void EdgeRMClient::init() {
	ctx_.device_ = 0;
	ctx_.channel_ = 0;
	ctx_.allocation_ = 0;
	ctx_.deviceName_.clear();
	ctx_.channelName_.clear();

	prop_.clear();
}

bool EdgeRMClient::checkConnection() const {
	if(!ctx_.erm_) {
		std::cerr << "not connected with any EdgeResource server" << std::endl;
		return false;
	}
	return true;
}

bool EdgeRMClient::isInteractive() const {
	return interactive_;
}

void EdgeRMClient::setInteractive(bool val) {
	interactive_ = val;
}

void EdgeRMClient::setProperty(const std::string& key, const std::string& val) {
	if(key == LISTMD) {
		listMetadata_ = ZQ::common::stringHelper::split(val, ',');
		/* not show as a context variable */
		return;
	}
	if(key == TIMEOUT) {
		std::istringstream iss(val);
		iss >> defaultTimeout;
	}
	prop_[key] = val;
}

void EdgeRMClient::info() const {
	if(!checkConnection()) {
		return;
	} 

	/* get current device info */
	if(ctx_.device_) {
		TRY_BEGIN

		TianShanIce::StrValues expectedMetaData;
		expectedMetaData.push_back(Zone);
		expectedMetaData.push_back(Vendor);
		expectedMetaData.push_back(Model);
		expectedMetaData.push_back(Description);
		expectedMetaData.push_back(TFTP);
		expectedMetaData.push_back(AdminUrl);
		EdgeDeviceInfo info = ctx_.device_->getInfo(expectedMetaData);

		std::cout << "========== " << info.ident.name << " ==========" << "\n"
			<< "Zone: " << info.props[Zone] << "\n"
			<< "Vendor: " << info.props[Vendor] << "\n"
			<< "Model: " << info.props[Model] << "\n"
			<< "Description: " << info.props[Description] << "\n"
			<< "TFTP: " << info.props[TFTP] << "\n"
			<< "Telnet: " << info.props[AdminUrl] << "\n"
			<< "Identity: " << info.ident.category << info.ident.name << std::endl;
		TRY_END
	}
	else
	{
		std::cerr << "no open device" << std::endl;
		return;
	}
}

void EdgeRMClient::current() const {
	std::ostringstream oss;

	if(ctx_.device_) {
		oss <<  "Device Name: " << ctx_.deviceName_ << "\n";
	}
	if(ctx_.channel_) {
		oss << "Channel Name: " << ctx_.channelName_ << "\n";
	}

	TianShanIce::Properties::const_iterator iter = prop_.begin();
	for(; iter != prop_.end(); ++iter) {
		oss << "var[" << iter->first << "]: " << iter->second << "\n";
	}

	if(!oss.str().empty()) {
		std::cout << oss.str();
	}
}

void EdgeRMClient::openDevice(const std::string& name)
{
	TRY_BEGIN
	if(ctx_.erm_)
	{
		ctx_.device_  = ctx_.erm_->openDevice(name);
		if(ctx_.device_)
		{
			ctx_.deviceName_ = name;
			ctx_.channelName_ = "";
			ctx_.channel_ = NULL;
		}
	}
	else
	{
		std::cerr << "no connect to EdgeResourceManager" << std::endl;
		return;
	}
	TianShanIce::StrValues expectedMetaData;
	expectedMetaData.push_back(Zone);
	expectedMetaData.push_back(Vendor);
	expectedMetaData.push_back(Model);
	expectedMetaData.push_back(Description);
	expectedMetaData.push_back(TFTP);
	expectedMetaData.push_back(AdminUrl);
	EdgeDeviceInfo info = ctx_.device_->getInfo(expectedMetaData);

	std::cout << "========== " << info.ident.name << " ==========" << "\n"
		<< "Zone: " << info.props[Zone] << "\n"
		<< "Vendor: " << info.props[Vendor] << "\n"
		<< "Model: " << info.props[Model] << "\n"
		<< "Description: " << info.props[Description] << "\n"
		<< "TFTP: " << info.props[TFTP] << "\n"
		<< "Telnet: " << info.props[AdminUrl] << "\n"
		<< "Identity: " << info.ident.category << info.ident.name << std::endl;

	TRY_END

}

void EdgeRMClient::addDevice(const std::string& name)
{
	TRY_BEGIN
	if(!ctx_.erm_)
	{
		std::cerr << "no connect to EdgeResourceManager" << std::endl;
		return;
	}

	std::string zone = (prop_.find(Zone) == prop_.end() ? "" : prop_[Zone]);
	std::string vendor = (prop_.find(Vendor) == prop_.end() ? "" : prop_[Vendor]);
	std::string model = (prop_.find(Model) == prop_.end() ? "" : prop_[Model]);
	std::string tftp = (prop_.find(TFTP) == prop_.end() ? "" : prop_[TFTP]);
	std::string telnet = (prop_.find(AdminUrl) == prop_.end() ? "" : prop_[AdminUrl]);

	ctx_.device_ = ctx_.erm_->addDevice(name, zone, vendor, model, telnet, tftp);

	TRY_END
}

void EdgeRMClient::removeDevice(const std::string& name)
{
	TRY_BEGIN

	EdgeDevicePrx devicePrx;

	if(ctx_.erm_)
		devicePrx = ctx_.erm_->openDevice(name);
	else
	{
		std::cerr << "no connect to EdgeResourceManager" << std::endl;
		return;
	}

	devicePrx->destroy();

	TRY_END
}

void EdgeRMClient::updateDevice(const std::string& name)
{
	std::cout << "not implemented yet" << "\n";
}

void EdgeRMClient::addChannel(const int portId,const int channelId,const int frequency,const int symbolrate,
							  const int startUDPport,const int portStep,const int startPn,const int maxSession,
							  const int lowBandwidth,const int highBandwidth)
{
	TRY_BEGIN
		if(ctx_.device_)
		{

			EdgeDeviceExPrx deviceEx = EdgeDeviceExPrx::uncheckedCast(ctx_.device_);

			TianShanIce::Properties attributes;
			char buf[128];
			memset(buf,0,128);
			sprintf(buf,"%d",frequency);
			attributes.insert(std::pair<std::string,std::string >(RF,buf));

			sprintf(buf,"%d",symbolrate);
			attributes.insert(std::pair<std::string,std::string >(SymRate,buf));

			sprintf(buf,"%d",startUDPport);
			attributes.insert(std::pair<std::string,std::string >(StartUDP,buf));

			sprintf(buf,"%d",portStep);
			attributes.insert(std::pair<std::string,std::string >(UdpSBP,buf));

			sprintf(buf,"%d",startPn);
			attributes.insert(std::pair<std::string,std::string >(PN,buf));

			sprintf(buf,"%d",maxSession);
			attributes.insert(std::pair<std::string,std::string >(MaxSessions,buf));

			sprintf(buf,"%d",lowBandwidth);
			attributes.insert(std::pair<std::string,std::string >(LBandWidth,buf));

			sprintf(buf,"%d",highBandwidth);
			attributes.insert(std::pair<std::string,std::string >(HBandWidth,buf));

			deviceEx->addChannel((Ice::Short)portId, (Ice::Int)channelId, attributes);

		}
		else
		{
			std::cerr << "no open device" << std::endl;
			return;
		}
		TRY_END

}

void EdgeRMClient::importDevice(const std::string& xmlDefFile)
{
	TRY_BEGIN
	if(ctx_.erm_)
	{
		TianShanIce::Properties privateData;
		EdgeRMPrx erm = EdgeRMPrx::uncheckedCast(ctx_.erm_);
		//erm->importDevice("zhuzhixiang/172_16_31_7", "xor.hf", xmlDefFile, 0, privateData);
		erm->importDevice("", "", xmlDefFile, 0, privateData);
	}
	else
	{
		std::cerr << "no connect to EdgeResourceManager" << std::endl;
		return;
	}
	TRY_END
}

void EdgeRMClient::importDevice(const std::string&name, const std::string&deviceGroup, const std::string&xmlDefFile, int bCompress, int count)
{
	for (int i = 1; i <= count; i++)
	{
		char devName[256] = {0};
		snprintf(devName, sizeof(devName), "%s%d", name.c_str(), i);
		importDevice(xmlDefFile);
	}
}

void EdgeRMClient::importRoutes(const std::string& routeFilePath)
{
	
		if (!ctx_.erm_)
		{
			std::cerr << "no connect to EdgeResourceManager" << std::endl;
			return;
		}
		ZQ::EdgeRMClient::RouteNameXml::EdgeRMRoutesConfig routeConfig("");
		if (!routeConfig.load(routeFilePath.c_str()))
		{
			std::cout << "load config file error" <<std::endl;
			return;
		}
		for (int i=0;i<routeConfig.device.size();i++)
		{
			TRY_BEGIN
			ctx_.device_  = ctx_.erm_->openDevice(routeConfig.device[i].deviceName);
			TRY_END
			if(!ctx_.device_)
			{
				std::cerr << "importRoutes: open device error;" <<"device name:"<< routeConfig.device[i].deviceName << std::endl;
				continue;;
			}
			for (int j=0;j<routeConfig.device[i].rfport.size();j++)
			{
				TianShanIce::Variant var_freqs;
				var_freqs.type = ::TianShanIce::vtLongs;
				std::string strfreq = routeConfig.device[i].rfport[j].frequencys;
				int found = strfreq.find("~");
				if ( found != std::string::npos)
				{
					var_freqs.bRange = true;
					var_freqs.lints.push_back(atol((strfreq.substr(0,found)).c_str()));
					var_freqs.lints.push_back(atol((strfreq.substr(found+1,strfreq.length())).c_str()));

				}
				else
				{
					var_freqs.bRange = false;
				
					if (!strfreq.empty())
					{
						int m = 0,n = 0;
						while (m = strfreq.find(";",n))
						{
							if (m<1)
							{
								var_freqs.lints.push_back(atol((strfreq.substr(n,strfreq.length())).c_str()));
								break;
							}
							var_freqs.lints.push_back(atol((strfreq.substr(n,m-n)).c_str()));
							n = m+1;
							m = 0;
						}
					}
				}
				TRY_BEGIN
				ctx_.device_->linkRoutes(routeConfig.device[i].rfport[j].portId, routeConfig.device[i].rfport[j].routeName, var_freqs);
				TRY_END
			}
		}	
	
}

void EdgeRMClient::exportRoutes(const std::string& deviceName,const std::string& outRoutesFilePath)
{

		if (!ctx_.erm_)
		{
			std::cerr << "no connect to EdgeResourceManager" << std::endl;
			return;
		}
		std::ofstream routesFile(outRoutesFilePath.c_str());
		if (!routesFile.is_open())
		{
			std::cout << "Routes file open failed" << std::endl;
			return;
		}
		routesFile << "<Routes>\n";
		EdgeDeviceInfos infos;
		TianShanIce::EdgeResource::EdgePortInfos  portInfos;
		TianShanIce::EdgeResource::RoutesMap routesmap;

		if (!deviceName.empty())
		{
			EdgeDeviceInfo deviceinfo;
			deviceinfo.ident.name = deviceName;

			infos.push_back(deviceinfo);

/*			ctx_.device_  = ctx_.erm_->openDevice(deviceName);
			if(!ctx_.device_)
			{
				std::cerr << "open device error" << std::endl;
				routesFile.close();
				return;
			}
			portInfos = ctx_.device_->listEdgePorts();
			routesFile << "    <Device name=\""<<deviceName<<"\"" << ">\n";
			for (PortInfos_iter itport = portInfos.begin(); itport != portInfos.end(); itport++)
			{
				TianShanIce::EdgeResource::EdgePort portInfo = *itport;
				routesmap = ctx_.device_->getRoutesRestriction(portInfo.Id);
				TianShanIce::EdgeResource::RoutesMap::const_iterator routeIt;
				for(routeIt = routesmap.begin();routeIt != routesmap.end();routeIt++)
				{
					routesFile << "        <RFPort id=\""<<portInfo.Id<<"\"";
					routesFile << " routeName=\""<<routeIt->first<<"\"";

					if (routeIt->second.lints.empty())
					{
						routesFile << " frequencys=\""<<"\"";
					}
					else
					{
						routesFile << " frequencys=\"";
						if (routeIt->second.bRange)
						{
							routesFile << routeIt->second.lints[0];
							routesFile << "~";
							routesFile << routeIt->second.lints[1];
						}
						else
						{
							int i = 0;
							while(i<routeIt->second.lints.size())
							{
								routesFile << routeIt->second.lints[i];
								i++;
								if (i<routeIt->second.lints.size())
									routesFile <<";";
							}
						}
						routesFile <<"\"";
					}
					routesFile <<"/>"<< std::endl;
				}
			}
			routesFile <<"    </Device>"<< std::endl;*/
		}
		else
		{
			TianShanIce::StrValues expectedMetaData;
			expectedMetaData.push_back(Zone);
			expectedMetaData.push_back(Vendor);
			expectedMetaData.push_back(Model);
			expectedMetaData.push_back(Description);
			expectedMetaData.push_back(TFTP);
			expectedMetaData.push_back(AdminUrl);
			TRY_BEGIN
				infos = ctx_.erm_->listDevices(expectedMetaData);
			TRY_END
		}

		for (DeviceInfos_iter itdevice = infos.begin(); itdevice != infos.end(); itdevice++)
		{
			ctx_.device_ = NULL;
			TRY_BEGIN
			ctx_.device_  = ctx_.erm_->openDevice(itdevice->ident.name);
			TRY_END
			if(NULL == ctx_.device_)
			{
				std::cerr << "exportRoutes: open device error[" << itdevice->ident.name<< "]" <<  std::endl;
				//routesFile.close();
				//return;
				continue;
			}
			portInfos = ctx_.device_->listEdgePorts();
			if (portInfos.empty())
				continue;

			routesFile << "    <Device name=\""<<itdevice->ident.name<<"\"" << ">\n";

			for (PortInfos_iter itport = portInfos.begin(); itport != portInfos.end(); itport++)
			{
				TianShanIce::EdgeResource::EdgePort portInfo = *itport;
				TianShanIce::EdgeResource::RoutesMap routesmap;
				TRY_BEGIN
				routesmap = ctx_.device_->getRoutesRestriction(portInfo.Id);
				TRY_END
				TianShanIce::EdgeResource::RoutesMap::const_iterator routeIt;
				for(routeIt = routesmap.begin();routeIt != routesmap.end();routeIt++)
				{
					routesFile << "        <RFPort id=\""<<portInfo.Id<<"\"";
					routesFile << " routeName=\""<<routeIt->first<<"\"";

					if (routeIt->second.lints.empty())
					{
						routesFile << " frequencys=\""<<"\"";
					}
					else
					{
						routesFile << " frequencys=\"";
						if (routeIt->second.bRange)
						{
							routesFile << routeIt->second.lints[0];
							routesFile << "~";
							routesFile << routeIt->second.lints[1];
						}
						else
						{
							int i = 0;
							while(i<routeIt->second.lints.size())
							{
								routesFile << routeIt->second.lints[i];
								i++;
								if (i<routeIt->second.lints.size())
									routesFile <<";";
							}
						}
						routesFile <<"\"";
					}
					routesFile <<"/>"<< std::endl;
				}
			}
			routesFile <<"    </Device>"<< std::endl;
		}

		routesFile << "</Routes>\n";
		routesFile.close();
}

void EdgeRMClient::exportDevice(const std::string& deviceName, const std::string& xmlFile)
{
	TRY_BEGIN
		if(ctx_.erm_)
		{
			TianShanIce::Properties privateData;
			EdgeRMPrx erm = EdgeRMPrx::uncheckedCast(ctx_.erm_);
			erm->exportDevices(deviceName, xmlFile);
		}
		else
		{
			std::cerr << "no connect to EdgeResourceManager" << std::endl;
			return;
		}
	TRY_END
}

void EdgeRMClient::linkRouteName(const int EdgePortId, const std::string& servicegroup, const int freq)
{
	TRY_BEGIN
		if(ctx_.device_)
		{
			TianShanIce::Variant var_freqs;
			var_freqs.type = ::TianShanIce::vtLongs;
			var_freqs.bRange = false;
			var_freqs.lints.push_back(freq);
			ctx_.device_->linkRoutes(EdgePortId, servicegroup, var_freqs);
		}
		else
		{
			std::cerr << "no open device" << std::endl;
			return;
		}
	TRY_END
}

void EdgeRMClient::linkRouteName(const int EdgePortId, const std::string& servicegroup, const std::string& freqs)
{
	TRY_BEGIN
		if(ctx_.device_)
		{
			bool bRange = false;
			// check digit only
			for(int i = 0; i < freqs.size(); i++)
			{
				if(!isdigit(freqs[i]) && freqs[i] != ',' && freqs[i] != '~')
				{
					std::cerr << "illegal input" << std::endl;
					return;
				}
				else if(freqs[i] == '~')
				{
					bRange = true;
				}
			}
			TianShanIce::Variant var_freqs;
			var_freqs.type = ::TianShanIce::vtLongs;
			TianShanIce::StrValues strVec;
			if(bRange)
			{
				var_freqs.bRange = true;
				strVec = ZQ::common::stringHelper::split(freqs, '~');
				if(strVec.size() < 2)
				{
					std::cerr << "illegal input" << std::endl;
					return;
				}
				else
				{
					if(atol(strVec[0].c_str()) >= atol(strVec[1].c_str()))
					{
						std::cerr << "illegal input" << std::endl;
						return;
					}
					var_freqs.lints.push_back(atol(strVec[0].c_str()));
					var_freqs.lints.push_back(atol(strVec[1].c_str()));
				}
			}
			else
			{
				var_freqs.bRange = false;
				strVec = ZQ::common::stringHelper::split(freqs, ',');
				for(TianShanIce::StrValues::iterator it = strVec.begin(); it != strVec.end(); it++)
				{
					var_freqs.lints.push_back(atol(it->c_str()));
				}
			}
			ctx_.device_->linkRoutes(EdgePortId, servicegroup, var_freqs);
		}
		else
		{
			std::cerr << "no open device" << std::endl;
			return;
		}
	TRY_END
}

void EdgeRMClient::linkRouteName(const std::string& servicegroup, const std::string& freqs)
{
	TRY_BEGIN
		bool bRange = false;
		// check digit only
		for(int i = 0; i < freqs.size(); i++)
		{
			if(!isdigit(freqs[i]) && freqs[i] != ',' && freqs[i] != '~')
			{
				std::cerr << "illegal input" << std::endl;
				return;
			}
			else if(freqs[i] == '~')
			{
				bRange = true;
			}
		}
		TianShanIce::Variant var_freqs;
		var_freqs.type = ::TianShanIce::vtLongs;
		TianShanIce::StrValues strVec;
		if(bRange)
		{
			var_freqs.bRange = true;
			strVec = ZQ::common::stringHelper::split(freqs, '~');
			if(strVec.size() < 2)
			{
				std::cerr << "illegal input" << std::endl;
				return;
			}
			else
			{
				if(atol(strVec[0].c_str()) >= atol(strVec[1].c_str()))
				{
					std::cerr << "illegal input" << std::endl;
					return;
				}
				var_freqs.lints.push_back(atol(strVec[0].c_str()));
				var_freqs.lints.push_back(atol(strVec[1].c_str()));
			}
		}
		else
		{
			var_freqs.bRange = false;
			strVec = ZQ::common::stringHelper::split(freqs, ',');
			for(TianShanIce::StrValues::iterator it = strVec.begin(); it != strVec.end(); it++)
			{
				var_freqs.lints.push_back(atol(it->c_str()));
			}
		}

		EdgeDeviceInfos infos;
		TianShanIce::StrValues expectedMetaData;
		if(ctx_.erm_)
			infos = ctx_.erm_->listDevices(expectedMetaData);
		else
		{
			std::cerr << "no connect to EdgeResourceManager" << std::endl;
			return;
		}

		for (DeviceInfos_iter it = infos.begin(); it != infos.end(); it++)
		{
			EdgeDeviceInfo info = *it;
			TianShanIce::ValueMap::const_iterator vMap_itor; // value map iterator
			TianShanIce::EdgeResource::EdgePortInfos  portInfos;
			TianShanIce::EdgeResource::EdgeDevicePrx devicePrx;
			devicePrx = ctx_.erm_->openDevice(info.ident.name);
			portInfos = devicePrx->listEdgePorts();
			for (PortInfos_iter it = portInfos.begin(); it != portInfos.end(); it++)
			{
				devicePrx->linkRoutes(it->Id, servicegroup, var_freqs);
			}
		}
	TRY_END
}

void EdgeRMClient::unlinkRouteName(const std::string& servicegroup)
{
	TRY_BEGIN
		EdgeDeviceInfos infos;
	TianShanIce::StrValues expectedMetaData;
	if(ctx_.erm_)
		infos = ctx_.erm_->listDevices(expectedMetaData);
	else
	{
		std::cerr << "no connect to EdgeResourceManager" << std::endl;
		return;
	}

	for (DeviceInfos_iter it = infos.begin(); it != infos.end(); it++)
	{
		EdgeDeviceInfo info = *it;
		TianShanIce::ValueMap::const_iterator vMap_itor; // value map iterator
		TianShanIce::EdgeResource::EdgePortInfos  portInfos;
		TianShanIce::EdgeResource::EdgeDevicePrx devicePrx;
		devicePrx = ctx_.erm_->openDevice(info.ident.name);
		portInfos = devicePrx->listEdgePorts();
		for (PortInfos_iter it = portInfos.begin(); it != portInfos.end(); it++)
		{
			devicePrx->unlinkRoutes(it->Id, servicegroup);
		}
	}
	TRY_END
}

void EdgeRMClient::unlinkRouteName(const int EdgePortId, const std::string& servicegroup)
{
	TRY_BEGIN
		if(ctx_.device_)
		{
			ctx_.device_->unlinkRoutes(EdgePortId, servicegroup);
		}
		else
		{
			std::cerr << "no open device" << std::endl;
			return;
		}
	TRY_END
}

void EdgeRMClient::listDevice() const
{
	TRY_BEGIN
	TianShanIce::StrValues expectedMetaData;
	expectedMetaData.push_back(Zone);
	expectedMetaData.push_back(Vendor);
	expectedMetaData.push_back(Model);
	expectedMetaData.push_back(Description);
	expectedMetaData.push_back(TFTP);
	expectedMetaData.push_back(AdminUrl);
	EdgeDeviceInfos infos;
	if(ctx_.erm_)
		infos = ctx_.erm_->listDevices(expectedMetaData);
	else
	{
		std::cerr << "no connect to EdgeResourceManager" << std::endl;
		return;
	}

	for (DeviceInfos_iter it = infos.begin(); it != infos.end(); it++)
	{
		EdgeDeviceInfo info = *it;

		std::cout << "========== " << info.ident.name << " ==========" << "\n"
			<< "Zone: " << info.props[Zone] << "\n"
			<< "Vendor: " << info.props[Vendor] << "\n"
			<< "Model: " << info.props[Model] << "\n"
			<< "Description: " << info.props[Description] << "\n"
			<< "TFTP: " << info.props[TFTP] << "\n"
			<< "Telnet: " << info.props[AdminUrl] << "\n"
			<< "Identity: " << info.ident.category << info.ident.name << std::endl;
	}

	TRY_END
}

void EdgeRMClient::openChannel(const std::string& name)
{
	if(ctx_.erm_) 
	{
		TRY_BEGIN
		EdgeRMPrx edgeRM = EdgeRMPrx::uncheckedCast(ctx_.erm_);
		ctx_.channel_ = (EdgeChannelPrx)edgeRM->openChannel(name);
		if(ctx_.channel_)
		{
			ctx_.channelName_ = name;
		}
		TianShanIce::StrValues expectedMetaData;
		expectedMetaData.push_back(RF);
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

		TianShanIce::State state =   ctx_.channel_->getState();
		EdgeChannelInfo info = ctx_.channel_->getInfo(expectedMetaData);
		std::cout << "========== " << info.ident.name << " ==========" << "\n"
			<< "Admin State: " << switchState(state) << "\n"
			<< "RF Freq(Mhz):   " << info.props[RF] << "\n"
			<< "TSID:           " << info.props[TSID] << "\n"
			<< "PAT Interval:   " << info.props[PAT_Interval] << "\n"
			<< "PMT Interval:   " << info.props[PMT_Interval] << "\n"
			<< "NITPID:         " << info.props[NITPID] << "\n"
			<< "Start UDP Port: " << info.props[StartUDP] << "\n"
			<< "UDPPortStepByPn:" << info.props[UdpSBP] << "\n"
			<< "Start PN:       " << info.props[PN] << "\n"
			<< "Max Sessions:   " << info.props[MaxSessions] << "\n"
			<< "Low BandWidth:  " << info.props[LBandWidth] << "\n"
			<< "High BandWidth: " << info.props[HBandWidth] << "\n"
			<< "Enabled:        " << info.props[Enabled] << "\n"
			<< "LastUpdated:    " << TianShanTime2String(_atoi64(info.props[LastUpdated].c_str())) << "\n"
			<< "Identity:       " << info.ident.category << info.ident.name << std::endl;
// 		std::string deviceName;
// 		Ice::Short EdgePort;
// 		Ice::Short chNum;
// 		ctx_.channel_->getHiberarchy(deviceName, EdgePort, chNum);
//		openPort(EdgePort);
		TRY_END
	}
}

void EdgeRMClient::openChannel(const int EdgePortId, const int chNum)
{
	if(ctx_.device_)
	{
		TRY_BEGIN
		ctx_.channel_ = ctx_.device_->openChannel(EdgePortId, chNum);
		if(ctx_.channel_)
		{
			ctx_.channelName_ = ctx_.channel_->getId();
		}
		TianShanIce::StrValues expectedMetaData;
		expectedMetaData.push_back(RF);
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

		TianShanIce::State state =   ctx_.channel_->getState();
		EdgeChannelInfo info = ctx_.channel_->getInfo(expectedMetaData);
		std::cout << "========== " << info.ident.name << " ==========" << "\n"
			<< "Admin State:    " << switchState(state) << "\n"
			<< "RF Freq(Mhz):   " << info.props[RF] << "\n"
			<< "TSID:           " << info.props[TSID] << "\n"
			<< "PAT Interval:   " << info.props[PAT_Interval] << "\n"
			<< "PMT Interval:   " << info.props[PMT_Interval] << "\n"
			<< "NITPID:         " << info.props[NITPID] << "\n"
			<< "Start UDP Port: " << info.props[StartUDP] << "\n"
			<< "UDPPortStepByPn:" << info.props[UdpSBP] << "\n"
			<< "Start PN:       " << info.props[PN] << "\n"
			<< "Max Sessions:   " << info.props[MaxSessions] << "\n"
			<< "Low BandWidth:  " << info.props[LBandWidth] << "\n"
			<< "High BandWidth: " << info.props[HBandWidth] << "\n"
			<< "Enabled:        " << info.props[Enabled] << "\n"
			<< "LastUpdated:    " << TianShanTime2String(_atoi64(info.props[LastUpdated].c_str())) << "\n"
			<< "Identity:       " << info.ident.category << info.ident.name << std::endl;
//		openPort(EdgePortId);
		TRY_END
	}
	else
	{
		std::cerr << "no open device" << std::endl;
		return;
	}
}

void EdgeRMClient::enableChannel(int flag)
{
	bool bflag;
	if (flag == 1)
		bflag = true;
	else
		bflag = false;
	if(ctx_.channel_)
	{
		TRY_BEGIN
		ctx_.channel_->enable(bflag);
		TRY_END
	}
	else
	{
		std::cerr << "no opened channle" << std::endl;
		return;
	}
}

void EdgeRMClient::updateChannel()
{
	if(ctx_.channel_)
	{
		TRY_BEGIN
		EdgeChannelExPrx channleEx = EdgeChannelExPrx::uncheckedCast(ctx_.channel_);

		std::string  device;
		short  portID; 
		short  chNum; 
		channleEx->getHiberarchy(device, portID, chNum);

		EdgePort portInfo;
		TianShanIce::Properties attrs;
		TianShanIce::Properties::const_iterator iter;

		iter = prop_.find("startPort");
		if (iter != prop_.end())
		{
			attrs[StartUDP] = iter->second;
		}

		iter = prop_.find("step");
		if (iter != prop_.end())
		{
			attrs[UdpSBP] = iter->second;
		}

		iter = prop_.find("startPN");
		if (iter != prop_.end())
		{
			attrs[PN] = iter->second;
		}

		iter = prop_.find("nitPid");
		if (iter != prop_.end())
		{
			attrs[NITPID] = iter->second;
		}

		iter = prop_.find("maxSession");
		if (iter != prop_.end())
		{
			attrs[MaxSessions] = iter->second;
		}

		iter = prop_.find("low");
		if (iter != prop_.end())
		{
			attrs[LBandWidth] = iter->second;
		}

		iter = prop_.find("high");
		if (iter != prop_.end())
		{
			attrs[HBandWidth] = iter->second;
		}

		iter = prop_.find("tsId");
		if (iter != prop_.end())
		{
			attrs[TSID] = iter->second;
		}

		iter = prop_.find("intervalPAT");
		if (iter != prop_.end())
		{
			attrs[PAT_Interval] = iter->second;
		}

		iter = prop_.find("intervalPMT");
		if (iter != prop_.end())
		{
			attrs[PMT_Interval] = iter->second;
		}

		channleEx->updateAttributes(attrs);
		TRY_END
	}
	else
	{
		std::cerr << "no open channel" << std::endl;
		return;
	}
}

void EdgeRMClient::listChannel(const int EdgePortId, bool enabledOnly) const
{
	if(ctx_.device_)
	{
		TRY_BEGIN
		TianShanIce::StrValues expectedMetaData;
		expectedMetaData.push_back(RF);
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

		EdgeChannelInfos infos = ctx_.device_->listChannels(EdgePortId, expectedMetaData, enabledOnly);
		for(ChannelInfos_iter iter = infos.begin(); iter != infos.end(); iter++)
		{
			EdgeChannelInfo info = *iter; 
			std::cout << "========== " << info.ident.name << " ==========" << "\n"
				<< "RF Freq(Mhz):   " << info.props[RF] << "\n"
				<< "TSID:           " << info.props[TSID] << "\n"
				<< "PAT Interval:   " << info.props[PAT_Interval] << "\n"
				<< "PMT Interval:   " << info.props[PMT_Interval] << "\n"
				<< "NITPID:         " << info.props[NITPID] << "\n"
				<< "Start UDP Port: " << info.props[StartUDP] << "\n"
				<< "UDPPortStepByPn:" << info.props[UdpSBP] << "\n"
				<< "Start PN:       " << info.props[PN] << "\n"
				<< "Max Sessions:   " << info.props[MaxSessions] << "\n"
				<< "Low BandWidth:  " << info.props[LBandWidth] << "\n"
				<< "High BandWidth: " << info.props[HBandWidth] << "\n"
				<< "Enabled:        " << info.props[Enabled] << "\n"
				<< "LastUpdated:    " << TianShanTime2String(_atoi64(info.props[LastUpdated].c_str())) << "\n"
				<< "Identity:       " << info.ident.category << info.ident.name << std::endl;
		}
		TRY_END
	}
	else
	{
		std::cerr << "no open device" << std::endl;
		return;
	}
}

void EdgeRMClient::populateChannel(const int EdgePortId)
{
	if(ctx_.device_)
	{
		TRY_BEGIN
		EdgeChannelInfos infos = ctx_.device_->populateChannels(EdgePortId);
		for(ChannelInfos_iter iter = infos.begin(); iter != infos.end(); iter++)
		{
			EdgeChannelInfo info = *iter; 
			std::cout << "========== " << info.ident.name << " ==========" << "\n"
				<< "RF Freq(Mhz): " << info.props[Freq] << "\n"
				<< "TSID: " << info.props[TSID] << "\n"
				<< "PAT Interval: " << info.props[PAT_Interval] << "\n"
				<< "PMT Interval: " << info.props[PMT_Interval] << "\n"
				<< "Identity: " << info.ident.category << info.ident.name << std::endl;
		}
		TRY_END
	}
	else
	{
		std::cerr << "no open device" << std::endl;
		return;
	}
}

void EdgeRMClient::openPort(const int& id)
{
	TRY_BEGIN
	if(ctx_.device_)
	{
		ctx_.channelName_ = "";
		ctx_.channel_ = NULL;

		TianShanIce::ValueMap::const_iterator vMap_itor; // value map iterator
		TianShanIce::EdgeResource::EdgePortInfos  portInfos;
		portInfos = ctx_.device_->listEdgePorts();
		for (PortInfos_iter it = portInfos.begin(); it != portInfos.end(); it++)
		{
			TianShanIce::EdgeResource::EdgePort portInfo = *it;
			if(portInfo.Id == id)
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

				std::cout << "========== " << portInfo.Id << " ==========" << "\n"					
					<< "Power Level(dBmv): " << portInfo.powerLevel << "\n"
					<< "Modulation Format: " << switchModulation(intFormat) << "\n"
					<< "Interleaver Depth: " << depth << "\n"
					<< "Symbol Rate:       " << symbolRate << "\n"
					<< "Interleaver Mode:  " << mode << "\n"
					<< "FEC:               " << fec << "\n"
					<< "DeviceName:        " << edgeDeviceName << "\n"
					<< "DeviceIP:          " << edgeDeviceIP << "\n"
					<< "DeviceGroup:       " << edgeDeviceGroup
					<< std::endl;

				break;
			}
		}
	}
	else
	{
		std::cerr << "no open device" << std::endl;
		return;
	}
	TRY_END
}

void EdgeRMClient::addPort(const int& id)
{
	TRY_BEGIN
		if(ctx_.device_)
		{
			EdgeDeviceExPrx deviceEx = EdgeDeviceExPrx::uncheckedCast(ctx_.device_);

			EdgePort portInfo;
			portInfo.Id = id;

			TianShanIce::Properties::const_iterator iter = prop_.find(Power);
			portInfo.powerLevel = (iter == prop_.end() ? 50 : atoi(iter->second.c_str()));

			iter = prop_.find(Depth);
			if(iter != prop_.end())
			{
				TianShanIce::Variant depth;
				depth.type = TianShanIce::vtInts;
				depth.ints[0] = atoi(iter->second.c_str());
				portInfo.resAtscModulationMode.resourceData[Depth] = depth;
			}

			iter = prop_.find(Mode);
			if(iter != prop_.end())
			{
				TianShanIce::Variant mode;
				mode.type = TianShanIce::vtInts;
				mode.ints[0] = getModeCode(iter->second);
				portInfo.resAtscModulationMode.resourceData[Mode] = mode;
			}

			iter = prop_.find(Modulation);
			if(iter != prop_.end())
			{
				TianShanIce::Variant modulation;
				modulation.type = TianShanIce::vtInts;
				modulation.ints[0] = getModulationCode(iter->second);
				portInfo.resAtscModulationMode.resourceData[Modulation] = modulation;
			}

			iter = prop_.find(SymRate);
			if(iter != prop_.end())
			{
				TianShanIce::Variant symrate;
				symrate.type = TianShanIce::vtInts;
				symrate.ints[0] = atoi(iter->second.c_str());
				portInfo.resAtscModulationMode.resourceData[SymRate] = symrate;
			}

			iter = prop_.find(Fec);
			if(iter != prop_.end())
			{
				TianShanIce::Variant fec;
				fec.type = TianShanIce::vtInts;
				fec.ints[0] = atoi(iter->second.c_str());
				portInfo.resAtscModulationMode.resourceData[Fec] = fec;
			}

			iter = prop_.find(EDeviceName);
			if(iter != prop_.end())
			{
				TianShanIce::Variant deviceName;
				deviceName.type = TianShanIce::vtStrings;
				deviceName.strs[0] = iter->second;
				portInfo.resPhysicalChannel.resourceData[EDeviceName] = deviceName;
			}

			iter = prop_.find(EDeviceIP);
			if(iter != prop_.end())
			{
				TianShanIce::Variant deviceIP;
				deviceIP.type = TianShanIce::vtStrings;
				deviceIP.strs[0] = iter->second;
				portInfo.resPhysicalChannel.resourceData[EDeviceIP] = deviceIP;
			}

			iter = prop_.find(EDeviceGroup);
			if(iter != prop_.end())
			{
				TianShanIce::Variant deviceGroup;
				deviceGroup.type = TianShanIce::vtStrings;
				deviceGroup.strs[0] = iter->second;
				portInfo.resPhysicalChannel.resourceData[EDeviceGroup] = deviceGroup;
			}

			deviceEx->addEdgePort(portInfo);

		}
		else
		{
			std::cerr << "no open device" << std::endl;
			return;
		}
	TRY_END
}

void EdgeRMClient::removePort(const int& id)
{
	TRY_BEGIN
		if(ctx_.device_)
		{
			EdgeDeviceExPrx deviceEx = EdgeDeviceExPrx::uncheckedCast(ctx_.device_);
			deviceEx->removeEdgePort(id);
		}
		else
		{
			std::cerr << "no open device" << std::endl;
			return;
		}
	TRY_END
}

void EdgeRMClient::updatePort(const int& id)
{
	std::cout << "not implemented yet" << "\n";
}

void EdgeRMClient::listPort() const
{
	TRY_BEGIN
		if(ctx_.device_)
		{
			TianShanIce::ValueMap::const_iterator vMap_itor; // value map iterator
			TianShanIce::EdgeResource::EdgePortInfos  portInfos;
			portInfos = ctx_.device_->listEdgePorts();
			for (PortInfos_iter it = portInfos.begin(); it != portInfos.end(); it++)
			{
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

				std::cout << "========== " << portInfo.Id << " ==========" << "\n"
					<< "Power Level(dBmv): " << portInfo.powerLevel << "\n"
					<< "Modulation Format: " << switchModulation(intFormat) << "\n"
					<< "Interleaver Depth: " << depth << "\n"
					<< "Symbol Rate:       " << symbolRate << "\n"
					<< "Interleaver Mode:  " << mode << "\n"
					<< "FEC:               " << fec << "\n"
					<< "DeviceName:        " << edgeDeviceName << "\n"
					<< "DeviceIP:          " << edgeDeviceIP << "\n"
					<< "DeviceGroup:       " << edgeDeviceGroup
					<< std::endl;
			}
		}
		else
		{
			std::cerr << "no open device" << std::endl;
			return;
		}
	TRY_END
}

void EdgeRMClient::openAllocation(const std::string& id)
{
	TRY_BEGIN
		if(ctx_.erm_)
		{
			ctx_.channelName_ = "";
			ctx_.channel_ = NULL;

			ctx_.allocation_ = ctx_.erm_->openAllocation(id);
			AllocationInfo allocationInfo;
			allocationInfo = ctx_.allocation_->getInfo(); 
			
			std::cout << "========== " << allocationInfo.ident.name << " ==========" << "\n"
//				<< "Video Module Address: " << allocationInfo.props[Address] << "\n"
				<< "UDP Port:         " << allocationInfo.props[UDP] << "\n"
//				<< "Output QAM: " << allocationInfo.props[Qam] << "\n"
				<< "Program Number:   " << allocationInfo.props[ProgramNumber] << "\n"
				<< "Source IP:        " << allocationInfo.props[SourceIP] << "\n"
				<< "Bandwidth (Kbps): " << allocationInfo.props[BandWidth] << "\n"
				<< "State:            " << switchState(ctx_.allocation_->getState()) << "\n"
				<< "Status:           " << allocationInfo.props[Status] << "\n"
				<< "Maximum Jitter:   " << allocationInfo.props[MaximumJitter] << "\n"
				<< "Stamp Created:    " << TianShanTime2String(_atoi64(allocationInfo.props[StpCreated].c_str())) << "\n"
				<< "Stamp Provisioned:" << TianShanTime2String(_atoi64(allocationInfo.props[StpProvisioned].c_str())) << "\n"
				<< "Stamp Committed:  " << TianShanTime2String(_atoi64(allocationInfo.props[StpCommitted].c_str())) << "\n"
				<< "Expiration:       " << TianShanTime2String(_atoi64(allocationInfo.props[Expire].c_str())) << "\n"
				<< "Identity: " << allocationInfo.ident.category << allocationInfo.ident.name 
				<< std::endl;
		}
		else
		{
			std::cerr << "no connect to EdgeResourceManager" << std::endl;
			return;
		}
	TRY_END
}

void EdgeRMClient::createAllocation(const std::string& id)
{
	std::cout << "not implemented yet" << "\n";
}

void EdgeRMClient::removeAllocation(const std::string& id)
{
	TRY_BEGIN
		if(ctx_.erm_)
		{
			AllocationPrx allocation = ctx_.erm_->openAllocation(id);
			allocation->destroy();
		}
		else
		{
			std::cerr << "no connect to EdgeResourceManager" << std::endl;
			return;
		}
	TRY_END
}

void EdgeRMClient::listAllocation() const
{
	TRY_BEGIN
		if(ctx_.erm_)
		{
			short EdgePort;
			short chNum;
			std::string deviceName;
			if(ctx_.device_)
			{
				deviceName = ctx_.device_->getName();
			}
			else
			{
				std::cerr << "no open device" << std::endl;
				return;
			}
			if(ctx_.channel_)
			{
				ctx_.channel_->getHiberarchy(deviceName, EdgePort, chNum);
			}
			else
			{
				std::cerr << "no open channel" << std::endl;
				return;
			}
			TianShanIce::StrValues expectedMetaData;
			expectedMetaData.push_back(UDP);
			expectedMetaData.push_back(ProgramNumber);
			expectedMetaData.push_back(SourceIP);
			expectedMetaData.push_back(BandWidth);
			expectedMetaData.push_back(Status);
			expectedMetaData.push_back(MaximumJitter);
			expectedMetaData.push_back(StpCreated);
			expectedMetaData.push_back(StpProvisioned);
			expectedMetaData.push_back(StpCommitted);
			expectedMetaData.push_back(Expire);
			AllocationInfos infos = ctx_.erm_->listAllocations (deviceName, EdgePort, chNum, expectedMetaData);
			for(AllocationInfos_iter iter = infos.begin(); iter != infos.end(); iter++)
			{
//				AllocationInfo& allocationInfo = *iter;
				std::cout << "========== " << (*iter).ident.name << " ==========" << "\n"
//					<< "Video Module Address: " << allocationInfo.props[Address] << "\n"
					<< "UDP Port:         " << (*iter).props[UDP] << "\n"
//					<< "Output QAM: " << allocationInfo.props[Qam] << "\n"
					<< "Program Number:   " << (*iter).props[ProgramNumber] << "\n"
					<< "Source IP:        " << (*iter).props[SourceIP] << "\n"
					<< "Bandwidth (Kbps): " << (*iter).props[BandWidth] << "\n"
//					<< "State:            " << switchState(ctx_.allocation_->getState()) << "\n"
					<< "Status:           " << (*iter).props[Status] << "\n"
					<< "Maximum Jitter:   " << (*iter).props[MaximumJitter] << "\n"
					<< "Stamp Created:    " << TianShanTime2String(_atoi64((*iter).props[StpCreated].c_str())) << "\n"
					<< "Stamp Provisioned:" << TianShanTime2String(_atoi64((*iter).props[StpProvisioned].c_str())) << "\n"
					<< "Stamp Committed:  " << TianShanTime2String(_atoi64((*iter).props[StpCommitted].c_str())) << "\n"
					<< "Expiration:       " << TianShanTime2String(_atoi64((*iter).props[Expire].c_str())) << "\n"
					<< "Identity: " << (*iter).ident.category << ":" << (*iter).ident.name 
					<< std::endl;
			}
		}
		else
		{
			std::cerr << "no connect to EdgeResourceManager" << std::endl;
			return;
		}
	TRY_END
	
}


extern FILE *yyin;
extern int yyparse();

extern bool isEOF;

extern EdgeRMClient client;

void usage() {
	std::cout << "Usage: ContentClient [option] [arg]\n\n"
		<< "Options:\n"
		<< "  -h              show this message\n"
		<< "  -e <endpoint>   ICE endpoint to be connected with\n"
		<< "  -f <file>       read instruction from file, for batch jobs\n"
		<< "  -v              output product version\n"
		<< std::endl;
}

int main(int argc, char* argv[])
{
	std::string file;

	if(argc > 1) {
		int ch = 0;
		while((ch = getopt(argc, argv, "he:f:v")) != EOF) {
			if(ch == 'h') {
				usage();
				return (0);
			}			
			else if(ch == 'e') {
				client.connect(optarg);
			}
			else if(ch == 'f') {
				file = optarg;
			}
			else if(ch == 'v') {
				std::cout << "Console client for EdgeRM version: " 
					<< ZQ_PRODUCT_VER_MAJOR << "." 
					<< ZQ_PRODUCT_VER_MINOR << "." 
					<< ZQ_PRODUCT_VER_PATCH << "(build " 
					<< ZQ_PRODUCT_VER_BUILD << ")\n"
					<< std::endl;
				return (0);
			}
			else {
				std::cerr << "invalid option" <<  std::endl;
				return (0);
			}
		}
	}

	if(!file.empty()) {
		FILE* fp = std::fopen(file.c_str(), "r");
		yyin = fp;

		client.setInteractive(false);
		while(!isEOF) {
			yyparse();
		}
		if(!client.quit()) {
			client.exit();
		}
		fclose(fp);
	}	
	else {
		client.setInteractive(true);

		while(!client.quit()) {
			client.prompt();
			yyparse();
		}
	}

	return 0;
}

