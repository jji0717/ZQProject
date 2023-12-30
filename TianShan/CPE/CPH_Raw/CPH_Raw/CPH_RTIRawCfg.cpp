#include "CPH_RTIRawCfg.h"
ZQ::common::Config::Loader<RtiRawConfig> _gCPHCfg("CPH_Raw.xml");


RtiRawConfig::RtiRawConfig()
{
	memset(szCacheDir, 0, sizeof(szCacheDir));
	mediaSampleSize=64*1024;		//64K for vstrm
	enableProgEvent = 1;
	enableStreamEvent = 1;
	enablePacingTrace = 0;	
	streamReqSecs = 5;				//5 seconds

	rtfMaxInputBufferBytes = 64*1024;	
	rtfMaxInputBuffersPerSession = 100;
	rtfSessionFailThreshold = 0;
	enableMD5 = 0;

	szDumpPath[0] = '\0';
	enableDump = 0;
	deleteDumpOnSuccess = 1;
	deleteTargetFileCapFail = 1;

	retryCaptureCount = 0;
	warningDiskWriteLongLatency = 100;
}

void RtiRawConfig::structure(ZQ::common::Config::Holder<RtiRawConfig> &holder)
{
	using namespace ZQ::common::Config;
	typedef ZQ::common::Config::Holder<RtiRawConfig>::PMem_CharArray PMem_CharArray;
	//holder.addDetail("CPH_RTIRaw", "localIp", &RtiRawConfig::localIp, "127.0.0.1",optReadOnly);

	holder.addDetail("CPH_RTIRaw/MediaSample", "bufferSize", &RtiRawConfig::mediaSampleSize, NULL, optReadOnly);
	holder.addDetail("CPH_RTIRaw/Session", "enableMD5", &RtiRawConfig::enableMD5, NULL, optReadWrite);
	holder.addDetail("CPH_RTIRaw/Session", "enableCacheModeForIndex", &RtiRawConfig::enableCacheForIndex, "1", optReadWrite);
	holder.addDetail("CPH_RTIRaw/Session", "enableRAID1ForIndex", &RtiRawConfig::enableRAID1ForIndex, "1", optReadWrite);
	holder.addDetail("CPH_RTIRaw/Session", "preLoad", &RtiRawConfig::preloadTime, "10000", optReadWrite);
	holder.addDetail("CPH_RTIRaw/Session", "warningDiskWriteLongLatency", &RtiRawConfig::warningDiskWriteLongLatency, "100", optReadOnly);

	//holder.addDetail("CPH_RTIRaw/Capture/NetworkInterface", &RtiRawConfig::readNetInterface,&RtiRawConfig::registerNothing);
	holder.addDetail("CPH_RTIRaw/Capture","captureIp",&RtiRawConfig::captureIp, NULL,optReadOnly);
	holder.addDetail("CPH_RTIRaw/Capture","bandwidth",&RtiRawConfig::totalBandwidth,"1000000000", optReadOnly);
	holder.addDetail("CPH_RTIRaw/Capture", "sessionTimeout", &RtiRawConfig::timeoutInterval, "30", optReadOnly);
	holder.addDetail("CPH_RTIRaw/Capture","winpcapKernelBuffer",&RtiRawConfig::winpcapKernelBufferInMB,"128", optReadOnly);
	holder.addDetail("CPH_RTIRaw/Capture", "winpcapMinBufferCopy", &RtiRawConfig::winpcapMinBufferToCopyInKB, "512", optReadOnly);
	
	holder.addDetail("CPH_RTIRaw/CaptureDumper", "enable",  &RtiRawConfig::enableDump, NULL, optReadWrite);
	holder.addDetail("CPH_RTIRaw/CaptureDumper", "dumpPath", (PMem_CharArray)&RtiRawConfig::szDumpPath, sizeof(holder.szDumpPath), NULL, optReadOnly);
	holder.addDetail("CPH_RTIRaw/CaptureDumper", "deleteOnSuccess", &RtiRawConfig::deleteDumpOnSuccess, NULL, optReadWrite);

	holder.addDetail("CPH_RTIRaw/Event/Progress", "enable", &RtiRawConfig::enableProgEvent, NULL, optReadOnly);
	holder.addDetail("CPH_RTIRaw/Event/Streamable", "enable", &RtiRawConfig::enableStreamEvent, NULL, optReadOnly);
	holder.addDetail("CPH_RTIRaw/Event/Streamable", "lagAfterStart", &RtiRawConfig::streamReqSecs, NULL, optReadOnly);

	holder.addDetail("CPH_RTIRaw/ProvisionMethod/Method",&RtiRawConfig::readMethod,&RtiRawConfig::registerNothing);
}

void RtiRawConfig::readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<Method> methodholder("name");
	methodholder.read(node, hPP);
	methods.push_back(methodholder);
}

/*void RtiRawConfig::readNetInterface(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<NetInterface> interfaceholder("ip");
	interfaceholder.read(node, hPP);
	nInterface.push_back(interfaceholder);
}
*/