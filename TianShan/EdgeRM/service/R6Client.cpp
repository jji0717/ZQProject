#include "EdgeRMEnv.h"
#include "strHelper.h"

using namespace ZQ::common;

namespace ZQTianShan {
	namespace EdgeRM {
// -----------------------------
// class R6Client
// -----------------------------
R6Client::R6Client(R6SessionGroup& sessGroup, Log& log, NativeThreadPool& thrdpool, InetHostAddress& bindAddress, const std::string& baseURL, const char* userAgent, Log::loglevel_t verbosityLevel, tpport_t bindPort)
:RTSPClient(log, thrdpool, bindAddress, baseURL, userAgent, verbosityLevel, bindPort), _sessGroup(sessGroup)
{
	int poolSize, activeCount, pendingSize;
	TCPSocket::getThreadPoolStatus(poolSize, activeCount, pendingSize);
	//	TCPSocket::getPendingSize(poolSize);
	TCPSocket::resizeThreadPool(poolSize+1);
}

R6Client::~R6Client(void)
{

}

void R6Client::OnServerRequest(RTSPClient& rtspClient, const char* cmdName, const char* url, RTSPMessage::Ptr& pInMessage)
{
	_log(Log::L_INFO, CLOGFMT(R6Client, "OnServerRequest() conn[%s] received peer request %s(%d)"), connDescription(), pInMessage->startLine.c_str(), pInMessage->cSeq);
	do
	{
		//check if it is session in progress
		if (0 == strcmp(cmdName, "ANNOUNCE"))
		{
			std::string notice = "";
			if (pInMessage->headers.find("Notice") == pInMessage->headers.end())
				break; // ignore those ANNOUNCE with no Notice

			notice = pInMessage->headers["Notice"];
			std::vector<std::string> temp;
			ZQ::common::stringHelper::SplitString(notice, temp, " ", " ");
			if (temp.empty())
				break; // ignore those ANNOUNCE with illegal Notice

			int AnnounceCode = atoi(temp[0].c_str());

			switch (AnnounceCode)
			{
			case RTSPSink::racSessionInProgress:
				{
					std::string cseq;
					if (pInMessage->headers.find("CSeq") != pInMessage->headers.end())
					{
						cseq = pInMessage->headers["CSeq"];
					}
					RTSPMessage::Ptr pMessage = new RTSPMessage(atoi(cseq.c_str()));
					pMessage->startLine = "RTSP/1.0 404 Not Found";
					RTSPMessage::AttrMap     headers;
					if (pInMessage->headers.find("Session") != pInMessage->headers.end())
						headers.insert(std::make_pair("Session", pInMessage->headers["Session"]));

					sendMessage(pMessage, headers);
				}
				break;

			case RTSPSink::racClientSessionTerminated:
				break;

			default:
				break;
			}
		}
	} while(0);
}
void R6Client::OnRequestError(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RequestError errCode, const char* errDesc)
{
	if (!pReq)
		return;

	if (NULL == errDesc || strlen(errDesc) <=0)
		errDesc = requestErrToStr(errCode);

	char buf[64];
	_log(Log::L_ERROR, CLOGFMT(R6Client, "OnRequestError() conn[%s] %s(%d): %d %s; %s +%d"),
		connDescription(), pReq->_commandName.c_str(), pReq->cSeq, errCode, errDesc, TimeUtil::TimeToUTC(pReq->stampCreated, buf, sizeof(buf)-2, true), _timeout);

}
void R6Client::OnConnected()
{
	RTSPClient::OnConnected();

	if (Socket::stConnected != TCPSocket::state())
		return; // the current connecting has problem, skip the handling

	if (now() - _sessGroup.getLastSync() > 60000)
	{	
		if(_sessGroup.sync(true) > 0)
			_sessGroup.setLastSync(now(), R6SessionGroup::Sync);
	}

	RTSPRequest::AttrMap paramMap;
	MAPSET(RTSPMessage::AttrMap, paramMap, "clab-SessionGroup", _sessGroup.getName());

	RTSPMessage::AttrMap headers;
	MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_CONTENTTYPE,      "text/parameters");
	MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_REQUIRE,			ERMI_HEADER_REQUIRE_VAL);
	MAPSET(RTSPMessage::AttrMap, headers, "SessionGroup",			_sessGroup.getName());

