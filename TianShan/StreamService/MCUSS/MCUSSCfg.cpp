#include "MCUSSCfg.h"
namespace ZQTianShan{
	namespace MCUSS{
		MCUSSCfg::MCUSSCfg(void)
		{
		}

		MCUSSCfg::~MCUSSCfg(void)
		{
		}

		void MCUSSCfg::structure(Config::Holder<MCUSSCfg> &holder)
		{
			using namespace ZQ::common;
			typedef Config::Holder<MCUSSCfg>::PMem_CharArray PMem_CharArray;
			//default
			holder.addDetail("default/CrashDump", "path", (PMem_CharArray)&MCUSSCfg::_szCrashDumpPath, sizeof(holder._szCrashDumpPath) ,NULL,Config::optReadOnly);
			holder.addDetail("default/CrashDump", "enabled", &MCUSSCfg::_crashDumpEnabled, "1", Config::optReadOnly);
			holder.addDetail("default/IceTrace", "enabled", &MCUSSCfg::_iceTraceLogEnabled, "1", Config::optReadOnly);
			holder.addDetail("default/IceTrace", "level", &MCUSSCfg::_iceTraceLogLevel, "6", Config::optReadOnly);
			holder.addDetail("default/IceTrace", "size", &MCUSSCfg::_iceTraceLogSize, "102400000", Config::optReadOnly);
			holder.addDetail("default/EventChannel", "endPoint", (PMem_CharArray)&MCUSSCfg::_eventChannelEndpoint, sizeof(holder._eventChannelEndpoint) ,NULL,Config::optReadOnly);
			holder.addDetail("default/Database", "path", (PMem_CharArray)&MCUSSCfg::_szIceDbFolder, sizeof(holder._szIceDbFolder) ,NULL,Config::optReadOnly);
			holder.addDetail("default/Database", "runtimePath", (PMem_CharArray)&MCUSSCfg::_szIceRuntimeDbFolder, sizeof(holder._szIceRuntimeDbFolder) ,NULL,Config::optReadOnly);
			//MCUSS
			holder.addDetail("MCUSS", &MCUSSCfg::readMCUSS, &MCUSSCfg::registerMCUSS);
		}

		void MCUSSCfg::readMCUSS( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP )
		{
			ST_MCUSS::MCUSSHolder mcussHolder("");
			mcussHolder.read(node, hPP);
			_mcussList.push_back(mcussHolder);
		}
		void MCUSSCfg::registerMCUSS(const std::string& full_path )
		{
			for (MCUSSList::iterator it = _mcussList.begin(); it != _mcussList.end(); ++it)
			{
				it->snmpRegister(full_path);
			}
		}

		//ST_MCUSS
		void ST_MCUSS::structure(MCUSSHolder &holder)
		{
			holder.addDetail("", "netId", &ST_MCUSS::_netId, "", ZQ::common::Config::optReadOnly);
			holder.addDetail("Bind", "endPoint", &ST_MCUSS::_bindEndpoint, "", Config::optReadOnly);
			holder.addDetail("Bind", "dispatchSize", &ST_MCUSS::_bindDispatchSize, "", Config::optReadOnly);
			holder.addDetail("Bind", "dispatchMax", &ST_MCUSS::_bindDispatchMax, "", Config::optReadOnly);
			holder.addDetail("Bind", "pendingMax", &ST_MCUSS::_bindPendingMax, "100", Config::optReadOnly);
			holder.addDetail("Bind", "evictorSize", &ST_MCUSS::_bindEvictorSize, "", Config::optReadOnly);
			holder.addDetail("Bind", "threadPoolSize", &ST_MCUSS::_bindThreadPoolSize, "10", Config::optReadOnly);
			holder.addDetail("Bind", "contentstoreThreadPoolSize", &ST_MCUSS::_bindContentStoreThreadPoolSize, "10", Config::optReadOnly);
			//holder.addDetail("sync", "enable", &ST_MCUSS::_syncCall, "1", Config::optReadOnly);
			//holder.addDetail("sync", "timeout", &ST_MCUSS::_syncTimeOut, "10000", Config::optReadOnly);

			holder.addDetail("IceProperties/prop", &ST_MCUSS::readIceProp, &ST_MCUSS::registerNothing);
			holder.addDetail("SessionHistory", &ST_MCUSS::readSessionHistory, &ST_MCUSS::registerSessionHistory);
			holder.addDetail("PostEvent", &ST_MCUSS::readPostEvent, &ST_MCUSS::registerPostEvent);
			holder.addDetail("VideoServer", &ST_MCUSS::readVideoServer, &ST_MCUSS::registerVideoServer);
			holder.addDetail("PublishedLogs", &ST_MCUSS::readPublishedLog, &ST_MCUSS::registerPublishedLog);
		}

