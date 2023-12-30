#include <WinSock2.h>

#include "TMVSS.nsmap"
#include "soapH.h"
#include "soapStub.h"
#include "stdsoap2.h"
#include "stdsoap2.cpp"

#include "TMVSSServer.h"

int TMVSSServer::run()
{
	if	(false == _urlStr.parse(_strLocalEndPoint.c_str()))
	{
		_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(TMVSSServer, "input URL(%s) format error"), _strLocalEndPoint.c_str());
		return -1;
	}
	int nCount = 1;

	//initialize the server service soap object
	while(!soap_valid_socket(bind(_urlStr.getHost(), _urlStr.getPort(), 100)))
	{
		_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(TMVSSServer, "Attempting to bind Notification soap service - count %d"), nCount++);
		sleep(2);
	}
	_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(TMVSSServer, "Notification SOAP bound at %s"), _strLocalEndPoint.c_str());

	//main loop to process client call
	while(1)
	{
		SOCKET nASock;

		send_timeout = 600;
		recv_timeout = 600;
		max_keep_alive = 1000;

		//listen for client remote call
		nASock = accept();
		if (::ZQ::common::NativeThread::getStatus() == ::ZQ::common::NativeThread::stDisabled)
		{
			_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(TMVSSServer, "get thread exit signal, stop thread"));
			closesocket(nASock);
			break;
		}
		if(nASock < 0)
		{
			//error occur and record
			_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(TMVSSServer, "client soap call accept failed, socket=%d, error=%s, detail=%s"), nASock, soap_faultstring(this), soap_faultdetail(this));
			closesocket(nASock);
			continue;
		}
		else
			_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(TMVSSServer, "Accepted soap client connection, IP=%d.%d.%d.%d socket=%d"), (ip>>24)&0xFF, (ip>>16)&0xFF, (ip>>8)&0xFF, ip&0xFF, nASock);

		//serve this client soap call
		if(serve() != SOAP_OK)
		{
			//soap_print_fault(&_pServerSoap, stdout);
			//record error
			_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(TMVSSServer, "client soap call serve fail, socket=%d, error=%s, detail=%s"), nASock, soap_faultstring(this), soap_faultdetail(this));
		}
		else
			_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(TMVSSServer, "client soap call serve success, socket=%d"), nASock);
	}
	return 1;
}

/// Web service operation 'setup' (returns error code or SOAP_OK)
int TMVSSServer::setup(ZQ2__setupInfo *setupInfo, struct ZQ2__setupResponse &_param_1)
{
	return SOAP_OK;
}

/// Web service operation 'teardown' (returns error code or SOAP_OK)
int TMVSSServer::teardown(std::string sessionId, int &ret)
{
	return SOAP_OK;
}

/// Web service operation 'getStatus' (returns error code or SOAP_OK)
int TMVSSServer::getStatus(std::string sessionId, struct ZQ2__getStatusResponse &_param_2)
{
	return SOAP_OK;
}

/// Web service operation 'notifyStatus' (returns error code or SOAP_OK)
int TMVSSServer::notifyStatus(ZQ2__notifyStatusInfo *notifyStatusInfo, int &ret)
{
	_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(TMVSSServer, "notifyStatus: cbNotification: %s, sessionId: %s, lastError: %s, status: %s"), notifyStatusInfo->ctxNotification.c_str(), notifyStatusInfo->sessionId.c_str(), notifyStatusInfo->lastError.c_str(), notifyStatusInfo->state.c_str());

	TMVSoapClientSession *tmpSoapClientSession = _soapClientMap.getSoapClient(notifyStatusInfo->sessionId);
	if (tmpSoapClientSession != NULL)
	{
		_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(TMVSSServer, "notifyStatus: find soap client session (%s)"), notifyStatusInfo->sessionId.c_str());
		if (notifyStatusInfo->state.compare("STOP") == 0)
		{
			//try to destroy TMVSStream
			tmpSoapClientSession->_bDestroy = true;
			_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(TMVSSServer, "notifyStatus:set soap client session(%s) to destroy status"), notifyStatusInfo->sessionId.c_str());
		}
	}
	else
	{
		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(TMVSSServer, "notifyStatus: can't find soap client session with sessionId(%s)"), notifyStatusInfo->sessionId.c_str());
	}

	
	ret = 1;
	return SOAP_OK;
}

///super class function initialize
/// Web service operation 'setup' (returns error code or SOAP_OK)
int TMVSSService::setup(ZQ2__setupInfo *setupInfo, struct ZQ2__setupResponse &_param_1)
{
	return SOAP_OK;
}

/// Web service operation 'teardown' (returns error code or SOAP_OK)
int TMVSSService::teardown(std::string sessionId, int &ret)
{
	return SOAP_OK;
}

/// Web service operation 'getStatus' (returns error code or SOAP_OK)
int TMVSSService::getStatus(std::string sessionId, struct ZQ2__getStatusResponse &_param_2)
{
	return SOAP_OK;
}

/// Web service operation 'notifyStatus' (returns error code or SOAP_OK)
int TMVSSService::notifyStatus(ZQ2__notifyStatusInfo *notifyStatusInfo, int &ret)
{
	return SOAP_OK;
}