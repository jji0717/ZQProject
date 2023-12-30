#include "SessionWatchDog.h"
#include "RTSP_MessageSendThrd.h"

extern SessCSeq g_SessCseq(1);
extern int iSessTimeOut;

SessionHandler::SessionHandler(SessionWatchDog &sessionWatchDog, ::ZQ::common::FileLog &fileLog, ::ZQ::common::NativeThreadPool &pool, XML_RtspServerHandler &xml_RtspServerHandler, ::ZQ::common::XMLPreferenceDocumentEx &root, XML_SessCtxHandler &gxml_SessCtxHandler, SessionMap &sessIdMap, uint32 loopNum)
:_sessionWatchDog(sessionWatchDog)
,_log(&fileLog)
,_pool(pool)
//,_xml_RtspServerHandler(xml_RtspServerHandler)
,_XMLNodeRoot(::ZQ::common::XMLUtil::toShared(root.getRootPreference()))
,_gxml_SessCtxHandler(gxml_SessCtxHandler)
,_sessIdMap(sessIdMap)
,_loopNum(loopNum)
,_bConnected(false)
,_bExit(false)
{
	if (xml_RtspServerHandler._rtspServerNode.type.compare(PERSESSCONNECTION) == 0)
	{
		//_xml_RtspServerHandler = xml_RtspServerHandler;
		_xml_RtspServerHandler.setLogger(&fileLog);
		_xml_RtspServerHandler._rtspServerNode = xml_RtspServerHandler._rtspServerNode;
		
		if (_xml_RtspServerHandler.initSessionSocket() == false)
			printf("create socket failed!\n");
		
		_sessionSocket = &(_xml_RtspServerHandler._sessionSocket);
	}
	else if (xml_RtspServerHandler._rtspServerNode.type.compare(SHARECONNECTION) == 0)
	{
		_bConnected = true;
		_sessionSocket = &(xml_RtspServerHandler._sessionSocket);
	}

	if (iSessTimeOut <= 0)
		_timeout = REQ_TIMEOUT;
	else
		_timeout = iSessTimeOut * 1000;

	bWaitingForResponse = false;
	bXMLEnd = false;
	bTimeout = false;

	_xml_SessCtxHandler.setLogger(_log);
	_xml_RequestHandler.setLogger(_log);
	_xml_ResponseHandler.setLogger(_log);
	_xml_SleepHandler.setLogger(_log);

	_sessId.clear();
	

	_XMLRootNodeBase = _XMLNodeRoot;
	_XMLChildNode = _XMLNodeRoot->firstChild();

	_cseqIdx = 0;
	//InitializeCriticalSection(&_CS);
}

SessionHandler::~SessionHandler()
{
	//lock();
	::ZQ::common::MutexGuard guard(_mutex);
	if (_cseqIdx > 0)
	{
		_sessIdMap.removeSessionHandlerByCSeq(_cseqIdx);
		_cseqIdx = 0;
	}
	else
		_sessIdMap.removeSessionHandlerByCSeq(this);

	_sessionSocket = NULL;
	
	//_XMLNodeRoot->free();
	//_XMLRootNodeBase->free();
	_log = NULL;
	//unlock();
	//DeleteCriticalSection(&_CS);
}

void SessionHandler::PostRequest(::std::string &strMessage)
{
	if (_sessionSocket->m_Status == false)
	{
		CloseSocket(_xml_RtspServerHandler._sessionSocket.m_Socket);
		if (_xml_RtspServerHandler.initSessionSocket())
			_sessionSocket = &(_xml_RtspServerHandler._sessionSocket);
		else
		{
			XMLLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SessionHandler, "Session[%s](CSeq:%d) socket error"), _sessId.c_str(), _uLastCSeq);
			return;
		}
	}
	//compose and send out the request
	RTSP_MessageSendThrd *rtspSendThrd = new RTSP_MessageSendThrd(*_log, _pool, *_sessionSocket, strMessage, _xml_SessCtxHandler);
	rtspSendThrd->start();

	bWaitingForResponse=true;
	bTimeout = false;
	if (_sessId.empty())
	{
		if (_cseqIdx > 0)
			_sessionWatchDog.watchSession(_cseqIdx, _timeout);
		else
			_sessionWatchDog.watchSession(_uLastCSeq, _timeout);
	}
	else
		_sessionWatchDog.watchSession(_sessId, _timeout);
}

