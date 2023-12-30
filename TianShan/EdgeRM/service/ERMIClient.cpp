#include "EdgeRMEnv.h"
//#include "ERMIClient.h"
#include "strHelper.h"

using namespace ZQ::common;

namespace ZQTianShan {
	namespace EdgeRM {
// -----------------------------
// class ERMIClient
// -----------------------------
ERMIClient::ERMIClient(ERMISessionGroup& sessGroup, Log& log, NativeThreadPool& thrdpool, InetHostAddress& bindAddress, const std::string& baseURL, const char* userAgent, Log::loglevel_t verbosityLevel, tpport_t bindPort)
:RTSPClient(log, thrdpool, bindAddress, baseURL, userAgent, verbosityLevel, bindPort), _sessGroup(sessGroup)
{
	int poolSize, activeCount, pendingSize;
	TCPSocket::getThreadPoolStatus(poolSize, activeCount, pendingSize);
	//	TCPSocket::getPendingSize(poolSize);
	TCPSocket::resizeThreadPool(poolSize+1);
}

ERMIClient::~ERMIClient(void)
{

}

void ERMIClient::OnServerRequest(RTSPClient& rtspClient, const char* cmdName, const char* url, RTSPMessage::Ptr& pInMessage)
{
	_log(Log::L_INFO, CLOGFMT(ERMIClient, "OnServerRequest() conn[%s] received peer request %s(%d)"), connDescription(), pInMessage->startLine.c_str(), pInMessage->cSeq);
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
void ERMIClient::OnRequestError(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RequestError errCode, const char* errDesc)
{
	if (!pReq)
		return;

	if (NULL == errDesc || strlen(errDesc) <=0)
		errDesc = requestErrToStr(errCode);

	char buf[64];
	_log(Log::L_ERROR, CLOGFMT(ERMIClient, "OnRequestError() conn[%s] %s(%d): %d %s; %s +%d"),
		connDescription(), pReq->_commandName.c_str(), pReq->cSeq, errCode, errDesc, TimeUtil::TimeToUTC(pReq->stampCreated, buf, sizeof(buf)-2, true), _timeout);

}
void ERMIClient::OnConnected()
{
	RTSPClient::OnConnected();

	if (Socket::stConnected != TCPSocket::state())
		return; // the current connecting has problem, skip the handling

	if (now() - _sessGroup.getLastSync() > 60000)
	{	
		if(_sessGroup.sync(true) > 0)
			_sessGroup.setLastSync(now(), ERMISessionGroup::Sync);
	}

	RTSPRequest::AttrMap paramMap;
	MAPSET(RTSPMessage::AttrMap, paramMap, "clab-SessionGroup", _sessGroup.getName());

	RTSPMessage::AttrMap headers;
	MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_CONTENTTYPE,      "text/parameters");
	MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_REQUIRE,			ERMI_HEADER_REQUIRE_VAL);
	MAPSET(RTSPMessage::AttrMap, headers, "SessionGroup",			_sessGroup.getName());

