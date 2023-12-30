#ifndef __CRM_DMSCC_CONFIG_H__
#define __CRM_DMSCC_CONFIG_H__

#include <ZQ_common_conf.h>
#include "ConfigHelper.h"
#include "FileLog.h"
#include <list>
#include <utility>
using namespace ZQ::common;
struct FixedSpeedSet 
{
	int32			speedSetEnable;
	int32			speedSetMode;
//	int32			enableSpeedLoop;
	int32			speedSetErrorcode;
	std::string		strForwardSpeeds;
	std::string		strBackwardSpeeds;
	std::string		strInputFF;
	std::string		strInputREW;
	typedef ZQ::common::Config::Holder<FixedSpeedSet> FixedSpeedSetHolder;
	static void structure(FixedSpeedSetHolder& holder )
	{
		holder.addDetail("","enable",&FixedSpeedSet::speedSetEnable,"0",ZQ::common::Config::optReadOnly);	
		holder.addDetail("","mode",&FixedSpeedSet::speedSetMode,"1",ZQ::common::Config::optReadOnly);	
//		holder.addDetail("","enableSpeedLoop",&FixedSpeedSet::enableSpeedLoop,"1",ZQ::common::Config::optReadOnly);	
		holder.addDetail("","errorCode",&FixedSpeedSet::speedSetErrorcode,"455",ZQ::common::Config::optReadOnly);
		holder.addDetail("","forward",&FixedSpeedSet::strForwardSpeeds,"",ZQ::common::Config::optReadOnly);
		holder.addDetail("","backward",&FixedSpeedSet::strBackwardSpeeds,"",ZQ::common::Config::optReadOnly);
		holder.addDetail("","inputFF",&FixedSpeedSet::strInputFF,"",ZQ::common::Config::optReadOnly);
		holder.addDetail("","inputREW",&FixedSpeedSet::strInputREW,"",ZQ::common::Config::optReadOnly);
	}
};

struct NodeGroup 
{
	int32 rangeStart;
	int32 rangeStop;
	std::string SMEndpoint;
	static void structure(ZQ::common::Config::Holder<NodeGroup> &holder )
	{
		holder.addDetail("", "rangeStart",&NodeGroup::rangeStart,"0",ZQ::common::Config::optReadOnly);
		holder.addDetail("", "rangeEnd",&NodeGroup::rangeStop,"0",ZQ::common::Config::optReadOnly);
		holder.addDetail("", "SM",&NodeGroup::SMEndpoint,"",ZQ::common::Config::optReadOnly);
	}
};
typedef std::vector< ZQ::common::Config::Holder<NodeGroup> >NodeGroups;

struct PhysicalChannel
{
	int convertFreq;
	static void structure(ZQ::common::Config::Holder<PhysicalChannel>& holder )
	{
		holder.addDetail("","convertFreq",&PhysicalChannel::convertFreq,"0",ZQ::common::Config::optReadOnly);	
	}
};
// config loader structure
struct CRMDmsccCfg
{	
///Default config
	// defines the TianshanEvent properties
	std::string EventChannelEndPoint;
	std::string ListenEventEndPoint;
    
	// define Ice trace log info
	int32 iceTraceEnable;
	int32 iceTraceLevel;
	int32 iceTraceSize;
	int32 iceTraceCount; 

///crm_dmscc config
	//log file config
    int32 logFileSize;
	int32 logFileLevel;
	int32 logFileCount;
	int32 logFileBufferSize;

	//weiwoo service endpoint
	std::string sessionMgrEndpoint;
	int32 heartbeatInterval;
	int32 optionalInterval;

	NodeGroups nodegroups;

	//client session config
	int32 csTimeout;
	int32 csMonitorThreads;

	//stream ctrl config
	int32 proxyMode;
	int32 lscUdp;
	//FixedSpeedSet
	FixedSpeedSet::FixedSpeedSetHolder	_fixedSpeedSet;
	std::vector<float>	forwardSpeeds;
	std::vector<float>	backwardSpeeds;
	std::vector<float>	inputFFs;
	std::vector<float>	inputREWs;

	//Application URLpattern
	std::string appURLPattern;
	
	//eosNptErr
	int32 eosNptErr;
	//TS pumper config
	int32 SGAdsInterval;
	std::string SGAdsTsFolder;
	std::string SGAdsBindIp;
	int32 SGAdsHexMode;
	std::string SGAdsDeHexCommand;