void SessionHandler::OnTimer()
{
	::ZQ::common::MutexGuard guard(_mutex);
	//lock();
	if (_xml_RtspServerHandler._rtspServerNode.type.compare(PERSESSCONNECTION) == 0 && _bConnected == false)
	{
		if (_xml_RtspServerHandler.connectServer() == false)
		{
			XMLLOG(ZQ::common::Log::L_INFO, CLOGFMT(SessionHandler, "Session[%s](CSeq:%d, socket:%d) connect to server faild"), _sessId.c_str(), _uLastCSeq, _xml_RtspServerHandler._sessionSocket.m_Socket);
			unlock();
			return;
		}
		else
		{
			XMLLOG(ZQ::common::Log::L_INFO, CLOGFMT(SessionHandler, "Session[%s](CSeq:%d, socket:%d) connect to server success"), _sessId.c_str(), _uLastCSeq, _xml_RtspServerHandler._sessionSocket.m_Socket);
			_bConnected = true;
		}
	}
	if (bWaitingForResponse)
	{
		bTimeout = true;
		bWaitingForResponse = false;

		if (!_sessId.empty())
			_sessIdMap.removeSessionHandler(this);

		if (_cseqIdx > 0)
		{
			_sessIdMap.removeSessionHandlerByCSeq(_cseqIdx);
			_cseqIdx = 0;
		}
		else
			_sessIdMap.removeSessionHandlerByCSeq(this);

		XMLLOG(ZQ::common::Log::L_INFO, CLOGFMT(SessionHandler, "Session[%s](CSeq:%d, socket:%d) response timeout"), _sessId.c_str(), _uLastCSeq, _xml_RtspServerHandler._sessionSocket.m_Socket);
	}
	if (/*bWaitingForResponse || */bXMLEnd)
	{
		//TODO: remove session from list
		XMLLOG(ZQ::common::Log::L_INFO, CLOGFMT(SessionHandler, "Session[%s](CSeq:%d, socket:%d) End of Script and removed"), _sessId.c_str(), _uLastCSeq, _xml_RtspServerHandler._sessionSocket.m_Socket);

		if (!_sessId.empty())
			_sessIdMap.removeSessionHandler(this);
		_sessIdMap.removeSessionHandlerByCSeq(this);

		//unlock();
		//delete this;
		setExit(true);
		return;
	}

	char name[32];

	uint16 preCSeq = 0;
	while (_loopNum > 0)
	{
		while (_XMLChildNode != NULL)
		{
			if(_XMLChildNode->name(name, 32))
			{
				if (string(SESSCTXElement).compare(name) == 0)
				{
					if (_xml_SessCtxHandler.parseSessCtx(_XMLChildNode, strLocalType))
					{
						if (!_sessId.empty())
						{
							_sessionWatchDog.watchSession(_sessId, DEFAULT_TIMEOUT);
						}
					}
					nextXMLNode();
				}
				else if (string(REQUESTElement).compare(name) == 0)
				{
					if (_xml_RequestHandler.readAttribute(_XMLChildNode))
					{
						if (_xml_RequestHandler.bSkip && bTimeout)
						{
							nextXMLNode();
							continue;
						}
						else
							bTimeout = false;
					}
					if (_xml_RequestHandler.getContent(_XMLChildNode))
					{
						::std::string strCSeq = g_SessCseq.getCSeq();
						_uLastCSeq = atoi(strCSeq.c_str());

						if (_sessId.empty())
						{
							//_sessIdMap.getLock();

							if (_cseqIdx > 0)
							{
								_sessIdMap.removeSessionHandlerByCSeq(_cseqIdx);
								_cseqIdx = 0;
							}
							_cseqIdx = _uLastCSeq;
							_sessIdMap.addSessionHandler(_uLastCSeq, this);

							//if (preCSeq > 0)
							//	_sessIdMap.removeSessionHandlerByCSeq(preCSeq);
							//_sessIdMap.releaseLock();
						}

						::std::string strRequest = _xml_RequestHandler.formatRequest();

						_xml_SessCtxHandler.getGlobalSessCtxKey(_gxml_SessCtxHandler);

						_xml_SessCtxHandler.updateMacro(SESSCSEQ, strCSeq);
						
						PostRequest(strRequest);

						_gxml_SessCtxHandler.modifyGlobalMacro();

						_xml_RequestHandler.RequestVec.clear();
					}
					nextXMLNode();
				}
				else if (string(RESPONSEElement).compare(name) == 0)
				{
					if (bTimeout)
					{
						nextXMLNode();
						continue;
					}
					if (_xml_ResponseHandler.getResponseSessCtx(_XMLChildNode))
					{
						//TODO:
					}
					if (_sessId.empty())
					{
						if (_cseqIdx > 0)
							_sessionWatchDog.watchSession(_cseqIdx, _timeout);
						else
							_sessionWatchDog.watchSession(_uLastCSeq, _timeout);
					}
					else
						_sessionWatchDog.watchSession(_sessId, _timeout);
					//unlock();
					nextXMLNode();
					return;
				}
				else if (string(SLEEPElement).compare(name) == 0)
				{
					if (bTimeout)
					{
						nextXMLNode();
						continue;
					}
					if (_xml_SleepHandler.readAttribute(_XMLChildNode))
					{
						nextXMLNode();
						XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionHandler, "Session[%s](CSeq[%d])<Sleep wait=\"%d\" />"), _sessId.c_str(), _uLastCSeq, _xml_SleepHandler._sleepNode.wait);
						//cout << "<Sleep wait=\"" << _xml_SleepHandler._sleepNode.wait << "\" />" << endl;
						//Sleep(_xml_SleepHandler._sleepNode.wait);
						if (_sessId.empty())
						{
							if (_cseqIdx > 0)
								_sessionWatchDog.watchSession(_cseqIdx, _xml_SleepHandler._sleepNode.wait);
							else
								_sessionWatchDog.watchSession(_uLastCSeq, _xml_SleepHandler._sleepNode.wait);
						}
						else
							_sessionWatchDog.watchSession(_sessId, _xml_SleepHandler._sleepNode.wait);
						//unlock();
						return;
					}
					nextXMLNode();
				}
				else
					nextXMLNode();
			}//if (name...)
		}

		if (!bTimeout && bWaitingForResponse)
		{
			_sessionWatchDog.watchSession(_sessId, iSessTimeOut);
			XMLLOG(ZQ::common::Log::L_INFO, CLOGFMT(SessionHandler, "Session[%s](CSeq:%d) wait for response"), _sessId.c_str(), _uLastCSeq);
			//unlock();
			return;
		}
		_loopNum--;
		_XMLNodeRoot = _XMLRootNodeBase;
		_XMLChildNode = _XMLNodeRoot->firstChild();

		//preCSeq = _sessIdMap.getSessionHandlerCSeqKey(this);

		//remove session from map
		_sessIdMap.removeSessionHandler(this);
		//_sessIdMap.removeSessionHandlerByCSeq(this);
		_sessId.clear();
		//bTimeout = false;
	}
	
	bXMLEnd = true;
	XMLLOG(ZQ::common::Log::L_INFO, CLOGFMT(SessionHandler, "Session[%s](CSeq:%d) hammer task over and removed"), _sessId.c_str(), _uLastCSeq);

	//_sessionWatchDog.watchSession(_sessId, DEFAULT_TIMEOUT);
	//unlock();
	//delete this;
	setExit(true);
}

