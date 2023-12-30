// FileName : RtspSessionManager.h
// Author   : Zheng Junming
// Date     : 2009-11
// Desc     : 

#ifndef __TS_HAMMER_RTSP_SESSION_MANAGER_H__
#define __TS_HAMMER_RTSP_SESSION_MANAGER_H__

#include "RtspInterface.h"
#include "RtspSession.h"

#include "XML_Handler/XML_SessionHandler.h"
#include "XML_Handler/XML_ClientHandler.h"
#include "XML_Handler/XML_RTSPServerHandler.h"
#include "XML_Handler/XML_SessCtxHandler.h"

#include "XML_Handler/XML_RequestHandler.h"
#include "XML_Handler/XML_ResponseHandler.h"
#include "XML_Handler/XML_SleepHandler.h"

namespace ZQHammer
{

typedef enum
{
	SESSION_TYPE,
	CLIENT_TYPE,
	SERVER_TYPE,
	SESSIONCTX_TYPE,
	REQUEST_TYPE,
	RESPONSE_TYPE,
	SLEEP_TYPE,
	UNKNOWN_TYPE
}XMLElementType;

typedef struct
{
	XML_RequestHandler xml_RequestHandler;
	XML_ResponseHandler xml_ResponseHandler;
	XML_SleepHandler xml_SleepHandler;
}RequestTransaction;


class RtspSessionHandler;

class RtspSessionManager
{
public:
	RtspSessionManager();

	~RtspSessionManager();

	int start(int argc, char* argv[]);

public:
	bool getRequestSkip(size_t requestNum);

	bool composeRequest(RtspSession* session, XML_SessCtxHandler& _xml_SessCtxHanlder, 
		size_t requestNum, std::string& strRequest);
	
	int getSleepTime(size_t requestNum);

	bool parseResponse(size_t requestNum, std::string& strRawMsg, XML_SessCtxHandler& _xml_SessCtxHanlder);

private:
	void help();

	bool parseCommand(int argc, char* argv[], std::string& strLogFile, std::string& strXMLFile);

	void openLog(const std::string& strLogFile);

	bool parseConfig(std::string& strConfigFile);

	XMLElementType getElementType(const char* elementName);

	bool startDak();

	bool initial();

	void createSessions();

	void startSessions();

	void deleteSessions();

public:
	RtspSessionMap* _sessionMap;
	ZQ::common::NativeThreadPool _sessionPool;
	SessionWatchDog* _sessionWatchDog;
	int _adjustTimeout;

private:
	ZQ::common::FileLog* _log;
	ZQRtspCommon::IRtspDak* _rtspDak;	
	std::list<RtspSession*> _rtspSesssionList;

public:
	XML_CLIENTHandler _xml_ClientHandler;

private:
	XML_SessionHandler _xml_SessionHandler;
	XML_RtspServerHandler _xml_RtspServerHandler;
	XML_SessCtxHandler _global_xml_SessCtxHanlder;
	XML_SessCtxHandler _local_xml_SessCtxHandler; // pass to session
	std::vector<RequestTransaction> _requestTransactions;
	size_t _requestNums;
	int _runningSession;

private:
	RtspSessionHandler* _sessionHandler;
};

} // end for ZQHammer

#endif
