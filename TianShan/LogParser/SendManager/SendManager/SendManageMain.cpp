#include <iostream>
#include "getopt.h"
#include <string>
#include <FileLog.h>
#include <TimeUtil.h>
#include "SenderManager.h"

void usage() {

	std::cout << "Usage: LogParseClient [option] [arg]\n\n"
		<< "Options:\n"
		<< "  -l <log file path>                               The log file output path\n"
		<< "  -t <send type>                                   Event send type\n"
		<< "  -d <send dll path>                               Dynamic send library paths\n"
		<< "  -c <send dll config file path>                   Dynamic send library configuration file path\n"
		<< "  -h	                                           display this screen\n"
		<< std::endl;
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		usage();
		return -1;
	}

	std::string logFilePath,dllpath,type,configpath;
	int ch = 0;
	while((ch = getopt(argc,argv,"l:t:d:c:h:")) != EOF)
	{
		switch(ch)
		{
		case 'l':
			logFilePath = optarg;
			std::cout << "logFilePath:" << logFilePath <<std::endl;
			break;
		case 't':
			type = optarg;
			std::cout << "type:" << type <<std::endl;
			break;
		case 'd':
			dllpath = optarg;
			std::cout << "dllpath:" << dllpath <<std::endl;
			break;
		case 'c':
			configpath = optarg;
			std::cout << "configPath:" << configpath <<std::endl;
			break;
		case 'h':
			usage();
			return 0;
		default:
			break;
		}
	}

	ZQ::common::Log* pLog = new ZQ::common::FileLog(logFilePath.c_str(),7, 5,10240000);
	if (pLog == NULL)
	{
		printf("Failed to create a log file %s.\n",logFilePath.c_str());
		return -1;
	}
	MessageSenderPump::_sendModule module;
	module.dll = dllpath;
	module.config = configpath;
	module.type = type;
	SenderManager* sendManager = new SenderManager((*pLog),module);
	int64 i = 0;
 	while(1)
 	{
		MSGSTRUCT msg;
		msg.id = 12;
		msg.category = "ClusterContentStore";
		msg.timestamp = "13115444042894";
		msg.eventName = "ServiceStarted";
		msg.sourceNetId = "192.168.81.55";
		//	msg.property = 

		MessageIdentity mid;
		mid.source = "test.log";
		mid.position = i;
		mid.stamp = ZQ::common::now();
		sendManager->SendMsg(msg,mid,type);
		i++;
		Sleep(200);
	}





}