	sendSET_PARAMETER(paramMap, NULL, headers);
}
void ERMIClient::OnError()
{
	RTSPClient::OnError();
	_sessGroup.setStatus(ERMISessionGroup::Idle);
}
int ERMIClient::sendTousyMsg(const std::string& startLine, RTSPMessage::AttrMap& headers, const std::string& body, const int cseq)
{
	RTSPMessage::Ptr pMessage = new RTSPMessage((cseq>0) ? lastCSeq() : cseq);
	pMessage->startLine = startLine;
	return sendMessage(pMessage, headers);
}
void ERMIClient::OnResponse(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	if (!pReq)
		return;

	_log(Log::L_INFO, CLOGFMT(ERMIClient, "OnResponse() sessgroup[%s]-conn[%s] %s(%d) received response: %d %s"), _sessGroup.getName().c_str(), connDescription(), pReq->_commandName.c_str(), pReq->cSeq, resultCode, resultString);

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
					_log(Log::L_WARNING, CLOGFMT(ERMIClient, "OnResponse() sessgroup[%s]-conn[%s] %s(%d) unmatched SessionGroup[%s] in response, ignore"),
						groupname.c_str(), connDescription(), pReq->_commandName.c_str(), pReq->cSeq, gnOfResponse.c_str());
					break;
				}
			}

			std::vector<ERMIClient::Session_Pair> list;
			size_t index = 0;
			std::string key("session_list:");
			index = pResp->contentBody.find(key);
			if (std::string::npos == index)
			{
				_log(Log::L_DEBUG, CLOGFMT(ERMIClient, "OnResponse() conn[%s] %s(%d) SessionGroup[%s] no session_list found in response, ignore"), 
					connDescription(), pReq->_commandName.c_str(), pReq->cSeq, groupname.c_str());
				break;
			}

			index += key.length();
			// chop string to get session_list: line
			std::string line = pResp->contentBody.substr(index, pResp->contentBody.size());
			_log(Log::L_DEBUG, CLOGFMT(ERMIClient, "OnResponse() SessionGroup[%s] session_list[%s]"), groupname.c_str(), line.c_str());
			std::vector<std::string> tmpVec;	
			stringHelper::SplitString(line, tmpVec, " \t", " \t\r\n","","");
			for (size_t i = 0; i < tmpVec.size(); i++)
			{
				ERMIClient::Session_Pair sessPair;
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

	} while(0);
}

// -----------------------------
// class ERMISession
// -----------------------------
ERMISession::ERMISession(ERMISessionGroup& sessGroup, std::string ODSessId, size_t idxClient)
:_sessGroup(sessGroup),_idxClient(idxClient),
RTSPSession(sessGroup._env.getLogger(), sessGroup._env.getClientThreadPool(), "", NULL, sessGroup._env.getRtspLogLevel(), sessGroup._env._ermiSessTimeout, ODSessId.c_str())

{
	_sessGroup.add(*this);
	_log(Log::L_DEBUG, CLOGFMT(Session, "ERMISess[%s] instantized in SessionGroup[%s]"), _sessGuid.c_str(), getSessionGroupName().c_str());
}

ERMISession::~ERMISession()
{
	destroy();
}

std::string	ERMISession::getSessionGroupName() const
{ 
	return _sessGroup.getName();
}

void ERMISession::destroy()
{
	_sessGroup.remove(*this);
	RTSPSession::destroy();
}

std::string ERMISession::getBaseURL()
{
	return _sessGroup.getBaseURL();
}
ERMIClient*		ERMISession::getERMIClient()
{
	return _sessGroup.getERMIClient(_idxClient);
}
void ERMISession::setSessionId(std::string sessionId) 
{
	_sessionId = sessionId;
	_sessGroup.updateIndex(*this);
}
void ERMISession::OnResponse_SETUP(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	RTSPSession::OnResponse_SETUP(rtspClient, pReq, pResp, resultCode, resultString);
	_log(Log::L_DEBUG, CLOGFMT(ERMISession, "Session[%s, %s, %d, %s] OnResponse_SETUP(), return [%d,%s]"), _sessGuid.c_str(), _sessionId.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);
	_resultCode = resultCode;
	_stampSetup = now();

	uint32 respCSeq = pResp->cSeq;

	RTSPMessage::AttrMap::const_iterator itorHeaders;
	itorHeaders = pResp->headers.find(ERMI_HEADER_SESSION);
	if(itorHeaders == pResp->headers.end())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(ERMISession, "OnResponse_SETUP() CSeq[%d] missed 'Session' header"), respCSeq);
		destroy();
		return;
	}

	_sessionId = itorHeaders->second;;

	if (resultCode == RTSPSink::rcOK)
	{
		_sessGroup.updateIndex(*this);
	}
	_sessGroup._env.updataAllocation(_sessGuid, _sessGroup.getName(), _sessionId);
}
void ERMISession::OnResponse_TEARDOWN(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	_log(Log::L_DEBUG, CLOGFMT(ERMISession, "Session[%s, %d, %s] OnResponse_TEARDOWN(), return [%d,%s]"), _sessGuid.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);
	_resultCode = resultCode;
/*	if (resultCode == RTSPSink::rcSessNotFound)
		destroy();*/
	destroy();
}
void ERMISession::OnResponse_SET_PARAMETER(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	_log(Log::L_DEBUG, CLOGFMT(ERMISession, "Session[%s, %d, %s] OnResponse_SET_PARAMETER(), return [%d,%s]"), _sessGuid.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);
	_resultCode = resultCode;
	if (resultCode == RTSPSink::rcSessNotFound)
		destroy();
}
void ERMISession::OnResponse_GET_PARAMETER(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	_log(Log::L_DEBUG, CLOGFMT(ERMISession, "Session[%s, %d, %s] OnResponse_GET_PARAMETER(), return [%d,%s]"), _sessGuid.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);
	_resultCode = resultCode;
	
	if(resultCode == RTSPSink::rcSessNotFound)
	{
		destroy();
	}
}
void ERMISession::OnSessionTimer()
{
	ERMIClient* client = _sessGroup.getERMIClient(); 
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
		_log(Log::L_DEBUG, CLOGFMT(ERMISession, "Session[%s] OnDemandSession[%s]OnSessionTimer(), trigger PING"), _sessionId.c_str(), _sessGuid.c_str());
	}
}

