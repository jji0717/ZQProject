
#include "CPH_Cfg.h"

using namespace ZQ::common;

ZQ::common::Config::Loader<CPHConfig> _gCPHCfg("CPH_CDN.xml");

CPHConfig::CPHConfig()
{
	//memset(szCacheDir, 0, sizeof(szCacheDir));
	mediaSampleSize=64*1024;		//64K for vstrm
	maxAllocSampleCount = 400;
	enableProgEvent = 1;
	enableStreamEvent = 1;
	//enablePacingTrace = 0;	
	bandwidthLimitRate = 110;
	streamReqSecs = 5;				//5 seconds
	enablePacing = 0;

	//maxBandwidthKBps = 100*1024;	//100mbps
	//maxSessionNum = 30;
	rtfMaxInputBufferBytes = 64*1024;	
	rtfMaxInputBuffersPerSession = 100;
	rtfSessionFailThreshold = 0;
	enableMD5 = 0;
	enableNSF = 0;
    decodeSourceURL = 0;
	//vstrmBwClientId = 773220;
	//vstrmDisableBufDrvThrottle = 1;

	// for test
	memset(szNTFSOutputDir, 0, sizeof(szNTFSOutputDir));
	memset(szLocalNetIf, 0, sizeof(szLocalNetIf));
	enableTestNTFS = 0;			//if 1 then do not write to vstrm but write to NTFS

	timeoutForGrowing = 10000;  //10000 ms
	enableResumeForDownload = 0;

	strAugmentationPids ="";

	memset(szTargetDir, 0, sizeof(szTargetDir));
	memset(szPaceDllPath,0,sizeof(szPaceDllPath));
    unifiedtrickfile.enable = 1;
	ciscofileext.mode = 0;

	memset(szUrlEncode, 0, sizeof(szUrlEncode));

	mountpath.entry = "/mnt/CPESvc/";

	deleteOnFail = true;

	transferIpForCache = "";
}

CPHConfig::~CPHConfig()
{

}

