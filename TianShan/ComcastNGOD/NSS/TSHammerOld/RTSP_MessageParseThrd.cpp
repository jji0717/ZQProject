#include "RTSP_MessageParseThrd.h"
#include "RTSP_MessageParser.h"

class FindByCSeq
{
public:
	FindByCSeq(uint16 &uCSeq):_uCSeq(uCSeq){}
	bool operator() (XML_SessCtxHandler *xml_SessCtxHandler)
	{
		if (_uCSeq == atoi(xml_SessCtxHandler->_sessCtxMap[SESSCSEQ].value.c_str()))
			return true;
		else
			return false;
	}
private:
	uint16 _uCSeq;
};

RTSP_MessageParseThrd::RTSP_MessageParseThrd(::ZQ::common::FileLog &fileLog, RTSPMessageList &rtspMessageList, SessionMap &sessionMap)
:_fileLog(&fileLog)
,_rtspMessageList(rtspMessageList)
,_sessionMap(sessionMap)
{
	InitializeCriticalSection(&_CS);
}

RTSP_MessageParseThrd::~RTSP_MessageParseThrd()
{
	DeleteCriticalSection(&_CS);
}

int RTSP_MessageParseThrd::run(void)
{
	//define a pointer for convenient use
	RTSPMessageList *pMessList = &_rtspMessageList;

	while (1)
	{
		//no message in list
		if (pMessList->m_MessageList.empty())
			SleepContinue

		int startTime = GetTickCount();

		//get the first data
		void *idx = NULL;
		::std::string queMessage;
		pMessList->First(&idx, queMessage);

		//get the first data
		::std::string *queiter = &queMessage;

		const char *tmpstr = (*queiter).c_str();

		DWORD sTime = GetTickCount();
		MYLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTSP_MessageParseThrd,"begin parse one message\r\n%s"), tmpstr);

		::std::string strSess;
		RTSPMessageParser::GetSessionID(queiter->c_str(), (uint16)queiter->length(), strSess);	

		bool b = RTSPMessageParser::CheckAnnounceMessage(queiter->c_str(), queiter->length());
		if (b)
		{
			pMessList->PopFront();
			continue;
		}
		
		//TODO: find SessCtx from Vector
		
		//XML_SessCtxHandlerVec::iterator SessCtxIter = find_if(_xml_SessCtxHandlerVec.begin(), _xml_SessCtxHandlerVec.end(), FindByCSeq(pCSeq));
		//_sessionMap.getLock();
		SessionHandler *sessHandler = NULL;
		if (idx != NULL)
			sessHandler = (SessionHandler *)idx;
		else
			sessHandler = _sessionMap.getSessionHandler(strSess);
		if (sessHandler == NULL)
		{
			//TODO: log error search information
			//_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(RTSP_MessageParseThrd,"Not Find CSeq %d of SessionHandler"), pCSeq);

			uint16 pCSeq = 0;
			RTSPMessageParser::GetSequence(queiter->c_str(), (uint16)queiter->length(), pCSeq);
			sessHandler = _sessionMap.getSessionHandler(pCSeq);

			if (sessHandler != NULL)
			{
				//_sessionMap.releaseLock();
				MYLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTSP_MessageParseThrd,"Find CSeq %d of SessionHandler"), pCSeq);
				sessHandler->OnResponse(queiter->c_str());
			}
			else
			{
				
 				MYLOG(ZQ::common::Log::L_ERROR, CLOGFMT(RTSP_MessageParseThrd,"Not Find Session[%s](CSeq[%d]) of SessionHandler"), strSess.c_str(), pCSeq);
				//_sessionMap.releaseLock();
			}
		}
		else//TODO: parse message to get specify data
		{
			//_sessionMap.releaseLock();
			//parse message
			MYLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTSP_MessageParseThrd,"Find Session %s of SessionHandler"), strSess.c_str());
			sessHandler->OnResponse(queiter->c_str());
			//EnterCriticalSection(&_CS);

			//XML_ResponseHandler tmpResponseHandler = *_xml_ResponseHandler;
			//tmpResponseHandler.parseResponse(*queiter);

			//tmpResponseHandler.updateSessCtxHandler(*(*SessCtxIter));
			//
			//LeaveCriticalSection(&_CS);

			//_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(RTSP_MessageParseThrd,"SetEvent of CSeq %d"), pCSeq);
			//SetEvent((*SessCtxIter)->_handle);
		}
		MYLOG(ZQ::common::Log::L_INFO, CLOGFMT(RTSP_MessageParseThrd,"Parse one message cost %dms"), GetTickCount() - sTime);
		pMessList->PopFront();
	}

	return 1;
}