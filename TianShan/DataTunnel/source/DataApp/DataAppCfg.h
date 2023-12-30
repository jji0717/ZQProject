#ifndef __DODAppCfgLoader_H__
#define __DODAppCfgLoader_H__
#include <ZQ_common_conf.h>
#include "ConfigHelper.h"
#include <list>
#include <map>
#include <string>
#include <vector>
using namespace ZQ::common;

struct DODAppCfg
{
	std::string				szDODAppEndpoint;

	std::string				szDODAppDBFolder;

	int32					lIceTraceLogLevel;
	int32					lIceTraceLogSize;

	int32					lLocalFoldNotifytime;

	int32					lRenewtime;
	int32					lStreamPingtime;

	int32					lDODAppServiceGroup;

	std::string				szSpaceName;

	std::string				szDODRtspURL;

	std::string				szDODContentEndpoint;

	std::string				szSRMEndpoint;

	std::string				szJBossIpPort;
	std::string				szConfigQueueName;
	int32				    lConfigTimeOut;
	int32				    lUsingJboss;
	std::string				szCachepath;

	// stores the ice properties
	std::map<std::string, std::string> icePropMap;

	static void structure(Config::Holder<DODAppCfg> &holder)
	{
		holder.addDetail("DataTunnel/Bind", "endpoint", &DODAppCfg::szDODAppEndpoint, NULL, Config::optReadOnly);	
		
		holder.addDetail("DataTunnel/DatabaseFolder", "path", &DODAppCfg::szDODAppDBFolder, "", Config::optReadOnly);	
		
		holder.addDetail("DataTunnel/IceTrace", "level", &DODAppCfg::lIceTraceLogLevel, "7", Config::optReadOnly);	
		holder.addDetail("DataTunnel/IceTrace", "size", &DODAppCfg::lIceTraceLogSize, "10240000", Config::optReadOnly);	
		
		holder.addDetail("DataTunnel/LocalFolder", "notifytime", &DODAppCfg::lLocalFoldNotifytime, "180", Config::optReadOnly);	
		
		holder.addDetail("DataTunnel/Stream", "renewtime", &DODAppCfg::lRenewtime, "60", Config::optReadOnly);	
		holder.addDetail("DataTunnel/Stream", "streampingtime", &DODAppCfg::lStreamPingtime, "90", Config::optReadOnly);	
		
		holder.addDetail("DataTunnel/ServiceGroup", "groupID", &DODAppCfg::lDODAppServiceGroup, "9999", Config::optReadOnly);	
		
		holder.addDetail("DataTunnel/SpaceName", "configname", &DODAppCfg::szSpaceName, "", Config::optReadOnly);	
		
		holder.addDetail("DataTunnel/RtspURL", "url", &DODAppCfg::szDODRtspURL, "Rtsp://www.i-zq.com/DataOndemand_DODApp?", Config::optReadOnly);	
		
		holder.addDetail("DataContent/Bind", "endpoint", &DODAppCfg::szDODContentEndpoint, NULL, Config::optReadOnly);	
		
		holder.addDetail("SRM/Bind", "endpoint", &DODAppCfg::szSRMEndpoint, NULL, Config::optReadOnly);	
		
		holder.addDetail("JmsDispatch/JBossIPPort", "ipport", &DODAppCfg::szJBossIpPort, NULL, Config::optReadOnly);	
		holder.addDetail("JmsDispatch/ConfigQueueName", "queuename", &DODAppCfg::szConfigQueueName, "queue/queue_cf", Config::optReadOnly);	
		holder.addDetail("JmsDispatch/ConfigMsgTimeOut", "timeout", &DODAppCfg::lConfigTimeOut, "30000", Config::optReadOnly);	
		holder.addDetail("JmsDispatch/SourceCachePath", "CachePath", &DODAppCfg::szCachepath, "", Config::optReadOnly);	

//		holder.addDetail("JmsDispatch/UsingJboss", "useflag", &DODAppCfg::lUsingJboss, "1", Config::optReadOnly, Config::Range(0,1));	

		holder.addDetail("DataTunnel/IceProperties/prop", &DODAppCfg::readProp, &DODAppCfg::registerNothing);

	};
	DODAppCfg()
	{
		lIceTraceLogLevel=7;
		lIceTraceLogSize=10*1024*1024;

		lLocalFoldNotifytime = 300;
		lDODAppServiceGroup  = 9999;
		lConfigTimeOut = 30000;
		lUsingJboss = 1;
	}
	void readProp(XMLUtil::XmlNode node, const Preprocessor* hPP)
	{
		Config::Holder<Config::NVPair> nvholder("");
		nvholder.read(node, hPP);
		icePropMap[nvholder.name] = nvholder.value;
	}
	void registerNothing(const std::string&){}
};

static void showCfg(const DODAppCfg& cfg)
{
	using namespace std;
	cout << "DODApp CfgLoader: \n";
}
#endif //__DODAppCfgLoader_H__