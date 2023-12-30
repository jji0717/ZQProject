// ===========================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: NoiseSim.cpp,v 1.1 2005-05-17 15:30:26 Ouyang Exp $
// Branch: $Name:  $
// Author: Lin Ouyang
// Desc  : the main file for NoiseSim
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/IngestSim/Ingestsim.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 11    06-10-23 11:56 Fei.huang
// 
// 10    06-10-20 20:01 Fei.huang
// 
// 9     06-04-06 17:21 Bernie.zhao
// fixed bug of local file write
// 
// 8     06-03-16 17:09 Bernie.zhao
// use WinPcap to capture multicast
// 
// 7     05-10-20 14:54 Jie.zhang
// 
// 6     05-09-26 7:02 Jie.zhang
// 
// 5     05-09-26 3:48 Jie.zhang
// performance turning, and add save file to local machine (-f)
// 
// 4     9/07/05 2:10p Hui.shao
// 
// 3     05-08-31 16:41 Mei.zhang
// 
// 2     05-08-31 16:20 Build
// 
// 1     05-08-26 17:04 Lin.ouyang
// init version
// Revision 1.1  2005-08-22 15:30:26  Ouyang
// initial created
// ===========================================================================


/// @file "IngestSim.cpp"
/// @brief the main file for IngestSim project
/// @author (Lorenzo) Lin Ouyang
/// @date 2005-8-22 15:18
/// @version 0.1.0
//
// IngestSim.cpp: Main file for project IngestSim.
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "afx.h"
#include <afxwin.h>
#include <afxinet.h>
#include "FtpTransfer.h"
#include <deque>

#include <iostream>
#include <fstream>
#include "Socket.h"
#include "getopt.h"
#include "McastUdpSocket.h"

#ifdef USE_WPCAP
#	include "McastUdpCatcher.h"
#endif

#include <signal.h>
#include "sclog.h"

using namespace std;

/// Default stream source IP
#define SOURCE_IP "225.25.25.25"
/// Default stream source port
#define SOURCE_PORT 1234
/// Default destination ftp IP
#define DEST_IP "127.0.0.1"
/// Default destination ftp port
#define DEST_PORT 21
/// Default message size
#define MSG_SIZE 8192
/// Default log file
#define LOG_FILE "c:\\IngestSim.log"


/// Print usage message
void usage();

/// thread entry function
DWORD WINAPI ThreadFunc( LPVOID lpParam );

/// create a thread for menu
bool createMenuThread(HANDLE *phThread);

/// global variable
typedef enum enMenuOption
{
	MO_EXIT,
	MO_NOTSELECT,
	MO_NOMORE
} MenuOption;

// for menu thread
MenuOption g_mo = MO_NOTSELECT;
int g_iSockFamily = AF_INET;
int g_iSockType = SOCK_DGRAM;
int g_iSockProtocol = 0;
string g_strRemoteIP = SOURCE_IP;
int g_iRemotePort = SOURCE_PORT;


extern void LogMsg(DWORD dwTraceLevel, LPCTSTR lpszFmt, ...);


// for ctrl+c hook
BOOL bTerminated = FALSE;
void signal_handler(int signal_value);

int main(int argc, char* argv[])
{
	// install Ctrl-C signal
	signal(SIGINT, signal_handler);
	
	//
	// init MFC wininet
	//
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
	  // catastropic error! MFC can't initialize
	  return -1;
	}

	// define variable
	init_WSA init_wsa;
	Socket *sockRecv;// *sockSend;

#ifdef USE_WPCAP
	McastUdpCatcher	*pCatcher = NULL;	// wpcap packet catcher
	char	errBuffer[260] = {0};
	int		errLen = 260;
