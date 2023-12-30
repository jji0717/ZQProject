#include "S6Client.h"
#include "IPathHelperObj.h"
#include "public.h"
#include "strHelper.h"
#include "TianShanIceHelper.h"
#include "Guid.h"
#include "PhoEdgeRMEnv.h"

extern ZQ::common::Log* phoErmLog;
extern ZQ::common::Config::Loader<PHOConfig>  _cfg;
namespace ZQTianShan {
	namespace EdgeRM {

		 using namespace ZQ::common;

		 // -----------------------------
		 // class S6Client
		 // -----------------------------
		 S6Client::S6Client(S6SessionGroup& sessGroup, Log& log, NativeThreadPool& thrdpool, InetHostAddress& bindAddress, const std::string& baseURL, const char* userAgent, Log::loglevel_t verbosityLevel, tpport_t bindPort)
			 : RTSPClient(log, thrdpool, bindAddress, baseURL, userAgent, verbosityLevel, bindPort), _sessGroup(sessGroup)
		 {
			 int poolSize, activeCount, pendingSize;
			 TCPSocket::getThreadPoolStatus(poolSize, activeCount, pendingSize);
			 //	TCPSocket::getPendingSize(poolSize);
			 TCPSocket::resizeThreadPool(poolSize+1);

			 _cContinuousTimeout = 0;
		 }

		S6Client::~S6Client()
		{
			std::string str;
			ZQ::common::MutexGuard g(_lkEventMap);
			for (EventMap::iterator it = _eventMap.begin(); it !=_eventMap.end(); it ++)
			{
				char buf[64];
				//		Event* pEvent = (Event*) it->second;
				snprintf(buf, sizeof(buf)-2, "seq(%d)[%p] ", it->first, it->second.get());
				str += buf;

				//		if (NULL == pEvent)
				//			continue;
				//		delete pEvent;
			}
			_eventMap.clear();

			_log(Log::L_DEBUG, CLOGFMT(S6Client, "~S6Client() SessionGroup[%s]BaseURL[%s] conn[%s]: cleaned await events: %s"), _sessGroup.getName().c_str(), _sessGroup.getBaseURL().c_str(), connDescription(), str.c_str());
		}
		void S6Client::OnServerRequest(RTSPClient& rtspClient, const char* cmdName, const char* url, RTSPMessage::Ptr& pInMessage)
		{
			_log(Log::L_INFO, CLOGFMT(S6Client, " OnServerRequest() SessionGroup[%s] conn[%s] received peer request %s(%d)"), _sessGroup.getName().c_str(), connDescription(), pInMessage->startLine.c_str(), pInMessage->cSeq);
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

					// the session must not found when reaches S6Client::OnServerRequest(), otherwise should be S6Session::OnServerRequest
					switch (AnnounceCode)
					{
					case RTSPSink::racSessionInProgress:
						{
							// the session must not found when reaches S6Client::OnServerRequest(), otherwise should be S6Session::OnServerRequest
							RTSPMessage::AttrMap::const_iterator itorHeaders;
							itorHeaders = pInMessage->headers.find(NGOD_HEADER_SEQ);
							if(itorHeaders == pInMessage->headers.end())
							{
								break;
							}
							std::string cSeq = itorHeaders->second;;

							itorHeaders = pInMessage->headers.find(NGOD_HEADER_ONDEMANDSESSIONID);
							if(itorHeaders == pInMessage->headers.end())
							{
								break;
							}
							std::string OnDemandSession = itorHeaders->second;

							itorHeaders = pInMessage->headers.find(NGOD_HEADER_SESSION);
							if(itorHeaders == pInMessage->headers.end())
							{
								break;
							}
							std::string Session = itorHeaders->second;

							std::string SessionGroup;

							RTSPMessage::Ptr pMessage = new RTSPMessage(atoi(cSeq.c_str()));
							pMessage->startLine = ResponseSessionNotFound;

							RTSPMessage::AttrMap     headers;

                            MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_ONDEMANDSESSIONID,OnDemandSession.c_str());
							MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_SESSION,Session.c_str());
							MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_SESSIONGROUP, _sessGroup.getName().c_str());

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

		void S6Client::OnConnected()
		{
			RTSPClient::OnConnected();

			if (now() - _sessGroup.getLastSync() > 60000)
			{	
				if(_sessGroup.sync(true) > 0)
					_sessGroup.setLastSync(now(), S6SessionGroup::Sync);
			}

			RTSPRequest::AttrMap paramMap;
			MAPSET(RTSPMessage::AttrMap, paramMap, "sessionGroups", _sessGroup.getName());

			RTSPMessage::AttrMap headers;
			MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_CONTENTTYPE,      "text/parameters");
			MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_REQUIRE,			"com.comcast.ngod.s6");
			MAPSET(RTSPMessage::AttrMap, headers, "SessionGroup",			_sessGroup.getName());

			sendSET_PARAMETER(paramMap, NULL, headers);

