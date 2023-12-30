
#include "BaseClass.h"
#include "NTFSTarget.h"
#include "BaseClass.h"
#include "FileLog.h"
//#include "QueueBufMgr.h"
#include "BufferPool.h"
#include "McastCapSrc.h"
#include "WPCapThread.h"

using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define CPH_RTI			"CPH_RTI"
#define MOLOG					(glog)

#define RTI_PROVISION_ERRCODE		1

#include "getopt.h"
#ifdef ZQ_OS_MSWIN
BOOL WINAPI ConsoleHandler(DWORD CEvent);
#endif

ZQ::common::FileLog		filelog;

class RTICap : public ZQTianShan::ContentProvision::BaseGraph, public ZQ::common::BufferPool
{
public:
	RTICap()
		: BaseGraph(400), _bQuit(false) 
	{
		_bCleaned = false;
		_pMainTarget = NULL;
		initialize(64*1024, 12000, 6000);
	}
	
	~RTICap(){};
	
public:
	
	bool preLoad(std::string& strLocalIp, const std::string& strFilename,const std::string& Ip, int port, int capturetimeout);

	void OnProgress(int64& prcvBytes);
	void OnStreamable(bool bStreamable);
	void OnMediaInfoParsed(MediaInfo& mInfo);

	bool run();
	
protected:
	void cleanup();
	
	bool _bQuit;
	bool _bCleaned;
	ZQTianShan::ContentProvision::BaseTarget*		_pMainTarget;	//main target that will send streamable event
};

void InitCPH(std::string& strLocalIp)
{
	filelog.open("RTICap.log", ZQ::common::Log::L_DEBUG, 1);

	// set log handler	
	ZQ::common::setGlogger(&filelog);

	//multicast capture initialize
	{
		WinpCapThreadInterface* pCaptureInterface;
		pCaptureInterface = new WinpCapThreadInterface();
		pCaptureInterface->setKernelBufferBytes(24*1024*1024);
		pCaptureInterface->setMinBytesToCopy(64*1024);

		MulticastCaptureInterface::setInstance(pCaptureInterface);
		pCaptureInterface->setLog(&filelog);

		pCaptureInterface->addNIC(strLocalIp);

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
	if (&filelog == ZQ::common::getGlogger())
		ZQ::common::setGlogger(NULL);
//	ZQ::common::setGlogger(NULL);
}

///////////////
#include "urlstr.h"
#ifdef ZQ_OS_MSWIN
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
#endif

bool RTICap::preLoad(std::string& strLocalIp, const std::string& strFilename,const std::string& Ip, int port, int capturetimeout)
{	
	std::string  multicastIp= Ip;
    int multicastPort  = port;

	SetLog(&filelog);
	SetMemAlloc(this);
	SetLogHint(strFilename.c_str());
	SetMaxAllocSampleCount(12000);

	int timeoutInterval = capturetimeout;

	McastCapSource* pSource = (McastCapSource*)SourceFactory::Create(SOURCE_TYPE_MCASTCAPSRC);
	AddFilter(pSource);
	pSource->setInspectPara(multicastIp,multicastPort,timeoutInterval, strLocalIp);
	//dumper parameters
	pSource->enableDump(true);
	pSource->deleteDumpOnSuccess(false);
	SetMediaSampleSize(64*1024);

	if (!Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(CPH_RTI, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}

	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] init successful"), _strLogHint.c_str());
	return true;
}

bool RTICap::run()
{
	McastCapSource* pSource = (McastCapSource*)getSourceFilter();
	if (!pSource)
	{
		return false;
	}
	
	pSource->Start();

	_bStop = false;
	while(!_bStop)
	{
		MediaSample* pSample = pSource->GetData();
		if (pSample)
		{
			freeMediaSample(pSample);
		}
		else
		{
			break;
		}
	}

	char tmp[64];
	snprintf(tmp, sizeof(tmp)-2, FMT64, _llProcBytes);

	Close();
	MOLOG(Log::L_INFO, CLOGFMT(CPH_RTI, "[%s] notifyStopped() called"), _strLogHint.c_str());
	return !IsErrorOccurred();
}

void RTICap::OnProgress(int64& prcvBytes)
{
}

void RTICap::OnStreamable(bool bStreamable)
{
}

void RTICap::OnMediaInfoParsed(MediaInfo& mInfo)
{

}

void usage()
{
	printf("usage: RTICap <filename to save> <multicast ip> <multicast port> <local bind ip>  [timeout]\n");
	printf("       [timeout] capture time out in seconds, default is 20 seconds\n");

}
RTICap* _pCap = NULL;
#ifdef ZQ_OS_MSWIN
BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
	switch(CEvent)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		if (_pCap)
		{
			_pCap->Stop();
		}
		break;

	}
	return TRUE;
}
#endif
int  main(int argc, char** argv)
{
	if (argc < 5)
	{
		usage();
		return 0;
	}

	std::string strFile, strIp, strLocalIp;
	int port;
	int nTimeout= 20;
	strFile = argv[1];
	strIp = argv[2];
	port = atoi(argv[3]);
	strLocalIp = argv[4];

	if (argc >= 6)
		nTimeout = atoi(argv[5]);

#ifdef ZQ_OS_MSWIN
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		printf("Unable to install handler!\n");
	}
#endif
	InitCPH(strLocalIp);

	do
	{
		RTICap aaa;
		_pCap = &aaa;
		if (!aaa.preLoad(strLocalIp, strFile, strIp, port, nTimeout))
		{
			printf("failed to preload with error [%s], check RTICap.log\n", aaa.GetLastError().c_str());
			break;
		}


		if(!aaa.run())
		{
			printf("failed to run with error [%s], check RTICap.log\n", aaa.GetLastError().c_str());
		}
		else
		{
			printf("RTI trick generation success\n");
		}

		aaa.Close();
		_pCap = NULL;
	}while(0);	

	UninitCPH();
        return 1;
}