#endif

	//
	// config value
	//
	int cfg_iMsgSize = MSG_SIZE;
	string cfg_strSockFamily = "inet";
	string cfg_strSockType = "udp";
	string cfg_strSockProtocol = "0";
	string cfg_strSrcIP = SOURCE_IP;
	string cfg_strDestIP = DEST_IP;
	int cfg_iSrcPort = SOURCE_PORT;
	int cfg_iDestPort = DEST_PORT;

	int iSockFamily;
	int iSockType;
	int iSockProtocol;

	string strSrc = SOURCE_IP;
	string strDst;
	string strLogFile = LOG_FILE;
	string strTimeout;
	string strTiming;

	string strBindIP="0.0.0.0";

	bool bEnableLocalSave=false;

	string strFilename;	// filename for local save
	
	int iRet = 0;

	//
	// get command line arguments
	//
	if(argc == 1)
	{
		usage();
		return -1;
	}

	int ch;
	while((ch = getopt(argc, argv, "hs:b:d:f:l:t:T:")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
			usage();
			return -1;

		case 'b':
			strBindIP = optarg;
			break;
			
		case 's':
			strSrc = optarg;
			break;

		case 'd':
			strDst = optarg;
			break;

		case 'f':
			strFilename = optarg;
			bEnableLocalSave = true;
			break;

		case 'l':
			strLogFile = optarg;
			break;

		case 't':
			strTimeout = optarg;
			break;
		
		case 'T':
			strTiming = optarg;
			break;
			
		default:
			usage();
			printf("Error: unknown option %c specified\n", ch);
			return -1;
		}
	}

	// init WSA
	if(!init_wsa.init())
	{
		cout << "WSAStartup error" << endl;
		return -1;
	}

	//
	// set socket attribute
	//
	// socket family
	if(cfg_strSockFamily == "inet")
		iSockFamily = AF_INET;

	// socket type
	if(cfg_strSockType == "udp")
		iSockType = SOCK_DGRAM;
	else if(cfg_strSockType == "tcp")
		iSockType = SOCK_STREAM;
	else
		iSockType = SOCK_RAW;

	// socket protocol
	if(cfg_strSockProtocol == "0")
		iSockProtocol = 0;

	// source host
	string::size_type idx;
	idx = strSrc.find(':');
	if(string::npos == idx)		// didn't find ':', only have ip, use default port 1234
	{
		cfg_strSrcIP = strSrc;
	}
	else
	{
		cfg_strSrcIP = strSrc.substr(0, idx);
		cfg_iSrcPort = atoi(strSrc.substr(idx+1).c_str());
		if(cfg_iSrcPort == 0)
		{
			cout << "Error: covert source port to int" << endl;
			return -1;
		}
	}