	sendSET_PARAMETER(paramMap, NULL, headers);
}
void R6Client::OnError()
{
	RTSPClient::OnError();
	_sessGroup.setStatus(R6SessionGroup::Idle);
}
int R6Client::sendTousyMsg(const std::string& startLine, RTSPMessage::AttrMap& headers, const std::string& body, const int cseq)
{
	RTSPMessage::Ptr pMessage = new RTSPMessage((cseq>0) ? lastCSeq() : cseq);
	pMessage->startLine = startLine;
	return sendMessage(pMessage, headers);
}
void R6Client::OnResponse(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	if (!pReq)
		return;
	
	_log(Log::L_INFO, CLOGFMT(R6Client, "OnResponse() sessgroup[%s]-conn[%s] %s(%d) received response: %d %s"), _sessGroup.getName().c_str(), connDescription(), pReq->_commandName.c_str(), pReq->cSeq, resultCode, resultString);

	do
	{
		// wildcast to check "551 Option Not Supported" about ECR: Protocol Versioning for RTSP protocols
		if (resultCode == RTSPSink::rcOptionNotSupport)
		{
			if (pResp->headers.end() != pResp->headers.find("Unsupported"))
			{
				std::string& valOfResponse = pResp->headers["Unsupported"];
			}
		}

		if (pReq->_commandName == "GET_PARAMETER")
		{
			if (resultCode != RTSPSink::rcOK) 
				break; // ingore failed GET_PARAMETER

			// GET_PARAMETER ok
			std::string groupname = _sessGroup.getName();
			if (pResp->headers.end() != pResp->headers.find("SessionGroup"))
			{
				std::string& gnOfResponse = pResp->headers["SessionGroup"];
				if (0 != groupname.compare(gnOfResponse))
				{
					_log(Log::L_WARNING, CLOGFMT(R6Client, "OnResponse() sessgroup[%s]-conn[%s] %s(%d) unmatched SessionGroup[%s] in response, ignore"),
						groupname.c_str(), connDescription(), pReq->_commandName.c_str(), pReq->cSeq, gnOfResponse.c_str());
					break;
				}
			}

			std::vector<R6Client::Session_Pair> list;
			size_t index = 0;
			std::string key("session_list:");
			index = pResp->contentBody.find(key);
			if (std::string::npos == index)
			{
				_log(Log::L_DEBUG, CLOGFMT(R6Client, "OnResponse() conn[%s] %s(%d) SessionGroup[%s] no session_list found in response, ignore"), 
					connDescription(), pReq->_commandName.c_str(), pReq->cSeq, groupname.c_str());
				break;
			}

			index += key.length();
			// chop string to get session_list: line
			std::string line = pResp->contentBody.substr(index, pResp->contentBody.size());
			_log(Log::L_DEBUG, CLOGFMT(R6Client, "OnResponse() SessionGroup[%s] session_list[%s]"), groupname.c_str(), line.c_str());
			std::vector<std::string> tmpVec;	
			stringHelper::SplitString(line, tmpVec, " \t", " \t\r\n","","");
			for (size_t i = 0; i < tmpVec.size(); i++)
			{
				R6Client::Session_Pair sessPair;
				index = tmpVec[i].find(":");
				if(index != std::string::npos)
				{
					sessPair._sessionId = tmpVec[i].substr(0, index);
					sessPair._onClientSessionId = tmpVec[i].substr(index+1, tmpVec[i].size());						
				}
				else sessPair._sessionId = tmpVec[i];

				list.push_back(sessPair);
			}

			_sessGroup.OnSessionListOfSS(list);
			break;
		} // if GET_PARAMETER

		// response of set up request,send startChecking
		/*
		_log(Log::L_DEBUG, CLOGFMT(R6ClientTest, "commandName:%s,lastRequest:%s,_bProPortDone:%d"), pReq->_commandName,_lastRequest,_bProPortDone);
		if(pReq->_commandName == "SETUP" && _lastRequest.compare(LASTREQUEST_PROPORT) == 0 && _bProPortDone)
		{
			// send startChecking request
			std::string sessionId;
			RTSPMessage::AttrMap::const_iterator itorHeaders;
			itorHeaders = pResp->headers.find(R6_HEADER_SESSION);
			if(itorHeaders == pResp->headers.end())
			{
				//_log(ZQ::common::Log::L_ERROR, CLOGFMT(R6Session, "OnResponse_SETUP() CSeq[%d] missed 'Session' header"), respCSeq);

				return;
			}
			sessionId = itorHeaders->second;
			std::string serverIP(_serverAddress.getHostAddress());
			R6StartCheckCmd* pR6SessStart = new R6StartCheckCmd(_sessGroup._env,pReq->_sessGuid,sessionId,serverIP,_serverPort);
			if(pR6SessStart)
				pR6SessStart->execute();
		}

		// response of set up startChecking request
		if(pReq->_commandName == "SETUP" && _lastRequest.compare(LASTREQUEST_STOPCHECK))
		{
			//r6tearDown
			std::string sessionId;
			RTSPMessage::AttrMap::const_iterator itorHeaders;
			itorHeaders = pResp->headers.find(R6_HEADER_SESSION);
			if(itorHeaders == pResp->headers.end())
				return;
			sessionId = itorHeaders->second;
			R6SessTearDownCmd* pR6SessTearDown = new R6SessTearDownCmd(_sessGroup._env,pReq->_sessGuid,_sessGroup.getName(),sessionId,R6_RESPONSE_200);
			if(pR6SessTearDown)
				pR6SessTearDown->execute();
		}
		*/

	} while(0);
	
}

