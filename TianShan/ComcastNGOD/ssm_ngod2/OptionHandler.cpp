#include "./OptionHandler.h"

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif

OptionHandler::OptionHandler(NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq) : RequestHandler(ssm, pSite, pReq)
{
	_method = "OPTION";
	_inoutMap[MAP_KEY_METHOD] = _method;
#ifdef _DEBUG
	cout<<"construct OPTION handler"<<endl;
#endif
}

OptionHandler::~OptionHandler()
{
#ifdef _DEBUG
	cout<<"deconstruct OPTION handler"<<endl;
#endif

}

const char* OptionHandler::getMonthStr(WORD month)
{
	switch (month)
	{
	case 1: {return "Jan"; break;}
	case 2: {return "Feb"; break;}
	case 3: {return "Mar"; break;}
	case 4: {return "Apr"; break;}
	case 5: {return "May"; break;}
	case 6: {return "Jun"; break;}
	case 7: {return "Jul"; break;}
	case 8: {return "Aug"; break;}
	case 9: {return "Sep"; break;}
	case 10: {return "Oct"; break;}
	case 11: {return "Nov"; break;}
	case 12: {return "Dec"; break;}
	}
	return "";
}

const char* OptionHandler::getDayOfWeekStr(WORD dayOfWeek)
{
	switch (dayOfWeek)
	{
	case 0: {return "Sun"; break;}
	case 1: {return "Mon"; break;}
	case 2: {return "Tue"; break;}
	case 3: {return "Wed"; break;}
	case 4: {return "Thu"; break;}
	case 5: {return "Fri"; break;}
	case 6: {return "Sat"; break;}
	}
	return "";
}

void OptionHandler::getGMTTime(char* buff, uint16 len)
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	snprintf(buff, len, "%s, %02d %s %04d %02d:%02d:%02d GMT"
		, getDayOfWeekStr(time.wDayOfWeek), time.wDay, getMonthStr(time.wMonth), time.wYear, time.wHour, time.wMinute, time.wSecond);
}

RequestProcessResult OptionHandler::process()
{
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(OptionHandler, "start to be processed"));

	if (false == _canProcess)
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(OptionHandler, "we can't process the request because of [%s]"), szBuf);
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(OptionHandler, "we can't process the request because of [%s]"), szBuf);
		return RequestError;
	}
	if (_ngodConfig._MessageFmt.rtspNptUsage <= 0)
	{
		if (!handshake(_requireProtocol, 0, 4, false))
		{
			return RequestError;
		}
	}
	

	snprintf(szBuf, MY_BUFFER_SIZE - 1, "DESCRIBE, GET_PARAMETER, SET_PARAMETER, PING, OPTIONS, SETUP, TEARDOWN, PLAY, PAUSE");
	_pResponse->setHeader(NGOD_HEADER_PUBLIC, szBuf);
	getGMTTime(szBuf, sizeof(szBuf) - 1);
	_pResponse->setHeader(NGOD_HEADER_DATE, szBuf);
	responseOK();
	
	return RequestProcessed;
}

