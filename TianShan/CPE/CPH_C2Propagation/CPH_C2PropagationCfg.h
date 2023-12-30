
#ifndef _CPH_C2PROPAGATION_CONFIG_
#define _CPH_C2PROPAGATION_CONFIG_

#include <ConfigHelper.h>

struct Method
{
	std::string methodName;
	int32 maxSession;
	int32 maxBandwidth;
	int32 enableFlag;
	static void structure(ZQ::common::Config::Holder<Method > &holder)
	{
		holder.addDetail("", "name", &Method::methodName, NULL,ZQ::common::Config::optReadOnly);
		holder.addDetail("", "maxSessions", &Method::maxSession, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "maxBandwidth", &Method::maxBandwidth, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "enable", &Method::enableFlag, NULL, ZQ::common::Config::optReadOnly);
	}
};
struct TestForCisco
{
	int32 nspeed;
	int32 transferdelay;
	int32 timeout;
	int32 bandwidth;//Kbps;
	std::string range;
	int32 locateBeginpos;
	int32 locateEndpos;
	static void structure(ZQ::common::Config::Holder<TestForCisco > &holder)
	{
		holder.addDetail("", "speed", &TestForCisco::nspeed, NULL,ZQ::common::Config::optReadOnly);
		holder.addDetail("", "transferdelay", &TestForCisco::transferdelay, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "timeout", &TestForCisco::timeout, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "bandwidth", &TestForCisco::bandwidth, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "range", &TestForCisco::range, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "locatebeginpos", &TestForCisco::locateBeginpos, NULL, ZQ::common::Config::optReadOnly);
		holder.addDetail("", "locateendpos", &TestForCisco::locateEndpos, NULL, ZQ::common::Config::optReadOnly);
        
	}
	TestForCisco()
	{
		nspeed = 1;
		transferdelay = 0;
		timeout = 100;
		bandwidth = 0;
		range = "";
		locateBeginpos = 0;
		locateEndpos = -1;
	}
};
struct SparseFile
{
    int32 sparseFilesize;
	static void structure(ZQ::common::Config::Holder<SparseFile > &holder)
	{
		holder.addDetail("", "sparseFilesize", &SparseFile::sparseFilesize, NULL,ZQ::common::Config::optReadOnly);

	}
	SparseFile()
	{
       sparseFilesize = 20000000; //kb 20G
	}
};
struct SleepTime
{
	int32 timeInterval;
	static void structure(ZQ::common::Config::Holder<SleepTime > &holder)
	{
		holder.addDetail("", "timeInterval", &SleepTime::timeInterval, NULL,ZQ::common::Config::optReadOnly);

	}
	SleepTime()
	{
		timeInterval = 5000; //ms
	}
};
/*
struct SessionFailed
{
	int32 deleteOnFail;
	static void structure(ZQ::common::Config::Holder<SessionFailed > &holder)
	{
		holder.addDetail("", "deleteOnFail", &SessionFailed::deleteOnFail, NULL,ZQ::common::Config::optReadOnly);
	}
	SessionFailed()
	{
		deleteOnFail = 1;
	}
};
*/
class C2PROPAGATION
{
public:
	C2PROPAGATION ();
	~C2PROPAGATION ();

	static void structure(ZQ::common::Config::Holder<C2PROPAGATION> &holder);
	void registerNothing(const std::string&){}

	typedef std::vector< ZQ::common::Config::Holder< Method > > Methods;
	Methods methods;

	void readMethod(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readTestCsico(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readsparseFile(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
	void readsleepTime(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);
//	void readsessionFailed(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP);

	char	szCacheDir[256];
	int32	mediaSampleSize;
	int32	bandwidthLimitRate;
	int32	enableProgEvent;
	int32	enableStreamEvent;
	int32	streamReqSecs;
	int32	enableMD5;
	int32	enableCacheForIndex;
	int32   enablePacing;

	//for url encode
/*	char    szUrlEncode[256];*/

	char	szLocalNetIf[256];	

	// process ability
	//int32	maxBandwidthKBps;
	//int32	maxSessionNum;

	// for vstrm bandwidth management
	int32	vstrmBwClientId;

	int32	vstrmDisableBufDrvThrottle;

	// for test
	char	szNTFSOutputDir[256];	
	int32	enableTestNTFS;			//if 1 then do not write to vstrm but write to NTFS

	int32  enableCmdGetsize; 
	int32  timeoutForGrowing;
	int32  bandwidthCtrlInterval;
	char   szTargetDir[256];
	int32  enableResumeForDownload;
	char   szPaceDllPath[256];

	int32  deleteOnFail;

	int32  ingressCapcaity;
	std::string bindip;
	std::string transferip;
	int32 enableNSF;
    
	int32   nspeed;
	int32   transferdelay;
	int32   transferPort;
	int32   timeout;

	TestForCisco testforcisco;
	SparseFile   sparsefile;
	SleepTime    sleeptime;
//	SessionFailed sessionFailed;
};


extern ZQ::common::Config::Loader<C2PROPAGATION> _gCPHCfg;

#endif

