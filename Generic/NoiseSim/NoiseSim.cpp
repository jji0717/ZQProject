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
// $Log: /ZQProjs/Generic/NoiseSim/NoiseSim.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 7     05-07-11 11:12 Lin.ouyang
// add menu control
// 
// 6     05-07-08 13:36 Lin.ouyang
// 
// 5     05-07-07 19:43 Lin.ouyang
// modify config file and usage description
// 
// 4     05-07-07 19:05 Lin.ouyang
// add drop_data option support
// it support drop/modify data method
// 
// 3     05-07-06 19:56 Lin.ouyang
// add multicast support
// 
// 2     05-05-19 12:09 Lin.ouyang
// by: lorenzo(lin ouyang)
// add comment for doxgen
// 
// 1     05-05-17 15:48 Lin.ouyang
// init version
// 
// Revision 1.1  2005-05-17 15:30:26  Ouyang
// initial created
// ===========================================================================


/// @file "NoiseSim.cpp"
/// @brief the main file for NoiseSim project
/// @author (Lorenzo) Lin Ouyang
/// @date 2005-5-19 9:52
/// @version 0.1.0
//
// NoiseSim.cpp: Main file for project NoiseSim.
//
//////////////////////////////////////////////////////////////////////

#include "NoiseSim.h"
#include "McastUdpSocket.h"

/// Default local IP
#define LOCAL_IP "127.0.0.1"
/// Default local port
#define LOCAL_PORT 8888
/// Default remote IP
#define REMOTE_IP "192.168.80.141"
/// Default remote port
#define REMOTE_PORT 80
/// Default message size
#define MSG_SIZE 8192

/// @brief Convert hex to int
///
/// convert hex value in null terminated string to int value
/// @param[in] szHex Hex null terminated string, such as 0xff
/// @return if succeed, return decimal int value, otherwise return -1
int HexStrToInt(const char *szHex);

/// Print usage message
void usage();

/// thread entry function
DWORD WINAPI ThreadFunc( LPVOID lpParam );

/// create a thread for menu
bool createMenuThread();

/// global variable
typedef enum enMenuOption
{
	MO_EXIT,
	MO_NOTSELECT,
	MO_NOMORE
} MenuOption;

MenuOption g_mo = MO_NOTSELECT;
int g_iSockFamily = AF_INET;
int g_iSockType = SOCK_DGRAM;
int g_iSockProtocol = 0;
string g_strRemoteIP = LOCAL_IP;
int g_iRemotePort = LOCAL_PORT;

