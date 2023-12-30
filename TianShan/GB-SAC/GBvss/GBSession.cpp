#include "GBSession.h"
#include "strHelper.h"
#include "SsServiceImpl.h"
#include "GBVSSUtil.h"
#include "GBVSSEnv.h"

extern ZQ::common::NativeThreadPool* gPool;

namespace ZQ{ namespace StreamService {

extern ZQ::StreamService::SsServiceImpl* pServiceInstance;
// -----------------------------
// class GBSession
// -----------------------------
GBSession::GBSession(Log& log, NativeThreadPool& thrdpool, const char* streamDestUrl, const char* filePath, Log::loglevel_t verbosityLevel, int timeout, const char* onDemandSessionId)
: RTSPSession(log, thrdpool, streamDestUrl, filePath, verbosityLevel, timeout, onDemandSessionId), _group(NULL)
{
	_group = GBVSSSessionGroup::allocateSessionGroup("");
	if(_group)
	{
		_groupName = _group->getName();
		_group->add(*this);
	}
}

GBSession::~GBSession()
{
	destroy();
}

void GBSession::destroy()
{
	if(_group)
		_group->remove(*this);
	RTSPSession::destroy();
}

std::string GBSession::getBaseURL()
{
	if (_group)
	{
		return _group->getBaseURL();
	}
	else
		return "";
}

void GBSession::OnResponse_SETUP(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	RTSPSession::OnResponse_SETUP(rtspClient, pReq, pResp, resultCode, resultString);
	_log(Log::L_DEBUG, CLOGFMT(GBSession, "Session[%s, %s, %d, %s] OnResponse_SETUP() , return [%d,%s]"), _sessGuid.c_str(), _sessionId.c_str(), pResp->cSeq, _groupName.c_str(), resultCode, resultString);
	_resultCode = resultCode;
	_stampSetup = now();
	if(pResp->headers.find("TianShan-NoticeParam") != pResp->headers.end())
	{
		std::string noticeParam = pResp->headers["TianShan-NoticeParam"];
		if(noticeParam.find("primaryItemNPT") != std::string::npos)
			_primartItemNPT = noticeParam.substr(noticeParam.find('=')+1, noticeParam.size());
	}
	if(resultCode == RTSPSink::rcOK)
	{
		if(_group)
            _group->updateIndex(*this);
        // get the control url
        std::string ctrlUrl;
        RTSPMessage::AttrMap::const_iterator ctrlSessIt = pResp->headers.find("ControlSession");
        if(ctrlSessIt != pResp->headers.end()) {
            // save the ControlSession header
            _controlSession = ctrlSessIt->second;
            // parse the control session
            // ControlSession: = control-session-id ";" "ControlHost" ":" control-host ";" "ControlPort" ":" control-port [";"  "ControlTimeout" "=" delta-seconds]
            std::string::size_type pos = _controlSession.find_first_of(';');
            if(pos != std::string::npos) {
                std::map<std::string, std::string> csParams;
                ZQ::common::stringHelper::STRINGVECTOR vec;
                stringHelper::SplitString(_controlSession.substr(pos + 1), vec, ";");
                for(size_t i = 0; i < vec.size(); ++i) {
                    ZQ::common::stringHelper::STRINGVECTOR item;
                    ZQ::common::stringHelper::SplitString(vec[i], item, ":", ": ");
                    if(item.size() != 2)
                        continue; // discard

                    csParams[item[0]] = item[1];
                }
                std::string ctrlHost = csParams["ControlHost"];
                std::string ctrlPort = csParams["ControlPort"];
                if(!ctrlHost.empty() && !ctrlPort.empty()) {
                    ctrlUrl = "rtsp://" + ctrlHost + ":" + ctrlPort;
                }
            }
        }
        if(!ctrlUrl.empty()) {
            _controlUri = ctrlUrl;
            _log(Log::L_DEBUG, CLOGFMT(GBSession, "Session[%s] OnResponse_SETUP(), get control URI [%s]"), _sessGuid.c_str(), _controlUri.c_str());
        } else {
            _log(Log::L_DEBUG, CLOGFMT(GBSession, "Session[%s] OnResponse_SETUP(), No control URI provided"), _sessGuid.c_str(), _controlUri.c_str());
        }
	}
	else
	{
	}

    _eventHandle.m_SetEvent(pResp->cSeq);
}

void GBSession::OnResponse_PLAY(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	_log(Log::L_DEBUG, CLOGFMT(GBSession, "Session[%s, %d, %s] OnResponse_PLAY() , return [%d,%s]"), _sessGuid.c_str(), pResp->cSeq, _groupName.c_str(), resultCode, resultString);
	_resultCode = resultCode;
	parseStreamInfo(pResp);
	if(resultCode == RTSPSink::rcOK)
	{
	}	
	else if(resultCode == RTSPSink::rcSessNotFound)
	{
		destroy();
	}
	else
	{
    }
    _eventHandle.m_SetEvent(pResp->cSeq);
}

void GBSession::OnResponse_TEARDOWN(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	_log(Log::L_DEBUG, CLOGFMT(GBSession, "Session[%s, %d, %s] OnResponse_TEARDOWN() , return [%d,%s]"), _sessGuid.c_str(), pResp->cSeq, _groupName.c_str(), resultCode, resultString);
	_resultCode = resultCode;
	_sessionHistory = pResp->contentBody;
    if(resultCode == RTSPSink::rcOK)
    {
        RTSPMessage::AttrMap::const_iterator itStopNPT = pResp->headers.find("StopNPT");
        if(itStopNPT != pResp->headers.end()) {
            _stopNPT = itStopNPT->second;
        }
    }
    else if(resultCode == RTSPSink::rcSessNotFound)
	{
		destroy();
    }
    _eventHandle.m_SetEvent(pResp->cSeq);
}

void GBSession::OnResponse_PAUSE(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	_log(Log::L_DEBUG, CLOGFMT(GBSession, "Session[%s, %d, %s] OnResponse_PAUSE() , return [%d,%s]"), _sessGuid.c_str(), pResp->cSeq, _groupName.c_str(), resultCode, resultString);
	_resultCode = resultCode;
	parseStreamInfo(pResp);
	if(resultCode == RTSPSink::rcSessNotFound)
	{
		destroy();
    }
    _eventHandle.m_SetEvent(pResp->cSeq);
}

void GBSession::OnResponse_GET_PARAMETER(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	_log(Log::L_DEBUG, CLOGFMT(GBSession, "Session[%s, %d, %s] OnResponse_GET_PARAMETER() , return [%d,%s]"), _sessGuid.c_str(), pResp->cSeq, _groupName.c_str(), resultCode, resultString);
	_resultCode = resultCode;
	if(pResp->contentBody.empty())
	{
		_streamInfos.scale = 0.0f;
		_streamInfos.timeoffset = 0;
		_streamInfos.state = "init";
	}
	else
	{
		std::vector<std::string> temp;
		ZQ::common::stringHelper::SplitString(pResp->contentBody, temp, "\r\n", "\r\n");
		for (size_t i = 0; i < temp.size(); i++)
		{
			std::string::size_type pos_begin	= std::string::npos;
			if((pos_begin=temp[i].find(":"))!= std::string::npos)				
			{
				::std::string strKey   = temp[i].substr(0, pos_begin);
				::std::string strValue = temp[i].substr(pos_begin+1, temp[i].size());
				_log(Log::L_DEBUG, CLOGFMT(GBSession, "Session[%s] OnResponse_GET_PARAMETER() , get content [%s]=[%s]"), _sessGuid.c_str(), strKey.c_str(), strValue.c_str());
				std::transform(strKey.begin(), strKey.end(), strKey.begin(), tolower); // to be case-insensitive at parings
				if(strKey.compare("scale")==0)
				{
					_streamInfos.scale = atof(strValue.c_str());
				}
				else if (strKey.compare("position")==0)
				{
					_streamInfos.timeoffset = atof(strValue.c_str())*1000.0;
				}
				else if (0 == strKey.compare("stream_state") || 0 == strKey.compare("presentation_state"))
				{
					_streamInfos.state = strValue;
                } 
				else if (strKey.compare("streamsource") == 0)
				{
                    _streamInfos.source = strValue;
                }
			}
		}
	}
	if(resultCode == RTSPSink::rcSessNotFound)
	{
		destroy();
    }
    _eventHandle.m_SetEvent(pResp->cSeq);
}

void GBSession::OnResponse_SET_PARAMETER(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	_log(Log::L_DEBUG, CLOGFMT(GBSession, "Session[%s, %d, %s] OnResponse_SET_PARAMETER() , return [%d,%s]"), _sessGuid.c_str(), pResp->cSeq, _groupName.c_str(), resultCode, resultString);
	_resultCode = resultCode;
	if(resultCode == RTSPSink::rcSessNotFound)
	{
		destroy();
    }
    _eventHandle.m_SetEvent(pResp->cSeq);
}

void GBSession::OnANNOUNCE(RTSPClient& rtspClient, RTSPMessage::Ptr& pInMessage)
{
	_log(Log::L_DEBUG, CLOGFMT(GBSession, "Session[%s, %s] OnANNOUNCE()"), _sessGuid.c_str(), _groupName.c_str());
	if (pInMessage->headers.find("Session") != pInMessage->headers.end())
	{
		std::string notice = "";
		if (pInMessage->headers.find("notice") != pInMessage->headers.end())
		{
			notice = pInMessage->headers["notice"];
		}
		std::string tianShanNoticeParam = "";
		TianShanIce::Properties props;
		if (pInMessage->headers.find("TianShan-NoticeParam") != pInMessage->headers.end())
		{
			tianShanNoticeParam = pInMessage->headers["TianShan-NoticeParam"];
			std::vector<std::string> params;
			ZQ::common::stringHelper::SplitString(tianShanNoticeParam, params, ";", ";");
			for(size_t i = 0; i < params.size(); i++)
			{
				std::string::size_type pos_begin	= std::string::npos;
				if((pos_begin=params[i].find("="))!= std::string::npos)				
				{
					::std::string strKey   = params[i].substr(0, pos_begin);
					::std::string strValue = params[i].substr(pos_begin+1, params[i].size());
					props.insert(std::make_pair("sys." + strKey, strValue));
					_log(Log::L_DEBUG, CLOGFMT(GBSession, "Session[%s] OnANNOUNCE() , insert property [sys.%s]=[%s]"), _sessGuid.c_str(), strKey.c_str(), strValue.c_str());
				}
			}
		}
		_log(Log::L_DEBUG, CLOGFMT(GBSession, "OnANNOUNCE() , Session[%s, %d] Notice[%s]"), pInMessage->headers["Session"].c_str(), pInMessage->cSeq, notice.c_str());
		std::vector<std::string> temp;
		ZQ::common::stringHelper::SplitString(notice, temp, " ", " ");
		if (!temp.empty())
		{
			ZQ::StreamService::StreamParams paras;
			for(size_t i = 0; i < temp.size(); i++)
			{
				std::string::size_type pos_begin	= std::string::npos;
				if((pos_begin=temp[i].find("="))!= std::string::npos)				
				{
					::std::string strKey   = temp[i].substr(0, pos_begin);
					::std::string strValue = temp[i].substr(pos_begin+1, temp[i].size());
					_log(Log::L_DEBUG, CLOGFMT(GBSession, "Session[%s] OnANNOUNCE() , get content [%s]=[%s]"), _sessGuid.c_str(), strKey.c_str(), strValue.c_str());
                    /*
					if(strKey.compare("npt")==0)
					{
						paras.mask |= MASK_TIMEOFFSET;
						int64 ret	= 0;
						sscanf( strValue.c_str() ,"%lx" , &ret );
						paras.timeoffset = ret * 1000; // convert sec to msec
					}
					else if (strKey.compare("presentation_state")==0)
					{
						paras.mask |= MASK_STATE;
						paras.streamState =  convertState(strValue);
					}
					else if (strKey.compare("scale")==0)
					{
						paras.mask |= MASK_SCALE;
						paras.scale = atof(strValue.c_str());
					}
                    */
				}
			}
			int AnnounceCode = atoi(temp[0].c_str());
			switch (AnnounceCode)
			{
			case RTSPSink::racStateChanged: // call lib OnEvent
				ZQ::StreamService::pServiceInstance->OnStreamEvent( ZQ::StreamService::SsServiceImpl::seStateChanged , _sessGuid , paras , props );
				break;
			case RTSPSink::racScaleChanged:
				ZQ::StreamService::pServiceInstance->OnStreamEvent( ZQ::StreamService::SsServiceImpl::seScaleChanged , _sessGuid , paras , props );
				break;
			case RTSPSink::racEndOfStream:
				ZQ::StreamService::pServiceInstance->OnStreamEvent( ZQ::StreamService::SsServiceImpl::seGone , _sessGuid , paras , props );
				break;
			case RTSPSink::racBeginOfStream:
				ZQ::StreamService::pServiceInstance->OnStreamEvent( ZQ::StreamService::SsServiceImpl::seGone , _sessGuid , paras , props );
				break;
			case RTSPSink::racSessionInProgress:
				{
					GBClient* client = _group->getC1Client(controlUri());
					if(client)
					{
						std::string cseq;
						if (pInMessage->headers.find("CSeq") != pInMessage->headers.end())
						{
							cseq = pInMessage->headers["CSeq"];
						}
						RTSPMessage::Ptr pMessage = new RTSPMessage(atoi(cseq.c_str()));
						pMessage->startLine = "RTSP/1.0 200 OK";
						RTSPMessage::AttrMap     headers;
						if (pInMessage->headers.find("Session") != pInMessage->headers.end())
						{
							headers.insert(std::make_pair("Session",pInMessage->headers["Session"]));
                        }
                        if (pInMessage->headers.find("GlobalSession") != pInMessage->headers.end())
                        {
                            headers.insert(std::make_pair("GlobalSession",pInMessage->headers["GlobalSession"]));
                        }
						client->sendMessage2(pMessage, headers);
					}
				}
				break;
			default:
				break;
			}
		}
	}
}

void GBSession::OnSessionTimer()
{
	GBClient* client = _group->getC1Client(controlUri()); 
	if(client)
	{
		if(_sessionId.empty())
		{
			destroy();
			return;
		}
		RTSPRequest::AttrList parameterNames;
		client->sendGET_PARAMETER(*this, parameterNames);
		_log(Log::L_DEBUG, CLOGFMT(GBSession, "Session[%s] OnSessionTimer() , trigger PING"), _sessGuid.c_str());
	}
}

bool GBSession::parseStreamInfo(const RTSPMessage::Ptr& pResp)
{
//	ZQ::common::MutexGuard g(_mutex);
	_streamInfos = StreamInfos(); //reset _streamInfos so that we won't get the garbage data
	if (pResp->headers.find("Range") != pResp->headers.end())
	{
		std::vector<std::string> temp;
		ZQ::common::stringHelper::SplitString(pResp->headers["Range"], temp, "=- ", "=- ");
		if (temp.size() >= 2)
		{
			_streamInfos.timeoffset = (int64)atof(temp[1].c_str())*1000;			
		}
		if( temp.size() >= 3 )
		{
			//_streamInfos.duration = (int64)(atof(temp[2].c_str()) - atof(temp[1].c_str()))*1000;
			_streamInfos.duration = (int64)(atof(temp[2].c_str())*1000);
		}
	}
	if (pResp->headers.find("Scale") != pResp->headers.end())
	{
		_streamInfos.scale = atof(pResp->headers["Scale"].c_str());
	}
	if (pResp->headers.find("TianShan-Notice") != pResp->headers.end())
	{
		_tianShanNotice = pResp->headers["TianShan-Notice"];
	}
	return true;
}

GBClient*	GBSession::getR2Client()
{
	if(_group)
		return _group->getR2Client();
	else
		return NULL;
}

// -----------------------------
// class GBClient
// -----------------------------
GBClient::GBClient(GBVSSSessionGroup& group, ClientType type, Log& log, NativeThreadPool& thrdpool, InetHostAddress& bindAddress, const std::string& baseURL, const char* userAgent, Log::loglevel_t verbosityLevel, tpport_t bindPort)
: RTSPClient(log, thrdpool, bindAddress, baseURL, userAgent, verbosityLevel, bindPort), _group(group), _type(type)
{
	int poolSize, activeCount, pendingSize;
	TCPSocket::getThreadPoolStatus(poolSize, activeCount, pendingSize);
//	TCPSocket::getPendingSize(poolSize);
	TCPSocket::resizeThreadPool(poolSize+1);
}

GBClient::~GBClient()
{
}

void GBClient::sendTeardown(const std::string& sessionId, const std::string& onDemandSessionId)
{
	RTSPMessage::Ptr pMessage = new RTSPMessage(lastCSeq());
	pMessage->startLine = "TEARDOWN * RTSP/1.0";
	RTSPMessage::AttrMap     headers;
	headers.insert(std::make_pair("Session",sessionId));
	if (!onDemandSessionId.empty())
	{
		headers.insert(std::make_pair("OnDemandSessionId",onDemandSessionId));
	}
	sendMessage(pMessage, headers);
}

void GBClient::sendPing()
{
	RTSPMessage::Ptr pMessage = new RTSPMessage(lastCSeq());
	pMessage->startLine = "PING * RTSP/1.0";
	RTSPMessage::AttrMap     headers;
	sendMessage(pMessage, headers);
}

int GBClient::OnRequestPrepare(RTSPRequest::Ptr& pReq)
{
	if (!pReq)
		return 0;

	if (pReq->headers.end() == pReq->headers.find("Content-Type"))
		pReq->headers.insert(RTSPRequest::AttrMap::value_type("Content-Type", "text/parameters"));

	return 0;
}

void GBClient::OnResponse(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
{
	_log(Log::L_INFO, CLOGFMT(RTSPClient, "OnResponse() conn[%s] %s(%d) received response: %d %s"), connDescription(), pReq->_commandName.c_str(), pReq->cSeq, resultCode, resultString);
	if(pReq->_commandName == "GET_PARAMETER" && resultCode == RTSPSink::rcOK) // ok 
	{
		if(pResp->headers.find("SessionGroup") != pResp->headers.end())
		{
			GBVSSSessionGroup* group = GBVSSSessionGroup::findSessionGroup(pResp->headers["SessionGroup"]);
			if (!group)
				return;//no session group is specified, just skip processing

			std::vector<GBClient::Session_Pair> list;
			size_t index = 0;
			std::string key("session_list: ");
			index = pResp->contentBody.find(key);

			if (index == std::string::npos)
				return; // no session list is found

			index += key.length();
			// chop string to get session_list: line
			std::string line = pResp->contentBody.substr(index, pResp->contentBody.size());
			_log(Log::L_DEBUG, CLOGFMT(RTSPClient, "OnResponse() SessionGroup[%s] session_list[%s]"), pResp->headers["SessionGroup"].c_str(), pResp->contentBody.c_str());
			std::vector<std::string> tmpVec;	
			stringHelper::SplitString(line, tmpVec, " \t", " \t\r\n","","");
			for (size_t i = 0; i < tmpVec.size(); i++)
			{
				GBClient::Session_Pair sessPair;
				index = tmpVec[i].find(":");
				if(index != std::string::npos)
				{
					sessPair._sessionId = tmpVec[i].substr(0, index);
					sessPair._onDemandSessionId = tmpVec[i].substr(index+1, tmpVec[i].size());						
				}
				else
				{
					sessPair._sessionId = tmpVec[i];
				}
				list.push_back(sessPair);
			}
			group->syncSessionList(list);
		}
	}
}

void GBClient::OnServerRequest(RTSPClient& rtspClient, const char* cmdName, const char* url, RTSPMessage::Ptr& pInMessage)
{
	_log(Log::L_INFO, CLOGFMT(RTSPClient, "OnServerRequest() conn[%s] received peer request %s(%d)"), connDescription(), pInMessage->startLine.c_str(), pInMessage->cSeq);
	//check if it is session in progress
	if(0 != strcmp(cmdName, "ANNOUNCE"))
		return;
	
	std::string notice = "";
	if (pInMessage->headers.find("Notice") == pInMessage->headers.end())
		return;

	notice = pInMessage->headers["Notice"];
	std::vector<std::string> temp;
	ZQ::common::stringHelper::SplitString(notice, temp, " ", " ");
	if (temp.empty())
		return;
	
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
			pMessage->startLine = "RTSP/1.0 454 Session Not Found";
			RTSPMessage::AttrMap     headers;
			if (pInMessage->headers.find("Session") != pInMessage->headers.end())
			{
				headers.insert(std::make_pair("Session",pInMessage->headers["Session"]));
			}
			sendMessage(pMessage, headers);
		}
		break;
	default:
		break;
	}
}

void GBClient::OnRequestError(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RequestError errCode, const char* errDesc)
{
	if (NULL == errDesc || strlen(errDesc) <=0)
		errDesc = requestErrToStr(errCode);

	char buf[64];
	_log(Log::L_INFO, CLOGFMT(RTSPClient, "OnRequestError() conn[%s] %s(%d): %d %s; %s +%d"),
		connDescription(), pReq->_commandName.c_str(), pReq->cSeq, errCode, errDesc, TimeUtil::TimeToUTC(pReq->stampCreated, buf, sizeof(buf)-2, true), _timeout);
}

void GBClient::OnConnected()
{
	RTSPClient::OnConnected();
	if(NC_R2 == _type)	//need sync and set_parameter
	{
		if (now() - _group.getLastSync() > 60000)
		{	
			if(_group.sync(true) > 0)
			{
				_group.setLastSync(now(), GBVSSSessionGroup::Sync);
			}
		}

		RTSPRequest::AttrMap paramMap;
		paramMap.insert(std::make_pair("session_groups", _group.getName()));
		RTSPMessage::AttrMap headers;
		headers.insert(std::make_pair("SessionGroup",_group.getName()));
		sendSET_PARAMETER(paramMap, NULL, headers);
	}
}

void GBClient::OnError()
{
	RTSPClient::OnError();
	if(NC_R2 == _type)
	{
		_group.setStatus(GBVSSSessionGroup::Idle);
	}
}

// -----------------------------
// class GBVSSSessionGroup
// -----------------------------
GBVSSSessionGroup::SessionGroups GBVSSSessionGroup::_groups;
Mutex GBVSSSessionGroup::_lockGroups;
GBVSSSessionGroup::GBVSSSessionGroup(GBVSSEnv& env, Log& log, const std::string& name, const std::string& baseURL, int max, Ice::Long syncInterval)
:_env(env), _log(log), _name(name), _baseURL(baseURL), _max(max), _status(Idle), _syncInterval(syncInterval), _stampLastSync(0), _R2Clinet(NULL)
{
	{
		MutexGuard g(GBVSSSessionGroup::_lockGroups);
		_groups.insert(std::make_pair(_baseURL, this));
	}
	if(!_R2Clinet)
	{
		InetHostAddress bindAddress;
		_R2Clinet = new GBClient(*this, GBClient::NC_R2, glog, _env.thrdPoolRTSP , bindAddress, baseURL, NULL, Log::L_DEBUG);
	}
}

GBVSSSessionGroup::~GBVSSSessionGroup()
{
	{
		MutexGuard g(_lockSessMap);
		_sessMap.clear();
	}
	clearGBClient();
}

void GBVSSSessionGroup::add(GBSession& sess)
{
	MutexGuard g(_lockSessMap);
	MAPSET(SessionMap, _sessMap, sess._sessGuid, &sess);
	if (!sess._sessionId.empty())
		_sessIdIndex.insert(SessionIndex::value_type(sess._sessionId, sess._sessGuid));
}

void GBVSSSessionGroup::remove(GBSession& sess)
{
	if (!sess._sessionId.empty())
		eraseSessionId(sess._sessionId, sess);

	MutexGuard g(_lockSessMap);
	_sessMap.erase(sess._sessGuid);
}

void GBVSSSessionGroup::updateIndex(GBSession& sess, const char* indexName, const char* oldValue)
{
	std::string oldSessId = oldValue? oldValue:sess._sessionId;

	if (!oldSessId.empty())
		eraseSessionId(oldSessId, sess);

	MutexGuard g(_lockSessMap);
	_sessIdIndex.insert(SessionIndex::value_type(sess._sessionId, sess._sessGuid));
}

GBSession::List GBVSSSessionGroup::lookupByIndex(const char* sessionId, const char* indexName)
{
	GBSession::List list;

	if (NULL == sessionId)
		return list;

	SessionIndexRange range;

	MutexGuard g(_lockSessMap);
	range = _sessIdIndex.equal_range(sessionId);

	for (SessionIndex::iterator it = range.first; it != range.second; ++it)
	{
		SessionMap::iterator itS = _sessMap.find(it->second);
		if (_sessMap.end() ==itS)
			continue;

		list.push_back(itS->second);
	}

	return list;
}

GBSession::Ptr& GBVSSSessionGroup::lookupByOnDemandSessionId(const char* sessOnDemandId)
{
	static GBSession::Ptr Nil=NULL;

	if (NULL == sessOnDemandId)
		return Nil;

	MutexGuard g(_lockSessMap);
	SessionMap::iterator it = _sessMap.find(sessOnDemandId);
	if (_sessMap.end() ==it)
		return Nil;

	return it->second;
}

void GBVSSSessionGroup::eraseSessionId(const std::string& sessionId, GBSession& sess)
{
	SessionIndexRange range;

	MutexGuard g(_lockSessMap);
	range = _sessIdIndex.equal_range(sessionId);
	for (SessionIndex::iterator it = range.first; it != range.second; ++it)
	{
		if (0 != it->second.compare(sess._sessGuid))
			continue;

		_sessIdIndex.erase(it);
		break;
	}
}

GBClient* GBVSSSessionGroup::getC1Client(const std::string& controlURL)
{
	if(controlURL.empty())
		return NULL;
	{
		MutexGuard g(_lockClients);
		GBClientMap::iterator iter = _C1ClientMap.find(controlURL);
		if(iter != _C1ClientMap.end())
			return iter->second;
	}
	//	if no instance, new one
	InetHostAddress bindAddress;
	GBClient* client = new GBClient(*this, GBClient::NC_C1, glog, _env.thrdPoolRTSP, bindAddress, controlURL, NULL, Log::L_DEBUG);
	GBVSSSessionGroup::addC1Client(controlURL, client);
	return client;
}

GBClient* GBVSSSessionGroup::getR2Client()
{ 
	if (!_R2Clinet)
	{
		InetHostAddress bindAddress;
		_R2Clinet = new GBClient(*this, GBClient::NC_R2, glog, *gPool, bindAddress, _baseURL, NULL, Log::L_DEBUG);
	}
	return _R2Clinet;
};

int GBVSSSessionGroup::sync(bool bOnConnected)
{
	_log(Log::L_DEBUG, CLOGFMT(GBVSSSessionGroup, "SessionGroup[%s] sync()"), _name.c_str());
	if(!bOnConnected && _status != Idle)
	{
		_log(Log::L_DEBUG, CLOGFMT(GBVSSSessionGroup, "SessionGroup[%s] is already in sync"), _name.c_str());
		return 0;
	}
	//set group status
	if(_R2Clinet)
	{
		RTSPRequest::AttrList parameterNames;
		parameterNames.push_back("session_list");
//		parameterNames.push_back("connection_timeout");
		RTSPMessage::AttrMap headers;
		headers.insert(std::make_pair("SessionGroup",_name));
		headers.insert(std::make_pair("Require","com.comcast.ngod.r2"));
		return _R2Clinet->sendGET_PARAMETER(parameterNames, NULL, headers);
	}
	else
		return -1;
}

void GBVSSSessionGroup::syncSessionList(const std::vector<GBClient::Session_Pair>& list)
{
	_log(Log::L_DEBUG, CLOGFMT(GBVSSSessionGroup, "SessionGroup[%s] enter sync SessionList"), _name.c_str());
	try{
		std::vector<GBSession::Ptr> _awaitDestroySession;
		{
			MutexGuard g(_lockSessMap);
			for (std::vector<GBClient::Session_Pair>::const_iterator iter = list.begin();
				iter != list.end();
				iter++)
			{
				bool bFound = false;

				//find session by key
				if (!iter->_onDemandSessionId.empty())
				{
					if(_sessMap.find(iter->_onDemandSessionId) != _sessMap.end())
						bFound = true;
				}
				else if (!iter->_sessionId.empty())
				{
					if(_sessIdIndex.find(iter->_sessionId) != _sessIdIndex.end())
						bFound = true;
				}

				std::string tmpStr = iter->_onDemandSessionId + ":" + iter->_sessionId;
				if (!bFound)
				{
					_log(Log::L_DEBUG, CLOGFMT(GBVSSSessionGroup, "syncSessionList() , SessionGroup[%s] not find by key %s"), _name.c_str(), tmpStr.c_str());
					//TODO: remove session from media server by TEARDOWN
					if(_R2Clinet)
					{
						RTSPRequest::AttrList parameterNames;
						parameterNames.push_back("session_list");
						parameterNames.push_back("connection_timeout");
						RTSPMessage::AttrMap headers;
						headers.insert(std::make_pair("SessionGroup",_name));
						headers.insert(std::make_pair("Require","com.comcast.ngod.r2"));
						_R2Clinet->sendTeardown(iter->_sessionId, iter->_onDemandSessionId);
					}
				}
				else
				{
					_log(Log::L_DEBUG, CLOGFMT(GBVSSSessionGroup, "syncSessionList() , SessionGroup[%s] find by key %s"), _name.c_str(), tmpStr.c_str());
				}
			}

			//sync sessions in memory
			for (SessionMap::iterator sessIter = _sessMap.begin(); sessIter != _sessMap.end(); sessIter++)
			{
				std::string onDemandSessionId = (sessIter->second)->_sessGuid;
				std::vector<GBClient::Session_Pair>::const_iterator pIter = find_if(list.begin(), list.end(), GBClient::FindByOnDemandSessionID(onDemandSessionId));
				if (pIter == list.end() && (sessIter->second)->getStampSetup() > getLastSync())
				{
					_awaitDestroySession.push_back(sessIter->second);
				}
			}
		}

		//sync streamers in database
		::TianShanIce::Streamer::SsPlaylistS sessions = ZQ::StreamService::pServiceInstance->listSessions();
		for (::TianShanIce::Streamer::SsPlaylistS::iterator sIter = sessions.begin(); sIter != sessions.end(); sIter++)
		{
			std::string sessName = (*sIter)->getAttribute(ONDEMANDNAME_NAME);
			std::string groupName = (*sIter)->getAttribute(SESSION_GROUP);
			if(groupName != _name)
				continue;

			std::string attr = (*sIter)->getAttribute(ONDEMANDNAME_NAME);
			std::vector<GBClient::Session_Pair>::const_iterator pIter = find_if(list.begin(), list.end(), GBClient::FindByOnDemandSessionID(attr));
			if (pIter == list.end())
			{
				std::string timeStamp = (*sIter)->getAttribute(SETUP_TIMESTAMP);
				if(_atoi64(timeStamp.c_str()) > getLastSync())
				{
					//if sess setup after sync, do not destroy it
					char szSessSetupTime[256];
					char szSyncStartTime[256];
					ZQTianShan::TimeToUTC( _atoi64(timeStamp.c_str()) , szSessSetupTime, sizeof(szSessSetupTime));
					ZQTianShan::TimeToUTC( getLastSync() , szSyncStartTime, sizeof(szSyncStartTime));
					_log(Log::L_INFO, CLOGFMT(GBVSSSessionGroup, "syncSessionList() , SessionGroup[%s] session[%s] setup time[%s] after sync start time[%s], skipped"), _name.c_str(), sessName.c_str(), szSessSetupTime, szSyncStartTime);
					continue;
				}
				// destroy this session
				Ice::Context ctx;
				ctx.insert(std::make_pair("caller", "SYNC"));
				(*sIter)->destroy(ctx);
			}
		}

		for (std::vector<GBSession::Ptr>::iterator desIter = _awaitDestroySession.begin(); desIter != _awaitDestroySession.end(); desIter++)
		{
			_log(Log::L_DEBUG, CLOGFMT(GBVSSSessionGroup, "syncSessionList() , destroy GBSession[%s]"), (*desIter)->_sessGuid.c_str());
			(*desIter)->destroy();
		}
	}
	catch(...)
	{
	};

	setLastSync(now(), Idle);

	_log(Log::L_DEBUG, CLOGFMT(GBVSSSessionGroup, "SessionGroup[%s] leave sync SessionList"), _name.c_str());
}

void GBVSSSessionGroup::OnTimer(const ::Ice::Current&)
{
	if (getLastSync() + getSyncInterval() > now()) // do not need sync now
		return;
	if (sync() > 0)
	{
		setLastSync(now(), Sync);
	}
}

GBVSSSessionGroup* GBVSSSessionGroup::allocateSessionGroup(const std::string& baseURL)
{
	MutexGuard g(_lockGroups);
	SessionGroupsRange range;
	size_t count = 0;
	if(baseURL.empty() || baseURL=="*")
	{
		range.first = _groups.begin();
		range.second = _groups.end();
		count = _groups.size();
	}
	else 
	{
		range = _groups.equal_range(baseURL);
		count = _groups.count(baseURL);
	}

	if(count <= 0)
	{
		glog(Log::L_ERROR, CLOGFMT(GBVSSSessionGroup, "allocateSessionGroup() , no SessionGroup available"));	
		return NULL;
	}

	srand( (unsigned)time( NULL ) );
	int random = rand() % count; 

	SessionGroups::iterator mid = range.first;
	for (int i=0; i < random; i++)
	{
		mid++;
	}
	
	SessionGroups::iterator iter = mid;
	for(; iter!=range.second;iter++)
	{
		if(iter->second->size() < (size_t)iter->second->getMaxSize())
		{
			glog(Log::L_DEBUG, CLOGFMT(GBVSSSessionGroup, "allocateSessionGroup() , SessionGroup[%s] selected"), iter->second->getName().c_str());
			return iter->second;
		}
	}
	for(iter = mid; iter!=range.first;iter--)
	{
		if(iter->second->size() < (size_t)iter->second->getMaxSize())
		{
			glog(Log::L_DEBUG, CLOGFMT(GBVSSSessionGroup, "allocateSessionGroup() , SessionGroup[%s] selected"), iter->second->getName().c_str());
			return iter->second;
		}
	}
	glog(Log::L_ERROR, CLOGFMT(GBVSSSessionGroup, "allocateSessionGroup() , no SessionGroup available"));
	return NULL;
}

GBVSSSessionGroup* GBVSSSessionGroup::findSessionGroup(const std::string& name)
{
	MutexGuard g(_lockGroups);

	SessionGroups::iterator iter = _groups.begin();
	for(; iter!=_groups.end();iter++)
	{
		if(iter->second->getName() == name)
			return iter->second;
	}
	return NULL;
}

std::vector<std::string> GBVSSSessionGroup::getAllSessionGroup()
{
	MutexGuard g(_lockGroups);
	std::vector<std::string> result;

	SessionGroups::iterator iter = _groups.begin();
	for(; iter!=_groups.end();iter++)
	{
		result.push_back(iter->second->getName());
	}
	return result;
}

void GBVSSSessionGroup::addC1Client(const std::string& controlURL, GBClient* client)
{
	MutexGuard g(_lockClients);
	if(_C1ClientMap.find(controlURL) == _C1ClientMap.end())
		_C1ClientMap.insert(std::make_pair(controlURL, client));
}

void GBVSSSessionGroup::reomoveC1Client(const std::string& controlURL)
{
	MutexGuard g(_lockClients);
	GBClientMap::iterator iter = _C1ClientMap.find(controlURL);
	if(iter != _C1ClientMap.end())
		_C1ClientMap.erase(iter);
}

void GBVSSSessionGroup::addSessionGroup(const std::string& name, GBVSSSessionGroup* group)
{
	MutexGuard g(_lockGroups);
	if(_groups.find(name) != _groups.end())
		_groups.insert(std::make_pair(name, group));
}

void GBVSSSessionGroup::removeSessionGroup(const std::string& name)
{
	MutexGuard g(_lockGroups);
	SessionGroups::iterator iter = _groups.find(name);
	if(iter != _groups.end())
		_groups.erase(iter);
}

void GBVSSSessionGroup::clearSessionGroup()
{
	MutexGuard g(_lockGroups);
	SessionGroups::iterator iter = _groups.begin();
	for (; iter != _groups.end(); iter++)
	{
		try
		{
			GBVSSSessionGroup* pGroup = iter->second;
			if(pGroup)
				delete pGroup;
		}
		catch(...)
		{
		}
	}
	_groups.clear();
}

void GBVSSSessionGroup::clearGBClient()
{
	MutexGuard g(_lockClients);
	GBClientMap::iterator iter = _C1ClientMap.begin();
	for (; iter != _C1ClientMap.end(); iter++)
	{
		try
		{
			GBClient* pClient = iter->second;
			if(pClient)
				delete pClient;
		}
		catch(...)
		{
		}
	}
	_C1ClientMap.clear();
	if(_R2Clinet)
	{
		delete _R2Clinet;
		_R2Clinet = NULL;
	}
}

Ice::Long GBVSSSessionGroup::checkAll()
{
	MutexGuard g(_lockGroups);
	::Ice::Long stampNow = now();
	::Ice::Long   _nextWakeup = stampNow + 1000;
	std::vector<GBVSSSessionGroup*> groupsToOntimer;
	GBVSSSessionGroup::SessionGroups::iterator iter = _groups.begin();
	for (; iter != _groups.end(); iter++)
	{
		try
		{
			GBVSSSessionGroup* group = iter->second;
			if(group)
			{
				if (stampNow - group->getLastSync() > group->getSyncInterval())
					groupsToOntimer.push_back(group);
				else
					_nextWakeup = (_nextWakeup > group->getSyncInterval()) ? group->getSyncInterval() : _nextWakeup;
			}
		}
		catch(...)
		{
		}
	}
	for (std::vector<GBVSSSessionGroup*>::iterator iter2 = groupsToOntimer.begin(); iter2 != groupsToOntimer.end(); iter2++)
	{
		try
		{
			(*iter2)->OnTimer();
		}
		catch(...)
		{
		}
	}
	groupsToOntimer.clear();
	return _nextWakeup;
}

// -----------------------------
// class SyncThread
// -----------------------------
SyncWatchDog::SyncWatchDog(ZQ::common::Log& log)
: _log(log), _bRun(false) 
{
}

SyncWatchDog::~SyncWatchDog()
{	
	//exit thread
	terminate(0);
	{
		ZQ::common::MutexGuard gd(_lockGroups);
		_groupsToSync.clear();
	}
	exit();
}

int SyncWatchDog::terminate(int code)
{
	if (_bRun == false)
		return 1;
	_bRun = false;
	m_Event.signal();
	//wait until the run function exit
	waitHandle(100000);

	return 1;
}
int SyncWatchDog::run()
{
	_bRun = true;

	while (_bRun)
	{
		ZQ::common::MutexGuard gd(_lockGroups);
//		Ice::Long stampNow = now();
		Ice::Long _nextWakeup = 0;
		SyncMap::iterator iter = _groupsToSync.begin();
		for (; iter != _groupsToSync.end(); iter++)
		{
			GBVSSSessionGroup* group = iter->first;
			if(group)
			{
				group->OnTimer();
				if( 0 == _nextWakeup )
					_nextWakeup = group->getLastSync() + group->getSyncInterval();
				else
					_nextWakeup = (_nextWakeup > (group->getLastSync() + group->getSyncInterval())) ? (group->getLastSync() + group->getSyncInterval()) : _nextWakeup;
			}
		}
		Ice::Long sleepTime = (Ice::Long) (_nextWakeup - now());

		if (sleepTime < 100)
			sleepTime = 100;
		m_Event.wait(sleepTime);
	}	
	return 1;
}

void SyncWatchDog::watch(GBVSSSessionGroup* group, ::Ice::Long syncInterval)
{
	ZQ::common::MutexGuard gd(_lockGroups);

	if (syncInterval < 0)
		syncInterval = 0;
	::Ice::Long newSync = now() + syncInterval;

	_groupsToSync.insert(std::make_pair(group, newSync));
}

}}//namespace ZQ::StreamSerice
