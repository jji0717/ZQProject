
#include "CPH_RTFRDSCfg.h"

using namespace ZQ::common;

ZQ::common::Config::Loader<CPHRTFRDSCfg> _gCPHCfg("CPH_RTFRDS.xml");

CPHRTFRDSCfg::CPHRTFRDSCfg()
{
	memset(szCacheDir, 0, sizeof(szCacheDir));
	mediaSampleSize=64*1024;		//64K for vstrm
	maxAllocSampleCount = 400;
	enableProgEvent = 1;
	enableStreamEvent = 1;
	enablePacingTrace = 0;	
	bandwidthLimitRate = 110;
	streamReqSecs = 5;				//5 seconds

	//maxBandwidthKBps = 100*1024;	//100mbps
	//maxSessionNum = 30;
	rtfMaxInputBufferBytes = 64*1024;	
	rtfMaxInputBuffersPerSession = 100;
	rtfSessionFailThreshold = 0;
	enableMD5 = 0;
    decodeSourceURL = 0;
	vstrmBwClientId = 773220;
	vstrmDisableBufDrvThrottle = 1;

	// for test
	memset(szNTFSOutputDir, 0, sizeof(szNTFSOutputDir));
	memset(szLocalNetIf, 0, sizeof(szLocalNetIf));
	enableTestNTFS = 0;			//if 1 then do not write to vstrm but write to NTFS
	enableFtpPassiveMode = 1;
	ftpConnectionInterval = 30000; //ms
	traceIndexWrite = 0;
	unifiedtrickfile.enable = 1;
}

CPHRTFRDSCfg::~CPHRTFRDSCfg()
{

}