void CPHConfig::structure(ZQ::common::Config::Holder<CPHConfig> &holder)
{
    using namespace ZQ::common;
    typedef Config::Holder<CPHConfig>::PMem_CharArray PMem_CharArray;
	
    holder.addDetail("CPH_CDN/MediaSample", "bufferSize", &CPHConfig::mediaSampleSize, NULL, Config::optReadOnly);
	holder.addDetail("CPH_CDN/MediaSample", "maxAllocSampleCount", &CPHConfig::maxAllocSampleCount, "400", Config::optReadOnly);
    holder.addDetail("CPH_CDN/Session", "overSpeedRate", &CPHConfig::bandwidthLimitRate, NULL, Config::optReadWrite);
	holder.addDetail("CPH_CDN/Session", "enableMD5", &CPHConfig::enableMD5, "1", Config::optReadWrite);
	holder.addDetail("CPH_CDN/Session", "enableCacheModeForIndex", &CPHConfig::enableCacheForIndex, "1", Config::optReadWrite);
	holder.addDetail("CPH_CDN/Session", "localNetworkInterface", (PMem_CharArray)&CPHConfig::szLocalNetIf, sizeof(holder.szLocalNetIf), "", Config::optReadOnly);

	holder.addDetail("CPH_CDN/DownLoad", "waitTime", &CPHConfig::timeoutForGrowing, NULL, Config::optReadWrite);		
	holder.addDetail("CPH_CDN/DownLoad", "bandwidthCtrlInterval", &CPHConfig::bandwidthCtrlInterval, NULL, Config::optReadWrite);		
	holder.addDetail("CPH_CDN/DownLoad", "targetDir", (PMem_CharArray)&CPHConfig::szTargetDir,sizeof(holder.szTargetDir), NULL, Config::optReadOnly);		
	holder.addDetail("CPH_CDN/DownLoad", "enableResume", &CPHConfig::enableResumeForDownload, NULL, Config::optReadWrite);		
	holder.addDetail("CPH_CDN/DownLoad", "deleteOnFail", &CPHConfig::deleteOnFail, "1", Config::optReadWrite);		

	//holder.addDetail("CPH_CDN/PacedIndex", "cacheDir", (PMem_CharArray)&CPHConfig::szCacheDir, sizeof(holder.szCacheDir), NULL, Config::optReadOnly);	
	//holder.addDetail("CPH_CDN/PacedIndex", "enableTrace", &CPHConfig::enablePacingTrace, NULL, Config::optReadWrite);		
	holder.addDetail("CPH_CDN/PacedIndex", "enablePacing", &CPHConfig::enablePacing, "0", Config::optReadWrite);
	holder.addDetail("CPH_CDN/PacedIndex", "pacedFile", (PMem_CharArray)&CPHConfig::szPaceDllPath, sizeof(holder.szPaceDllPath), NULL, Config::optReadOnly);	

	holder.addDetail("CPH_CDN/Encode", "urlEncode", (PMem_CharArray)&CPHConfig::szUrlEncode, sizeof(holder.szUrlEncode), "", Config::optReadOnly);	
	holder.addDetail("CPH_CDN/Encode", "decodePecentOfSourceURL", &CPHConfig::decodeSourceURL, NULL, Config::optReadOnly);

	holder.addDetail("CPH_CDN/ProvisionMethod/Method",&CPHConfig::readMethod,&CPHConfig::registerNothing);

    holder.addDetail("CPH_CDN/TrickGeneration", "maxSessionNum", &CPHConfig::rtfMaxSessionNum, NULL, Config::optReadOnly);
    holder.addDetail("CPH_CDN/TrickGeneration", "maxInputBufferBytes", &CPHConfig::rtfMaxInputBufferBytes, NULL, Config::optReadWrite);
    holder.addDetail("CPH_CDN/TrickGeneration", "maxInputBuffersPerSession", &CPHConfig::rtfMaxInputBuffersPerSession, NULL, Config::optReadOnly);
    holder.addDetail("CPH_CDN/TrickGeneration", "sessionFailThreshold", &CPHConfig::rtfSessionFailThreshold, NULL, Config::optReadWrite);

   // holder.addDetail("CPH_CDN/Vstream", "BWMgrClientId", &CPHConfig::vstrmBwClientId, NULL, Config::optReadWrite);
    //holder.addDetail("CPH_CDN/Vstream", "disableBufDrvThrottle", &CPHConfig::vstrmDisableBufDrvThrottle, NULL, Config::optReadWrite);

    holder.addDetail("CPH_CDN/Event/Progress", "enable", &CPHConfig::enableProgEvent, NULL, Config::optReadWrite);
    holder.addDetail("CPH_CDN/Event/Streamable", "enable", &CPHConfig::enableStreamEvent, NULL, Config::optReadWrite);
    holder.addDetail("CPH_CDN/Event/Streamable", "lagAfterStart", &CPHConfig::streamReqSecs, NULL, Config::optReadWrite);

	holder.addDetail("CPH_CDN/NtfsOutputMode", "enable", &CPHConfig::enableTestNTFS, NULL, Config::optReadWrite);
	holder.addDetail("CPH_CDN/NtfsOutputMode", "homeDir", (PMem_CharArray)&CPHConfig::szNTFSOutputDir, sizeof(holder.szNTFSOutputDir), NULL, Config::optReadOnly);

	holder.addDetail("CPH_CDN/C2Client", "ingressCapacity", &CPHConfig::ingressCapcaity, NULL, Config::optReadWrite);
	holder.addDetail("CPH_CDN/C2Client", "cacheDir", (PMem_CharArray)&CPHConfig::szCacheDir, sizeof(holder.szCacheDir), NULL, Config::optReadOnly);
	holder.addDetail("CPH_CDN/C2Client", "bind", &CPHConfig::bindip, NULL, Config::optReadWrite);
	holder.addDetail("CPH_CDN/C2Client", "transferip", &CPHConfig::transferip, NULL, Config::optReadWrite);
	holder.addDetail("CPH_CDN/C2Client", "speed", &CPHConfig::nspeed, NULL, Config::optReadWrite);
	holder.addDetail("CPH_CDN/C2Client", "transferdelay", &CPHConfig::transferdelay, NULL, Config::optReadWrite);
	holder.addDetail("CPH_CDN/C2Client", "transferPort", &CPHConfig::transferPort, NULL, Config::optReadWrite);
	holder.addDetail("CPH_CDN/C2Client", "timeout", &CPHConfig::timeout, "20", Config::optReadWrite);

	holder.addDetail("CPH_CDN/NormalizeSparseFile", "enable", &CPHConfig::enableNSF, NULL, Config::optReadWrite);
   
	holder.addDetail("CPH_CDN/Augmentation", "pids", &CPHConfig::strAugmentationPids, NULL, Config::optReadWrite);
	holder.addDetail("CPH_CDN/UnifiedTrickFile",&CPHConfig::readUnifiedTrickFile,&CPHConfig::registerNothing, Config::Range(0, 1)); 
	holder.addDetail("CPH_CDN/FileExtName",&CPHConfig::readCiscoFileExt,&CPHConfig::registerNothing, Config::Range(0, 1)); 
	holder.addDetail("CPH_CDN/MountPath",&CPHConfig::readMountPath,&CPHConfig::registerNothing, Config::Range(0, 1)); 
	holder.addDetail("CPH_CDN/DiskWrite",&CPHConfig::readFileSync,&CPHConfig::registerNothing, Config::Range(0, 1)); 
	holder.addDetail("CPH_CDN/SparseFile",&CPHConfig::readsparseFile,&CPHConfig::registerNothing, Config::Range(0, 1));
	holder.addDetail("CPH_CDN/SleepTime",&CPHConfig::readsleepTime,&CPHConfig::registerNothing, Config::Range(0, 1));

}

void CPHConfig::readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<Method> methodholder("name");
	methodholder.read(node, hPP);
	methods.push_back(methodholder);
}

void CPHConfig::readUnifiedTrickFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<UnifiedTrickFile> unifiedholder("");
	unifiedholder.read(node, hPP);
	unifiedtrickfile = unifiedholder;
}

void CPHConfig::readCiscoFileExt(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<CiscoFileExt> ciscofileextholder("");
	ciscofileextholder.read(node, hPP);
	ciscofileext = ciscofileextholder;
}

void CPHConfig::readMountPath(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<MountPath> MountPathholder("");
	MountPathholder.read(node, hPP);
	mountpath = MountPathholder;
}

void CPHConfig::readFileSync(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<FileSync> filesyncholder("");
	filesyncholder.read(node, hPP);
	filesync = filesyncholder;
	
}

void CPHConfig::readsparseFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<SparseFile> sparsefileHolder("");
	sparsefileHolder.read(node, hPP);
	sparsefile = sparsefileHolder;
}

void CPHConfig::readsleepTime(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<SleepTime> sleeptimeHolder("");
	sleeptimeHolder.read(node, hPP);
	sleeptime = sleeptimeHolder;
}