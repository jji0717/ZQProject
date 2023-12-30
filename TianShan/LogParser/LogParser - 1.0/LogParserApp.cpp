#include <iostream>
#include "getopt.h"
#include <string>
#include <FileLog.h>
#include "ZQ_common_conf.h"
#include "LogParserManager.h"



void usage() {

	std::cout << "Usage: LogParseClient [option] [arg]\n\n"
		<< "Options:\n"
		<< "  -l <log file path>                               The log file output path\n"
		<< "  -i <filePath fileType syntaxFilePath syntaxKey>  Be monitored log file information\n"  
		<< "  -s <thread pool size>                            The thread pool size\n"
		<< "  -d <database path>                               The database path\n"
		<< "  eg: LogParserApp.exe -l logparser.log -i \"test.log WinEventLog syntax.xml WinEventApp\" -s 2 -d ../data                        \n"
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
	std::string logFilePath,dataBasePath;
	int threadPoolSize;
	std::vector <LogParserManager::LoggerInfo> logInfos;
	int ch = 0;
	while((ch = getopt(argc,argv,"l:i:s:d:h:")) != EOF)
	{
		std::vector<std::string> result;
		LogParserManager::LoggerInfo info;
		switch(ch)
		{
		case 'l':
			logFilePath = optarg;
			std::cout << "logFilePath:" << logFilePath <<std::endl;
			break;
		case 'i':
			std::cout << "fileInof:" << optarg <<std::endl;
			ZQ::common::stringHelper::SplitString(optarg, result, " ", " ");
			if (result.size() < 4)
				break;
			info.logFile = result[0];
			info.logType = result[1];
			info.syntaxFile = result[2];
			info.syntaxKey = result[3];
			logInfos.push_back(info);
			break;
		case 's':
			threadPoolSize = atoi(optarg);
			std::cout << "threadPoolSize:" << threadPoolSize <<std::endl;
			break;
		case 'd':
			dataBasePath = optarg;
			std::cout << "dataBasePath:" << dataBasePath <<std::endl;
			break;
		case 'h':
			usage();
			return 0;
		default:
			break;
		}
	}

	for(int i = 0;i < logInfos.size();i++)
	{
		std::cout << "logFile:" << logInfos[i].logFile <<std::endl;
		std::cout << "logType:" << logInfos[i].logType <<std::endl;
		std::cout << "syntaxFile:" << logInfos[i].syntaxFile <<std::endl;
		std::cout << "syntaxKey:" << logInfos[i].syntaxKey <<std::endl;
	}

	ZQ::common::Log* pLog = new ZQ::common::FileLog(logFilePath.c_str(),7, 5,10240000);
	if (pLog == NULL)
	{
		printf("Failed to create a log file %s.\n",logFilePath.c_str());
		return -1;
	}

	Ice::InitializationData initData;
	// init the ice properties
	Ice::PropertiesPtr proper= Ice::createProperties();

	proper->setProperty("Ice.Trace.GC", "2");
	proper->setProperty("Ice.GC.Interval", "20");
	proper->setProperty("Ice.Override.Timeout", "30000");
	proper->setProperty("Ice.Override.ConnectTimeout", "2000");

	initData.properties = proper;
	Ice::CommunicatorPtr ic = Ice::initialize(initData);

	ZQ::common::NativeThreadPool monitorPool(threadPoolSize);
	LogParserManager* pMgr = new LogParserManager((*pLog),monitorPool,200,1000);
	if (!pMgr->init(ic,dataBasePath,10))
	{
		delete pLog;
		pLog = NULL;
		delete pMgr;
		pMgr = NULL;
		return -1;
	}
	
	for (int i = 0; i< logInfos.size();i++)
	{
		if (!pMgr->addMonitoring(logInfos[i]))
		{
			(*pLog)(ZQ::common::Log::L_ERROR, CLOGFMT(LogParserManager, "addMonitoring() failing to monitor the log file. file=%s"), logInfos[i].logFile.c_str());
			delete pLog;
			pLog = NULL;
			delete pMgr;
			pMgr = NULL;
			return -1;
		}	
	}
	pMgr->start();
	while (1)
	{
		Sleep(5000);
	}
	return 0;
}

