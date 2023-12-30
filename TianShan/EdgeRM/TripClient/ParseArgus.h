#ifndef		__PARSE_ARGUS_H__
#define		__PARSE_ARGUS_H__

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <getopt.h>
#include "ZQ_common_conf.h"

#define DEFAULT_LOGPATH		"c:\\tianshan\\logs\\TripClient.log"
#define DEFAULT_MSGPATH		""
#define DEFAULT_RECVTHPOOLSIZE	10
#define DEFAULT_PROCTHPOOLSIZE	10
#define DEFAULT_MAXCONNECTION	100
#define DEFAULT_HOLDTIME		1800
#define DEFAULT_INTERVAL		100

typedef struct _ARGUS{
	char*		bindIP;
	char*		logPath;
	char*		msgPath;
	int			bindPort;
	uint32		holdTime;//seconds
	int			recvThPoolSize;
	int			procThPoolSize;
	int			maxConnection;
	uint64		interval;
}Argus;

void print_usage()
{
			 //******************************************************************************   ruler
	std::cout<<"Usage:\n"
			 <<"-I      [necessary]Remote server ip.\n"
			 <<"-P      [necessary]Remote server port.\n"
			 <<"[-r]    [optional]Receive thread pool size.Default is 10\n"
			 <<"[-p]    [optional]Process thread pool size.Default is 10\n"
			 <<"[-t]    [optional]Session hold time.Default is 1800 second\n"
			 <<"[-c]    [optional]The count of session.Default is 1000\n"
			 <<"[-i]    [optional]Interval of create session.Default is 100ms\n"
			 <<"example:TripClient -I192.168.81.99 -P6069 -r10 -c10"
			 <<std::endl;
	return;
}

bool parseArgus(int argc,char* argv[],Argus& argus)
{
	if(argc < 3)
	{
		print_usage();
		return 0;
	}

	//set default value;
	argus.logPath = DEFAULT_LOGPATH;
	argus.msgPath = DEFAULT_MSGPATH;
	argus.recvThPoolSize = DEFAULT_RECVTHPOOLSIZE;
	argus.procThPoolSize = DEFAULT_PROCTHPOOLSIZE;
	argus.maxConnection = DEFAULT_MAXCONNECTION;
	argus.holdTime = DEFAULT_HOLDTIME;
	argus.interval = DEFAULT_INTERVAL;

	int option_index = 0,c;
	while((c = getopt(argc, argv, "I:P:r:p:c:t:i:")) != EOF)
	{
		if(c == -1)
		{
			print_usage();
			return false;
		}

		switch(c)
		{
		case 'I':
			argus.bindIP = optarg;
			//printf("bindIP:%s\n",argus.bindIP);
			break;
		case 'P':
			argus.bindPort = atoi(optarg);
			//printf("bindPort:%d\n",argus.bindPort);
			break;
		case 'r':
			argus.recvThPoolSize = atoi(optarg);
			//printf("recvThPoolSize:%d\n",argus.recvThPoolSize);
			break;
		case 'p':
			argus.procThPoolSize = atoi(optarg);
			//printf("procThPoolSize:%d\n",argus.procThPoolSize);
			break;
		case 'c':
			argus.maxConnection = atoi(optarg);
			break;
		case 't':
			argus.holdTime = (uint32)atoi(optarg);
			break;
		case 'i':
			argus.interval = (uint64)atoi(optarg);
			break;
		default:
			print_usage();
		}
	}

	return true;

}

#endif // end of __PARSE_ARGUS_H__