void CPHRTFRDSCfg::structure(ZQ::common::Config::Holder<CPHRTFRDSCfg> &holder)
{
    using namespace ZQ::common;
    typedef Config::Holder<CPHRTFRDSCfg>::PMem_CharArray PMem_CharArray;
	
    holder.addDetail("CPH_RtfRds/MediaSample", "bufferSize", &CPHRTFRDSCfg::mediaSampleSize, NULL, Config::optReadOnly);
	holder.addDetail("CPH_RtfRds/MediaSample", "maxAllocSampleCount", &CPHRTFRDSCfg::maxAllocSampleCount, "400", Config::optReadOnly);
    holder.addDetail("CPH_RtfRds/Session", "overSpeedRate", &CPHRTFRDSCfg::bandwidthLimitRate, NULL, Config::optReadWrite);
	holder.addDetail("CPH_RtfRds/Session", "enableMD5", &CPHRTFRDSCfg::enableMD5, "1", Config::optReadWrite);
	holder.addDetail("CPH_RtfRds/Session", "enableCacheModeForIndex", &CPHRTFRDSCfg::enableCacheForIndex, "1", Config::optReadWrite);
	holder.addDetail("CPH_RtfRds/Session", "enableRAID1ForIndex", &CPHRTFRDSCfg::enableRAID1ForIndex, "1", Config::optReadWrite);
	holder.addDetail("CPH_RtfRds/Session", "localNetworkInterface", (PMem_CharArray)&CPHRTFRDSCfg::szLocalNetIf, sizeof(holder.szLocalNetIf), "", Config::optReadOnly);
	holder.addDetail("CPH_RtfRds/Session", "ftpPassiveMode", &CPHRTFRDSCfg::enableFtpPassiveMode, "0", Config::optReadWrite);
	holder.addDetail("CPH_RtfRds/Session", "ftpConnectionTimeout", &CPHRTFRDSCfg::ftpConnectionInterval, "30000", Config::optReadWrite);

	holder.addDetail("CPH_RtfRds/PacedIndex", "cacheDir", (PMem_CharArray)&CPHRTFRDSCfg::szCacheDir, sizeof(holder.szCacheDir), NULL, Config::optReadOnly);	
	holder.addDetail("CPH_RtfRds/PacedIndex", "enableTrace", &CPHRTFRDSCfg::enablePacingTrace, "0", Config::optReadWrite);		
	holder.addDetail("CPH_RtfRds/PacedIndex", "traceIndexWrite", &CPHRTFRDSCfg::traceIndexWrite, "0", Config::optReadWrite);		
		

	holder.addDetail("CPH_RtfRds/Encode", "urlEncode", (PMem_CharArray)&CPHRTFRDSCfg::szUrlEncode, sizeof(holder.szUrlEncode), NULL, Config::optReadOnly);	
	holder.addDetail("CPH_RtfRds/Encode", "decodePecentOfSourceURL", &CPHRTFRDSCfg::decodeSourceURL, NULL, Config::optReadOnly);

	holder.addDetail("CPH_RtfRds/ProvisionMethod/Method",&CPHRTFRDSCfg::readMethod,&CPHRTFRDSCfg::registerNothing);

    holder.addDetail("CPH_RtfRds/TrickGeneration", "maxSessionNum", &CPHRTFRDSCfg::rtfMaxSessionNum, NULL, Config::optReadOnly);
    holder.addDetail("CPH_RtfRds/TrickGeneration", "maxInputBufferBytes", &CPHRTFRDSCfg::rtfMaxInputBufferBytes, NULL, Config::optReadWrite);
    holder.addDetail("CPH_RtfRds/TrickGeneration", "maxInputBuffersPerSession", &CPHRTFRDSCfg::rtfMaxInputBuffersPerSession, NULL, Config::optReadOnly);
    holder.addDetail("CPH_RtfRds/TrickGeneration", "sessionFailThreshold", &CPHRTFRDSCfg::rtfSessionFailThreshold, NULL, Config::optReadWrite);
	holder.addDetail("CPH_RtfRds/TrickGeneration", "legacyAudioOnly", &CPHRTFRDSCfg::enableLegacyAudioOnly, "0", Config::optReadWrite);

    holder.addDetail("CPH_RtfRds/Vstream", "BWMgrClientId", &CPHRTFRDSCfg::vstrmBwClientId, NULL, Config::optReadWrite);
    holder.addDetail("CPH_RtfRds/Vstream", "disableBufDrvThrottle", &CPHRTFRDSCfg::vstrmDisableBufDrvThrottle, NULL, Config::optReadWrite);

	holder.addDetail("CPH_RtfRds/FtpPortRange",&CPHRTFRDSCfg::readRange,&CPHRTFRDSCfg::registerNothing);
	//holder.addDetail("CPH_RtfRds/FtpPortRange", "min", &CPHRTFRDSCfg::portMin, "15000", Config::optReadWrite);
	//holder.addDetail("CPH_RtfRds/FtpPortRange", "max", &CPHRTFRDSCfg::portMax, "15200", Config::optReadWrite);

    holder.addDetail("CPH_RtfRds/Event/Progress", "enable", &CPHRTFRDSCfg::enableProgEvent, NULL, Config::optReadWrite);
    holder.addDetail("CPH_RtfRds/Event/Streamable", "enable", &CPHRTFRDSCfg::enableStreamEvent, NULL, Config::optReadWrite);
    holder.addDetail("CPH_RtfRds/Event/Streamable", "lagAfterStart", &CPHRTFRDSCfg::streamReqSecs, NULL, Config::optReadWrite);

	holder.addDetail("CPH_RtfRds/NtfsOutputMode", "enable", &CPHRTFRDSCfg::enableTestNTFS, NULL, Config::optReadWrite);
	holder.addDetail("CPH_RtfRds/NtfsOutputMode", "homeDir", (PMem_CharArray)&CPHRTFRDSCfg::szNTFSOutputDir, sizeof(holder.szNTFSOutputDir), NULL, Config::optReadOnly);

	holder.addDetail("CPH_RtfRds/Augmentation", "pids", &CPHRTFRDSCfg::strAugmentationPids, NULL, Config::optReadWrite);

	holder.addDetail("CPH_RtfRds/UnifiedTrickFile",&CPHRTFRDSCfg::readUnifiedTrickFile,&CPHRTFRDSCfg::registerNothing, Config::Range(0, 1));

}

void CPHRTFRDSCfg::readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<Method> methodholder("name");
	methodholder.read(node, hPP);
	methods.push_back(methodholder);
}
void CPHRTFRDSCfg::readRange( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
{
	ZQ::common::Config::Holder<PortRange> rangeholder;
	rangeholder.read(node, hPP);
	ranges.push_back(rangeholder);
}
void CPHRTFRDSCfg::readUnifiedTrickFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<UnifiedTrickFile> unifiedholder("");
	unifiedholder.read(node, hPP);
	unifiedtrickfile = unifiedholder;
}