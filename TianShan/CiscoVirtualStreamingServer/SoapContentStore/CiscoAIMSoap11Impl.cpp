#include <winsock2.h>
#include "CiscoAIMSoap11Impl.h"
//#include "CiscoAIMSoap11.nsmap"
#include "CiscoAIMNotificationSoap11.nsmap"
#include "soapH.h"
#include "soapStub.h"
#include "stdsoap2.h"
#include "stdsoap2.cpp"

namespace ZQTianShan{

namespace CVSS{

CiscoAIMSoap11Impl::CiscoAIMSoap11Impl(ZQ::common::FileLog *logfile, const char *localEndPoint, const char *remoteEndPoint)
:_pFileLog(logfile)
,_strLocalEndPoint(localEndPoint)
,_strRemoteEndPoint(remoteEndPoint)
{
	//init server soap object
	soap_init(this);

	_bInitSymbol = true;

	//set to default local endpoint if null endpoint given
	if (NULL == localEndPoint)
	{
		SOAPLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(CiscoAIMSoap11Impl,"input endpoint is NULL, set to default(%s)"), DEFAULTENDPOINT);
		setLocalEndPoint(DEFAULTENDPOINT);
	}
	else
		setLocalEndPoint(localEndPoint);

	//check the remote endpoint, init client side soap struct
	if (NULL == remoteEndPoint)
	{
		SOAPLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(CiscoAIMSoap11Impl,"input URL is NULL, please call setRemoteEdnPoint function"), DEFAULTENDPOINT);
		_bInitSymbol = false;
	}
	else
		setRemoteEndPoint(remoteEndPoint);
}

CiscoAIMSoap11Impl::~CiscoAIMSoap11Impl()
{
	//destroy server soap service
	soap_destroy(this);
	soap_end(this);
	soap_done(this);

	//destroy client soap service
	soap_destroy(&_ciscoAIMSoap11Proxy);
	soap_end(&_ciscoAIMSoap11Proxy);
	soap_done(&_ciscoAIMSoap11Proxy);
}

void CiscoAIMSoap11Impl::setLocalEndPoint(const char *localEndPoint)
{
	//set end point
	_strLocalEndPoint = localEndPoint;
	if	(false == _urlStr.parse(_strLocalEndPoint.c_str()))
	{
		SOAPLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CiscoAIMSoap11Impl,"input URL(%s) format error"), _strLocalEndPoint.c_str());
		_bInitSymbol = false;
	}
	else
	{
		SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap11Impl,"input URL(%s), set local endpoint ok"), _strLocalEndPoint.c_str());
		memcpy(endpoint, _strLocalEndPoint.c_str(), _strLocalEndPoint.length());
		_bInitSymbol = true;
	}
}

void CiscoAIMSoap11Impl::setRemoteEndPoint(const char *remoteEndPoint)
{
	//set end point
	if (NULL != remoteEndPoint)
	{
		//set client soap information
		_strRemoteEndPoint = remoteEndPoint;		
		
		//set client soap object
		soap_init(&_ciscoAIMSoap11Proxy);
		_ciscoAIMSoap11Proxy.soap_endpoint = _strRemoteEndPoint.c_str();

		SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap11Impl,"Remote Soap Service Endpoint is %d, init soap client object"), remoteEndPoint);
	}
	else
		//log error
		SOAPLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CiscoAIMSoap11Impl,"Remote Soap Service Endpoint is NULL"));
}

int CiscoAIMSoap11Impl::run(void)
{
	//initialize fail and don't start this thread
	if (!_bInitSymbol)
		return -1;

	int nLSock = -1;
	int nCount = 1;

	//initialize the server service soap object
	while(!soap_valid_socket(bind(_urlStr.getHost(), _urlStr.getPort(), 100)))
	{
		SOAPLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(CiscoAIMSoap11Impl,"Attempting to bind Notification soap service - count %d"), nCount++);
		sleep(2);
	}
	SOAPLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(CiscoAIMSoap11Impl,"Notification SOAP bound at %s"), _strLocalEndPoint.c_str());

	//main loop to process client call
	while(1)
	{
		SOCKET nASock;
		
		send_timeout = 600;
		recv_timeout = 600;
		max_keep_alive = 1000;

		//listen for client remote call
		nASock = accept();

		if (getStatus() == ::ZQ::common::NativeThread::stDisabled)
		{
			SOAPLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CiscoAIMSoap11Impl,"get thread exit signal, stop thread"));
			closesocket(nASock);
			break;
		}
		if(nASock < 0)
		{
			//error occur and record
			SOAPLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CiscoAIMSoap11Impl,"client soap call accept failed, socket=%d, error=%s, detail=%s"), nASock, soap_faultstring(this), soap_faultdetail(this));
			closesocket(nASock);
			continue;
		}
		else
			SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap11Impl,"Accepted soap client connection, IP=%d.%d.%d.%d socket=%d"), (ip>>24)&0xFF, (ip>>16)&0xFF, (ip>>8)&0xFF, ip&0xFF, nASock);
		
		//serve this client soap call
		if(serve() != SOAP_OK)
		{
			//soap_print_fault(&_pServerSoap, stdout);
			//record error
			SOAPLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CiscoAIMSoap11Impl,"client soap call serve fail, socket=%d, error=%s, detail=%s"), nASock, soap_faultstring(this), soap_faultdetail(this));
		}
		else
			SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap11Impl,"client soap call serve success, socket=%d"), nASock);
	}

	//release all resource of server soap object and close socket
	return 1;
}

