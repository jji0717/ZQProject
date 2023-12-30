#include "ngod_parse_thread.h"
#include "ngod_send_threadreq.h"
#include "ngod_rtsp_parser/RTSPMessage/RTSPMessageParser.h"

#define MYLOG (*m_pLogFile)

ngod_parse_thread::ngod_parse_thread(ZQ::common::FileLog *logfile, 
									 ZQ::common::NativeThreadPool &pool,
									 ::ZQTianShan::NSS::NssEventList &eventList,
									 StreamStrList &streamStrList)
:m_NSSSessionGroupList(NULL),
m_pPool(&pool),
m_pLogFile(logfile),
_eventList(eventList),
_streamStrList(streamStrList)
{
}

ngod_parse_thread::ngod_parse_thread(ZQ::common::FileLog *logfile, 
									 ZQ::common::NativeThreadPool &pool,
									 ::ZQTianShan::NSS::NssEventList &eventList,
									 NSSSessionGroupList &_NSSSessionGroupList,
									 StreamStrList &streamStrList)
:m_NSSSessionGroupList(&_NSSSessionGroupList),
m_pPool(&pool),
m_pLogFile(logfile),
_eventList(eventList),
_streamStrList(streamStrList)
{
}

ngod_parse_thread::~ngod_parse_thread()
{
	cout << "ngod_parse_thread exit" << endl;
	m_NSSSessionGroupList = NULL;
	m_pPool = NULL;
	m_pLogFile = NULL;
}

void ngod_parse_thread::setNSSSessionGroupList(NSSSessionGroupList &_NSSSessionGroupList)
{
	m_NSSSessionGroupList = &_NSSSessionGroupList;
}

