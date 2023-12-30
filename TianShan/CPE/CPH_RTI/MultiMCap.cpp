
#include "BaseClass.h"
#include "NTFSSource.h"
#include "NTFSTarget.h"
#include "BaseClass.h"
#include "FileLog.h"
#include "QueueBufMgr.h"
#include "McastCapsrc.h"
#include "NICSelector.h"
#include "getopt.h"
#include "WPCapThread.h"


#define PRINT_CHANNEL_BITRATE


using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define MRTI			"MRTI"
#define MOLOG					(glog)

#define RTI_PROVISION_ERRCODE		1

BOOL WINAPI ConsoleHandler(DWORD CEvent);

ZQ::common::FileLog		filelog;

class RTICap : public ZQTianShan::ContentProvision::BaseGraph, public QueueBufMgr, public ZQ::common::NativeThread
{
public:
	RTICap()
		: _bQuit(false) 
	{
		_bCleaned = false;
	}
	
	~RTICap(){};
	
public:
	
	bool preLoad(std::string& strLocalIp,const std::string& Ip, int port, int capturetimeout, int nSampleSize = 64*1024);

	virtual bool start();

protected:
	virtual int run(void);

	void OnProgress(LONGLONG& prcvBytes);
	void OnStreamable(bool bStreamable);
	void OnMediaInfoParsed(MediaInfo& mInfo);

	void cleanup();
	
	bool _bQuit;
	bool _bCleaned;
};

void InitCPH(std::string& strLocalIp)
{
	filelog.open("MultiMCap.log", ZQ::common::Log::L_DEBUG, 1);

	// set log handler	
	ZQ::common::setGlogger(&filelog);

	//multicast capture initialize
	{
		WinpCapThreadInterface* pCaptureInterface;
		pCaptureInterface = new WinpCapThreadInterface();
		pCaptureInterface->setKernelBufferBytes(192*1024*1024);
		pCaptureInterface->setMinBytesToCopy(768*1024);

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
	
	ZQ::common::setGlogger(NULL);
}

///////////////
#include "UrlStr.h"

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


bool RTICap::preLoad(std::string& strLocalIp, const std::string& Ip, int port, int capturetimeout, int nSampleSize)
{	
	std::string  multicastIp= Ip;
    int multicastPort  = port;

	static int nIndexNum = 0;

	int nCurrentNum = InterlockedIncrement((volatile LONG *)&nIndexNum);
	char szMM[256];
	sprintf(szMM, "%s_%d_%d", Ip.c_str(), port, nIndexNum);

	SetLog(&filelog);
	SetMemAlloc(this);
	SetLogHint(szMM);

    MOLOG(Log::L_INFO, CLOGFMT(MRTI, "[%s] localIp : %s"), _strLogHint.c_str(),strLocalIp.c_str());
	
	DWORD timeoutInterval = capturetimeout;

	McastCapSource* pSource = (McastCapSource*)SourceFactory::Create(SOURCE_TYPE_MCASTCAPSRC);	
	AddFilter(pSource);
	pSource->setInspectPara(multicastIp,multicastPort,timeoutInterval, strLocalIp);
	
	SetMediaSampleSize(nSampleSize);

	if (!Init())
	{
		MOLOG(Log::L_ERROR, CLOGFMT(MRTI, "[%s] Failed to initialize graph with error: %s"), _strLogHint.c_str(), _strLastErr.c_str());
		return false;
	}

	_bCleaned = false;
	MOLOG(Log::L_INFO, CLOGFMT(MRTI, "[%s] init successful"), _strLogHint.c_str());
	return true;
}

int RTICap::run()
{
	MOLOG(Log::L_INFO, CLOGFMT(MRTI, "[%s] data process thread enter"), _strLogHint.c_str());

	McastCapSource* pSource = (McastCapSource*)getSourceFilter();
	pSource->Start();

	DWORD dwLast = GetTickCount();
	DWORD dwLen = 0;
	_bStop = false;
	while(!_bStop)
	{
		MediaSample* pSample = pSource->GetData();
		if (pSample)
		{
#ifdef PRINT_CHANNEL_BITRATE
			dwLen += pSample->getDataLength();

			DWORD dwNow = GetTickCount();
			if (dwNow - dwLast > 4000)
			{
				printf("TID[%d] BitRate[%d]bps\n", GetCurrentThreadId(), int(dwLen*8000.0/(dwNow-dwLast)));
				dwLen = 0;
				dwLast = dwNow;
			}
#endif
			freeMediaSample(pSample);
		}
		else
		{
			break;
		}
	}

	char tmp[64];
	sprintf(tmp, "%lld", _llProcBytes);

	Close();
	MOLOG(Log::L_INFO, CLOGFMT(MRTI, "[%s] data process thread leave"), _strLogHint.c_str());
	return true;
}

void RTICap::OnProgress(LONGLONG& prcvBytes)
{
}

void RTICap::OnStreamable(bool bStreamable)
{
}

void RTICap::OnMediaInfoParsed(MediaInfo& mInfo)
{

}

bool RTICap::start()
{
	return NativeThread::start();
}

void usage()
{
	printf("usage: MutiCap  <-p file_of_capture_parameters> [-t timeout] [-s buffer_size] [-f file_of_localips]\n");	
	printf("       -p the parameter file format:<multicast ip> <multicast port> <bandwidth> <local bind ip>\n");	
	printf("       -t capture time out in seconds, default is 20 seconds\n");
	printf("       -s the buffer size in buffer pool, default is 64*1024 bytes\n");
	printf("       -f file format: <localIp> <total bandwidth>, NIC load balance test\n");	
}

typedef std::vector<RTICap*> RTICaptures;
RTICaptures		_pCaptures;

bool			_bStopCall = false;

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
	switch(CEvent)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		if (!_bStopCall)
		{
			RTICaptures::iterator it = _pCaptures.begin();
			for(;it!=_pCaptures.end();it++)
			{
				(*it)->Stop();
			}

			_bStopCall = true;
		}
		break;

	}
	return TRUE;
}