		void ST_MCUSS::readIceProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
		{
			using namespace ZQ::common::Config;
			Holder<NVPair> propHolder;
			propHolder.read(node, hPP);
			_icePropertiesMap[propHolder.name] = propHolder.value;
		}

		void ST_MCUSS::readPublishedLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
		{
			PublishedLogHolder holder("");
			holder.read(node, hPP);
			_publishedLog = holder;
		}

		void ST_MCUSS::registerPublishedLog(const std::string &full_path)
		{
			_publishedLog.snmpRegister(full_path);
		}

		void ST_MCUSS::readSessionHistory( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP )
		{
			_sessHistoryHolder.read(node,hPP);
		}

		void ST_MCUSS::registerSessionHistory(const std::string& full_path )
		{
			_sessHistoryHolder.snmpRegister(full_path);
		}

		void ST_MCUSS::readPostEvent( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP )
		{
			_postEventHolder.read(node,hPP);
		}

		void ST_MCUSS::registerPostEvent(const std::string& full_path )
		{
			_postEventHolder.snmpRegister(full_path);
		}

		void ST_MCUSS::readVideoServer( ZQ::common::XMLUtil::XmlNode node , const ZQ::common::Preprocessor* hPP )
		{
			_videoServerHolder.read(node,hPP);
		}

		void ST_MCUSS::registerVideoServer(const std::string& full_path )
		{
			_videoServerHolder.snmpRegister(full_path);
		}

		// PublisthLog
		void PublishLog::structure(ZQ::common::Config::Holder<PublishLog>& holder) {
			holder.addDetail("", "path", &PublishLog::_path, "", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "syntax", &PublishLog::_syntax, "", ZQ::common::Config::optReadOnly);
			holder.addDetail("","key",&PublishLog::_key,"");
			holder.addDetail("","type",&PublishLog::_type,"");
		}

		void PublishLogs::structure(ZQ::common::Config::Holder<PublishLogs>& holder) {
			holder.addDetail("Log", &PublishLogs::readPublishLog, &PublishLogs::registerPublishLog);
		}

		void PublishLogs::readPublishLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP) {
			PublishLogHolder logHolder("path");
			logHolder.read(node, hPP);
			_logDatas.push_back(logHolder);
		}

