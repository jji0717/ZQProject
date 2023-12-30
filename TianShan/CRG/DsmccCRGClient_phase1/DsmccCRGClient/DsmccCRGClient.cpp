// DsmccCRGClient.cpp : Defines the entry point for the console application.

#include "DsmccClientCfg.h"
#include "SingleSession.h"
#include "DsmccThread.h"
#include "getopt.h"
#include <FileLog.h>
#include <iostream>

#define FAILED "SSS_MONITOR_1002"
#define SUCCESS "SSS_MONITOR_1001"

void usage() {

	std::cout << "Usage: DsmccCRGClient [option] [arg]\n\n"
		<< "Options:\n"
		<< "  -l <log file path>				The log file output path\n"
		<< "  -c <config file>					Configuration file\n"
		<< "  -d <ip:port>						Dsmcc server endpoint\n"
		<< "  -p <ip:port>						Lscp server endpoint\n"
		<< "  -m <comment>						comment\n"
		<< "  -i <MonitorIp>					MonitorIp\n"
		<< "  -h								display this screen\n"
		<< std::endl;
}

ZQ::DsmccCRGClient::DsmccCRGClientConfig _gConfig("");
ZQ::common::FileLog  *_gLog;
int main(int argc, char* argv[])
{
	std::string logfilepath = "./",configfile = "./DsmccScript.xml";
	std::string dsmcc,lscp,comment,MonitorIp;
	int option_index = 0;
	MonitorIp = "MonitorIp";

	if (argc < 5)
	{
		usage();
		return -1;
	}
	int ch = 0;
	while((ch = getopt(argc,argv,"c:l:d:p:i:m:h")) != EOF)
	{
		switch(ch)
		{
		case 'l':
			logfilepath = optarg;
			break;
		case 'c':
			configfile = optarg;
			break;
		case 'd':
			dsmcc = optarg;
			break;
		case 'p':
			lscp = optarg;
			break;
		case 'i':
			MonitorIp = optarg;
			break;
		case 'm':
			comment = optarg;
			break;
		case 'h':
			usage();
			return 0;
		default:
			break;
		}
	}

 	if(!_gConfig.load(configfile.c_str()))
 	{
 		std::cout << "load config file " << configfile  << " error" <<std::endl;
 		return -1;
 	}

	int i = 0;
	while(i < _gConfig.dsmccserver.size())
	{
		if (0 ==_gConfig.dsmccserver[i].protocol.compare("dsmcc"))
		{
			_gConfig.dsmccserver[i].ip = dsmcc.substr(0,dsmcc.find(":"));
			_gConfig.dsmccserver[i].port = dsmcc.substr(dsmcc.find(":")+1);
		}
		else if (0 == _gConfig.dsmccserver[i].protocol.compare("lscp"))
		{
			_gConfig.dsmccserver[i].ip = lscp.substr(0,lscp.find(":"));
			_gConfig.dsmccserver[i].port = lscp.substr(lscp.find(":")+1);
		}
		i++;
	}
/*	std::string dsmccIp = dsmcc.substr(0,dsmcc.find(":"));
	std::string dsmccPort = dsmcc.substr(dsmcc.find(":")+1);
	std::string lscpIp = lscp.substr(0,lscp.find(":"));
	std::string lscpPort = lscp.substr(lscp.find(":")+1);

	std::cout << dsmcc << std::endl;
	std::cout << "dsmccIp:" << dsmccIp <<std::endl<< "dsmccPort:" << dsmccPort << std::endl;
	std::cout << "lscpIp:" <<lscpIp <<std::endl << "lscpPort:" <<lscpPort << std::endl;
*/



	if ((logfilepath.find(".log")) == -1)
	{
		std::string str = logfilepath.substr(logfilepath.length()-1,1);
		if (str.compare("\\") != 0)
		{
			logfilepath += "\\";
		}
		logfilepath += _gConfig.logcfg.filename;
	}

	_gLog =new ZQ::common::FileLog(logfilepath.c_str(), ZQ::common::Log::L_DEBUG,_gConfig.logcfg.level, _gConfig.logcfg.size);
		
	(*_gLog)(ZQ::common::Log::L_INFO, LOGFMT("Start DsmccCRG Client"));

	_gLog->flush();
	
	SingleSession singlesession;
	bool ret = true;
	singlesession.Init();
	ret = singlesession.Start();

/*
	if (!singlesession.Init())
	{
//		(*_gLog)(ZQ::common::Log::L_ERROR, _T("SingleSession Init error"));
		(*_gLog)(ZQ::common::Log::L_INFO, LOGFMT("session completed: status[failed]"));
		_gLog->flush();
		delete _gLog;
		_gLog = NULL;
		return -1;
	}
	if (!singlesession.Start())
	{
		std::string cmdName,errorMessage;
		int responseCode, stepNum;
		singlesession.GetErrorInfo(cmdName,responseCode,errorMessage, stepNum);
		(*_gLog)(ZQ::common::Log::L_ERROR,LOGFMT("session completed:status[failed] step[%d] command[%s] ErrorCode[%d] ErrorMessage[%s]"),stepNum,cmdName.c_str(),responseCode,errorMessage.c_str());
		printf("session completed: status[failed] step[%d] command[%s]  ErrorCode[%d] ErrorMessage[%s]\n",stepNum,cmdName.c_str(),responseCode,errorMessage.c_str());
		_gLog->flush();
		delete _gLog;
		_gLog = NULL;
		return stepNum;
	}
*/
//	printf("session completed: status[success]\n");
//	(*_gLog)(ZQ::common::Log::L_INFO, LOGFMT("session completed: status[success]"));

	int retNum = 0;
	std::string cmdName,errorMessage,status;
	int responseCode, stepNum;
	singlesession.GetErrorInfo(status,cmdName,responseCode,errorMessage, stepNum);
	
	if(status == "failed")
		status = FAILED;
	else
		status = SUCCESS;

/*
	if (ret)
	{
		printf("session completed: status[success] comment[%s]\n",comment.c_str());
		(*_gLog)(ZQ::common::Log::L_INFO, LOGFMT("session completed: status[success] comment[%s]"),comment.c_str());
	}
	else
	{
		(*_gLog)(ZQ::common::Log::L_ERROR,LOGFMT("session completed:status[failed] comment[%s] step[%d] command[%s] ErrorCode[%d] ErrorMessage[%s]"),comment.c_str(),stepNum,cmdName.c_str(),responseCode,errorMessage.c_str());
		printf("session completed: status[failed] comment[%s] step[%d] command[%s]  ErrorCode[%d] ErrorMessage[%s]\n",comment.c_str(),stepNum,cmdName.c_str(),responseCode,errorMessage.c_str());
		retNum = stepNum;
	}
	*/

	(*_gLog)(ZQ::common::Log::L_INFO,LOGFMT("session completed:status[%s] step[%d][%s] result[%d][%s] MonitorIp[%s] comment[%s]"),status.c_str(),stepNum,cmdName.c_str(),responseCode,errorMessage.c_str(),MonitorIp.c_str(),comment.c_str());
	printf("session completed:status[%s] step[%d][%s] result[%d][%s] MonitorIp[%s] comment[%s]\n",status.c_str(),stepNum,cmdName.c_str(),responseCode,errorMessage.c_str(),MonitorIp.c_str(),comment.c_str());

	_gLog->flush();

	if(_gLog)
		delete _gLog;
	_gLog = NULL;
	return 0;
}

