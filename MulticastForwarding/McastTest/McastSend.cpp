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
// Ident : $Id: McastSend.cpp,v 1.8 2004/08/05 07:14:49 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : the multicast package sender utility
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MulticastForwarding/McastTest/McastSend.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 7     04-10-10 15:55 Hui.shao
// check valid local address to bind
// 
// 6     04-10-09 18:13 Hui.shao
// 
// 5     04-09-28 14:20 Hui.shao
// added sequence sending process for missing package test
// 
// 4     04-09-15 14:40 Hui.shao
// added new option for performance test
// Revision 1.8  2004/08/05 07:14:49  shao
// -l and -i
//
// Revision 1.7  2004/07/29 05:14:36  shao
// default count 1
//
// Revision 1.6  2004/07/28 09:30:34  shao
// unspec bind address
//
// Revision 1.5  2004/06/25 06:46:20  shao
// default got to help scr
//
// Revision 1.4  2004/06/24 12:34:40  shao
// corrected comment
//
// Revision 1.3  2004/06/24 11:03:48  shao
// getopt routine
//
// Revision 1.2  2004/06/24 06:28:36  shao
// count number
//
// Revision 1.1  2004/06/24 06:25:36  shao
// Created
//
// ===========================================================================

#include "UDPSocket.h"
#include "getopt.h"

#define DEFAULT_PORT     1000
#define DEFAULT_INTERVAL  500 // 0.5 sec
#define DEFAULT_SEND_COUNT 1
#define DEFAULT_TTL 128

void usage()
{
	printf("Usage: McastSend [-b <bindIP>] [-r <bindport>] [-t <TTL>] [-i <n>] [-c <n>]\n");
	printf("                 ¨Cg <mcastIP> -p <port> {[-s] | [-m <message>] [-l <n>]}\n");
	printf("       McastSend -h\n");
	printf("Multicast a message to the specified group and port.\n");
	printf("options:\n");
	printf("\t-b   the local IP address to bind\n");
	printf("\t-r   the local port to send message thru. default %d\n", DEFAULT_PORT);
	printf("\t-t   package TTL (time to live). default %d\n", DEFAULT_TTL);
	printf("\t-i   repeat interval. default %d msec\n", DEFAULT_INTERVAL);
	printf("\t-c   repeat times. default %d\n", DEFAULT_SEND_COUNT);
	printf("\t-g   the mulitcast group IP address\n");
	printf("\t-p   the mulitcast port. default %d\n", DEFAULT_PORT);
	printf("\t-s   send sequence number as message body, -m and -l will be ignored\n");
	printf("\t-m   the message to send\n");
	printf("\t-l   the length of message to send\n");
	printf("\t-h   display this help\n");
}

int main(int argc, char* argv[])
{
	ZQ::common::InetMcastAddress group;
	ZQ::common::InetHostAddress bind;
	int port=DEFAULT_PORT;
	int bindport=DEFAULT_PORT;
	int count =DEFAULT_SEND_COUNT;
	int interval = DEFAULT_INTERVAL;
	int ttl   =DEFAULT_TTL;
	bool bgroup = false;
	bool bbind = false;
	std::string msg2send = "McastSend::Hello World";
	int         msglen = -1;
	bool bSeq = false;
	const char* msg = NULL;

	// parse the command options
	if (argc <2)
	{
		usage();
		exit(0);
	}

	int ch;
	while((ch = getopt(argc, argv, "hg:p:c:b:r:t:m:i:l:s")) != EOF)
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
				printf("Error: illegal <mcastIP> specified: %s\n", optarg);
				exit(1);
			}
			else bgroup = true;
			break;

		case 'p':
			port = atoi(optarg);
			break;

		case 'c':
			count = atoi(optarg);
			break;

		case 'i':
			interval = atoi(optarg);
			break;
			
		case 'b':
			if (!bind.setAddress(optarg))
			{
				printf("Error: illegal <bindIP> value\n");
				exit(1);
			}
			else bbind =true;
			break;

		case 'r':
			bindport = atoi(optarg);
			break;

		case 't':
			ttl = atoi(optarg);
			break;

		case 's':
			bSeq = true;
			break;

		case 'm':
			msg2send = optarg;
			break;

		case 'l':
			msglen = atoi(optarg);
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

	//verify the multicat times
	if (count <=0)
	{
		count=DEFAULT_SEND_COUNT;
		printf("Warning: send %d times as default\n", port);
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
	}
	//verify the bind port
	if (bindport <=0)
	{
		bindport=DEFAULT_PORT;
		printf("Warning: use %d as default bind port\n", bindport);
	}

	//verify the TTL
	if (ttl <=0)
	{
		bindport=DEFAULT_PORT;
		printf("Warning: use %d as default TTL\n", ttl);
	}

	if (!bSeq)
	{

		// adjust message to send based on the length
		if (msglen <0)
			msglen = msg2send.length();

		if (msglen < (int)msg2send.length())
			msg2send = msg2send.substr(0, msglen);
		else
		{
			for (int i =msg2send.length(); i< msglen; i++)
				msg2send.append(".");
		}
	}

	// initial a multicast socket and target/join the group
	ZQ::common::UDPMulticast sock(bind, bindport);

	if (!sock.isActive())
	{
		printf("Error: %s is not a valid local address\n", bind.getHostAddress());
		exit(1);
	}

	sock.setGroup(group, port);
	sock.setTimeToLive(ttl);

	// send the messages
	if (!bSeq)
		printf("Multicast [%s]:%d with %d bytes of data:\n\"%s\"\n", group.getHostAddress(), port, msglen, msg2send.c_str());
	else
	{
		msglen = 10;
		printf("Multicast [%s]:%d with %d bytes of data serially...\n", group.getHostAddress(), port, msglen);
	}


	SYSTEMTIME starttime, endtime;
	::GetSystemTime(&starttime);

	int s =0;
	for(int i=1; i<=count; i++)
	{
		if (bSeq)
		{
			char buf[12];
			sprintf(buf, "s%09d",i);
			if (sock.send(buf, msglen)>0)
				s++;
		}
		else
		{
			if (sock.send(msg2send.c_str(), msglen)>0)
				s++;
		}
		printf("\r%03d messages has been sent                 \r", i);

		if (interval>0)
			::Sleep(interval);
	}
	::GetSystemTime(&endtime);

	__int64 intv = ((((endtime.wDay-starttime.wDay)*24
				+endtime.wHour-starttime.wHour)*60
				+endtime.wMinute-starttime.wMinute)*60
				+endtime.wSecond-starttime.wSecond)*1000
				+ endtime.wMilliseconds-starttime.wMilliseconds;

	printf(  "\rmessages sent   : %d                  \n", s);
	if (intv > 200 && s>1)
	{
		printf("total duration  : %5.3f sec\n"
			   "sending interval: %d msec\n"
			   "average sending : %4.3f msec\n"
			   "rate            : %4.2f Packsage/sec; %4.2f Byte/sec \n",
			   ((float)intv)/1000, interval, (float)(intv -interval*count)/s, (float)count*1000/intv, (float)s*msglen*1000/intv);
	}
	
}

