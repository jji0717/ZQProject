

#include "CPHInc.h"
#include "QueueBufMgr.h"
#include "CPH_NPVRCfg.h"
#include "RTFProc.h"
#include "HostToIP.h"
#include "FileLog.h"
#include "NPVRWrapper.h"
#include "NativeThreadPool.h"
#include "VirtualSessI.h"


using namespace ZQTianShan::ContentProvision;
using namespace ZQ::common;

#define CPH_NPVR			"CPH_NPVR"
#define MOLOG					(glog)

#define RTI_PROVISION_ERRCODE		1

ZQ::common::FileLog					_filelog;

ZQ::common::NativeThreadPool		_pool(2);
QueueBufMgr							_bufmgr;

NPVRWrapper							_nPVRWrapper;

bool InitCPH(std::string& strCfgDir)
{
	_filelog.open("NPVRGen.log", ZQ::common::Log::L_DEBUG, 1);

	// set log handler	
	ZQ::common::setGlogger(&_filelog);
	
	_nPVRWrapper.setLog(&_filelog);
	_nPVRWrapper.setMemoryAllocator(&_bufmgr);
	_nPVRWrapper.setConfigPath(strCfgDir.c_str());
	_nPVRWrapper.setThreadPool(&_pool);

	if (!_nPVRWrapper.initialize())
	{
		return false;
	}

	return true;
}


void UninitCPH()
{
	_nPVRWrapper.uninitialize();
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



void usage()
{
	printf("usage: RTIGen <target filename> <multicast ip> <multicast port> <config file path> [contenttype]\n");
	printf("       [contenttype] M for mpeg2, H for H264, default is M\n");

}

void purecall(void)
{
	int a=0;
	a=1;
}

#include "LeadSessColI.h"
void main(int argc, char** argv)
{
	_set_purecall_handler(purecall);


	if (argc < 5)
	{
		usage();
		return;
	}

	std::string strFile, strCfgdir,strIp, strContentType;
	int port;
	strFile = argv[1];
	strIp = argv[2];
	port = atoi(argv[3]);
	strCfgdir = argv[4];


	int nBandwidth = 0;
	if (argc >= 6)
		nBandwidth = atoi(argv[5]);

	int nTypeH264 = 0;
	if (argc >5)
	{
		strContentType = argv[5];
		if (strContentType=="H" || strContentType=="h")
			nTypeH264 = 1;
	}

	int nDuration = 0;
	if (argc >6)
	{
		nDuration = atoi(argv[6]) * 1000;
	}

	if (strFile[0] == '/' || strFile[0] == '\\')
	{
		std::string fileName = strFile.substr(1,strFile.size()-1);
		strFile = fileName;
	}

	if (!InitCPH(strCfgdir))
	{
		printf("Failed to initialize NPVR, refer to the NPVRGen.log for detail\n");
		return;
	}

	VirtualSessI* pVirtualSess = _nPVRWrapper.generateVirtualSession(nBandwidth, strFile,strIp, port, nTypeH264);
	pVirtualSess->execute();
	Sleep(5000);
	pVirtualSess->uninitialize();
	LeadSessColI::instance()->monitorIdleSession();

	Sleep(160000);
	LeadSessColI::instance()->monitorIdleSession();


/*	do
	{
		VirtualSessI* pVirtualSess = _nPVRWrapper.generateVirtualSession(nBandwidth, strFile,strIp, port, nTypeH264);
		if (!pVirtualSess)
		{
		//	printf("failed to preload with error [%s], check NPVRGen.log\n", aaa.GetLastError().c_str());
			break;
		}

		pVirtualSess->execute();
	
		DWORD dwStart = GetTickCount();
		while(1)
		{
			Sleep(1000);
			LONGLONG procv, total;
			pVirtualSess->getProgress(procv, total);

			printf("Proceeded: %lld bytes\n", procv);

			VirtualSessI::SessionState st = pVirtualSess->getState();
			if (st == VirtualSessI::StateSuccess)
			{
				printf("status: success\n");
				break;
			}
			else if (st == VirtualSessI::StateFailure)
			{
				printf("status: failure\n");
				break;
			}

			if (nDuration && GetTickCount()-dwStart > nDuration)
			{
				pVirtualSess->uninitialize();
				break;
			}
		}
		
		pVirtualSess->uninitialize();		
		
	}while(0);	
*/
	UninitCPH();
}
