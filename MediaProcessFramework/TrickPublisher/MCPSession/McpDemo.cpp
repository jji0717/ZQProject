// McpDemo.cpp : Defines the entry point for the console application.
//

#include"McpDemo.h"
#include "McpSession.h"
#include "getopt.h"
//#include "ScReporter.h"
#include "thread.h"
#include "sclog.h"



extern "C" {
//	extern int getopt(int argc, char **argv, char *opts);
//	extern char *optarg;
}

struct _subscriber
{
	wchar_t awcName[256];
	wchar_t awcFilename[_MAX_PATH];
	wchar_t awcIface[256];
	

	MCP_HANDLE tHandle;
	MCP_STATUS tBindStatus;
	MCP_STATUS tSubscribeStatus;
	MCP_STATUS tStatus;
	MCP_CUR_STATE tState;

	wchar_t awcFatalReason[256];
} atSubList[128] = {0};

int iSubCount = 0;
int iFailedTransfers = 0;
unsigned long ulSport = 5000;
unsigned long ulDport = 6000;
unsigned long ulBitrate = 3750000;
unsigned long ulEventInterval = 1000;
bool bVerbose = false;
unsigned long ulSubXferTimeout = 0;
bool bVodpkg = true;
bool bZerolenFail = true;
unsigned long ulMaxWaitSecs = 2;
unsigned long ulRteStreamVersion = 2;
bool bVodpkgDeleteOnFail = true;
bool bEnableServerSideVodpkgDelete = true;
bool bTxSpeedFiles = false;
bool bVvtTransfer = false;

unsigned long ulContentBitrate = 0;
unsigned int uiContentHorizRes = 0;
unsigned int uiContentVerticalRes = 0;

wchar_t awcLogName[_MAX_PATH] = L"multidest.log";
wchar_t awcMcastAddr[16] = L"225.20.10.5";

wchar_t awcPubName[256] = L"";
wchar_t awcPubFilename[_MAX_PATH] = L"";
wchar_t awcPubIface[16] = L"";
wchar_t awcPubGSI[MCP_GSID_SIZE];

MCP_HANDLE tPubHandle;
MCP_STATUS tPubStatus;
MCP_CUR_STATE tPubState;

unsigned long ulBriefPrivdatLen = 0;
unsigned long ulFullPrivdatLen = 0;

char acFullPriv[2000 + 1];

// always 0 because we're live transfer
ULONGLONG ullObjSize = 0;

unsigned char aucTmpBriefPriv[24] = {0};
unsigned char aucFinalBriefPriv[12] = {0};

DWORD MCASTOptionMask = MCP_OPTM_NONE;
MCP_XFER_TYPE MCASTXferType = MCP_XFER_LIVE;

//MCP_RTE_STREAM_VERSION strVer = MCP_RTE_STREAM_VERSION_ONE;
MCP_RTE_STREAM_VERSION strVer = MCP_RTE_STREAM_VERSION_TWO;

MCP_RTE_STREAM_ECO strEco = MCP_RTE_STREAM_ECO_ZERO;