void ERMISession::OnANNOUNCE(RTSPClient& rtspClient, RTSPMessage::Ptr& pInMessage)
{
	_log(Log::L_DEBUG, CLOGFMT(ERMISession, "Session[%s, %s] OnANNOUNCE()"), _sessGuid.c_str(), getSessionGroupName().c_str());
	if (pInMessage->headers.end() == pInMessage->headers.find("Session"))
		return;

}
void ERMISession::OnResponse(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	// wildcast to check "551 Option Not Supported" about ECR: Protocol Versioning for RTSP protocols
	ERMIClient* pClient = (ERMIClient*) &rtspClient;

	return RTSPSession::OnResponse(rtspClient, pReq, pResp, resultCode, resultString); // do the dispatching
}
void ERMISession::OnRequestError(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RequestError errCode, const char* errDesc)
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
// class ERMISessionGroup
// -----------------------------
ERMISessionGroup::SessionGroupMap ERMISessionGroup::_groupMap;
ERMISessionGroup::StringIndex     ERMISessionGroup::_idxGroupBaseUrl;
Mutex ERMISessionGroup::_lockGroups;

#define _log        (_env.getLogger())

ERMISessionGroup::ERMISessionGroup(EdgeRMEnv& env, const std::string& qamName, const std::string& baseURL, int max, Ice::Long syncInterval)
: _env(env), _qamName(qamName), _baseURL(baseURL), _max(max), _syncInterval(syncInterval),
_ERMIClient(*this, _log, _env.getThreadPool(), _env._ermiBindAddr, _baseURL, _env._userAgent.c_str(), _env.getRtspLogLevel())
{
	glog(Log::L_DEBUG, CLOGFMT(ERMISessionGroup, "SessionGroup[%s] adding connection to baseUrl[%s]]"), _qamName.c_str(), _baseURL.c_str());
	
	_idxGroupBaseUrl.insert(StringIndex::value_type(_baseURL, _qamName));

}
ERMISessionGroup::~ERMISessionGroup()
{
	{
		MutexGuard g(_lockSessMap);
		_sessMap.clear();
	}

	clearERMIClient();
}

void ERMISessionGroup::add(ERMISession& sess)
{
	MutexGuard g(_lockSessMap);
	MAPSET(SessionMap, _sessMap, sess._sessGuid, &sess);
}

void ERMISessionGroup::remove(ERMISession& sess)
{
	MutexGuard g(_lockSessMap);
	_sessMap.erase(sess._sessGuid);
}
void ERMISessionGroup::updateIndex(ERMISession& sess, const char* indexName, const char* oldValue)
{
	std::string oldSessId = oldValue? oldValue:sess._sessionId;

	if (!oldSessId.empty())
		eraseSessionId(oldSessId, sess);

	MutexGuard g(_lockSessMap);
	_sessIdIndex.insert(StringIndex::value_type(sess._sessionId, sess._sessGuid));
}

