#include <winsock2.h>
#include "CiscoAIMSoap12Impl.h"
#include "CiscoAIMNotificationSoap12.nsmap"
#include "soapH.h"
#include "soapStub.h"
#include "stdsoap2.h"
#include "stdsoap2.cpp"

namespace ZQTianShan{

namespace CVSS{

CiscoAIMSoap12Impl::CiscoAIMSoap12Impl(ZQ::common::FileLog *logfile, const char *localEndPoint, const char *remoteEndPoint)
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
		SOAPLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(CiscoAIMSoap12Impl,"input endpoint is NULL, set to default(%s)"), DEFAULTENDPOINT);
		setLocalEndPoint(DEFAULTENDPOINT);
	}
	else
		setLocalEndPoint(localEndPoint);

	//check the remote endpoint, init client side soap struct
	if (NULL == remoteEndPoint)
	{
		SOAPLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(CiscoAIMSoap12Impl,"input URL is NULL, please call setRemoteEdnPoint function"), DEFAULTENDPOINT);
		_bInitSymbol = false;
	}
	else
		setRemoteEndPoint(remoteEndPoint);
}

CiscoAIMSoap12Impl::~CiscoAIMSoap12Impl()
{
}

bool CiscoAIMSoap12Impl::setLocalEndPoint(const char *localEndPoint)
{
	//set end point
	_strLocalEndPoint = localEndPoint;
	if	(false == _urlStr.parse(_strLocalEndPoint.c_str()))
	{
		SOAPLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CiscoAIMSoap12Impl,"input URL(%s) format error"), _strLocalEndPoint.c_str());
		_bInitSymbol = false;
	}
	else
	{
		SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap12Impl,"input URL(%s), set local endpoint ok"), _strLocalEndPoint.c_str());
		memcpy(endpoint, _strLocalEndPoint.c_str(), _strLocalEndPoint.length());
		_bInitSymbol = true;
	}
}

bool CiscoAIMSoap12Impl::setRemoteEndPoint(const char *remoteEndPoint)
{
	//set end point
	if (NULL != remoteEndPoint)
	{
		//set client soap information
		_strRemoteEndPoint = remoteEndPoint;		
		
		//set client soap object
		soap_init(&_ciscoAIMSoap12Proxy);
		_ciscoAIMSoap12Proxy.soap_endpoint = _strRemoteEndPoint.c_str();

		SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap12Impl,"Remote Soap Service Endpoint is %d, init soap client object"), remoteEndPoint);
	}
	else
		//log error
		SOAPLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CiscoAIMSoap12Impl,"Remote Soap Service Endpoint is NULL"));
}

int CiscoAIMSoap12Impl::run(void)
{
	//initialize fail and don't start this thread
	if (!_bInitSymbol)
		return -1;

	int nLSock = -1;
	int nCount = 1;

	//initialize the server service soap object
	while(!soap_valid_socket(bind(_urlStr.getHost(), _urlStr.getPort(), 100)))
	{
		SOAPLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(CiscoAIMSoap12Impl,"Attempting to bind Notification soap service - count %d"), nCount++);
		sleep(2);
	}
	SOAPLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(CiscoAIMSoap12Impl,"Notification SOAP bound at %s"), _strLocalEndPoint.c_str());

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
			SOAPLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CiscoAIMSoap12Impl,"get thread exit signal, stop thread"));
			closesocket(nASock);
			break;
		}
		if(nASock < 0)
		{
			//error occur and record
			SOAPLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CiscoAIMSoap12Impl,"client soap call accept failed, socket=%d, error=%s, detail=%s"), nASock, soap_faultstring(this), soap_faultdetail(this));
			closesocket(nASock);
			continue;
		}
		else
			SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap12Impl,"Accepted soap client connection, IP=%d.%d.%d.%d socket=%d"), (ip>>24)&0xFF, (ip>>16)&0xFF, (ip>>8)&0xFF, ip&0xFF, nASock);
		
		//serve this client soap call
		if(serve() != SOAP_OK)
		{
			//soap_print_fault(&_pServerSoap, stdout);
			//record error
			SOAPLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CiscoAIMSoap12Impl,"client soap call serve fail, socket=%d, error=%s, detail=%s"), nASock, soap_faultstring(this), soap_faultdetail(this));
		}
		else
			SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap12Impl,"client soap call serve success, socket=%d"), nASock);
	}

	//release all resource of server soap object and close socket
	return 1;
}