DWORD MCASTRteZerolenFail = 0;
DWORD MCASTDisableBufDrvThrottle = 0;
DWORD MCASTServerSideDeleteOnFail = 0;
/*****************************************************************************/
/*
*/
/*****************************************************************************/
unsigned char
ConvertChar(char in)
{
	switch(in)
	{
	case '0':
		return 0;
	case '1':
		return 1;
	case '2':
		return 2;
	case '3':
		return 3;
	case '4':
		return 4;
	case '5':
		return 5;
	case '6':
		return 6;
	case '7':
		return 7;
	case '8':
		return 8;
	case '9':
		return 9;
	case 'A':
	case 'a':
		return 10;
	case 'B':
	case 'b':
		return 11;
	case 'C':
	case 'c':
		return 12;
	case 'D':
	case 'd':
		return 13;
	case 'E':
	case 'e':
		return 14;
	case 'F':
	case 'f':
		return 15;
	}
	return 0;
}
/*****************************************************************************/
/*
*/
/*****************************************************************************/
void Usage(void)
{
	wprintf(L"Usage: McpDemo\n");
	wprintf(L"\t-a source port (default:%d)\n", ulSport);
	wprintf(L"\t-b destination port (default:%d)\n", ulDport);
	wprintf(L"\t-c max bitrate in bits per second (default:%d)\n", ulBitrate);
	wprintf(L"\t-d logfile name (default: %s)\n", awcLogName);
	wprintf(L"\t-e event update interval (default: %d)\n", ulEventInterval);
	wprintf(L"\t-f RTE stream version <1 or 2> (default: 2)\n");
	wprintf(L"\t-g disable vodpkg subscriber delete on fail\n");
	wprintf(L"\t-h help\n");
	wprintf(L"\t-i disable client-side vodpkg\n");
	wprintf(L"\t-j subscriber live transfer timeout in seconds (default: not set)\n");
	wprintf(L"\t-k disable bufdrv throttling\n");
	wprintf(L"\t-l disable server-side vodpkg delete\n");
	wprintf(L"\t-m multicast address (default: %s)\n", awcMcastAddr);
	wprintf(L"\t-n seconds to wait for subs to finish after pub done (default:%d)\n", ulMaxWaitSecs);
	wprintf(L"\t-o transfer only .mpg and .vvx files (default: disabled)\n");
	wprintf(L"\t-p publisher, -p <name or ip address>,<filename>,[interface]\n");
	wprintf(L"\t-q content bitrate, used for brief private data\n");
	wprintf(L"\t-r enable VVT transfer\n");
	wprintf(L"\t-s subscriber, -s <name or ip address>,<filename>,[interface]\n");
	wprintf(L"\t-t content horizontal resolution, used for brief private data\n");
	wprintf(L"\t-u content vertical resolution, used for brief private data\n");
	wprintf(L"\t-v verbose\n");
	wprintf(L"\t-w brief private data length (must use with -x)\n");
	wprintf(L"\t-x brief private data string (max 12 HEX character pairs)\n");
	wprintf(L"\t\t -> for example -x bd01393839006001e00100 -w 11\n");
	wprintf(L"\t\t -> means bitrate: 3749945, hres 352, vres 480\n");
	wprintf(L"\t-y full private data string (max 2000 characters)\n");
	wprintf(L"\t-z disable publisher zerolen fail\n");

	wprintf(L"\n");

	exit(0);
}
/*****************************************************************************/
/*
*/
/*****************************************************************************/
void 
GetArgs(int argc, char *argv[])
{
	int c;
	int opterr = 0;

	if( 1 == argc)

	{
		Usage();
	}

	while((c = getopt(argc, argv, "a:b:c:d:e:f:ghij:klm:n:op:q:rs:t:u:vw:x:y:z")) != -1)
	{
		switch(c)
		{
		case 'a':
			// set source port
			ulSport = atol(optarg);
			break;

		case 'b':
			// set destination port
			ulDport = atol(optarg);
			break;

		case 'c':
			// set bitrate
			ulBitrate = atol(optarg);
			break;

		case 'd':
			// set log name
			mbstowcs(awcLogName, optarg, strlen(optarg) + 1);
			break;

		case 'e':
			// set event reporting interval
			ulEventInterval = atol(optarg);
			break;

		case 'f':
			// set RTE stream version
			ulRteStreamVersion = atol(optarg);
			break;
		case 'g':
			// disable delete on fail
			bVodpkgDeleteOnFail = false;
			break;

		case 'h':
			// print help, exit
			Usage();

		case 'i':
			// don't use client-side vodpkg transfers
			bVodpkg = false;
			break;

		case 'j':
			// set sub liveXfer timeout
			ulSubXferTimeout = atoi(optarg);
			break;

		case 'k':
			// disable bufdrv throttle
			MCASTDisableBufDrvThrottle = 1;
			break;

		case 'l':
			// disable server-side vodpkg delete
			bEnableServerSideVodpkgDelete = false;
			break;

		case 'm':
			// set mcast addr
			mbstowcs(awcMcastAddr, optarg, strlen(optarg) + 1);
			break;

		case 'n':
			// max seconds after pub finishes
			ulMaxWaitSecs = atol(optarg);
			break;

		case 'o':
			// don't transfer speed files, .mpg & .vvx only
			bTxSpeedFiles = false;
			break;

		case 'p':
			{
				// set pub IP, filename, interface
				wchar_t awcTmp[512];
				mbstowcs(awcTmp, optarg, strlen(optarg) + 1);

				wchar_t *pwcComma = wcsstr(awcTmp, L",");
				if(pwcComma != 0)
				{
					*pwcComma = L'\0';
					wcscpy(awcPubName, awcTmp);

					wchar_t *pwcComma2 = wcsstr(pwcComma + 1, L",");
					if(pwcComma2 != 0)
					{
						*pwcComma2 = L'\0';
						wcscpy(awcPubFilename, pwcComma + 1);

						wcscpy(awcPubIface, pwcComma2 + 1);			
					}
					else
					{
						wcscpy(awcPubFilename, pwcComma + 1);
					}
				}
				else
				{
					wcscpy(awcPubName, awcTmp);
				}

				break;
			}

		case 'q':
			ulContentBitrate = atol(optarg);
			break;

		case 'r':
			bVvtTransfer = true;
			break;

		case 's':
			{
				wchar_t awcTmp[512];
				mbstowcs(awcTmp, optarg, strlen(optarg) + 1);

				wchar_t *pwcComma = wcsstr(awcTmp, L",");
				if(pwcComma != 0)
				{
					*pwcComma = L'\0';
					wcscpy(atSubList[iSubCount].awcName, awcTmp);

					wchar_t *pwcComma2 = wcsstr(pwcComma + 1, L",");
					if(pwcComma2 != 0)
					{
						*pwcComma2 = L'\0';
						wcscpy(atSubList[iSubCount].awcFilename, pwcComma + 1);

						wcscpy(atSubList[iSubCount].awcIface, pwcComma2 + 1);			
					}
					else
					{
						wcscpy(atSubList[iSubCount].awcFilename, pwcComma + 1);
					}
				}
				else
				{
					wcscpy(atSubList[iSubCount].awcName, awcTmp);
				}

				iSubCount++;

				break;
			}
		case 't':
			uiContentHorizRes = atoi(optarg);
			break;

		case 'u':
			uiContentVerticalRes = atoi(optarg);
			break;

		case 'v':
			// set verbose
			bVerbose = true;

			break;

		case 'w':
			// brief priv dat len
			ulBriefPrivdatLen = atol(optarg);
			break;

		case 'x':
			{
				// convert brief private data on the spot
				int i, j;

				for(i = 0; optarg[i] != 0; i++)
				{
					aucTmpBriefPriv[i] = ConvertChar(optarg[i]);
				}

				ulBriefPrivdatLen = 0;
				i = j = 0;
				do
				{
					aucFinalBriefPriv[j] = aucTmpBriefPriv[i++] << 4;
					aucFinalBriefPriv[j] |= aucTmpBriefPriv[i++];

					j++;

				} while(i < 24);

				break;
			}
		case 'y':
			// full private data
			strcpy(acFullPriv, optarg);
			ulFullPrivdatLen = strlen(acFullPriv);

			break;

		case 'z':
			// disable zerolen fail
			bZerolenFail = false;
			break;

		case '?':
			// unknown option
			opterr++;
		}
	}

	if(opterr)
	{
		Usage();
	}

	if(bVerbose)
	{
		//		printf("Version %d.%d build %d\n",
		//			VER_PRODUCTVERSION_MAJOR, VER_PRODUCTVERSION_MINOR, VER_PRODUCTBUILD);
		//		printf("Baselevel: %s, %s %s\n", VER_PRODUCTVERSION_STR, __DATE__, __TIME__);

		// log input params
		wprintf(L"Multidest using the following parameters:\n");
		wprintf(L"\tsource port (-a): %d\n", ulSport);
		wprintf(L"\tdestination port (-b): %d\n", ulDport);
		wprintf(L"\tbitrate (-c): %d\n", ulBitrate);
		wprintf(L"\tlogfile name (-d): %s\n", awcLogName);
		wprintf(L"\tevent update interval (-e): %d\n", ulEventInterval);
		wprintf(L"\tRTE stream version (-f): %d\n", ulRteStreamVersion);
		wprintf(L"\tVodPkg delete on fail (-g): %s\n",
			true == bVodpkgDeleteOnFail ? L"enabled" : L"disabled");

		if(false == bVodpkg)
			wprintf(L"\tclient-side vodpkg (-i): disabled\n");

		if(ulSubXferTimeout)
			wprintf(L"\tsubscriber live tx timeout (-j): %d\n", ulSubXferTimeout);

		wprintf(L"\tDisableBufDrvThrottle (-k): %d\n", MCASTDisableBufDrvThrottle);
		wprintf(L"\tserver-side vodpkg delete on fail (-l): %s\n",
			true == bEnableServerSideVodpkgDelete ? L"enabled" : L"disabled");
		wprintf(L"\tmulticast address (-m): %s\n", awcMcastAddr);
		wprintf(L"\tmax secs to wait after pub complete (-n): %d\n", ulMaxWaitSecs);
		wprintf(L"\ttransfer only .mpg and.vvx (-o): %s\n",
			true == bTxSpeedFiles ? L"disabled" : L"enabled");

		wprintf(L"\tpublisher (-p): %s,%s,%s\n",
			awcPubName[0] ? awcPubName : L"<empty>",
			awcPubFilename[0] ? awcPubFilename : L"<empty>",
			awcPubIface[0] ? awcPubIface : L"<empty>");

		if(ulContentBitrate)
			wprintf(L"\tcontent bitrate: %d\n", ulContentBitrate);

		wprintf(L"\tVVT transfer (-r): %s\n",
			true == bVvtTransfer ? L"enabled" : L"disabled");

		for(int i = 0; i < iSubCount; i++)
		{
			struct _subscriber *ptSub = &atSubList[i];

			wprintf(L"\tsubscriber (-s): %s,%s,%s\n",
				ptSub->awcName[0] ? ptSub->awcName : L"<empty>",
				ptSub->awcFilename[0] ? ptSub->awcFilename : L"<empty>",
				ptSub->awcIface[0] ? ptSub->awcIface : L"<empty>");
		}

		if(uiContentHorizRes)
			wprintf(L"\tcontent horizontal res: %d\n", uiContentHorizRes);

		if(uiContentVerticalRes)
			wprintf(L"\tcontent vertical res: %d\n", uiContentVerticalRes);

		if(false == bZerolenFail)
			wprintf(L"\tzerolen fail (-z): disabled\n");
	}
}
ZQ::common::ThreadPool gMcpThreadPool;
ZQ::common::ScLog *gpScLog;
int main(int argc ,char* argv[])
{
//	ZQ::common::ScReporter progReporter("MCP Session DEMO");
//	pProglog = &progReporter;
//	ZQ::common::setGlogger(&progReporter);
	GetArgs(argc,argv);
	
	MCPSetLogName(L"c:\\Mcp.log");
	MCPSetAppLogFlush(1);
	int runTime = 0;
	McpSession* pSession = NULL;
	ZQ::common::ScLog sclog(awcLogName,ZQ::common::Log::L_DEBUG,4*1024*1024);
	gpScLog = &sclog;
	(*gpScLog)(ZQ::common::Log::L_DEBUG,L"**************************MCP DEMO TEST PROGRAM LAUNCH************************************");
	while(runTime < 1)
	{
		runTime ++;
		pSession = new McpSession(gMcpThreadPool);

		pSession->setVODpkgAttr(bVodpkg,
								bVodpkgDeleteOnFail,
								bTxSpeedFiles,
								bVvtTransfer,
								bEnableServerSideVodpkgDelete);
		pSession->setXferAttr(MCASTXferType,
								ulBitrate,
								ulRteStreamVersion,
								bZerolenFail,
								ulSubXferTimeout);
	
		pSession->setMediaAttr(awcPubFilename,
							ulContentBitrate,
							uiContentHorizRes,
							uiContentVerticalRes,
							(char*)aucTmpBriefPriv,
							acFullPriv);
		//wchar_t buf[256];
		//wsprintf(buf,L"Set Media file as %s",L"C:\\ISAUpload\\MPGfile\\short.mpg");
		(*gpScLog)(ZQ::common::Log::L_DEBUG,L"***************************%d************************************",runTime);

		pSession->setMcpAttr(awcMcastAddr,ulEventInterval,ulMaxWaitSecs);

		
		pSession->setPublisher(awcPubName,ulSport,awcPubIface);

		for(int i =0 ; i < iSubCount ; i ++)
		{
			
			pSession->addSubscriber(atSubList[i].awcFilename,atSubList[i].awcName,ulDport,atSubList[i].awcIface);
		}		
		pSession->start();
		


//	Sleep(20000);
//	pSession->terminate();
	
//	pSession->close();
//	delete pSession;
		bool bLoop = true;
		while (bLoop)
		{
			Sleep(5000);
			MCPSESSIONSTATUS status = pSession->getCurStatus();
			if(( MCP_SESSION_FAILED == status)||( MCP_SESSION_CLOSED == status))
			{
	//			Sleep(5000);
	//			add your code to dispose the mcp session 
				if(status == MCP_SESSION_CLOSED)
				{
					wprintf(L"Success to MCP runtime(%d).\n",runTime);

				}
				if(status == MCP_SESSION_FAILED)
				{
					std::wstring Err = pSession->getLatestErrMsg();
					wprintf(L"%s rutime(%d)\n",Err.c_str(),runTime);
				}
				delete pSession;
				bLoop = false;
			}
		}
		Sleep(4000);
	}
}
/*
int main1(int argc, char* argv[])
{
	GetArgs(argc, argv);
	MCPSetLogName(L"Mcp.log");
	MCPSetAppLogFlush(1);
	
	McpSession* pSession = new McpSession;
	char buf[256];
	std::wstring wsBuf = awcPubName;
	wcstombs(buf,wsBuf.data(),wsBuf.length());
	buf[wsBuf.length()] = '\0';
	char buf1[256];
	wsBuf = awcPubIface;
	wcstombs(buf,wsBuf.data(),wsBuf.length());
	buf1[wsBuf.length()] = '\0';
	pSession->setPublisher(buf,ulSport,buf1);
	char buf2[256];

	for(int i = 0 ; i < iSubCount; i++)
	{
		memset(buf,0,256);
		memset(buf1,0,256);
		memset(buf2,0,256);
		wsBuf = atSubList[i].awcName;
		wcstombs(buf,wsBuf.data(),wsBuf.length());
		buf[wsBuf.length()] = '\0';
		wsBuf = atSubList[i].awcIface;
		wcstombs(buf1,wsBuf.data(),wsBuf.length());
		buf1[wsBuf.length()] = '\0';
		wsBuf = atSubList[i].awcFilename;
		wcstombs(buf2,wsBuf.data(),wsBuf.length());
		buf2[wsBuf.length()] = '\0';		
		pSession->addSubscriber(buf2,buf,ulDport,buf1);
	}
	
	pSession->setVODpkgAttr(bVodpkg,
							bVodpkgDeleteOnFail,
							bTxSpeedFiles,
							bVvtTransfer,
							bEnableServerSideVodpkgDelete);
	pSession->setXferAttr(MCASTXferType,
							ulBitrate,
							ulRteStreamVersion,
							bZerolenFail,
							ulSubXferTimeout);
	memset(buf,0,256);
	wsBuf = awcMcastAddr;
	wcstombs(buf,wsBuf.data(),wsBuf.length());
	buf[wsBuf.length()] = '\0';
	pSession->setMcpAttr(buf,ulEventInterval,ulMaxWaitSecs);
	memset(buf,0,256);
	wsBuf = awcPubFilename;
	wcstombs(buf,wsBuf.data(),wsBuf.length());	  
	pSession->setMediaAttr(buf,						
						ulContentBitrate,
						uiContentHorizRes,
						uiContentVerticalRes,
						(LPCTSTR)aucFinalBriefPriv,
						acFullPriv);
	pSession->start();
	return 0;
}
*/