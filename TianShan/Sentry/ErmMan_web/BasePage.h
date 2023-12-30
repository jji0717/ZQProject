#ifndef __EdgeResourceManager_BasePage_H__
#define __EdgeResourceManager_BasePage_H__

#define _WINSOCK2API_

#include <Ice/Ice.h>
#include "../httpdInterface.h"
#include <Log.h>
#include <urlstr.h>
#include "stroprt.h"
#include "TsEdgeResource.h"
#include "DataTypes.h"
#include "TianShanDefines.h"

#define DebugLog ZQ::common::Log::L_DEBUG
#define InfoLog ZQ::common::Log::L_INFO
#define NoticeLog ZQ::common::Log::L_NOTICE
#define WarningLog ZQ::common::Log::L_WARNING
#define ErrorLog ZQ::common::Log::L_ERROR
#define CritLog ZQ::common::Log::L_CRIT
#define ALertLog ZQ::common::Log::L_ALERT
#define EmergLog ZQ::common::Log::L_EMERG

// up# means "url parameter"
#define ChannelNameKey			"erm#chnl"
#define ChannelNumberKey		"erm#chnum"
#define DeviceNameKey			"erm#dev"
#define EdgePortKey			    "erm#pt"
//#define ServiceGroupKey			"erm#sg"
#define SessionNameKey			"erm#ssn"
#define RouteNamesKey			"erm#rn"
#define PublisherKey			"erm#pubep"
#define TemplateKey				"#template"
#define RefreshKey				"#refresh"

#define ERMServicePagePath		"service.ermweb.tswl"
#define ERMMethodPagePath		"method.ermweb.tswl"

#define ErmMainPage				"ErmMain.ermweb.tswl"
#define ShowChannelPage			"ShowChannel.ermweb.tswl"
#define AddChannelPage			"AddChannel.ermweb.tswl"
#define EditChannelPage			"EditChannel.ermweb.tswl"
#define RemoveChannelPage		"RemoveChannel.ermweb.tswl"
#define AddDevicePage			"AddDevice.ermweb.tswl"
#define ShowDevicePage			"ShowDevice.ermweb.tswl"
#define RemoveDevicePage		"RemoveDevice.ermweb.tswl"
#define EditDevicePage			"EditDevice.ermweb.tswl"
#define EditQAMPage			    "EditQAM.ermweb.tswl"
#define ShowAllocationPage      "ShowAllocation.ermweb.tswl"
#define ShowEdgePortPage        "ShowEdgePort.ermweb.tswl"
#define ChannelDetailPage       "ChannelDetail.ermweb.tswl"
//#define EditServiceGroupPage    "EditServiceGroup.ermweb.tswl"
//#define ShowServiceGroupPage    "ShowServiceGroup.ermweb.tswl"
#define EditRouteNamesPage      "EditRouteNames.ermweb.tswl"
#define ShowRouteNamesPage      "ShowRouteNames.ermweb.tswl"

#define CHANNEL_SAFESTORE       "D:\\ERM\\ERM-Channels.ini"
#define DEVICE_SAFESTORE        "D:\\ERM\\ERM-Devices.ini"
#define ALLOCATION_SAFESTORE    "D:\\ERM\\ERM-Allocations.ini"

#define LinkSpace responser<<" | ";

namespace ErmWebPage
{
	class BasePage
	{
	public: 
		BasePage(IHttpRequestCtx* pHttpRequestCtx);
		virtual ~BasePage();

	public: 
		const char* getLastError() const;
		void setLastError(const char* error);
		void addToLastError(const char* error);
		bool process();
		void splitString(std::vector< std::string > &result, const std::string &str, const std::string &delimiter);
		std::string neighborLayout(const std::string& currentTemplate, const std::string& neighborLayoutId);

	protected:
		bool isPostBack() const;
/*
		bool readChannels(ChannelInfos& channelInfos, string path = "");
		bool writeChannels(ChannelInfos& channelInfos, string path = "");
		bool appendChannel(ChannelInfo& channelInfo, string path = "");
		bool modifyChannel(ChannelInfo& channelInfo, string path = "");
*/
/*
		bool readChannels(TianShanIce::EdgeResource::EdgeChannelInfos& channelInfos, string path = "");
		bool appendChannel(EdgeChannelInfo& channelInfo, string path = "");
		bool modifyChannel(EdgeChannelInfo& channelInfo, string path = "");
		bool removeChannel(string channelName, string path = "");

		bool readDevices(TianShanIce::EdgeResource::EdgeDeviceInfos& deviceInfos, string path = "");
		bool appendDevice(EdgeDeviceInfo& deviceInfo, string path = "");
		bool modifyDevice(EdgeDeviceInfo& deviceInfo, string path = "");
		bool removeDevice(string deviceName, string path = "");

		bool readAllocations(TianShanIce::EdgeResource::AllocationInfos& allocationInfos, string path = "");
		bool appendAllocation(AllocationInfo& allocationInfo, string path = "");
		bool modifyAllocation(AllocationInfo& allocationInfo, string path = "");
		bool removeAllocation(string allocationName, string path = "");
*/

		virtual bool get() = 0;
		virtual bool post() = 0;

	protected: 
		std::string _lastError;
		char szBuf[8 * 1024];
		ZQ::common::URLStr url;

		Ice::CommunicatorPtr _ic;
		TianShanIce::EdgeResource::EdgeResouceManagerPrx _ERM;

		IHttpRequestCtx* _reqCtx;
		IHttpRequestCtx::RequestVars _varMap;

	};
}
#endif // __EdgeResourceManager_BasePage_H__

