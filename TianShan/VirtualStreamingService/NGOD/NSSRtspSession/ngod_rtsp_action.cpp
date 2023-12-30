#include "ngod_rtsp_action.h"
#include "ngod_send_threadreq.h"
#include "ngod_rtsp_parser/ClientSocket.h"

int32 iTimeOut = 0;

bool ngod_rtsp_action::SetupAction(string &strOnDemandSessionId,
								   NSSSessionGroupList &pNSSSessionGroupList,
								   ZQ::common::NativeThreadPool &pool,
								   ZQ::common::FileLog &fileLog)
{
	//find session by on demand session id
	SessionMapKey key;
	key.strOnDemandSessionId = strOnDemandSessionId;

	//try to find session and session group
	NSSSessionGroup *pNSSSessionGroup = NULL;
	RTSPClientSession *sess = ngod_rtsp_action::FindOnDemandSessIter(pNSSSessionGroupList, strOnDemandSessionId, &pNSSSessionGroup);

	if (sess == NULL)
		return false;

	//check socket status
	bool b = checkGroupSocketStatus(pNSSSessionGroup->strSessionGroup, pNSSSessionGroupList, pool, fileLog);
	if (!b)
		return false;

	EnterCriticalSection(sess->m_pCS_ClientSeq);
	sess->uClientCSeq = pNSSSessionGroup->usClientSeq++;
	uint16 index = sess->uClientCSeq;
	sess->iRTSPClientState = SETUP;
	sess->m_RTSPR2Header.Require.strComPath = NGODProvider;
	sess->m_RTSPR2Header.Require.strInterface_id = NGODR2Type;
	LeaveCriticalSection(sess->m_pCS_ClientSeq);

	//start the thread pool to send request
	//HANDLE pHandle = CreateEvent(NULL, true, false, NULL);
	if (sess->m_pEventHandle.m_Init(index) == false)
		return false;

	ngod_send_threadreq* req = new ngod_send_threadreq(&fileLog, pool, sess, false);
	req->start();
	//WaitForSingleObject(*sess->m_pEventHandle._pHandle, INFINITE);
	//long time no response
	if (waitSignal(sess, index) == false)
	{
		//CloseHandle(sess->m_pEventHandle);
		fileLog(::ZQ::common::Log::L_ERROR, CLOGFMT(ngod_rtsp_action,"session(%s) SetupAction timeout"), sess->m_RTSPR2Header.strOnDemandSessionID.c_str());
		sess->m_pEventHandle.m_CloseEvent(index);
		return false;
	}

	//update key
	//key.strSessionId = sess->m_RTSPR2Header.strSessionID;
	//pNSSSessionGroup->m_NSSSessionMap[key.strSessionId] = sess;

	sess->m_pEventHandle.m_CloseEvent(index);

	if (sess->iRTSPSessionState == RTSPOK)
		return true;
	else
		return false;
}

bool ngod_rtsp_action::PlayAction(string &strOnDemandSessionId,
								  NSSSessionGroupList &pNSSSessionGroupList,
								  ZQ::common::NativeThreadPool &pool,
								  ZQ::common::FileLog &fileLog)
{
	//find session by on demand session id
	SessionMapKey key;
	key.strOnDemandSessionId = strOnDemandSessionId;

	//try to find session and session group
	NSSSessionGroup *pNSSSessionGroup = NULL;;
	RTSPClientSession *sess = ngod_rtsp_action::FindOnDemandSessIter(pNSSSessionGroupList, strOnDemandSessionId, &pNSSSessionGroup);

	if (sess == NULL)
	{
		sess = ngod_rtsp_action::FindSessIter(pNSSSessionGroupList, strOnDemandSessionId, &pNSSSessionGroup);
		if (sess == NULL)
			return false;
	}

	//check socket status
	bool b = checkGroupSocketStatus(pNSSSessionGroup->strSessionGroup, string(sess->m_streamCtrlURL.generate()), pNSSSessionGroupList);
	//bool b = checkGroupSocketStatus(pNSSSessionGroup->strSessionGroup, pNSSSessionGroupList, pool, fileLog);
	if (!b)
		return false;

	EnterCriticalSection(sess->m_pCS_ClientSeq);
	sess->uClientCSeq = pNSSSessionGroup->usClientSeq++;
	uint16 index = sess->uClientCSeq;
	sess->iRTSPClientState = PLAY;
	sess->m_RTSPR2Header.Require.strComPath = NGODProvider;
	sess->m_RTSPR2Header.Require.strInterface_id = NGODC1Type;
	LeaveCriticalSection(sess->m_pCS_ClientSeq);

	//start the thread pool to send request
	//	HANDLE hEvent = pNSSSessionGroup->newEvent(CSeq);

	//HANDLE pHandle = CreateEvent(NULL, true, false, NULL);
	sess->m_pEventHandle.m_Init(index);

	ngod_send_threadreq* req = new ngod_send_threadreq(&fileLog, pool, sess, false);
	req->start();
	//WaitForSingleObject(*sess->m_pEventHandle._pHandle, INFINITE);
	if (waitSignal(sess, index) == false)
	{
		sess->m_pEventHandle.m_CloseEvent(index);
		fileLog(::ZQ::common::Log::L_ERROR, CLOGFMT(ngod_rtsp_action,"session(%s) PlayAction timeout"), sess->m_RTSPR2Header.strOnDemandSessionID.c_str());
		return false;
	}

	sess->m_pEventHandle.m_CloseEvent(index);

	if (sess->iRTSPSessionState == RTSPOK)
		return true;
	else
		return false;
}

