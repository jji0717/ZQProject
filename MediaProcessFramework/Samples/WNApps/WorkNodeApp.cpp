// WorkNodeApp.cpp : Defines the entry point for the console application.
//

// #include "stdafx.h"

#include "MyWork.h"
#include "Daemon.h"
#include "MPFLogHandler.h"
#include "getopt.h"
#include <signal.h>
#include <map>
#include <string>
using namespace std;

BOOL bTerminated = FALSE;
void signal_handler(int code)
{
	printf("Do you really want to Quit? [y/n] :");
	char c;
	c = getchar();
	if (c == 'y' || c == 'Y')
		bTerminated = TRUE;
}

void usage()
{
	printf("parameters:\n");
	printf("\t-a	work node ip address\n");
	printf("\t-p	work node port\n");
	printf("\t-h	display this help\n");
}

class MyLog : public ZQ::MPF::MPFLogHandler
{
private:
	FILE* m_hFile;
public:
	MyLog(const char* logfile)
		:m_hFile(NULL)
	{
		m_hFile = fopen(logfile, "w");
	}

	virtual ~MyLog()
	{
		if (m_hFile)
			fclose(m_hFile);
	}

	void writeMessage(const char* msg)
	{
		time_t tm;
		time(&tm);

		//printf("%s\n", msg);
		if (m_hFile)
		{
			//fwrite(msg, strlen(msg), 1, m_hFile);
			char strTemp[1024] = {0};
			_snprintf(strTemp, 1024, "<%ld> %s\n", (long)tm, msg);
			fwrite(strTemp, strlen(strTemp), 1, m_hFile);

//			fputs(" - <%ld>\n", m_hFile);
		}
	}
};

class MyDaemon: public ZQ::MPF::WorkNode::Daemon
{
public:
	MyDaemon(const char* url):Daemon(url)
	{
	}

	~MyDaemon()
	{
	}
	
	virtual void MyDaemon::OnUserHeartbeat(RpcValue& out)
	{
		out.SetStruct("MyData",RpcValue("UserData"));
	}
};

int main(int argc, char* argv[])
{
	// parse the command options
	if (argc <2)
	{
		usage();
		return -1;
	}

	int ch;
	
	char wip[MAX_PATH]={0};
	char mip[MAX_PATH]={0};
	int wport = 0; 
	int mport = 0;

	while((ch = getopt(argc, argv, "p:a:P:A:h:H")) != EOF)
	{
		switch (ch)
		{
		case 'p'://work node port
			if(optarg==0)exit(1);
			if(*optarg==0)exit(1);
			if ((wport = atoi(optarg)) <=0)
			{
				printf("Error: illegal work node port specified!\n");
				exit(1);
			}
			break;
		case 'a'://work node ip
			if(optarg==0)exit(1);
			if(*optarg==0)
			{
				printf("Error: illegal work node ip specified!\n");
				exit(1);
			}
			else
				strncpy(wip,optarg,MAX_PATH);
			break;
		case 'P'://manage node port
			if(optarg==0)exit(1);
			if(*optarg==0)exit(1);
			if ((mport = atoi(optarg)) <=0)
			{
				printf("Error: illegal manage node port specified!\n");
				exit(1);
			}
			break;
		case 'A'://manage node ip
			if(optarg==0)exit(1);
			if(*optarg==0)
			{
				printf("Error: illegal manage node ip specified!\n");
				exit(1);
			}
			else
				strncpy(mip,optarg,MAX_PATH);
			break;
		case '?':
		case 'h':
		case 'H':
			usage();
			return 0;
		}
	}

	// install Ctrl-C signal
	signal(SIGINT, signal_handler);

	printf("Press Ctrl+C to stop\n");
	//////////////////////////////////////////////////////////////////////////
	//following codes setup work node
	//MyLog lg("c:\\worknode.log");
	//MPFLogHandler::setLogHandler(&lg);


	//1  first generate daemon object
	URLStr wnurl;
	wnurl.setProtocol("MPF");
	wnurl.setHost(wip);
	wnurl.setPort(wport);

	URLStr mnurl;
	mnurl.setProtocol("MPF");
	mnurl.setHost(mip);
	mnurl.setPort(mport);

	MyDaemon daemon(wnurl.generate());
	daemon.addMgmNode(mnurl.generate());

	if(mport==0)
		daemon.setLeaseTerm(-1);
	else
		daemon.setLeaseTerm(10000);	// force heartbeat to be 10 seconds
	
	//2  second generate task acceptor object
	TaskAcceptor _Tasking(daemon);

	//3. create work factory and register it
	MyFileWorkFactory _Factory(&_Tasking);
	_Tasking.regFactory(&_Factory);

	//4. start daemon
	daemon.start();

	while(!bTerminated)
	{
		::Sleep(1000);
	}

	//5. after CTRL-c, stop daemon
	daemon.stop();
	
	return 0;

}

