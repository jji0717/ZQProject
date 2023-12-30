#include "TsConfig.h"

namespace HSNTree
{


void FixedSpeedSet::structure(FixedSpeedSet::FixedSpeedSetHolder& holder )
{
	holder.addDetail("","enable",&FixedSpeedSet::enable,"0",ZQ::common::Config::optReadOnly);
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
	holder.addDetail("","key",&LogPublish::key,"",ZQ::common::Config::optReadOnly);
	holder.addDetail("","type",&LogPublish::type,"",ZQ::common::Config::optReadOnly);
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
}

void LogFile::structure(LogFileHolder& holder)
{
	holder.addDetail("", "size", &LogFile::_size, "10000000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "level", &LogFile::_level, "6", ZQ::common::Config::optReadOnly);	
	holder.addDetail("", "maxCount", &LogFile::_maxCount, "5", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "bufferSize", &LogFile::_bufferSize, "16000", ZQ::common::Config::optReadOnly);
}

void SessionManager::structure(SessionManagerHolder& holder)
{
	holder.addDetail("", "endpoint", &SessionManager::_endpoint, "", ZQ::common::Config::optReadOnly);
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
}

void DoSProtection::structure(DoSProtectionHolder& holder)
{
	holder.addDetail("", "timeWindow", &DoSProtection::_timeWindow, "5", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxRequests", &DoSProtection::_maxRequests, "50", ZQ::common::Config::optReadOnly);
}

void DWH::structure(DWHHolder& holder)
{
	holder.addDetail("", "enable", &DWH::_enable, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "endpoint", &DWH::_endpoint, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "activeConnections", &DWH::_activeConnections, "8", ZQ::common::Config::optReadOnly);
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

void TSConfig::structure(TSConfigHolder& holder)
{
	holder.addDetail("default/EventChannel", &TSConfig::readIceStrom, &TSConfig::registerIceStrom);
	holder.addDetail("default/IceTrace", &TSConfig::readIceTrace, &TSConfig::registerIceTrace);
	holder.addDetail("default/Database", &TSConfig::readDatabase, &TSConfig::registerDatabase);

	holder.addDetail("ssm_hsn_tree/LogFile", &TSConfig::readLogFile, &TSConfig::registerLogFile);
	holder.addDetail("ssm_hsn_tree/SessionManager", &TSConfig::readSessionManager, &TSConfig::registerSessionManager);
	holder.addDetail("ssm_hsn_tree/Bind", &TSConfig::readBind, &TSConfig::registerBind);
	holder.addDetail("ssm_hsn_tree/RTSPSession", &TSConfig::readRTSPSession, &TSConfig::registerRTSPSession);
	holder.addDetail("ssm_hsn_tree/DefaultParameter", &TSConfig::readDefaultParams, &TSConfig::registerDefaultParams);
	holder.addDetail("ssm_hsn_tree/Response", &TSConfig::readResponse, &TSConfig::registerResponse);
	holder.addDetail("ssm_hsn_tree/DoSProtection", &TSConfig::readDoSProtection, &TSConfig::registerDoSProtection);
	holder.addDetail("ssm_hsn_tree/DWH", &TSConfig::readDWH, &TSConfig::registerDWH);
	holder.addDetail("ssm_hsn_tree/IceProperties", &TSConfig::readIcePropertys, &TSConfig::registerIcePropertys);
	holder.addDetail("ssm_hsn_tree/PublishedLogs/Log", &TSConfig::readLogPublish, &TSConfig::registerIcePropertys);
	holder.addDetail("ssm_hsn_tree/VirtualServiceGroup", &TSConfig::readVirtualServiceGroup, &TSConfig::registerVirtualServiceGroup);
	holder.addDetail("ssm_hsn_tree/FixedSpeedSet",&TSConfig::readFixedSpeedSet,&TSConfig::regsiterFixedSpeedSet );
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

void TSConfig::readDWH(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_dwh.read(node, hPP);
}

void TSConfig::registerDWH(const std::string &full_path)
{
	_dwh.snmpRegister(full_path);
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

}

