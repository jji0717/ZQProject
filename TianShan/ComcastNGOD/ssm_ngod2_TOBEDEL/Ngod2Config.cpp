#include "./Ngod2Config.h"

ZQ::common::Config::Loader<NGOD2::Config> _ngodConfig("");

namespace NGOD2
{



void MessageFormat::structure( ZQ::common::Config::Holder<MessageFormat>& holder )
{
	//holder.addDetail("","RTSPNptUsage",&MessageFormat::rtspNptUsage , "0" ,ZQ::common::Config::optReadOnly);
}

void PlaylistControl::structure(ZQ::common::Config::Holder<PlaylistControl>& holder)
{
	holder.addDetail("","enableEOT",&PlaylistControl::enableEOT,"1",ZQ::common::Config::optReadOnly);
	holder.addDetail("","ignoreDestMac",&PlaylistControl::ignoreDestMac,"0",ZQ::common::Config::optReadOnly);
}

void ProtocolVersioning::structure(ZQ::common::Config::Holder<ProtocolVersioning>& holder)
{
	holder.addDetail("", "enable", &ProtocolVersioning::enableVersioning, "0", ZQ::common::Config::optReadOnly);
}

void SessionHistory::structure(ZQ::common::Config::Holder<SessionHistory>& holder)
{
	holder.addDetail("", "enable", &SessionHistory::enableHistory, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "enablePlayEvent", &SessionHistory::enablePlayEvent, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "enablePauseEvent", &SessionHistory::enablePauseEvent, "0", ZQ::common::Config::optReadOnly);
}

void LogFile::structure(ZQ::common::Config::Holder<LogFile>& holder)
{
	holder.addDetail("", "size", &LogFile::_size, "20000000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "level", &LogFile::_level, "7", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxCount", &LogFile::_maxCount, "10", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "bufferSize", &LogFile::_bufferSize, "8192", ZQ::common::Config::optReadOnly);
}

void IceStorm::structure(ZQ::common::Config::Holder<IceStorm>& holder)
{
	holder.addDetail("", "endpoint", &IceStorm::_endpoint, "", ZQ::common::Config::optReadOnly);
}

void Bind::structure(ZQ::common::Config::Holder<Bind>& holder)
{
	holder.addDetail("", "endpoint", &Bind::_endpoint, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "dispatchSize", &Bind::_dispatchSize, "5", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "dispatchMax", &Bind::_dispatchMax, "30", ZQ::common::Config::optReadOnly);
}

void RTSPSession::structure(ZQ::common::Config::Holder<RTSPSession>& holder)
{
	holder.addDetail("", "timeout", &RTSPSession::_timeout, "600", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "cacheSize", &RTSPSession::_cacheSize, "1000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "monitorThreads", &RTSPSession::_monitorThreads, "5", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "defaultServiceGroup", &RTSPSession::_defaultServiceGroup, "5", ZQ::common::Config::optReadOnly);

	// add by zjm to control numbers of send "session in progress"
	holder.addDetail("", "timeoutCount", &RTSPSession::_timeoutCount, "3", ZQ::common::Config::optReadOnly);
}

void Database::structure(ZQ::common::Config::Holder<Database>& holder)
{
	holder.addDetail("", "path", &Database::_path, "", ZQ::common::Config::optReadOnly);
}

void Announce::structure(ZQ::common::Config::Holder<Announce>& holder)
{
	holder.addDetail("", "useGlobalCSeq", &Announce::_useGlobalCSeq, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "includeTransition", &Announce::_includeTransition, "1", ZQ::common::Config::optReadOnly);
	//_useTianShanAnnounceCode
	//	int32 _useTianShanAnnounceCodeStateChanged;
	//	int32 _useTianShanAnnounceCodeScaleChanged;
	holder.addDetail("", "sendTianShanStateChangeAnnounce", &Announce::_useTianShanAnnounceCodeStateChanged, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "sendTianShanScaleChangeAnnounce", &Announce::_useTianShanAnnounceCodeScaleChanged, "0", ZQ::common::Config::optReadOnly);
}

void Response::structure(ZQ::common::Config::Holder<Response>& holder)
{
	holder.addDetail("", "setupFailureWithSessId", &Response::_setupFailureWithSessId, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "streamCtrlProt",&Response::_streamCtrlProt,"rtsp",ZQ::common::Config::optReadOnly);
}

void PublishLog::structure(ZQ::common::Config::Holder<PublishLog>& holder)
{
	holder.addDetail("", "path", &PublishLog::_path, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "syntax", &PublishLog::_syntax, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("","key",&PublishLog::_key,"");
	holder.addDetail("","type",&PublishLog::_type,"");
}

void PublishLogs::structure(ZQ::common::Config::Holder<PublishLogs>& holder)
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

void IceProperty::structure(IcePropertyHolder& holder)
{
	holder.addDetail("", "name", &IceProperty::_name, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "value", &IceProperty::_value, "", ZQ::common::Config::optReadOnly);
}

void IceProperties::structure(ZQ::common::Config::Holder<IceProperties>& holder)
{
	holder.addDetail("prop", &IceProperties::readIceProperty, &IceProperties::registerIceProperty);
}

void IceProperties::readIceProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	IcePropertyHolder IcePropHolder("name");
	IcePropHolder.read(node, hPP);
	_propDatas.push_back(IcePropHolder);
}

void IceProperties::registerIceProperty(const std::string &full_path)
{
    // disable the snmp register
    /*
	for (std::vector<IcePropertyHolder>::iterator it = _propDatas.begin(); it != _propDatas.end(); ++it)
	{
		it->snmpRegister(full_path);
	}
    */
}

void SOPProp::structure(ZQ::common::Config::Holder<SOPProp>& holder)
{
	holder.addDetail("", "fileName", &SOPProp::fileName, "", ZQ::common::Config::optReadOnly);
}

void ImportChannel::structure(ZQ::common::Config::Holder<ImportChannel>& holder)
{
	holder.addDetail("", "name", &ImportChannel::_name, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "bandwidth", &ImportChannel::_bandwidth, "100000000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxImport", &ImportChannel::_maxImport, "25", ZQ::common::Config::optReadOnly);
}

void PassThruStreaming::structure(ZQ::common::Config::Holder<PassThruStreaming>& holder)
{
	holder.addDetail("", "enabled", &PassThruStreaming::_enabled, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "testMode", &PassThruStreaming::_testMode, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "muteStat", &PassThruStreaming::_muteStat, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("ImportChannel", &PassThruStreaming::readImportChannel, &PassThruStreaming::registerImportChannel);
}

void PassThruStreaming::readImportChannel(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ImportChannelHolder importChannel("name");
	importChannel.read(node, hPP);
	_importChannelDatas[ importChannel._name ]=importChannel;
}

void PassThruStreaming::registerImportChannel(const std::string &full_path)
{
    // disable the snmp register
    /*
	for (std::map< std::string , ImportChannelHolder>::iterator it = _importChannelDatas.begin(); it != _importChannelDatas.end(); ++it)
	{
		it->second.snmpRegister(full_path);
	}
    */
}

void LAMTestModeSubItemVolume::structure(ZQ::common::Config::Holder<LAMTestModeSubItemVolume>& holder)
{
	using namespace ZQ::common::Config;
	holder.addDetail("","name",&LAMTestModeSubItemVolume::itemVolume,"",optReadOnly );
}

void LAMTestModeSubItemUrl::structure(ZQ::common::Config::Holder<LAMTestModeSubItemUrl>& holder)
{
	using namespace ZQ::common::Config;
	holder.addDetail("","name", &LAMTestModeSubItemUrl::itemUrl,"",optReadOnly);
}
void LAMSubTestSet::readUrls(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	using namespace ZQ::common::Config;
	Holder<LAMTestModeSubItemUrl> subUrl;
	subUrl.read(node,hPP);
	urls.push_back( subUrl.itemUrl );
}

void LAMSubTestSet::readVolumeList(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	using namespace ZQ::common::Config;
	Holder<LAMTestModeSubItemVolume> subItemVolume;
	subItemVolume.read( node , hPP );
	volumeList.push_back( subItemVolume.itemVolume );
}

void LAMSubTestSet::structure( ZQ::common::Config::Holder<LAMSubTestSet>& holder )
{
	using namespace ZQ::common::Config;
	holder.addDetail("","name",&LAMSubTestSet::contentName,"", optReadOnly);
	holder.addDetail("","bandwidth",&LAMSubTestSet::bandwidth,"3750000",optReadOnly);
	holder.addDetail("","cuein",&LAMSubTestSet::cueIn,"0",optReadOnly);
	holder.addDetail("","cueout",&LAMSubTestSet::cueOut,"0",optReadOnly);
	holder.addDetail("remoteurl/url",&LAMSubTestSet::readUrls,&LAMSubTestSet::registerNothing);
	holder.addDetail("volumeList/volume",&LAMSubTestSet::readVolumeList,&LAMSubTestSet::registerNothing);
}

void LAMTestMode::readSubTestSet(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	using namespace ZQ::common::Config;
	Holder<LAMSubTestSet> subTestHolder;
	//PluginAttr
	subTestHolder.read( node , hPP );
	subTests.push_back(subTestHolder);
}
void LAMTestMode::structure(ZQ::common::Config::Holder<LAMTestMode>& holder)
{
	holder.addDetail("", "enabled", &LAMTestMode::_enabled, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("item",&LAMTestMode::readSubTestSet,&LAMTestMode::registerNothing);
}

void ContentVolume::structure( ContentVolumeHolder& holder )
{
	holder.addDetail("","name",&ContentVolume::name,"",ZQ::common::Config::optReadOnly);
	holder.addDetail("","cache",&ContentVolume::cache,"0",ZQ::common::Config::optReadOnly);
	holder.addDetail("","cacheLevel",&ContentVolume::cacheLevel,"50",ZQ::common::Config::optReadOnly);
	holder.addDetail("","supportNasStreaming",&ContentVolume::supportNasStreaming,"0",ZQ::common::Config::optReadOnly);
	holder.addDetail("","endpoint",&ContentVolume::endpoint,"",ZQ::common::Config::optReadOnly);
}

void LAMServer::structure(ZQ::common::Config::Holder<LAMServer>& holder)
{
	holder.addDetail("", "volumeName", &LAMServer::_volumeName, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "endpoint", &LAMServer::_endpoint, "", ZQ::common::Config::optReadOnly);
	//holder.addDetail("", "referenceOnly", &LAMServer::_iReferenceOnly, "0", ZQ::common::Config::optReadOnly);
// 	holder.addDetail("", "cache",&LAMServer::_lamType,"1",ZQ::common::Config::optReadOnly);
// 	holder.addDetail("", "cacheLevel" ,&LAMServer::_iLibPriority,"50",ZQ::common::Config::optReadOnly);
// 	holder.addDetail("","supportNasStreaming",&LAMServer::_supportNasStream,"0",ZQ::common::Config::optReadOnly);
	holder.addDetail("Volume",&LAMServer::readContentVolume,&LAMServer::registerContentVolume);
}


void LAM::structure(ZQ::common::Config::Holder<LAM>& holder)
{	
	holder.addDetail("", "enableWarmup", &LAM::_enableWarmup, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("TestMode", &LAM::readTestMode, &LAM::registerTestMode);
	holder.addDetail("LAMServer", &LAM::readLAMServer, &LAM::registerLAMServer);	
}
void LAMServer::registerContentVolume( const std::string &full_path )
{//do nothing
}

void LAMServer::readContentVolume( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
{
	ContentVolume::ContentVolumeHolder volume;
	volume.read(node,hPP);
	contentVolumes[volume.name] = volume;
}

void LAM::readTestMode(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_lamTestMode.read(node, hPP);
}

void LAM::registerTestMode(const std::string &full_path)
{
	_lamTestMode.snmpRegister(full_path);
}

void LAM::readLAMServer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	LAMServerHolder lamServer("volumeName");
	_lamServer.read(node, hPP);
	//_lamServerDatas.push_back(lamServer);
}

void LAM::registerLAMServer(const std::string &full_path)
{	
// 	for (std::vector<LAMServerHolder>::iterator it = _lamServerDatas.begin(); it != _lamServerDatas.end(); ++it)
// 	{
// 		it->snmpRegister(full_path);
// 	}
}

void Library::structure(ZQ::common::Config::Holder<Library>& holder)
{
	holder.addDetail("", "staticed", &Library::_staticed, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "urlTemplate", &Library::_urlTemplate, "", ZQ::common::Config::optReadOnly);
}

void Config::structure(ZQ::common::Config::Holder<Config>& holder)
{
	holder.addDetail("ssm_NGOD2/LogFile", &Config::readLogFile, &Config::registerLogFile);
	holder.addDetail("ssm_NGOD2/EventChannel", &Config::readIceStorm, &Config::registerIceStorm);
	holder.addDetail("ssm_NGOD2/Bind", &Config::readBind, &Config::registerBind);
	holder.addDetail("ssm_NGOD2/RTSPSession", &Config::readRTSPSession, &Config::registerRTSPSession);
	holder.addDetail("ssm_NGOD2/Database", &Config::readDatabase, &Config::registerDatabase);
	holder.addDetail("ssm_NGOD2/Announce", &Config::readAnnounce, &Config::registerAnnounce);
	holder.addDetail("ssm_NGOD2/Response", &Config::readResponse, &Config::registerResponse);
	holder.addDetail("ssm_NGOD2/PublishedLogs", &Config::readPublishLogs, &Config::registerPublishLogs);

	holder.addDetail("ssm_NGOD2/SOPProp", &Config::readSOPProp, &Config::registerSOPProp);
	
	holder.addDetail("ssm_NGOD2/IceProperties", &Config::readIceProperties, &Config::registerIceProperties);
	holder.addDetail("ssm_NGOD2/PassThruStreaming", &Config::readPassThruStreaming, &Config::registerPassThruStreaming);
	holder.addDetail("ssm_NGOD2/LAM", &Config::readLAM, &Config::registerLAM);
	holder.addDetail("ssm_NGOD2/Library", &Config::readLibrary, &Config::registerLibrary);
	holder.addDetail("ssm_NGOD2/playlistControl", &Config::readPlaylistControl, &Config::registerPlaylistControl);
	holder.addDetail("ssm_NGOD2/protocolVersioning", &Config::readProtocolVersioning, &Config::registerProtocolVersioning);
	holder.addDetail("ssm_NGOD2/MessageFormat",&Config::readMessageFormat,&Config::registerAnnounce);

	holder.addDetail("ssm_NGOD2/sessionHistory", &Config::readSessionHistory, &Config::registerSessionHistory);
}

void Config::readPlaylistControl(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_PlaylistControl.read( node, hPP );
}
void Config::registerPlaylistControl(const std::string &full_path)
{
	_PlaylistControl.snmpRegister(full_path);
}

void Config::readProtocolVersioning(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_protocolVersioning.read(node, hPP);
}

void Config::registerProtocolVersioning(const std::string &full_path)
{
	_protocolVersioning.snmpRegister(full_path);
}

void Config::readSessionHistory(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_sessionHistory.read(node, hPP);
}

void Config::registerSessionHistory(const std::string &full_path)
{
	_sessionHistory.snmpRegister(full_path);
}

void Config::readLibrary(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_library.read(node, hPP);
}

void Config::registerLibrary(const std::string &full_path)
{
	_library.snmpRegister(full_path);
}

void Config::readLAM(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_lam.read(node, hPP);
}

void Config::registerLAM(const std::string &full_path)
{
	_lam.snmpRegister(full_path);
}

void Config::readPassThruStreaming(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_passThruStreaming.read(node, hPP);
}

void Config::registerPassThruStreaming(const std::string &full_path)
{
	_passThruStreaming.snmpRegister(full_path);
}

void Config::readIceProperties(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_iceProps.read(node, hPP);
}

void Config::registerIceProperties(const std::string &full_path)
{
	_iceProps.snmpRegister(full_path);
}

void Config::readPublishLogs(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_publishLogs.read(node, hPP);
}

void Config::registerPublishLogs(const std::string &full_path)
{
	_publishLogs.snmpRegister(full_path);
}

void Config::readResponse(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_response.read(node, hPP);
}

void Config::registerResponse(const std::string &full_path)
{
	_response.snmpRegister(full_path);
}

void Config::readAnnounce(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_announce.read(node, hPP);
}

void Config::registerAnnounce(const std::string &full_path)
{
	_announce.snmpRegister(full_path);
}

void Config::readDatabase(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_database.read(node, hPP);
}

void Config::registerDatabase(const std::string &full_path)
{
	_database.snmpRegister(full_path);
}

void Config::readRTSPSession(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_rtspSession.read(node, hPP);
}

void Config::registerRTSPSession(const std::string &full_path)
{
	_rtspSession.snmpRegister(full_path);
}

void Config::readBind(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_bind.read(node, hPP);
}

void Config::registerBind(const std::string &full_path)
{
	_bind.snmpRegister(full_path);
}

void Config::readIceStorm(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_iceStorm.read(node, hPP);
}

void Config::registerIceStorm(const std::string &full_path)
{
	_iceStorm.snmpRegister(full_path);
}

void Config::readLogFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_pluginLog.read(node, hPP);
}

void Config::registerLogFile(const std::string &full_path)
{
	_pluginLog.snmpRegister(full_path);
}

void Config::readMessageFormat( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
{
	_MessageFmt.read(node,hPP);
}

void Config::readSOPProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	_sopProp.read(node, hPP);
}

void Config::registerSOPProp(const std::string &full_path)
{
	_sopProp.snmpRegister(full_path);
}

void Config::regsiterNone( const std::string& )
{

}

}