struct MutiCapParam
{
	std::string		strMulticastIp;
	int				nMulticastPort;
	int				nBandwidth;
	std::string		strLocalIp;
};

typedef std::vector<MutiCapParam> MutiCapParamList;

struct NetInterface
{
	std::string strIp;
	int totalBandwidth;
};
typedef std::vector<NetInterface> NetInterfaces;

bool readParam(const std::string& strFile, MutiCapParamList& multiParams)
{
	FILE* fp = fopen(strFile.c_str(), "r");
	if (!fp)
		return false;

	char szMulIp[256];
	char szLocalIp[256];
	int  nPort, nBandwidth;
	while(!feof(fp))
	{
		fscanf(fp, "%s %d %d %s\n", szMulIp, &nPort, &nBandwidth, szLocalIp);
		MutiCapParam mcp;
		mcp.strMulticastIp = szMulIp;
		mcp.strLocalIp = szLocalIp;
		mcp.nMulticastPort = nPort;
		mcp.nBandwidth = nBandwidth;
		multiParams.push_back(mcp);
	}
	fclose(fp);

	return multiParams.size()>0;
}

bool readLocalIPBandwidth(const std::string& strlocalFile, NetInterfaces& netInterfaces)
{
	FILE* fp = fopen(strlocalFile.c_str(), "r");
	if (!fp)
		return false;

	char szLocalIp[256];
	int  bandwidth;
	while(!feof(fp))
	{
		fscanf(fp, "%s %d\n",szLocalIp , &bandwidth);
		NetInterface nic;
		nic.strIp = szLocalIp;
		nic.totalBandwidth = bandwidth;
		netInterfaces.push_back(nic);
	}
	fclose(fp);

	return netInterfaces.size()>0;
}

void closeAllCap()
{
	RTICaptures::iterator it=_pCaptures.begin();
	for(;it!=_pCaptures.end();it++)
	{
		RTICap* pCap = (*it);
		pCap->Stop();
		pCap->waitHandle(INFINITE);
		pCap->Close();
		delete pCap;
	}

	_pCaptures.clear();
}

