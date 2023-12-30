#include "./TeardownHandler.h"

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif

TeardownHandler::TeardownHandler(ssmNGODr2c1& ssm, IStreamSmithSite* pSite, IClientRequestWriter* pReq) : RequestHandler(ssm, pSite, pReq)
{
	_method = "TEARDOWN";
	_inoutMap[MAP_KEY_METHOD] = _method;
#ifdef _DEBUG
	cout<<"construct TEARDOWN handler"<<endl;
#endif

}

TeardownHandler::~TeardownHandler()
{
#ifdef _DEBUG
	cout<<"deconstruct TEARDOWN handler"<<endl;
#endif

}

RequestProcessResult TeardownHandler::process()
{
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(TeardownHandler, "start to be processed"));

	if (false == _canProcess)
	{
		HANDLERLOG(ZQ::common::Log::L_ERROR, HANDLERLOGFMT(TeardownHandler, "we can't process the request because of [%s]"), szBuf);
		return RequestError;
	}

	std::string onDemandID;
	onDemandID = getRequestHeader(NGOD_HEADER_ONDEMANDSESSIONID);

	if (false == getContext())
	{
		responseError(RESPONSE_INTERNAL_ERROR);
		return RequestError;
	}

	if (true == _ssmNGODr2c1._pSite->destroyClientSession(_session.c_str()))
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(TeardownHandler, "rtspProxy session destroyed"));

	if (true == getWeiwooSession())
		destroyWeiwooSession((false == _xreason.empty()) ? _xreason : "teardown");

	removeContext((false == _xreason.empty()) ? _xreason : "teardown");

	std::string tmp = getRequestHeader("NeedResponse", ZQ::common::Log::L_NOTICE);
	if (0 == stricmp(tmp.c_str(), "no"))
	{
		HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(TeardownHandler, "it needn't response"));
		return RequestProcessed;
	}

	_pResponse->setHeader(NGOD_HEADER_ONDEMANDSESSIONID, onDemandID.c_str());
	_pResponse->setHeader(NGOD_HEADER_CONTENTTYPE, "text/xml");
	_retContent = "<ResponseData><ODRMSessionHistory></ODRMSessionHistory></ResponseData>";
	_pResponse->printf_postheader(_retContent.c_str());
	responseOK();

	return RequestProcessed;
}