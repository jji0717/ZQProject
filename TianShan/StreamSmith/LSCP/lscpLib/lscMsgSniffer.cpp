// lscMsgSniffer.cpp: implementation of the lscMsgSniffer class.
//
//////////////////////////////////////////////////////////////////////
#include "lscMsgSniffer.h"
#include <Log.h>

#ifdef ZQ_OS_MSWIN
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace ZQ::common;
using namespace lsc;
const char* ConvertOpCodeToString(lsc::OperationCode opCode)
{
	switch (opCode)
	{
	case LSC_PAUSE:			return "PAUSE";
	case LSC_RESUME:		return "RESUME";
	case LSC_STATUS:		return "STATUS";
	case LSC_RESET:			return "RESET";
	case LSC_JUMP:			return "JUMP";
	case LSC_PLAY:			return "PLAY";
	case LSC_DONE:			return "DONE";
	case LSC_PAUSE_REPLY:	return "PAUSEREPLY";
	case LSC_RESUME_REPLY:	return "RESUMEREPLAY";
	case LSC_STATUS_REPLY:	return "STATUSREPLY";
	case LSC_RESET_REPLY:	return "RESETREPLY";
	case LSC_JUMP_REPLY:	return "JUMPREPLY";
	case LSC_PLAY_REPLY:	return "PLAYREPLY";
	default:				return NULL;
    }
	return NULL;
}
void lscMsgSniffer::showMsgDetail (lsc::lscMessage* msg,const std::string& strHint)
{
	if (!msg) return;
	
	const lsc::LSCMESSAGE& lscmsg = msg->GetLscMessageContent ();
	const lsc::StandardHeader_t& standHeader = lscmsg.jump.header;
	char	szLocalBuf[1024];	
	int iSize = sizeof(szLocalBuf);
	szLocalBuf[iSize-1]='\0';
	int iPos = snprintf(szLocalBuf,iSize-1,
					"%s version[%x] TransCode[%x] opCode[%x][%s] statusCode[%x] streamHandle[%x]",
					strHint.c_str (),standHeader.version,standHeader.transactionId,
					standHeader.opCode,ConvertOpCodeToString((lsc::OperationCode)standHeader.opCode),
					standHeader.statusCode,standHeader.streamHandle);
	iSize -= iPos;

	switch(standHeader.opCode) 
	{
	case LSC_PAUSE:
		{
			snprintf (szLocalBuf+iPos,iSize-1," stopNpt[%u]",lscmsg.pause.stopNpt);
		}
		break;
	case LSC_RESUME:
		{
			snprintf (szLocalBuf+iPos,iSize-1," startNpt[%u] numerator[%d] denominator[%d]",
						lscmsg.resume.startNpt,
						lscmsg.resume.numerator,
						lscmsg.resume.denominator);
		}
		break;
	case LSC_STATUS:
		{
			
		}
		break;
	case LSC_RESET:
		{
		}
		break;
	case LSC_JUMP:		
	case LSC_PLAY:
		{
			snprintf (szLocalBuf+iPos,iSize-1," startNpt[%u] stopNpt[%u] numerator[%u] denominator[%u]",
						lscmsg.play.data.startNpt,
						lscmsg.play.data.stopNpt,
						lscmsg.play.data.numerator,
						lscmsg.play.data.denominator);
		}
		break;
	case LSC_DONE:
	case LSC_PAUSE_REPLY:
	case LSC_RESUME_REPLY:
	case LSC_STATUS_REPLY:
	case LSC_RESET_REPLY:
	case LSC_JUMP_REPLY:
	case LSC_PLAY_REPLY:
		{
			snprintf (szLocalBuf+iPos,iSize-1," currentNpt[%u] numerator[%d] denominator[%d] mode[%d]",
						lscmsg.response.data.currentNpt,
						lscmsg.response.data.numerator,
						lscmsg.response.data.denominator,
						lscmsg.response.data.mode);
		}
		break;
	default:
		{
			glog(ZQ::common::Log::L_DEBUG,"%s Unkown OpCode[%x] maybe it's invalid message",
						strHint.c_str () ,standHeader.opCode );
			return;
		}
		break;
	}
	glog(ZQ::common::Log::L_DEBUG,"%s",szLocalBuf);
}