void SessionHandler::OnResponse(const char* msg)
{
	{
		::ZQ::common::MutexGuard guard(_mutex);
		//lock();
		if (_cseqIdx > 0)
			_sessionWatchDog.removeSession(_cseqIdx);
		if (!_sessId.empty())
			_sessionWatchDog.removeSession(_sessId);

		//update sessId
		::std::string strMsg = msg;
		_xml_ResponseHandler.parseResponse(strMsg);
		_xml_ResponseHandler.updateSessCtxHandler(_xml_SessCtxHandler);

		if (strMsg.find("Method-Code: SETUP") != ::std::string::npos)
		{
			//_sessIdMap.removeSessionHandlerByCSeq(this);
			MapValue ret = _xml_SessCtxHandler.findSessCtxKey("SessId");
			XMLLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionHandler, "Set Session Id to %s"), _sessId.c_str());
			_sessId = ret.value;
			_sessIdMap.addSessionHandler(_sessId, this);
		}

		//find operation by seqId
		bWaitingForResponse =false;
		//unlock();
	}
	
	OnTimer();
	/*if (_cseqIdx > 0)
		_sessionWatchDog.watchSession(_cseqIdx, DEFAULT_TIMEOUT);
	if (!_sessId.empty())
		_sessionWatchDog.watchSession(_sessId, DEFAULT_TIMEOUT);*/
}

