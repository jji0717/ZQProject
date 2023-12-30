#ifndef __ZQTianShan_CiscoAIMSoap12Impl_H__
#define __ZQTianShan_CiscoAIMSoap12Impl_H__

//include ZQ common dll and lib header files
#include "FileLog.h"
#include "urlstr.h"
#include "NativeThread.h"

//include Cisco AIM lib file header files
#include "CiscoAIMSoap12.nsmap"
#include "soapCiscoAIMSoap12Proxy.h"
#include "soapCiscoAIMNotificationSoap12Service.h"
#include "soapCiscoAIMNotificationSoap12Proxy.h"
#include "soapH.h"
#include "soapStub.h"

namespace ZQTianShan{

namespace CVSS{

#define DEFAULTENDPOINT "http://localhost:9793"

typedef ::std::list<_CISCO2__AIMPackageNotification*> AIMPackageNotificaionList;
typedef struct SoapAIMNotifyResponesList
{
	AIMPackageNotificaionList _aimPackageNotificationList;

	bool pushBack(_CISCO2__AIMPackageNotification *inStruct)
	{
		if (inStruct == NULL)//input param is null
			return false;

		//otherwise, push back to list
		::ZQ::common::MutexGuard guard(_mutex);
		_aimPackageNotificationList.push_back(inStruct);
		return true;
	}

	_CISCO2__AIMPackageNotification *popFront()
	{
		//no data in list
		if (_aimPackageNotificationList.empty())
			return NULL;

		//otherwise, get the first data and return the address
		::ZQ::common::MutexGuard guard(_mutex);
		AIMPackageNotificaionList::iterator iter = _aimPackageNotificationList.begin();
		_CISCO2__AIMPackageNotification *outStruct = (*iter);
		_aimPackageNotificationList.pop_front();
		return outStruct;
	}

private:
	::ZQ::common::Mutex _mutex;
}SoapAIMNotifyResponesList;

//use soap 1.2 protocol
class CiscoAIMSoap12Impl : public CiscoAIMNotificationSoap12Service,
						 ::ZQ::common::NativeThread
{
public:
	CiscoAIMSoap12Impl(::ZQ::common::FileLog *logfile = NULL, const char *localendPoint = NULL, const char *remoteEndPoint = NULL);
	~CiscoAIMSoap12Impl();

	//function derive from ::ZQ::common::NativeThread
	int run(void);
	int terminate(int code=0);

	//function derive from CiscoAIMNotificationSoap11Service
	/// Web service operation 'AIMPackageNotification' (returns error code or SOAP_OK)
	virtual	int AIMPackageNotification(_CISCO2__AIMPackageNotification *CISCO2__AIMPackageNotification, _CISCO2__AIMPackageNotificationResponse *CISCO2__AIMPackageNotificationResponse);

	//self function
	inline void setLogger(::ZQ::common::FileLog *logfile){_pFileLog = logfile;}
	bool setLocalEndPoint(const char *localEndPoint);
	bool setRemoteEndPoint(const char *remoteEndPoint);

	int IngestPackage(_CISCO1__IngestPackage &inStruct, _CISCO1__IngestPackageResponse &outStruct);
	int DeletePackage(_CISCO1__DeletePackage &inStruct, _CISCO1__DeletePackageResponse &outStruct);
	int UpdatePackage(_CISCO1__UpdatePackage &inStruct, _CISCO1__UpdatePackageResponse &outStruct);
	int GetPackageStatus(_CISCO1__GetPackageStatus &inStruct, _CISCO1__GetPackageStatusResponse &outStruct);
	int GetAllPackages(char *pLocation, _CISCO1__GetAllPackagesResponse &outStruct);

	const char *printSoapFault(struct SOAP_ENV__Fault *soapFault);

	SoapAIMNotifyResponesList _soapAIMNotifyResponseList;
private:
	::ZQ::common::FileLog	*_pFileLog;				//log file pointer
	bool					_bInitSymbol;			//specify if the initialize of this class success

	//soap server information variables
	//struct soap				_pServerSoap;			//soap object for initialize soap service
	CiscoAIMNotificationSoap12Proxy	_ciscoAIMNotificationSoap12Proxy;
	::std::string			_strLocalEndPoint;		//local soap service access endpoint
	::ZQ::common::URLStr	_urlStr;				//class to parse the _strEndPoint

	//soap client information variables
	CiscoAIMSoap12Proxy		_ciscoAIMSoap12Proxy;	//client soap object to send soap request
	struct soap				_pClientSoap;			//soap object for call remote soap service
	::std::string			_strRemoteEndPoint;		//remote soap service access endpoint
};

#define SOAPLOG if (_pFileLog) (*_pFileLog)
}//namespace CVSS

}//namespace ZQ

#endif __ZQTianShan_CiscoAIMSoap12Impl_H__