
#include "BaseClass.h"
#include "NTFSSource.h"
#include "NTFSTarget.h"
#include "IPushTrigger.h"
#include "PushSource.h"
#include "BaseClass.h"
#include "VstrmFilesetTarget.h"
#include "CPH_RTICfg.h"
#include "RTFProc.h"
#include "FileLog.h"
//#include "QueueBufMgr.h"
#include "BufferPool.h"
#include "McastCapsrc.h"
#include <list>
#include <math.h>
#include "getopt.h"
#include "WPCapThread.h"

using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define CPH_RTI			"CPH_RTI"
#define MOLOG					(glog)

#define RTI_PROVISION_ERRCODE		1

ZQ::common::FileLog		filelog;

//ZQ::common::Config::Loader<RtiConfig> _gCPHCfg("CPH_RTI.xml");
class RTIGen : public ZQTianShan::ContentProvision::BaseGraph, public ZQ::common::BufferPool
{
public:
	RTIGen()
		:BaseGraph(400), _bQuit(false) 
	{
		_bCleaned = false;
		_pMainTarget = NULL;
	}
	
	~RTIGen(){};
	
public:
	
	bool preLoad( int nBandwidth, std::string& strFilename,std::string& Ip, int& port,std::string& strtrickspeed, bool bH264Type);
	void OnProgress(LONGLONG& prcvBytes);
	void OnStreamable(bool bStreamable);
	void OnMediaInfoParsed(MediaInfo& mInfo);

	bool run();
	
protected:
	void cleanup();
	
	bool _bQuit;
	bool _bCleaned;
	ZQTianShan::ContentProvision::BaseTarget*		_pMainTarget;	//main target that will send streamable event
};

void InitCPH(std::string& strCfgDir)
{
	filelog.open("RTIGen.log", ZQ::common::Log::L_DEBUG, 1);

	// set log handler	
	ZQ::common::setGlogger(&filelog);
	_gCPHCfg.setLogger(&glog);
	
	// load configurations
	if (!_gCPHCfg.loadInFolder(strCfgDir.c_str()))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "Failed to load configuration [%s]"), 
			_gCPHCfg.getConfigFilePath().c_str());		
	}
	else
	{
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "Load configuration from [%s] successfully"),
			_gCPHCfg.getConfigFilePath().c_str());
		
		_gCPHCfg.snmpRegister("");
	}

	//
	// do some module initialize
	//
	RTFProcess::initRTFLib(_gCPHCfg.rtfMaxSessionNum, &glog, _gCPHCfg.rtfMaxInputBufferBytes, 
		_gCPHCfg.rtfMaxInputBuffersPerSession, _gCPHCfg.rtfSessionFailThreshold);
	
	std::string errstr;
	std::string localIp = _gCPHCfg.szlocalIp;

	//multicast capture initialize
	{
		WinpCapThreadInterface* pCaptureInterface;
		pCaptureInterface = new WinpCapThreadInterface();
		pCaptureInterface->setKernelBufferBytes(_gCPHCfg.winpcapKernelBufferInMB*1024*1024);
		pCaptureInterface->setMinBytesToCopy(_gCPHCfg.winpcapMinBufferToCopyInKB*1024);

		MulticastCaptureInterface::setInstance(pCaptureInterface);
		pCaptureInterface->setLog(&glog);

		for(int i=0;i<_gCPHCfg.nInterface.size();i++)
		{
			std::string strLocalIp;
			strLocalIp = _gCPHCfg.nInterface[i].strIp;

			pCaptureInterface->addNIC(strLocalIp, _gCPHCfg.nInterface[i].totalBandwidth);
		}

		if (!pCaptureInterface->init())
		{
			MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "Failed to initialize capture interface with error: %s"),
				pCaptureInterface->getLastError().c_str());
		}
	}
}


void UninitCPH()
{
	//
	//do some module uninitialize
	//
	MulticastCaptureInterface::destroyInstance();
	VstrmFsTarget::uninitVstrm();
	RTFProcess::uninitRTFLib();
	if (&filelog == ZQ::common::getGlogger())
		ZQ::common::setGlogger(NULL);
//	ZQ::common::setGlogger(NULL);
}