//impl of SessionMap
SessionMap::SessionMap(::ZQ::common::FileLog &fileLog, XML_RtspServerHandler &xml_RtspServerHandler)
:_log(&fileLog)
,_xml_RtspServerHandler(xml_RtspServerHandler)
{
	//InitializeCriticalSection(&_CS);
}

SessionMap::~SessionMap()
{
	//DeleteCriticalSection(&_CS);
}

SessionHandler* SessionMap::getSessionHandler(::std::string &sessId)
{
	printSessionNumber();
	if (sessId.empty())
		return NULL;
#ifdef USELOCK
	::ZQ::common::AutoReadLock rlock(_lock);
#endif
#ifdef USEMUTEX
	::ZQ::common::MutexGuard guard(_mutex);
#endif
	SessionHandlerMap::iterator iter = _sessionHandlerMap.find(sessId);
	if (iter != _sessionHandlerMap.end())
		return (*iter).second;
	else
		return NULL;
}

SessionHandler* SessionMap::getSessionHandler(uint16 uCSeq)
{
	printSessionNumber();
#ifdef USELOCK
	::ZQ::common::AutoReadLock rlock(_idmapLock);
#endif
#ifdef USEMUTEX
	::ZQ::common::MutexGuard guard(_idmapMutex);
#endif
	::std::map<uint16, SessionHandler*>::iterator iter = _seqIdMap.find(uCSeq);
	if (iter != _seqIdMap.end())
		return (*iter).second;
	else
		return NULL;
}

SessionHandler* SessionMap::getSessionHandler(SOCKET sock)
{
	printSessionNumber();
	
	if (_xml_RtspServerHandler._rtspServerNode.type.compare(SHARECONNECTION) == 0)
		return NULL;
	{
#ifdef USELOCK
		::ZQ::common::AutoReadLock rlock(_idmapLock);
#endif
#ifdef USEMUTEX
		::ZQ::common::MutexGuard guard(_idmapMutex);
#endif
		for (::std::map<uint16, SessionHandler*>::iterator iter = _seqIdMap.begin(); iter != _seqIdMap.end(); iter++)
		{
			if (((*iter).second)->_sessionSocket->m_Socket == sock)
				return (*iter).second;
		}
	}
#ifdef USELOCK
	::ZQ::common::AutoReadLock rlock(_lock);
#endif
#ifdef USEMUTEX
	::ZQ::common::MutexGuard guard(_mutex);
#endif
	for (SessionHandlerMap::iterator iter = _sessionHandlerMap.begin(); iter != _sessionHandlerMap.end(); iter++)
	{
		if (((*iter).second)->_sessionSocket->m_Socket == sock)
			return (*iter).second;
	}

	return NULL;
}

bool SessionMap::addSessionHandler(::std::string &sessId, SessionHandler *sessionHandler)
{
	printSessionNumber();
	if (sessionHandler == NULL)
		return false;

	if (sessId.empty())
		return false;

	//getLock();
#ifdef USELOCK
	::ZQ::common::AutoWriteLock wlock(_lock);
#endif
#ifdef USEMUTEX
	::ZQ::common::MutexGuard guard(_mutex);
#endif
	SessionHandlerMap::iterator iter = _sessionHandlerMap.find(sessId);
	if (iter != _sessionHandlerMap.end())
	{
		//already in session list
		XMLLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SessionMap, "Session[id:%s] already in list"), sessId.c_str());
		//releaseLock();
		return false;
	}
	else
	{
		XMLLOG(ZQ::common::Log::L_INFO, CLOGFMT(SessionMap, "add Session[id:%s]"), sessId.c_str());
		_sessionHandlerMap[sessId] = sessionHandler;
		//releaseLock();
		return true;
	}
}

bool SessionMap::addSessionHandler(uint16 uCSeq, SessionHandler *sessionHandler)
{
	printSessionNumber();
	if (sessionHandler == NULL)
		return false;

	//getLock();
#ifdef USELOCK
	::ZQ::common::AutoWriteLock wlock(_idmapLock);
#endif
#ifdef USEMUTEX
	::ZQ::common::MutexGuard guard(_idmapMutex);
#endif
	::std::map<uint16, SessionHandler*>::iterator iter = _seqIdMap.find(uCSeq);
	if (iter != _seqIdMap.end())
	{
		//already in session list
		XMLLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SessionMap, "Session of CSeq[%d] already in list"), uCSeq);
		//releaseLock();
		return false;
	}
	else
	{
		XMLLOG(ZQ::common::Log::L_INFO, CLOGFMT(SessionMap, "add Session of CSeq[%d]"), uCSeq);
		_seqIdMap[uCSeq] = sessionHandler;
		//releaseLock();
		return true;
	}
}

