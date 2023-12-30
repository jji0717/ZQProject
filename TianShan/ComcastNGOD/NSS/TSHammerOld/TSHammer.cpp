// RTSPScriptHammer.cpp : 定义控制台应用程序的入口点。
//
#include <winsock2.h>
#include <Windows.h>
#include <Mmsystem.h>

#include <vector>
#include <FileLog.h>
#include <Log.h>
#include <XMLPreferenceEx.h>
#include "ConfigHelper.h"
#include "getopt.h"

//XML handler
#include "XML_RTSPServerHandler.h"
#include "XML_SessionHandler.h"
#include "XML_SessCtxHandler.h"
#include "XML_RequestHandler.h"
#include "XML_ResponseHandler.h"
#include "XML_SleepHandler.h"

//RTSP thread
#include "RTSP_MessageChopThrd.h"
#include "RTSP_MessageParseThrd.h"
#include "RTSP_MessageRecvThrd.h"
#include "RTSP_MessageSendThrd.h"

//Session Watch Dog
#include "SessionWatchDog.h"

#include <sstream>
using namespace std;

#include <boost/regex.hpp>

uint16 g_uCSeq = 1;

SessCSeq g_SessCSeq(1);
int iSessTimeOut;
#define DEFAULTTHREADNUM 30

void printfUsage()
{
	printf("press test tool for tianshan.\n");
	printf("\n");
	printf("TSHammer.exe [-c XMLConfigFile] [-l LOGFile]");
	printf("\n");
	printf("  -c  the config file\n");
	printf("  -l  the log File\n");
}

