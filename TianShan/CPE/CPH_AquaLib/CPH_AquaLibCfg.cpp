#include "CPH_AquaLibCfg.h"

using namespace ZQ::common;

namespace ZQTianShan {
namespace ContentProvision{

ZQ::common::Config::Loader<CPH_AquaLibConfig> _gCPHCfg("CPH_AquaLib.xml");

CPH_AquaLibConfig::CPH_AquaLibConfig()
{
    //memset(szCacheDir, 0, sizeof(szCacheDir));
    mediaSampleSize=64*1024;        //64K for vstrm
	maxAllocSampleCount = 400;
    enableProgEvent = 1;
    enableStreamEvent = 1;
    //enablePacingTrace = 0;    
    bandwidthLimitRate = 110;
    streamReqSecs = 5;              //5 seconds
    enablePacing = 0;

    //maxBandwidthKBps = 100*1024;  //100mbps
    //maxSessionNum = 30;
    rtfMaxInputBufferBytes = 64*1024;   
    rtfMaxInputBuffersPerSession = 100;
    rtfSessionFailThreshold = 0;
    enableMD5 = 0;
    decodeSourceURL = 0;
    //vstrmBwClientId = 773220;
    //vstrmDisableBufDrvThrottle = 1;

    // for test
    memset(szLocalNetIf, 0, sizeof(szLocalNetIf));

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
}

CPH_AquaLibConfig::~CPH_AquaLibConfig()
{

}

void CPH_AquaLibConfig::structure(ZQ::common::Config::Holder<CPH_AquaLibConfig> &holder)
{
    using namespace ZQ::common;
    typedef Config::Holder<CPH_AquaLibConfig>::PMem_CharArray PMem_CharArray;
    
    holder.addDetail("CPH_AquaLib/MediaSample",         "bufferSize",               &CPH_AquaLibConfig::mediaSampleSize,    "65536", Config::optReadOnly);
	holder.addDetail("CPH_AquaLib/MediaSample",         "maxAllocSampleCount",      &CPH_AquaLibConfig::maxAllocSampleCount,"400", Config::optReadOnly);
    holder.addDetail("CPH_AquaLib/Session",             "overSpeedRate",            &CPH_AquaLibConfig::bandwidthLimitRate, NULL, Config::optReadWrite);
    holder.addDetail("CPH_AquaLib/Session",             "enableMD5",                &CPH_AquaLibConfig::enableMD5,          "1", Config::optReadWrite);
    holder.addDetail("CPH_AquaLib/Session",             "enableCacheModeForIndex",  &CPH_AquaLibConfig::enableCacheForIndex,"1", Config::optReadWrite);
    holder.addDetail("CPH_AquaLib/Session",             "localNetworkInterface",    (PMem_CharArray)&CPH_AquaLibConfig::szLocalNetIf, sizeof(holder.szLocalNetIf), "", Config::optReadOnly);      
     
    holder.addDetail("CPH_AquaLib/DownLoad",            "waitTime",                 &CPH_AquaLibConfig::timeoutForGrowing,          NULL, Config::optReadWrite);     
    holder.addDetail("CPH_AquaLib/DownLoad",            "bandwidthCtrlInterval",    &CPH_AquaLibConfig::bandwidthCtrlInterval,      NULL, Config::optReadWrite);        
    holder.addDetail("CPH_AquaLib/DownLoad",            "targetDir",                (PMem_CharArray)&CPH_AquaLibConfig::szTargetDir,sizeof(holder.szTargetDir), NULL, Config::optReadOnly);             
    holder.addDetail("CPH_AquaLib/DownLoad",            "enableResume",             &CPH_AquaLibConfig::enableResumeForDownload,    NULL, Config::optReadWrite);       
    holder.addDetail("CPH_AquaLib/DownLoad",            "deleteOnFail",             &CPH_AquaLibConfig::deleteOnFail, "1", Config::optReadWrite);       
     
    //holder.addDetail("CPH_AquaLib/PacedIndex",        "cacheDir",                 (PMem_CharArray)&CPH_AquaLibConfig::szCacheDir, sizeof(holder.szCacheDir), NULL, Config::optReadOnly);      
    //holder.addDetail("CPH_AquaLib/PacedIndex",        "enableTrace",              &CPH_AquaLibConfig::enablePacingTrace, NULL, Config::optReadWrite);      
    holder.addDetail("CPH_AquaLib/PacedIndex",          "enablePacing",             &CPH_AquaLibConfig::enablePacing, "0", Config::optReadWrite);
    holder.addDetail("CPH_AquaLib/PacedIndex",          "pacedFile",                (PMem_CharArray)&CPH_AquaLibConfig::szPaceDllPath, sizeof(holder.szPaceDllPath), NULL, Config::optReadOnly);        
     
    holder.addDetail("CPH_AquaLib/Encode",              "urlEncode",                (PMem_CharArray)&CPH_AquaLibConfig::szUrlEncode, sizeof(holder.szUrlEncode), "", Config::optReadOnly);    
    holder.addDetail("CPH_AquaLib/Encode",              "decodePecentOfSourceURL",  &CPH_AquaLibConfig::decodeSourceURL,                NULL, Config::optReadOnly);
     
    holder.addDetail("CPH_AquaLib/TrickGeneration",     "maxSessionNum",            &CPH_AquaLibConfig::rtfMaxSessionNum,               NULL, Config::optReadOnly);
    holder.addDetail("CPH_AquaLib/TrickGeneration",     "maxInputBufferBytes",      &CPH_AquaLibConfig::rtfMaxInputBufferBytes,         NULL, Config::optReadWrite);
    holder.addDetail("CPH_AquaLib/TrickGeneration",     "maxInputBuffersPerSession",&CPH_AquaLibConfig::rtfMaxInputBuffersPerSession,   NULL, Config::optReadOnly);
    holder.addDetail("CPH_AquaLib/TrickGeneration",     "sessionFailThreshold",     &CPH_AquaLibConfig::rtfSessionFailThreshold,        NULL, Config::optReadWrite);
    
   // holder.addDetail("CPH_AquaLib/Vstream",           "BWMgrClientId",            &CPH_AquaLibConfig::vstrmBwClientId, NULL, Config::optReadWrite);
    //holder.addDetail("CPH_AquaLib/Vstream",           "disableBufDrvThrottle",    &CPH_AquaLibConfig::vstrmDisableBufDrvThrottle, NULL, Config::optReadWrite);
    
    holder.addDetail("CPH_AquaLib/Event/Progress",      "enable",                   &CPH_AquaLibConfig::enableProgEvent,    NULL, Config::optReadWrite);
	holder.addDetail("CPH_AquaLib/Event/Progress",      "minIntervalMs",            &CPH_AquaLibConfig::minIntervalMs,     "120000", Config::optReadWrite);
	holder.addDetail("CPH_AquaLib/Event/Progress",      "maxPecentageStep",         &CPH_AquaLibConfig::maxPecentageStep,   "20", Config::optReadWrite);
    holder.addDetail("CPH_AquaLib/Event/Streamable",    "enable",                   &CPH_AquaLibConfig::enableStreamEvent,  NULL, Config::optReadWrite);
    holder.addDetail("CPH_AquaLib/Event/Streamable",    "lagAfterStart",            &CPH_AquaLibConfig::streamReqSecs,      NULL, Config::optReadWrite);

    holder.addDetail("CPH_AquaLib/Augmentation",        "pids",                     &CPH_AquaLibConfig::strAugmentationPids,NULL, Config::optReadWrite);

    holder.addDetail("CPH_AquaLib/UnifiedTrickFile",    &CPH_AquaLibConfig::readUnifiedTrickFile,   &CPH_AquaLibConfig::regsiterUnifiedTrickFile,   Config::Range(0, 1)); 
    holder.addDetail("CPH_AquaLib/FileExtName",         &CPH_AquaLibConfig::readCiscoFileExt,       &CPH_AquaLibConfig::regsiterCiscoFileExt,       Config::Range(0, 1)); 
    holder.addDetail("CPH_AquaLib/MountPath",           &CPH_AquaLibConfig::readMountPath,          &CPH_AquaLibConfig::regsiterMountPath,          Config::Range(0, 1)); 
    holder.addDetail("CPH_AquaLib/RetryCapture",        &CPH_AquaLibConfig::readRetryCapture,       &CPH_AquaLibConfig::regsiterRetryCapture);
    holder.addDetail("CPH_AquaLib/Capture",             &CPH_AquaLibConfig::readCapture,            &CPH_AquaLibConfig::regsiterCapture);
    holder.addDetail("CPH_AquaLib/CaptureDumper",       &CPH_AquaLibConfig::readCaptureDumper,      &CPH_AquaLibConfig::regsiterCaptureDumper);
    holder.addDetail("CPH_AquaLib/ProvisionMethod",     &CPH_AquaLibConfig::readProvisionMethod,    &CPH_AquaLibConfig::regsiterProvisionMethod);
    holder.addDetail("CPH_AquaLib/AquaServer",          &CPH_AquaLibConfig::readAquaServer,         &CPH_AquaLibConfig::regsiterAquaServer);
	holder.addDetail("CPH_AquaLib/Cache",               &CPH_AquaLibConfig::readAquaCache,          &CPH_AquaLibConfig::regsiterAquaCache);
}

void CPH_AquaLibConfig::readCapture(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    capture.read(node, hPP);
}

void CPH_AquaLibConfig::regsiterCapture(const std::string& full_path )
{
    capture.snmpRegister(full_path);
}

void CPH_AquaLibConfig::readCaptureDumper(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    captureDumper.read(node, hPP);
}

void CPH_AquaLibConfig::regsiterCaptureDumper(const std::string& full_path )
{
    captureDumper.snmpRegister(full_path);
}

void CPH_AquaLibConfig::readUnifiedTrickFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    unifiedtrickfile.read(node, hPP);
}

void CPH_AquaLibConfig::regsiterUnifiedTrickFile(const std::string& full_path )
{
    unifiedtrickfile.snmpRegister(full_path);
}

void CPH_AquaLibConfig::readCiscoFileExt(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    ciscofileext.read(node, hPP);
}

void CPH_AquaLibConfig::regsiterCiscoFileExt(const std::string& full_path )
{
    ciscofileext.snmpRegister(full_path);
}

void CPH_AquaLibConfig::readMountPath(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    mountpath.read(node, hPP);
}

void CPH_AquaLibConfig::regsiterMountPath(const std::string& full_path )
{
    mountpath.snmpRegister(full_path);
}

void CPH_AquaLibConfig::readRetryCapture(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    retryCapture.read(node, hPP);
}

void CPH_AquaLibConfig::regsiterRetryCapture(const std::string& full_path )
{
    retryCapture.snmpRegister(full_path);
}

void CPH_AquaLibConfig::readProvisionMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    provisionMethod.read(node, hPP);
}

void CPH_AquaLibConfig::regsiterProvisionMethod(const std::string& full_path )
{
    provisionMethod.snmpRegister(full_path);
}

void CPH_AquaLibConfig::readAquaServer(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
    aquaServer.read(node, hPP);
}

void CPH_AquaLibConfig::regsiterAquaServer(const std::string& full_path )
{
    aquaServer.snmpRegister(full_path);
}

void CPH_AquaLibConfig::readAquaCache(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	aquaCache.read(node, hPP);
}
void CPH_AquaLibConfig::regsiterAquaCache(const std::string& full_path )
{
	aquaCache.snmpRegister(full_path);
}
}}