ERMISession::List ERMISessionGroup::lookupByIndex(const char* sessionId, const char* indexName)
{
	ERMISession::List list;

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
ERMISession::Ptr ERMISessionGroup::lookupByOnDemandSessionId(const char* sessOnDemandId)
{
	if (NULL == sessOnDemandId)
		return NULL;

	MutexGuard g(_lockSessMap);
	SessionMap::iterator it = _sessMap.find(sessOnDemandId);
	if (_sessMap.end() ==it)
		return NULL;

	return it->second;
}
void ERMISessionGroup::eraseSessionId(const std::string& sessionId, ERMISession& sess)
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
ERMIClient* ERMISessionGroup::getERMIClient(size_t idxClient)
{
	return &_ERMIClient;
}

ERMISession::Ptr ERMISessionGroup::createSession(const std::string& ODSessId, const char* qamName)
{
	ERMISessionGroup::Ptr pSessGroup = NULL;

	if (ODSessId.empty())
		return NULL;

	pSessGroup = findSessionGroup(qamName);

	if (NULL == pSessGroup)
	{
		glog(Log::L_WARNING, CLOGFMT(ERMISessionGroup, "createSession() session[%s]: none of valid SessionGroup(s) has quota available"), ODSessId.c_str());
		return NULL;
	}

	return (new ERMISession(*pSessGroup, ODSessId));
}

ERMISession::Ptr ERMISessionGroup::openSession(const std::string& ODSessId, const std::string& groupName, bool bRestore)
{
	ERMISessionGroup::Ptr pSessGroup = ERMISessionGroup::findSessionGroup(groupName);
	if (NULL == pSessGroup)
		return NULL;

	ERMISession::Ptr clientSession = pSessGroup->lookupByOnDemandSessionId(ODSessId.c_str());

	if (NULL != clientSession)
		return clientSession;

	if (!bRestore)
		return NULL;

	return (new ERMISession(*pSessGroup, ODSessId));
}


ERMISessionGroup::Ptr ERMISessionGroup::findSessionGroup(const std::string& name)
{
	ERMISessionGroup::Ptr group;

	MutexGuard g(_lockGroups);
	SessionGroupMap::iterator it = _groupMap.find(name);
	if (_groupMap.end() !=it)
		group = it->second;

	return group;
}

std::vector<std::string> ERMISessionGroup::getAllSessionGroup()
{
	MutexGuard g(_lockGroups);
	std::vector<std::string> result;

	for (SessionGroupMap::iterator it = _groupMap.begin(); it != _groupMap.end(); it++)
		result.push_back(it->first);

	return result;
}

void ERMISessionGroup::clearSessionGroup()
{
	MutexGuard g(_lockGroups);

	_groupMap.clear();
}

void ERMISessionGroup::clearERMIClient()
{
/*	MutexGuard g(_lockClients);
	ERMICLients::iterator iter = _ERMIClients.begin();
	for (; iter != _ERMIClients.end(); iter++)
	{
		try
		{
			ERMIClient* pClient = *iter;
			if (pClient)
				delete pClient;
		}
		catch(...)
		{
		}
	}

	_ERMIClients.clear();*/
}


int ERMISessionGroup::sync(bool bOnConnected)
{
	_log(Log::L_DEBUG, CLOGFMT(ERMISessionGroup, "SessionGroup[%s] sync() per %s"), _qamName.c_str(), bOnConnected ? "connected" :"timer");
	if (!bOnConnected && Idle != _status)
	{
		_log(Log::L_DEBUG, CLOGFMT(ERMISessionGroup, "SessionGroup[%s] is already in sync"), _qamName.c_str());
		return 0;
	}

	if (bOnConnected && Socket::stConnected != _ERMIClient.state())
		return -1;

	//set group status
	RTSPRequest::AttrList parameterNames;
	parameterNames.push_back("clab-session-list");
	//	parameterNames.push_back("connection_timeout");
	RTSPMessage::AttrMap headers;
	headers.insert(std::make_pair(ERMI_HEADER_CONTENTTYPE, "text/Parameters"));
	headers.insert(std::make_pair("SessionGroup", _qamName));
	headers.insert(std::make_pair(ERMI_HEADER_REQUIRE,      ERMI_HEADER_REQUIRE_VAL));

	return _ERMIClient.sendGET_PARAMETER(parameterNames, NULL, headers);
}

void ERMISessionGroup::OnSessionListOfSS(const std::vector<ERMIClient::Session_Pair>& listOnSS)
{
	_log(Log::L_DEBUG, CLOGFMT(ERMISessionGroup, "OnSessionListOfSS() SessionGroup[%s] %d sessions on SS"), _qamName.c_str(), listOnSS.size());

	int64 stampStart= ZQ::common::now();
	size_t cSSOrphan =0, cSynced=0, cTorndown=0;

	try {

		{
			MutexGuard g(_lockSessMap);
			for (std::vector<ERMIClient::Session_Pair>::const_iterator iter = listOnSS.begin();	iter != listOnSS.end(); iter++)
			{
				std::string tmpStr = iter->_onClientSessionId + ":" + iter->_sessionId;
				bool bFound = false;

				// find session by key
				if (!iter->_onClientSessionId.empty() && _sessMap.end() != _sessMap.find(iter->_onClientSessionId))
					bFound = true;
				else if (!iter->_sessionId.empty() && _sessIdIndex.end() != _sessIdIndex.find(iter->_sessionId))
					bFound = true;
				else if(_env.syncERMISession(iter->_onClientSessionId, _qamName, iter->_sessionId))
					bFound = true;

				if (bFound)
				{
					cSynced ++;
					//					_log(Log::L_DEBUG, CLOGFMT(ERMISessionGroup, "OnSessionListOfSS() SessionGroup[%s] session[%s] sync-ed"), _name.c_str(), tmpStr.c_str());
					continue;
				}

				cSSOrphan ++;
				_log(Log::L_INFO, CLOGFMT(ERMISessionGroup, "OnSessionListOfSS() SessionGroup[%s] SS-orphan[%s] found, tearing it down"), _qamName.c_str(), tmpStr.c_str());

				//TODO: remove session from media server by TEARDOWN
				RTSPMessage::AttrMap headers;
				MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_REQUIRE, ERMI_HEADER_REQUIRE_VAL);
				MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_SESSION, iter->_sessionId);
				MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_CLABCLIENTSESSIONID, iter->_onClientSessionId);
				MAPSET(RTSPMessage::AttrMap, headers, ERMI_HEADER_CLABREASON, "428 Session Orphan Detected on PHO");

				_ERMIClient.sendTousyMsg("TEARDOWN * RTSP/1.0", headers);

				cTorndown++;
			}
		}
	}
	catch(...)
	{
	}

	setLastSync(now(), Idle);
	_log(Log::L_INFO, CLOGFMT(ERMISessionGroup, "syncSessionList() SessionGroup[%s] done, took %lldmsec: %d sync-ed, %d SS-orphans found, %d torndown"), _qamName.c_str(), ZQ::common::now() -stampStart, cSynced, cSSOrphan, cTorndown);
}

void ERMISessionGroup::OnTimer(const ::Ice::Current& c)
{
	int64 stampNow = now();
	if (getLastSync() + getSyncInterval() > stampNow) // do not need sync now
		return;

	sync();
	setLastSync(stampNow, Sync);
}
Ice::Long ERMISessionGroup::checkAll()
{
	std::vector<ERMISessionGroup::Ptr> groupsToOntimer;
	::Ice::Long stampNow = now();
	::Ice::Long _nextWakeup = stampNow + 1000;

	{
		MutexGuard g(_lockGroups);

		for (SessionGroupMap::iterator iter = _groupMap.begin(); iter != _groupMap.end(); iter++)
		{
			try
			{
				ERMISessionGroup::Ptr group = iter->second;
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

	for (std::vector<ERMISessionGroup::Ptr>::iterator iter2 = groupsToOntimer.begin(); iter2 != groupsToOntimer.end(); iter2++)
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
