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
// Ident : $Id: McastRcv.cpp,v 1.8 2004/07/28 09:30:34 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : the multicast package receiver utility
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/JndiClient/JndiClientTest.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 11    10-02-24 17:10 Hui.shao
// supported message properties
// 
// 10    10-02-22 20:30 Hui.shao
// 
// 9     10-02-22 19:55 Hui.shao
// supported setProducerOptions; value types of MapMessage covered Integer
// and Boolean; todo:OnMapMessage() need more works
// 
// 8     10-02-22 15:20 Hui.shao
// 
// 7     10-02-22 15:11 Hui.shao
// sinks the event of connection-establish and connection-lost
// 
// 6     10-02-21 18:35 Hui.shao
// added the support of MapMessage
// 
// 5     10-02-11 14:25 Fei.huang
// 
// 4     10-02-10 18:10 Fei.huang
// 
// 3     10-02-09 14:16 Fei.huang
// + merge to linux
// 
// 2     10-02-04 18:46 Hui.shao
// reviewed the log printing on reconnecting
// 
// 1     10-02-04 16:51 Hui.shao
// created
// ===========================================================================

#include "JndiClient.h"
#include "FileLog.h"
#include "getopt.h"
#include "SystemUtils.h"
extern "C"
{
#include <time.h>
#include <stdio.h>
}

#ifdef ZQ_OS_MSWIN
#include <direct.h>
#endif

#define BUFF_SIZE 255

#define MyName "JndiClientTest"

#ifdef ZQ_OS_MSWIN

#define GetCwd _getcwd

BOOL WINAPI ConsoleHandler(DWORD event);

#define JVM "\\jre\\bin\\client\\jvm.dll"

#else

#define GetCwd getcwd

#ifdef __x86_64
#define __a "amd64/server"
#else
#define __a "i386/client"
#endif

#define JVM "/jre/lib/"__a"/libjvm.so"
#endif

#define ClassPath "." PHSEPS "jbossall-client.jar"

bool bQuit = false;

class MyJmsSession : public ZQ::JndiClient::JmsSession
{
public:
	MyJmsSession(::ZQ::JndiClient::ClientContext& context, ::ZQ::JndiClient::JmsSession::DestType destType, const std::string& destName, bool asProducer =true, bool asConsumer=false) 
		throw(::ZQ::JndiClient::JndiException)
		: JmsSession(context, destType,destName, asProducer, asConsumer)
	{
	}

	virtual ~MyJmsSession() {}

	virtual void OnConnected(const std::string& notice)
	{
		printf("\nMyJmsSession::OnConnected() msg=%s", notice.c_str());
	}

	virtual void OnConnectionLost(const std::string& notice)
	{
		printf("\nMyJmsSession::OnConnectionLost() msg=%s", notice.c_str());
	}

	virtual void OnTextMessage(const std::string& message, const ZQ::JndiClient::ClientContext::Properties& msgProps)
	{
		std::string propstr;
		for (ZQ::JndiClient::ClientContext::Properties::const_iterator it2 = msgProps.begin(); it2 !=msgProps.end(); it2 ++)
			propstr += "[" + it2->first +"]=" +it2->second +"; ";

		printf("\nMyJmsSession::OnTextMessage() props: %s", propstr.c_str());
		printf("\nMyJmsSession::OnTextMessage() msg=%s", message.c_str());
	}

	virtual void OnMapMessage(const ZQ::JndiClient::JmsSession::MapMessage& message, const ZQ::JndiClient::ClientContext::Properties& msgProps)
	{
		std::string msgstr, propstr;
		for (ZQ::JndiClient::JmsSession::MapMessage::const_iterator it = message.begin(); it !=message.end(); it ++)
			msgstr += "[" + it->first +"]=" +it->second +"; ";

		for (ZQ::JndiClient::ClientContext::Properties::const_iterator it2 = msgProps.begin(); it2 !=msgProps.end(); it2 ++)
			propstr += "[" + it2->first +"]=" +it2->second +"; ";

		printf("\nMyJmsSession::OnMapMessage() props: %s", propstr.c_str());
		printf("\nMyJmsSession::OnMapMessage() %s", msgstr.c_str());
	}
};

int main(int argc, char* argv[])
{
    char buff[BUFF_SIZE];
    memset(buff, '\0', sizeof(buff));
    const char* path = GetCwd(buff, sizeof(buff)-1); 
    if(!path) {
        fprintf(stderr, "failed to get working directory: %s\n", strerror(errno));
        return errno;
    }
    std::string LogFile = std::string(path) + "/" + MyName".log";

    memset(buff, '\0', sizeof(buff));
    const char* jhome = getenv("JAVA_HOME"); 
    if(!jhome) {
        fprintf(stderr, "unknown java home directory: no JAVA_HOME set\n");
        return errno;
    }

    std::string java_home;

    size_t len = strlen(jhome);
    if(jhome[0] == '"' && jhome[len-1] == '"') {
        java_home = std::string(&jhome[1], &jhome[len-1]) + JVM;
    }
    else {
        java_home = std::string(jhome) + JVM;
    }

	ZQ::common::FileLog Logger(LogFile.c_str(), ZQ::common::Log::L_DEBUG, 1024*1024*50);
	Logger(ZQ::common::Log::L_NOTICE, "===================test starts =========");
	ZQ::JndiClient::ClientContext::initJVM(Logger, ClassPath, java_home.c_str()); 
	try
	{
		ZQ::JndiClient::ClientContext ctx("10.15.10.32:13001", ZQ::common::Log::L_DEBUG);
		MyJmsSession testSess(ctx, ZQ::JndiClient::JmsSession::DT_Queue, "queue/testQueue", true, true); 
//		MyJmsSession testSess(ctx, ZQ::JndiClient::JmsSession::DT_Topic, "topic/testTopic", true, true); 

#ifdef ZQ_OS_MSWIN
		::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE);
#endif
		::ZQ::JndiClient::JmsSession::MapMessage mapmsg, mapProps;
		::ZQ::JndiClient::JmsSession::setProperty(mapmsg, "k1", "v1");
		::ZQ::JndiClient::JmsSession::setBooleanProperty(mapmsg, "k4",  true);

		::ZQ::JndiClient::JmsSession::setProperty(mapProps, "p1", "i1");
		::ZQ::JndiClient::JmsSession::setBooleanProperty(mapProps, "p4",  true);

		testSess.setProducerOptions(5, 2000, ::ZQ::JndiClient::JmsSession::DM_Persisent); 

		int i=0;
		char buf[80];
		while (i< 20 && !bQuit)
		{
			SYS::sleep(1000);
			snprintf(buf, sizeof(buf)-2, "ping JMS: hello %03d", i);
			::ZQ::JndiClient::JmsSession::setProperty(mapmsg, "k3", buf);
			::ZQ::JndiClient::JmsSession::setIntegerProperty(mapmsg, "k2",  i);

			if (testSess.sendTextMessage(buf, mapProps))
//			if (testSess.sendMapMessage(mapmsg, mapProps))
				i++;
		}

		SYS::sleep(2000);
        printf("\n");
	}
	catch(...) {}

	ZQ::JndiClient::ClientContext::uninitJVM();

	return 0;
}

#ifdef ZQ_OS_MSWIN
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
#endif
