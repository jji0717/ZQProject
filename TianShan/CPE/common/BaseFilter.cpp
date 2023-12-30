

#include "BaseClass.h"
#ifdef ZQ_OS_LINUX
extern "C"
{
#include <errno.h>
}
#endif

#define BaseFlt			"BaseFlt"
#define BaseTag			"BaseTag"

using namespace ZQ::common;

#define MOLOG (*_pLog)

namespace ZQTianShan {
	namespace ContentProvision {

#ifdef ZQ_OS_MSWIN
void getSystemErrorText(std::string& strErrMsg)
{
	const int MAX_SYS_ERROR_TEXT = 256;
	char sErrorText[MAX_SYS_ERROR_TEXT+50]={0};
	
	DWORD lastError = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, 
		lastError,
		0,
		sErrorText, 
		MAX_SYS_ERROR_TEXT, 
		NULL);
	char errcode[24];
	sprintf(errcode, "[%d]", lastError);

	if (sErrorText[0])
	{
		char* pPtr = sErrorText + strlen(sErrorText) - 1;
		while(pPtr>=sErrorText && (*pPtr == 0x0d || *pPtr == 0x0a))
		{
			*pPtr = '\0';
			pPtr--;
		}

		strErrMsg = sErrorText;
		return;
	}

	{
		char wszErrMsg [256];
		sprintf(wszErrMsg, "Error Code = %d", lastError);
		strErrMsg = wszErrMsg;
	}
}
#else

void getSystemErrorText(std::string& strErrMsg)
{
	char buf[256] = {0};
	char* pbuf = strerror_r(errno,buf,sizeof(buf));
	if(!pbuf)
	{
		sprintf(buf,"Error code = %d",errno);
		strErrMsg = buf;
	}
	else
		strErrMsg = pbuf;
}
#endif



//void BitrateControlor::start()
//{
//	_nBytesInterval = int32(((float)_nBitrateBps) * _nCtrlIntervalMs / 8000);
//	_nStart = SYS::getTickCount();
//	_lastCtrlBytes = 0;
//
//}
//
//// return the sleep time in milliseconds
//uint32 BitrateControlor::control(uint64 llProcBytes)
//{
//	if (!_nBitrateBps)
//		return 0;
//
//	if (llProcBytes < _lastCtrlBytes + _nBytesInterval)
//	{
//		//do nothing
//		return 0;
//	}
//
//	_lastCtrlBytes = llProcBytes;
//
//	uint32 nSpent = SYS::getTickCount() - _nStart;			
//	uint32 nShouldSpent = uint32(llProcBytes*8000/_nBitrateBps); //ms
//
//	if (nShouldSpent > nSpent)
//	{	
//		if (nShouldSpent > nSpent)
//		{	
//			if (nShouldSpent > nSpent)
//			{
//				uint32 nSleep = nShouldSpent - nSpent;
//				if (nSleep > BITRATE_CONTROL_MAX_WAITMS)
//					nSleep = BITRATE_CONTROL_MAX_WAITMS;
//
//#ifdef ZQ_OS_MSWIN
//				Sleep(nSleep);			
//#else
//			usleep((nSleep)*1000);
//#endif
//
//				return nSleep;
//			}
//		}	
//#ifdef ZQ_OS_MSWIN
//		Sleep(nShouldSpent - nSpent);			
//#else
//		usleep((nShouldSpent-nSpent)*1000);
//#endif
//	}	
//
//	return 0;
//}


///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////



BaseFilter::BaseFilter()
	:_pGraph(NULL),_llProcBytes(0),_nInputCount(0),_nOutputCount(0),_nType(0), _nLastErrCode(0), _bFailed(false),_bDriverModule(false),_bStop(false) {
//	ZQ::common::setGlogger();
}

BaseFilter::~BaseFilter()
{

}

bool BaseFilter::Next(int nOutputIndex, BaseFilter* pNextFilter, int nInputIndex)
{
	if (_nOutputCount<=nOutputIndex)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(BaseFlt, "[%s] (%s) invalid  outputindex[%d]"), _strLogHint.c_str(),
			GetName(), nOutputIndex);
		return false;
	}

	if (pNextFilter->GetInputCount()<=nInputIndex)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(BaseFlt, "[%s] (%s) invalid  inputindex[%d]"), _strLogHint.c_str(),
			pNextFilter->GetName(), nInputIndex);
		return false;
	}

	_outputPin[nOutputIndex].nNextPin = nInputIndex;
	_outputPin[nOutputIndex].pNextFilter = pNextFilter;
	MOLOG(Log::L_DEBUG, CLOGFMT(BaseFlt, "[%s] (%s)[%d] link next to (%s)[%d] successfully"), _strLogHint.c_str(),
		GetName(), nOutputIndex, pNextFilter->GetName(), nInputIndex);

	return true;
}

bool BaseFilter::Prev(int nInputIndex, BaseFilter* pPrevFilter, int nOutputIndex)
{
	if (_nInputCount<=nInputIndex)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(BaseFlt, "[%s] (%s) invalid  inputindex[%d]"), _strLogHint.c_str(),
			GetName(), nInputIndex);
		return false;
	}
	
	if (pPrevFilter->GetOutputCount()<=nOutputIndex)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(BaseFlt, "[%s] (%s) invalid  inputoutput[%d]"), _strLogHint.c_str(),
			pPrevFilter->GetName(), nOutputIndex);
		return false;
	}
	
	_inputPin[nInputIndex].nPrevPin = nOutputIndex;
	_inputPin[nInputIndex].pPrevFilter = pPrevFilter;
	MOLOG(Log::L_DEBUG, CLOGFMT(BaseFlt, "[%s] (%s)[%d] link prev to (%s)[%d] successfully"), _strLogHint.c_str(),
		GetName(), nInputIndex, pPrevFilter->GetName(), nOutputIndex);
	
	return true;
}

void BaseFilter::Stop()
{
	_bStop = true;
	MOLOG(Log::L_INFO, CLOGFMT(BaseFlt, "[%s] (%s) stop called"), _strLogHint.c_str(), GetName());
}

void BaseFilter::SetLastError(const std::string& strErr, int nErrorCode)
{
	_strLastErr = strErr;_bFailed = true;
	_nLastErrCode = nErrorCode;
        if(!GetGraph())
        {
	MOLOG(Log::L_INFO, CLOGFMT(BaseFlt, "Graph is Null now, can't SetLastError."));
        return;
        }
	GetGraph()->SetLastError(strErr, nErrorCode);
}

BaseTarget::BaseTarget()
{
	_nType = TYPE_TARGET;
	_nOutputCount = 0;
	_nStreamableBytes = 0;
	_bProgressEvent = false;
	_bStreamableEvent = false;
	_bIsStreamable = false;
}

void BaseTarget::IncProcvBytes(int nBytes)
{
	_llProcBytes+=nBytes;

	if (_bProgressEvent)
	{
		// send progress
		GetGraph()->OnProgress(_llProcBytes);
	}

	if (_bStreamableEvent&&!_bIsStreamable&&_nStreamableBytes&&_llProcBytes>=_nStreamableBytes)
	{
		_bIsStreamable = true;
		GetGraph()->OnStreamable(true);
	}
}

void BaseTarget::setStreamableBytes(int nStreamableBytes)
{
	_nStreamableBytes = nStreamableBytes;
	MOLOG(Log::L_DEBUG, CLOGFMT(BaseTag, "[%s] (%s) set streamable bytes to %d"), _strLogHint.c_str(),
		GetName(), nStreamableBytes);
}

}}