void HammerSchedule(int argc, char* argv[])
{
	char* strXMLFilePath= NULL;

	::ZQ::common::FileLog *fileLog = new ::ZQ::common::FileLog();
	if (argc > 1)
	{
		if (::std::string(argv[1]).compare("/?") == 0 || ::std::string(argv[1]).compare("--help") == 0)
		{
			printfUsage();
			return;
		}
	}
	//else if (argc == 3)
	//{
	//	strXMLFilePath = argv[1];
	//	fileLog->open(argv[2], ::ZQ::common::Log::L_INFO, 10, 1024*1024*10, 1024, 1);
	//}
	//else if (argc == 2)
	//{
	//	strXMLFilePath = argv[1];
	//}
	//else
	//{
	//	printf("input parameter exceed max number required, only use the front two parameters");
	//	strXMLFilePath = argv[1];
	//	fileLog->open(argv[2], ::ZQ::common::Log::L_INFO, 10, 1024*1024*10, 1024, 1);
	//}
	char *logFile = NULL;
	int opt;
	while( (opt=getopt(argc, argv, "c:l:")) != EOF )
	{
		switch(opt)
		{
			case 'c':
				strXMLFilePath = strdup(optarg);
				break;
			case 'l':
				logFile = strdup(optarg);
				break;
			default:
				break;
		}
	}
	if (strXMLFilePath == NULL)
		strXMLFilePath = "RTSPScript.xml";
	if (NULL != logFile)
		fileLog->open(logFile, ::ZQ::common::Log::L_INFO, 10, 1024*1024*10, 1024, 1);
	else
		fileLog = dynamic_cast<::ZQ::common::FileLog *>(ZQ::common::getGlogger());
	
	::ZQ::common::NativeThreadPool pool(DEFAULTTHREADNUM);
	::ZQ::common::NativeThreadPool pool_watchDog(DEFAULTTHREADNUM);

	//open xml file
	::ZQ::common::XMLPreferenceDocumentEx xmlDoc;

	try 
	{
		if(xmlDoc.open(strXMLFilePath))
			printf("Opened file [%s] successfully.\n", strXMLFilePath);
	}
	catch (...)
	{
		printf("Failed to open default config file [%s].\r\n", strXMLFilePath);
		printfUsage();
		return;
	}

	::ZQ::common::XMLUtil::XmlNode root = ::ZQ::common::XMLUtil::toShared(xmlDoc.getRootPreference());
	::ZQ::common::XMLUtil::XmlNode current = root;

	XML_RtspServerHandler	_xml_RtspServerHandler;
	_xml_RtspServerHandler.setLogger(fileLog);

	XML_SessionHandler		_xml_SessionHandler;
	_xml_SessionHandler.setLogger(fileLog);

	char name[iNameLen];

	RTSPMessageBuffer	rtspMessageBuffer;
	RTSPMessageList		rtspMessageList;

	::ZQ::common::XMLPreferenceEx *node;
	if(current->name(name, iNameLen))
	{
		if (string(SESSIONElement).compare(name) == 0)
		{
			if (_xml_SessionHandler.readAttribute(current))
			{
				cout << "<Session seqId=\"" << _xml_SessionHandler._sessionNode.seqId 
					 << "\" desc=\"" << _xml_SessionHandler._sessionNode.desc
					 << "\" iteration=\"" << _xml_SessionHandler._sessionNode.iteration
					 << "\" loop=\"" << _xml_SessionHandler._sessionNode.loop
					 << "\" interval=\"" << _xml_SessionHandler._sessionNode.interval
					 << "\" timeout=\"" << _xml_SessionHandler._sessionNode.timeout
					 << "\">" << endl;
				iSessTimeOut = _xml_SessionHandler._sessionNode.timeout;
			}
		}

		RTSP_MessageRecvThrd	*rtspRecvThrd = NULL;
		RTSP_MessageChopThrd	*rtspChopThrd = NULL;
		RTSP_MessageParseThrd	*rtspParseThrd = NULL;

		//set global SessCtxHandler
		XML_SessCtxHandler xml_SessionCtxHandler;
		xml_SessionCtxHandler.setLogger(fileLog);

		SessionMap sessionMap(*fileLog, _xml_RtspServerHandler);

		//parse <Session> content
		node = current->firstChild();

		while (current->hasNextChild())
		{
			if(node->name(name, 32))
			{
				if (string(RTSPSERVERElement).compare(name) == 0)
				{
					if (_xml_RtspServerHandler.readAttribute(node))
					{
						RTSPMessageBufferSize = _xml_RtspServerHandler._rtspServerNode.bufferSize * 1024;;
						if (_xml_RtspServerHandler._rtspServerNode.type.compare(SHARECONNECTION) == 0)
						{
							_xml_RtspServerHandler.initSessionSocket();
							_xml_RtspServerHandler.connectServer();
						}
						rtspRecvThrd = new RTSP_MessageRecvThrd(*fileLog, sessionMap);
					}
				}
				else if (string(SESSCTXElement).compare(name) == 0)
				{
					//get global sessCtx
					xml_SessionCtxHandler.parseSessCtx(node, strGlobalType);
				}
			}

			delete node;
			node = current->nextChild();
		}//while
	
		SessionWatchDog sessionWatchDog(pool_watchDog, *fileLog, sessionMap);
		sessionWatchDog.start();
		typedef ::std::list<SessionHandler *> SessionHandlerList;
		SessionHandlerList sessionHandlerList;
		for (int i = 0; i < _xml_SessionHandler._sessionNode.iteration; i++)//0 is used for default global session
		{
			//XML_SessCtxHandler *xml_SessStxHandler = new XML_SessCtxHandler();
			//xml_SessStxHandler->setLogger(&fileLog);	
			SessionHandler *sessionHandler = new SessionHandler(sessionWatchDog, *fileLog, pool, _xml_RtspServerHandler, xmlDoc, xml_SessionCtxHandler,sessionMap, _xml_SessionHandler._sessionNode.loop);
			rtspRecvThrd->addSocket(sessionHandler->_sessionSocket);	
			sessionHandlerList.push_back(sessionHandler);
			//printf("initialize %d session\n", i);
			/*sessionHandler->OnTimer();
			DWORD startTime = GetTickCount();
			while (1)
			{
				DWORD endTime = GetTickCount();
				if (endTime - startTime >= _xml_SessionHandler._sessionNode.interval)
					break;
				else
					Sleep(_xml_SessionHandler._sessionNode.interval - (endTime - startTime));
			}*/
			//sessionMap.addSessionHandler()
		}

		//connect to server success, start rtsp session thread
		rtspRecvThrd->startThrd();
		int s = 1;
		for (SessionHandlerList::iterator iter = sessionHandlerList.begin(); iter != sessionHandlerList.end(); iter++)
		{
			(*iter)->OnTimer();
			printf("current session : %d\n", s++);

			timeBeginPeriod(1);			
			//DWORD startTime = GetTickCount();
			DWORD startTime = timeGetTime();
			while (1)
			{
				//DWORD endTime = GetTickCount();
				DWORD endTime = timeGetTime();
				if (endTime - startTime >= _xml_SessionHandler._sessionNode.interval)
					break;
				else
				{
					Sleep((_xml_SessionHandler._sessionNode.interval - (endTime - startTime)));
					//for (int i = 1; i < 2; i++)
						//;
				}
			}
			timeEndPeriod(1);
		}
		//sessionHandlerList.clear();

		//Sleep(2*60*1000);
		while (!sessionHandlerList.empty() && sessionMap.getSessionNum() > 0)
		{
			printf("%d session running task\n", sessionMap.getSessionNum());
			for (SessionHandlerList::iterator iter = sessionHandlerList.begin(); iter != sessionHandlerList.end(); iter++)
			{
				if ((*iter)->getExit())
				{
					SessionHandler *sess = *iter;
					sessionHandlerList.erase(iter);
					delete sess;
					if (sessionHandlerList.empty())
						break;
					iter = sessionHandlerList.begin();
				}
			}
			Sleep(3000);
		}
		delete rtspRecvThrd;
		printf("TSHammer task over...\n");
		//system("pause");

	}//if getname()...
}

int main(int argc, char* argv[])
{
	//test_regex();
	//test_xml();
	HammerSchedule(argc, argv);
	//system("pause");
	return 0;
}