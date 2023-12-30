
#include "CPH_RTINasCfg.h"

using namespace ZQ::common;

ZQ::common::Config::Loader<CPHRTINasCfg> _gCPHCfg("CPH_RTINAS.xml");

CPHRTINasCfg::CPHRTINasCfg()
{
	memset(szCacheDir, 0, sizeof(szCacheDir));
	mediaSampleSize=64*1024;		//64K for vstrm
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


	szDumpPath[0] = '\0';
	enableDump = 0;
	deleteDumpOnSuccess = 1;
	enableCacheForIndex = 0;
}

CPHRTINasCfg::~CPHRTINasCfg()
{

}

void CPHRTINasCfg::structure(ZQ::common::Config::Holder<CPHRTINasCfg> &holder)
{
    using namespace ZQ::common;
    typedef Config::Holder<CPHRTINasCfg>::PMem_CharArray PMem_CharArray;
	
	//holder.addDetail("CPH_RTINas", "maxSessions", &CPHRTINasCfg::maxSessionNum, NULL, Config::optReadWrite);
	//holder.addDetail("CPH_RTINas", "maxBandwidth", &CPHRTINasCfg::maxBandwidthKBps, NULL, Config::optReadWrite);	

    holder.addDetail("CPH_RTINas/MediaSample", "bufferSize", &CPHRTINasCfg::mediaSampleSize, NULL, Config::optReadOnly);
	holder.addDetail("CPH_RTINas/Session", "enableMD5", &CPHRTINasCfg::enableMD5, NULL, Config::optReadWrite);
	holder.addDetail("CPH_RTINas/Session", "enableCacheModeForIndex", &CPHRTINasCfg::enableCacheForIndex, "0", Config::optReadWrite);
	holder.addDetail("CPH_RTINas/Session", "preLoad", &CPHRTINasCfg::preloadTime, "10000", Config::optReadWrite);

	holder.addDetail("CPH_RTINas/PacedIndex", "cacheDir", (PMem_CharArray)&CPHRTINasCfg::szCacheDir, sizeof(holder.szCacheDir), NULL, Config::optReadOnly);	
	holder.addDetail("CPH_RTINas/PacedIndex", "enableTrace", &CPHRTINasCfg::enablePacingTrace, NULL, Config::optReadWrite);		

    holder.addDetail("CPH_RTINas/TrickGeneration", "maxSessionNum", &CPHRTINasCfg::rtfMaxSessionNum, NULL, Config::optReadOnly);
    holder.addDetail("CPH_RTINas/TrickGeneration", "maxInputBufferBytes", &CPHRTINasCfg::rtfMaxInputBufferBytes, NULL, Config::optReadWrite);
    holder.addDetail("CPH_RTINas/TrickGeneration", "maxInputBuffersPerSession", &CPHRTINasCfg::rtfMaxInputBuffersPerSession, NULL, Config::optReadOnly);
    holder.addDetail("CPH_RTINas/TrickGeneration", "sessionFailThreshold", &CPHRTINasCfg::rtfSessionFailThreshold, NULL, Config::optReadWrite);

    holder.addDetail("CPH_RTINas/Event/Progress", "enable", &CPHRTINasCfg::enableProgEvent, NULL, Config::optReadWrite);
    holder.addDetail("CPH_RTINas/Event/Streamable", "enable", &CPHRTINasCfg::enableStreamEvent, NULL, Config::optReadWrite);
    holder.addDetail("CPH_RTINas/Event/Streamable", "lagAfterStart", &CPHRTINasCfg::streamReqSecs, NULL, Config::optReadWrite);

	holder.addDetail("CPH_RTINas/Capture", "localIp", (PMem_CharArray)&CPHRTINasCfg::szlocalIp, sizeof(holder.szlocalIp), NULL, Config::optReadOnly);
	holder.addDetail("CPH_RTINas/Capture", "sessionTimeout", &CPHRTINasCfg::timeoutInterval, NULL, Config::optReadOnly);
	holder.addDetail("CPH_RTINas/Capture", "winpcapKernelBuffer", &CPHRTINasCfg::winpcapKernelBufferInMB, "128", Config::optReadOnly);
	holder.addDetail("CPH_RTINas/Capture", "winpcapMinBufferCopy", &CPHRTINasCfg::winpcapMinBufferToCopyInKB, "512", Config::optReadOnly);


	holder.addDetail("CPH_RTINas/CaptureDumper", "enable",  &CPHRTINasCfg::enableDump, NULL, Config::optReadWrite);
	holder.addDetail("CPH_RTINas/CaptureDumper", "dumpPath", (PMem_CharArray)&CPHRTINasCfg::szDumpPath, sizeof(holder.szDumpPath), NULL, Config::optReadOnly);
	holder.addDetail("CPH_RTINas/CaptureDumper", "deleteOnSuccess", &CPHRTINasCfg::deleteDumpOnSuccess, NULL, Config::optReadWrite);

	holder.addDetail("CPH_RTINas/ProvisionMethod/Method",&CPHRTINasCfg::readMethod,&CPHRTINasCfg::registerNothing);

}

void CPHRTINasCfg::readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<Method> methodholder("name");
	methodholder.read(node, hPP);
	methods.push_back(methodholder);
}
