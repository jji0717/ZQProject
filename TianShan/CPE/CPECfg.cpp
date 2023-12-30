

#include "CPECfg.h"

ZQ::common::Config::Loader<CPECfg> _gCPECfg("CPE.xml");

CPECfg::CPECfg()
{
	memset(_ftpBindIP,0,sizeof(_ftpBindIP));
	memset(_ftpRootUrl,0,sizeof(_ftpRootUrl));
	memset(_homeDir,0,sizeof(_homeDir));
	memset(_cpeNetId,0,sizeof(_cpeNetId));
	memset(_szCrashDumpPath,0,sizeof(_szCrashDumpPath));
	memset(_cpeEndPoint,0,sizeof(_cpeEndPoint));
	memset(_szCrashDumpPath,0,sizeof(_szCrashDumpPath));
	memset(_cpeNetId,0,sizeof(_cpeNetId));	
	memset(_cpcEndPoint,0,sizeof(_cpcEndPoint));	

	_maxConnection = 200;
	_dwSocketReadTimeoutSecs = 20;
	_dwFtpSpeedRate = 105;
	_dwDumpFullMemory = 1;
	_ftpThreadPoolSize = 40;
	_dwMinProgressInterval = 60*1000;	//60 seconds
	_dwMaxPecentageStep = 20;

	_dwEnableFtpOverVstrm = 0;			
	_dwVstrmBwClientId = 773220;
	_dwExportBitrate = 3750000;

	strcpy(_cpeEndPoint, "default -p 10010");
	
	_dwtimerThreadPool = 5;
	_dwThreadPool = 40;
	_ftpListenPort = 21;
	_dwMaxStartDelay = 120*000;		//millisecond
	_dwStopRemainTimeout = 60*1000; //millisecond

    _dwEnableIceLog = 0;
    _dwIceLogFileCount = 1;
    _dwIceLogFileSize = 10*1024*1024;
    
    _dwMaxWaitMsForQuit = 20*1000;	//20 seconds

	_dwRestartOnCertainError = 0;
	_dwMaxPecentageStep = 20;
	_minDurationSeconds = 5;	//5 seconds

	//setConfigFileName("CPE.xml");
}