			_log(Log::L_INFO, CLOGFMT(S6Client, "OnConnected() SessionGroup[%s] new conn: %s"), _sessGroup.getName().c_str(), connDescription());
		}

		void S6Client::OnError()
		{
			RTSPClient::OnError();
			_sessGroup.setStatus(S6SessionGroup::Idle);
			_log(Log::L_WARNING, CLOGFMT(S6Client, "OnError() SessionGroup[%s] Connect error"), _sessGroup.getName().c_str());
		}

		void S6Client::OnRequestError(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RequestError errCode, const char* errDesc)
		{
			if (!pReq)
				return;

			if (NULL == errDesc || strlen(errDesc) <=0)
				errDesc = requestErrToStr(errCode);

			char buf[64];
			_log(Log::L_ERROR, CLOGFMT(S6Client, "OnRequestError() SessionGroup[%s] conn[%s] %s(%d): %d %s; %s +%d"),
				_sessGroup.getName().c_str(), connDescription(), pReq->_commandName.c_str(), pReq->cSeq, errCode, errDesc, TimeUtil::TimeToUTC(pReq->stampCreated, buf, sizeof(buf)-2, true), _timeout);

			// wake up the waiting
			wakeupByCSeq(pReq->cSeq);
		}

		int S6Client::OnRequestPrepare(RTSPRequest::Ptr& pReq)
		{
			if (!pReq)
				return -1;

			ZQ::common::MutexGuard g(_lkEventMap);
			if (pReq->cSeq<=0 || _eventMap.end() != _eventMap.find(pReq->cSeq))
				return -2;

			Event::Ptr pEvent = new Event();
			if (!pEvent)
				return -3;

			MAPSET(EventMap, _eventMap, pReq->cSeq, pEvent);

			_log(Log::L_DEBUG, CLOGFMT(S6Client, "OnRequestPrepare() SessionGroup[%s] conn[%s](%d): added syncEvent[%p]"),
				_sessGroup.getName().c_str(), connDescription(), pReq->cSeq, pEvent.get());

			return 0;
		}

		void S6Client::OnRequestClean(RTSPRequest& req)
		{
			ZQ::common::MutexGuard g(_lkEventMap);
			EventMap::iterator it = _eventMap.find(req.cSeq);
			if (_eventMap.end() == it)
				return;

			_log(Log::L_DEBUG, CLOGFMT(S6Client, "OnRequestClean() SessionGroup[%s] conn[%s](%d): syncEvent[%p] cleaned"),
				_sessGroup.getName().c_str(), connDescription(), req.cSeq, it->second.get());

			_eventMap.erase(it);

		}

		bool S6Client::waitForResponse(uint32 cseq)
		{
			Event::Ptr pEvent =NULL;

			{
				ZQ::common::MutexGuard g(_lkEventMap);
				EventMap::iterator it = _eventMap.find(cseq);
				if (_eventMap.end() != it)
					pEvent = it->second;
			}

			if (!pEvent)
				return false;

			_log(Log::L_DEBUG, CLOGFMT(S6Client, "waitForResponse() SessionGroup[%s] conn[%s](%d): waiting for syncEvent[%p] timeout[%d]"),
				_sessGroup.getName().c_str(), connDescription(), cseq, pEvent.get(), _messageTimeout);

			if (SYS::SingleObject::SIGNALED == pEvent->wait(_messageTimeout))
			{
				_cContinuousTimeout =0;
				return true;
			}

			if (_cfg.sessionGroup.SessionInterfaceDisconnectAtTimeout > 1 && isConnected() && (int32)(++_cContinuousTimeout) >= _cfg.sessionGroup.SessionInterfaceDisconnectAtTimeout)
			{
				_log(Log::L_WARNING, CLOGFMT(S6Client, "waitForResponse() SessionGroup[%s] conn[%s]: counted timeout[%d]msec continously for [%d]times but limitation[%d], give it up and reconnecting"),
					_sessGroup.getName().c_str(), connDescription(), _messageTimeout, _cContinuousTimeout, _cfg.sessionGroup.SessionInterfaceDisconnectAtTimeout);

				disconnect();
				_cContinuousTimeout =0;
			}

			return false;
		}

		void S6Client::wakeupByCSeq(uint32 cseq)
		{
			Event::Ptr pEvent =NULL;

			{
				ZQ::common::MutexGuard g(_lkEventMap);
				EventMap::iterator it = _eventMap.find(cseq);
				if (_eventMap.end() != it)
					pEvent = it->second;
			}

			_log(Log::L_DEBUG, CLOGFMT(S6Client, "wakeupByCSeq() SessionGroup[%s] conn[%s](%d): signalling syncEvent[%p]"),
				_sessGroup.getName().c_str(), connDescription(), cseq, pEvent.get());

			if (!pEvent)
				return;

			pEvent->signal();
		}

		int S6Client::sendTousyMsg(const std::string& startLine, RTSPMessage::AttrMap& headers, const std::string& body, const int cseq)
		{
			RTSPMessage::Ptr pMessage = new RTSPMessage((cseq>0) ? lastCSeq() : cseq);
			pMessage->startLine = startLine;
			return sendMessage(pMessage, headers);
		}
		// about the non-session requests
		void S6Client::OnResponse(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
		{
			if (!pReq)
				return;

			_log(Log::L_INFO, CLOGFMT(S6Client, "OnResponse() SessionGroup[%s]-conn[%s] %s(%d) received response: %d %s"), _sessGroup.getName().c_str(), connDescription(), pReq->_commandName.c_str(), pReq->cSeq, resultCode, resultString);

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
							_log(Log::L_WARNING, CLOGFMT(S6Client, "OnResponse() SessionGroup[%s]-conn[%s] %s(%d) unmatched SessionGroup[%s] in response, ignore"),
								groupname.c_str(), connDescription(), pReq->_commandName.c_str(), pReq->cSeq, gnOfResponse.c_str());
							break;
						}
					}

					std::vector<S6Client::Session_Pair> list;
					size_t index = 0;
					std::string key("session_list:");
					index = pResp->contentBody.find(key);
					if (std::string::npos == index)
					{
						_log(Log::L_DEBUG, CLOGFMT(S6Client, "OnResponse() conn[%s] %s(%d) SessionGroup[%s] no session_list found in response, ignore"), 
							connDescription(), pReq->_commandName.c_str(), pReq->cSeq, groupname.c_str());
						break;
					}

					index += key.length();
					// chop string to get session_list: line
					std::string line = pResp->contentBody.substr(index, pResp->contentBody.size());
					_log(Log::L_DEBUG, CLOGFMT(S6Client, "OnResponse() SessionGroup[%s] session_list[%s]"), groupname.c_str(), line.c_str());
					std::vector<std::string> tmpVec;	
					stringHelper::SplitString(line, tmpVec, " \t", " \t\r\n","","");
					for (size_t i = 0; i < tmpVec.size(); i++)
					{
						S6Client::Session_Pair sessPair;
						index = tmpVec[i].find(":");
						if(index != std::string::npos)
						{
							sessPair._sessionId = tmpVec[i].substr(0, index);
							sessPair._onDemandSessionId = tmpVec[i].substr(index+1, tmpVec[i].size());						
						}
						else sessPair._sessionId = tmpVec[i];

						list.push_back(sessPair);
					}

					_sessGroup.OnSessionListOfSS(list);
					break;
				} // if GET_PARAMETER

			} while(0);

			// wake up the waiting
			wakeupByCSeq(pReq->cSeq);
		}

		////////////////////////////////////
		//////Class S6Session  //////////
		///////////////////////////////////
		S6Session::S6Session(S6SessionGroup& sessGroup, std::string ODSessId)
			: RTSPSession( sessGroup._env._log, *sessGroup._env._pThreadPool, "udp://211.1.1.1", NULL, sessGroup._env._rtspTraceLevel, sessGroup._env._sessTimeout, ODSessId.c_str()),
			_sessGroup(sessGroup) 
		{
			_resultCode = 0;
			_sessGroup.add(*this);
			_log(Log::L_DEBUG, CLOGFMT(S6Session, "ODSess[%s] instantized in SessionGroup[%s]"), _sessGuid.c_str(), getSessionGroupName().c_str());
		}
		S6Session::~S6Session()
		{
			destroy();
			_log(Log::L_DEBUG, CLOGFMT(S6Session, "~S6Session() ODSess[%s]"), _sessGuid.c_str());
		}
		void S6Session::destroy()
		{
			_sessGroup.remove(*this);
			RTSPSession::destroy();
		}

		void S6Session::setSessionId(std::string sessionId) 
		{
			_sessionId = sessionId;
			_sessGroup.updateIndex(*this);
		}

		std::string S6Session::getBaseURL()
		{
			return _sessGroup.getBaseURL();
		}
		std::string	S6Session::getSessionGroupName() const 
		{
			return  _sessGroup.getName();
		}
		S6Client* S6Session::getS6Client()
		{
			return _sessGroup.getS6Client();
		}
		void S6Session::OnResponse_SETUP(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
		{
			RTSPSession::OnResponse_SETUP(rtspClient, pReq, pResp, resultCode, resultString);
			_log(Log::L_DEBUG, CLOGFMT(S6Session, "Session[%s, %s, %d, %s] OnResponse_SETUP(), return [%d,%s]"), _sessGuid.c_str(), _sessionId.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);

			uint32 respCSeq = pResp->cSeq;

			_resultCode = resultCode;
			_errorMsg = std::string(resultString);
			///setp1.1 if the response code != 200, 说明资源分配没有成功
			if(resultCode != RTSPSink::rcOK)
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(S6Session, "OnResponse_SETUP() CSeq[%d] fail to create allocation object with error[%s]") ,respCSeq, resultString);
				return;
			}

			RTSPMessage::AttrMap::const_iterator itorHeaders;

			itorHeaders = pResp->headers.find(NGOD_HEADER_SESSION);
			if(itorHeaders == pResp->headers.end())
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(S6Session, "OnResponse_SETUP() CSeq[%d] missed 'Session' header"), respCSeq);
				return;
			}
			std::string respSessionID = itorHeaders->second;

			_sessionId = respSessionID;

			itorHeaders = pResp->headers.find(NGOD_HEADER_ONDEMANDSESSIONID);
			if(itorHeaders == pResp->headers.end())
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(S6Session, "OnResponse_SETUP() CSeq[%d] missed 'OnDemandSessionId' header"), respCSeq);
				return;
			}
			std::string respOnDemandSession = itorHeaders->second;

			itorHeaders = pResp->headers.find(NGOD_HEADER_TRANSPORT);
			if(itorHeaders == pResp->headers.end())
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(S6Session, "OnResponse_SETUP() CSeq[%d] missed 'Transport' header"), respCSeq);
				return;
			}
			std::string respTransPort = itorHeaders->second;

			Ice::Long lStart = ZQTianShan::now();

			_sessGroup.updateIndex(*this);

			///setp1.2 get related resource from transport
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(S6Session, "OnResponse_SETUP() CSeq[%d] sessionID[%s]"),respCSeq, respSessionID.c_str());		
			::TianShanIce::StrValues  transportList;
			::ZQ::common::stringHelper::SplitString(respTransPort, transportList, ",\r\n");
			if (transportList.empty())
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(S6Session, "OnResponse_SETUP() CSeq[%d] sessionID[%s] the empty transport"), respCSeq, respSessionID.c_str());
				_resultCode = 0;
				return ;
			}
			for (::TianShanIce::StrValues::iterator iter = transportList.begin(); iter != transportList.end(); iter++)
			{
				::TianShanIce::StrValues transportContent;
				::ZQ::common::stringHelper::SplitString(*iter, transportContent, ";");
				_qamInfo.symbolRate = 0;

				for (::TianShanIce::StrValues::iterator contentIter = transportContent.begin(); contentIter != transportContent.end(); contentIter++)
				{
					::TianShanIce::StrValues content;
					content = ::ZQ::common::stringHelper::split(*contentIter, '=');
                    
					//not xx=xx
					if (content.size() < 2)
						continue;
					else if (content[0] == "client")
						_qamInfo.client = content[1];
					else if (content[0] == "qam_destination")
					{
						std::string dest = content[1];
						std::string::size_type npos = dest.find('.');
						if(npos != std::string::npos)
						{
							int nLen = dest.size();
							_qamInfo.channelId = atol((dest.substr(0, npos)).c_str());
							_qamInfo.PnId = atol((dest.substr(npos + 1, nLen - npos - 1)).c_str());
						}	
					}
					else if(content[0] == "destination")
						_qamInfo.edgeDeviceIP = content[1];
					else if (content[0] == "client_port")
						_qamInfo.destPort = atoi(content[1].c_str());
					else if (content[0] == "qam_name")
						_qamInfo.edgeDeviceName = content[1];
					else if (content[0] == "qam_group")
						_qamInfo.qam_group = content[1];
					else if (content[0] == "modulation")
						_qamInfo.modulationFormat = (modulationStr2Int(content[1]));
					else if (content[0] == "edge_input_group")
						_qamInfo.qam_input_group = content[1];
					else if (content[0] == "symbolRate")
						_qamInfo.symbolRate = atoi(content[1].c_str());
					else if (content[0] == "qam_mac")
						_qamInfo.edgeDeviceMac = content[1];
					else if(content[0] == "qam_zone")
						_qamInfo.edgeDeviceZone = content[1];

				}
			}
		}
		void S6Session::OnResponse_TEARDOWN(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
		{
//			_log(Log::L_DEBUG, CLOGFMT(ERMISession, "Session[%s, %d, %s] OnResponse_TEARDOWN(), return [%d,%s]"), _sessGuid.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);
//			_resultCode = resultCode;

			Ice::Long lStart = ZQTianShan::now();

			_resultCode = resultCode;
			_errorMsg = resultString;

			uint32 respCSeq = pResp->cSeq;
			RTSPMessage::AttrMap::const_iterator itorHeaders;
			itorHeaders = pResp->headers.find(NGOD_HEADER_SESSION);
			if(itorHeaders == pResp->headers.end())
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(S6Session, "OnResponse_TEARDOWN() CSeq[%d] missed 'Session' header"), respCSeq);
				return;
			}
			std::string respSessionID = itorHeaders->second;

			itorHeaders = pResp->headers.find(NGOD_HEADER_ONDEMANDSESSIONID);
			if(itorHeaders == pResp->headers.end())
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(S6Session, "OnResponse_TEARDOWN() CSeq[%d] missed 'OnDemandSessionId' header"), respCSeq);
				return;
			}
			std::string respOnDemandSession = itorHeaders->second;

			///setp1.1 if the response code != 200
			if(resultCode != 200)
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(S6Session, "OnResponse_TEARDOWN() CSeq[%d] sessionID[%s] OnDemandSession[%s] failed to teardown session with reason[%s] took %d ms") , respCSeq, respSessionID.c_str(), respOnDemandSession.c_str(), resultString, (int)(ZQTianShan::now() - lStart));
			}
			else
			{
				_log(ZQ::common::Log::L_INFO, CLOGFMT(S6Session, "OnResponse_TEARDOWN() CSeq[%d] sessionID[%s] OnDemandSession[%s] teardown session successfully took %d ms"), respCSeq, respSessionID.c_str(), respOnDemandSession.c_str(), (int)(ZQTianShan::now() - lStart));
			}
		}

		void S6Session::OnResponse_SET_PARAMETER(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
		{
			_log(Log::L_DEBUG, CLOGFMT(S6Session, "Session[%s, %d, %s] OnResponse_SET_PARAMETER(), return [%d,%s]"), _sessGuid.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);
			return RTSPSession::OnResponse_SET_PARAMETER(rtspClient, pReq, pResp, resultCode, resultString);
		}

		void S6Session::OnResponse_GET_PARAMETER(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
		{
			_log(Log::L_DEBUG, CLOGFMT(S6Session, "Session[%s, %d, %s] OnResponse_GET_PARAMETER(), return [%d,%s]"), _sessGuid.c_str(), pResp->cSeq, getSessionGroupName().c_str(), resultCode, resultString);
			_resultCode = resultCode;

			if(resultCode == RTSPSink::rcSessNotFound)
			{
				_sessGroup._env.removeOnDemandSession(_sessGuid);
				destroy();
			}
		}
		void S6Session::OnANNOUNCE(RTSPClient& rtspClient, RTSPMessage::Ptr& pInMessage)
		{
			_log(Log::L_DEBUG, CLOGFMT(S6Session, "Session[%s, %s] OnANNOUNCE()"), _sessGuid.c_str(), getSessionGroupName().c_str());
			if (pInMessage->headers.end() == pInMessage->headers.find("Session"))
				return;

			std::string notice = "";
			if (pInMessage->headers.find("Notice") != pInMessage->headers.end())
				notice = pInMessage->headers["Notice"];

			TianShanIce::Properties props;

			std::vector<std::string> paramTokens;
			size_t i =0;
			int AnnounceCode = -1;

			// processing Notice
			_log(Log::L_DEBUG, CLOGFMT(S6Session, "OnANNOUNCE(), Session[%s, %d] Notice[%s]"), pInMessage->headers["Session"].c_str(), pInMessage->cSeq, notice.c_str());
			ZQ::common::stringHelper::SplitString(notice, paramTokens, " ", " ");
			if (paramTokens.size() >0)
				AnnounceCode = atoi(paramTokens[0].c_str());

			for (i = 0; i < paramTokens.size(); i++)
			{
				std::string::size_type pos_begin = std::string::npos;
				if((pos_begin=paramTokens[i].find("="))!= std::string::npos)				
				{
					::std::string strKey   = paramTokens[i].substr(0, pos_begin);
					::std::string strValue = paramTokens[i].substr(pos_begin+1);
					_log(Log::L_DEBUG, CLOGFMT(S6Session, "Session[%s] OnANNOUNCE(), get content %s[%s]"), _sessGuid.c_str(), strKey.c_str(), strValue.c_str());
				}
			}

			switch (AnnounceCode)
			{
//			case RTSPSink::racClientSessionTerminated:
//				destroy();
//				break;
			case RTSPSink::racSessionInProgress:
				{
					S6Client* client = _sessGroup.getS6Client();
					if(client)
					{
						int cseq =0;
						if (pInMessage->headers.find("CSeq") != pInMessage->headers.end())
							cseq = atoi(pInMessage->headers["CSeq"].c_str());

						RTSPMessage::AttrMap headers;
						MAPSET(RTSPMessage::AttrMap, headers, "OnDemandSessionId", getOnDemandSessionId());

						if (pInMessage->headers.find("Session") != pInMessage->headers.end())
							MAPSET(RTSPMessage::AttrMap, headers, "Session", pInMessage->headers["Session"]);

						MAPSET(RTSPMessage::AttrMap, headers, "Require", "com.comcast.ngod.s6");

						client->sendTousyMsg("RTSP/1.0 200 OK", headers, "", cseq);
					}
				}
				break;
			default:
				break;
			}
		}

		void S6Session::OnSessionTimer()
		{
			S6Client* client = _sessGroup.getS6Client(); 
			if (client)
			{
				if(_sessionId.empty())
				{
					destroy();
					return;
				}

				RTSPRequest::AttrList parameterNames;
				RTSPMessage::AttrMap headers;
				MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_CONTENTTYPE,      "text/parameters");
				MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_SESSION, _sessionId.c_str());
				MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_ONDEMANDSESSIONID, _sessGuid.c_str());
				MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_REQUIRE, "com.comcast.ngod.s6");
				client->sendGET_PARAMETER(*this, parameterNames, NULL,    headers);
				_log(Log::L_DEBUG, CLOGFMT(S6Session, "Session[%s] OnDemandSession[%s]OnSessionTimer(), trigger PING"), _sessionId.c_str(), _sessGuid.c_str());
			}
		}

		void S6Session::OnResponse(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString)
		{
			RTSPSession::OnResponse(rtspClient, pReq, pResp, resultCode, resultString); // do the dispatching
			if (pReq)
				((S6Client*) &rtspClient)->wakeupByCSeq(pReq->cSeq);
		}

		void S6Session::OnRequestError(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RequestError errCode, const char* errDesc)
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

			((S6Client*) &rtspClient)->wakeupByCSeq(pReq->cSeq);
		}
		// -----------------------------
		// class S6SessionGroup
		// -----------------------------
		S6SessionGroup::SessionGroupMap S6SessionGroup::_groupMap;
		S6SessionGroup::StringIndex     S6SessionGroup::_idxGroupBaseUrl;
