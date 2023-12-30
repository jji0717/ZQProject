

#ifndef _CME_CONFIG_
#define _CME_CONFIG_
#include <ConfigHelper.h>

namespace CacheManagement {

class CMECfg
{
private:
	std::string	_cmeEndPointRaw;    // the endpoint read from xml
	std::string _lamEndPointRaw;	//  the endpoint read from xml

public:
    CMECfg();
    ~CMECfg();

	int32			_cacheLogSize;
	int32			_cacheLogLevel;
	int32           _cacheLogCount;
	int32           _cacheLogBuff;

    int32			_crashDumpEnabled;		// enable crash dump
	int32			_dumpFullMemory;
	std::string		_crashDumpPath;
	std::string		_cmeEndPoint;
	std::string		_cmeNetId;
	std::string		_lamEndPoint;

	int32			_vsisRecBuffSize;
	int32			_vsisConnTimeout;
	int32			_vsisSendTimeout;
	int32			_vsisIdleTimeout;
	int32			_vsisScanInterval;
	int32			_vsisReportInterval;
	int32			_lengthOfPAID;

	int32			_storageThreshold;
    int32			_storageCushion;
	int32			_adDuration;		// in seconds
	int32			_visaSeconds;
	int32			_trimDays;
	int32			_playTrigger;
	int32           _playTrigger2;
	int32			_ageDenominator;	// 1/_ageDenominator will be used for aging
	int32			_agePeriod;			// in seconds
	int32			_subFileCount;		

	int32           _bwThreshold;
	int32           _bwReserved;

	std::string		_datPath;
	std::string     _datType;
	int32			_datFlushInterval;
	int32			_syncIOAtStart;

	int32           _statEnabled;
	int32           _statWindow;
	int32           _statInterval;
	int32           _statDelay;
	int32           _statCntExtraSizePer;
	int32           _statPrintDetails;

	int32           _proactiveEnabled;
	std::string     _proactivePath;
	int32           _proactiveRetryInterval;

    static void structure(ZQ::common::Config::Holder<CMECfg> &holder);

	void registerNothing(const std::string&){}

public:
	std::string _cmeBindIP;
	int32 _cmeBindPort;

	bool parseCMEEndpoint();
	bool parseLAMEndpoint();
};


extern ZQ::common::Config::Loader<CMECfg> _gCMECfg;

} // namespace CacheManagement
#endif