// -----------------------------
// class R6Session
// -----------------------------
R6Session::R6Session(R6SessionGroup& sessGroup, std::string ODSessId, size_t idxClient)
:_sessGroup(sessGroup),_idxClient(idxClient), _lastRequest(""),_bProPortDone(false),
RTSPSession(sessGroup._env.getLogger(), sessGroup._env.getClientThreadPool(), "", NULL, sessGroup._env.getRtspLogLevel(), sessGroup._env._r6SessTimeout, ODSessId.c_str())

{
	_sessGroup.add(*this);
	_log(Log::L_DEBUG, CLOGFMT(Session, "R6Sess[%s] instantized in SessionGroup[%s]"), _sessGuid.c_str(), getSessionGroupName().c_str());
}

R6Session::~R6Session()
{
	destroy();
}

std::string	R6Session::getSessionGroupName() const
{ 
	return _sessGroup.getName();
}

void R6Session::destroy()
{
	_sessGroup.remove(*this);
	RTSPSession::destroy();
}

std::string R6Session::getBaseURL()
{
	return _sessGroup.getBaseURL();
}
R6Client*		R6Session::getR6Client()
{
	return _sessGroup.getR6Client(_idxClient);
}
void R6Session::setSessionId(std::string sessionId) 
{
	_sessionId = sessionId;
	_sessGroup.updateIndex(*this);
}
void R6Session::OnResponse_SETUP(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	RTSPSession::OnResponse_SETUP(rtspClient, pReq, pResp, resultCode, resultString);
	_log(Log::L_DEBUG, CLOGFMT(R6Session, "Session[%s, %s, %d, %s] OnResponse_SETUP(), return [%d,%s]"), _sessGuid.c_str(), _sessionId.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);
	_resultCode = resultCode;
	_stampSetup = now();

	uint32 respCSeq = pResp->cSeq;

	RTSPMessage::AttrMap::const_iterator itorHeaders;
	itorHeaders = pResp->headers.find(R6_HEADER_SESSION);
	if(itorHeaders == pResp->headers.end())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(R6Session, "OnResponse_SETUP() CSeq[%d] missed 'Session' header"), respCSeq);
		destroy();
		return;
	}

	_sessionId = itorHeaders->second;

	if (resultCode == RTSPSink::rcOK)
	{
		_sessGroup.updateIndex(*this);
		R6Client* r6Client = getR6Client();
		if(_lastRequest.compare(LASTREQUEST_PROPORT) == 0 && _bProPortDone)
		{
			// send startChecking request
			std::string sessionId;
			RTSPMessage::AttrMap::const_iterator itorHeaders;
			itorHeaders = pResp->headers.find(R6_HEADER_SESSION);
			if(itorHeaders == pResp->headers.end())
			{
				//_log(ZQ::common::Log::L_ERROR, CLOGFMT(R6Session, "OnResponse_SETUP() CSeq[%d] missed 'Session' header"), respCSeq);

				return;
			}
			sessionId = itorHeaders->second;
			std::string serverIP(r6Client->_serverAddress.getHostAddress());
			R6StartCheckCmd* pR6SessStart = new R6StartCheckCmd(_sessGroup._env,pReq->_sessGuid,sessionId,serverIP,r6Client->_serverPort);
			if(pR6SessStart)
				pR6SessStart->execute();
			_bProPortDone = false;
		}

		// response of set up startChecking request
		if(_lastRequest.compare(LASTREQUEST_STOPCHECK))
		{
			//r6tearDown
			std::string sessionId;
			RTSPMessage::AttrMap::const_iterator itorHeaders;
			itorHeaders = pResp->headers.find(R6_HEADER_SESSION);
			if(itorHeaders == pResp->headers.end())
				return;
			sessionId = itorHeaders->second;
			R6SessTearDownCmd* pR6SessTearDown = new R6SessTearDownCmd(_sessGroup._env,pReq->_sessGuid,_sessGroup.getName(),sessionId,R6_RESPONSE_200);
			if(pR6SessTearDown)
				pR6SessTearDown->execute();
			_lastRequest = LASTREQUEST_NULL;
		}
	}
	_sessGroup._env.updataAllocation(_sessGuid, _sessGroup.getName(), _sessionId);
}
void R6Session::OnResponse_TEARDOWN(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	_log(Log::L_DEBUG, CLOGFMT(R6Session, "Session[%s, %d, %s] OnResponse_TEARDOWN(), return [%d,%s]"), _sessGuid.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);
	_resultCode = resultCode;
/*	if (resultCode == RTSPSink::rcSessNotFound)
		destroy();*/
	destroy();
}
void R6Session::OnResponse_SET_PARAMETER(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	_log(Log::L_DEBUG, CLOGFMT(R6Session, "Session[%s, %d, %s] OnResponse_SET_PARAMETER(), return [%d,%s]"), _sessGuid.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);
	_resultCode = resultCode;
	if (resultCode == RTSPSink::rcSessNotFound)
		destroy();
}
void R6Session::OnResponse_GET_PARAMETER(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	_log(Log::L_DEBUG, CLOGFMT(R6Session, "Session[%s, %d, %s] OnResponse_GET_PARAMETER(), return [%d,%s]"), _sessGuid.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);
	_resultCode = resultCode;
	
	if(resultCode == RTSPSink::rcSessNotFound)
	{
		destroy();
	}
}
void R6Session::OnSessionTimer()
{
	R6Client* client = _sessGroup.getR6Client(); 
	if (client)
	{
		if(_sessionId.empty())
		{
			destroy();
			return;
		}

		RTSPRequest::AttrList parameterNames;
		RTSPMessage::AttrMap headers;
		MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_SESSION, _sessionId.c_str());
		MAPSET(RTSPMessage::AttrMap, headers,ERMI_HEADER_REQUIRE, ERMI_HEADER_REQUIRE_VAL);
		client->sendGET_PARAMETER(*this, parameterNames, NULL,    headers);
		_log(Log::L_DEBUG, CLOGFMT(R6Session, "Session[%s] OnDemandSession[%s]OnSessionTimer(), trigger PING"), _sessionId.c_str(), _sessGuid.c_str());
	}
}