bool ngod_rtsp_action::PauseAction(string &strOnDemandSessionId,
								   NSSSessionGroupList &pNSSSessionGroupList,
								   ZQ::common::NativeThreadPool &pool,
								   ZQ::common::FileLog &fileLog)
{
	//find session by on demand session id
	SessionMapKey key;
	key.strOnDemandSessionId = strOnDemandSessionId;
	
	//try to find session and session group
	NSSSessionGroup *pNSSSessionGroup = NULL;;
	RTSPClientSession *sess = ngod_rtsp_action::FindOnDemandSessIter(pNSSSessionGroupList, strOnDemandSessionId, &pNSSSessionGroup);

	if (sess == NULL)
	{
		sess = ngod_rtsp_action::FindSessIter(pNSSSessionGroupList, strOnDemandSessionId, &pNSSSessionGroup);
		if (sess == NULL)
			return false;
	}

	//check socket status
	bool b = checkGroupSocketStatus(pNSSSessionGroup->strSessionGroup, string(sess->m_streamCtrlURL.generate()), pNSSSessionGroupList);
	//bool b = checkGroupSocketStatus(pNSSSessionGroup->strSessionGroup, pNSSSessionGroupList, pool, fileLog);
	if (!b)
		return false;

	EnterCriticalSection(sess->m_pCS_ClientSeq);
	sess->uClientCSeq = pNSSSessionGroup->usClientSeq++;
	uint16 index = sess->uClientCSeq;
	sess->iRTSPClientState = PAUSE;
	sess->m_RTSPR2Header.Require.strComPath = NGODProvider;
	sess->m_RTSPR2Header.Require.strInterface_id = NGODC1Type;
	LeaveCriticalSection(sess->m_pCS_ClientSeq);

	//start the thread pool to send request
	sess->m_pEventHandle.m_Init(index);

	ngod_send_threadreq* req = new ngod_send_threadreq(&fileLog, pool, sess, false);
	req->start();

	if (waitSignal(sess, index) == false)
	{
		//CloseHandle(*sess->m_pEventHandle);
		fileLog(::ZQ::common::Log::L_ERROR, CLOGFMT(ngod_rtsp_action,"session(%s) PauseAction timeout"), sess->m_RTSPR2Header.strOnDemandSessionID.c_str());
		sess->m_pEventHandle.m_CloseEvent(index);
		return false;
	}

	//CloseHandle(*sess->m_pEventHandle);
	sess->m_pEventHandle.m_CloseEvent(index);

	if (sess->iRTSPSessionState == RTSPOK)
		return true;
	else
		return false;
}