/// Main function
int main(int argc, char *argv[])
{
	init_WSA init_wsa;
	Socket *sockRecv, *sockSend;
	Filter filter;
	Noiser noiser;

	string strCfgFile = "NoiseSim.cfg";
	FileCfg fileCfg;

	// config value
	int cfg_iMsgSize = MSG_SIZE;
	string cfg_strSockFamily = "inet";
	string cfg_strSockType = "udp";
	string cfg_strSockProtocol = "0";
	string cfg_strLocalIP = LOCAL_IP;
	string cfg_strRemoteIP = REMOTE_IP;
	int cfg_iLocalPort = LOCAL_PORT;
	int cfg_iRemotePort = REMOTE_PORT;
	double cfg_dProbability = 0.0;
	int cfg_iInterval = 100;
	int cfg_iBitMask = 0x0;
	string cfg_strNoiseDistType = "random";
	string cfg_strNoiseUnit = "bit";
	string cfg_strDropData = "true";

	int iSockFamily;
	int iSockType;
	int iSockProtocol;
	NoiseDistType ndtNoiseDistType;
	NoiseUnit nuNoiseUnit;
	bool bDropData;
	
	char *szBuff = new char[cfg_iMsgSize];
	int iRet = 0;

	// judge the package whether been noised
	bool bNoised = false;

	// get command line arguments
	int ch;
	while((ch = getopt(argc, argv, "hf:")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
			usage();
			goto exit;

		case 'f':
			strCfgFile = optarg;
			break;
		default:
			usage();
			printf("Error: unknown option %c specified\n", ch);
			goto exit;
		}
	}

	// read config file
	if(false == fileCfg.Open(strCfgFile))
	{
		usage();
		cout << "can not find config file" << endl;
		goto exit;
	}

	for(;!fileCfg.isEof();)
	{
		string strKey, strValue;

		if(fileCfg.GetLine())
		{
			strKey = fileCfg.getKey();
			strValue = fileCfg.getValue();
			
			if(strKey == "msg_size")
			{
				cfg_iMsgSize = atoi(strValue.c_str());
				continue;
			}
	
			if(strKey == "sock_family")
			{
				cfg_strSockFamily = strValue;
				continue;
			}

			if(strKey == "sock_type")
			{
				cfg_strSockType = strValue;
				continue;
			}

			if(strKey == "sock_protocol")
			{
				cfg_strSockProtocol = strValue;
				continue;
			}

			if(strKey == "recv_ip")
			{
				cfg_strLocalIP = strValue;
				continue;
			}

			if(strKey == "recv_port")
			{
				cfg_iLocalPort = atoi(strValue.c_str());
				continue;
			}

			if(strKey == "send_ip")
			{
				cfg_strRemoteIP = strValue;
				continue;
			}

			if(strKey == "send_port")
			{
				cfg_iRemotePort = atoi(strValue.c_str());
				continue;
			}
			
			if(strKey == "dist_probability")
			{
				cfg_dProbability = atof(strValue.c_str());
				continue;
			}

			if(strKey == "noise_interval")
			{
				cfg_iInterval = atoi(strValue.c_str());
				continue;
			}

			if(strKey == "bitmask")
			{
				cfg_iBitMask = HexStrToInt(strValue.c_str());
				continue;
			}

			if(strKey == "noise_type")
			{
				cfg_strNoiseDistType = strValue;
				continue;
			}

			if(strKey == "noise_unit")
			{
				cfg_strNoiseUnit = strValue;
				continue;
			}

			if(strKey == "drop_data")
			{
				cfg_strDropData = strValue;
				continue;
			}
		}
	}

	// init WSA
	if(!init_wsa.init())
	{
		cout << "WSAStartup error" << endl;
		goto exit;
	}

	// set socket attribute
	if(cfg_strSockFamily == "inet")
		iSockFamily = AF_INET;

	if(cfg_strSockType == "udp")
		iSockType = SOCK_DGRAM;
	else if(cfg_strSockType == "tcp")
		iSockType = SOCK_STREAM;
	else
		iSockType = SOCK_RAW;

	if(cfg_strSockProtocol == "0")
		iSockProtocol = 0;

	// set attribute
	if(cfg_strNoiseDistType == "random")
		ndtNoiseDistType = NDT_RANDOM;

	if(cfg_strNoiseUnit == "bit")
		nuNoiseUnit = NU_BIT;
	else if(cfg_strNoiseUnit == "byte")
		nuNoiseUnit = NU_BYTE;
	else if(cfg_strNoiseUnit == "pkg")
		nuNoiseUnit = NU_PKG;

	if(cfg_strDropData == "true")
		bDropData = true;
	else
		bDropData = false;

	// judge the ip address type, multicast/unicast ip
	unsigned int iIpClass;
	char szIpClass[4];
	string::size_type idx;

	idx = cfg_strLocalIP.find('.');
	if(string::npos == idx)
	{
		cout << "recv_ip error" << endl;
		return false;
	}
	strcpy(szIpClass, cfg_strLocalIP.substr(0, idx).c_str());
	iIpClass = atoi(szIpClass);
	// cout << iIpClass << endl;

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
			sprintf(szMode, "recv socket is multicast socket");
		}
		else
		{
			cout << "recv_ip error" << endl;
			return false;
		}
	}
	cout << endl;

	// set socket attrib for menu thread
	g_iSockFamily = iSockFamily;
	g_iSockType = iSockType;
	g_iSockProtocol = iSockProtocol;
	g_strRemoteIP = cfg_strLocalIP;
	g_iRemotePort = cfg_iLocalPort;

	// init send socket, it only need common socket 
	// because it can send message even in multicast
	sockSend = new Socket;

	// set sockRecv attrib
	sockRecv->setDomain(iSockFamily);
	sockRecv->setSockType(iSockType);
	sockRecv->setProtocol(iSockProtocol);
	sockRecv->setLocalAddr(cfg_strLocalIP);
	sockRecv->setLocalPort(cfg_iLocalPort);

	// set sockSend attrib
	sockSend->setDomain(iSockFamily);
	sockSend->setSockType(iSockType);
	sockSend->setProtocol(iSockProtocol);
	sockSend->setRemoteAddr(cfg_strRemoteIP);
	sockSend->setRemotePort(cfg_iRemotePort);

	if(!sockRecv->SocketCreate())
	{
		cout << "Socket create error" << endl;
		goto exit;
	}

	if(!sockSend->SocketCreate())
	{
		cout << "Socket create error" << endl;
		goto exit;
	}

	if(!sockRecv->Bind())
	{
		cout << "Socket bind error" << endl;
		goto exit;
	}

	// print socket information
	cout << "recv socket: " << endl;
	sockRecv->PrintSocket();
	cout << endl;
	cout << "send socket: " << endl;
	sockSend->PrintSocket();
	cout << endl;

	// set noiser's attrib
	noiser.setMask(cfg_iBitMask);
	noiser.setNoiseDistType(ndtNoiseDistType);
	noiser.setNoiseUnit(nuNoiseUnit);
	noiser.setProbability(cfg_dProbability);
	noiser.setInterval(cfg_iInterval);
	
	cout << szMode << endl;
	cout << "Noise mode: " << (bDropData ? "drop data" : "modify data") << endl;
	cout << "Noise percent: " << (2.0 / (noiser.m_iInterval + 1)) * noiser.m_dProbability << endl;
	cout << "NOISING..." << endl;
	
	// create menu thread
	cout << endl << "select a number from main menu" << endl;
	if(!createMenuThread())
	{
		cout << "create thread error" << endl;
		goto exit;
	}

