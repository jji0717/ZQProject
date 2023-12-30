#ifndef _ROUTENAMEXMLPARSE_H__
#define _ROUTENAMEXMLPARSE_H__

#include <vector>
#include <ConfigHelper.h>

namespace ZQ { 
	namespace EdgeRMClient
	{
		namespace RouteNameXml
		{
			struct RFPort
			{
				int portId;
				std::string frequencys;
				std::string routeName;
				static void structure( ZQ::common::Config::Holder<RFPort>& holder )
				{
					using namespace ZQ::common::Config;
					holder.addDetail("","id",&RFPort::portId,"0",optReadOnly);
					holder.addDetail("","routeName",&RFPort::routeName,"",optReadOnly);
					holder.addDetail("","frequencys",&RFPort::frequencys,"",optReadOnly);
				}
			};
			
			struct EdgeRMDevice
			{
				std::string deviceName;

				typedef ZQ::common::Config::Holder<RFPort> RFPortHolder;
				std::vector<RFPortHolder> rfport;
				void readRFPortHolder( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
				{
					RFPortHolder holder;
					holder.read( node , hPP );
					rfport.push_back(holder);
				}
				void registerRFPortHolder(const std::string&){}

				static void structure( ZQ::common::Config::Holder<EdgeRMDevice>& holder)
				{
					using namespace ZQ::common::Config;
					holder.addDetail("","name",&EdgeRMDevice::deviceName,"",optReadOnly);
 					holder.addDetail("RFPort",&EdgeRMDevice::readRFPortHolder,&EdgeRMDevice::registerRFPortHolder);
				}
			};

			struct EdgeRMRoutes
			{
				typedef ZQ::common::Config::Holder<EdgeRMDevice> DeviceHolder;
				std::vector<DeviceHolder> device;
				void readDeviceHolder( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
				{
					DeviceHolder holder;
					holder.read( node , hPP );
					device.push_back(holder);
				}
				void registerDeviceHolder(const std::string&){}

				static void structure( ZQ::common::Config::Holder<EdgeRMRoutes>& holder)
				{
					using namespace ZQ::common::Config;
					holder.addDetail("Device",&EdgeRMRoutes::readDeviceHolder,&EdgeRMRoutes::registerDeviceHolder);
				}
			};

			typedef ZQ::common::Config::Loader<EdgeRMRoutes> EdgeRMRoutesConfig;
		}
	}
}

#endif