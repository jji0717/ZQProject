// TsPump.cpp : Defines the entry point for the console application.
//
#include  <io.h>
#include  <stdio.h>
#include  <stdlib.h>
#include "TsPump.h"

void Usage()
{
	printf("Usage: <TsPump> <bindIP>  <bindPort> <FileFolder> <interval> <hexMode> <deHexCommand> \n");
	printf("       bindIp:       the local IP from the UDP pumper to bind \n");
	printf("       bindPort:     the local port from the UDP pumper to bind \n");
	printf("       FileFolder:   the file folder where to monitor the TS/HEX input files \n");
	printf("       interval:     interval between two pump for the packets of a same service group. default value(100ms)\n");
	printf("       hexMode:      0 to input TS file, 1 to input hexfile. default value(0)\n");
	printf("       deHexCommand: command line to covert hex file to binary ts file.default value(xxd -s)\n");
}
bool bQuit = false;
int main(int argc, char* argv[])
{
	int interval = 100;
	bool bHex = false;
	std::string deHexCommond = "xxd -s";

	if(argc < 4)
	{
		Usage();
		return -1;
	}

	std::string ipAddress = argv[1];
	int         ipPort = atoi(argv[2]);
	std::string fileFolder = argv[3];

	if(_access(fileFolder.c_str(), 0) != 0)
	{
		printf("The invaild file folder(%s)\n", fileFolder.c_str());
		Usage();
		return -1;
	}
	if(argc > 4)
		interval = atoi(argv[4]);
	if(argc > 5)
	{
		int hex = atoi(argv[5]);
		if(hex == 0)
			bHex = 0;
		else
			bHex = 1;
	}
	if(argc >6)
		deHexCommond = argv[6];

	printf("  bindIP(%s)bindPort(%d)FileFolder(%s)interval(%d)hexMode(%d)deHexCommand(%s)\n", 
		ipAddress.c_str(), ipPort, fileFolder.c_str(),interval, bHex,deHexCommond.c_str());

	ZQ::common::InetHostAddress bindAddr(ipAddress.c_str());
	// TODO: if want to read hex file instead of ts file, pls specify the de-hex command first, such as
	//         TsPumper::_cmd_Dehex = "xxd -s";

   //TsPumper ts(bindAddr, 22222, "d:\\temp\\aaa\\ts", true, 000);

	TsPumper ts(bindAddr, ipPort, fileFolder.c_str(), bHex, interval);
	ts.start();
	while(getchar() != 'q')
	{
      Sleep(1000);
	}
    ts.stop();
	return 0;
}
