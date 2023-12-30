// File Name : DescribeHandler.cpp

#include "DescribeHandler.h"

#include "Environment.h"

#include "stroprt.h"

#include "RtspRelevant.h"


namespace GBss
{

DescribeHandler::DescribeHandler(ZQ::common::Log& fileLog, Environment& env, 
						   IStreamSmithSite* pSite, IClientRequestWriter* pReq)
: RequestHandler(fileLog, env, pSite, pReq)
{
	_method = "DESCRIBE";
}

DescribeHandler::~DescribeHandler()
{

}

RequestProcessResult DescribeHandler::doContentHandler()
{
	HANDLERLOG(ZQ::common::Log::L_INFO, HANDLERLOGFMT(DescribeHandler, "start processing"));
	_response->setHeader(HeaderContentType, "application/sdp");


	if (!renewSession())
	{
		return RequestError;
	}

	std::string range, scale;
	if (!getPositionAndScale(range, scale))
	{
		return RequestError;
	}

	std::stringstream ss;
	ss << "a=type:vod" << CRLF;
	ss << "a=range:npt=" << range << CRLF;
	std::string strContent = ss.str();
	_response->printf_postheader(strContent.c_str());

	return RequestProcessed;
}

} // end GBss