///////////////
#include "UrlStr.h"
void getUnifiedTrickExt(int speedNo, char* ext)
{
	if(0 == speedNo)
	{
		sprintf(ext, ".FFR");
	}
	else
	{
		sprintf(ext, ".FFR%d", speedNo);
	}		
}

void getTrickExt(int speedNo, char* ext1, char* ext2)
{
	if(0 == speedNo)
	{
		sprintf(ext1, ".FF");
		sprintf(ext2, ".FR");
	}
	else
	{
		sprintf(ext1, ".FF%d", speedNo);
		sprintf(ext2, ".FR%d", speedNo);
	}		
}

static bool fixpath(std::string& path, bool bIsLocal = true)
{
	char* pathbuf = new char[path.length() +2];
	if (NULL ==pathbuf)
		return false;
	
	strcpy(pathbuf, path.c_str());
	pathbuf[path.length()] = '\0';
	for (char* p = pathbuf; *p; p++)
	{
		if ('\\' == *p || '/' == *p)
			*p = FNSEPC;
	}
	
	if (!bIsLocal && ':' == pathbuf[1])
		pathbuf[1] = '$';
	else if (bIsLocal && '$' == pathbuf[1])
		pathbuf[1] = ':';
	
	path = pathbuf;
	
	return true;
	
}

static unsigned long timeval()
{
	unsigned long rettime = 1;
	
	FILETIME systemtimeasfiletime;
	LARGE_INTEGER litime;
	
	GetSystemTimeAsFileTime(&systemtimeasfiletime);
	memcpy(&litime,&systemtimeasfiletime,sizeof(LARGE_INTEGER));
	litime.QuadPart /= 10000;  //convert to milliseconds
	litime.QuadPart &= 0xFFFFFFFF;    //keep only the low part
	rettime = (unsigned long)(litime.QuadPart);
	
	return rettime;
}

