#include "./PingHandler.h"

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif

PingHandler::PingHandler(NGODEnv& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq) : RequestHandler(ssm, pSite, pReq)
{
	_method = "PING";
	_inoutMap[MAP_KEY_METHOD] = _method;
#ifdef _DEBUG
	cout<<"construct PING handler"<<endl;
#endif

}

PingHandler::~PingHandler()
{
#ifdef _DEBUG
	cout<<"deconstruct PING handler"<<endl;
#endif

}

RequestProcessResult PingHandler::process()
{
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(PingHandler, "start to be processed"));

	if (false == _canProcess)
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PingHandler, "we can't process the request because of [%s]"), szBuf);
		HANDLEREVENTLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(PingHandler, "we can't process the request because of [%s]"), szBuf);
		return RequestError;
	}
	if (_ngodConfig._MessageFmt.rtspNptUsage <= 0)
	{
		if (!handshake(_requireProtocol))
		{
			return RequestError;
		}
	}
	

	if (false == getContext())
	{
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}

	if (false == renewSession())
	{
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}

	std::string tmp = getRequestHeader("NeedResponse");
	if (0 == stricmp(tmp.c_str(), "no"))
	{
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(PingHandler, "it need't response"));
		return RequestProcessed;
	}

	//update C1ConnectionId
	::std::string connId = getRequestHeader("SYS#ConnID");
	if (false == updateContextProp(C1CONNID, connId))
	{
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}
	
	responseOK();

	return RequestProcessed;
}