int ngod_parse_thread::run(void)
{
	//cout << "parse thread start" << endl;
	bool bSessionStatus = false;
#ifdef NGODPARSEDEBUG
	//FILE *f = fopen("message.txt", "wb");
#endif
	while (1)
	{
		while (m_NSSSessionGroupList == NULL)
			Sleep(1000);

		//check the socket in this list
		if (m_NSSSessionGroupList->size() > 0)
		{
			for (NSSSessionGroupList::iterator iter = m_NSSSessionGroupList->begin();
			iter != m_NSSSessionGroupList->end(); iter++)
			{
				//define a pointer for convenient use
				NSSRTSPMessageList *pMessList = &((*iter)->m_NSSRTSPMessageList);

				//parse the received message from list
				while (1)
				{
					//no message in list
					if (pMessList->m_MessageList.empty())
						break;

					//get the first data
					::std::string queMessage = pMessList->First();
					//pop the first message
					pMessList->PopFront();

					//strlist::iterator queiter = pMessList->m_MessageList.begin();
					string *queiter = &queMessage;

					MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ngod_parse_thread,"begin parse one message"));
					if (m_pLogFile)
					m_pLogFile->hexDump(ZQ::common::Log::L_DEBUG, queMessage.c_str(), queMessage.length());
					

#ifdef NGODPARSEDEBUG
			const char *tmpstr = (*queiter).c_str();
#endif
					SessionMapKey key;
					RTSPMessageParser::GetOnDemandSessionID((*queiter).c_str(), (*queiter).length(), key.strOnDemandSessionId);
					if (key.strOnDemandSessionId.length() == 0)
						RTSPMessageParser::GetSessionID((*queiter).c_str(), (*queiter).length(), key.strSessionId);
					uint16 usContLen = RTSPMessageParser::GetContentLength((*queiter).c_str(), (*queiter).length());

					uint16 index = 0;
					RTSPMessageParser::GetSequence((*queiter).c_str(), (*queiter).length(), index);

					//check if announce message
					bool b = RTSPMessageParser::CheckAnnounceMessage((*queiter).c_str(), (*queiter).length());

					//find session by key
					EnterCriticalSection(&(*iter)->m_CS_SessionMap);
					NSSSessionMap::iterator findIter;
					NSSSessionMap *pMap = NULL;
					
					if (!key.strOnDemandSessionId.empty())
					{
						findIter = (*iter)->m_NSSOnDemandSessionMap.find(key.strOnDemandSessionId);
						pMap = &(*iter)->m_NSSOnDemandSessionMap;
					}
					else if (!key.strSessionId.empty())
					{
						findIter = (*iter)->m_NSSSessionMap.find(key.strSessionId);
						pMap = &(*iter)->m_NSSSessionMap;
					}
					else
					{
						findIter = (*iter)->m_NSSSessionMap.end();
						pMap = &(*iter)->m_NSSSessionMap;
					}

					//NSSSessionMap::iterator findIter = find((*iter)->m_NSSSessionMap.begin(), (*iter)->m_NSSSessionMap.end(), key);
					if (findIter == pMap->end())
					{
						LeaveCriticalSection(&(*iter)->m_CS_SessionMap);
						//response announce
						//this status means the session not in our session list, we should tell Media Cluster to delete the session
						if (b == true)
						{
							RTSPNoticeHeader pHeader;
							RTSPMessageParser::AnnouceNoticeParser((*queiter).c_str(), (*queiter).length(),  pHeader);
							if (pHeader.strNotice_code.compare(strInProgress) == 0)
							{
								RTSPClientSession *tmpSess = new RTSPClientSession();
								tmpSess->iRTSPClientState = TEARDOWN;
								EnterCriticalSection(&(*iter)->m_CS_ClientSeq);
								tmpSess->uServerCSeq = (*iter)->usClientSeq++;
								LeaveCriticalSection(&(*iter)->m_CS_ClientSeq);
								tmpSess->m_pCS = &((*iter)->m_CS);
								tmpSess->m_RTSPR2Header.strOnDemandSessionID = key.strOnDemandSessionId;
								tmpSess->m_RTSPR2Header.strSessionID = key.strSessionId;
								tmpSess->iRTSPSessionState = RTSPSessionState(454);
								tmpSess->RTSPSocket = &(*iter)->m_SessionSocket.m_Socket;

								//create thread pool request
								ngod_send_threadreq* req = new ngod_send_threadreq(m_pLogFile, *m_pPool, tmpSess, true);
								req->start();
							}
						}
						else
						{
							if (RTSPMessageParser::CheckGetParameterMessage((*queiter).c_str(), (*queiter).length()) && (*iter)->m_SessionGroupStatus.getStatus() == Sync)
							{
								RTSPClientSession tmpSess;
								//try to sync session_list
								int ret = RTSPMessageParser::MessageParser((*queiter).c_str(), 
											(*queiter).length(),
											tmpSess);

								//sync the session list
								syncSessionList(*iter, tmpSess.m_RTSPR2Header.m_GetPramameterRes_ExtHeader);
								//clean the session list
								tmpSess.m_RTSPR2Header.m_GetPramameterRes_ExtHeader.vstrSession_list.clear();
								(*iter)->m_SessionGroupStatus.setStatus(Idle);
							}
							else
							{
								//nothing to do here
								MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ngod_parse_thread,"parse(not find session[%s:%s])"), key.strOnDemandSessionId.c_str(), key.strSessionId.c_str());
							}
						}//check if announce
					}
					else//get session and process the response
					{
						RTSPClientSession *pSession = NULL;
						
						if ((*findIter).second != NULL)
						{
							pSession = (*findIter).second;
						
							//response announce
							if (b == true)
							{
								RTSPNoticeHeader ptmpNoticeHeader;
								int16 ret = RTSPMessageParser::AnnouceNoticeParser((*queiter).c_str(), 
								(*queiter).length(),
								ptmpNoticeHeader);
								RTSPMessageParser::GetSequence((*queiter).c_str(), (*queiter).length(), pSession->uServerCSeq);
								if (ptmpNoticeHeader.strNotice_code.compare(strInProgress) == 0)
								{
									EnterCriticalSection(&(*iter)->m_CS_ServerSeq);
									pSession->iRTSPClientState = ANNOUNCE;
									//pSession->uServerCSeq = (*iter)->usServerSeq++;
									LeaveCriticalSection(&(*iter)->m_CS_ServerSeq);
									pSession->iRTSPSessionState = (RTSPSessionState)200;
									ngod_send_threadreq* req = new ngod_send_threadreq(m_pLogFile, *m_pPool, pSession, false);
									req->start();
								}
								else if (ptmpNoticeHeader.strNotice_code.compare(strEndOfStream) == 0)
								{
									//TODO: event sink, end of stream
									::ZQTianShan::NSS::listmem params;
									params.type = ::ZQTianShan::NSS::E_PLAYLIST_END;
									params.param[EventField_PlaylistGuid] = pSession->m_RTSPR2Header.strOnDemandSessionID;
									params.param[EventField_EventCSEQ] = (long)pSession->uClientCSeq;
									_eventList.PushBack(params);
								}
								else if (ptmpNoticeHeader.strNotice_code.compare(strStartOfStream) == 0)
								{
									//TODO: event sink, start of stream
									::ZQTianShan::NSS::listmem params;
									params.type = ::ZQTianShan::NSS::E_PLAYLIST_BEGIN;
									params.param[EventField_PlaylistGuid] = pSession->m_RTSPR2Header.strOnDemandSessionID;
									params.param[EventField_EventCSEQ] = (long)pSession->uClientCSeq;
									_eventList.PushBack(params);
								}
							}
							else
							{
								//get the session state
								if (pSession != NULL)
								{
									int16 ret = RTSPMessageParser::MessageParser((*queiter).c_str(), (*queiter).length(), *pSession);
								
									pSession->iRTSPSessionState = (RTSPSessionState)ret;
									//SetEvent(*(pSession->m_pEventHandle));
									pSession->m_pEventHandle.m_SetEvent(index);
								}						
							}//check if announce
						}
						MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ngod_parse_thread,"parse(find session[%s:%s])"), key.strOnDemandSessionId.c_str(), key.strSessionId.c_str());
						LeaveCriticalSection(&(*iter)->m_CS_SessionMap);
					}
					//parse over
				}//while(1)				
			}//for(...)
			if (!bSessionStatus)
					Sleep(10);
		}//if
	}