bool IsAllCapStop()
{
	RTICaptures::iterator it=_pCaptures.begin();
	for(;it!=_pCaptures.end();it++)
	{
		RTICap* pCap = (*it);
		if (pCap->isRunning())
		{
			return false;
		}
	}

	return true;
}

void main(int argc, char** argv)
{
	if (argc < 2)
	{
		usage();
		return;
	}

	std::string strFileOfParam,strFileOfIPBandwidth;
	int nTimeout= 20;
	int nSampleSize = 64*1024;
	bool bLoadBalance = false;
	int ch;
	while((ch = getopt(argc, argv, "h:H:p:P:t:T:f:F:s:S:")) != EOF)
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
				nSampleSize = atoi(optarg);
			}
			break;
		case 'p':
		case 'P':
			if (optarg)
			{
				strFileOfParam= optarg;
			}
			break;
		case 't':
		case 'T':
			if (optarg)
			{
				nTimeout = atoi(optarg);
			}
			break;
		case 'f':
		case 'F':
			if (optarg)
			{
				strFileOfIPBandwidth = optarg;
				bLoadBalance = true;
			}
			break;
		}
	}

	//
	// read the paramers
	//
	MutiCapParamList  mutiParams;
	if (!readParam(strFileOfParam, mutiParams))
	{
		printf("failed to read the parameter file: %s\n", strFileOfParam.c_str());
		usage();
		return;
	}

	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		printf("Unable to install handler!\n");
	}

	NetInterfaces netInterfaces;
	std::auto_ptr<NetworkIFSelector>  netselector;
	if (bLoadBalance)
	{
		if (!readLocalIPBandwidth(strFileOfIPBandwidth,netInterfaces))
		{
			printf("failed to read the local ip file for bandwidth balance: %s\n", strFileOfIPBandwidth.c_str());
			usage();
			return;
		}
	}

	std::string strlocalIP = mutiParams[0].strLocalIp;
	if (netInterfaces.size() != 0)
	{
		netselector.reset(new NetworkIFSelector());
		if (!netselector.get())
		{
			printf("failed to create NetworkIFSelector object\n");
			return;
		}
		for (NetInterfaces::iterator iter = netInterfaces.begin();
			iter != netInterfaces.end(); iter++)
		{
			netselector->addInterface((*iter).strIp,(*iter).totalBandwidth);
		}
	}
	
	InitCPH(strlocalIP);

	do
	{
		bool bError = false;
		{
			MutiCapParamList::iterator it;
			int nIndex=1;
			for(it=mutiParams.begin();it!=mutiParams.end();it++,nIndex++)
			{
				std::string strlocalIP = it->strLocalIp;
				
				if (bLoadBalance && netInterfaces.size())
				{
					if(!netselector->allocInterface(it->nBandwidth, strlocalIP))
						printf("failed to alloc interface for the session\n");
					else
						printf("capture url[udp://%s:%d/] bandwidth[%d] NIC[%s]\n", it->strMulticastIp.c_str(), it->nMulticastPort, it->nBandwidth, strlocalIP.c_str());
				}
				else
				{
					printf("capture url[udp://%s:%d/] bandwidth[%d] localip[%s]\n", it->strMulticastIp.c_str(), it->nMulticastPort, it->nBandwidth, strlocalIP.c_str());
				}

				RTICap* pCap = new RTICap();
				if (!pCap->preLoad(strlocalIP, it->strMulticastIp, it->nMulticastPort, nTimeout, nSampleSize))
				{
					printf("ParameterLine[%d], failed to preload with error [%s], check RTICap.log\n", 
						nIndex, pCap->GetLastError().c_str());

					bError = true;
					break;
				}

				pCap->start();
				
				_pCaptures.push_back(pCap);
			}
		}

		if (bError)
		{
			break;
		}

		printf("There is %d multicast capture running\n", _pCaptures.size());

		while(!_bStopCall)
		{
			Sleep(1000);
			
			if (IsAllCapStop())
				break;
		}

		if (_bStopCall)
		{
			Sleep(2000);
		}

	}while(0);	

	closeAllCap();

	UninitCPH();

	if (netselector.get())
	{
		netselector.reset(0);
	}
}
