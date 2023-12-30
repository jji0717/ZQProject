
#include "CPH_RTICfg.h"

ZQ::common::Config::Loader<RtiConfig> _gCPHCfg("CPH_RTI.xml");


RtiConfig::RtiConfig()
{
	memset(szCacheDir, 0, sizeof(szCacheDir));
	mediaSampleSize=64*1024;		//64K for vstrm
	maxAllocSampleCount = 400;
	enableProgEvent = 1;
	enableStreamEvent = 1;
	enablePacingTrace = 0;	
	streamReqSecs = 5;				//5 seconds

	//maxBandwidthKBps = 100*1024;	//100mbps
	//maxSessionNum = 30;
	rtfMaxInputBufferBytes = 64*1024;	
	rtfMaxInputBuffersPerSession = 100;
	rtfSessionFailThreshold = 0;
	enableMD5 = 0;

	szDumpPath[0] = '\0';
	enableDump = 0;
	deleteDumpOnSuccess = 1;
	deleteTargetFileCapFail = 1;

	retryCaptureCount = 0;

	unifiedtrickfile.enable = 1;
	warningDiskWriteLongLatency = 100;
}

void RtiConfig::structure(ZQ::common::Config::Holder<RtiConfig> &holder)
{
	using namespace ZQ::common::Config;
	typedef ZQ::common::Config::Holder<RtiConfig>::PMem_CharArray PMem_CharArray;
	//holder.addDetail("CPH_RTI", "maxSessions", &RtiConfig::maxSessionNum, NULL, optReadOnly);
	//holder.addDetail("CPH_RTI", "maxBandwidth", &RtiConfig::maxBandwidthKBps, NULL, optReadOnly);
	holder.addDetail("CPH_RTI/MediaSample", "bufferSize", &RtiConfig::mediaSampleSize, NULL, optReadOnly);
	holder.addDetail("CPH_RTI/MediaSample", "maxAllocSampleCount", &RtiConfig::maxAllocSampleCount, "400", optReadOnly);
	holder.addDetail("CPH_RTI/Session", "enableMD5", &RtiConfig::enableMD5, NULL, optReadWrite);
	holder.addDetail("CPH_RTI/Session", "enableCacheModeForIndex", &RtiConfig::enableCacheForIndex, "1", optReadWrite);
  holder.addDetail("CPH_RTI/Session", "enableRAID1ForIndex", &RtiConfig::enableRAID1ForIndex, "1", optReadWrite);
	holder.addDetail("CPH_RTI/Session", "preLoad", &RtiConfig::preloadTime, "10000", optReadWrite);
	holder.addDetail("CPH_RTI/Session", "warningDiskWriteLongLatency", &RtiConfig::warningDiskWriteLongLatency, "100", optReadOnly);

	holder.addDetail("CPH_RTI/Capture/NetworkInterface", &RtiConfig::readNetInterface,&RtiConfig::registerNothing);
	holder.addDetail("CPH_RTI/Capture", "sessionTimeout", &RtiConfig::timeoutInterval, NULL, optReadOnly);
	holder.addDetail("CPH_RTI/Capture", "winpcapKernelBuffer", &RtiConfig::winpcapKernelBufferInMB, "128", optReadOnly);
	holder.addDetail("CPH_RTI/Capture", "winpcapMinBufferCopy", &RtiConfig::winpcapMinBufferToCopyInKB, "512", optReadOnly);

	holder.addDetail("CPH_RTI/CaptureDumper", "enable",  &RtiConfig::enableDump, NULL, optReadWrite);
	holder.addDetail("CPH_RTI/CaptureDumper", "dumpPath", (PMem_CharArray)&RtiConfig::szDumpPath, sizeof(holder.szDumpPath), NULL, optReadOnly);
	holder.addDetail("CPH_RTI/CaptureDumper", "deleteOnSuccess", &RtiConfig::deleteDumpOnSuccess, NULL, optReadWrite);

	holder.addDetail("CPH_RTI/TrickGeneration", "maxSessionNum", &RtiConfig::rtfMaxSessionNum, NULL, optReadOnly);
	holder.addDetail("CPH_RTI/TrickGeneration", "maxInputBufferBytes", &RtiConfig::rtfMaxInputBufferBytes, NULL, optReadOnly);
	holder.addDetail("CPH_RTI/TrickGeneration", "maxInputBuffersPerSession", &RtiConfig::	rtfMaxInputBuffersPerSession, NULL, optReadOnly);
	holder.addDetail("CPH_RTI/TrickGeneration", "sessionFailThreshold", &RtiConfig::rtfSessionFailThreshold, NULL, optReadOnly);
	holder.addDetail("CPH_RTI/Event/Progress", "enable", &RtiConfig::enableProgEvent, NULL, optReadOnly);
	holder.addDetail("CPH_RTI/Event/Streamable", "enable", &RtiConfig::enableStreamEvent, NULL, optReadOnly);
	holder.addDetail("CPH_RTI/Event/Streamable", "lagAfterStart", &RtiConfig::streamReqSecs, NULL, optReadOnly);
	holder.addDetail("CPH_RTI/PacedIndex", "cacheDir", (PMem_CharArray)&RtiConfig::szCacheDir, sizeof(holder.szCacheDir), "", optReadOnly);
	holder.addDetail("CPH_RTI/PacedIndex", "enableTrace", &RtiConfig::enablePacingTrace, NULL, optReadOnly);
	holder.addDetail("CPH_RTI/Vstream", "BWMgrClientId", &RtiConfig::vstrmBwClientId, NULL, optReadOnly);
	holder.addDetail("CPH_RTI/Vstream", "disableBufDrvThrottle", &RtiConfig::vstrmDisableBufDrvThrottle, NULL, optReadOnly);
	holder.addDetail("CPH_RTI/NtfsOutputMode", "enable", &RtiConfig::enableTestNTFS, NULL, optReadOnly);
	holder.addDetail("CPH_RTI/NtfsOutputMode", "homeDir", (PMem_CharArray)&RtiConfig::szNTFSOutputDir, sizeof(holder.szNTFSOutputDir), NULL, optReadOnly);
	holder.addDetail("CPH_RTI/RetryCapture", "retrycount", &RtiConfig::retryCaptureCount, NULL, optReadOnly);

	holder.addDetail("CPH_RTI/ProvisionMethod/Method",&RtiConfig::readMethod,&RtiConfig::registerNothing);
	holder.addDetail("CPH_RTI/UnifiedTrickFile",&RtiConfig::readUnifiedTrickFile,&RtiConfig::registerNothing, ZQ::common::Config::Range(0, 1));
}

void RtiConfig::readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<Method> methodholder("name");
	methodholder.read(node, hPP);
	methods.push_back(methodholder);
}

void RtiConfig::readNetInterface(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<NetInterface> interfaceholder("ip");
	interfaceholder.read(node, hPP);
	nInterface.push_back(interfaceholder);
}
void RtiConfig::readUnifiedTrickFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<UnifiedTrickFile> unifiedholder("");
	unifiedholder.read(node, hPP);
	unifiedtrickfile = unifiedholder;
}