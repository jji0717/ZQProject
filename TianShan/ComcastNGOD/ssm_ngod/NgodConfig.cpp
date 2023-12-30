
#include "./NgodConfig.h"

ZQ::common::Config::Loader<NGOD::Config> ngodConfig("");

namespace NGOD
{

void AssetStack::structure( ZQ::common::Config::Holder<AssetStack>& holder )
{
	using namespace ZQ::common::Config;
	holder.addDetail("","enable" , &AssetStack::enable , "1" , optReadOnly );	
	holder.addDetail("","adjustWeight",&AssetStack::adjustWeight,"9900",optReadOnly);
	holder.addDetail("","startMode",&AssetStack::startMode,"0",optReadOnly);
}

void SubD5Listener::structure(ZQ::common::Config::Holder<SubD5Listener>& holder )
{
	using namespace ZQ::common::Config;
	holder.addDetail("","listenerIP",&SubD5Listener::d5serverIp ,"" ,optReadOnly);
	holder.addDetail("","listenerPort",&SubD5Listener::d5serverPort,"0",optReadOnly);

}
void D5MessageConf::readSubListenr(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	SubListenerHolder l;
	l.read(node,hPP);
	if( !l.d5serverIp.empty() )
	{
		subListener.push_back(l);
	}
}
void D5MessageConf::structure( ZQ::common::Config::Holder<D5MessageConf>& holder )
{
	using namespace ZQ::common::Config;
	holder.addDetail("","enable",&D5MessageConf::enable ,"0" ,optReadOnly);
	holder.addDetail("","listenerIP",&D5MessageConf::d5serverIp ,"" ,optReadOnly);
	holder.addDetail("","listenerPort",&D5MessageConf::d5serverPort,"0",optReadOnly);
	holder.addDetail("","holdTimer",&D5MessageConf::holdTimer,"30000",optReadOnly);
	holder.addDetail("","keepAliveInterval",&D5MessageConf::keepAliveInterval,"100000",optReadOnly);
	holder.addDetail("","streamZone",&D5MessageConf::streamZone,"ZQ",optReadOnly);
	holder.addDetail("","hopServer",&D5MessageConf::nextHopServer,"",optReadOnly);
	holder.addDetail("","updateInterval",&D5MessageConf::msgUpdateInterval,"30000",optReadOnly);
	holder.addDetail("","diffPercent",&D5MessageConf::diffPercent,"5",optReadOnly);
	holder.addDetail("SubSlistener",&D5MessageConf::readSubListenr,&D5MessageConf::registerSubListenr);
}


void PlaylistControl::structure(ZQ::common::Config::Holder<PlaylistControl>& holder)
{
	holder.addDetail("","enableEOT",&PlaylistControl::enableEOT,"1",ZQ::common::Config::optReadOnly);
	holder.addDetail("","ignoreDestMac",&PlaylistControl::ignoreDestMac,"0",ZQ::common::Config::optReadOnly);
	holder.addDetail("","ignoreAbsentItems",&PlaylistControl::ignoreAbsentItems,"1",ZQ::common::Config::optReadOnly);
	holder.addDetail("","minClipDuration",&PlaylistControl::minClipDuration,"5000",ZQ::common::Config::optReadOnly);
	holder.addDetail("","nptByPrimary",&PlaylistControl::nptByPrimary,"0",ZQ::common::Config::optReadOnly);
}

void ProtocolVersioning::structure(ZQ::common::Config::Holder<ProtocolVersioning>& holder)
{
	holder.addDetail("", "enable",   &ProtocolVersioning::enableVersioning, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "customer", &ProtocolVersioning::customer, "", ZQ::common::Config::optReadOnly);
}

void MessageFormat::structure( ZQ::common::Config::Holder<MessageFormat>& holder )
{
	//holder.addDetail("","RTSPNptUsage",&MessageFormat::rtspNptUsage , "0" ,ZQ::common::Config::optReadOnly);
}

void SessionHistory::structure(ZQ::common::Config::Holder<SessionHistory>& holder)
{
	holder.addDetail("", "enable", &SessionHistory::enableHistory, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "enablePlayEvent", &SessionHistory::enablePlayEvent, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "enablePauseEvent", &SessionHistory::enablePauseEvent, "0", ZQ::common::Config::optReadOnly);
}

void LogFile::structure(ZQ::common::Config::Holder<LogFile>& holder)
{
	holder.addDetail("", "size", &LogFile::size, "20000000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "level", &LogFile::level, "7", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxCount", &LogFile::maxCount, "10", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "bufferSize", &LogFile::bufferSize, "8192", ZQ::common::Config::optReadOnly);
}

void EventLogFile::structure(ZQ::common::Config::Holder<EventLogFile>& holder)
{
	holder.addDetail("", "size", &EventLogFile::size, "20000000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "level", &EventLogFile::level, "7", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxCount", &EventLogFile::maxCount, "10", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "bufferSize", &EventLogFile::bufferSize, "8192", ZQ::common::Config::optReadOnly);
}

void EventChannelConf::structure(ZQ::common::Config::Holder<EventChannelConf>& holder)
{
	holder.addDetail("", "endpoint", &EventChannelConf::endpoint, "", ZQ::common::Config::optReadOnly);
}

void Bind::structure(ZQ::common::Config::Holder<Bind>& holder)
{
	holder.addDetail("", "endpoint", &Bind::endpoint, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "dispatchSize", &Bind::dispatchSize, "5", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "dispatchMax", &Bind::dispatchMax, "30", ZQ::common::Config::optReadOnly);
}

void RTSPSession::structure(ZQ::common::Config::Holder<RTSPSession>& holder)
{
	holder.addDetail("", "timeout", &RTSPSession::timeout, "600", ZQ::common::Config::optReadOnly);	
	holder.addDetail("", "cacheSize", &RTSPSession::cacheSize, "5000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "defaultServiceGroup", &RTSPSession::defaultServiceGroup, "5", ZQ::common::Config::optReadOnly);	
	holder.addDetail("", "reqPriorityIfStreamerNA", &RTSPSession::requestPriorityIfStreamerNA, "200", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxSessionDestroyInterval", &RTSPSession::maxSessionDestroyTimeInterval, "3600", ZQ::common::Config::optReadOnly);

	holder.addDetail("", "monitorThreads", &RTSPSession::monitorThreads, "5", ZQ::common::Config::optNone);

	// add by zjm to control numbers of send "session in progress"
	holder.addDetail("", "timeoutCount",      &RTSPSession::timeoutCount, "3", ZQ::common::Config::optNone); // ZQ::common::Config::optNone:optReadOnly);
	holder.addDetail("", "timeoutParamCache", &RTSPSession::timeoutParamCache, "10000", ZQ::common::Config::optNone); 
}

void Database::structure(ZQ::common::Config::Holder<Database>& holder)
{	
	holder.addDetail("", "path", &Database::path, "", ZQ::common::Config::optReadOnly);
    holder.addDetail("", "runtimePath", &Database::runtimePath, "", ZQ::common::Config::optNone);
	holder.addDetail("", "fatalRecover", &Database::fatalRecover, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "checkpointPeriod", &Database::checkpointPeriod, "120", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "saveSizeTrigger", &Database::saveSizeTrigger, "10", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "savePeriod", &Database::savePeriod, "60000", ZQ::common::Config::optNone); // ZQ::common::Config::optReadOnly);
}

void Announce::structure(ZQ::common::Config::Holder<Announce>& holder)
{
	holder.addDetail("", "useGlobalCSeq", &Announce::useGlobalCSeq, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "includeTransition", &Announce::includeTransition, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "notifyTrickRestriction", &Announce::notifyTrickRestriction, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "notifyItemSkip", &Announce::notifyItemSkip, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "includeTeardownRange", &Announce::includeTeardownRange, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "sendTianShanStateChangeAnnounce", &Announce::useTianShanAnnounceCodeStateChanged, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "sendTianShanScaleChangeAnnounce", &Announce::useTianShanAnnounceCodeScaleChanged, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "resubscribeAtIdle", &Announce::resubscribeAtIdle, "3600000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "skipEventOfRequested",&Announce::skipEventOfRequested,"0",ZQ::common::Config::optReadOnly);
	holder.addDetail("", "eventTTL", &Announce::eventTTL, "1800000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "announceThread", &Announce::announceSenderThreadCount, "50", ZQ::common::Config::optReadOnly);	
}

void Response::structure(ZQ::common::Config::Holder<Response>& holder)
{
	holder.addDetail("", "setupFailureWithSessId", &Response::setupFailureWithSessId, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "streamCtrlProt",&Response::streamCtrlProt,"rtsp",ZQ::common::Config::optReadOnly);
}

void PublishLog::structure(ZQ::common::Config::Holder<PublishLog>& holder)
{
	holder.addDetail("", "path", &PublishLog::path, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "syntax", &PublishLog::syntax, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("","key",&PublishLog::key,"");
	holder.addDetail("","type",&PublishLog::type,"");
}

void PublishLogs::structure(ZQ::common::Config::Holder<PublishLogs>& holder)
{
	holder.addDetail("", "enabled", &PublishLogs::enabled, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("Log", &PublishLogs::readPublishLog, &PublishLogs::registerPublishLog);
}

void PublishLogs::readPublishLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	PublishLogHolder logHolder("path");
	logHolder.read(node, hPP);
	logDatas.push_back(logHolder);
}

void PublishLogs::registerPublishLog(const std::string &full_path)
{
    
}

void IceProperty::structure(IcePropertyHolder& holder)
{
	holder.addDetail("", "name", &IceProperty::name, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "value", &IceProperty::value, "", ZQ::common::Config::optReadOnly);
}

void IceProperties::structure(ZQ::common::Config::Holder<IceProperties>& holder)
{
	holder.addDetail("prop", &IceProperties::readIceProperty, &IceProperties::registerIceProperty);
}

void IceProperties::readIceProperty(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	IcePropertyHolder IcePropHolder("name");
	IcePropHolder.read(node, hPP);
	propDatas.push_back(IcePropHolder);
}

void IceProperties::registerIceProperty(const std::string &full_path)
{
   
}

void SOPProp::structure(ZQ::common::Config::Holder<SOPProp>& holder)
{
	holder.addDetail("", "fileName", &SOPProp::fileName, "", ZQ::common::Config::optReadOnly);
}

void ImportChannel::structure(ZQ::common::Config::Holder<ImportChannel>& holder)
{
	holder.addDetail("", "name", &ImportChannel::name, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "bandwidth", &ImportChannel::bandwidth, "100000000", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "maxImport", &ImportChannel::maxImport, "25", ZQ::common::Config::optReadOnly);
		
}

void PassThruStreaming::structure(ZQ::common::Config::Holder<PassThruStreaming>& holder)
{	
	holder.addDetail("", "enable", &PassThruStreaming::enable,     "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "testMode", &PassThruStreaming::testMode, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "muteStat", &PassThruStreaming::muteStat, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "excludeC2Down", &PassThruStreaming::excludeC2Down, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("ImportChannel", &PassThruStreaming::readImportChannel, &PassThruStreaming::registerImportChannel);
}

void PassThruStreaming::readImportChannel(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ImportChannelHolder importChannel("name");
	importChannel.read(node, hPP);
	importChannelDatas[ importChannel.name ]=importChannel;
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
void LAMSubTestSet::readAttributes(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	using namespace ZQ::common::Config;
	Holder<LAMSubTestContentAttribute> attr;
	attr.read( node, hPP );
	if( attr.value.empty() || attr.key.empty() )
		return;
	attrs[attr.key] =  attr.value ;
}
void LAMSubTestSet::structure( ZQ::common::Config::Holder<LAMSubTestSet>& holder )
{
	using namespace ZQ::common::Config;
	holder.addDetail("","name",&LAMSubTestSet::contentName,"", optReadOnly );
	holder.addDetail("","bandwidth",&LAMSubTestSet::bandwidth,"3750000",optReadOnly  );
	holder.addDetail("","cuein",&LAMSubTestSet::cueIn,"0",optReadOnly );
	holder.addDetail("","cueout",&LAMSubTestSet::cueOut,"0",optReadOnly );
	holder.addDetail("","pid",&LAMSubTestSet::pid,"",optReadOnly );
	holder.addDetail("remoteurl/url",&LAMSubTestSet::readUrls,&LAMSubTestSet::registerNothing );
	holder.addDetail("volumeList/volume",&LAMSubTestSet::readVolumeList,&LAMSubTestSet::registerNothing );
	holder.addDetail("attrs/attr",&LAMSubTestSet::readAttributes,&LAMSubTestSet::registerNothing );
}

void LAMTestMode::readSubTestSet(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	using namespace ZQ::common::Config;
	Holder<LAMSubTestSet> subTestHolder;
	//PluginAttr
	subTestHolder.read( node , hPP );
	if( !subTestHolder.pid.empty())
		subTestHolder.attrs["TEST_PID"] = subTestHolder.pid;
	subTests.push_back(subTestHolder);
}
void LAMTestMode::structure(ZQ::common::Config::Holder<LAMTestMode>& holder)
{
	holder.addDetail("", "enabled", &LAMTestMode::enabled, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("item",&LAMTestMode::readSubTestSet,&LAMTestMode::registerNothing);
}

void ContentLibMode::structure(ZQ::common::Config::Holder<ContentLibMode>& holder)
{
	holder.addDetail("", "enabled", &ContentLibMode::enabled, "0", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "name", &ContentLibMode::name, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "endpoint", &ContentLibMode::endpoint, "", ZQ::common::Config::optReadOnly);
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
	holder.addDetail("", "volumeName", &LAMServer::volumeName, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "endpoint", &LAMServer::endpoint, "", ZQ::common::Config::optReadOnly);	
	holder.addDetail("Volume",&LAMServer::readContentVolume,&LAMServer::registerContentVolume);
}


void LAM::structure(ZQ::common::Config::Holder<LAM>& holder)
{	
	holder.addDetail("", "enableWarmup", &LAM::enableWarmup, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "contentNameFormat", &LAM::contentNameFmt, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("TestMode", &LAM::readTestMode, &LAM::registerTestMode);
	holder.addDetail("ContentLibMode", &LAM::readContentLibMode, &LAM::registerContentLibMode);
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
	lamTestMode.read(node, hPP);
}

void LAM::registerTestMode(const std::string &full_path)
{
	lamTestMode.snmpRegister(full_path);
}

void LAM::readContentLibMode(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	contentLibMode.read(node, hPP);
}

void LAM::registerContentLibMode(const std::string &full_path)
{
	contentLibMode.snmpRegister(full_path);
}

void LAM::readLAMServer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	//LAMServerHolder lamServer("volumeName");
	lamServer.read(node, hPP);	
}

void LAM::registerLAMServer(const std::string &full_path)
{	
// 	for (std::vector<LAMServerHolder>::iterator it = _lamServerDatas.begin(); it != _lamServerDatas.end(); ++it)
// 	{
// 		it->snmpRegister(full_path);
// 	}
}
void LAMSubTestContentAttribute::structure( ZQ::common::Config::Holder<LAMSubTestContentAttribute>& holder )
{
	holder.addDetail("","key",&LAMSubTestContentAttribute::key,"",ZQ::common::Config::optReadOnly);
	holder.addDetail("","value",&LAMSubTestContentAttribute::value,"",ZQ::common::Config::optReadOnly);
};

void AdsProvider::structure(ZQ::common::Config::Holder<AdsProvider>& holder)
{
	holder.addDetail("", "pid", &AdsProvider::pid, "", ZQ::common::Config::optReadOnly);
}

void AdsReplacment::registerProvider( const std::string &full_path )
{
}

void AdsReplacment::readProvider( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
{
	AdsProvider::AdsProviderHolder provider;
	provider.read(node,hPP);
	providers.push_back(provider);
}

void AdsReplacment::structure(ZQ::common::Config::Holder<AdsReplacment>& holder)
{
	holder.addDetail("", "enable", &AdsReplacment::enabled, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "provideLeadingAdsPlaytime", &AdsReplacment::provideLeadingAdsPlaytime, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "defaultTrickRestriction", &AdsReplacment::defaultTrickRestriction, "", ZQ::common::Config::optReadOnly);
	holder.addDetail("", "playOnce", &AdsReplacment::playOnce, "1", ZQ::common::Config::optReadOnly);
	holder.addDetail("AdsProvider",&AdsReplacment::readProvider,&AdsReplacment::registerProvider);
}

void Config::structure(ZQ::common::Config::Holder<Config>& holder)
{
	holder.addDetail("ssm_NGOD2/LogFile", &Config::readLogFile, &Config::registerLogFile);
	holder.addDetail("ssm_NGOD2/EventLogFile", &Config::readEventLogFile, &Config::regsiterNone);
	//readEventLogFile
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
	//holder.addDetail("ssm_NGOD2/Library", &Config::readLibrary, &Config::registerLibrary);
	holder.addDetail("ssm_NGOD2/playlistControl", &Config::readPlaylistControl, &Config::registerPlaylistControl);
	holder.addDetail("ssm_NGOD2/protocolVersioning", &Config::readProtocolVersioning, &Config::registerProtocolVersioning);
	holder.addDetail("ssm_NGOD2/MessageFormat",&Config::readMessageFormat,&Config::registerAnnounce);

	holder.addDetail("ssm_NGOD2/sessionHistory", &Config::readSessionHistory, &Config::registerSessionHistory);

	holder.addDetail("ssm_NGOD2/D5Speaker",&Config::readD5Message,&Config::regsiterNone);

	holder.addDetail("ssm_NGOD2/AssetStack" , &Config::readAssetStack , &Config::registerAssetStack );

	holder.addDetail("ssm_NGOD2/AdsReplacement", &Config::readAdsReplacment, &Config::registerAdsReplacment);
}

void Config::readAssetStack( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
{
	assetStack.read( node , hPP );
}

void Config::registerAssetStack( const std::string &full_path )
{

}
void Config::readD5Message( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
{
	d5messsage.read( node , hPP );
}

void Config::readPlaylistControl(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	playlistControl.read( node, hPP );
}
void Config::registerPlaylistControl(const std::string &full_path)
{
	playlistControl.snmpRegister(full_path);
}

void Config::readProtocolVersioning(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	protocolVersioning.read(node, hPP);
}

void Config::registerProtocolVersioning(const std::string &full_path)
{
	protocolVersioning.snmpRegister(full_path);
}

void Config::readSessionHistory(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	sessionHistory.read(node, hPP);
}

void Config::registerSessionHistory(const std::string &full_path)
{
	sessionHistory.snmpRegister(full_path);
}

void Config::readAdsReplacment(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	adsReplacment.read(node, hPP);
}

void Config::registerAdsReplacment(const std::string &full_path)
{
	adsReplacment.snmpRegister(full_path);
}

void Config::readLAM(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	lam.read(node, hPP);
}

void Config::registerLAM(const std::string &full_path)
{
	lam.snmpRegister(full_path);
}

void Config::readPassThruStreaming(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	passThruStreaming.read(node, hPP);
}

void Config::registerPassThruStreaming(const std::string &full_path)
{
	passThruStreaming.snmpRegister(full_path);
}

void Config::readIceProperties(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	iceProps.read(node, hPP);
}

void Config::registerIceProperties(const std::string &full_path)
{
	iceProps.snmpRegister(full_path);
}

void Config::readPublishLogs(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	publishLogs.read(node, hPP);
}

void Config::registerPublishLogs(const std::string &full_path)
{
	publishLogs.snmpRegister(full_path);
}

void Config::readResponse(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	response.read(node, hPP);
}

void Config::registerResponse(const std::string &full_path)
{
	response.snmpRegister(full_path);
}

void Config::readAnnounce(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	announce.read(node, hPP);
}

void Config::registerAnnounce(const std::string &full_path)
{
	announce.snmpRegister(full_path);
}

void Config::readDatabase(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	database.read(node, hPP);
}

void Config::registerDatabase(const std::string &full_path)
{
	database.snmpRegister(full_path);
}

void Config::readRTSPSession(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	rtspSession.read(node, hPP);
	//adjust second to millisecond
	rtspSession.timeout = rtspSession.timeout*1000;
	rtspSession.destroyRetryInterval = rtspSession.destroyRetryInterval*1000;
	rtspSession.maxSessionDestroyTimeInterval *= 1000;
}

void Config::registerRTSPSession(const std::string &full_path)
{
	rtspSession.snmpRegister(full_path);
}

void Config::readBind(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	bind.read(node, hPP);
}

void Config::registerBind(const std::string &full_path)
{
	bind.snmpRegister(full_path);
}

void Config::readIceStorm(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	iceStorm.read(node, hPP);
}

void Config::registerIceStorm(const std::string &full_path)
{
	iceStorm.snmpRegister(full_path);
}

void Config::readLogFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	pluginLog.read(node, hPP);
}

void Config::readEventLogFile( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
{
	eventLog.read(node,hPP);
}

void Config::registerLogFile(const std::string &full_path)
{
	pluginLog.snmpRegister(full_path);
}

void Config::readMessageFormat( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
{
	messageFmt.read(node,hPP);
}

void Config::readSOPProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	sopProp.read(node, hPP);
}

void Config::registerSOPProp(const std::string &full_path)
{
	sopProp.snmpRegister(full_path);
}

void Config::regsiterNone( const std::string& )
{

}


}