//		S6SessionGroup::StringIndex     S6SessionGroup::_idxGroupStreamLinkId;
		Mutex S6SessionGroup::_lockGroups;

		S6SessionGroup::S6SessionGroup(PhoEdgeRMEnv& env, const std::string& name, const std::string& baseURL, int max, Ice::Long syncInterval)
			: _env(env), _name(name), _baseURL(baseURL), _max(max), _status(Idle), _syncInterval(syncInterval), _stampLastSync(0),
			_S6Client(*this, env._log, *env._pThreadPool, env._bindAddr, baseURL, env._userAgent.c_str(), env._rtspTraceLevel)  
		{
			_S6Client.setClientTimeout(_cfg.connectTimeout, _cfg.requestTimeOut);
			_idxGroupBaseUrl.insert(StringIndex::value_type(_baseURL, name));
//			_idxGroupStreamLinkId.insert(StringIndex::value_type(streamLinkId, name));
		}

#define _log        (_env._log)
		S6SessionGroup::~S6SessionGroup()
		{
			try
			{
				MutexGuard g(_lockSessMap);
				_log(Log::L_DEBUG, CLOGFMT(S6SessionGroup, "~S6SessionGroup()[%s] baseURL[%s] SessionCount[%d]"), _name.c_str(), _baseURL.c_str(), _sessMap.size());

/*				S6SessionGroup::SessionMap::iterator itor = _sessMap.begin();
				while(itor != _sessMap.end())
				{
					itor->second = NULL;
					itor++;
				}*/
				S6SessionGroup::SessionMap emptySessionMap;
				_sessMap.swap(emptySessionMap);

				_sessMap.clear();

				_idxGroupBaseUrl.erase(_baseURL);
				//			_idxGroupStreamLinkId.erase(_streamLinkId);
			}
			catch (...)
			{
			}
			_log(Log::L_DEBUG, CLOGFMT(S6SessionGroup, "~S6SessionGroup()[%s] baseURL[%s]"), _name.c_str(), _baseURL.c_str());
		}

		void S6SessionGroup::add(S6Session& sess)
		{
			MutexGuard g(_lockSessMap);
			MAPSET(SessionMap, _sessMap, sess._sessGuid, &sess);

			if (!sess._sessionId.empty())
				_sessIdIndex.insert(StringIndex::value_type(sess._sessionId, sess._sessGuid));
		}

		void S6SessionGroup::remove(S6Session& sess)
		{
			if (!sess._sessionId.empty())
				eraseSessionId(sess._sessionId, sess);

			MutexGuard g(_lockSessMap);
			_sessMap.erase(sess._sessGuid);
		}

		void S6SessionGroup::updateIndex(S6Session& sess, const char* indexName, const char* oldValue)
		{
			std::string oldSessId = oldValue? oldValue:sess._sessionId;

			if (!oldSessId.empty())
				eraseSessionId(oldSessId, sess);

			MutexGuard g(_lockSessMap);
			_sessIdIndex.insert(StringIndex::value_type(sess._sessionId, sess._sessGuid));
		}

		S6Session::List S6SessionGroup::lookupByIndex(const char* sessionId, const char* indexName)
		{
			S6Session::List list;

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
		S6Session::Ptr S6SessionGroup::lookupByOnDemandSessionId(const char* sessOnDemandId)
		{
			if (NULL == sessOnDemandId)
				return NULL;

			MutexGuard g(_lockSessMap);
			SessionMap::iterator it = _sessMap.find(sessOnDemandId);
			if (_sessMap.end() ==it)
				return NULL;

			return it->second;
		}

		void S6SessionGroup::eraseSessionId(const std::string& sessionId, S6Session& sess)
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
		S6Client* S6SessionGroup::getS6Client()
		{ 
			return &_S6Client;
		}

		int S6SessionGroup::sync(bool bOnConnected)
		{
			_log(Log::L_DEBUG, CLOGFMT(S6SessionGroup, "SessionGroup[%s] sync() per %s"), _name.c_str(), bOnConnected ? "connected" :"timer");
			if (!bOnConnected && Idle != _status)
			{
				_log(Log::L_DEBUG, CLOGFMT(S6SessionGroup, "SessionGroup[%s] is already in sync"), _name.c_str());
				return 0;
			}

			if (bOnConnected && Socket::stConnected != _S6Client.state())
				return -1;

			//set group status
			RTSPRequest::AttrList parameterNames;
			parameterNames.push_back("session_list");
			//	parameterNames.push_back("connection_timeout");
			RTSPMessage::AttrMap headers;
			headers.insert(std::make_pair("SessionGroup", _name));
			headers.insert(std::make_pair("Require",      "com.comcast.ngod.s6"));

			return _S6Client.sendGET_PARAMETER(parameterNames, NULL, headers);
		}

		void S6SessionGroup::OnSessionListOfSS(const std::vector<S6Client::Session_Pair>& listOnSS)
		{
			_log(Log::L_DEBUG, CLOGFMT(S6SessionGroup, "OnSessionListOfSS() SessionGroup[%s] %d sessions on SS"), _name.c_str(), listOnSS.size());

			int64 stampStart= ZQ::common::now();
			size_t cSSOrphan =0, cSynced=0, cTorndown=0;

			try {

				{
					MutexGuard g(_lockSessMap);
					for (std::vector<S6Client::Session_Pair>::const_iterator iter = listOnSS.begin();	iter != listOnSS.end(); iter++)
					{
						std::string tmpStr = iter->_onDemandSessionId + ":" + iter->_sessionId;
						bool bFound = false;

						// find session by key
						if (!iter->_onDemandSessionId.empty() && _sessMap.end() != _sessMap.find(iter->_onDemandSessionId))
							bFound = true;
						else if (!iter->_sessionId.empty() && _sessIdIndex.end() != _sessIdIndex.find(iter->_sessionId))
							bFound = true;
						else if(_env.syncS6Session(iter->_sessionId,iter->_onDemandSessionId, _name))
							bFound = true;

						if (bFound)
						{
							cSynced ++;
							//					_log(Log::L_DEBUG, CLOGFMT(S6SessionGroup, "OnSessionListOfSS() SessionGroup[%s] session[%s] sync-ed"), _name.c_str(), tmpStr.c_str());
							continue;
						}

						cSSOrphan ++;
						_log(Log::L_INFO, CLOGFMT(S6SessionGroup, "OnSessionListOfSS() SessionGroup[%s] SS-orphan[%s] found, tearing it down"), _name.c_str(), tmpStr.c_str());

						//TODO: remove session from media server by TEARDOWN


						RTSPMessage::AttrMap headers;
						MAPSET(RTSPMessage::AttrMap, headers, "Content-Type",      "text/parameters");
						MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_REQUIRE, "com.comcast.ngod.s6");
						MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_SESSION, iter->_sessionId);
						if (!iter->_onDemandSessionId.empty())
							MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_ONDEMANDSESSIONID, iter->_onDemandSessionId);

						MAPSET(RTSPMessage::AttrMap, headers, NGOD_HEADER_REASON, "428 Session Orphan Detected on PHO");

						_S6Client.sendTousyMsg("TEARDOWN * RTSP/1.0", headers);

						cTorndown++;
					}
				}
			}
			catch(...)
			{
			}

			setLastSync(now(), Idle);
			_log(Log::L_INFO, CLOGFMT(S6SessionGroup, "syncSessionList() SessionGroup[%s] done, took %lldmsec: %d sync-ed, %d SS-orphans found, %d torndown"), _name.c_str(), ZQ::common::now() -stampStart, cSynced, cSSOrphan, cTorndown);
		}

		void S6SessionGroup::OnTimer(const ::Ice::Current& c)
		{
			int64 stampNow = now();
			if (getLastSync() + getSyncInterval() > stampNow) // do not need sync now
				return;

			sync();
			setLastSync(stampNow, Sync);
		}

		S6Session::Ptr S6SessionGroup::createSession(const std::string& ODSessId, const char* baseURL, const std::string& streamLinkId)
		{
			S6SessionGroup::Ptr pSessGroup = NULL;

			if (ODSessId.empty())
				return NULL;

			size_t count = 0;

//			std::vector<std::string> gnlist = getSessionGroupNames(streamLinkId);
            std::vector<std::string> gnlist = getSessionGroupNames(baseURL);

			int startIdx = rand(); 
			count = gnlist.size();

			for (size_t i = 0; i< count; i++)
			{
				std::string& groupName = gnlist[(startIdx +i) %count];
				if (_groupMap.end() == _groupMap.find(groupName))
					continue;

				S6SessionGroup::Ptr pGroup = _groupMap[groupName];
				if (NULL == pGroup || pGroup->size() >= (size_t) pGroup->getMaxSize())
					continue;

				pSessGroup = pGroup;
				(*phoErmLog)(Log::L_DEBUG, CLOGFMT(S6SessionGroup, "createSession()StreamLink[%s] SessionGroup[%s] selected for session[%s]"),streamLinkId.c_str(), pSessGroup->getName().c_str(), ODSessId.c_str());
				break;
			}

			if (NULL == pSessGroup)
			{
				(*phoErmLog)(Log::L_WARNING, CLOGFMT(S6SessionGroup, "createSession()StreamLink[%s] session[%s]: none of %d valid SessionGroup(s) has quota available"),streamLinkId.c_str(), ODSessId.c_str(), count);
				return NULL;
			}

			//	S6SessionGroup& group, const std::string& ODSessId, const char* streamDestUrl, const char* filePath
			return (new S6Session(*pSessGroup, ODSessId));
		}

		S6Session::Ptr S6SessionGroup::openSession(const std::string& ODSessId, const std::string& groupName, bool bRestore)
		{
			S6SessionGroup::Ptr pSessGroup = S6SessionGroup::findSessionGroup(groupName);
			if (NULL == pSessGroup)
				return NULL;

			S6Session::Ptr clientSession = pSessGroup->lookupByOnDemandSessionId(ODSessId.c_str());

			if (NULL != clientSession)
				return clientSession;

			if (!bRestore)
				return NULL;

			return (new S6Session(*pSessGroup, ODSessId));
		}


		S6SessionGroup::Ptr S6SessionGroup::findSessionGroup(const std::string& name)
		{
			S6SessionGroup::Ptr group;

			MutexGuard g(_lockGroups);
			SessionGroupMap::iterator it = _groupMap.find(name);
			if (_groupMap.end() !=it)
				group = it->second;

			return group;
		}

		std::vector<std::string> S6SessionGroup::getSessionGroupNames(const std::string baseURL)
		{
			std::vector<std::string> gnlist;

			MutexGuard g(_lockGroups);
			StringIdxRange range = _idxGroupBaseUrl.equal_range(baseURL);
			for (StringIndex::iterator it = range.first; it != range.second; it++)
				gnlist.push_back(it->second);

			return gnlist;
		}

