#include "TsConfig.h"

namespace TianShanS1
{


void FixedSpeedSet::structure(FixedSpeedSet::FixedSpeedSetHolder& holder )
{
	holder.addDetail("","enable",&FixedSpeedSet::enable,"0",ZQ::common::Config::optReadOnly);	
	holder.addDetail("","enableSpeedLoop",&FixedSpeedSet::enableSpeedLoop,"1",ZQ::common::Config::optReadOnly);	
	holder.addDetail("","errorCode",&FixedSpeedSet::errorcode,"455",ZQ::common::Config::optReadOnly);
	holder.addDetail("","forward",&FixedSpeedSet::strForwardSpeeds,"",ZQ::common::Config::optReadOnly);
	holder.addDetail("","backward",&FixedSpeedSet::strBackwardSpeeds,"",ZQ::common::Config::optReadOnly);
}

void IceStorm::structure(IceStormHolder& holder)
{
	holder.addDetail("", "endpoint", &IceStorm::_endpoint, "", ZQ::common::Config::optReadOnly);
}

void LogPublish::structure(  ZQ::common::Config::Holder<LogPublish>& holder )
{
	holder.addDetail("","path",&LogPublish::logPath,"",ZQ::common::Config::optReadOnly);
	holder.addDetail("","syntax",&LogPublish::syntaxPath,"",ZQ::common::Config::optReadOnly);
	holder.addDetail("","key",&LogPublish::key,"");
	holder.addDetail("","type",&LogPublish::type,"");
};

void StreamCtrl::structure(StreamCtrlHolder& holder)
{
    holder.addDetail("", "exportEndpoint", &StreamCtrl::exportEndpoint, "", ZQ::common::Config::optReadOnly);
}

void IceTrace::structure(IceTraceHolder& holder)
{
	holder.addDetail("", "enabled", &IceTrace::_enabled, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "level", &IceTrace::_level, "6", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "size", &IceTrace::_size, "10000000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxCount", &IceTrace::_maxCount, "5", ZQ::common::Config::optReadOnly);
}

void Database::structure(DatabaseHolder& holder)
{
	holder.addDetail("", "path", &Database::_path, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "runtimePath", &Database::_runtimePath, "", ZQ::common::Config::optReadOnly);
    holder.addDetail("", "fatalRecover", &Database::_fatalRecover, "1", ZQ::common::Config::optReadOnly);
    holder.addDetail("", "checkpointPeriod", &Database::_checkpointPeriod, "120", ZQ::common::Config::optReadOnly);
    holder.addDetail("", "saveSizeTrigger", &Database::_saveSizeTrigger, "10", ZQ::common::Config::optReadOnly);
    holder.addDetail("", "savePeriod", &Database::_savePeriod, "60000", ZQ::common::Config::optReadOnly);
    holder.addDetail("", "evictorSize", &Database::_evictorSize, "5000", ZQ::common::Config::optReadOnly);
}

void LogFile::structure(LogFileHolder& holder)
{
	holder.addDetail("", "size", &LogFile::_size, "10000000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "level", &LogFile::_level, "6", ZQ::common::Config::optReadOnly);	
	holder.addDetail("", "maxCount", &LogFile::_maxCount, "5", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "bufferSize", &LogFile::_bufferSize, "16000", ZQ::common::Config::optReadOnly);
}

void AnnounceCfg::structure(AnnounceCfgHolder& holder)
{
	//holder.addDetail("", "notifyTransition", &AnnounceCfg::_notifyTransition, "1", ZQ::common::Config::optReadOnly);
	//holder.addDetail("", "notifyTrickRestriction", &AnnounceCfg::_notifyTrickRestriction, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "notifyItemSkipped", &AnnounceCfg::_notifyItemSkipped, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "notifyStateChanged", &AnnounceCfg::_notifyStateChanged, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "notifyScaleChanged", &AnnounceCfg::_notifyScaleChanged, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "notifyBOS", &AnnounceCfg::_notifyBOS, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "notifyEOS", &AnnounceCfg::_notifyEOS, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "notifyExit", &AnnounceCfg::_notifyExit, "1", ZQ::common::Config::optReadOnly);
	//holder.addDetail("", "notifyErrors", &AnnounceCfg::_notifyErrors, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "notifyAds", &AnnounceCfg::_notifyAds, "0", ZQ::common::Config::optReadOnly);
}

void NodeGroup::structure(NodeGroupHolder& holder)
{
	holder.addDetail("", "rangeStart", &NodeGroup::_rangeStart, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "rangeEnd", &NodeGroup::_rangeEnd, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "SM", &NodeGroup::_sm, "", ZQ::common::Config::optReadOnly);
}

void SessionManager::structure(SessionManagerHolder& holder)
{
	holder.addDetail("", "endpoint", &SessionManager::_endpoint, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("NodeGroup", &SessionManager::readNodeGroups, &SessionManager::registerNodeGroups);
}

void SessionManager::readNodeGroups(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	NodeGroup::NodeGroupHolder ngodGroupHolder;
	ngodGroupHolder.read(node, hPP);
	_ngodGroups.push_back(ngodGroupHolder);
}

void SessionManager::registerNodeGroups(const std::string &full_path)
{
	for (std::vector<NodeGroup::NodeGroupHolder>::iterator 
		it = _ngodGroups.begin(); 
		it != _ngodGroups.end(); it ++)
	{
		it->snmpRegister(full_path);
	}
}

void Bind::structure(BindHolder& holder)
{
	holder.addDetail("", "endpoint", &Bind::_endpoint, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "dispatchSize", &Bind::_dispatchSize, "3", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "dispatchMax", &Bind::_dispatchMax, "5", ZQ::common::Config::optReadOnly);
}

void RTSPSession::structure(RTSPSessionHolder& holder)
{
	holder.addDetail("", "timeout", &RTSPSession::_timeout, "600", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "cacheSize", &RTSPSession::_cacheSize, "500", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "monitorThreads", &RTSPSession::_monitorThreads, "3", ZQ::common::Config::optReadOnly);
	holder.addDetail("","eventResubscribeAtIdle",&RTSPSession::_announceResubscribeAtIdle,"60000",ZQ::common::Config::optReadOnly);
	holder.addDetail("","pingCacheSize",&RTSPSession::_pingCacheSize,"2000",ZQ::common::Config::optReadOnly);
	//_setupMsgTimeoutInterval
	holder.addDetail("", "setupTimeoutInterval", &RTSPSession::_setupMsgTimeoutInterval, "10000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxPendingAnnounce", &RTSPSession::_maxPendingAnnounce, "100", ZQ::common::Config::optReadOnly);	
}

void DefaultParam::structure(DefaultParamHolder& holder)
{
	holder.addDetail("", "name", &DefaultParam::_name, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "value", &DefaultParam::_value, "", ZQ::common::Config::optReadOnly);	
}

void DefaultParams::structure(DefaultParamsHolder& holder)
{
	holder.addDetail("param", &DefaultParams::readDefaultParams, &DefaultParams::registerDefaultParams);
	holder.addDetail("AxiomMsg", "defaultPath", &DefaultParams::_axiomMsgDefaultPath, "", ZQ::common::Config::optReadOnly);
}

void DefaultParams::readDefaultParams(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	DefaultParamHolder paramHolder("name");
	paramHolder.read(node, hPP);
	_paramDatas.push_back(paramHolder);
}

void DefaultParams::registerDefaultParams(const std::string &full_path)
{
	for (std::vector<DefaultParamHolder>::iterator 
		it = _paramDatas.begin(); 
		it != _paramDatas.end(); it ++)
	{
		it->snmpRegister(full_path);
	}
}

void Response::structure(ResponseHolder& holder)
{
	holder.addDetail("", "maxFieldLen", &Response::_maxFieldLen, "200", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "dummyEndNpt", &Response::_dummyEndNpt, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxResponseTimeout", &Response::_maxResponseTimeout, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "exportPlaylist", &Response::_exportPlaylist, "0", ZQ::common::Config::optReadOnly);
}

void DoSProtection::structure(DoSProtectionHolder& holder)
{
	holder.addDetail("", "timeWindow", &DoSProtection::_timeWindow, "5", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxRequests", &DoSProtection::_maxRequests, "50", ZQ::common::Config::optReadOnly);
}

void IceProperty::structure(IcePropertyHolder& holder)
{
	holder.addDetail("", "name", &IceProperty::_name, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "value", &IceProperty::_value, "", ZQ::common::Config::optReadOnly);
}

void IcePropertys::structure(IcePropertysHolder& holder)
{
	holder.addDetail("prop", &IcePropertys::readIcePropertys, &IcePropertys::registerIcePropertys);
}

void IcePropertys::readIcePropertys(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	IceProperty::IcePropertyHolder propHolder("name");
	propHolder.read(node, hPP);
	_propDatas.push_back(propHolder);
}

void IcePropertys::registerIcePropertys(const std::string &full_path)
{
	for (std::vector<IceProperty::IcePropertyHolder>::iterator 
		it = _propDatas.begin(); 
		it != _propDatas.end(); it ++)
	{
		it->snmpRegister(full_path);
	}
}

void SessionGroup::structure(SessionGroupHolder& holder)
{
	holder.addDetail("", "expression", &SessionGroup::expression, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "group", &SessionGroup::group, "", ZQ::common::Config::optReadOnly);
}

void IpSubnet::structure(IpSubnetHolder& holder)
{
	holder.addDetail("", "ip", &IpSubnet::ip, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "mask", &IpSubnet::mask, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "group", &IpSubnet::group, "", ZQ::common::Config::optReadOnly);
}

void SessionGroups::structure(SessionGroupsHolder& holder)
{
	holder.addDetail("", "defaultServiceGroup", &SessionGroups::defaultServiceGroup, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("qam", &SessionGroups::readSessionGroups, &SessionGroups::registerSessionGroups);
	holder.addDetail("IpSubnet", &SessionGroups::readIpSubnets, &SessionGroups::registerIpSubnets);
}

void SessionGroups::readSessionGroups(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	SessionGroup::SessionGroupHolder qamHolder("name");
	qamHolder.read(node, hPP);
	sessionGroups.push_back(qamHolder);
}

void SessionGroups::registerSessionGroups(const std::string &full_path)
{
	for (std::vector<SessionGroup::SessionGroupHolder>::iterator 
		it = sessionGroups.begin(); 
		it != sessionGroups.end(); it ++)
	{
		it->snmpRegister(full_path);
	}
}

void SessionGroups::readIpSubnets(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	IpSubnet::IpSubnetHolder ipSubnetHolder;
	ipSubnetHolder.read(node, hPP);
	ipSubnets.push_back(ipSubnetHolder);
}

void SessionGroups::registerIpSubnets(const std::string &full_path)
{
	for (std::vector<IpSubnet::IpSubnetHolder>::iterator 
		it = ipSubnets.begin(); 
		it != ipSubnets.end(); it ++)
	{
		it->snmpRegister(full_path);
	}
}

void NGODS1::structure(NGODS1Holder& holder)
{
	holder.addDetail("", "defaultAppPath", &NGODS1::defaultAppPath, "60010001", ZQ::common::Config::optReadOnly);
	holder.addDetail("SessionGroups", &NGODS1::readSessionGroups, &NGODS1::registerSessionGroups);
    holder.addDetail("StreamCtrl", &NGODS1::readStreamCtrl, &NGODS1::registerStreamCtrl);
}

void NGODS1::readSessionGroups(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_sessionGroups.read(node, hPP);
}

void NGODS1::registerSessionGroups(const std::string &full_path)
{
	_sessionGroups.snmpRegister(full_path);
}

void NGODS1::readStreamCtrl(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    _streamCtrl.read(node, hPP);
}

void NGODS1::registerStreamCtrl(const std::string &full_path)
{
    _streamCtrl.snmpRegister(full_path);
}

void C2Stream::structure(C2StreamHolder& holder)
{
	holder.addDetail("", "enable", &C2Stream::enable, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "sessionTTL", &C2Stream::sessionTTL, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "keyfile", &C2Stream::keyfile, "", ZQ::common::Config::optReadOnly);
}

void TSConfig::structure(TSConfigHolder& holder)
{
	holder.addDetail("default/EventChannel", &TSConfig::readIceStrom, &TSConfig::registerIceStrom);
	holder.addDetail("default/IceTrace", &TSConfig::readIceTrace, &TSConfig::registerIceTrace);
	holder.addDetail("default/Database", &TSConfig::readDatabase, &TSConfig::registerDatabase);

	holder.addDetail("ssm_TianShan_s1", "netId", &TSConfig::_NetId,"", ZQ::common::Config::optReadOnly);

	holder.addDetail("ssm_TianShan_s1/LogFile", &TSConfig::readLogFile, &TSConfig::registerLogFile);
	holder.addDetail("ssm_TianShan_s1/Announce", &TSConfig::readAnnounceCfg, &TSConfig::registerAnnounceCfg);
	holder.addDetail("ssm_TianShan_s1/SessionManager", &TSConfig::readSessionManager, &TSConfig::registerSessionManager);
	holder.addDetail("ssm_TianShan_s1/Bind", &TSConfig::readBind, &TSConfig::registerBind);
	holder.addDetail("ssm_TianShan_s1/RTSPSession", &TSConfig::readRTSPSession, &TSConfig::registerRTSPSession);
	holder.addDetail("ssm_TianShan_s1/DefaultParameter", &TSConfig::readDefaultParams, &TSConfig::registerDefaultParams);
	holder.addDetail("ssm_TianShan_s1/Response", &TSConfig::readResponse, &TSConfig::registerResponse);
	holder.addDetail("ssm_TianShan_s1/DoSProtection", &TSConfig::readDoSProtection, &TSConfig::registerDoSProtection);
	holder.addDetail("ssm_TianShan_s1/IceProperties", &TSConfig::readIcePropertys, &TSConfig::registerIcePropertys);
	holder.addDetail("ssm_TianShan_s1/PublishedLogs/Log", &TSConfig::readLogPublish, &TSConfig::registerIcePropertys);
	holder.addDetail("ssm_TianShan_s1/VirtualServiceGroup", &TSConfig::readVirtualServiceGroup, &TSConfig::registerVirtualServiceGroup);
	holder.addDetail("ssm_TianShan_s1/FixedSpeedSet",&TSConfig::readFixedSpeedSet,&TSConfig::regsiterFixedSpeedSet );
	holder.addDetail("ssm_TianShan_s1/NGODS1",&TSConfig::readNGODS1,&TSConfig::registerNGODS1 );
	holder.addDetail("ssm_TianShan_s1/C2Stream",&TSConfig::readC2Stream,&TSConfig::registerC2Stream );
	holder.addDetail("ssm_TianShan_s1/RTSPSession", "scale0AtPaused", &TSConfig::scale0Paused,"", ZQ::common::Config::optReadOnly);
}

void TSConfig::readIceStrom(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_iceStorm.read(node, hPP);
}

void TSConfig::registerIceStrom(const std::string &full_path)
{
	_iceStorm.snmpRegister(full_path);
}

void TSConfig::readIceTrace(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_iceTrace.read(node, hPP);
}

void TSConfig::registerIceTrace(const std::string &full_path)
{
	_iceTrace.snmpRegister(full_path);
}

void TSConfig::readDatabase(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_database.read(node, hPP);
}

void TSConfig::registerDatabase(const std::string &full_path)
{
	_database.snmpRegister(full_path);
}

void TSConfig::readLogFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_pluginLog.read(node, hPP);
}

void TSConfig::registerLogFile(const std::string &full_path)
{
	_pluginLog.snmpRegister(full_path);
}

void TSConfig::readAnnounceCfg(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_announce.read(node, hPP);
}

void TSConfig::registerAnnounceCfg(const std::string &full_path)
{
	_announce.snmpRegister(full_path);
}

void TSConfig::readSessionManager(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_sessionManager.read(node, hPP);
}

void TSConfig::registerSessionManager(const std::string &full_path)
{
	_sessionManager.snmpRegister(full_path);
}

void TSConfig::readBind(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_bind.read(node, hPP);
}

void TSConfig::registerBind(const std::string &full_path)
{
	_bind.snmpRegister(full_path);
}

void TSConfig::readRTSPSession(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_rtspSession.read(node, hPP);
}

void TSConfig::registerRTSPSession(const std::string &full_path)
{
	_rtspSession.snmpRegister(full_path);
}

void TSConfig::readDefaultParams(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_defaultParams.read(node, hPP);
}

void TSConfig::registerDefaultParams(const std::string &full_path)
{
	_defaultParams.snmpRegister(full_path);
}

void TSConfig::readResponse(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_response.read(node, hPP);
}

void TSConfig::registerResponse(const std::string &full_path)
{
	_response.snmpRegister(full_path);
}

void TSConfig::readDoSProtection(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_doSProtect.read(node, hPP);
}

void TSConfig::registerDoSProtection(const std::string &full_path)
{
	_doSProtect.snmpRegister(full_path);
}

void TSConfig::readIcePropertys(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_iceProps.read(node, hPP);
}

void TSConfig::registerIcePropertys(const std::string &full_path)
{
	_iceProps.snmpRegister(full_path);
}

void TSConfig::readLogPublish( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
{
	ZQ::common::Config::Holder<LogPublish> pb;
	
	pb.read(node,hPP);
	_logpublish.push_back( pb );
}

void VirtualServiceGroup::structure(VirtualServiceGroupHolder& holder)
{
	holder.addDetail("", "group", &VirtualServiceGroup::group, "1", ZQ::common::Config::optReadOnly);
}

void TSConfig::readVirtualServiceGroup(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_virtualServiceGroup.read(node, hPP);
}

void TSConfig::registerVirtualServiceGroup(const std::string &full_path)
{
	_virtualServiceGroup.snmpRegister(full_path);
}

void TSConfig::readFixedSpeedSet( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP )
{
	_fixedSpeedSet.read(node,hPP);
}

void TSConfig::regsiterFixedSpeedSet(const std::string& full_path )
{
	_fixedSpeedSet.snmpRegister(full_path);
}

void TSConfig::readNGODS1(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_ngods1.read(node,hPP);
}

void TSConfig::registerNGODS1(const std::string &full_path)
{
	_ngods1.snmpRegister(full_path);
}

void TSConfig::readC2Stream(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_c2stream.read(node, hPP);
}

void TSConfig::registerC2Stream(const std::string &full_path)
{
	_c2stream.snmpRegister(full_path);
}

}