bool RTIGen::preLoad( int nBandwidth, std::string& strFilename,std::string& Ip, int& port,std::string& strtrickspeed, bool bH264Type)
{
	
	std::string  multicastIp= Ip;
    int multicastPort  = port;

	SetLog(&filelog);
	SetMemAlloc(this);
	SetLogHint(strFilename.c_str());

	int nMaxBandwidth;
	nMaxBandwidth = nBandwidth;
	
	MOLOG(Log::L_DEBUG, CLOGFMT(CPH_RTI, "[%s] bandwidth[%d]"), _strLogHint.c_str(), 
		nBandwidth);
	
	DWORD timeoutInterval = _gCPHCfg.timeoutInterval;
	
	std::string strLocalIp = _gCPHCfg.nInterface[0].strIp;
	McastCapSource* pSource = (McastCapSource*)SourceFactory::Create(SOURCE_TYPE_MCASTCAPSRC);
	AddFilter(pSource);	
	pSource->setInspectPara(multicastIp,multicastPort,timeoutInterval, strLocalIp);
	std::list<int> trickspeed;
	if (strtrickspeed.empty())
		trickspeed.push_back(1);
	else
	{
		std::vector<std::string> strVec;
		strVec = ZQ::common::stringHelper::split(strtrickspeed,',');

		for (std::vector<std::string>::iterator it = strVec.begin();it != strVec.end(); it++)
		{
			trickspeed.push_back(int(ceil((atof((*it).c_str()))/float(7.5))));
		}
	}

	std::map<std::string, int> exMap;
	std::map<std::string, int>::iterator iter;
	int i = 0;
	trickspeed.sort();
	for (std::list<int>::iterator iter = trickspeed.begin();iter != trickspeed.end();iter++)
	{
		char ex[10]={0};
		char exr[10] ={0};

		if (bH264Type)
		{
			getUnifiedTrickExt(i,ex);

			exMap.insert(std::make_pair(std::string(ex),(*iter)-1));		
		}
		else
		{
			getTrickExt(i,ex,exr);

			exMap.insert(std::make_pair(std::string(ex),(*iter)-1));
			exMap.insert(std::make_pair(std::string(exr),(*iter)-1));
		}
		i++;
	}

	int outPutNum = 2 + exMap.size();
	if (outPutNum < 2)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] Not specify trick speed"), _strLogHint.c_str());
		return false;
	}

	RTFProcess* pProcess = (RTFProcess*)ProcessFactory::Create(PROCESS_TYPE_RTF);
	AddFilter(pProcess);
	pProcess->setTrickFile(exMap);
	if (bH264Type)
	{
#if defined(RTFLIB_SDK_VERSION) && (RTFLIB_SDK_VERSION <= 20)
		pProcess->setTrickGenParam(RTF_INDEX_TYPE_VV2, RTF_VIDEO_CODEC_TYPE_H264);
#else
		pProcess->setTrickGenParam(CTF_INDEX_TYPE_VV2, CTF_VIDEO_CODEC_TYPE_H264);
#endif
	}
	
	if (_gCPHCfg.enableTestNTFS)
	{
		NTFSTarget* pTarget[10] = {0};

		for (int i = 0 ; i < 10 ; i++)
			pTarget[i]=(NTFSTarget*)TargetFactory::Create(TARGET_TYPE_NTFS);

		for (int i =0; i < outPutNum;i++)
			if(!AddFilter(pTarget[i]))
				return false;

		std::string strPath = _gCPHCfg.szNTFSOutputDir;
		if(!( strPath[strPath.length()-1]=='\\' || strPath[strPath.length()-1]=='/'))
			strPath+="\\";
		std::string strFile = strPath + strFilename;

		pTarget[0]->setFilename(strFile.c_str());
		if (bH264Type)
		{
			int i = 1;
			strFile = strPath + strFilename + ".vv2";
			pTarget[i++]->setFilename(strFile.c_str());

			//no rte required for h264
			pTarget[0]->enableStreamableEvent(false);


			for (iter = exMap.begin(); iter != exMap.end();iter++)
			{
				strFile = strPath + strFilename + (*iter).first;
				pTarget[i++]->setFilename(strFile.c_str());	
			}	
		}
		else
		{
			int i = 1;
			strFile = strPath + strFilename + ".vvx";
			pTarget[i++]->setFilename(strFile.c_str());

			pTarget[0]->enableStreamableEvent(true);

			for (iter = exMap.begin(); iter != exMap.end();iter++)
			{
				strFile = strPath + strFilename + (*iter).first;
				pTarget[i++]->setFilename(strFile.c_str());	
			}	
		}

		pTarget[0]->enableProgressEvent(true);
		_pMainTarget = pTarget[0];

		InitPins();

		if (!ConnectTo(pSource, 0, pProcess, 0))
			return false;

		for (int i =0; i < outPutNum;i++)
			if (!ConnectTo(pProcess, i, pTarget[i], 0))
				return false;
	}
	else
	{
		VstrmFsTarget* pTarget = (VstrmFsTarget*)TargetFactory::Create(TARGET_TYPE_VSTRMFS);
		if(!AddFilter(pTarget))
			return false;
		pTarget->setFilename(strFilename.c_str());
		pTarget->setCacheDirectory(_gCPHCfg.szCacheDir);
		pTarget->enablePacingTrace(_gCPHCfg.enablePacingTrace);
		pTarget->setBandwidth(nMaxBandwidth);
		pTarget->setVstrmBwClientId(_gCPHCfg.vstrmBwClientId);		
		pTarget->disableBufDrvThrottle(_gCPHCfg.vstrmDisableBufDrvThrottle?true:false);
		pTarget->enableProgressEvent(_gCPHCfg.enableProgEvent);
		pTarget->enableMD5(_gCPHCfg.enableMD5);
		pTarget->setTrickFile(exMap);
		if (bH264Type)
		{
			bool bPacing = true;
			pTarget->setTypeH264();
#if defined(RTFLIB_SDK_VERSION) && (RTFLIB_SDK_VERSION <= 20)
			pTarget->enablePacing(false);
#else
			pTarget->enablePacing(bPacing);
#endif
			pTarget->enableStreamableEvent(false);
		}
		else
		{
			pTarget->enablePacing(true);
			pTarget->enableStreamableEvent(_gCPHCfg.enableStreamEvent);
		}

		_pMainTarget = pTarget;
		InitPins();

		if (!ConnectTo(pSource, 0, pProcess, 0))
			return false;

		for (int i = 0; i < outPutNum; i++)
			if (!ConnectTo(pProcess, i, pTarget, i))
				return false;
	}

	SetMediaSampleSize(_gCPHCfg.mediaSampleSize);

	if (!Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}

	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] init successful"), _strLogHint.c_str());
	return true;
}

