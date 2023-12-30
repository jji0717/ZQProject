
#include "CPH_NPVRCfg.h"

ZQ::common::Config::Loader<NPVRConfig> _gCPHCfg("CPH_NPVR.xml");


NPVRConfig::NPVRConfig()
{
	memset(szCacheDir, 0, sizeof(szCacheDir));
	memset(szlocalIp, 0, sizeof(szlocalIp));
	memset(szNTFSOutputDir, 0, sizeof(szNTFSOutputDir));
	memset(szNTFSSource, 0, sizeof(szNTFSOutputDir));
	enableTestNTFS = 0;
	mediaSampleSize=64*1024;		//64K for vstrm
	timeoutInterval = 5;  //5 seconds
	streamReqSecs = 5;				//5 seconds
	enableProgEvent = 1;
	enableStreamEvent = 1;

	rtfMaxInputBufferBytes = 64*1024;	
	rtfMaxInputBuffersPerSession = 100;
	rtfSessionFailThreshold = 0;

	szDumpPath[0] = '\0';
	enableDump = 0;
	deleteDumpOnSuccess = 1;
    
	enablePacingTrace = 1;

	enableNtfsSource = 0;
	monitorInterval = 30000; //30 seconds
	leadsesslagAfterIdle = 10000; //10 seconds
	progressSendInterval = 10000; //10 seonds

	delayDataNotify = 300;			//milliseconds
	preloadTime = 5000;
}