bool ngod_rtsp_action::TeardownAction(string &strOnDemandSessionId,
								      NSSSessionGroupList &pNSSSessionGroupList,
								      ZQ::common::NativeThreadPool &pool,
								      ZQ::common::FileLog &fileLog)
{
	//find session by on demand session id
	SessionMapKey key;
	key.strOnDemandSessionId = strOnDemandSessionId;
	
	//try to find session and session group
	NSSSessionGroup *pNSSSessionGroup = NULL;
	RTSPClientSession *sess = ngod_rtsp_action::FindOnDemandSessIter(pNSSSessionGroupList, strOnDemandSessionId, &pNSSSessionGroup);

	if (sess == NULL)
	{
		sess = ngod_rtsp_action::FindSessIter(pNSSSessionGroupList, strOnDemandSessionId, &pNSSSessionGroup);
		if (sess == NULL)
			return false;
	}

	//check socket status
	bool b = checkGroupSocketStatus(pNSSSessionGroup->strSessionGroup, pNSSSessionGroupList, pool, fileLog);
	if (!b)
		return false;

	EnterCriticalSection(sess->m_pCS_ClientSeq);
	sess->uClientCSeq = pNSSSessionGroup->usClientSeq++;
	uint16 index = sess->uClientCSeq;
	sess->iRTSPClientState = TEARDOWN;
	sess->m_RTSPR2Header.Reason.strReason_code = "200";
	sess->m_RTSPR2Header.Reason.strReason_text = "User Stop";
	sess->m_RTSPR2Header.Require.strComPath = NGODProvider;
	sess->m_RTSPR2Header.Require.strInterface_id = NGODR2Type;
	LeaveCriticalSection(sess->m_pCS_ClientSeq);

	//start the thread pool to send request
	//HANDLE pHandle = CreateEvent(NULL, true, false, NULL);
	sess->m_pEventHandle.m_Init(index);

	ngod_send_threadreq* req = new ngod_send_threadreq(&fileLog, pool, sess, false);
	req->start();
	if (waitSignal(sess, index) == false)
	{
		//CloseHandle(*sess->m_pEventHandle);
		fileLog(::ZQ::common::Log::L_ERROR, CLOGFMT(ngod_rtsp_action,"session(%s) TearDown timeout"), sess->m_RTSPR2Header.strOnDemandSessionID.c_str());
		sess->m_pEventHandle.m_CloseEvent(index);
		return false;
	}
	//CloseHandle(*sess->m_pEventHandle);
	sess->m_pEventHandle.m_CloseEvent(index);

	if (sess->iRTSPSessionState == RTSPOK)
		return true;
	else
		return false;
}

bool ngod_rtsp_action::GetParameterAction(string &strSessionGroup,
										  NSSSessionGroupList &pNSSSessionGroupList,
										  ZQ::common::NativeThreadPool &pool,
										  ZQ::common::FileLog &fileLog)
{
	//check socket status
	bool b = checkGroupSocketStatus(strSessionGroup, pNSSSessionGroupList, pool, fileLog);
	if (!b)
		return false;
	//get session group first
	NSSSessionGroupList::iterator iter = find_if(pNSSSessionGroupList.begin(), pNSSSessionGroupList.end(), FindBySessionGroup(strSessionGroup));
	if (iter == pNSSSessionGroupList.end())
		return false;

	RTSPClientSession *tmpSess = new RTSPClientSession();
	NSSSessionGroup *pGroup = *iter;
	tmpSess->RTSPSocket		= &pGroup->m_SessionSocket.m_Socket;
	tmpSess->m_RTSPR2Header.SessionGroup.strToken = pGroup->strSessionGroup;
	tmpSess->m_RTSPR2Header.m_GetPramameterReq_ExtHeader.header.push_back(session_list);
	EnterCriticalSection(&pGroup->m_CS_ClientSeq);
	tmpSess->uClientCSeq = pGroup->usClientSeq++;
	tmpSess->iRTSPClientState = GETPARAMETER;
	LeaveCriticalSection(&pGroup->m_CS_ClientSeq);
	pGroup->m_SessionGroupStatus.setStatus(Sync);
	tmpSess->m_pCS = &pGroup->m_CS;
	tmpSess->m_RTSPR2Header.m_SDPResponseContent.strHost = pGroup->strServerPath;
	tmpSess->m_RTSPR2Header.m_SDPResponseContent.uPort = pGroup->uServerPort;
	tmpSess->m_RTSPR2Header.Require.strComPath = NGODProvider;
	tmpSess->m_RTSPR2Header.Require.strInterface_id = NGODR2Type;
	ngod_send_threadreq *req = new ngod_send_threadreq(&fileLog, pool, tmpSess, true);
	req->start();

	return true;
}