bool RTIGen::run()
{
	McastCapSource* pSource = (McastCapSource*)getSourceFilter();
	if (!pSource)
	{
		return false;
	}
	
	pSource->Start();

	bool bRet = Run();
	if (!bRet)
	{
		Close();
		MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] notifyStopped() called"), _strLogHint.c_str());
		return bRet;
	}

	char tmp[64];
	sprintf(tmp, "%lld", _llProcBytes);

	Close();
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] notifyStopped() called"), _strLogHint.c_str());
	return true;
}

void RTIGen::OnProgress(LONGLONG& prcvBytes)
{
}

void RTIGen::OnStreamable(bool bStreamable)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] notifyStreamable() called"), _strLogHint.c_str());
}

void RTIGen::OnMediaInfoParsed(MediaInfo& mInfo)
{
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] OnMediaInfoParsed() called"), _strLogHint.c_str());

	if (_pMainTarget)
	{
		_pMainTarget->setStreamableBytes(mInfo.bitrate*_gCPHCfg.streamReqSecs/8);
	}
}

void usage()
{
	printf("usage: RTIGen -s <udp src url>\n");	
	printf("       -d <destination filename>\n");		
	printf("       -c [configuration file path], default is curent directory: \".\\\" \n");
	printf("       -b [bandwidth]  the trick generation speed in bps, 0 for no limit, default is 0\n");
	printf("       -t [content type]  M for mpeg2, H for H264, default is M\n");
	printf("       -m multiple trick speed specified.eg:\"7.5\" or \"7.5,15,22.5\"\n");
	printf("       -l <specify log file name> default is \"RTFGen.log\"\n");
	printf("       -h display this help\n");	
}

void main(int argc, char** argv)
{
	if (argc <2)
	{
		usage();
		return;                                                                                                                                                                                                                                                                       
	}

	std::string strFile, strCfgdir,strUrl, strContentType,trickspeed,strIp;
	int port,nBandwidth;
	int nTypeH264 = 0;
	strCfgdir = ".\\";
	nBandwidth = 3750000;
	int ch;
	while((ch = getopt(argc, argv, "hHs:S:d:D:b:B:c:C:t:T:m:M:")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
		case 'H':
			usage();
			return;

		case 's':
		case 'S':
			if (optarg)
			{
				strUrl = optarg;
			}
			break;

		case 'd':
		case 'D':
			if (optarg)
			{
				strFile = optarg;
			}
			break;

		case 'b':
		case 'B':
			if (optarg)
			{
				nBandwidth = atoi(optarg);
			}
			break;
		case 't':
		case 'T':
			if (optarg)
			{
				strContentType = optarg;
				if (strContentType=="H" || strContentType=="h")
					nTypeH264 = 1;
			}
			break;

		case 'c':
		case 'C':
			if (optarg)
			{
				strCfgdir = optarg;
			}
			break;
		case 'm':
		case 'M':
			if (optarg)
			{
				trickspeed = optarg;
			}
			break;
		default:
			printf("Error: unknown option %c specified\n\n", ch);
			usage();
			return;
		}
	}

	strIp = strUrl.substr(strUrl.find_first_of(':')+3,strUrl.find_last_of(':')-6);
	std::string strmulticastPort = strUrl.substr(strUrl.find_last_of(':')+1,strUrl.size()-strUrl.find_last_of(':')-1);
    port = atoi(strmulticastPort.c_str());

	InitCPH(strCfgdir);

	{
		RTIGen aaa;
		do
		{
			if (!aaa.preLoad(nBandwidth, strFile,strIp, port, trickspeed,nTypeH264))
			{
				printf("failed to preload with error [%s], check RTIGen.log\n", aaa.GetLastError().c_str());
				break;
			}

			if(!aaa.run())
			{
				printf("failed to run with error [%s], check RTIGen.log\n", aaa.GetLastError().c_str());
			}
			else
			{
				printf("RTI trick generation success\n");
			}
		}while(0);

		aaa.Close();
	}

	UninitCPH();
}
