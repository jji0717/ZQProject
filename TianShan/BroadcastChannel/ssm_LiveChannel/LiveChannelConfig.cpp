#include "LiveChannelConfig.h"

namespace LiveChannel 
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

void BcastPublish::structure(BcastPublishHolder& holder)
{
	holder.addDetail("", "endpoint", &BcastPublish::_endpoint, "", ZQ::common::Config::optReadOnly);
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

void SessionGroups::structure(SessionGroupsHolder& holder)
{
	holder.addDetail("", "defaultServiceGroup", &SessionGroups::defaultServiceGroup, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("qam", &SessionGroups::readSessionGroups, &SessionGroups::registerSessionGroups);
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

void NGODS1::structure(NGODS1Holder& holder)
{
	holder.addDetail("", "defaultAppPath", &NGODS1::defaultAppPath, "60010001", ZQ::common::Config::optReadOnly);
	holder.addDetail("SessionGroups", &NGODS1::readSessionGroups, &NGODS1::registerSessionGroups);
}

void NGODS1::readSessionGroups(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_sessionGroups.read(node, hPP);
}

void NGODS1::registerSessionGroups(const std::string &full_path)
{
	_sessionGroups.snmpRegister(full_path);
}

void LiveChannelConfig::structure(LiveChannelConfigHolder& holder)
{
	holder.addDetail("ssm_LiveChannel", "path", &LiveChannelConfig::_path, "LiveChannel", ZQ::common::Config::optReadOnly);

	holder.addDetail("default/EventChannel", &LiveChannelConfig::readIceStrom, &LiveChannelConfig::registerIceStrom);
	holder.addDetail("default/IceTrace", &LiveChannelConfig::readIceTrace, &LiveChannelConfig::registerIceTrace);
	holder.addDetail("default/Database", &LiveChannelConfig::readDatabase, &LiveChannelConfig::registerDatabase);

	holder.addDetail("ssm_LiveChannel/LogFile", &LiveChannelConfig::readLogFile, &LiveChannelConfig::registerLogFile);
	holder.addDetail("ssm_LiveChannel/BcastPublish", &LiveChannelConfig::readBcastPublish, &LiveChannelConfig::registerBcastPublish);
	//holder.addDetail("ssm_LiveChannel/SessionManager", &LiveChannelConfig::readSessionManager, &LiveChannelConfig::registerSessionManager);
	holder.addDetail("ssm_LiveChannel/Bind", &LiveChannelConfig::readBind, &LiveChannelConfig::registerBind);
	holder.addDetail("ssm_LiveChannel/RTSPSession", &LiveChannelConfig::readRTSPSession, &LiveChannelConfig::registerRTSPSession);
	holder.addDetail("ssm_LiveChannel/DefaultParameter", &LiveChannelConfig::readDefaultParams, &LiveChannelConfig::registerDefaultParams);
	holder.addDetail("ssm_LiveChannel/Response", &LiveChannelConfig::readResponse, &LiveChannelConfig::registerResponse);
	holder.addDetail("ssm_LiveChannel/DoSProtection", &LiveChannelConfig::readDoSProtection, &LiveChannelConfig::registerDoSProtection);
	holder.addDetail("ssm_LiveChannel/IceProperties", &LiveChannelConfig::readIcePropertys, &LiveChannelConfig::registerIcePropertys);
	holder.addDetail("ssm_LiveChannel/PublishedLogs/Log", &LiveChannelConfig::readLogPublish, &LiveChannelConfig::registerIcePropertys);
	holder.addDetail("ssm_LiveChannel/VirtualServiceGroup", &LiveChannelConfig::readVirtualServiceGroup, &LiveChannelConfig::registerVirtualServiceGroup);
	holder.addDetail("ssm_LiveChannel/FixedSpeedSet",&LiveChannelConfig::readFixedSpeedSet,&LiveChannelConfig::regsiterFixedSpeedSet );
	holder.addDetail("ssm_LiveChannel/NGODS1",&LiveChannelConfig::readNGODS1,&LiveChannelConfig::registerNGODS1 );
}

void LiveChannelConfig::readIceStrom(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_iceStorm.read(node, hPP);
}

void LiveChannelConfig::registerIceStrom(const std::string &full_path)
{
	_iceStorm.snmpRegister(full_path);
}

void LiveChannelConfig::readIceTrace(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_iceTrace.read(node, hPP);
}

void LiveChannelConfig::registerIceTrace(const std::string &full_path)
{
	_iceTrace.snmpRegister(full_path);
}

void LiveChannelConfig::readDatabase(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_database.read(node, hPP);
}

void LiveChannelConfig::registerDatabase(const std::string &full_path)
{
	_database.snmpRegister(full_path);
}

void LiveChannelConfig::readLogFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_pluginLog.read(node, hPP);
}

void LiveChannelConfig::registerLogFile(const std::string &full_path)
{
	_pluginLog.snmpRegister(full_path);
}

void LiveChannelConfig::readBcastPublish(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_bcastPublish.read(node, hPP);
}

void LiveChannelConfig::registerBcastPublish(const std::string &full_path)
{
	_bcastPublish.snmpRegister(full_path);
}

void LiveChannelConfig::readBind(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_bind.read(node, hPP);
}

void LiveChannelConfig::registerBind(const std::string &full_path)
{
	_bind.snmpRegister(full_path);
}

void LiveChannelConfig::readRTSPSession(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_rtspSession.read(node, hPP);
}

void LiveChannelConfig::registerRTSPSession(const std::string &full_path)
{
	_rtspSession.snmpRegister(full_path);
}

void LiveChannelConfig::readDefaultParams(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_defaultParams.read(node, hPP);
}

void LiveChannelConfig::registerDefaultParams(const std::string &full_path)
{
	_defaultParams.snmpRegister(full_path);
}

void LiveChannelConfig::readResponse(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_response.read(node, hPP);
}

void LiveChannelConfig::registerResponse(const std::string &full_path)
{
	_response.snmpRegister(full_path);
}

void LiveChannelConfig::readDoSProtection(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_doSProtect.read(node, hPP);
}

void LiveChannelConfig::registerDoSProtection(const std::string &full_path)
{
	_doSProtect.snmpRegister(full_path);
}

void LiveChannelConfig::readIcePropertys(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_iceProps.read(node, hPP);
}

void LiveChannelConfig::registerIcePropertys(const std::string &full_path)
{
	_iceProps.snmpRegister(full_path);
}

void LiveChannelConfig::readLogPublish( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
{
	ZQ::common::Config::Holder<LogPublish> pb;
	
	pb.read(node,hPP);
	_logpublish.push_back( pb );
}

void VirtualServiceGroup::structure(VirtualServiceGroupHolder& holder)
{
	holder.addDetail("", "group", &VirtualServiceGroup::group, "1", ZQ::common::Config::optReadOnly);
}

void LiveChannelConfig::readVirtualServiceGroup(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_virtualServiceGroup.read(node, hPP);
}

void LiveChannelConfig::registerVirtualServiceGroup(const std::string &full_path)
{
	_virtualServiceGroup.snmpRegister(full_path);
}

void LiveChannelConfig::readFixedSpeedSet( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP )
{
	_fixedSpeedSet.read(node,hPP);
}

void LiveChannelConfig::regsiterFixedSpeedSet(const std::string& full_path )
{
	_fixedSpeedSet.snmpRegister(full_path);
}

void LiveChannelConfig::readNGODS1(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_ngods1.read(node,hPP);
}

void LiveChannelConfig::registerNGODS1(const std::string &full_path)
{
	_ngods1.snmpRegister(full_path);
}
}

