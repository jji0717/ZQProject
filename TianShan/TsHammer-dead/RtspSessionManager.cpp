#include "RtspSessionManager.h"
#include "getopt.h"
#include "RtspSessionHandler.h"

#include "RtspDak.h"
#include "RtspClientFactory.h"

#include "ConfigHelper.h"
#include "XMLPreferenceEx.h"

#include <boost/regex.hpp>

#include <Mmsystem.h>

namespace ZQHammer
{

RtspSessionManager::RtspSessionManager()
:_sessionMap(NULL), _sessionWatchDog(NULL), _adjustTimeout(0), 
_log(NULL), _rtspDak(NULL)
{

}

RtspSessionManager::~RtspSessionManager()
{
	if (_sessionWatchDog != NULL)
	{
		delete _sessionWatchDog;
		_sessionWatchDog = NULL;
	}
	if (_sessionMap != NULL)
	{
		delete _sessionMap;
		_sessionMap = NULL;
	}
}

void RtspSessionManager::help()
{
	std::cout << "TSHammer.exe [-c XMLConfigFile] [-l LOGFile]" << std::endl;
	std::cout << "  -c  the config file"                        << std::endl;
	std::cout << "  -l  the log File"                           << std::endl;
}

bool RtspSessionManager::initial()
{
	// start rtsp dak
	_sessionMap = new (std::nothrow) RtspSessionMap(_log);
	if (NULL == _sessionMap)
	{
		std::cout << "Interval Error : failed to create rtsp session map" << std::endl;
		return false;
	}

	// start session watch dog
	_sessionPool.resize(_xml_ClientHandler.sessionThreads);
	_sessionWatchDog = new (std::nothrow) SessionWatchDog(_sessionPool, _log, *_sessionMap);
	if (!_sessionWatchDog)
	{
		std::cout << "Interval Error : failed to create session watch dog" << std::endl;
		return false;
	}
	_sessionWatchDog->start();
	return true;
}

int RtspSessionManager::start(int argc, char *argv[])
{
	// step 1 : parse command
	std::string strLogFile;
	std::string strConfigFile;
	if (!parseCommand(argc, argv, strLogFile, strConfigFile))
	{
		help();
		return 1;
	}

	// step 2 : open log
	openLog(strLogFile);

	// step 3 : parse configure file
	if (!parseConfig(strConfigFile))
	{
		help();
		return 1;
	}

	// step 4 : create session map and session watch dog
	if (!initial())
	{
		return 1;
	}

	if (!startDak())
	{
		return 1;
	}

	createSessions();

	startSessions();

	while (_sessionMap->getSessionNumbers() > 0)
	{
		size_t currentSessions = _sessionMap->getSessionNumbers();
		if (currentSessions < _runningSession)
		{
			std::cout <<  currentSessions << " session running task - current success : " << _sessionMap->getSuccessSessions() << std::endl;
		}
		else
		{
			std::cout <<  _runningSession << " session running task - current success : " << _sessionMap->getSuccessSessions() << std::endl;
		}
		Sleep(3000);
	}

	deleteSessions();

	_rtspDak->stop();
	_sessionWatchDog->quit();
	_rtspDak->release();
	_log->flush();
	std::cout << "TSHammer task over..." << std::endl;
	return 0;
}

bool RtspSessionManager::startDak()
{
	_sessionHandler = new (std::nothrow) RtspSessionHandler(_log, *_sessionMap);
	if (NULL == _sessionHandler)
	{
		return false;
	}
	_rtspDak = new (std::nothrow) ZQRtspCommon::RtspDak(*_log, _xml_ClientHandler.receiveThreads, _xml_ClientHandler.processThreads);
	if (NULL == _rtspDak)
	{
		std::cout << "Interval Error : failed to create rtsp dak" << std::endl;
		return false;
	}
	_rtspDak->registerHandler(_sessionHandler);
	if (!_rtspDak->start())
	{
		_rtspDak->release();
		std::cout << "Interval Error : failed to start rtsp dak" << std::endl;
		return false;
	}
	return true;
}

bool RtspSessionManager::parseCommand(int argc, char *argv[], std::string &strLogFile, std::string &strXMLFile)
{
	if (argc > 1)
	{
		if (::std::string(argv[1]).compare("/?") == 0 || ::std::string(argv[1]).compare("--help") == 0)
		{
			return false;
		}
	}
	int opt;
	while ((opt=getopt(argc, argv, "c:l:")) != EOF)
	{
		switch(opt)
		{
		case 'c':
			strXMLFile = strdup(optarg);
			break;
		case 'l':
			strLogFile = strdup(optarg);
			break;
		default:
			break;
		}
	}
	return true;
}

void RtspSessionManager::openLog(const std::string& strLogFile)
{
	_log = new ZQ::common::FileLog();
	std::string strLog = strLogFile;
	if (strLog.empty())
	{
		strLog = "TsHammerTest.log";
	}
	_log->open(strLog.c_str(), ::ZQ::common::Log::L_INFO, 10, 1024*1024*10, 1024, 1);
}

bool RtspSessionManager::parseConfig(std::string& strConfigFile)
{
	if (strConfigFile.empty())
	{
		strConfigFile = "RTSPScript.xml";
	}
	::ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	try 
	{
		if(!xmlDoc.open(strConfigFile.c_str()))
		{
			std::cout << "failed to open " << strConfigFile << std::endl;
			return false;
		}
		std::cout << "successed to open " << strConfigFile << std::endl;
	}
	catch (...)
	{
		std::cout << "failed to open " << strConfigFile << std::endl;
		return false;
	}

	// define 
	_xml_SessionHandler.setLogger(_log);
	_global_xml_SessCtxHanlder.setLogger(_log);
	_local_xml_SessCtxHandler.setLogger(_log);
	_xml_RtspServerHandler.setLogger(_log);
	_xml_ClientHandler.setLogger(_log);

	::ZQ::common::XMLUtil::XmlNode root = ::ZQ::common::XMLUtil::toShared(xmlDoc.getRootPreference());
	::ZQ::common::XMLUtil::XmlNode current = root;

	// read session configuration
	char name[iNameLen];
	if (current->name(name, iNameLen))
	{
		if (0 == std::string(SESSIONElement).compare(name) && _xml_SessionHandler.readAttribute(current))
		{
			std::cout << "<Session iteration=\"" << _xml_SessionHandler.iteration
				<< "\" loop=\""            << _xml_SessionHandler.loop
				<< "\" interval=\""        << _xml_SessionHandler.interval
				<< "\" timeout=\""         << _xml_SessionHandler.timeout
				<< "\">"                   << std::endl;
		}// end read session context

		RequestTransaction requestTransaction;
		requestTransaction.xml_RequestHandler.setLogger(_log);
		requestTransaction.xml_ResponseHandler.setLogger(_log);
		requestTransaction.xml_SleepHandler.setLogger(_log);

		::ZQ::common::XMLPreferenceEx * node = current->firstChild();
		//while (current->hasNextChild())
		bool bHaveRequest = false;
		while (node != NULL)
		{
			if(node->name(name, 32))
			{
				switch (getElementType(name))
				{
				case CLIENT_TYPE:
					if (_xml_ClientHandler.readAttribute(node))
					{
						std::cout << "<Client receiveThreads =\"" << _xml_ClientHandler.receiveThreads
							<< "\" processThreads=\""       << _xml_ClientHandler.processThreads
							<< "\" sessionThreads=\""       << _xml_ClientHandler.sessionThreads
							<< "\">"                        << std::endl;
					}
					break;
				case SERVER_TYPE:
					_xml_RtspServerHandler.readAttribute(node);
					break;
				case SESSIONCTX_TYPE:
					_global_xml_SessCtxHanlder.parseSessCtx(node, strGlobalType);
					_local_xml_SessCtxHandler.parseSessCtx(node, strLocalType);
					break;
				case REQUEST_TYPE:
					if (bHaveRequest)
					{
						_requestTransactions.push_back(requestTransaction);
						requestTransaction.xml_RequestHandler.bSkip = false;
						requestTransaction.xml_RequestHandler.strRequest = "";
						requestTransaction.xml_ResponseHandler.ResponseVec.clear();
						requestTransaction.xml_SleepHandler._sleepNode.wait = 0;
						bHaveRequest = false;
					}
					bHaveRequest = true;
					requestTransaction.xml_RequestHandler.readAttribute(node);
					requestTransaction.xml_RequestHandler.getContent(node);
					break;
				case RESPONSE_TYPE:
					requestTransaction.xml_ResponseHandler.getResponseSessCtx(node);
					break;
				case SLEEP_TYPE:
					requestTransaction.xml_SleepHandler.readAttribute(node);
					if (bHaveRequest)
					{
						_requestTransactions.push_back(requestTransaction);
						requestTransaction.xml_RequestHandler.bSkip = false;
						requestTransaction.xml_RequestHandler.strRequest = "";
						requestTransaction.xml_ResponseHandler.ResponseVec.clear();
						requestTransaction.xml_SleepHandler._sleepNode.wait = 0;
						bHaveRequest = false;
					}
					break;
				default:
					break;
				}
			}
			delete node;
			if (current->hasNextChild())
			{
				node = current->nextChild();
			}
			else
			{
				node = NULL;
			}
		} // end for while
		if (bHaveRequest)
		{
			_requestTransactions.push_back(requestTransaction);
			bHaveRequest = false;
		}
	} // end for if

	_adjustTimeout = _xml_SessionHandler.timeout;
	if (_adjustTimeout < 5000)
	{
		_adjustTimeout = 5000;
		std::cout << "adjust timeout - " << _adjustTimeout << std::endl;
	}
	_requestNums = _requestTransactions.size();

	_global_xml_SessCtxHanlder.setTailorProp(_xml_ClientHandler.strTailorType, _xml_ClientHandler.tailorRange);
	_local_xml_SessCtxHandler.setTailorProp(_xml_ClientHandler.strTailorType, _xml_ClientHandler.tailorRange);
	return true;
}

XMLElementType RtspSessionManager::getElementType(const char *name)
{
	if (0 == std::string(SESSIONElement).compare(name))
	{
		return SESSION_TYPE;
	}
	if (0 == std::string(CLIENTElement).compare(name))
	{
		return CLIENT_TYPE;
	}
	if (0 == std::string(RTSPSERVERElement).compare(name))
	{
		return SERVER_TYPE;
	}
	if (0 == std::string(SESSCTXElement).compare(name))
	{
		return SESSIONCTX_TYPE;
	}
	if (0 == std::string(REQUESTElement).compare(name))
	{
		return REQUEST_TYPE;
	}
	if (0 == std::string(RESPONSEElement).compare(name))
	{
		return RESPONSE_TYPE;
	}
	if (0 == std::string(SLEEPElement).compare(name))
	{
		return SLEEP_TYPE;
	}
	return UNKNOWN_TYPE;
}

void RtspSessionManager::createSessions()
{
	//ZQ::common::MutexGuard mutexGuard(_mutex);
	ZQRtspEngine::RtspClientFactory fac(*_log, _rtspDak);
	ZQRtspEngine::RtspClientPtr rtspCommunicator = NULL;
	RtspSession* rtspSession = NULL;
	if (SHARECONNECTION == _xml_RtspServerHandler.connectType)
	{
		rtspCommunicator = fac.createRtspClient(_xml_RtspServerHandler.ip, _xml_RtspServerHandler.port);
		if (rtspCommunicator != NULL)
		{
			for (register int i = 0; i <  _xml_SessionHandler.iteration; i++)
			{
				rtspSession = new (std::nothrow) RtspSession(_log, rtspCommunicator, *this, _xml_SessionHandler.loop, _requestNums, _local_xml_SessCtxHandler);
				_rtspSesssionList.push_back(rtspSession);
			}
		}
	}
	else
	{
		if (_xml_SessionHandler.iteration > 10000)
		{
			std::cout << "creating sessions........." << std::endl;
		}
		for (register int i = 0; i <  _xml_SessionHandler.iteration; i++)
		{
			rtspCommunicator = fac.createRtspClient(_xml_RtspServerHandler.ip, _xml_RtspServerHandler.port);
			if (rtspCommunicator != NULL)
			{
				rtspSession = new (std::nothrow) RtspSession(_log, rtspCommunicator, *this, _xml_SessionHandler.loop, _requestNums, _local_xml_SessCtxHandler);
				_rtspSesssionList.push_back(rtspSession);
			}
		}
	}
	if (_xml_SessionHandler.iteration > 0 && _rtspSesssionList.empty())
	{
		std::cout << "fail to connect server " <<_xml_RtspServerHandler.ip 
			<< " : "                      << _xml_RtspServerHandler.port << std::endl;
	}
}

void RtspSessionManager::deleteSessions()
{
	//ZQ::common::MutexGuard mutexGuard(_mutex);
	std::list<RtspSession*>::iterator iter = _rtspSesssionList.begin();
	for(iter = _rtspSesssionList.begin(); iter != _rtspSesssionList.end(); iter++)
	{
		delete (*iter);
	}
}

void RtspSessionManager::startSessions()
{
	DWORD interval = _xml_SessionHandler.interval;
	std::list<RtspSession*>::iterator iter = _rtspSesssionList.begin();
	register int i = 1;
	for (; iter != _rtspSesssionList.end(); iter++)
	{
		(*iter)->onTimer();
		std::cout << "current session " << i++  << " - current success :" << _sessionMap->getSuccessSessions() << std::endl;
		timeBeginPeriod(1);
		DWORD startTime = timeGetTime();
		while (1)
		{
			DWORD endTime = timeGetTime();
			if (endTime - startTime >= _xml_SessionHandler.interval)
			{
				break;
			}
			else
			{
				Sleep((_xml_SessionHandler.interval - (endTime - startTime)));
			}
		}
		timeEndPeriod(1);

		/*OnTimerRequest* request = new (std::nothrow) OnTimerRequest(_sessionPool, _log, *iter);
		if (request)
		{
			request->start();
			std::cout << "current session " << i++  << " - current success :" << _sessionMap->getSuccessSessions() << std::endl;
			if (interval > 0)
			{
				Sleep(interval);
			}
		}*/
	}
	_runningSession = i - 1;
}


bool RtspSessionManager::composeRequest(RtspSession* session, XML_SessCtxHandler& _xml_SessCtxHanlder, 
										size_t requestNum, std::string& strRequest)
{
	if (requestNum < 0 || requestNum >= _requestNums)
	{
		return false;
	}
	_xml_SessCtxHanlder.getGlobalSessCtxKey(_global_xml_SessCtxHanlder);
	_global_xml_SessCtxHanlder.modifyGlobalMacro();
	strRequest = _requestTransactions[requestNum].xml_RequestHandler.strRequest;
	if (_xml_SessCtxHanlder.fixupMacro(strRequest))
	{
		return true;
	}
	return false;
}

int RtspSessionManager::getSleepTime(size_t requestNum)
{
	if (requestNum < 0 || requestNum >= _requestNums)
	{
		return 0;
	}
	return _requestTransactions[requestNum].xml_SleepHandler._sleepNode.wait;
}

bool RtspSessionManager::parseResponse(size_t requestNum, std::string& strMsg, XML_SessCtxHandler& xml_SessCtxHandler)
{
	if (requestNum < 0 || requestNum >= _requestNums)
	{
		return false;
	}
	ResponseNodeVector ResponseVec = _requestTransactions[requestNum].xml_ResponseHandler.ResponseVec;

	boost::regex _regex;
	::std::vector<::std::string> msgLine;

	//split message to line by line
	ZQRtspCommon::StringHelper::splitMsg2Line(strMsg.c_str(), (uint16)strMsg.length(), msgLine);


	::std::string strValue;
	//try to match each regular expression
	for (ResponseNodeVector::iterator iter = ResponseVec.begin(); iter != ResponseVec.end(); iter++)
	{
		::std::string strSyntax;		
		try
		{
			strSyntax = (*iter).name + (*iter).syntax;
			_regex.assign(strSyntax);
			boost::cmatch results;
			for (::std::vector<::std::string>::iterator strIter = msgLine.begin(); strIter != msgLine.end(); strIter++)
			{
				if (boost::regex_match((*strIter).c_str(), results, _regex))
				{
					strValue.assign(results[0].first, results[0].second);
					XMLLOG(::ZQ::common::Log::L_DEBUG, CLOGFMT(XML_ResponseHandler,"match regular expression(%s) with value %s"), strSyntax.c_str(), strValue.c_str());

					//get value
					int iLen = (int)(*iter).name.length();
					while (strValue[iLen] == ' ')
						iLen++;
					strValue = strValue.substr(iLen);

					if ((*iter).value.compare("${1}") == 0)
						(*iter).value = strValue;
				}
			}
		}
		catch(boost::bad_expression& ex)
		{
			XMLLOG(::ZQ::common::Log::L_ERROR, CLOGFMT(XML_ResponseHandler, "Syntax [%S] error at %S"),strSyntax.c_str(), ex.what());
			return false;
		}
		catch(...)
		{
			XMLLOG(::ZQ::common::Log::L_ERROR, CLOGFMT(XML_ResponseHandler, "Initialize expression %s catch a exception"),strSyntax.c_str());
			return false;
		}
	}

	for (ResponseNodeVector::iterator iter = ResponseVec.begin(); iter != ResponseVec.end(); iter++)
	{
		MapValue value;
		value.type = strLocalType;
		value.value = (*iter).value;
		xml_SessCtxHandler._sessCtxMap[(*iter).key] = value;
		xml_SessCtxHandler.updateMacro((*iter).key, (*iter).value);
	}
	return true;
}

bool RtspSessionManager::getRequestSkip(size_t requestNum)
{
	if (requestNum < 0 || requestNum >= _requestNums)
	{
		return false;
	}
	return _requestTransactions[requestNum].xml_RequestHandler.bSkip;
}

}
