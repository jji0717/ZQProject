

#ifndef _CPE_CONFIG_
#define _CPE_CONFIG_
#include <ConfigHelper.h>

struct MonitoredLog
{
    std::string name;
    std::string syntax;
	std::string syntaxKey;
	std::string logType;
    static void structure(ZQ::common::Config::Holder<MonitoredLog> &holder)
    {
        holder.addDetail("", "path", &MonitoredLog::name);
        holder.addDetail("", "syntax", &MonitoredLog::syntax);
		holder.addDetail("", "key", &MonitoredLog::syntaxKey);
		holder.addDetail("", "type", &MonitoredLog::logType);
    }
};

struct Plugin
{
	std::string file;
	std::string enable;
	static void structure(ZQ::common::Config::Holder<Plugin> &holder)
	{
		holder.addDetail("", "file", &Plugin::file);
		holder.addDetail("", "enable", &Plugin::enable);
	}
};

struct CriticalProvisionError
{
	std::string keyword;
	static void structure(ZQ::common::Config::Holder<CriticalProvisionError> &holder)
	{
		holder.addDetail("", "keyword", &CriticalProvisionError::keyword);
	}
};

struct VolumeConfig {
	std::string name;
	std::string value;

	static void structure(ZQ::common::Config::Holder<VolumeConfig>& holder) {
		holder.addDetail("", "name", &VolumeConfig::name);
		holder.addDetail("", "value", &VolumeConfig::value);
	}
};
struct MediaSampleBuffer {

	int32  _mediaSampleBufferSize;
	int32  _maxBufferPoolSize;
	int32  _minBufferPoolSize;

	static void structure(ZQ::common::Config::Holder<MediaSampleBuffer>& holder) {
		holder.addDetail("", "bufferSize", &MediaSampleBuffer::_mediaSampleBufferSize);
		holder.addDetail("", "maxBufferPoolSize", &MediaSampleBuffer::_maxBufferPoolSize);
		holder.addDetail("", "minBufferPoolSize", &MediaSampleBuffer::_minBufferPoolSize);
	}
	MediaSampleBuffer()
	{
		_mediaSampleBufferSize = 65536;
		_maxBufferPoolSize = 12000;
		_minBufferPoolSize = 50;
	}
};
struct Volume
{
	std::string name;
	std::string path;
	std::string fstype;
	int32 isDefault;

	static void structure(ZQ::common::Config::Holder<Volume>& holder) 
	{
		holder.addDetail("", "name", &Volume::name);
		holder.addDetail("", "path", &Volume::path);
		holder.addDetail("", "fstype", &Volume::fstype,"enfs");
		holder.addDetail("", "default", &Volume::isDefault, "0");
	}
};

typedef std::vector< ZQ::common::Config::Holder<Volume> > Volumes;

struct VolumeMounts
{

	int32 _enableVoumeMounts;
	Volumes volumes;

    VolumeMounts()
	{
			_enableVoumeMounts = 0 ;
	};
	~VolumeMounts(){};
	static void structure(ZQ::common::Config::Holder<VolumeMounts>& holder) 
	{
		holder.addDetail("", "enable", &VolumeMounts::_enableVoumeMounts, "0", ZQ::common::Config::optReadOnly);

		holder.addDetail("Volume", &VolumeMounts::readVolume, &VolumeMounts::registerNothing, ZQ::common::Config::Range(0, -1));
	}
	void readVolume(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP) 
	{
		using namespace ZQ::common::Config;
		Holder<Volume> volumeHolder;
		volumeHolder.read(node, hPP);
		volumes.push_back(volumeHolder);
	}
	 void registerNothing(const std::string&){}
};

class CPECfg
{
public:
    CPECfg();
    ~CPECfg();
	int32	_ftpListenPort;
	char	_ftpBindIP[128];
	char	_ftpRootUrl[128];
	int32	_ftpThreadPoolSize;
	char	_homeDir[256];
	int32	_maxConnection;
	int32   _ftpMaxBandWidth;
	int32	_dwSocketReadTimeoutSecs;

	int32	_dwEnableFtpOverVstrm;			
	int32	_dwVstrmBwClientId;
	int32	_dwExportBitrate;
	int32   _dwMaxBitrate;
	
	int32	_dwFtpSpeedRate;
	int32	_dwTicketBitrateCheckRate;

	char	_szCrashDumpPath[256];

    int32   _crashDumpEnabled; // enable crash dump

	int32	_dwDumpFullMemory;

	int32	_dwRestartOnCertainError;

	char	_cpeNetId[128];
	char	_cpcEndPoint[128];
	char	_cpeEndPoint[128];
	int32	_dwMinProgressInterval;
	int32   _dwMaxPecentageStep;
	int32	_dwMaxStartDelay;
	int32	_dwStopRemainTimeout;
	int32	_dwThreadPool;
	int32   _dwMaxThreadPool;
	int32   _dwtimerThreadPool;

	int32	_minDurationSeconds;

	//ice property
	int32   _nProSessEvictorSize;		//Ice.ThreadPool.Server.Size
	int32   _cpeDispatchSize;		//Ice.ThreadPool.Server.Size
    int32   _cpeDispatchSizeMax;	//Ice.ThreadPool.Server.SizeMax
	char	_szIceDbFolder[256];
    char    _szIceRuntimeDbFolder[256];

    int32	_dwEnableIceLog;
    int32	_dwIceLogFileCount;
    int32	_dwIceLogFileSize;
    int32   _iceLogLevel;
    int32	_dwMaxWaitMsForQuit;

	int32  _enableAuthorization;
	int32  _timeForLive;

    std::vector<MonitoredLog> monitoredLogs;
    std::map<std::string, std::string> iceProperties;
	std::map<std::string, int> cphPlugins;
	std::vector<CriticalProvisionError>		criticalErrors;
	std::map<std::string, std::string> dirInfos;

	MediaSampleBuffer _mediasamplebuffer;

	VolumeMounts volumeMounts;
    static void structure(ZQ::common::Config::Holder<CPECfg> &holder);
    void readIceProp(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP);
	void readCPHPluginfile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP);
    void readMonitoredLog(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP);
	void readCriticalProError(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP);
	void readDirInfo(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP);
	void readMediaSamplebuffer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor *hPP);
	void readVolumeMounts(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
    void registerNothing(const std::string&){}
};


extern ZQ::common::Config::Loader<CPECfg> _gCPECfg;

#endif