	//Resource config
	//MpegProgram  resource
	  int32 pmtPid;//uint32
	  int32 caPid;//uint32
    //TsDownBW  resource
	  int32 tsBWTsid;
	  //EthernetInterface  resource
	  std::string srcMac;
	  std::string srcIp;
	  int32      srcPort; //uint16
	  std::string destIp;
	  std::string destPort;
	  //AtscModulationMode  resource
      int32 transSystem; //uint8
	  int32 interleaveDepth; //uint8
	  int32 modulationMode; //uint8
	  int32 FEC; //uint8
	  //HeadendId resource
	  std::string headendId;
	  int32      headendFlag;//uint16
	  int32      headendtsid;//uint16

	  int32 convertFreq;
	  void readNodeGroup(ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP)
	  {
		  Config::Holder<NodeGroup> itemholder("");
		  itemholder.read(node, hPP);
		  nodegroups.push_back(itemholder);
	  }

	  void readFixedSpeedSet( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP )
	  {
		  _fixedSpeedSet.read(node,hPP);
	  }
	  void regsiterFixedSpeedSet(const std::string& full_path )
	  {
		  _fixedSpeedSet.snmpRegister(full_path);
	  }

	  void readPhysicalChannel( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP )
	  {
		  Config::Holder<PhysicalChannel> itemholder("");
		  itemholder.read(node, hPP);
		  convertFreq = itemholder.convertFreq;
	  }
	static void structure(::ZQ::common::Config::Holder<CRMDmsccCfg> &holder)
	{
	///default
		//TianshanEvent
		holder.addDetail("default/TianShanEvents", "EventChannelEndPoint", &CRMDmsccCfg::EventChannelEndPoint, "", ZQ::common::Config::optReadOnly);
		holder.addDetail("default/TianShanEvents", "listenEndpoint", &CRMDmsccCfg::ListenEventEndPoint, "", ZQ::common::Config::optReadOnly);
        //IceTrace
		holder.addDetail("default/IceTrace", "enabled", &CRMDmsccCfg::iceTraceEnable, "1", ZQ::common::Config::optReadOnly);
		holder.addDetail("default/IceTrace", "level", &CRMDmsccCfg::iceTraceLevel, "7", ZQ::common::Config::optReadOnly);
		holder.addDetail("default/IceTrace", "size", &CRMDmsccCfg::iceTraceSize, "50000000", ZQ::common::Config::optReadOnly);
		holder.addDetail("default/IceTrace", "maxCount", &CRMDmsccCfg::iceTraceCount, "10", ZQ::common::Config::optReadOnly);

    ///crm_dmscc config
		// log file
		holder.addDetail("crm_Dsmcc/LogFile", "size", &CRMDmsccCfg::logFileSize, "52428800", ZQ::common::Config::optReadOnly);
		holder.addDetail("crm_Dsmcc/LogFile", "level", &CRMDmsccCfg::logFileLevel, "7", ZQ::common::Config::optReadOnly);
		holder.addDetail("crm_Dsmcc/LogFile", "maxCount", &CRMDmsccCfg::logFileCount, "10", ZQ::common::Config::optReadOnly);
		holder.addDetail("crm_Dsmcc/LogFile", "bufferSize", &CRMDmsccCfg::logFileBufferSize, "8192", ZQ::common::Config::optReadOnly);
        //sessionMgr 
		holder.addDetail("crm_Dsmcc/SessionManager", "endpoint", &CRMDmsccCfg::sessionMgrEndpoint, "", ZQ::common::Config::optReadOnly);
		holder.addDetail("crm_Dsmcc/SessionManager", "heartbeatInterval", &CRMDmsccCfg::heartbeatInterval, "600000", ZQ::common::Config::optReadOnly);
		holder.addDetail("crm_Dsmcc/SessionManager", "optionalInterval", &CRMDmsccCfg::optionalInterval, "120000", ZQ::common::Config::optReadOnly);
		holder.addDetail("crm_Dsmcc/SessionManager/NodeGroup",&CRMDmsccCfg::readNodeGroup,&CRMDmsccCfg::registerNothing, Config::Range(0, -1));

		//client session
		holder.addDetail("crm_Dsmcc/ClientSession", "timeout", &CRMDmsccCfg::csTimeout, "", ZQ::common::Config::optReadOnly);
		holder.addDetail("crm_Dsmcc/ClientSession", "monitorThreads", &CRMDmsccCfg::csMonitorThreads, "", ZQ::common::Config::optReadOnly);

		//stream ctrl
		holder.addDetail("crm_Dsmcc/StreamCtrl", "proxyMode", &CRMDmsccCfg::proxyMode, "", ZQ::common::Config::optReadOnly);
		holder.addDetail("crm_Dsmcc/StreamCtrl", "lscUdp", &CRMDmsccCfg::lscUdp, "", ZQ::common::Config::optReadOnly);

		//FixedSpeedSet
		holder.addDetail("crm_Dsmcc/StreamCtrl/FixedSpeedSet",&CRMDmsccCfg::readFixedSpeedSet,&CRMDmsccCfg::regsiterFixedSpeedSet);
       
	//application URLPattern
		holder.addDetail("crm_Dsmcc/Application", "urlPattern", &CRMDmsccCfg::appURLPattern, "", ZQ::common::Config::optReadOnly);
       // ts pumpr
		holder.addDetail("crm_Dsmcc/ServiceGroupAds", "interval", &CRMDmsccCfg::SGAdsInterval, "", ZQ::common::Config::optReadOnly);
		holder.addDetail("crm_Dsmcc/ServiceGroupAds", "tsFolder", &CRMDmsccCfg::SGAdsTsFolder, "", ZQ::common::Config::optReadOnly);
		holder.addDetail("crm_Dsmcc/ServiceGroupAds", "bindIp", &CRMDmsccCfg::SGAdsBindIp, "", ZQ::common::Config::optReadOnly);
		holder.addDetail("crm_Dsmcc/ServiceGroupAds", "hexMode", &CRMDmsccCfg::SGAdsHexMode, "", ZQ::common::Config::optReadOnly);
		holder.addDetail("crm_Dsmcc/ServiceGroupAds", "deHexCommand", &CRMDmsccCfg::SGAdsDeHexCommand, "", ZQ::common::Config::optReadOnly);
      
		holder.addDetail("crm_Dsmcc/Event", "eosNptErr", &CRMDmsccCfg::eosNptErr, "5000", ZQ::common::Config::optReadOnly);

		//Resource
		holder.addDetail("DefaultResource/MpegProgram", "pmtPId", &CRMDmsccCfg::pmtPid, "65536", ZQ::common::Config::optReadOnly);
		holder.addDetail("DefaultResource/MpegProgram", "caPid", &CRMDmsccCfg::caPid, "65536", ZQ::common::Config::optReadOnly);
		
		holder.addDetail("DefaultResource/TsDownstreamBandwidth", "tsid", &CRMDmsccCfg::tsBWTsid, "0", ZQ::common::Config::optReadOnly);

		holder.addDetail("DefaultResource/EthernetInterface", "srcMac", &CRMDmsccCfg::srcMac, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail("DefaultResource/EthernetInterface", "srcIP", &CRMDmsccCfg::srcIp, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail("DefaultResource/EthernetInterface", "srcPort", &CRMDmsccCfg::srcPort, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail("DefaultResource/EthernetInterface", "destIP", &CRMDmsccCfg::destIp, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail("DefaultResource/EthernetInterface", "destPort", &CRMDmsccCfg::destPort, "0", ZQ::common::Config::optReadOnly);

		holder.addDetail("DefaultResource/AtscModulationMode", "transmissionSystem", &CRMDmsccCfg::transSystem, "2", ZQ::common::Config::optReadOnly);
		holder.addDetail("DefaultResource/AtscModulationMode", "interleaveDepth", &CRMDmsccCfg::interleaveDepth, "255", ZQ::common::Config::optReadOnly);
		holder.addDetail("DefaultResource/AtscModulationMode", "modulationMode", &CRMDmsccCfg::modulationMode, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail("DefaultResource/AtscModulationMode", "FEC", &CRMDmsccCfg::FEC, "0", ZQ::common::Config::optReadOnly);

		holder.addDetail("DefaultResource/HeadendId", "id", &CRMDmsccCfg::headendId, "00000000000000000000", ZQ::common::Config::optReadOnly);
		holder.addDetail("DefaultResource/HeadendId", "flag", &CRMDmsccCfg::headendFlag, "1", ZQ::common::Config::optReadOnly);
		holder.addDetail("DefaultResource/HeadendId", "tsid", &CRMDmsccCfg::headendtsid, "0", ZQ::common::Config::optReadOnly);
		holder.addDetail("crm_Dsmcc/PhysicalChannel",&CRMDmsccCfg::readPhysicalChannel,&CRMDmsccCfg::registerNothing, Config::Range(0, 1));

	}

	void registerNothing(const std::string&){}
};
extern ZQ::common::Config::Loader< CRMDmsccCfg >_CRMDmsccConfig;

#endif //__CRM_DMSCC_CONFIG_H__