bool ngod_rtsp_action::GetParameterActionBySessionId(::std::string &strOnDemandSessionId,
													 NSSSessionGroupList &pNSSSessionGroupList,
													 ZQ::common::NativeThreadPool &pool,
													 ZQ::common::FileLog &fileLog,
													 ::std::vector<GETPARAMETER_EXT> &headerList)
{
	//find session by on demand session id
	SessionMapKey key;
	key.strOnDemandSessionId = strOnDemandSessionId;

	//try to find session and session group
	NSSSessionGroup *pNSSSessionGroup = NULL;
	RTSPClientSession *sess = ngod_rtsp_action::FindOnDemandSessIter(pNSSSessionGroupList, strOnDemandSessionId, &pNSSSessionGroup);

	if (sess == NULL)
	{
		sess = ngod_rtsp_action::FindSessIter(pNSSSessionGroupList, strOnDemandSessionId, &pNSSSessionGroup);
		if (sess == NULL)
			return false;
	}

	sess->m_RTSPR2Header.m_GetPramameterReq_ExtHeader.header.clear();
	for (::std::vector<GETPARAMETER_EXT>::iterator iter= headerList.begin(); iter != headerList.end(); iter++)
		sess->m_RTSPR2Header.m_GetPramameterReq_ExtHeader.header.push_back(*iter);
	//sess->m_RTSPR2Header.m_GetPramameterReq_ExtHeader.header.push_back(presentation_state);
	//sess->m_RTSPR2Header.m_GetPramameterReq_ExtHeader.header.push_back(position);
	//sess->m_RTSPR2Header.m_GetPramameterReq_ExtHeader.header.push_back(scale);

	EnterCriticalSection(sess->m_pCS_ClientSeq);
	sess->uClientCSeq = pNSSSessionGroup->usClientSeq++;
	uint16 index = sess->uClientCSeq;
	sess->iRTSPClientState = GETPARAMETER;
	sess->m_RTSPR2Header.Require.strComPath = NGODProvider;
	sess->m_RTSPR2Header.Require.strInterface_id = NGODC1Type;
	LeaveCriticalSection(sess->m_pCS_ClientSeq);

	sess->m_pEventHandle.m_Init(index);

	ngod_send_threadreq *req = new ngod_send_threadreq(&fileLog, pool, sess, false);
	req->start();

	//long time no response
	if (waitSignal(sess, index) == false)
	{
		fileLog(::ZQ::common::Log::L_ERROR, CLOGFMT(ngod_rtsp_action,"session(%s) GetParameter timeout"), sess->m_RTSPR2Header.strOnDemandSessionID.c_str());
		sess->m_pEventHandle.m_CloseEvent(index);
		return false;
	}

	sess->m_pEventHandle.m_CloseEvent(index);

	if (sess->iRTSPSessionState == RTSPOK)
		return true;
	else
		return false;
}

bool ngod_rtsp_action::SetParameterAction(string &strSessionGroup,
										  NSSSessionGroupList &pNSSSessionGroupList,
										  ZQ::common::NativeThreadPool &pool,
										  ZQ::common::FileLog &fileLog)
{
	//get session group first
	NSSSessionGroupList::iterator iter = find_if(pNSSSessionGroupList.begin(), pNSSSessionGroupList.end(), FindBySessionGroup(strSessionGroup));
	if (iter == pNSSSessionGroupList.end())
		return false;

	RTSPClientSession *tmpSess = new RTSPClientSession();
	NSSSessionGroup *pGroup = *iter;
	tmpSess->RTSPSocket		= &pGroup->m_SessionSocket.m_Socket;
	tmpSess->m_RTSPR2Header.m_SessionGroups.strSessionGroup.push_back(pGroup->strSessionGroup);
	tmpSess->m_RTSPR2Header.m_GetPramameterReq_ExtHeader.header.push_back(session_list);
	EnterCriticalSection(&pGroup->m_CS_ClientSeq);
	tmpSess->uClientCSeq = pGroup->usClientSeq++;
	//pGroup->m_SessionGroupStatus.setStatus(Sync);
	tmpSess->iRTSPClientState = SETPARAMETER;
	LeaveCriticalSection(&pGroup->m_CS_ClientSeq);
	tmpSess->m_pCS = &pGroup->m_CS;
	tmpSess->m_RTSPR2Header.m_SDPResponseContent.strHost = pGroup->strServerPath;
	tmpSess->m_RTSPR2Header.m_SDPResponseContent.uPort = pGroup->uServerPort;
	tmpSess->m_RTSPR2Header.Require.strComPath = NGODProvider;
	tmpSess->m_RTSPR2Header.Require.strInterface_id = NGODR2Type;
	ngod_send_threadreq *req = new ngod_send_threadreq(&fileLog, pool, tmpSess, true);
	req->start();

	return true;
}