void CPECfg::structure(ZQ::common::Config::Holder<CPECfg> &holder)
{
    using namespace ZQ::common;
    typedef Config::Holder<CPECfg>::PMem_CharArray PMem_CharArray;

    holder.addDetail("default/CrashDump", "path", (PMem_CharArray)&CPECfg::_szCrashDumpPath, sizeof(holder._szCrashDumpPath), NULL, Config::optReadOnly);
    holder.addDetail("default/CrashDump", "enabled", &CPECfg::_crashDumpEnabled, "1", Config::optReadOnly);

    holder.addDetail("default/IceTrace", "enabled", &CPECfg::_dwEnableIceLog, NULL, Config::optReadOnly);
    holder.addDetail("default/IceTrace", "level", &CPECfg::_iceLogLevel, NULL, Config::optReadWrite);
    holder.addDetail("default/IceTrace", "size", &CPECfg::_dwIceLogFileSize, NULL, Config::optReadOnly);
    holder.addDetail("default/IceTrace", "fileCount", &CPECfg::_dwIceLogFileCount, "1", Config::optReadOnly);

    holder.addDetail("default/Database", "path", (PMem_CharArray)&CPECfg::_szIceDbFolder, sizeof(holder._szIceDbFolder), NULL, Config::optReadOnly);
    holder.addDetail("default/Database", "runtimePath", (PMem_CharArray)&CPECfg::_szIceRuntimeDbFolder, sizeof(holder._szIceRuntimeDbFolder), "", Config::optReadOnly);

    holder.addDetail("ContentProvisionEngine", "netId", (PMem_CharArray)&CPECfg::_cpeNetId, sizeof(holder._cpeNetId), NULL, Config::optReadOnly);
	holder.addDetail("ContentProvisionEngine", "threads", &CPECfg::_dwThreadPool, NULL, Config::optReadOnly);
	holder.addDetail("ContentProvisionEngine", "maxthreads", &CPECfg::_dwMaxThreadPool, NULL, Config::optReadOnly);
	holder.addDetail("ContentProvisionEngine", "sessionCacheSize", &CPECfg::_nProSessEvictorSize, "500", Config::optReadOnly);
	holder.addDetail("ContentProvisionEngine", "timerthreads", &CPECfg::_dwtimerThreadPool, NULL, Config::optReadOnly);

    holder.addDetail("ContentProvisionEngine/Bind", "endpoint", (PMem_CharArray)&CPECfg::_cpeEndPoint, sizeof(holder._cpeEndPoint), NULL, Config::optReadOnly);
    holder.addDetail("ContentProvisionEngine/Bind", "dispatchSize", &CPECfg::_cpeDispatchSize, "30", Config::optReadOnly);
    holder.addDetail("ContentProvisionEngine/Bind", "dispatchMax", &CPECfg::_cpeDispatchSizeMax, "40", Config::optReadOnly);

	holder.addDetail("ContentProvisionEngine/ProvisionError", "restartOnCritical", &CPECfg::_dwRestartOnCertainError, "0", Config::optReadWrite);

    holder.addDetail("ContentProvisionEngine/ContentProvisionCluster", "endpoint", (PMem_CharArray)&CPECfg::_cpcEndPoint, sizeof(holder._cpcEndPoint), NULL, Config::optReadOnly);

    holder.addDetail("ContentProvisionEngine/PushTriggers/FTPServer", "bindIP", (PMem_CharArray)&CPECfg::_ftpBindIP, sizeof(holder._ftpBindIP), NULL, Config::optReadOnly);
    holder.addDetail("ContentProvisionEngine/PushTriggers/FTPServer", "port", &CPECfg::_ftpListenPort, NULL, Config::optReadOnly);
    holder.addDetail("ContentProvisionEngine/PushTriggers/FTPServer", "rootUrl", (PMem_CharArray)&CPECfg::_ftpRootUrl, sizeof(holder._ftpRootUrl), NULL, Config::optReadOnly);
    holder.addDetail("ContentProvisionEngine/PushTriggers/FTPServer", "homeDir", (PMem_CharArray)&CPECfg::_homeDir, sizeof(holder._homeDir), NULL, Config::optReadOnly);
    holder.addDetail("ContentProvisionEngine/PushTriggers/FTPServer", "threads", &CPECfg::_ftpThreadPoolSize, NULL, Config::optReadOnly);
    holder.addDetail("ContentProvisionEngine/PushTriggers/FTPServer", "maxConnection", &CPECfg::_maxConnection, NULL, Config::optReadWrite);
	holder.addDetail("ContentProvisionEngine/PushTriggers/FTPServer", "maxBandwidth", &CPECfg::_ftpMaxBandWidth, NULL, Config::optReadWrite);
    holder.addDetail("ContentProvisionEngine/PushTriggers/FTPServer", "dataConnTimeout", &CPECfg::_dwSocketReadTimeoutSecs, NULL, Config::optReadWrite);
	holder.addDetail("ContentProvisionEngine/PushTriggers/FTPServer/VirtualDirectory/Directory", &CPECfg::readDirInfo,&CPECfg::registerNothing);

	holder.addDetail("ContentProvisionEngine/PushTriggers/ContentExport/FtpOverVstrm", "enable", &CPECfg::_dwEnableFtpOverVstrm, NULL, Config::optReadOnly);
	holder.addDetail("ContentProvisionEngine/PushTriggers/ContentExport/FtpOverVstrm", "BWMgrClientId", &CPECfg::_dwVstrmBwClientId, NULL, Config::optReadOnly);
	holder.addDetail("ContentProvisionEngine/PushTriggers/ContentExport/FtpOverVstrm", "defaultBitrate", &CPECfg::_dwExportBitrate, NULL, Config::optReadWrite);	
	holder.addDetail("ContentProvisionEngine/PushTriggers/ContentExport/FtpOverVstrm", "maxBitrate", &CPECfg::_dwMaxBitrate, NULL, Config::optReadWrite);	
	holder.addDetail("ContentProvisionEngine/PushTriggers/ContentExport/FtpOverVstrm", "enableAuthorization", &CPECfg::_enableAuthorization, NULL, Config::optReadWrite);	
	holder.addDetail("ContentProvisionEngine/PushTriggers/ContentExport/FtpOverVstrm", "ttl", &CPECfg::_timeForLive, NULL, Config::optReadWrite);	

    holder.addDetail("ContentProvisionEngine/ScheduleExecution", "maxDelayStartMs", &CPECfg::_dwMaxStartDelay, "600000", Config::optReadWrite);
    holder.addDetail("ContentProvisionEngine/ScheduleExecution", "maxStoppedLingerMs", &CPECfg::_dwStopRemainTimeout, "1000", Config::optReadWrite);
	holder.addDetail("ContentProvisionEngine/ScheduleExecution", "minDurationSeconds", &CPECfg::_minDurationSeconds, "5", Config::optReadWrite);

	holder.addDetail("ContentProvisionEngine/Event/Progress", "minIntervalMs", &CPECfg::_dwMinProgressInterval, NULL, Config::optReadWrite);
	holder.addDetail("ContentProvisionEngine/Event/Progress", "maxPecentageStep", &CPECfg::_dwMaxPecentageStep, "20", Config::optReadWrite);

    holder.addDetail("ContentProvisionEngine/IceProperties/prop", &CPECfg::readIceProp, &CPECfg::registerNothing);    
	 holder.addDetail("ContentProvisionEngine/Plugins/CPH", &CPECfg::readCPHPluginfile, &CPECfg::registerNothing);   
	holder.addDetail("default/PublishedLogs/Log", &CPECfg::readMonitoredLog, &CPECfg::registerNothing);
	holder.addDetail("ContentProvisionEngine/CriticalProvisionError/Error", &CPECfg::readCriticalProError, &CPECfg::registerNothing);   
	holder.addDetail("ContentProvisionEngine/MediaSample", &CPECfg::readMediaSamplebuffer, &CPECfg::registerNothing, Config::Range(0,1));    
	holder.addDetail("ContentProvisionEngine/Volumes/", &CPECfg::readVolumeMounts, &CPECfg::registerNothing, Config::Range(0, 1));
}
CPECfg::~CPECfg()
{

}

