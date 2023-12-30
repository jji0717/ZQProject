#include "../Tunnel.h"

BOOL WINAPI ConsoleHandler(DWORD dwEvent);
bool bQuit = false;

int main(int argc, char* argv[])
{
	if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		// unable to install handler... 
		// display message to the user
		printf("Unable to install handler!\n");
		return -1;
	}

	ZQ::common::FtAddressPair bind("192.168.80.8", "192.168.0.138");
	Tunnels tunnels;

	TunnelConnection::_localid = "88888888-c447-4044-927f-fe0275fc70bd";

	if (!tunnels.listen(bind, 1970))
		return -1;

	// Sleep
	while (!::bQuit)
		::Sleep(200);
	printf("\n");

	tunnels.close();

	exit(0);
}

BOOL WINAPI ConsoleHandler(DWORD dwEvent)
{
    switch(dwEvent)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
		::bQuit = true;
        break;

    }
    return TRUE;
}