bool ngod_rtsp_action::PingAction(string &strOnDemandSessionId,
								  NSSSessionGroupList &pNSSSessionGroupList,
								  ZQ::common::NativeThreadPool &pool,
								  ZQ::common::FileLog &fileLog)
{
	//find session by on demand session id
	SessionMapKey key;
	key.strOnDemandSessionId = strOnDemandSessionId;
	
	//try to find session and session group
	NSSSessionGroup *pNSSSessionGroup = NULL;
	RTSPClientSession *sess = ngod_rtsp_action::FindOnDemandSessIter(pNSSSessionGroupList, strOnDemandSessionId, &pNSSSessionGroup);

	if (sess == NULL)
	{
		sess = ngod_rtsp_action::FindSessIter(pNSSSessionGroupList, strOnDemandSessionId, &pNSSSessionGroup);
		if (sess == NULL)
			return false;
	}

	//check socket status
	bool b = checkGroupSocketStatus(pNSSSessionGroup->strSessionGroup, pNSSSessionGroupList, pool, fileLog);
	if (!b)
		return false;

	EnterCriticalSection(sess->m_pCS_ClientSeq);
	sess->uClientCSeq = pNSSSessionGroup->usClientSeq++;
	uint16 index = sess->uClientCSeq;
	sess->iRTSPClientState = PING;
	LeaveCriticalSection(sess->m_pCS_ClientSeq);

	//start the thread pool to send request
	//HANDLE pHandle = CreateEvent(NULL, true, false, NULL);
	sess->m_pEventHandle.m_Init(index);

	ngod_send_threadreq* req = new ngod_send_threadreq(&fileLog, pool, sess, false);
	req->start();
	if (waitSignal(sess, index) == false)
	{
		//CloseHandle(*sess->m_pEventHandle);
		fileLog(::ZQ::common::Log::L_ERROR, CLOGFMT(ngod_rtsp_action,"session(%s) PingAction timeout"), sess->m_RTSPR2Header.strOnDemandSessionID.c_str());
		sess->m_pEventHandle.m_CloseEvent(index);
		return false;
	}
	//CloseHandle(*sess->m_pEventHandle);
	sess->m_pEventHandle.m_CloseEvent(index);

	if (sess->iRTSPSessionState == RTSPOK)
		return true;
	else
		return false;
}

RTSPClientSession* ngod_rtsp_action::FindOnDemandSessIter(NSSSessionGroupList &pNSSSessionGroupList,
												  string &strOnDemandSessionId,
												  NSSSessionGroup **pNSSSessionGroup)
{
	NSSSessionMap::iterator iter;

	for (NSSSessionGroupList::iterator pIter = pNSSSessionGroupList.begin();
	pIter != pNSSSessionGroupList.end(); pIter++)
	{
		*pNSSSessionGroup = *pIter;
		iter = (*pNSSSessionGroup)->m_NSSOnDemandSessionMap.find(strOnDemandSessionId);
		if (iter != (*pNSSSessionGroup)->m_NSSOnDemandSessionMap.end())
		{
			return	(*iter).second;
		}
	}

	return NULL;
}

RTSPClientSession* ngod_rtsp_action::FindSessIter(NSSSessionGroupList &pNSSSessionGroupList,
												  string &strSessionId,
												  NSSSessionGroup **pNSSSessionGroup)
{
	NSSSessionMap::iterator iter;
	for (NSSSessionGroupList::iterator pIter = pNSSSessionGroupList.begin();
		pIter != pNSSSessionGroupList.end(); pIter++)
	{
		*pNSSSessionGroup = *pIter;
		iter = (*pNSSSessionGroup)->m_NSSSessionMap.find(strSessionId);
		if (iter != (*pNSSSessionGroup)->m_NSSSessionMap.end())
		{
			return	(*iter).second;
		}
	}

	return NULL;
}