/*		std::vector<std::string> S6SessionGroup::getSessionGroupNames(const std::string& streamLinkId)
		{
			std::vector<std::string> gnlist;

			MutexGuard g(_lockGroups);
			StringIdxRange range = _idxGroupStreamLinkId.equal_range(streamLinkId);
			for (StringIndex::iterator it = range.first; it != range.second; it++)
				gnlist.push_back(it->second);

			return gnlist;
		}
*/
		std::vector<std::string> S6SessionGroup::getAllSessionGroup()
		{
			MutexGuard g(_lockGroups);
			std::vector<std::string> result;

			for (SessionGroupMap::iterator it = _groupMap.begin(); it != _groupMap.end(); it++)
				result.push_back(it->first);

			return result;
		}

		void S6SessionGroup::clearSessionGroup()
		{
			int64 stampNow = now();

			try
			{
				MutexGuard g(_lockGroups);
				(*phoErmLog)(Log::L_DEBUG, CLOGFMT(S6SessionGroup, "clear SessionGroup[%d] _idxGroupBaseUrl[%d]"), _groupMap.size(), _idxGroupBaseUrl.size());

/*				S6SessionGroup::SessionGroupMap::iterator itor = _groupMap.begin();
				while(itor != _groupMap.end())
				{
					itor->second = NULL;
					++itor;
				}
*/
				S6SessionGroup::SessionGroupMap emptyGroup;
				_groupMap.swap(emptyGroup);
				_groupMap.clear();
				_idxGroupBaseUrl.clear();
			}
			catch (...)
			{
				
			}
			(*phoErmLog)(Log::L_INFO, CLOGFMT(S6SessionGroup, "clear SessionGroups took %d ms"), (int)(now() - stampNow));
		}

		Ice::Long S6SessionGroup::checkAll()
		{
			std::vector<S6SessionGroup::Ptr> groupsToOntimer;
			::Ice::Long stampNow = now();
			::Ice::Long _nextWakeup = stampNow + 1000;

			{
				MutexGuard g(_lockGroups);

				for (SessionGroupMap::iterator iter = _groupMap.begin(); iter != _groupMap.end(); iter++)
				{
					try
					{
						S6SessionGroup::Ptr group = iter->second;
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

			for (std::vector<S6SessionGroup::Ptr>::iterator iter2 = groupsToOntimer.begin(); iter2 != groupsToOntimer.end(); iter2++)
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
	} // end namespace EdgeRM
}// end namespace ZQTianShan
