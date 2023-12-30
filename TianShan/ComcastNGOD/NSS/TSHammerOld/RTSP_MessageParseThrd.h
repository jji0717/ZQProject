#ifndef __RTSP_MESSAGEPARSETHRD_H__
#define __RTSP_MESSAGEPARSETHRD_H__

#include "RTSP_common_structure.h"
#include "XML_SessCtxHandler.h"
#include "XML_ResponseHandler.h"
#include "SessionWatchDog.h"

typedef list<XML_SessCtxHandler *> XML_SessCtxHandlerVec;

class RTSP_MessageParseThrd : public ZQ::common::NativeThread
{
public:
	RTSP_MessageParseThrd(::ZQ::common::FileLog &fileLog, RTSPMessageList &rtspMessageList, SessionMap &sessionMap);

	~RTSP_MessageParseThrd();

	int		run(void);

	//inline void setResponseHandler(XML_ResponseHandler *xml_ResponseHandler)
	//{
	//	EnterCriticalSection(&_CS);
	//	_xml_ResponseHandler = xml_ResponseHandler;
	//	LeaveCriticalSection(&_CS);
	//}

private:
	::ZQ::common::FileLog	*_fileLog;
	RTSPMessageList			&_rtspMessageList;
	//XML_SessCtxHandlerVec	&_xml_SessCtxHandlerVec;
	//XML_ResponseHandler	8*_xml_ResponseHandler;
	SessionMap				&_sessionMap;

	CRITICAL_SECTION		_CS;
};

#endif __RTSP_MESSAGEPARSETHRD_H__