/*
	// destination host
	idx = strDst.find(':');
	if(string::npos == idx)		// didn't find ':', only have ip, use default port 21
	{
		cfg_strDestIP = strDst;
	}
	else
	{
		cfg_strDestIP = strDst.substr(0, idx);
		cfg_iDestPort = atoi(strDst.substr(idx+1).c_str());
		if(cfg_iDestPort == 0)
		{
			cout << "Error: covert destination port to int" << endl;
			return -1;
		}
	}
*/

	// timeout value
	timeval tv, *ptv  = NULL;		// timeout
	int iTimeout;
	if(!strTimeout.empty())
	{
		iTimeout = atoi(strTimeout.c_str());
		if(0 == iTimeout)	// can not convert strTimeout to int
		{
			printf("timeout time error, use default 15 sec\n");
			iTimeout = 15;
		}

		tv.tv_sec =  iTimeout;
		tv.tv_usec = 0;

		ptv = &tv;
	}
	else
	{
		// set default to 15 sec
		tv.tv_sec = 15;
		tv.tv_usec = 0;
		ptv = &tv;
	}
	
	// Timing to stop capturing packets
	int iTiming = 0;
	if(!strTiming.empty())
	{
		iTiming = atoi(strTiming.c_str());
		if(0 == iTiming)
		{
			printf("timing value error, fall back to defaults\n");
		}
		
	}
	
	//
	// judge the ip address type, multicast/unicast ip
	//
	unsigned int iIpClass;
	char szIpClass[4];

	idx = cfg_strSrcIP.find('.');
	if(string::npos == idx)
	{
		cout << "recv_ip error" << endl;
		return false;
	}
	strcpy(szIpClass, cfg_strSrcIP.substr(0, idx).c_str());
	iIpClass = atoi(szIpClass);
	if(iIpClass == 0)
	{
		cout << "Error: convert first field of ip to int" << endl;
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////
#ifdef USE_WPCAP

	pCatcher = new McastUdpCatcher();

	// bind to local adapter
	iRet = pCatcher->Bind(strBindIP.c_str());
	if(iRet)
	{
		errLen = 260;
		iRet = pCatcher->GetLastErr(errBuffer, &errLen);
		if(iRet)
			cout << "failed to bind to local address" << endl;
		else
			cout << "failed to bind to local address, details:" << endl << errBuffer << endl;
		return -1;
	}

	// open the catcher and listening to dest address
	iRet = pCatcher->Open(cfg_strSrcIP.c_str(), cfg_iSrcPort);
	if(iRet)
	{
		errLen = 260;
		iRet = pCatcher->GetLastErr(errBuffer, &errLen);
		if(iRet)
			cout << "failed to listening to destination address" << endl;
		else
			cout << "failed to listening to destination address, details:" << endl << errBuffer << endl;
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////
#else	// if not USE_WPCAP, using native socket

	// init recv socket according recv_ip
	char szMode[64];
	memset(szMode, 0, sizeof(szMode));
	// it's unicast
	if(iIpClass < 224)
	{
		sockRecv = new Socket;
		sprintf(szMode, "recv socket is unicast socket");
	}
	else
	{
		// between 224 ~ 239, it's multicast
		if(iIpClass <= 239)
		{
			sockRecv = new McastUdpSocket;
			((McastUdpSocket*)sockRecv)->setMulticastAddr(cfg_strSrcIP.c_str());
			sprintf(szMode, "recv socket is multicast socket");
		}
		else
		{
			cout << "recv_ip error" << endl;
			return -1;
		}
	}
	cout << endl;

	// set socket attrib for menu thread
	g_iSockFamily = iSockFamily;
	g_iSockType = iSockType;
	g_iSockProtocol = iSockProtocol;
	g_strRemoteIP = cfg_strSrcIP;
	g_iRemotePort = cfg_iSrcPort;

	// set sockRecv attrib
	sockRecv->setDomain(iSockFamily);
	sockRecv->setSockType(iSockType);
	sockRecv->setProtocol(iSockProtocol);
	sockRecv->setLocalAddr(strBindIP.c_str());
	sockRecv->setLocalPort(cfg_iSrcPort);

	if(!sockRecv->SocketCreate())
	{
		cout << "Socket create error" << endl;
		return -1;
	}

	if(!sockRecv->Bind())
	{
		cout << "Socket bind error" << endl;
		return -1;
	}

	// print socket information
	cout << "recv socket: " << endl;
	sockRecv->PrintSocket();
	cout << "Multicast addr: " <<cfg_strSrcIP<< endl;
	cout << endl;
	cout << szMode << endl;

#if 1
	// set recv buffer size
	{
		int size = 4096*16;
		int status = setsockopt(sockRecv->getSocket(), SOL_SOCKET, SO_RCVBUF, (char *)&size, sizeof(int));
		if (status == SOCKET_ERROR)
		{
			status = WSAGetLastError();
		}
	}
#endif

#endif	// USE_WPCAP
	//////////////////////////////////////////////////////////////////////////
	



	// create menu thread
	cout << endl << "press CTRL+C to quit" << endl;

	// set log
	ZQ::common::pGlog = new ZQ::common::ScLog(strLogFile.c_str(), 7, 4*1024*1024);

	void *buff = NULL;
	int iLen = 0;
	FtpPush_Queue ftpPush;

	// start to push
	bool bRet = ftpPush.StartPush(strDst.c_str());
	if(!bRet)
	{
		cout << "FtpPushBuff_Queue::StartPush error" << endl;
		cout << ftpPush.getErrorDesc() << endl;

		if (ZQ::common::pGlog != NULL && ZQ::common::pGlog != &ZQ::common::NullLogger)
		{
			delete ZQ::common::pGlog;
			ZQ::common::pGlog = &ZQ::common::NullLogger;
		}

		return -1;
	}


	HANDLE hLocalFile = NULL;
	if (bEnableLocalSave)
	{
		hLocalFile = ::CreateFile ( strFilename.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                                  NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		if (hLocalFile == INVALID_HANDLE_VALUE)
		{
			bEnableLocalSave = false;
			cout << "Fail to open local save file "<<strFilename<<endl;
		}
	}

	//////////////////////////////////////////////////////////////////////////
#ifdef USE_WPCAP	// if using WinPcap

	time_t	lastPackTime = 0, currPackTime = 0;
	time(&lastPackTime), time(&currPackTime);
	
	time_t startTime = 0;
	// start here
	time(&startTime);

	for(;;)
	{
		// ctrl+c handler
		if(bTerminated)
			break;

		// timeout elapsed
		time(&currPackTime);
		if(currPackTime-lastPackTime >= iTimeout)
		{
			LogMsg(ZQ::common::Log::L_ERROR, "no more data, receive timeout, exit");
			break;
		}
		if(iTiming && currPackTime-startTime >= iTiming)
		{
			LogMsg(ZQ::common::Log::L_NOTICE, "due time reached, stop capturing.");
			break;
		}

		if (!ftpPush.IsRunning())
		{
			LogMsg(ZQ::common::Log::L_ERROR, "push data error, stop receive");
			break;
		}
		
		ftpPush.AllocateBuffer(&buff, &iLen);
		
		// recv packet
		iRet = pCatcher->Recv((char*)buff, (u_int*)&iLen);

		if(iRet > 0)
		{
			// timeout
			ftpPush.ReleaseBuffer(buff);
			continue;
		}
		else if(iRet < 0)
		{
			// error occurs
			errLen = 260;
			int tmpRet = pCatcher->GetLastErr(errBuffer, &errLen);
			if(tmpRet)
				LogMsg(ZQ::common::Log::L_ERROR, "recv packet error, details: %s", errBuffer);
			else
				LogMsg(ZQ::common::Log::L_ERROR, "recv packet error");
			break;
		}

		//test for save localfile
		if (bEnableLocalSave)
		{
			DWORD dwWrited;
			WriteFile(hLocalFile, buff, iLen, &dwWrited, NULL);
			if (iLen != dwWrited)
			{
				LogMsg(ZQ::common::Log::L_ERROR, "WriteFile error, disable local file write");
				CloseHandle(hLocalFile);
				bEnableLocalSave = false;
			}
		}
		
		// deal with messages
		ftpPush.AddDataBuffer(buff, iLen);
		
		// reset timeout clock
		time(&lastPackTime);

	}

	ftpPush.DataBufferEnd();
	ftpPush.WaitForFinish();
	
	if (bEnableLocalSave)
	{
		CloseHandle(hLocalFile);
	}
	
	if (pCatcher)
	{
		pCatcher->Close();
		delete pCatcher;
		pCatcher = NULL;
	}
	
	if (ZQ::common::pGlog != NULL && ZQ::common::pGlog != &ZQ::common::NullLogger)
	{
		delete ZQ::common::pGlog;
		ZQ::common::pGlog = &ZQ::common::NullLogger;
	}

	//////////////////////////////////////////////////////////////////////////
#else	// if not using WinPcap
	
	
#if 1
	// select
	fd_set rfs;
	FD_ZERO(&rfs);
	FD_SET(sockRecv->getSocket(), &rfs);

	for(;;)
	{
		// ctrl+c handler
		if(bTerminated)
			break;

		// select
		iRet = select(0, &rfs, NULL, NULL, ptv);
		if(0 == iRet)								//timeout
		{
			LogMsg(ZQ::common::Log::L_ERROR, "no more data, receive timeout, exit");
			break;
		}

		if(SOCKET_ERROR == iRet)					// socket error
		{
			LogMsg(ZQ::common::Log::L_ERROR, "select return socket error, code: 0x%08x", WSAGetLastError());
			break;
		}

		if(!FD_ISSET(sockRecv->getSocket(), &rfs))	// data is not for this socket
		{
			continue;
		}

		if (!ftpPush.IsRunning())
		{
			LogMsg(ZQ::common::Log::L_ERROR, "push data error, stop receive");
			break;
		}

		ftpPush.AllocateBuffer(&buff, &iLen);

		// recv message
		iRet = sockRecv->RecvFrom((char *)buff, iLen);

		//test for save localfile
		if (bEnableLocalSave)
		{
			DWORD dwWrited;
			WriteFile(hLocalFile, buff, iRet, &dwWrited, NULL);
			if (iRet != dwWrited)
			{
				LogMsg(ZQ::common::Log::L_ERROR, "WriteFile error, disable local file write");
				CloseHandle(hLocalFile);
				bEnableLocalSave = false;
			}
		}

		// deal with messages
		ftpPush.AddDataBuffer(buff, iRet);

	}
#else 
	for(;;)
	{
		// ctrl+c handler
		if(bTerminated)
			break;

		if (!ftpPush.IsRunning())
		{
			LogMsg(ZQ::common::Log::L_ERROR, "push data error, stop receive");
			break;
		}

		ftpPush.AllocateBuffer(&buff, &iLen);

		// recv message
		iRet = sockRecv->RecvFrom((char *)buff, iLen);

		if (iRet <=0)
		{
			printf("recv data fail\n");
			break;
		}
		else
		{
			printf("received %d bytes\n", iRet);
		}

		//test for save localfile
		if (bEnableLocalSave)
		{
			DWORD dwWrited;
			WriteFile(hLocalFile, buff, iRet, &dwWrited, NULL);
			if (iRet != dwWrited)
			{
				LogMsg(ZQ::common::Log::L_ERROR, "WriteFile error, disable local file write");
				CloseHandle(hLocalFile);
				bEnableLocalSave = false;
			}
		}

		// deal with messages
		ftpPush.AddDataBuffer(buff, iRet);
	}
#endif

	ftpPush.DataBufferEnd();
	ftpPush.WaitForFinish();

	if (bEnableLocalSave)
	{
		CloseHandle(hLocalFile);
	}

	if (sockRecv)
	{
		delete sockRecv;
		sockRecv = NULL;
	}

	if (ZQ::common::pGlog != NULL && ZQ::common::pGlog != &ZQ::common::NullLogger)
	{
		delete ZQ::common::pGlog;
		ZQ::common::pGlog = &ZQ::common::NullLogger;
	}

#endif	// USE_WPCAP
	//////////////////////////////////////////////////////////////////////////
	

	// exit program
	return 0;
}


void usage()
{
	printf("Usage: IngestSim [-s multiIP:port] [-d ftp://username:passwd@server:port/filename] [-f filename] [-h]\n");
	printf("Forward received stream package from one interface to another ftp server interface.\n");
	printf("options:\n");
	printf("\t-b ip		  specify bind local ip \n");
	printf("\t-s ip:port  specify the listening ip and port, defaut is 225.25.25.25:1234\n");
	printf("\t-d ftp://username:passwd@server:port/filename\n");
	printf("\t-f filename, save to local machine, disable default, for test\n");
	printf("\t-l logfile log file name, default is c:\\IngestSim.log\n");
	printf("\t-t timeout timeout value in second, default means infinite\n");
	printf("\t-T stop capturing data packets at a specified timing\n");
	printf("\t-h            display this help\n");
}

bool createMenuThread(HANDLE *phThread)
{
	DWORD dwThreadId, dwThrdParam = 1;  
	HANDLE hThread;

    hThread = CreateThread( 
        NULL,                        // default security attributes 
        0,                           // use default stack size  
        ThreadFunc,                  // thread function 
        &dwThrdParam,                // argument to thread function 
        0,                           // use default creation flags 
        &dwThreadId);                // returns the thread identifier 
	
	*phThread = hThread;
	// Check the return value for success. 
	if (hThread == NULL)
		return false;
	else
		return true;
}

DWORD WINAPI ThreadFunc( LPVOID lpParam ) 
{ 
    char * szMsg = "a";

    cout << "main menu: " << endl;
	cout << "0) exit" << endl;

	Socket sockHelper;

	// set sockHelper attrib
	sockHelper.setDomain(g_iSockFamily);
	sockHelper.setSockType(g_iSockType);
	sockHelper.setProtocol(g_iSockProtocol);
	sockHelper.setRemoteAddr(g_strRemoteIP);
	sockHelper.setRemotePort(g_iRemotePort);

	sockHelper.SocketCreate();
	//sockHelper.PrintSocket();

	DWORD dwExitCode = 1;
	char chSlection;

	while(1)
	{
		chSlection = getchar();
		switch(chSlection)
		{
		case '0':
			g_mo = MO_EXIT;
			sockHelper.SendTo(szMsg, strlen(szMsg) + 1);
			//ExitThread(dwExitCode);
			break;

		default:
			g_mo = MO_NOTSELECT;
			break;
		}
	}

    return 0; 
}

void signal_handler(int code)
{
	printf("Do you really want to Quit? [y/n] :");
	
	char * szMsg = "a";
	Socket sockHelper;
	
	// set sockHelper attrib
	sockHelper.setDomain(g_iSockFamily);
	sockHelper.setSockType(g_iSockType);
	sockHelper.setProtocol(g_iSockProtocol);
	sockHelper.setRemoteAddr(g_strRemoteIP);
	sockHelper.setRemotePort(g_iRemotePort);
	
	sockHelper.SocketCreate();

	char c;
	c = getchar();
	if (c == 'y' || c == 'Y')
	{
		bTerminated = TRUE;
		sockHelper.SendTo(szMsg, strlen(szMsg) + 1);
	}
}
