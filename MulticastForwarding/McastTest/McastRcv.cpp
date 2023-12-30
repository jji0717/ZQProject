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
// Ident : $Id: McastRcv.cpp,v 1.8 2004/07/28 09:30:34 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : the multicast package receiver utility
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MulticastForwarding/McastTest/McastRcv.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 4     04-10-09 18:12 Hui.shao
// 
// 3     04-09-15 14:40 Hui.shao
// added new option for performance test
// 
// 2     04-09-14 14:08 Kaliven.lee
// Revision 1.8  2004/07/28 09:30:34  shao
// unspec bind address
//
// Revision 1.7  2004/07/27 09:41:27  shao
// match the renamed virtual method
//
// Revision 1.6  2004/07/22 09:48:50  shao
// typo
//
// Revision 1.5  2004/07/07 11:39:01  shao
// cleaned the test code
//
// Revision 1.4  2004/07/07 02:49:03  shao
// included denylist
//
// Revision 1.3  2004/06/25 06:45:55  shao
// denylist test
//
// Revision 1.2  2004/06/24 12:41:59  shao
// display messages
//
// Revision 1.1  2004/06/24 12:34:17  shao
// created
//
// ===========================================================================

#include "../McastListener.h"
#include "../DenyList.h"

#include "XMLPreference.h"

#include "getopt.h"
extern "C"
{
#include <time.h>
#include <stdio.h>
}

#define DEFAULT_RUNTIME 20 // 20sec
#define DEFAULT_PORT 1000

BOOL WINAPI ConsoleHandler(DWORD event);
bool bQuit = false;
DWORD gdwMaxUDPBufferSize = 16*1024;

class testListener : public McastListener
{
public:
	testListener(ZQ::common::InetMcastAddress group, int group_port, ZQ::common::InetHostAddress bind, bool timestamp=false)
		:McastListener(group,group_port, bind), _bStamp(timestamp)
	{
		_stamp[0]='\0';
	}

private:
	int OnCapturedData(const void* data, const int datalen, ZQ::common::InetHostAddress source, int sport)
	{
		if (datalen <=0)
			return 0;

		char *p=(char*)data;
		if (_bStamp)
		{
			time_t     longTime;
			time(&longTime);
			struct tm *timeData = localtime(&longTime);

			sprintf(_stamp, "%02d:%02d:%02d ", timeData->tm_hour, timeData->tm_min, timeData->tm_sec);
		}

		printf("%sreceived from [%s]:%d ", _stamp, source.getHostAddress(), sport);
		for (int j=0; j<datalen; j++)
			printf("%c", p[j], (isprint(p[j])?p[j]:'.'));

		printf("\n");

		return datalen;
	}

	bool _bStamp;
	char _stamp[16];
};

void usage()
{
	printf("Usage: McastRcv [-b <bindIP>] [-s] ¨Cg <mcastIP> -p <port>\n");
	printf("       McastRcv -h\n");
	printf("receive multicast messages on the specified group and port for a specified period.\n");
	printf("options:\n");
	printf("\t-b   the local IP address to bind\n");
	printf("\t-g   the mulitcast group IP address\n");
	printf("\t-p   the mulitcast port. default %d\n", DEFAULT_PORT);
	printf("\t-t   listen time, in sec. default %d\n", DEFAULT_RUNTIME);
	printf("\t-s   with timestamps\n");
	printf("\t-h   display this help\n");
}

/*
void denyTest()
{
	ZQ::common::ComInitializer init;

	ZQ::common::XMLPrefDoc doc(init);
	if (!doc.open("DenyList.xml"))
		return;

	ZQ::common::IPreference* root = doc.root();
	char buf[1024];
	root->name(buf);
	DenyList list(root);
	root->free();
	doc.close();

	bool ret;

 	ret = list.match("www.yahoo.com"); // yes
	ret =list.match("www.google.com"); // no
	ret =list.match("11.0.0.2"); // no
	ret =list.match("192.168.0.1", 80); // yes
	ret =list.match("192.168.0.1", 801); // no
	ret =list.match("3f::22", 80); // yes
	ret =list.match("10.5.5.1"); // no
}
*/

int main(int argc, char* argv[])
{
//	denyTest();

	ZQ::common::InetMcastAddress group;
	ZQ::common::InetHostAddress bind;
	bool bgroup = false;
	bool bbind = false;
	int ltime = DEFAULT_RUNTIME;
	int port = DEFAULT_PORT;
	bool bTimestamp = false;

	// parse the command options
	if (argc <2)
	{
		usage();
		exit(0);
	}

	int ch;
	while((ch = getopt(argc, argv, "hg:p:b:t:s")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
			usage();
			exit(0);

		case 'g':
			if (!group.setAddress(optarg))
			{
				printf("Error: illegal <mcastIP> specified\n");
				exit(1);
			}
			else bgroup = true;
			break;

		case 'p':
			port = atoi(optarg);
			break;

		case 'b':
			if (!bind.setAddress(optarg))
			{
				printf("Error: illegal <bindIP> value\n");
				exit(1);
			}
			else bbind =true;
			break;

		case 't':
			ltime = atoi(optarg);
			break;

		case 's':
			bTimestamp = true;
			break;

		default:
			printf("Error: unknown option %c specified\n", ch);
			exit(1);
		}
	}

	// -g is a must option
	if (!bgroup)
	{
		printf("Error: <mcastIP> is required\n");
		exit(1);
	}

	//verify the multicat port
	if (port <=0)
	{
		port=DEFAULT_PORT;
		printf("Warning: use %d as default port\n", port);
	}

	if (ltime <=0)
	{
		ltime=DEFAULT_RUNTIME;
		printf("Warning: use %d as default listening time\n", ltime);
	}

	// verify the IP address to bind
	if(!bbind)
	{
		const char * UNSPEC_BIND = (group.family() == PF_INET6) ? "::0" : "0.0.0.0";
		bind.setAddress(UNSPEC_BIND);
	}
	else
	{
		//TODO: validate if it is local address
		// see if the family is matched
		if (bind.family() != group.family())
		{
			printf("Error: <bindIP> and <mcastIP> not matched\n");
			exit(1);
		}

		ZQ::common::InetHostAddress gLocalAddrs;// = ZQ::common::InetHostAddress::getLocalAddress();
		if (bind.family() == PF_INET && bind.isInetAddress() && bind != gLocalAddrs)
		{
			printf("Error: %s is not a local address\n", bind.getHostAddress());
			exit(1);
		}
	}

	try
	{
		testListener listener(group, port, bind, bTimestamp);
		if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
		{
			printf("Unable to install handler!\n");
			return -1;
		}
		printf("Listening for %d sec..., \"Ctrl-C\" at any time to exit the program.\n", ltime);

		if (listener.start())
		{
			for (int i=0; i< ltime*2 && !bQuit; i++)
			::Sleep(500);
		}

		listener.quit();

		printf("quiting...", ltime);
		::Sleep(1000); // wait for quit
	}
	catch(...)
	{
		printf("\nError occurs\n");
	}
	printf("\n");
}

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
    switch(CEvent)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
		bQuit = true;
        break;

    }
    return TRUE;
}