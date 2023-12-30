#ifndef __RTSP_MESSAGESENDTHRD_H__
#define __RTSP_MESSAGESENDTHRD_H__

#include "RTSP_common_structure.h"
#include "XML_SessCtxHandler.h"
#include "RTSP_MessageParseThrd.h"

class RTSP_MessageSendThrd : public ZQ::common::ThreadRequest
{
public:
	RTSP_MessageSendThrd(::ZQ::common::FileLog &fileLog, 
						 ::ZQ::common::NativeThreadPool& Pool, 
						 SessionSocket &sessionSocket, 
						 ::std::string &rtspMessage,
						 XML_SessCtxHandler &xml_SessCtxHandler/*, 
						 XML_SessCtxHandlerVec &xml_SessCtxHandlerVec, 
						 int32 &interval, 
						 CRITICAL_SECTION &CS*/);
	~RTSP_MessageSendThrd();

protected:	
	int run(void);
	void final(int retcode, bool bCancelled)
	{
		delete this;
	}

private:
	ZQ::common::FileLog		*_log;
	SessionSocket			&_socket;
	::std::string			_rtspMessage;
	XML_SessCtxHandler		&_xml_SessCtxHandler;
	//XML_SessCtxHandlerVec	&_xml_SessCtxHandlerVec;
	//int32					_sleepInterval;
	//CRITICAL_SECTION		&_SessCtxCS;
};
#endif __RTSP_MESSAGESENDTHRD_H__