#ifdef _DEBUG
	int i;
	for(i = 0; i < 100000; ++i)
#else
	for(;;)
#endif
	{
		// recv message
		iRet = sockRecv->RecvFrom(szBuff, cfg_iMsgSize);

		// select exit from menu, exit loop
		if(g_mo == MO_EXIT)
			break;

		// filtrate message
		filter.Filtrate(szBuff, iRet);

		// noise message
		bNoised = noiser.doNoise(szBuff, iRet);

		// send message
		// if need drop data, drop data according bNoised variable
		if(bDropData)
		{
			// not been noised, so forward data simply, otherwise do nothing (drop data)
			if(!bNoised)
			{
				#ifdef _DEBUG
				//cout << "send data..." << endl;
				#endif
				sockSend->SendTo(szBuff, iRet);
			}
			#ifdef _DEBUG
			//cout << "drop data mode..." << endl;
			#endif
		}
		// if needn't drop data, then foward data simply
		else
		{
			sockSend->SendTo(szBuff, iRet);
			#ifdef _DEBUG
			//cout << "modify data..." << endl;
			#endif
		}
	}
//#ifdef _DEBUG
//	for(int j=0; j < iRet; ++j)
//		printf("%c", szBuff[i]);
//	printf("\n");
	cout << "         " << "pakage    " << "byte      " << "bit       " << endl;
	cout << "total  : " << noiser.m_dPkgCount << " : " << noiser.m_dByteCount 
		<< " : " << noiser.m_dByteCount*8 << endl;
	cout << "noised : " << noiser.m_dPkgNoiseCount << " : "
		<< noiser.m_dByteNoiseCount << " : " << noiser.m_dBitNoiseCount << endl;
	cout << "percent: " << noiser.m_dPkgNoiseCount/noiser.m_dPkgCount << " : "
		<< noiser.m_dByteNoiseCount/noiser.m_dByteCount << " : " 
		<< noiser.m_dBitNoiseCount/noiser.m_dByteCount << endl;
	cout << "theory percent: " << (2.0 / (noiser.m_iInterval + 1)) * noiser.m_dProbability 
		<< endl;
//#endif

exit:
	delete [] szBuff;
	return 0;
}

void usage()
{
	printf("Usage: NoiseSim [-f <cfgfile>] [-h]\n");
	printf("Forward message from one interface to another interface.\n");
	printf("options:\n");
	printf("\t-f <cfgfile>  specify the config file, defalut is NoiseSim.cfg\n");
	printf("\t-h            display this help\n");
}

// convert 0xff to int
int HexStrToInt(const char *szHex)
{
	int i, iResult = 0;

	if(strlen(szHex) != 4 || szHex[0] != '0')
		return -1;

	if(szHex[1] != 'x' && szHex[1] != 'X')
		return -1;

	for(i = 2; i < 4; ++i)
	{
		char ch;
		int iTemp;

		ch = szHex[i];
		switch(ch)
		{
		case 'f':
		case 'F':
			iTemp = 15;
			break;
		
		case 'e':
		case 'E':
			iTemp = 14;
			break;
		
		case 'd':
		case 'D':
			iTemp = 13;
			break;
		
		case 'c':
		case 'C':
			iTemp = 12;
			break;

		case 'b':
		case 'B':
			iTemp = 11;
			break;

		case 'a':
		case 'A':
			iTemp = 10;
			break;

		case '9':
			iTemp = 9;
			break;

		case '8':
			iTemp = 8;
			break;

		case '7':
			iTemp = 7;
			break;

		case '6':
			iTemp = 6;
			break;

		case '5':
			iTemp = 5;
			break;

		case '4':
			iTemp = 4;
			break;

		case '3':
			iTemp = 3;
			break;

		case '2':
			iTemp = 2;
			break;

		case '1':
			iTemp = 1;
			break;
			
		default:
			return -1;
		}
		iResult = iResult * 16 + iTemp;
	}

	return iResult;
}

bool createMenuThread()
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

	int iSlection;

	while(1)
	{
		cin >> iSlection;
		g_mo = static_cast<MenuOption>(iSlection);
		
		sockHelper.SendTo(szMsg, strlen(szMsg) + 1);
	}

    return 0; 
}