// File Name : OpenVBOConfig.cpp

#include "OpenVBOConfig.h"

namespace EventISVODI5
{

void EventChannel::structure(EventChannelHolder& holder)
{
	holder.addDetail("", "endpoint", &EventChannel::_endpoint, "", ZQ::common::Config::optReadOnly);
}

void IceTrace::structure(IceTraceHolder& holder)
{
	holder.addDetail("", "enabled", &IceTrace::_enabled, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "level", &IceTrace::_level, "6", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "size", &IceTrace::_size, "10000000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxCount", &IceTrace::_maxCount, "5", ZQ::common::Config::optReadOnly);
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
	IcePropertyHolder propHolder("name");
	propHolder.read(node, hPP);
	_propDatas.push_back(propHolder);
}

void IcePropertys::registerIcePropertys(const std::string &full_path)
{
	std::vector<IcePropertyHolder>::iterator it = _propDatas.begin();
	for (; it != _propDatas.end(); it ++)
	{
		it->snmpRegister(full_path);
	}
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

void PublishLog::structure(PublishLogHolder& holder)
{
	holder.addDetail("", "path", &PublishLog::_path, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "syntax", &PublishLog::_syntax, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "key", &PublishLog::_key,"");
}

void PublishLogs::structure(PublishLogsHolder& holder)
{
	holder.addDetail("Log", &PublishLogs::readPublishLog, &PublishLogs::registerPublishLog);
}

void PublishLogs::readPublishLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	PublishLogHolder logHolder("path");
	logHolder.read(node, hPP);
	_logDatas.push_back(logHolder);
}

void PublishLogs::registerPublishLog(const std::string &full_path)
{
	// disable the snmp register
	/*
	for (std::vector<PublishLogHolder>::iterator it = _logDatas.begin(); it != _logDatas.end(); ++it)
	{
	it->snmpRegister(full_path);
	}
	*/
}

void Bind::structure(BindHolder& holder)
{
	holder.addDetail("", "serviceName", &Bind::_serviceName,"ListenEventAdapter",ZQ::common::Config::optReadOnly);
	holder.addDetail("", "endpoint", &Bind::_endpoint, "", ZQ::common::Config::optReadOnly);
}

void RTSPSession::structure(RTSPSessionHolder& holder)
{
	holder.addDetail("", "cacheSize", &RTSPSession::_cacheSize, "1000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "monitorThreads", &RTSPSession::_monitorThreads, "5", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "timeout", &RTSPSession::_timeout, "600", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "timeoutCount", &RTSPSession::_timeoutCount, "3", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxSessionDestroyInterval",&RTSPSession::_maxDestroyInterval,"3600",ZQ::common::Config::optReadOnly);
}

void PlaylistControl::structure(PlaylistControlHolder& holder)
{
	holder.addDetail("", "enableEOT", &PlaylistControl::_enableEOT, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "ignoreDestMac", &PlaylistControl::_ignoreDestMac, "0", ZQ::common::Config::optReadOnly);
}

void RequestAdjust::structure(RequestAdjustHolder& holder)
{
	holder.addDetail("", "defaultPID",   &RequestAdjust::_defaultPID,   "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "defaultSRMID", &RequestAdjust::_defaultSRMID, "", ZQ::common::Config::optReadOnly);
}

void Response::structure(ResponseHolder& holder)
{
	holder.addDetail("", "setupFailureWithSessId", &Response::_setupFailureWithSessId, "1", ZQ::common::Config::optReadOnly);
}

void Announce::structure(AnnounceHolder &holder)
{
	holder.addDetail("", "useGlobalCSeq", &Announce::_useGlobalCSeq, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "SRMEnabled", &Announce::_SRMEnabled, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "STBEnabled", &Announce::_STBEnabled, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "sendTianShanStateChangeAnnounce", &Announce::_sendTianShanStateChangeAnnounce, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "sendTianShanScaleChangeAnnounce", &Announce::_sendTianShanScaleChangeAnnounce, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "sendPauseTimeout", &Announce::_sendPauseTimeout, "0", ZQ::common::Config::optReadOnly);
}

void ImportChannel::structure(ImportChannelHolder& holder)
{
	holder.addDetail("", "name", &ImportChannel::_name, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "bandwidth", &ImportChannel::_bandwidth, "100000000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxImport", &ImportChannel::_maxImport, "25", ZQ::common::Config::optReadOnly);
}

void PassThruStreaming::structure(PassThruStreamingHolder& holder)
{
	holder.addDetail("ImportChannel", &PassThruStreaming::readImportChannel, &PassThruStreaming::registerImportChannel);
}

void PassThruStreaming::readImportChannel(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ImportChannelHolder importChannel("name");
	importChannel.read(node, hPP);
	_importChannelDatas.push_back(importChannel);
}

void PassThruStreaming::registerImportChannel(const std::string &full_path)
{
}

void SourceStreamers::structure(SourceStreamersHolder& holder)
{
	holder.addDetail("", "fileName", &SourceStreamers::_fileName, "", ZQ::common::Config::optReadOnly);
}

void LAMTestModeSubItemVolume::structure(LAMTestModeSubItemVolumeHolder& holder)
{
	holder.addDetail("", "name", &LAMTestModeSubItemVolume::_itemVolume, "", ZQ::common::Config::optReadOnly);
}

void LAMTestModeSubItemUrl::structure(LAMTestModeSubItemUrlHolder& holder)
{
	holder.addDetail("", "name", &LAMTestModeSubItemUrl::_itemUrl, "", ZQ::common::Config::optReadOnly);
}

void LAMSubTestSet::readUrls(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	LAMTestModeSubItemUrlHolder subUrl;
	subUrl.read(node ,hPP);
	_urls.push_back(subUrl._itemUrl);
}

void LAMSubTestSet::readVolumeList(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	LAMTestModeSubItemVolumeHolder subItemVolume;
	subItemVolume.read(node, hPP);
	_volumeList.push_back(subItemVolume._itemVolume);
}

void LAMSubTestSet::structure(LAMSubTestSetHolder& holder)
{
	holder.addDetail("", "name", &LAMSubTestSet::_contentName, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "bandwidth", &LAMSubTestSet::_bandwidth, "3750000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "cuein", &LAMSubTestSet::_cueIn, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "cueout", &LAMSubTestSet::_cueOut, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("RemoteURL/URL", &LAMSubTestSet::readUrls, &LAMSubTestSet::registerNothing);
	holder.addDetail("Replicas/Volume", &LAMSubTestSet::readVolumeList, &LAMSubTestSet::registerNothing);
}

void LAMTestMode::readSubTestSet(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	LAMSubTestSetHolder subTestHolder;
	subTestHolder.read( node, hPP);
	_subTests.push_back(subTestHolder);
}
void LAMTestMode::structure(LAMTestModeHolder& holder)
{
	holder.addDetail("", "enabled", &LAMTestMode::_enabled, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("Item", &LAMTestMode::readSubTestSet, &LAMTestMode::registerNothing);
}

void ContentVolume::structure(ContentVolumeHolder& holder)
{
	holder.addDetail("", "name", &ContentVolume::_name, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "cacheLevel", &ContentVolume::_cacheLevel, "50", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "supportNasStreaming", &ContentVolume::_supportNasStreaming, "0", ZQ::common::Config::optReadOnly);
}

void LAMServer::structure(LAMServerHolder& holder)
{
	holder.addDetail("", "name", &LAMServer::_name, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "endpoint", &LAMServer::_endpoint, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("Volume", &LAMServer::readContentVolume, &LAMServer::registerContentVolume);
}

void LAMServer::registerContentVolume( const std::string &full_path )
{//do nothing
}

void LAMServer::readContentVolume( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
{
	ContentVolumeHolder volume;
	volume.read(node,hPP);
	_contentVolumes.push_back(volume);
}

void AssetManagement::structure(AssetManagementHolder& holder)
{	
	holder.addDetail("", "type", &AssetManagement::_type, "LAM", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "enableWarmup", &AssetManagement::_enableWarmup, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("LAMServer", &AssetManagement::readLAMServer, &AssetManagement::registerLAMServer);
	holder.addDetail("TestMode", &AssetManagement::readTestMode, &AssetManagement::registerTestMode);
}

void AssetManagement::readTestMode(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_lamTestMode.read(node, hPP);
}

void AssetManagement::registerTestMode(const std::string &full_path)
{
	_lamTestMode.snmpRegister(full_path);
}

void AssetManagement::readLAMServer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	AssetManagementHolder lamServer("volumeName");
	_lamServer.read(node, hPP);
	//_lamServerDatas.push_back(lamServer);
}

void AssetManagement::registerLAMServer(const std::string &full_path)
{	
	// 	for (std::vector<LAMServerHolder>::iterator it = _lamServerDatas.begin(); it != _lamServerDatas.end(); ++it)
	// 	{
	// 		it->snmpRegister(full_path);
	// 	}
}

void OpenVBOConfig::structure(EventISVODI5::OpenVBOConfig::ISVodConfigHolder &holder)
{
	holder.addDetail("default/EventChannel", &OpenVBOConfig::readIceStrom, &OpenVBOConfig::registerIceStrom);
	holder.addDetail("default/IceTrace", &OpenVBOConfig::readIceTrace, &OpenVBOConfig::registerIceTrace);
	holder.addDetail("default/IceProperties", &OpenVBOConfig::readIcePropertys, &OpenVBOConfig::registerIcePropertys);
	holder.addDetail("default/Database", &OpenVBOConfig::readDatabase, &OpenVBOConfig::registerDatabase);

	holder.addDetail("ssm_OpenVBO/LogFile", &OpenVBOConfig::readLogFile, &OpenVBOConfig::registerLogFile);
	holder.addDetail("ssm_OpenVBO/PublishedLogs", &OpenVBOConfig::readPublishLogs, &OpenVBOConfig::registerPublishLogs);
	holder.addDetail("ssm_OpenVBO/Bind", &OpenVBOConfig::readBind, &OpenVBOConfig::registerBind);
	holder.addDetail("ssm_OpenVBO/RTSPSession", &OpenVBOConfig::readRTSPSession, &OpenVBOConfig::registerRTSPSession);

	holder.addDetail("ssm_OpenVBO/PlaylistControl", &OpenVBOConfig::readPlaylistControl, &OpenVBOConfig::registerPlaylistControl);
	holder.addDetail("ssm_OpenVBO/RequestAdjust", &OpenVBOConfig::readRequestAdjust, &OpenVBOConfig::registerRequestAdjust);
	holder.addDetail("ssm_OpenVBO/Response", &OpenVBOConfig::readResponse, &OpenVBOConfig::registerResponse);
	holder.addDetail("ssm_OpenVBO/Announce", &OpenVBOConfig::readAnnounce, &OpenVBOConfig::registerAnnounce);
	holder.addDetail("ssm_OpenVBO/PassThruStreaming", &OpenVBOConfig::readPassThruStreaming, &OpenVBOConfig::registerPassThruStreaming);
	holder.addDetail("ssm_OpenVBO/SourceStreamers", &OpenVBOConfig::readSourceStreamers, &OpenVBOConfig::registerSourceStreamers);
	holder.addDetail("ssm_OpenVBO/AssetManagement", &OpenVBOConfig::readLAM, &OpenVBOConfig::registerLAM);
}


void OpenVBOConfig::readIceStrom(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_iceStorm.read(node, hPP);
}

void OpenVBOConfig::registerIceStrom(const std::string &full_path)
{
	_iceStorm.snmpRegister(full_path);
}

void OpenVBOConfig::readIceTrace(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_iceTrace.read(node, hPP);
}

void OpenVBOConfig::registerIceTrace(const std::string &full_path)
{
	_iceTrace.snmpRegister(full_path);
}

void OpenVBOConfig::readIcePropertys(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_iceProps.read(node, hPP);
}

void OpenVBOConfig::registerIcePropertys(const std::string &full_path)
{
	_iceProps.snmpRegister(full_path);
}

void OpenVBOConfig::readDatabase(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_database.read(node, hPP);
}

void OpenVBOConfig::registerDatabase(const std::string &full_path)
{
	_database.snmpRegister(full_path);
}

void OpenVBOConfig::readLogFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_pluginLog.read(node, hPP);
}

void OpenVBOConfig::registerLogFile(const std::string &full_path)
{
	_pluginLog.snmpRegister(full_path);
}

void OpenVBOConfig::readPublishLogs(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_publishLogs.read(node, hPP);
}

void OpenVBOConfig::registerPublishLogs(const std::string &full_path)
{
	_publishLogs.snmpRegister(full_path);
}

void OpenVBOConfig::readBind(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_bind.read(node, hPP);
}

void OpenVBOConfig::registerBind(const std::string &full_path)
{
	_bind.snmpRegister(full_path);
}

void OpenVBOConfig::readRTSPSession(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_rtspSession.read(node, hPP);
}

void OpenVBOConfig::registerRTSPSession(const std::string &full_path)
{
	_rtspSession.snmpRegister(full_path);
}

void OpenVBOConfig::readPlaylistControl(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_PlaylistControl.read( node, hPP );
}

void OpenVBOConfig::registerPlaylistControl(const std::string &full_path)
{
	_PlaylistControl.snmpRegister(full_path);
}

void OpenVBOConfig::readRequestAdjust(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_requestAdjust.read(node, hPP);
}

void OpenVBOConfig::registerRequestAdjust(const std::string &full_path)
{
	_requestAdjust.snmpRegister(full_path);
}

void OpenVBOConfig::readResponse(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_response.read(node, hPP);
}

void OpenVBOConfig::registerResponse(const std::string &full_path)
{
	_response.snmpRegister(full_path);
}

void OpenVBOConfig::readAnnounce(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_announce.read(node, hPP);
}

void OpenVBOConfig::registerAnnounce(const std::string &full_path)
{
	_announce.snmpRegister(full_path);
}

void OpenVBOConfig::readPassThruStreaming(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_passThruStreaming.read(node, hPP);
}

void OpenVBOConfig::registerPassThruStreaming(const std::string &full_path)
{
	_passThruStreaming.snmpRegister(full_path);
}

void OpenVBOConfig::readSourceStreamers(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_sourceStreamers.read(node, hPP);
}

void OpenVBOConfig::registerSourceStreamers(const std::string &full_path)
{
	_sourceStreamers.snmpRegister(full_path);
}

void OpenVBOConfig::readLAM(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
{
	_lam.read(node, hPP);
}

void OpenVBOConfig::registerLAM(const std::string &full_path)
{
	_lam.snmpRegister(full_path);
}

} // end EventISVODI5