void R6Session::OnANNOUNCE(RTSPClient& rtspClient, RTSPMessage::Ptr& pInMessage)
{
	_log(Log::L_DEBUG, CLOGFMT(R6Session, "Session[%s, %s] OnANNOUNCE()"), _sessGuid.c_str(), getSessionGroupName().c_str());
	if (pInMessage->headers.end() == pInMessage->headers.find("Session"))
		return;

}
void R6Session::OnResponse(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	// wildcast to check "551 Option Not Supported" about ECR: Protocol Versioning for RTSP protocols
	R6Client* pClient = (R6Client*) &rtspClient;

	return RTSPSession::OnResponse(rtspClient, pReq, pResp, resultCode, resultString); // do the dispatching
}
void R6Session::OnRequestError(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RequestError errCode, const char* errDesc)
{
	RTSPSession::OnRequestError(rtspClient, pReq, errCode, errDesc);

	if (!pReq)
		return;

	switch(errCode)
	{
	case Err_InvalidParams:
		_resultCode = rcBadParameter;
		break;

	case Err_RequestTimeout:
		_resultCode = rcRequestTimeout;
		break;

	case Err_ConnectionLost:
	case Err_SendFailed:
	default:
		_resultCode = rcServiceUnavail;
	}
}

// -----------------------------
// class R6SessionGroup
// -----------------------------
R6SessionGroup::SessionGroupMap R6SessionGroup::_groupMap;
R6SessionGroup::StringIndex     R6SessionGroup::_idxGroupBaseUrl;
Mutex R6SessionGroup::_lockGroups;

