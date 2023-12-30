

#ifndef _CPH_RDSRTF_CONFIG_
#define _CPH_RDSRTF_CONFIG_
//#include "ConfigLoader.h"
#include <ConfigHelper.h>

class FscpConfig
{
public:
	FscpConfig ();
	~FscpConfig (){};

	int32   maxSessionNum;
	int32	mediaSampleSize;
	
	char	szCacheDir[256];
	int32	enablePacingTrace;
	int32	bandwidthLimitRate;
	
	int32   waittime;
	
	int32 enableProgEvent;
	int32 enableStreamEvent;
	int32 maxBandwidthBps;
	int32 streamReqSecs;
	
	// for test
	char	szNTFSOutputDir[256];	
	int32	enableTestNTFS;
	
	
	
	// for vstrm bandwidth management
	int32	vstrmBwClientId;
	int32	vstrmDisableBufDrvThrottle;
	
	int32	rtfMaxSessionNum;
	int32	rtfMaxInputBufferBytes;
	int32	rtfMaxInputBuffersPerSession;
	int32	rtfSessionFailThreshold;
	
	
	static void structure(ZQ::common::Config::Holder<FscpConfig> &holder)
	{
		using namespace ZQ::common::Config;
		typedef ZQ::common::Config::Holder<FscpConfig>::PMem_CharArray PMem_CharArray;
		holder.addDetail("CPH_FSCP", "maxSessions", &FscpConfig::maxSessionNum, NULL, optReadOnly);
		holder.addDetail("CPH_FSCP", "maxBandwidth", &FscpConfig::maxBandwidthBps, NULL, optReadOnly);
		holder.addDetail("CPH_FSCP/MediaSample", "bufferSize", &FscpConfig::mediaSampleSize, NULL, optReadOnly);
        holder.addDetail("CPH_FSCP/WaitTime", "timeInterval", &FscpConfig::waittime, NULL, optReadOnly);
		holder.addDetail("CPH_FSCP/RTF", "maxSessionNum", &FscpConfig::rtfMaxSessionNum, NULL, optReadOnly);
		holder.addDetail("CPH_FSCP/RTF", "maxInputBufferBytes", &FscpConfig::rtfMaxInputBufferBytes, NULL, optReadOnly);
		holder.addDetail("CPH_FSCP/RTF", "maxInputBuffersPerSession", &FscpConfig::	rtfMaxInputBuffersPerSession, NULL, optReadOnly);
		holder.addDetail("CPH_FSCP/RTF", "sessionFailThreshold", &FscpConfig::rtfSessionFailThreshold, NULL, optReadOnly);
		holder.addDetail("CPH_FSCP/Event/Progress", "enable", &FscpConfig::enableProgEvent, NULL, optReadOnly);
		holder.addDetail("CPH_FSCP/Event/Streamable", "enable", &FscpConfig::enableStreamEvent, NULL, optReadOnly);
		holder.addDetail("CPH_FSCP/Event/Streamable", "lagAfterStart", &FscpConfig::streamReqSecs, NULL, optReadOnly);
		holder.addDetail("CPH_FSCP/PacedIndex", "cacheDir", (PMem_CharArray)&FscpConfig::szCacheDir, sizeof(holder.szCacheDir), "", optReadOnly);
		holder.addDetail("CPH_FSCP/PacedIndex", "enableTrace", &FscpConfig::enablePacingTrace, NULL, optReadOnly);
		holder.addDetail("CPH_FSCP/Vstream", "BWMgrClientId", &FscpConfig::vstrmBwClientId, NULL, optReadOnly);
		holder.addDetail("CPH_FSCP/Vstrm", "disableBufDrvThrottle", &FscpConfig::vstrmDisableBufDrvThrottle, NULL, optReadOnly);
		holder.addDetail("CPH_FSCP/NtfsOutputMode", "enable", &FscpConfig::enableTestNTFS, NULL, optReadOnly);
		holder.addDetail("CPH_FSCP/NtfsOutputMode", "homeDir", (PMem_CharArray)&FscpConfig::szNTFSOutputDir, sizeof(holder.szNTFSOutputDir), NULL, optReadOnly);
		
		
	}
	
	void registerNothing(const std::string&){}
	
	
};


// class CPHFSCPCfg : public ZQ::common::ConfigLoader 
// {
// public:
// 	CPHFSCPCfg ();
// 	~CPHFSCPCfg ();
// 	virtual ConfigSchemaItem*		getSchema();
// 	
// public:
// 	
// 	char	szCacheDir[256];
// 	DWORD	mediaSampleSize;
// 	DWORD	bandwidthLimitRate;
// 	DWORD	enableProgEvent;
// 	DWORD	enableStreamEvent;
// 	DWORD	streamReqSecs;
// 	DWORD	enableMD5;
// 	DWORD	enablePacingTrace;
// 
// 	// for RTF
// 	DWORD	maxSessionNum;
// 	DWORD	maxInputBufferBytes;
// 	DWORD	maxInputBuffersPerSession;
// 	DWORD	maxCodingError;
// 	
// 
// 	// for test
// 	char	szNTFSOutputDir[256];	
// 	DWORD	enableTestNTFS;			//if 1 then do not write to vstrm but write to NTFS
//     DWORD   waittime;
// };
// 
// extern CPHFSCPCfg _gCPHCfg;
extern ZQ::common::Config::Loader<FscpConfig> _gCPHCfg;

#endif