bool ngod_rtsp_action::waitSignal(RTSPClientSession *sess, uint16 index)
{
	DWORD ret = WaitForSingleObject(sess->m_pEventHandle._pHandle[index], iTimeOut);
	if (ret == 0x00000000L)
		return true;
	else if (ret == 0x00000102L)
		return false;
	else
		return false;
}

bool ngod_rtsp_action::checkGroupSocketStatus(string &strSessionGroup,
							NSSSessionGroupList &pNSSSessionGroupList,
							ZQ::common::NativeThreadPool &pool,
							ZQ::common::FileLog &fileLog)
{
	//get session group first
	NSSSessionGroupList::iterator iter = find_if(pNSSSessionGroupList.begin(), pNSSSessionGroupList.end(), FindBySessionGroup(strSessionGroup));
	if (iter == pNSSSessionGroupList.end())
		return false;

	//socket status ok
	const char *key = (*iter)->_groupUrlStr.generate();
	SessionGroupSocketMap::iterator sessionSocketIter = (*iter)->m_SessionGroupSocketMap.find(std::string(key));

	bool bStatus = true;;
	if (sessionSocketIter != (*iter)->m_SessionGroupSocketMap.end())
	{
		if (sessionSocketIter->second->_sessSock.m_Status == true)
			return true;
		else
		{
			//try to re-create socket and connect to server
			CloseSocket(sessionSocketIter->second->_sessSock.m_Socket);
			(*iter)->m_SessionSocket.m_Socket = CreateSocket(TCPSOCKET);
			sessionSocketIter->second->_sessSock.m_Socket = (*iter)->m_SessionSocket.m_Socket;
		}
	}

	if (!bConnection((*iter)->strServerPath, (*iter)->uServerPort, (*iter)->m_SessionSocket.m_Socket, 1))
		return false;
	else
	{
		(*iter)->m_SessionSocket.m_Status = true;
		bool b = ngod_rtsp_action::SetParameterAction(strSessionGroup, pNSSSessionGroupList, pool, fileLog);
		if (b)
		{
			SessionGroupSocket *pSessionGrouopSocket = NULL;
			::ZQ::common::MutexGuard guard((*iter)->m_SessionGroupSocketMapMutex);
			if (sessionSocketIter != (*iter)->m_SessionGroupSocketMap.end())
			{
				pSessionGrouopSocket = sessionSocketIter->second;
			}
			else
			{
				pSessionGrouopSocket = new SessionGroupSocket();
				(*iter)->m_SessionGroupSocketMap[std::string(key)] = pSessionGrouopSocket;
			}

			pSessionGrouopSocket->_iServerPort = (*iter)->uServerPort;
			pSessionGrouopSocket->_strServerIp = (*iter)->strServerPath;
			pSessionGrouopSocket->_sessSock.m_Socket = (*iter)->m_SessionSocket.m_Socket;
			pSessionGrouopSocket->_sessSock.m_Status = true;			
		}
		return b;
	}
		//return true;
}

bool ngod_rtsp_action::checkGroupSocketStatus(string &strSessionGroup,
											  string &socketKey,
											  NSSSessionGroupList &pNSSSessionGroupList)
{
	//get session group first
	NSSSessionGroupList::iterator iter = find_if(pNSSSessionGroupList.begin(), pNSSSessionGroupList.end(), FindBySessionGroup(strSessionGroup));
	if (iter == pNSSSessionGroupList.end())
		return false;

	SessionGroupSocketMap::iterator sessGroupSocketIter = (*iter)->m_SessionGroupSocketMap.find(socketKey);
	if (sessGroupSocketIter == (*iter)->m_SessionGroupSocketMap.end())
		return false;

	SessionGroupSocket *pTmpSessionGroupSock = sessGroupSocketIter->second;
	SessionSocket *pTmpSessSock = &(pTmpSessionGroupSock->_sessSock);
	//socket status ok
	if (pTmpSessSock->m_Status == true)
		return true;

	//try to re-create socket and connect to server
	CloseSocket(pTmpSessSock->m_Socket);
	pTmpSessSock->m_Socket = CreateSocket(TCPSOCKET);

	if (!bConnection(pTmpSessionGroupSock->_strServerIp, pTmpSessionGroupSock->_iServerPort, pTmpSessSock->m_Socket, 1))
		return false;
	else
	{
		pTmpSessSock->m_Status = true;
		return true;
	}
}