#define _log        (_env.getLogger())

R6SessionGroup::R6SessionGroup(EdgeRMEnv& env, const std::string& qamName, const std::string& baseURL, int max, Ice::Long syncInterval)
: _env(env), _qamName(qamName), _baseURL(baseURL), _max(max), _syncInterval(syncInterval),
_R6Client(*this, _log, _env.getThreadPool(), _env._ermiBindAddr, _baseURL, _env._userAgent.c_str(), _env.getRtspLogLevel())
{
	glog(Log::L_DEBUG, CLOGFMT(R6SessionGroup, "SessionGroup[%s] adding connection to baseUrl[%s]]"), _qamName.c_str(), _baseURL.c_str());
	
	_idxGroupBaseUrl.insert(StringIndex::value_type(_baseURL, _qamName));

}
R6SessionGroup::~R6SessionGroup()
{
	{
		MutexGuard g(_lockSessMap);
		_sessMap.clear();
	}

	clearR6Client();
}

void R6SessionGroup::add(R6Session& sess)
{
	MutexGuard g(_lockSessMap);
	MAPSET(SessionMap, _sessMap, sess._sessGuid, &sess);
}

void R6SessionGroup::remove(R6Session& sess)
{
	MutexGuard g(_lockSessMap);
	_sessMap.erase(sess._sessGuid);
}
void R6SessionGroup::updateIndex(R6Session& sess, const char* indexName, const char* oldValue)
{
	std::string oldSessId = oldValue? oldValue:sess._sessionId;

	if (!oldSessId.empty())
		eraseSessionId(oldSessId, sess);

	MutexGuard g(_lockSessMap);
	_sessIdIndex.insert(StringIndex::value_type(sess._sessionId, sess._sessGuid));
}

R6Session::List R6SessionGroup::lookupByIndex(const char* sessionId, const char* indexName)
{
	R6Session::List list;

	if (NULL == sessionId || (NULL != indexName && 0 !=stricmp(indexName, "SessionId")))
		return list; // unsupported input parameters

	MutexGuard g(_lockSessMap);
	StringIdxRange range = _sessIdIndex.equal_range(sessionId);

	for (StringIndex::iterator it = range.first; it != range.second; ++it)
	{
		SessionMap::iterator itS = _sessMap.find(it->second);
		if (_sessMap.end() ==itS)
			continue;

		list.push_back(itS->second);
	}

	return list;
}
R6Session::Ptr R6SessionGroup::lookupByOnDemandSessionId(const char* sessOnDemandId)
{
	if (NULL == sessOnDemandId)
		return NULL;

	MutexGuard g(_lockSessMap);
	SessionMap::iterator it = _sessMap.find(sessOnDemandId);
	if (_sessMap.end() ==it)
		return NULL;

	return it->second;
}
void R6SessionGroup::eraseSessionId(const std::string& sessionId, R6Session& sess)
{
	MutexGuard g(_lockSessMap);
	bool bFound =true;
	while (bFound)
	{
		StringIdxRange range = _sessIdIndex.equal_range(sessionId);
		bFound = false;

		for (StringIndex::iterator it = range.first; it != range.second; ++it)
		{
			if (0 != it->second.compare(sess._sessGuid))
				continue;

			bFound = true;
			_sessIdIndex.erase(it);
			break;
		}
	}
}
R6Client* R6SessionGroup::getR6Client(size_t idxClient)
{
	return &_R6Client;
}

