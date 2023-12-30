
#include "CPH_C2PropagationCfg.h"

using namespace ZQ::common;

ZQ::common::Config::Loader<C2PROPAGATION> _gCPHCfg("CPH_C2Propagation.xml");

C2PROPAGATION::C2PROPAGATION()
{
	memset(szCacheDir, 0, sizeof(szCacheDir));
	mediaSampleSize=64*1024;		//64K for vstrm
	enableProgEvent = 1;
	enableStreamEvent = 1;
	//enablePacingTrace = 0;	
	bandwidthLimitRate = 110;
	streamReqSecs = 5;				//5 seconds
	enablePacing = 0;

	enableMD5 = 0;


	// for test
	memset(szNTFSOutputDir, 0, sizeof(szNTFSOutputDir));
	memset(szLocalNetIf, 0, sizeof(szLocalNetIf));
	enableTestNTFS = 0;			//if 1 then do not write to vstrm but write to NTFS

	timeoutForGrowing = 10000;  //10000 ms
	enableResumeForDownload = 0;

	memset(szTargetDir, 0, sizeof(szTargetDir));
	memset(szPaceDllPath,0,sizeof(szPaceDllPath));

	testforcisco.nspeed = 1;
	testforcisco.transferdelay = 0;

	transferPort = 12000;

	vstrmBwClientId = 773220;
	vstrmDisableBufDrvThrottle = 1;

	nspeed = 1;
	transferip = "";
	bindip = "";
	transferdelay = 0;
}

C2PROPAGATION::~C2PROPAGATION()
{

}

