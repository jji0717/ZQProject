// CMESoapServerImpl.cpp : Started as a thread from CMESoapClient.cpp
//
#define  CME_SOAP_SERVER_TIMEOUT    (24*60*60) // timeout after 24hrs of inactivity

#define  PRIORITY_NORMAL				50
#define  PRIORITY_SESSION_NOTIFICATION	PRIORITY_NORMAL
#define  PRIORITY_CONTENT_DELETE		PRIORITY_NORMAL + 10
#define  PRIORITY_CONTENT_IMPORT		PRIORITY_NORMAL + 10
#define  PRIORITY_CONTENT_IMPORT_DONE	PRIORITY_NORMAL - 10


#include "CMESvc.h"
#include "CMESOAPStub.h"

extern CacheManagement::CMESvc g_server;


/*
 * We define a namespace here because we have to. CME uses two different .WSDL files,
 * one for client access to LAM and one for server access from LAM. When you build a
 * .WSDL file you get a bunch of "free" code tossed in. When you build two .WSDL files,
 * well, you get two bunches of "free" code tossed in. And the "free" code uses the
 * same global names in both cases. To avoid that you can build the .WSDL files with a
 * namespace switch and avoid multiply-defined globals. But that in turn means those
 * routines that interface with SOAP need to have the same namespace, hence what you
 * see here.
 *
 */
namespace CMESOAP {


/*
	This command is to notify CME the caching content succeed or fail.

	_status
	10 每 Cache Content Completed
	11 每 Cached Content Streamable (PWE)
	21 每 Cache Content Fail with read source content
	22 每 Cache Content Fail with No Space
	30 每 Cached Content Deleted
	500 每 Cache Content Fail with Internal Server Error
*/

SOAP_FMAC5 int SOAP_FMAC6 ns1__cacheNotification(
	struct soap* soap, 
	std::string _providerID, 
	std::string _providerAssetID, 
	std::string _clusterID, 
	LONG64 _status, 
	struct ns1__cacheNotificationResponse &_param_1)
{
	// log the entry
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CMESOAPStub, "cacheNotification: clusterID=%s pid=%s paid=%s, not supported, it is for legacy system"),
		_clusterID.c_str(), _providerID.c_str(), _providerAssetID.c_str());

	// do nothing, this API is for legacy system

    return SOAP_OK;	// SOAP_OK: return HTTP 202 ACCEPTED 
}

/*
	This command is to notify CME to force ingest a content for pro-active cache purpose

	_increment
	The value will be added to the asset's play count, raising or possibly lowering asset popularity. 
	If the resulting popularity value is above the current import threshold, 
	CME will ask VSIS to begin importing the asset immediately.

	_lifetime
	The specified lifetime for the content in seconds. Generally the asset won＊t be deleted till reach the end of life time. 
	CME might reduce the lifetime runtime based on IDLE allocution.
	If lifetime is <=0, CME will take its configuration as the default lifetime.
*/

SOAP_FMAC5 int SOAP_FMAC6 ns1__importNotification(
	struct soap* soap, 
	std::string _providerID, 
	std::string _providerAssetID, 
	std::string _clusterID, 
	LONG64 _increment, 
	LONG64 _lifetime, 
	LONG64 & _param_1)
{
	// log the entry
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CMESOAPStub, "importNotification: clusterID=%s pid=%s paid=%s increment=%d lifetime=%d seconds"),
		_clusterID.c_str(), _providerID.c_str(), _providerAssetID.c_str(), _increment, _lifetime);

	g_server.getCMEMain().proactiveImport(_clusterID, _providerID, _providerAssetID, _increment, _lifetime);

	return SOAP_OK;	// SOAP_OK: return HTTP 202 ACCEPTED 
}  // ns1__importNotification()

/*
	This command is to notify CME that an asset is no longer playable and can be deleted. 
	A normal (0) response indicates CME takes responsibility for deleting the asset soon after the last session finishes.
*/
SOAP_FMAC5 int SOAP_FMAC6 ns1__deletableNotification(
	struct soap* soap, 
	std::string _providerID, 
	std::string _providerAssetID,
	std::string _clusterID, 
	LONG64 & _param_1)
{
	// log the entry
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CMESOAPStub, "deletableNotification: clusterID=%s pid=%s paid=%s"),
		_clusterID.c_str(), _providerID.c_str(), _providerAssetID.c_str());

	g_server.getCMEMain().proactiveDelete(_clusterID, _providerID, _providerAssetID);

	return SOAP_OK;	// SOAP_OK: return HTTP 202 ACCEPTED 
}  // ns1__deletableNotification()

/*
	This command is to notify CME that a stream session started or completed

	_func
	1 每 Session Start
	2 每 Session Stop
	
	_timeStamp
	UTC time format
*/
SOAP_FMAC5 int SOAP_FMAC6 ns1__sessionNotification(
	struct soap*, 
	std::string _providerID, 
	std::string _providerAssetID, 
	int _func, 
	std::string _clusterID, 
	std::string _sessionID, 
	std::string _timeStamp, 
	struct ns1__sessionNotificationResponse &_param_2)
{
	// log the entry
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CMESOAPStub, "sessionNotification: clusterID=%s func=%d pid=%s paid=%s sessid=%s timestamp=%s"),
		_clusterID.c_str(), _func, _providerID.c_str(), _providerAssetID.c_str(), _sessionID.c_str(), _timeStamp.c_str());

	g_server.getCMEMain().sessionArrive(_clusterID, _func, _providerID, _providerAssetID, _sessionID, _timeStamp);

	return SOAP_OK;	// SOAP_OK: return HTTP 202 ACCEPTED 
}  // ns1__sessionNotification()


}  // namespace CMESOAP
