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
// $Log: /ZQProjs/MulticastForwarding/McastTest/McastSnfr.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 5     04-11-23 16:44 Hui.shao
// 
// 4     04-11-18 16:28 Hui.shao
// renamed openListener to open()
// 
// 3     04-11-08 15:38 Hui.shao
// accept to open multiple listener interfaces
// 
// 2     04-11-04 20:59 Hui.shao
// 
// 1     04-11-03 17:05 Hui.shao
// ===========================================================================

#include "../McastSniffer.h"
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

class testSubscriber : public McastSubscriber
{
public:

	testSubscriber() : _bStamp(false)
	{
		_stamp[0]='\0';
	}

	void enableStamp(bool enable =true) { _bStamp = enable; }

private:
	int OnMcastMessage(const McastMsgInfo& MsgInfo)
	{
		if (MsgInfo.datalen <=0)
			return 0;

		char *p=(char*) MsgInfo.data;
		if (_bStamp)
		{
			time_t     longTime;
			time(&longTime);
			struct tm *timeData = localtime(&longTime);

			sprintf(_stamp, "%02d:%02d:%02d ", timeData->tm_hour, timeData->tm_min, timeData->tm_sec);
		}

		std::string saddr = MsgInfo.source.getHostAddress();
		std::string gaddr = MsgInfo.group.getHostAddress();

		printf("%sreceived [%s]:%d from [%s]:%d ", _stamp, gaddr.c_str(), MsgInfo.gport, saddr.c_str(), MsgInfo.sport);
		for (int j=0; j<MsgInfo.datalen; j++)
			printf("%c", p[j], (isprint(p[j])?p[j]:'.'));

		printf("\n");

		return MsgInfo.datalen;
	}

	bool _bStamp;
	char _stamp[16];
};

testSubscriber mySubscriber;

void usage()
{
	printf("Usage: McastSnfr [-b <bindIP>] [-s] ¨Cg <mcastIP>[,<port>] -p <port>\n");
	printf("       McastSnfr -h\n");
	printf("receive multicast messages on the specified group and port for a specified period.\n");
	printf("options:\n");
	printf("\t-b   the local IP address to bind\n");
	printf("\t-g   the mulitcast group IP address, and port\n");
	printf("\t-p   the default mulitcast port if not specified in -l. default %d\n", DEFAULT_PORT);
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

typedef struct
{
	ZQ::common::InetMcastAddress gaddr;
	int gport;
} mgroup_t;

typedef std::vector<mgroup_t> mgroups_t;


int main(int argc, char* argv[])
{
//	denyTest();

	mgroups_t groups;

	ZQ::common::InetHostAddress bind;
	bool bgroup = false;
	bool bbind = false;
	int ltime = DEFAULT_RUNTIME;
	int defport = DEFAULT_PORT;
	bool bTimestamp = false;

	// parse the command options
	if (argc <2)
	{
		usage();
		exit(0);
	}

	char ch;
	while((ch = getopt(argc, argv, "hg:p:b:t:s")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
			usage();
			exit(0);

		case 'g':
			{
				char gbuf[1024], *groupstr = gbuf;
				strcpy(gbuf, optarg);
				char* semicolon = strchr(groupstr, ';');

				while (groupstr)
				{
					if (semicolon)
						*semicolon = '\0';

					mgroup_t group;
					char* comma = strchr(groupstr, ',');

					if (comma)
					{
						*comma = '\0';
						group.gport = atoi(++comma);
					}
					else group.gport =0;

					if (!group.gaddr.setAddress(groupstr))
					{
						printf("Error: illegal <mcastIP> specified: %s\n", group.gaddr.getHostAddress());
						exit(1);
					}
					else
					{
						groups.push_back(group);
						bgroup = true;
					}

					if (semicolon)
					{
						groupstr = ++semicolon;
						semicolon = strchr(++semicolon, ';');
					}
					else groupstr = NULL;

				}
			}

			break;

		case 'p':
			defport = atoi(optarg);
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
		printf("Error: <mcastIP>[,<port>] is required\n");
		exit(1);
	}

	//verify the multicat port
	if (defport <=0)
	{
		defport=DEFAULT_PORT;
		printf("Warning: use %d as default port\n", defport);
	}

	if (ltime <=0)
	{
		ltime=DEFAULT_RUNTIME;
		printf("Warning: use %d as default listening time\n", ltime);
	}

	// verify the IP address to bind
	if(!bbind)
	{
		const char * UNSPEC_BIND = (groups[0].gaddr.family() == PF_INET6) ? "::0" : "0.0.0.0";
		bind.setAddress(UNSPEC_BIND);
	}
	else
	{
		//TODO: validate if it is local address
		// see if the family is matched
		if (bind.family() != groups[0].gaddr.family())
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
		McastSniffer sniffer;
		bool hasListener = false;
		
		if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
		{
			printf("Unable to install handler!\n");
			return -1;
		}
		printf("Listening for %d sec..., \"Ctrl-C\" at any time to exit the program.\n", ltime);

		mySubscriber.enableStamp(bTimestamp);

		for (mgroups_t::iterator it = groups.begin(); it < groups.end(); it++)
		{
			if (it->gport <=0)
				it->gport = defport;

			sniffer.open(it->gaddr, it->gport, bind, &mySubscriber);
		}

		if (sniffer.size() >0)
		{
			sniffer.start();
			for (McastSniffer::iterator it = sniffer.begin(); it < sniffer.end(); it++)
			{
				std::string bind = (*it)->bind.getHostAddress();
				std::string group = (*it)->group.getHostAddress();

				printf("interface [%s] sniffering [%s]:%d\n", bind.c_str(), group.c_str(), (*it)->gport);
			}

			for (int i=0; i< ltime*2 && !bQuit; i++)
				::Sleep(500);
		}

		sniffer.stop();

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