void CPECfg::readIceProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
{
	using namespace ZQ::common::Config;
	Holder<NVPair> propHolder;
	propHolder.read(node, hPP);
	iceProperties[propHolder.name] = propHolder.value;
}
void CPECfg::readCPHPluginfile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
{
	using namespace ZQ::common::Config;
	Holder<Plugin> fileHolder;
	fileHolder.read(node, hPP);
	if (fileHolder.enable.empty())
		cphPlugins[fileHolder.file] = 0;
	else
		cphPlugins[fileHolder.file] = atoi(fileHolder.enable.c_str());
}

void CPECfg::readMonitoredLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
{
	using namespace ZQ::common::Config;
	Holder<MonitoredLog> lmHolder;
	lmHolder.read(node, hPP);
	monitoredLogs.push_back(lmHolder);
}

void CPECfg::readCriticalProError(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
{
	using namespace ZQ::common::Config;
	Holder<CriticalProvisionError> lmHolder;
	lmHolder.read(node, hPP);
	criticalErrors.push_back(lmHolder);
}

void CPECfg::readDirInfo( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP )
{
	using namespace ZQ::common::Config;
	Holder<NVPair> propHolder;
	propHolder.read(node, hPP);
	dirInfos[propHolder.name] = propHolder.value;

}

void CPECfg::readMediaSamplebuffer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP)
{
	using namespace ZQ::common::Config;
	Holder<MediaSampleBuffer> mediasampleHolder;
	mediasampleHolder.read(node, hPP);
	_mediasamplebuffer = mediasampleHolder;
}

void CPECfg::readVolumeMounts(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP) 
{
	using namespace ZQ::common::Config;
	Holder<VolumeMounts> volumeHolder;
	volumeHolder.read(node, hPP);

	volumeMounts = volumeHolder;
}