R6Session::Ptr R6SessionGroup::createSession(const std::string& ODSessId, const char* qamName)
{
	R6SessionGroup::Ptr pSessGroup = NULL;

	if (ODSessId.empty())
		return NULL;

	std::string strQamName(qamName);
	pSessGroup = findSessionGroup(strQamName);

	if (NULL == pSessGroup)
	{
		glog(Log::L_WARNING, CLOGFMT(R6SessionGroup, "createSession() session[%s]: none of valid SessionGroup(s) has quota available"), ODSessId.c_str());
		return NULL;
	}

	return (new R6Session(*pSessGroup, ODSessId));
}

R6Session::Ptr R6SessionGroup::openSession(const std::string& ODSessId, const std::string& groupName, bool bRestore)
{
	R6SessionGroup::Ptr pSessGroup = R6SessionGroup::findSessionGroup(groupName);
	if (NULL == pSessGroup)
		return NULL;

	R6Session::Ptr clientSession = pSessGroup->lookupByOnDemandSessionId(ODSessId.c_str());

	if (NULL != clientSession)
		return clientSession;

	if (!bRestore)
		return NULL;

	return (new R6Session(*pSessGroup, ODSessId));
}


R6SessionGroup::Ptr R6SessionGroup::findSessionGroup(const std::string& name)
{
	R6SessionGroup::Ptr group = NULL;

	MutexGuard g(_lockGroups);
	SessionGroupMap::iterator it = _groupMap.find(name);
	if (_groupMap.end() !=it)
		group = it->second;

	return group;
}

std::vector<std::string> R6SessionGroup::getAllSessionGroup()
{
	MutexGuard g(_lockGroups);
	std::vector<std::string> result;

	for (SessionGroupMap::iterator it = _groupMap.begin(); it != _groupMap.end(); it++)
		result.push_back(it->first);

	return result;
}

void R6SessionGroup::clearSessionGroup()
{
	MutexGuard g(_lockGroups);

	_groupMap.clear();
}

void R6SessionGroup::clearR6Client()
{
/*	MutexGuard g(_lockClients);
	ERMICLients::iterator iter = _R6Clients.begin();
	for (; iter != _R6Clients.end(); iter++)
	{
		try
		{
			R6Client* pClient = *iter;
			if (pClient)
				delete pClient;
		}
		catch(...)
		{
		}
	}

	_R6Clients.clear();*/
}


int R6SessionGroup::sync(bool bOnConnected)
{
	_log(Log::L_DEBUG, CLOGFMT(R6SessionGroup, "SessionGroup[%s] sync() per %s"), _qamName.c_str(), bOnConnected ? "connected" :"timer");
	if (!bOnConnected && Idle != _status)
	{
		_log(Log::L_DEBUG, CLOGFMT(R6SessionGroup, "SessionGroup[%s] is already in sync"), _qamName.c_str());
		return 0;
	}

	if (bOnConnected && Socket::stConnected != _R6Client.state())
		return -1;

	//set group status
	RTSPRequest::AttrList parameterNames;
	parameterNames.push_back("session-list");
	//	parameterNames.push_back("connection_timeout");
	RTSPMessage::AttrMap headers;
	headers.insert(std::make_pair(R6_HEADER_CONTENTTYPE, "text/Parameters"));
	headers.insert(std::make_pair(R6_HEADER_REQUIRE,      R6_HEADER_REQUIRE_VAL));

	return _R6Client.sendGET_PARAMETER(parameterNames, NULL, headers);
}

