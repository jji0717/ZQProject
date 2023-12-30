#include "OptionRequest.h"

namespace TianShanS1
{
	FixupOption::FixupOption(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: FixupRequest(env, pSite, pReq, pResponse)
	{
	}

	FixupOption::~FixupOption()
	{
	}

	bool FixupOption::process()
	{
		return true;
	}

	HandleOption::HandleOption(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse)
		: ContentHandler(env, pSite, pReq, pResponse)
	{
	}

	HandleOption::~HandleOption()
	{
	}

	bool HandleOption::process()
	{
		_pResponse->printf_preheader(ResponseOK);
		_pResponse->setHeader(HeaderMethodCode, _method.c_str());
		_pResponse->setHeader(HeaderServer, _env._serverHeader.c_str());
		_pResponse->setHeader(HeaderPublic, "SETUP, PLAY, PAUSE, TEARDOWN, GET_PARAMETER, OPTIONS, DESCRIBE");
		return true;
	}

	OptionResponse::OptionResponse(Environment& env, IStreamSmithSite* pSite, IClientRequest* pReq, IServerResponse* pResponse)
		: FixupResponse(env, pSite, pReq, pResponse)
	{
	}

	OptionResponse::~OptionResponse()
	{
	}

	bool OptionResponse::process()
	{
		return true;
	}

} // namespace TianShanS1

