#ifndef __ZQTianShan_CiscoAIMSoap11Impl_H__
#define __ZQTianShan_CiscoAIMSoap11Impl_H__

//include std header
#include <list>

//include ZQ common dll and lib header files
#include "FileLog.h"
#include "Locks.h"
#include "urlstr.h"
#include "NativeThread.h"

//include Cisco AIM lib file header files
#include "soapCiscoAIMSoap11Proxy.h"
#include "soapCiscoAIMNotificationSoap11Service.h"
#include "soapCiscoAIMNotificationSoap11Proxy.h"

static const char* UNKNOWN = "Unknown";
static const char* PENDING = "PENDING";
static const char* TRANSFER = "INCOMPLETE";
static const char* STREAMABLE = "Transfer/Play";
static const char* COMPLETE = "COMPLETE";
static const char* CANCELED = "Canceled";
static const char* FAILED = "FAILED";

namespace ZQTianShan{

namespace CVSS{

#define DEFAULTENDPOINT "http://localhost:9792"

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

	size_t size()
	{
		::ZQ::common::MutexGuard guard(_mutex);
		return _aimPackageNotificationList.size();
	}

private:
	::ZQ::common::Mutex _mutex;
}SoapAIMNotifyResponesList;

//use soap 1.1 protocol
class CiscoAIMSoap11Impl : public CiscoAIMNotificationSoap11Service,
						   public ::ZQ::common::NativeThread
{
public:
	CiscoAIMSoap11Impl(::ZQ::common::FileLog *logfile, const char *localendPoint, const char *remoteEndPoint);
	~CiscoAIMSoap11Impl();

	//function derive from ::ZQ::common::NativeThread
	int run(void);
	int terminate(int code=0);

	//function derive from CiscoAIMNotificationSoap11Service
	/// Web service operation 'AIMPackageNotification' (returns error code or SOAP_OK)
	virtual	int AIMPackageNotification(_CISCO2__AIMPackageNotification *CISCO2__AIMPackageNotification, _CISCO2__AIMPackageNotificationResponse *CISCO2__AIMPackageNotificationResponse);

	//self function
	inline void setLogger(::ZQ::common::FileLog *logfile){_pFileLog = logfile;}
	void setLocalEndPoint(const char *localEndPoint);
	void setRemoteEndPoint(const char *remoteEndPoint);

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
	CiscoAIMNotificationSoap11Proxy	_ciscoAIMNotificationSoap11Proxy;
	::std::string			_strLocalEndPoint;		//local soap service access endpoint
	::ZQ::common::URLStr	_urlStr;				//class to parse the _strEndPoint

	//soap client information variables
	CiscoAIMSoap11Proxy		_ciscoAIMSoap11Proxy;	//client soap object to send soap request
	struct soap				_pClientSoap;			//soap object for call remote soap service
	::std::string			_strRemoteEndPoint;		//remote soap service access endpoint
};

#define SOAPLOG if (_pFileLog) (*_pFileLog)
}//namespace CVSS

}//namespace ZQ

#endif __ZQTianShan_CiscoAIMSoap11Impl_H__