void R6SessionGroup::OnSessionListOfSS(const std::vector<R6Client::Session_Pair>& listOnSS)
{
	_log(Log::L_DEBUG, CLOGFMT(R6SessionGroup, "OnSessionListOfSS() SessionGroup[%s] %d sessions on SS"), _qamName.c_str(), listOnSS.size());

	int64 stampStart= ZQ::common::now();
	size_t cSSOrphan =0, cSynced=0, cTorndown=0;

	try {

		{
			MutexGuard g(_lockSessMap);
			for (std::vector<R6Client::Session_Pair>::const_iterator iter = listOnSS.begin();	iter != listOnSS.end(); iter++)
			{
				std::string tmpStr = iter->_onClientSessionId + ":" + iter->_sessionId;
				bool bFound = false;

				// find session by key
				if (!iter->_onClientSessionId.empty() && _sessMap.end() != _sessMap.find(iter->_onClientSessionId))
					bFound = true;
				else if (!iter->_sessionId.empty() && _sessIdIndex.end() != _sessIdIndex.find(iter->_sessionId))
					bFound = true;
				else if(_env.syncR6Session(iter->_onClientSessionId, _qamName, iter->_sessionId))
					bFound = true;

				if (bFound)
				{
					cSynced ++;
					//					_log(Log::L_DEBUG, CLOGFMT(R6SessionGroup, "OnSessionListOfSS() SessionGroup[%s] session[%s] sync-ed"), _name.c_str(), tmpStr.c_str());
					continue;
				}

				cSSOrphan ++;
				_log(Log::L_INFO, CLOGFMT(R6SessionGroup, "OnSessionListOfSS() SessionGroup[%s] SS-orphan[%s] found, tearing it down"), _qamName.c_str(), tmpStr.c_str());

				//TODO: remove session from media server by TEARDOWN
				RTSPMessage::AttrMap headers;
				MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_REQUIRE, ERMI_HEADER_REQUIRE_VAL);
				MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_SESSION, iter->_sessionId);
				MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_CLABCLIENTSESSIONID, iter->_onClientSessionId);
				MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_CLABREASON, "428 Session Orphan Detected on PHO");

				_R6Client.sendTousyMsg("TEARDOWN * RTSP/1.0", headers);

				cTorndown++;
			}
		}
	}
	catch(...)
	{
	}

	setLastSync(now(), Idle);
	_log(Log::L_INFO, CLOGFMT(R6SessionGroup, "syncSessionList() SessionGroup[%s] done, took %lldmsec: %d sync-ed, %d SS-orphans found, %d torndown"), _qamName.c_str(), ZQ::common::now() -stampStart, cSynced, cSSOrphan, cTorndown);
}

void R6SessionGroup::OnTimer(const ::Ice::Current& c)
{
	int64 stampNow = now();
	if (getLastSync() + getSyncInterval() > stampNow) // do not need sync now
		return;

	sync();
	setLastSync(stampNow, Sync);
}
Ice::Long R6SessionGroup::checkAll()
{
	std::vector<R6SessionGroup::Ptr> groupsToOntimer;
	::Ice::Long stampNow = now();
	::Ice::Long _nextWakeup = stampNow + 1000;

	{
		MutexGuard g(_lockGroups);

		for (SessionGroupMap::iterator iter = _groupMap.begin(); iter != _groupMap.end(); iter++)
		{
			try
			{
				R6SessionGroup::Ptr group = iter->second;
				if (NULL == group)
					continue;

				if (stampNow - group->getLastSync() > group->getSyncInterval())
					groupsToOntimer.push_back(group);
				else
					_nextWakeup = (_nextWakeup > group->getSyncInterval()) ? group->getSyncInterval() : _nextWakeup;
			}
			catch(...)	{}
		}
	}

	for (std::vector<R6SessionGroup::Ptr>::iterator iter2 = groupsToOntimer.begin(); iter2 != groupsToOntimer.end(); iter2++)
	{
		try
		{
			(*iter2)->OnTimer();
		}
		catch(...)	{}
	}

	groupsToOntimer.clear();
	return _nextWakeup;
}
}}//end namespace
