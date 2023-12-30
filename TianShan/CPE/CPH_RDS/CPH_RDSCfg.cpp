
#include "CPH_RDSCfg.h"

using namespace ZQ::common;

ZQ::common::Config::Loader<CPHRDSCfg> _gCPHCfg("CPH_RDS.xml");


CPHRDSCfg::CPHRDSCfg()
{
	memset(szCacheDir, 0, sizeof(szCacheDir));
	mediaSampleSize=64*1024;		//64K for vstrm
	enableProgEvent = 1;
	enableStreamEvent = 1;
	maxCodingError = 1000;
	enablePacingTrace = 0;
	bandwidthLimitRate = 110;
	streamReqSecs = 5;				//5 seconds

	//maxBandwidthKBps = 30*1024;	//30mbps
	//maxSessionNum = 8;

	vstrmBwClientId = 773220;
	vstrmDisableBufDrvThrottle = 1;

	// for test
	memset(szNTFSOutputDir, 0, sizeof(szNTFSOutputDir));
	enableTestNTFS = 0;			//if 1 then do not write to vstrm but write to NTFS
}

CPHRDSCfg::~CPHRDSCfg()
{

}

void CPHRDSCfg::structure(ZQ::common::Config::Holder<CPHRDSCfg> &holder)
{
    using namespace ZQ::common;
    typedef Config::Holder<CPHRDSCfg>::PMem_CharArray PMem_CharArray;
	
	//holder.addDetail("CPH_RDS", "maxSessions", &CPHRDSCfg::maxSessionNum, NULL, Config::optReadWrite);
	//holder.addDetail("CPH_RDS", "maxBandwidth", &CPHRDSCfg::maxBandwidthKBps, NULL, Config::optReadWrite);	
	
    holder.addDetail("CPH_RDS/MediaSample", "bufferSize", &CPHRDSCfg::mediaSampleSize, NULL, Config::optReadOnly);
    holder.addDetail("CPH_RDS/Session", "overSpeedRate", &CPHRDSCfg::bandwidthLimitRate, NULL, Config::optReadWrite);
	
	holder.addDetail("CPH_RDS/PacedIndex", "cacheDir", (PMem_CharArray)&CPHRDSCfg::szCacheDir, sizeof(holder.szCacheDir), NULL, Config::optReadOnly);	
	holder.addDetail("CPH_RDS/PacedIndex", "enableTrace", &CPHRDSCfg::enablePacingTrace, NULL, Config::optReadWrite);		
	
    holder.addDetail("CPH_RDS/TrickFilesLibraryUser", "maxCodingError", &CPHRDSCfg::maxCodingError, NULL, Config::optReadWrite);

    holder.addDetail("CPH_RDS/Vstream", "BWMgrClientId", &CPHRDSCfg::vstrmBwClientId, NULL, Config::optReadWrite);
    holder.addDetail("CPH_RDS/Vstream", "disableBufDrvThrottle", &CPHRDSCfg::vstrmDisableBufDrvThrottle, NULL, Config::optReadWrite);
	
    holder.addDetail("CPH_RDS/Event/Progress", "enable", &CPHRDSCfg::enableProgEvent, NULL, Config::optReadWrite);
    holder.addDetail("CPH_RDS/Event/Streamable", "enable", &CPHRDSCfg::enableStreamEvent, NULL, Config::optReadWrite);
    holder.addDetail("CPH_RDS/Event/Streamable", "lagAfterStart", &CPHRDSCfg::streamReqSecs, NULL, Config::optReadWrite);
	
	holder.addDetail("CPH_RDS/NtfsOutputMode", "enable", &CPHRDSCfg::enableTestNTFS, NULL, Config::optReadWrite);
	holder.addDetail("CPH_RDS/NtfsOutputMode", "homeDir", (PMem_CharArray)&CPHRDSCfg::szNTFSOutputDir, sizeof(holder.szNTFSOutputDir), NULL, Config::optReadOnly);

	holder.addDetail("CPH_RDS/ProvisionMethod/Method",&CPHRDSCfg::readMethod,&CPHRDSCfg::registerNothing);
}

void CPHRDSCfg::readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<Method> methodholder("name");
	methodholder.read(node, hPP);
	methods.push_back(methodholder);
}