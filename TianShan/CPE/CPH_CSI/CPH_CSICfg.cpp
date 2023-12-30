
#include "CPH_CSICfg.h"

using namespace ZQ::common;

ZQ::common::Config::Loader<CPHCSICfg> _gCPHCfg("CPH_CSI.xml");

CPHCSICfg::CPHCSICfg()
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

CPHCSICfg::~CPHCSICfg()
{

}

void CPHCSICfg::structure(ZQ::common::Config::Holder<CPHCSICfg> &holder)
{
    using namespace ZQ::common;
    typedef Config::Holder<CPHCSICfg>::PMem_CharArray PMem_CharArray;
	
    holder.addDetail("CPH_CSI/MediaSample", "bufferSize", &CPHCSICfg::mediaSampleSize, NULL, Config::optReadOnly);
	holder.addDetail("CPH_CSI/MediaSample", "maxAllocSampleCount", &CPHCSICfg::maxAllocSampleCount, "400", Config::optReadOnly);
    holder.addDetail("CPH_CSI/Session", "overSpeedRate", &CPHCSICfg::bandwidthLimitRate, NULL, Config::optReadWrite);
	holder.addDetail("CPH_CSI/Session", "enableMD5", &CPHCSICfg::enableMD5, "1", Config::optReadWrite);
	holder.addDetail("CPH_CSI/Session", "enableCacheModeForIndex", &CPHCSICfg::enableCacheForIndex, "1", Config::optReadWrite);
	holder.addDetail("CPH_CSI/Session", "enableRAID1ForIndex", &CPHCSICfg::enableRAID1ForIndex, "1", Config::optReadWrite);
	holder.addDetail("CPH_CSI/Session", "localNetworkInterface", (PMem_CharArray)&CPHCSICfg::szLocalNetIf, sizeof(holder.szLocalNetIf), "", Config::optReadOnly);
	holder.addDetail("CPH_CSI/Session", "ftpPassiveMode", &CPHCSICfg::enableFtpPassiveMode, "0", Config::optReadWrite);
	holder.addDetail("CPH_CSI/Session", "ftpConnectionTimeout", &CPHCSICfg::ftpConnectionInterval, "30000", Config::optReadWrite);

	holder.addDetail("CPH_CSI/Capture/NetworkInterface", &CPHCSICfg::readNetInterface,&CPHCSICfg::registerNothing);
	holder.addDetail("CPH_CSI/Capture", "sessionTimeout", &CPHCSICfg::timeoutInterval, NULL, Config::optReadOnly);
	holder.addDetail("CPH_CSI/Capture", "winpcapKernelBuffer", &CPHCSICfg::winpcapKernelBufferInMB, "128", Config::optReadOnly);
	holder.addDetail("CPH_CSI/Capture", "winpcapMinBufferCopy", &CPHCSICfg::winpcapMinBufferToCopyInKB, "512", Config::optReadOnly);

	holder.addDetail("CPH_CSI/CaptureDumper", "enable",  &CPHCSICfg::enableDump, NULL, Config::optReadWrite);
	holder.addDetail("CPH_CSI/CaptureDumper", "dumpPath", (PMem_CharArray)&CPHCSICfg::szDumpPath, sizeof(holder.szDumpPath), NULL, Config::optReadOnly);
	holder.addDetail("CPH_CSI/CaptureDumper", "deleteOnSuccess", &CPHCSICfg::deleteDumpOnSuccess, NULL, Config::optReadWrite);

	holder.addDetail("CPH_CSI/PacedIndex", "cacheDir", (PMem_CharArray)&CPHCSICfg::szCacheDir, sizeof(holder.szCacheDir), NULL, Config::optReadOnly);	
	holder.addDetail("CPH_CSI/PacedIndex", "enableTrace", &CPHCSICfg::enablePacingTrace, "0", Config::optReadWrite);		
	holder.addDetail("CPH_CSI/PacedIndex", "traceIndexWrite", &CPHCSICfg::traceIndexWrite, "0", Config::optReadWrite);		
		

	holder.addDetail("CPH_CSI/Encode", "urlEncode", (PMem_CharArray)&CPHCSICfg::szUrlEncode, sizeof(holder.szUrlEncode), NULL, Config::optReadOnly);	
	holder.addDetail("CPH_CSI/Encode", "decodePecentOfSourceURL", &CPHCSICfg::decodeSourceURL, NULL, Config::optReadOnly);

	holder.addDetail("CPH_CSI/ProvisionMethod/Method",&CPHCSICfg::readMethod,&CPHCSICfg::registerNothing);

    holder.addDetail("CPH_CSI/TrickGeneration", "maxSessionNum", &CPHCSICfg::rtfMaxSessionNum, NULL, Config::optReadOnly);
    holder.addDetail("CPH_CSI/TrickGeneration", "maxInputBufferBytes", &CPHCSICfg::rtfMaxInputBufferBytes, NULL, Config::optReadWrite);
    holder.addDetail("CPH_CSI/TrickGeneration", "maxInputBuffersPerSession", &CPHCSICfg::rtfMaxInputBuffersPerSession, NULL, Config::optReadOnly);
    holder.addDetail("CPH_CSI/TrickGeneration", "sessionFailThreshold", &CPHCSICfg::rtfSessionFailThreshold, NULL, Config::optReadWrite);
	holder.addDetail("CPH_CSI/TrickGeneration", "legacyAudioOnly", &CPHCSICfg::enableLegacyAudioOnly, "0", Config::optReadWrite);

    holder.addDetail("CPH_CSI/Vstream", "BWMgrClientId", &CPHCSICfg::vstrmBwClientId, NULL, Config::optReadWrite);
    holder.addDetail("CPH_CSI/Vstream", "disableBufDrvThrottle", &CPHCSICfg::vstrmDisableBufDrvThrottle, NULL, Config::optReadWrite);

	holder.addDetail("CPH_CSI/FtpPortRange",&CPHCSICfg::readRange,&CPHCSICfg::registerNothing);
	//holder.addDetail("CPH_CSI/FtpPortRange", "min", &CPHCSICfg::portMin, "15000", Config::optReadWrite);
	//holder.addDetail("CPH_CSI/FtpPortRange", "max", &CPHCSICfg::portMax, "15200", Config::optReadWrite);

    holder.addDetail("CPH_CSI/Event/Progress", "enable", &CPHCSICfg::enableProgEvent, NULL, Config::optReadWrite);
    holder.addDetail("CPH_CSI/Event/Streamable", "enable", &CPHCSICfg::enableStreamEvent, NULL, Config::optReadWrite);
    holder.addDetail("CPH_CSI/Event/Streamable", "lagAfterStart", &CPHCSICfg::streamReqSecs, NULL, Config::optReadWrite);

	holder.addDetail("CPH_CSI/NtfsOutputMode", "enable", &CPHCSICfg::enableTestNTFS, NULL, Config::optReadWrite);
	holder.addDetail("CPH_CSI/NtfsOutputMode", "homeDir", (PMem_CharArray)&CPHCSICfg::szNTFSOutputDir, sizeof(holder.szNTFSOutputDir), NULL, Config::optReadOnly);

	holder.addDetail("CPH_CSI/Augmentation", "pids", &CPHCSICfg::strAugmentationPids, NULL, Config::optReadWrite);

	holder.addDetail("CPH_CSI/UnifiedTrickFile",&CPHCSICfg::readUnifiedTrickFile,&CPHCSICfg::registerNothing, Config::Range(0, 1));

}

void CPHCSICfg::readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<Method> methodholder("name");
	methodholder.read(node, hPP);
	methods.push_back(methodholder);
}
void CPHCSICfg::readRange( ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP )
{
	ZQ::common::Config::Holder<PortRange> rangeholder;
	rangeholder.read(node, hPP);
	ranges.push_back(rangeholder);
}
void CPHCSICfg::readUnifiedTrickFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<UnifiedTrickFile> unifiedholder("");
	unifiedholder.read(node, hPP);
	unifiedtrickfile = unifiedholder;
}

void CPHCSICfg::readNetInterface(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<NetInterface> interfaceholder("ip");
	interfaceholder.read(node, hPP);
	nInterface.push_back(interfaceholder);
}