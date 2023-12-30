#include "./PingHandler.h"

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif

PingHandler::PingHandler(ssmNGODr2c1& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq) : RequestHandler(ssm, pSite, pReq)
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
		return RequestError;
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
	
	responseOK();

	return RequestProcessed;
}