int	CiscoAIMSoap11Impl::terminate(int code)
{
	setStatus(::ZQ::common::NativeThread::stDisabled);

	//make a soap call to wake up the soap_accept function
	_CISCO2__AIMPackageNotification in;
	_CISCO2__AIMPackageNotificationResponse out;
	in.PackageName = "SHUTDOWN";
	in.ADIURL = "SHUTDOWN";
	in.Result = "SHUTDOWN";

	soap_init(&_ciscoAIMNotificationSoap11Proxy);
	_ciscoAIMNotificationSoap11Proxy.soap_endpoint = _strLocalEndPoint.c_str();

	int res = 0;

	if(_ciscoAIMNotificationSoap11Proxy.AIMPackageNotification(&in,&out))
	{
		SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap11Impl,"Error shutting down notification interface"));
		res = 1;
	}
	else
	{
		SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap11Impl,"Notification interface shutdown"));
		res = -1;
	}

	soap_destroy(&_ciscoAIMNotificationSoap11Proxy);
	soap_end(&_ciscoAIMNotificationSoap11Proxy);
	soap_done(&_ciscoAIMNotificationSoap11Proxy);
	return res;
}

int CiscoAIMSoap11Impl::IngestPackage(_CISCO1__IngestPackage &inStruct,
									 _CISCO1__IngestPackageResponse &outStruct)
{
	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap11Impl,"call IngestPackage function"));
	if (*inStruct.DoAsync == 1)
	{
		inStruct.AsyncTargets->ServerIp.push_back(_urlStr.getHost());
		inStruct.AsyncTargets->Port.push_back(_urlStr.getPort());
		inStruct.AsyncTargets->Path.push_back(_urlStr.getPath());
	}
	return _ciscoAIMSoap11Proxy.IngestPackage(&inStruct, &outStruct);
}

int CiscoAIMSoap11Impl::DeletePackage(_CISCO1__DeletePackage &inStruct,
									 _CISCO1__DeletePackageResponse &outStruct)
{
	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap11Impl,"call DeletePackage function"));
	return _ciscoAIMSoap11Proxy.DeletePackage(&inStruct, &outStruct);
}

int CiscoAIMSoap11Impl::UpdatePackage(_CISCO1__UpdatePackage &inStruct,
									 _CISCO1__UpdatePackageResponse &outStruct)
{
	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap11Impl,"call UpdatePackage function"));
	if (*inStruct.DoAsync == 1)
	{
		inStruct.AsyncTargets->ServerIp.push_back(_urlStr.getHost());
		inStruct.AsyncTargets->Port.push_back(_urlStr.getPort());
		inStruct.AsyncTargets->Path.push_back(_urlStr.getPath());
	}
	return _ciscoAIMSoap11Proxy.UpdatePackage(&inStruct, &outStruct);
}

int CiscoAIMSoap11Impl::GetPackageStatus(_CISCO1__GetPackageStatus &inStruct, 
										_CISCO1__GetPackageStatusResponse &outStruct)
{
	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap11Impl,"call GetPackageStatus function"));
	return _ciscoAIMSoap11Proxy.GetPackageStatus(&inStruct, &outStruct);
}

int CiscoAIMSoap11Impl::GetAllPackages(char *pLocation, 
									  _CISCO1__GetAllPackagesResponse &outStruct)
{
	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap11Impl,"call GetAllPackages function"));
	return _ciscoAIMSoap11Proxy.GetAllPackages(pLocation, &outStruct);
}

int CiscoAIMSoap11Impl::AIMPackageNotification(_CISCO2__AIMPackageNotification *CISCO2__AIMPackageNotification, 
							_CISCO2__AIMPackageNotificationResponse *CISCO2__AIMPackageNotificationResponse)
{
	int nRC = 0;

	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap11Impl,"*****SOAP 1.1***PackageNotification incoming*****"));

	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap11Impl,"*** PackageName : %s"),
																CISCO2__AIMPackageNotification->PackageName);

	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap11Impl,"*** ADIURL : %s"),
																CISCO2__AIMPackageNotification->ADIURL);

	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap11Impl,"*** Ingest result : %s"),
																CISCO2__AIMPackageNotification->Result);

	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap11Impl,"***SOAP 1.1***PackageNotification complete********************"));

	CISCO2__AIMPackageNotificationResponse->NotificationResult = &nRC;
	_soapAIMNotifyResponseList.pushBack(CISCO2__AIMPackageNotification);
	return 0;
}

const char *CiscoAIMSoap11Impl::printSoapFault(struct SOAP_ENV__Fault *soapFault)
{
	::std::string res;
	res = ::std::string("FaultCode: ") + soapFault->faultcode + ", detail: " + (char *)soapFault->detail->fault;
	return res.c_str();
}

}//namespace CVSS

}//namespace ZQ

int CiscoAIMNotificationSoap11Service::AIMPackageNotification(_CISCO2__AIMPackageNotification *CISCO2__AIMPackageNotification, _CISCO2__AIMPackageNotificationResponse *CISCO2__AIMPackageNotificationResponse)
{
	return 0;
}