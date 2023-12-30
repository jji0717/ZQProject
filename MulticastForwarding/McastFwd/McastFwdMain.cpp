#include "../McastFwd.h"
#include "../Tunnel.h"
#include "XMLPreference.h"
#include "Thread.h"
#include "ScLog.h"
#include "getopt.h"

McastFwd theServer;
ZQ::common::Log* pProglog = &ZQ::common::NullLogger;
ZQ::common::InetHostAddress gLocalAddrs;
bool gbPeerStampFlag = false;

BOOL WINAPI ConsoleHandler(DWORD CEvent);
bool bQuit = false;
DWORD gdwMaxUDPBufferSize = 16*1024;
void usage()
{
	printf("Usage: McastFwd -c <configuration> [-l <logfilename>] [-v <loglevel>] -s\n");
	printf("       McastFwd -h\n");
	printf("McastFwd service console mode for testing.\n");
	printf("options:\n");
	printf("\t-c   configuration file name\n");
	printf("\t-s   stamp the forwarded traffic\n");
	printf("\t-l   specify a log file\n");
	printf("\t-v   specify the log level to trace\n");
	printf("\t-h   display this help\n");
}

int main(int argc, char* argv[])
{
	gLocalAddrs = ZQ::common::InetHostAddress::getLocalAddress();
	TunnelConnection::_pDefaultConversation = (Conversation*)&theServer;
	TunnelConnection::_localid.create();
	ZQ::common::ComInitializer init;

	ZQ::common::XMLPrefDoc doc(init);

	char confname[260] = "McastFwd.xml";
	char logname[260] =  "McastFwd.log";

	int loglevel = ZQ::common::Log::L_ERROR;

	// parse the command options
	if (argc <2)
	{
		usage();
		exit(0);
	}

	int ch;
	while((ch = getopt(argc, argv, "hc:l:v:s")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
			usage();
			exit(0);

		case 'c':
			strcpy(confname, optarg);
			break;

		case 'l':
			strcpy(logname, optarg);
			break;

		case 'v':
			loglevel = ZQ::common::Log::getVerbosityInt(optarg);
			break;

		case 's':
			gbPeerStampFlag = true;
			break;

		default:
			printf("Error: unknown option %c specified\n", ch);
			exit(1);
		}
	}

	ZQ::common::ScLog progLogger(logname);
	progLogger.setVerbosity(loglevel);

	pProglog = &progLogger;
	ZQ::common::setGlogger(&progLogger);

	printf("McastFwd starts via installation ID %s\n", TunnelConnection::localidstr());
	progLogger("****McastFwd starts via installation ID %s", TunnelConnection::localidstr());

	try
	{
		printf("openning preference %s\n", confname);
		if (!doc.open(confname))
		{
			// failed to open the configuration file
			progLogger("failed to open configuration: %s", confname);
			return -1;
		}
	}
	catch(ZQ::common::Exception e)
	{
		printf("%s", e.getString());
		return -1;
	}

	ZQ::common::IPreference* root = doc.root();

	if(root ==NULL)
	{
		printf("no root preference is found\n");
		return -1;
	}

	char buf[512];
	root->name(buf, sizeof(buf));
	bool bInited = theServer.initialize(root);
	root->free();

	if (bInited)
	{
		printf("\n\"Ctrl-C\" at any time followed by a empty line to exit the program.\n\n");
		if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
		{
			// unable to install handler... 
			// display message to the user
			printf("Unable to install handler!\n");
			return -1;
		}

		theServer.start();

		static const char prgs[] = "|/-\\";
		int i =0;
		while(!bQuit)
		{
			i = (++i)%4;
			printf("\rrunning %c\r", prgs[i]);
			::Sleep(500);
		}

		theServer.stop();
	}

	printf("\rquiting.....\r");
	::Sleep(2000);

	pProglog = &ZQ::common::NullLogger;
	ZQ::common::setGlogger();
	ZQ::common::gThreadPool.clean();
	return 0;
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
