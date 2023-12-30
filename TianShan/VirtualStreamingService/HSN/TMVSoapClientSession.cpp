//include winsock to avoid multi-definition error
#include <WinSock2.h>

//include gsoap header
//#include "TMVSS.nsmap"
//#include "soapH.h"
//#include "soapStub.h"
//#include "stdsoap2.h"
//#include "stdsoap2.cpp"

//include class header
#include "TMVSoapClientSession.h"

TMVSoapClientSession::TMVSoapClientSession(::ZQ::common::FileLog &fileLog, const char* soapServerAddr, const int& soapServerPort)
:_fileLog(&fileLog)
{
	//set url
	_urlStr.setHost(soapServerAddr);
	_urlStr.setPort(soapServerPort);
	_urlStr.setProtocol("http");

	//initialize soap client class
	soap_init(&_tmvssSoapClient);
	_tmvssSoapClient.soap_endpoint = _urlStr.generate();
	SOAPLOG(ZQ::common::Log::L_INFO, CLOGFMT(TMVSSServer, "SOAP server at %s"), _tmvssSoapClient.soap_endpoint);

	_bDestroy = false;
}

TMVSoapClientSession::~TMVSoapClientSession()
{
	//destroy client soap client service
	soap_destroy(&_tmvssSoapClient);
	soap_end(&_tmvssSoapClient);
	soap_done(&_tmvssSoapClient);
}

bool TMVSoapClientSession::soapSetup(ZQ2__setupInfo *setupInfo, struct ZQ2__setupResponse &setupResInfo)
{
	if (_tmvssSoapClient.setup(setupInfo, setupResInfo) != SOAP_OK)
	{
		SOAPLOG(::ZQ::common::Log::L_ERROR, CLOGFMT(TMVSoapClientSession,"encounter soap error when setup, fault=%s"), _tmvssSoapClient.fault->faultstring);
		return false;
	}
	else
	{
		SOAPLOG(::ZQ::common::Log::L_INFO, CLOGFMT(TMVSoapClientSession,"setup soap call ok"));
		return true;
	}
}

bool TMVSoapClientSession::soapTeardown(std::string sessionId, int &ret)
{
	if (_tmvssSoapClient.teardown(sessionId, ret) != SOAP_OK)
	{
		SOAPLOG(::ZQ::common::Log::L_ERROR, CLOGFMT(TMVSoapClientSession,"session(%d) encounter soap error when teardown, fault=%s, detail=%s"), sessionId.c_str(), _tmvssSoapClient.fault->faultstring);
		return false;
	}
	else
	{
		SOAPLOG(::ZQ::common::Log::L_INFO, CLOGFMT(TMVSoapClientSession,"session(%d) teardown soap call success"), sessionId.c_str());
		return true;
	}
}

bool TMVSoapClientSession::soapGetStatus(std::string sessionId, struct ZQ2__getStatusResponse &getStatusResInfo)
{
	if (_tmvssSoapClient.getStatus(sessionId, getStatusResInfo) != SOAP_OK)
	{
		SOAPLOG(::ZQ::common::Log::L_ERROR, CLOGFMT(TMVSoapClientSession,"session(%d) encounter soap error when getStatus, fault=%s, detail=%s"), sessionId.c_str(), _tmvssSoapClient.fault->faultstring);
		return false;
	}
	else
	{
		SOAPLOG(::ZQ::common::Log::L_INFO, CLOGFMT(TMVSoapClientSession,"session(%d) getStatus soap call ok"), sessionId.c_str());
		return true;
	}
}