int	CiscoAIMSoap12Impl::terminate(int code)
{
	setStatus(::ZQ::common::NativeThread::stDisabled);

	//make a soap call to wake up the soap_accept function
	_CISCO2__AIMPackageNotification in;
	_CISCO2__AIMPackageNotificationResponse out;
	in.PackageName = "SHUTDOWN";
	in.ADIURL = "SHUTDOWN";
	in.Result = "SHUTDOWN";

	soap_init(&_ciscoAIMNotificationSoap12Proxy);
	_ciscoAIMNotificationSoap12Proxy.soap_endpoint = _strLocalEndPoint.c_str();

	if(_ciscoAIMNotificationSoap12Proxy.AIMPackageNotification(&in,&out))
		SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap12Impl,"Error shutting down notification interface!"));
	else
		SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap12Impl,"Notification interface shutdown"));

	return 1;
}

int CiscoAIMSoap12Impl::IngestPackage(_CISCO1__IngestPackage &inStruct,
									 _CISCO1__IngestPackageResponse &outStruct)
{
	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap12Impl,"call IngestPackage function"));
	if (*inStruct.DoAsync == 1)
	{
		inStruct.AsyncTargets->ServerIp.push_back(_urlStr.getHost());
		inStruct.AsyncTargets->Port.push_back(_urlStr.getPort());
		inStruct.AsyncTargets->Path.push_back(_urlStr.getPath());
	}
	return _ciscoAIMSoap12Proxy.IngestPackage(&inStruct, &outStruct);
}

int CiscoAIMSoap12Impl::DeletePackage(_CISCO1__DeletePackage &inStruct,
									 _CISCO1__DeletePackageResponse &outStruct)
{
	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap12Impl,"call DeletePackage function"));
	return _ciscoAIMSoap12Proxy.DeletePackage(&inStruct, &outStruct);
}

int CiscoAIMSoap12Impl::UpdatePackage(_CISCO1__UpdatePackage &inStruct,
									 _CISCO1__UpdatePackageResponse &outStruct)
{
	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap12Impl,"call UpdatePackage function"));
	if (*inStruct.DoAsync == 1)
	{
		inStruct.AsyncTargets->ServerIp.push_back(_urlStr.getHost());
		inStruct.AsyncTargets->Port.push_back(_urlStr.getPort());
		inStruct.AsyncTargets->Path.push_back(_urlStr.getPath());
	}
	return _ciscoAIMSoap12Proxy.UpdatePackage(&inStruct, &outStruct);
}

int CiscoAIMSoap12Impl::GetPackageStatus(_CISCO1__GetPackageStatus &inStruct, 
										_CISCO1__GetPackageStatusResponse &outStruct)
{
	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap12Impl,"call GetPackageStatus function"));
	return _ciscoAIMSoap12Proxy.GetPackageStatus(&inStruct, &outStruct);
}

int CiscoAIMSoap12Impl::GetAllPackages(char *pLocation, 
									  _CISCO1__GetAllPackagesResponse &outStruct)
{
	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap12Impl,"call GetAllPackages function"));
	return _ciscoAIMSoap12Proxy.GetAllPackages(pLocation, &outStruct);
}

int CiscoAIMSoap12Impl::AIMPackageNotification(_CISCO2__AIMPackageNotification *CISCO2__AIMPackageNotification, 
							_CISCO2__AIMPackageNotificationResponse *CISCO2__AIMPackageNotificationResponse)
{
	int nRC = 0;

	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap12Impl,"*****SOAP 1.2***PackageNotification incoming*****"));

	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap12Impl,"*** PackageName : %s"),
																CISCO2__AIMPackageNotification->PackageName);

	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap12Impl,"*** ADIURL : %s"),
																CISCO2__AIMPackageNotification->ADIURL);

	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap12Impl,"*** Ingest result : %s"),
																CISCO2__AIMPackageNotification->Result);

	SOAPLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoAIMSoap12Impl,"***SOAP 1.2***PackageNotification complete********************"));

	CISCO2__AIMPackageNotificationResponse->NotificationResult = &nRC;
	_soapAIMNotifyResponseList.pushBack(CISCO2__AIMPackageNotification);
	return 0;
}

const char *CiscoAIMSoap12Impl::printSoapFault(struct SOAP_ENV__Fault *soapFault)
{
	::std::string res;
	res = ::std::string("FaultCode: ") + soapFault->faultcode + ", detail: " + (char *)soapFault->detail->fault;
	return res.c_str();
}

}//namespace CVSS

}//namespace ZQ


int CiscoAIMNotificationSoap12Service::AIMPackageNotification(_CISCO2__AIMPackageNotification *CISCO2__AIMPackageNotification, _CISCO2__AIMPackageNotificationResponse *CISCO2__AIMPackageNotificationResponse)
{
	return 0;
}