
#include "CPH_PCAPCfg.h"

ZQ::common::Config::Loader<PCAPConfig> _gCPHCfg("CPH_PCAP.xml");


PCAPConfig::PCAPConfig()
{
	mediaSampleSize=64*1024;		//64K for vstrm
	maxAllocSampleCount = 400;
	enableProgEvent = 1;
	enableStreamEvent = 1;
	streamReqSecs = 5;				//5 seconds

	//maxBandwidthKBps = 100*1024;	//100mbps
	//maxSessionNum = 30;
	rtfMaxInputBufferBytes = 64*1024;	
	rtfMaxInputBuffersPerSession = 100;
	rtfSessionFailThreshold = 0;
	enableMD5 = 0;
	enablePacing = 0;

	szDumpPath[0] = '\0';
	enableDump = 0;
	deleteDumpOnSuccess = 1;
	deleteTargetFileCapFail = 1;
	memset(szPaceDllPath,0,sizeof(szPaceDllPath));
	
	retryCaptureCount = 0;
	unifiedtrickfile.enable = 1;
	ciscofileext.mode = 0;
	warningDiskWriteLongLatency = 100;
}

void PCAPConfig::structure(ZQ::common::Config::Holder<PCAPConfig> &holder)
{
	using namespace ZQ::common::Config;
	typedef ZQ::common::Config::Holder<PCAPConfig>::PMem_CharArray PMem_CharArray;
	holder.addDetail("CPH_PCAP/MediaSample", "bufferSize", &PCAPConfig::mediaSampleSize, NULL, optReadOnly);
	holder.addDetail("CPH_PCAP/MediaSample", "maxAllocSampleCount", &PCAPConfig::maxAllocSampleCount, "400", optReadOnly);
	holder.addDetail("CPH_PCAP/Session", "enableMD5", &PCAPConfig::enableMD5, NULL, optReadWrite);
	holder.addDetail("CPH_PCAP/Session", "enableCacheModeForIndex", &PCAPConfig::enableCacheForIndex, "1", optReadWrite);
	holder.addDetail("CPH_PCAP/Session", "preLoad", &PCAPConfig::preloadTime, "10000", optReadWrite);
	holder.addDetail("CPH_PCAP/Session", "warningDiskWriteLongLatency", &PCAPConfig::warningDiskWriteLongLatency, "100", optReadOnly);

	holder.addDetail("CPH_PCAP/Capture/NetworkInterface", &PCAPConfig::readNetInterface,&PCAPConfig::registerNothing);
	holder.addDetail("CPH_PCAP/Capture", "sessionTimeout", &PCAPConfig::timeoutInterval, NULL, optReadOnly);
	holder.addDetail("CPH_PCAP/Capture", "winpcapKernelBuffer", &PCAPConfig::winpcapKernelBufferInMB, "128", optReadOnly);
	holder.addDetail("CPH_PCAP/Capture", "winpcapMinBufferCopy", &PCAPConfig::winpcapMinBufferToCopyInKB, "512", optReadOnly);

	holder.addDetail("CPH_PCAP/CaptureDumper", "enable",  &PCAPConfig::enableDump, NULL, optReadWrite);
	holder.addDetail("CPH_PCAP/CaptureDumper", "dumpPath", (PMem_CharArray)&PCAPConfig::szDumpPath, sizeof(holder.szDumpPath), NULL, optReadOnly);
	holder.addDetail("CPH_PCAP/CaptureDumper", "deleteOnSuccess", &PCAPConfig::deleteDumpOnSuccess, NULL, optReadWrite);
	holder.addDetail("CPH_PCAP/CaptureDumper", "deleteTargetFileCapFail", &PCAPConfig::deleteTargetFileCapFail, "1", optReadWrite);

	holder.addDetail("CPH_PCAP/PacedIndex", "enablePacing", &PCAPConfig::enablePacing, "0", optReadWrite);
	holder.addDetail("CPH_PCAP/PacedIndex", "pacedFile", (PMem_CharArray)&PCAPConfig::szPaceDllPath, sizeof(holder.szPaceDllPath), NULL, optReadOnly);	

	holder.addDetail("CPH_PCAP/TrickGeneration", "maxSessionNum", &PCAPConfig::rtfMaxSessionNum, NULL, optReadOnly);
	holder.addDetail("CPH_PCAP/TrickGeneration", "maxInputBufferBytes", &PCAPConfig::rtfMaxInputBufferBytes, NULL, optReadOnly);
	holder.addDetail("CPH_PCAP/TrickGeneration", "maxInputBuffersPerSession", &PCAPConfig::	rtfMaxInputBuffersPerSession, NULL, optReadOnly);
	holder.addDetail("CPH_PCAP/TrickGeneration", "sessionFailThreshold", &PCAPConfig::rtfSessionFailThreshold, NULL, optReadOnly);
	holder.addDetail("CPH_PCAP/Event/Progress", "enable", &PCAPConfig::enableProgEvent, NULL, optReadOnly);
	holder.addDetail("CPH_PCAP/Event/Streamable", "enable", &PCAPConfig::enableStreamEvent, NULL, optReadOnly);
	holder.addDetail("CPH_PCAP/Event/Streamable", "lagAfterStart", &PCAPConfig::streamReqSecs, NULL, optReadOnly);
	holder.addDetail("CPH_PCAP/NtfsOutputMode", "enable", &PCAPConfig::enableTestNTFS, NULL, optReadOnly);
	holder.addDetail("CPH_PCAP/NtfsOutputMode", "homeDir", (PMem_CharArray)&PCAPConfig::szNTFSOutputDir, sizeof(holder.szNTFSOutputDir), NULL, optReadOnly);

	holder.addDetail("CPH_PCAP/ProvisionMethod/Method",&PCAPConfig::readMethod,&PCAPConfig::registerNothing);
		
  holder.addDetail("CPH_PCAP/RetryCapture", "retrycount", &PCAPConfig::retryCaptureCount, NULL, optReadOnly);
  holder.addDetail("CPH_PCAP/UnifiedTrickFile",&PCAPConfig::readUnifiedTrickFile,&PCAPConfig::registerNothing, ZQ::common::Config::Range(0, 1));
  holder.addDetail("CPH_PCAP/FileExtName",&PCAPConfig::readCiscoFileExt,&PCAPConfig::registerNothing, ZQ::common::Config::Range(0, 1)); 
  holder.addDetail("CPH_PCAP/DirectIO",&PCAPConfig::readDirectIO,&PCAPConfig::registerNothing, ZQ::common::Config::Range(0, 1)); 
  holder.addDetail("CPH_PCAP/DiskWrite",&PCAPConfig::readFileSync,&PCAPConfig::registerNothing, ZQ::common::Config::Range(0, 1)); 

}

void PCAPConfig::readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<Method> methodholder("name");
	methodholder.read(node, hPP);
	methods.push_back(methodholder);
}

void PCAPConfig::readNetInterface(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<NetInterface> interfaceholder("ip");
	interfaceholder.read(node, hPP);
	nInterface.push_back(interfaceholder);
}

void PCAPConfig::readUnifiedTrickFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<UnifiedTrickFile> unifiedholder("");
	unifiedholder.read(node, hPP);
	unifiedtrickfile = unifiedholder;
}

void PCAPConfig::readCiscoFileExt(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<CiscoFileExt> ciscofileextholder("");
	ciscofileextholder.read(node, hPP);
	ciscofileext = ciscofileextholder;
}

void PCAPConfig::readDirectIO(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<DirectIO> directIOholder("");
	directIOholder.read(node, hPP);
	directIO = directIOholder;
}

void PCAPConfig::readFileSync(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<FileSync> filesyncholder("");
	filesyncholder.read(node, hPP);
	filesync = filesyncholder;
}