void NPVRConfig::structure(ZQ::common::Config::Holder<NPVRConfig> &holder)
{
	using namespace ZQ::common::Config;
	typedef ZQ::common::Config::Holder<NPVRConfig>::PMem_CharArray PMem_CharArray;

	//holder.addDetail("CPH_NPVR/Capture", "localIp", (PMem_CharArray)&NPVRConfig::szlocalIp, sizeof(holder.szlocalIp), "", optReadOnly);
	holder.addDetail("CPH_NPVR/Capture", "sessionTimeout", &NPVRConfig::timeoutInterval, "30", optReadWrite);
	holder.addDetail("CPH_NPVR/Capture", "winpcapKernelBuffer", &NPVRConfig::winpcapKernelBufferInMB, "256", optReadOnly);
	holder.addDetail("CPH_NPVR/Capture", "winpcapMinBufferCopy", &NPVRConfig::winpcapMinBufferToCopyInKB, "1024", optReadOnly);
	holder.addDetail("CPH_NPVR/Capture/NetworkInterface", &NPVRConfig::readNetInterface,&NPVRConfig::registerNothing);

	//	holder.addDetail("CPH_NPVR/Capture", "delayDataNotify", &NPVRConfig::delayDataNotify, "0", optReadWrite);

	holder.addDetail("CPH_NPVR/TrickGeneration", "maxSessionNum", &NPVRConfig::rtfMaxSessionNum, NULL, optReadOnly);
	holder.addDetail("CPH_NPVR/TrickGeneration", "maxInputBufferBytes", &NPVRConfig::rtfMaxInputBufferBytes, NULL, optReadOnly);
	holder.addDetail("CPH_NPVR/TrickGeneration", "maxInputBuffersPerSession", &NPVRConfig::	rtfMaxInputBuffersPerSession, NULL, optReadOnly);
	holder.addDetail("CPH_NPVR/TrickGeneration", "sessionFailThreshold", &NPVRConfig::rtfSessionFailThreshold, NULL, optReadOnly);
	holder.addDetail("CPH_NPVR/Vstream", "BWMgrClientId", &NPVRConfig::vstrmBwClientId, NULL, optReadOnly);
	holder.addDetail("CPH_NPVR/Vstream", "disableBufDrvThrottle", &NPVRConfig::vstrmDisableBufDrvThrottle, NULL, optReadOnly);

	holder.addDetail("CPH_NPVR/Event/Progress", "enable", &NPVRConfig::enableProgEvent, "1", optReadOnly);
	holder.addDetail("CPH_NPVR/Event/Progress", "interval", &NPVRConfig::progressSendInterval, "6000", optReadOnly);
	holder.addDetail("CPH_NPVR/Event/Streamable", "enable", &NPVRConfig::enableStreamEvent, "1", optReadOnly);
	holder.addDetail("CPH_NPVR/Event/Streamable", "lagAfterStart", &NPVRConfig::streamReqSecs, NULL, optReadOnly);

	holder.addDetail("CPH_NPVR/CaptureDumper", "enable",  &NPVRConfig::enableDump, NULL, optReadWrite);
	holder.addDetail("CPH_NPVR/CaptureDumper", "dumpPath", (PMem_CharArray)&NPVRConfig::szDumpPath, sizeof(holder.szDumpPath), NULL, optReadOnly);
	holder.addDetail("CPH_NPVR/CaptureDumper", "deleteOnSuccess", &NPVRConfig::deleteDumpOnSuccess, NULL, optReadWrite);

	holder.addDetail("CPH_NPVR/PacedIndex", "cacheDir", (PMem_CharArray)&NPVRConfig::szCacheDir, sizeof(holder.szCacheDir), "", optReadOnly);
	holder.addDetail("CPH_NPVR/PacedIndex", "enableTrace", &NPVRConfig::enablePacingTrace, NULL, optReadOnly);


    holder.addDetail("CPH_NPVR/Session", "enableMD5", &NPVRConfig::enableMD5, "0", optReadWrite);
	holder.addDetail("CPH_NPVR/Session", "enableCacheModeForIndex", &NPVRConfig::enableCacheForIndex, "1", optReadWrite);
	holder.addDetail("CPH_NPVR/Session", "enableRAID1ForIndex", &NPVRConfig::enableRAID1ForIndex, "1", optReadWrite);
	holder.addDetail("CPH_NPVR/Session", "preLoad", &NPVRConfig::preloadTime, "10000", optReadWrite);

    holder.addDetail("CPH_NPVR/MediaSample", "bufferSize", &NPVRConfig::mediaSampleSize, NULL, optReadOnly);

	holder.addDetail("CPH_NPVR/NtfsOutputMode", "enable", &NPVRConfig::enableTestNTFS, NULL, optReadOnly);
	holder.addDetail("CPH_NPVR/NtfsOutputMode", "homeDir", (PMem_CharArray)&NPVRConfig::szNTFSOutputDir, sizeof(holder.szNTFSOutputDir), NULL, optReadOnly);

	holder.addDetail("CPH_NPVR/LeadSession", "maxSessions", &NPVRConfig::maxLeadsessionNum, NULL, optReadOnly);
	holder.addDetail("CPH_NPVR/LeadSession", "lagAfterIdle", &NPVRConfig::leadsesslagAfterIdle, NULL, optReadOnly);
	holder.addDetail("CPH_NPVR/LeadSession", "monitorInterval", &NPVRConfig::monitorInterval, NULL, optReadOnly);
	holder.addDetail("CPH_NPVR/NtfsSourceTest", "enable", &NPVRConfig::enableNtfsSource, NULL, optReadOnly);
	holder.addDetail("CPH_NPVR/NtfsSourceTest", "sourceFile", (PMem_CharArray)&NPVRConfig::szNTFSSource, sizeof(holder.szNTFSSource), NULL, optReadOnly);

	holder.addDetail("CPH_NPVR/ProvisionMethod/Method",&NPVRConfig::readMethod,&NPVRConfig::registerNothing);

}

void NPVRConfig::readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<Method> methodholder("name");
	methodholder.read(node, hPP);
	methods.push_back(methodholder);
}

void NPVRConfig::readNetInterface(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<NetInterface> interfaceholder("ip");
	interfaceholder.read(node, hPP);
	nInterface.push_back(interfaceholder);
}