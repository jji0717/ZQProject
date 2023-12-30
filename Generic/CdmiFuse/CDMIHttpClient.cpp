#include "CDMIHttpClient.h"

// #include "ZQResouce.h"
#define ZQ_PRODUCT_VER_MAJOR 2
#define ZQ_PRODUCT_VER_MINOR 0

#define __N2S2__(x) #x
#define __N2S__(x) __N2S2__(x)
#define USER_AGENT		"CdmiFuse/"__N2S__(ZQ_PRODUCT_VER_MAJOR) "." __N2S__(ZQ_PRODUCT_VER_MINOR)

namespace ZQ {
namespace CDMIClient {

#define CLOG  (_log)
#define CURLFMT( _X) CLOGFMT(CDMIHttpClient, "client[%s]so[%x] " _X), _clientId.c_str(),_fd

///////////////////////////////////////////////////////////////////////////////////////
/////////////////// class CDMIHttpClient //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
CDMIHttpClient::CDMIHttpClient(char* url, ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdpool, 
							   int flags, HTTPMETHOD method,  char* bindIp, std::string clientId):
CURLClient(url, log, thrdpool, flags, method, bindIp,clientId)
{
	_userAgent = USER_AGENT;
}

void CDMIHttpClient::OnTxnCompleted(CURLcode code)
{
	CURLClient::OnTxnCompleted(code);
	if (_pRespBodyBuf)
		CLOG(ZQ::common::Log::L_INFO, CURLFMT("OnTxnCompleted() got response [%d/%d]bytes"), _pRespBodyBuf->length(), _pRespBodyBuf->size());
}

}} ///end namespace