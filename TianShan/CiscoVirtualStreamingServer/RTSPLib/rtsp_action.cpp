#include "rtsp_action.h"
#include "RTSPThreadRequest.h"
#include "CVSSRtspParser/ClientSocket.h"

int32 iTimeOut = 0;

bool rtsp_action::RTSPAction(RTSPClientState state,
							 RtspCSeqSignal &rtspCSeqSignal,
							 CVSSRtspSession *pCVSSSession,
							 ::ZQ::common::NativeThreadPool &pool,
							 ::ZQ::common::FileLog &fileLog)
{
	//find session by on demand session id
	if (pCVSSSession == NULL)
		return false;

	pCVSSSession->_commonReqHeader._iCSeq++;
	uint16 index = pCVSSSession->_rtspSocket._socket;
	pCVSSSession->iRTSPClientState = state;

	//start the thread pool to send request
	//HANDLE pHandle = CreateEvent(NULL, true, false, NULL);
	if (rtspCSeqSignal.m_Init(index) == false)
		return false;

	RTSPRequestSender *sender = new RTSPRequestSender(&fileLog, pool, pCVSSSession);
	sender->start();
	
	if (state == ANNOUNCE)
		return true;
	//long time no response
	if (waitSignal(rtspCSeqSignal, index) == false)
	{
		//CloseHandle(sess->_pEventHandle);
		fileLog(::ZQ::common::Log::L_ERROR, CLOGFMT(rtsp_action,"Sess(%s)Seq(%d) wait for rtsp response timeout"), pCVSSSession->_commonReqHeader._strSessionId.c_str(), pCVSSSession->_commonReqHeader._iCSeq);
		rtspCSeqSignal.m_CloseEvent(index);
		return false;
	}

	rtspCSeqSignal.m_CloseEvent(index);
	if (pCVSSSession->iRTSPSessionState == RTSPOK)
		return true;
	else
		return false;
}

bool rtsp_action::waitSignal( RtspCSeqSignal &rtspCSeqSignal, uint16 index)
{
	DWORD ret = WaitForSingleObject(rtspCSeqSignal._pHandle[index], iTimeOut);
	if (ret == 0x00000000L)
		return true;
	else if (ret == 0x00000102L)
		return false;
	else
		return false;
}