bool SessionMap::removeSessionHandler(SessionHandler *sessionHandler)
{
	printSessionNumber();
	//getLock();
#ifdef USELOCK
	::ZQ::common::AutoWriteLock wlock(_lock);
#endif
#ifdef USEMUTEX
	::ZQ::common::MutexGuard guard(_mutex);
#endif
	for (SessionHandlerMap::iterator iter = _sessionHandlerMap.begin(); iter != _sessionHandlerMap.end(); iter++)
	{
		//if ((*iter).second->_sessId.compare(sessionHandler->_sessId) == 0)
		if ((*iter).second == sessionHandler)
		{
			//remove session
			XMLLOG(ZQ::common::Log::L_INFO, CLOGFMT(SessionMap, "remove Session[id:%s]"), sessionHandler->_sessId.c_str());
			//delete (*iter).second;
			_sessionHandlerMap.erase(iter);
			//releaseLock();
			return true;
		}
	}

	XMLLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SessionMap, "Session[id:%s] not found"), sessionHandler->_sessId.c_str());
	//log error
	//releaseLock();
	return false;
}

bool SessionMap::removeSessionHandlerByCSeq(SessionHandler *sessionHandler)
{
	printSessionNumber();
	//getLock();
#ifdef USELOCK
	::ZQ::common::AutoWriteLock wlock(_idmapLock);
#endif
#ifdef USEMUTEX
	::ZQ::common::MutexGuard guard(_idmapMutex);
#endif
	for (::std::map<uint16, SessionHandler*>::iterator iter = _seqIdMap.begin(); iter != _seqIdMap.end(); iter++)
	{
		//if ((*iter).second->_sessId.compare(sessionHandler->_sessId) == 0)
		if ((*iter).second == sessionHandler)
		{
			//remove session
			XMLLOG(ZQ::common::Log::L_INFO, CLOGFMT(SessionMap, "remove Session of CSeq[%d]"), (*iter).first);
			//delete (*iter).second;
			_seqIdMap.erase(iter);
			//releaseLock();
			return true;
		}
	}

	XMLLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SessionMap, "error remove Session of CSeq[%d]"), sessionHandler->_uLastCSeq);
	//log error
	//releaseLock();
	return false;
}

bool SessionMap::removeSessionHandlerByCSeq(uint16 CSeq)
{
	printSessionNumber();
	//getLock();
#ifdef USELOCK
	::ZQ::common::AutoWriteLock wlock(_idmapLock);
#endif
#ifdef USEMUTEX
	::ZQ::common::MutexGuard guard(_idmapMutex);
#endif
	::std::map<uint16, SessionHandler*>::iterator iter = _seqIdMap.find(CSeq); 
	if (iter != _seqIdMap.end())
	{
		//remove session
		XMLLOG(ZQ::common::Log::L_INFO, CLOGFMT(SessionMap, "[removeSessonHanlderByCSeq]remove Session of CSeq[%d]"), CSeq);

		_seqIdMap.erase(iter);
		//releaseLock();
		return true;
	}

	XMLLOG(ZQ::common::Log::L_WARNING, CLOGFMT(SessionMap, "[removeSessonHanlderByCSeq]error remove Session of CSeq[%d]"), CSeq);
	//log error
	//releaseLock();
	return false;
}

uint16 SessionMap::getSessionHandlerCSeqKey(SessionHandler *sessionHandler)
{
	printSessionNumber();

	if (sessionHandler->_sessId.empty())
		return 0;

	//getLock();
#ifdef USELOCK
	::ZQ::common::AutoReadLock rlock(_idmapLock);
#endif
#ifdef USEMUTEX
	::ZQ::common::MutexGuard guard(_idmapMutex);
#endif
	for (::std::map<uint16, SessionHandler*>::iterator iter = _seqIdMap.begin(); iter != _seqIdMap.end(); iter++)
	{
		if ((*iter).second->_sessId.compare(sessionHandler->_sessId) == 0)
		{
			uint16 uCSeq = (*iter).first;
			//releaseLock();
			return uCSeq;
		}
	}

	//releaseLock();
	return 0;
}