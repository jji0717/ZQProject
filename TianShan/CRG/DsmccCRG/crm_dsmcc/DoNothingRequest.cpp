#include "crm_dsmcc.h"


//
clientses_ReleaseResponseHandler::clientses_ReleaseResponseHandler(DSMCC_Environment& env,RequestPtr request,TianShanIce::ClientRequest::SessionPrx sess)
:RequestHandler(env,request,sess ,"ClientReleaseResponse")
{
	_method = "ClientReleaseResponse" ;
#ifdef _DEBUG
	std::cout<< " construct ClientReleaseResponse handler "<< std::endl ;
#endif

}

//
clientses_ReleaseResponseHandler::~clientses_ReleaseResponseHandler()
{
#ifdef _DEBUG
	std::cout<< " deconstruct ClientReleaseResponse handler " <<std::endl ;
#endif
};

//
ProcessResult  clientses_ReleaseResponseHandler::doFixupRequest()
{
	hlog(ZQ::common::Log::L_DEBUG,HLOGFMT("clientses_ReleaseResponseHandler::doFixupRequest()")) ;
    return RESULT_PROCESSED ;
}

//
ProcessResult clientses_ReleaseResponseHandler::doContentHandler()
{
	hlog(ZQ::common::Log::L_DEBUG,HLOGFMT("clientses_ReleaseResponseHandler::doContentHandler()")) ;
	return RESULT_PROCESSED ;
}

//
ProcessResult  clientses_ReleaseResponseHandler::doFixupRespone()
{
	hlog(ZQ::common::Log::L_DEBUG,HLOGFMT("clientses_ReleaseResponseHandler::doFixupRespone()") )  ;
	return RESULT_PROCESSED ;
}