		void SessionHistory::structure(ZQ::common::Config::Holder< SessionHistory > &holder)
		{
			holder.addDetail("", "enable", &SessionHistory::enable, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("", "path", &SessionHistory::path, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("", "level", &SessionHistory::level, "", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "maxCount", &SessionHistory::maxCount, "", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "size", &SessionHistory::size, "", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "bufferSize", &SessionHistory::bufferSize, "", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "flushTimeout", &SessionHistory::flushTimeout, "", ZQ::common::Config::optReadOnly);
		}

		void PostEvent::structure(ZQ::common::Config::Holder< PostEvent > &holder)
		{
			holder.addDetail("", "enableScaleChangeEvent", &PostEvent::enableScaleChangeEvent, "0", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "enableStateChangeEvent", &PostEvent::enableStateChangeEvent, "0", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "passScaleChangeEvent", &PostEvent::passScaleChangeEvent, "1", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "passStateChangeEvent", &PostEvent::passStateChangeEvent, "1", ZQ::common::Config::optReadOnly);
		}

		void streamDatabasePerftune::structure( ZQ::common::Config::Holder< streamDatabasePerftune >& holder )
		{
			holder.addDetail("", "checkpointPeriod", &streamDatabasePerftune::databaseStreamCheckpointPeriod, "120", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "saveSizeTrigger", &streamDatabasePerftune::databaseStreamSaveSizeTrigger, "100", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "savePeriod", &streamDatabasePerftune::databaseStreamSavePeriod, "60000", ZQ::common::Config::optReadOnly);			
		}

		void SessionInterface::structure(SessionInterfaceHolder &holder)
		{
			//load SessionInterface attribute
			holder.addDetail("", "bind", &SessionInterface::SessionInterfaceBind, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("", "ip", &SessionInterface::SessionInterfaceIp, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("", "port", &SessionInterface::SessionInterfacePort, "554", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "maxSessionGroup", &SessionInterface::SessionInterfaceMaxSessionGroup, "", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "maxSessionsPerGroup", &SessionInterface::SessionInterfaceMaxSessionsPerGroup, "", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "requestTimeout", &SessionInterface::SessionInterfaceRequestTimeout, "", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "disconnectAtTimeout", &SessionInterface::SessionInterfaceDisconnectAtTimeout,"20",ZQ::common::Config::optReadOnly);
			//holder.addDetail(nssStreamerLayer, &SessionInterface::readStreamer, &VideoServer::registerStreamer);
			holder.addDetail("", "tryDecimalNpt", &SessionInterface::SessionInterfaceTryDecimalNpt, "0", ZQ::common::Config::optReadOnly);

			//load FixedSpeedSet attribute
			holder.addDetail("FixedSpeedSet", "enable", &SessionInterface::FixedSpeedSetEnable, "", ZQ::common::Config::optReadOnly);
			holder.addDetail("FixedSpeedSet", "enableSpeedLoop", &SessionInterface::EnableFixedSpeedLoop, "1", ZQ::common::Config::optReadOnly);
			holder.addDetail("FixedSpeedSet", "forward", &SessionInterface::FixedSpeedSetForward, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("FixedSpeedSet", "backward", &SessionInterface::FixedSpeedSetBackward, NULL, ZQ::common::Config::optReadOnly);
		}

		//vol
		void Vol::structure(VolHolder &holder)
		{
			holder.addDetail("", "mount", &Vol::mount, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("", "targetName", &Vol::targetName, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("", "default", &Vol::defaultVal, "1", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "defaultBitRate", &Vol::defaultBitRate, "37500000", ZQ::common::Config::optReadOnly);
		}		

		//ContentInterface
		void ContentInterface::structure(ContentInterfaceHolder &holder)
		{
			//load ContentInterface
			holder.addDetail("", "ip", &ContentInterface::ContentInterfaceIp, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("", "port", &ContentInterface::ContentInterfacePort, "8080", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "path", &ContentInterface::ContentInterfacePath, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("", "syncInterval", &ContentInterface::ContentInterfaceSyncInterval, "200000", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "syncRetry", &ContentInterface::ContentInterfaceSyncRetry, "3", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "mode", &ContentInterface::ContentInterfaceMode, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("", "httpTimeOut", &ContentInterface::ContentInterfaceHttpTimeOut, "200000", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "destroyEnable", &ContentInterface::ContentInterfaceDestroyEnable, "0", ZQ::common::Config::optReadOnly);
			holder.addDetail("", "urlPercentDecodeOnOutgoingMsg", &ContentInterface::urlPercentDecodeOnOutgoingMsg, "0", ZQ::common::Config::optReadOnly);

			// enh#17598 - For CacheServer's ContentEdge/ secondary ContentEdge, NSS to enable dual interface to A3 server
			holder.addDetail("", "secondaryIP",   &ContentInterface::ContentInterface2ndIp,   "",   ZQ::common::Config::optReadOnly);
			holder.addDetail("", "secondaryPort", &ContentInterface::ContentInterface2ndPort, "-1", ZQ::common::Config::optReadOnly);

			//load Feedback attribute
			holder.addDetail("Feedback", "ip", &ContentInterface::FeedbackIp, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("Feedback", "port", &ContentInterface::FeedbackPort, "", ZQ::common::Config::optReadOnly);

			//load StoreReplica attribute
			holder.addDetail("StoreReplica", "groupId", &ContentInterface::StoreReplicaGroupId, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("StoreReplica", "replicaId", &ContentInterface::StoreReplicaReplicaId, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("StoreReplica", "replicaPriority", &ContentInterface::StoreReplicaReplicaPriority, "", ZQ::common::Config::optReadOnly);
			holder.addDetail("StoreReplica", "timeout", &ContentInterface::StoreReplicaTimeout, "", ZQ::common::Config::optReadOnly);

			//load DatabaseCache attribute
			holder.addDetail("DatabaseCache", "volumeSize", &ContentInterface::DatabaseCacheVolumeSize, "", ZQ::common::Config::optReadOnly);
			holder.addDetail("DatabaseCache", "contentSize", &ContentInterface::DatabaseCacheContentSize, "", ZQ::common::Config::optReadOnly);
			holder.addDetail("DatabaseCache", "contentSavePeriod", &ContentInterface::DatabaseCacheContentSavePeriod, "", ZQ::common::Config::optReadOnly);
			holder.addDetail("DatabaseCache", "contentSaveSizeTrigger", &ContentInterface::DatabaseCacheContentSaveSizeTrigger, "", ZQ::common::Config::optReadOnly);

			holder.addDetail("Volumes/Vol", &ContentInterface::readVol, &ContentInterface::registerVol);
		}

		void ContentInterface::readVol(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
		{
			Vol::VolHolder volHolder("");
			volHolder.read(node, hPP);
			vols.push_back(volHolder);
		}

		void ContentInterface::registerVol(const std::string &full_path)
		{
			for (VolList::iterator it = vols.begin(); it != vols.end(); ++it)
			{
				it->snmpRegister(full_path);
			}
		}

		//video Server
		void VideoServer::structure(VideoServerHolder &holder)
		{
			holder.bSeperateIPStreaming =0; // not yet be configureable
			//load videoserver attribute
			holder.addDetail("", "vendor", &VideoServer::vendor, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("", "model", &VideoServer::model, NULL, ZQ::common::Config::optReadOnly);
			holder.addDetail("", "enableMessageBinaryDump", &VideoServer::enableMessageBinaryDump, "0", ZQ::common::Config::optReadWrite);
			holder.addDetail("", "streamSyncInterval", &VideoServer::streamSyncInterval, "3600", ZQ::common::Config::optReadWrite);
			holder.addDetail("", "sessionRenewInterval", &VideoServer::sessionRenewInterval, "600", ZQ::common::Config::optReadWrite);
			holder.addDetail("CDN", "libraryVolume", &VideoServer::libraryVolume, "library",  ZQ::common::Config::optReadOnly);

			holder.addDetail("DatabasePerfTune", &VideoServer::readStreamDatabasePerftune, &VideoServer::registerStreamDatabasePerftune);
			holder.addDetail("SessionInterface", &VideoServer::readSessionInterface, &VideoServer::registerSessionInterface);
			holder.addDetail("ContentInterface", &VideoServer::readContentInterface, &VideoServer::registerContentInterface);
		}

		void VideoServer::readStreamDatabasePerftune(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
		{
			streamDbPerfTune.read(node,hPP);
		}

		void VideoServer::registerStreamDatabasePerftune(const std::string &full_path)
		{
			streamDbPerfTune.snmpRegister(full_path);
		}

		void VideoServer::readSessionInterface(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
		{
			sessionInterfaceHolder.read(node, hPP);
		}

		void VideoServer::registerSessionInterface(const std::string &full_path)
		{
			sessionInterfaceHolder.snmpRegister(full_path);
		}

		void VideoServer::readContentInterface(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
		{
			contentInterfaceHolder.read(node, hPP);
		}

		void VideoServer::registerContentInterface(const std::string &full_path)
		{
			contentInterfaceHolder.snmpRegister(full_path);
		}

		extern ZQ::common::Config::Loader<ZQTianShan::MCUSS::MCUSSCfg> gMCUSSConfig;

	}//namespace ZQTianShan
}//namespace NssStream