#ifdef NGODPARSEDEBUG
		//fclose(f);
#endif
	return 1;
}

void ngod_parse_thread::syncSessionList(NSSSessionGroup *pGroup, 
										GetPramameterRes_ExtHeader &pHead)
{
	for (vector<Session_list>::iterator iter = pHead.vstrSession_list.begin();
	iter != pHead.vstrSession_list.end();
	iter++)
	{
		SessionMapKey key;
		NSSSessionMap *pMap = NULL;
		key.strOnDemandSessionId = (*iter).strOnDemandSessionID;
		key.strSessionId = (*iter).strRTSPSessionID;

		//find session by key
		NSSSessionMap::iterator findIter;
		if (key.strOnDemandSessionId.length() > 0)
		{
			findIter = pGroup->m_NSSOnDemandSessionMap.find(key.strOnDemandSessionId);
			pMap = &(pGroup->m_NSSOnDemandSessionMap);
		}
		else if (key.strSessionId.length() > 0)
		{
			findIter = pGroup->m_NSSSessionMap.find(key.strSessionId);
			pMap = &(pGroup->m_NSSSessionMap);
		}
		else
		{
			findIter = pGroup->m_NSSSessionMap.end();
			pMap = &(pGroup->m_NSSSessionMap);
		}

		if (findIter == pMap->end())
		{
			string tmpStr = key.strOnDemandSessionId + ":" + key.strSessionId;
			MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ngod_parse_thread,"not find by key %s"), tmpStr.c_str());
			//TODO: remove session from media server by TEARDOWN
			RTSPClientSession *tmpSess = new RTSPClientSession();
			tmpSess->iRTSPClientState = TEARDOWN;
			EnterCriticalSection(&pGroup->m_CS_ClientSeq);
			tmpSess->uClientCSeq = pGroup->usClientSeq++;
			tmpSess->uServerCSeq = pGroup->usServerSeq++;
			LeaveCriticalSection(&pGroup->m_CS_ClientSeq);
			tmpSess->m_pCS = &pGroup->m_CS;
			tmpSess->m_RTSPR2Header.strOnDemandSessionID = (*iter).strOnDemandSessionID;
			tmpSess->m_RTSPR2Header.strSessionID = (*iter).strRTSPSessionID;
			tmpSess->m_RTSPR2Header.m_SDPResponseContent.strHost = pGroup->strServerPath;
			tmpSess->m_RTSPR2Header.m_SDPResponseContent.strProtocol = "rtsp";
			tmpSess->m_RTSPR2Header.m_SDPResponseContent.uPort = pGroup->uServerPort;
			tmpSess->RTSPSocket = &pGroup->m_SessionSocket.m_Socket;
			
			//create thread pool request
			ngod_send_threadreq* req = new ngod_send_threadreq(m_pLogFile, *m_pPool, tmpSess, true);
			req->start();
		}
		else
		{
			string tmpStr = key.strOnDemandSessionId + ":" + key.strSessionId;
			//MYLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ngod_parse_thread,"find by key %s"), tmpStr.c_str());
		}
	}

	for (NSSSessionMap::iterator iter = pGroup->m_NSSOnDemandSessionMap.begin(); iter != pGroup->m_NSSOnDemandSessionMap.end(); iter++)
	{
		vector<Session_list>::iterator pIter = find_if(pHead.vstrSession_list.begin(), pHead.vstrSession_list.end(), FindByOnDemandSessionID((*iter).second->m_RTSPR2Header.strOnDemandSessionID));
		if (pIter == pHead.vstrSession_list.end())
		{
			//TODO:destroy this stream
			_streamStrList.PushBack((*iter).second->strStreamName);
		}
	}
}