void C2PROPAGATION::structure(ZQ::common::Config::Holder<C2PROPAGATION> &holder)
{
	using namespace ZQ::common;
	typedef Config::Holder<C2PROPAGATION>::PMem_CharArray PMem_CharArray;

	holder.addDetail("CPH_C2Propagation/Session", "overSpeedRate", &C2PROPAGATION::bandwidthLimitRate, NULL, Config::optReadWrite);
	holder.addDetail("CPH_C2Propagation/Session", "enableMD5", &C2PROPAGATION::enableMD5, "1", Config::optReadWrite);
	holder.addDetail("CPH_C2Propagation/Session", "enableCacheModeForIndex", &C2PROPAGATION::enableCacheForIndex, "1", Config::optReadWrite);
	holder.addDetail("CPH_C2Propagation/Session", "localNetworkInterface", (PMem_CharArray)&C2PROPAGATION::szLocalNetIf, sizeof(holder.szLocalNetIf), "", Config::optReadOnly);

	holder.addDetail("CPH_C2Propagation/DownLoad", "waitTime", &C2PROPAGATION::timeoutForGrowing, NULL, Config::optReadWrite);		
	holder.addDetail("CPH_C2Propagation/DownLoad", "bandwidthCtrlInterval", &C2PROPAGATION::bandwidthCtrlInterval, NULL, Config::optReadWrite);		
//	holder.addDetail("CPH_C2Propagation/DownLoad", "targetDir", (PMem_CharArray)&C2PROPAGATION::szTargetDir,sizeof(holder.szTargetDir), NULL, Config::optReadOnly);		
	holder.addDetail("CPH_C2Propagation/DownLoad", "enableResume", &C2PROPAGATION::enableResumeForDownload, NULL, Config::optReadWrite);		
	holder.addDetail("CPH_C2Propagation/DownLoad", "deleteOnFail", &C2PROPAGATION::deleteOnFail, "1", Config::optReadWrite);		

	holder.addDetail("CPH_C2Propagation/ProvisionMethod/Method",&C2PROPAGATION::readMethod,&C2PROPAGATION::registerNothing);

	holder.addDetail("CPH_C2Propagation/Event/Progress", "enable", &C2PROPAGATION::enableProgEvent, NULL, Config::optReadWrite);
	holder.addDetail("CPH_C2Propagation/Event/Streamable", "enable", &C2PROPAGATION::enableStreamEvent, NULL, Config::optReadWrite);
	holder.addDetail("CPH_C2Propagation/Event/Streamable", "lagAfterStart", &C2PROPAGATION::streamReqSecs, NULL, Config::optReadWrite);

	holder.addDetail("CPH_C2Propagation/NtfsOutputMode", "enable", &C2PROPAGATION::enableTestNTFS, NULL, Config::optReadWrite);
	holder.addDetail("CPH_C2Propagation/NtfsOutputMode", "homeDir", (PMem_CharArray)&C2PROPAGATION::szNTFSOutputDir, sizeof(holder.szNTFSOutputDir), NULL, Config::optReadOnly);

	holder.addDetail("CPH_C2Propagation/C2Client", "ingressCapacity", &C2PROPAGATION::ingressCapcaity, NULL, Config::optReadWrite);
	holder.addDetail("CPH_C2Propagation/C2Client", "cacheDir", (PMem_CharArray)&C2PROPAGATION::szCacheDir, sizeof(holder.szCacheDir), NULL, Config::optReadOnly);
	holder.addDetail("CPH_C2Propagation/C2Client", "bind", &C2PROPAGATION::bindip, NULL, Config::optReadWrite);
	holder.addDetail("CPH_C2Propagation/C2Client", "transferip", &C2PROPAGATION::transferip, NULL, Config::optReadWrite);
	holder.addDetail("CPH_C2Propagation/C2Client", "speed", &C2PROPAGATION::nspeed, NULL, Config::optReadWrite);
	holder.addDetail("CPH_C2Propagation/C2Client", "transferdelay", &C2PROPAGATION::transferdelay, NULL, Config::optReadWrite);
	holder.addDetail("CPH_C2Propagation/C2Client", "transferPort", &C2PROPAGATION::transferPort, NULL, Config::optReadWrite);
	holder.addDetail("CPH_C2Propagation/C2Client", "timeout", &C2PROPAGATION::timeout, NULL, Config::optReadWrite);

	holder.addDetail("CPH_C2Propagation/Vstream", "BWMgrClientId", &C2PROPAGATION::vstrmBwClientId, NULL, Config::optReadWrite);
	holder.addDetail("CPH_C2Propagation/Vstream", "disableBufDrvThrottle", &C2PROPAGATION::vstrmDisableBufDrvThrottle, NULL, Config::optReadWrite);

	holder.addDetail("CPH_C2Propagation/NormalizeSparseFile", "enable", &C2PROPAGATION::enableNSF, NULL, Config::optReadWrite);

	holder.addDetail("CPH_C2Propagation/testCisco",&C2PROPAGATION::readTestCsico,&C2PROPAGATION::registerNothing, Config::Range(0, 1));
	holder.addDetail("CPH_C2Propagation/SparseFile",&C2PROPAGATION::readsparseFile,&C2PROPAGATION::registerNothing, Config::Range(0, 1));
	holder.addDetail("CPH_C2Propagation/SleepTime",&C2PROPAGATION::readsleepTime,&C2PROPAGATION::registerNothing, Config::Range(0, 1));
//	holder.addDetail("CPH_C2Propagation/SessionFail",&C2PROPAGATION::readsparseFile,&C2PROPAGATION::registerNothing, Config::Range(0, 1));

}
void C2PROPAGATION::readTestCsico(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<TestForCisco> testforciscoHolder("");
	testforciscoHolder.read(node, hPP);
	testforcisco = testforciscoHolder;
}
void C2PROPAGATION::readsparseFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<SparseFile> sparsefileHolder("");
	sparsefileHolder.read(node, hPP);
	sparsefile = sparsefileHolder;
}
void C2PROPAGATION::readsleepTime(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<SleepTime> sleeptimeHolder("");
	sleeptimeHolder.read(node, hPP);
	sleeptime = sleeptimeHolder;
}
/*
void readsessionFailed(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<SessionFailed> sessionFailedholder("");
	sessionFailedholder.read(node, hPP);
	sessionFailed = sessionFailedholder;
}
*/
void C2PROPAGATION::readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
{
	ZQ::common::Config::Holder<Method> methodholder("name");
	methodholder.read(node, hPP);
	methods.push_back(methodholder);
}