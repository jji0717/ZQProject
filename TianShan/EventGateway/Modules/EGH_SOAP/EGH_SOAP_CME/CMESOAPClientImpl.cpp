#include "CMESOAPClientImpl.h"
// import the namespace map
#include "CMEServiceSoapBinding.nsmap"
#include <sstream>

namespace EventGateway{
namespace CMESOAPHelper{

namespace RequestData{

SessionNotification::SessionNotification():func(0){}
SessionNotification::SessionNotification(
    std::string theProviderID,
    std::string theProviderAssetID,
    int theFunc,
    std::string theClusterID,
    std::string theSessionID,
    std::string theTimeStamp
    )
    :providerID(theProviderID),
    providerAssetID(theProviderAssetID),
    func(theFunc),
    clusterID(theClusterID),
    sessionID(theSessionID),
    timeStamp(theTimeStamp)
{
}

std::string SessionNotification::printable()
{
    std::ostringstream buf;
    buf << "P=" << providerID
        << "  PA=" << providerAssetID
        << "  f=" << func << "(" << (func == 1 ? "start" : (func == 2 ? "stop" : "unknown")) << ")"
        << "  C=" << clusterID
        << "  S=" << sessionID
        << "  T=" << timeStamp;
    return buf.str();
}


} // namespace RequestData


using namespace ZQ::common;

// implement of ClientImpl

ClientImpl::ClientImpl(ZQ::common::Log &log, const SOAPClientConfig &cfg)
:_log(log), _config(cfg)
{
    if(_config.retryIntervalSec < CMEClient_RetryInterval_MIN)
        _config.retryIntervalSec = CMEClient_RetryInterval_MIN;
    else if(_config.retryIntervalSec > CMEClient_RetryInterval_MAX)
        _config.retryIntervalSec = CMEClient_RetryInterval_MAX;

    if(_config.retryCountMax < CMEClient_RetryCountMax_MIN)
        _config.retryCountMax = CMEClient_RetryCountMax_MIN;

    _quit = false;
    notifyNewRequest(); // start the serve cycle
    start();
}

ClientImpl::~ClientImpl()
{
    _quit = true;
    notifyNewRequest(); // signal to quit thread
    if(waitHandle(CMEClient_ExitTimeoutMSec))
    {
        // the thread can't quit normally
        _log(Log::L_WARNING, CLOGFMT(ClientImpl, "The thread can't quit normally in %d milliseconds, terminate it manually."), CMEClient_ExitTimeoutMSec);
        terminate(0);
    }
}

void ClientImpl::sessionNotification(const RequestData::SessionNotification &rqstData)
{
    ZQ::common::MutexGuard sync(_lockRqsts);
    _requests.push_back(rqstData);
    _log(Log::L_DEBUG, CLOGFMT(ClientImpl, "Request sessionNotification() P=%s, PA=%s, f=%d, C=%s, S=%s, T=%s. [%u] pending requests in the queue."),
        rqstData.providerID.c_str(),
        rqstData.providerAssetID.c_str(),
        rqstData.func,
        rqstData.clusterID.c_str(),
        rqstData.sessionID.c_str(),
        rqstData.timeStamp.c_str(),
        _requests.size());

    notifyNewRequest();
}

static void logSoapFault(struct soap* soap, ZQ::common::Log& log, const char* hint)
{
    if (soap_check_state(soap))
	{
        log(ZQ::common::Log::L_ERROR, "soap struct state not initialized");
		return;
	}
    
	if (!soap->error)
		return;

	const char *clazz, *subcode = NULL, *faultstr, **detail;
	detail = soap_faultcode(soap);
	if (!*detail)
		soap_set_fault(soap);
	clazz = *detail;
	if (soap->version == 2)
		subcode = *soap_faultsubcode(soap);
	faultstr  = *soap_faultstring(soap);
	detail    = soap_faultdetail(soap);

	if (NULL == clazz)		clazz = "";
	if (NULL == subcode) 	subcode = "n/a";
	if (NULL == faultstr)	faultstr = "n/a";

	log(ZQ::common::Log::L_ERROR, "%s SOAP 1.%d err %s(%s) %s: %s", (hint?hint:""), soap->version, clazz, subcode, faultstr, (detail && *detail) ? *detail : "");
}

int ClientImpl::run(void)
{
    CMEServiceSoapBinding* cmeSoapClient =NULL;
    timeout_t waitTimeoutMSec = TIMEOUT_INF; // the timeout of waiting new request

    int32 retryCount=0;
    bool sentOK = false;
    
	while (true)
	{
		waitTimeoutMSec = TIMEOUT_INF;
		if (!_requests.empty())
			waitTimeoutMSec = retryCount * 1000 * _config.retryIntervalSec; // retry after interval

		if (0 != waitTimeoutMSec)
			_hNotifyNewRequest.wait(waitTimeoutMSec);

		if(_quit)
			break;

		RequestData::SessionNotification rqstData;

		// take first request data from the queue
		try { // debugging on ticket#9910
			ZQ::common::MutexGuard sync(_lockRqsts);

			// discard the request if it had been send out successfully
			// or failed retryCountMax times
			if (sentOK || retryCount >= _config.retryCountMax)
			{
				if(!sentOK) // exceed the retry limit
					_log(Log::L_WARNING, CLOGFMT(ClientImpl, "discarding post sessionNotification[%s] after [%d] tries."), rqstData.printable().c_str(), retryCount);

				if(!_requests.empty())
					_requests.pop_front();

				// update the status
				sentOK = false;
				retryCount = 0;
				waitTimeoutMSec = TIMEOUT_INF; // reset the timeout for new request

				_log(Log::L_DEBUG, CLOGFMT(ClientImpl, "%u requests in the pending queue"), _requests.size());
			}

			// we can quit the thread safely now
			if(_quit)
				break;

			if(_requests.empty())
				continue; // wait for new request's arrival

			rqstData = _requests.front();
		} 
		catch (...) { _log(Log::L_ERROR, CLOGFMT(ClientImpl, "caught an exception in step 1")); }

		try {

			if (NULL == cmeSoapClient)
			{
				cmeSoapClient = new CMEServiceSoapBinding();
				if (NULL == cmeSoapClient)
					abort();

				// init the client with the config
				if(!_config.serviceEndpoint.empty())
					cmeSoapClient->endpoint =_config.serviceEndpoint.c_str();

				cmeSoapClient->soap->connect_timeout = _config.connectTimeout;
				cmeSoapClient->soap->send_timeout    = _config.sendTimeout;
				cmeSoapClient->soap->recv_timeout    = _config.recvTimeout;
				_log(Log::L_INFO, CLOGFMT(ClientImpl, "created new SOAP client to CME[%s]"), cmeSoapClient->endpoint);
			}
			
			// debugging on ticket#9910
			// now we got the request data
			struct ns1__sessionNotificationResponse sessNotificationResponse;
			int rqstResult = cmeSoapClient->ns1__sessionNotification(
				rqstData.providerID,
				rqstData.providerAssetID,
				rqstData.func,
				rqstData.clusterID,
				rqstData.sessionID,
				rqstData.timeStamp,
				sessNotificationResponse
				);

			_log(Log::L_DEBUG, CLOGFMT(ClientImpl, "post sessionNotification[%s] ret[%d]"), rqstData.printable().c_str(), rqstResult);

			switch (rqstResult)
			{
			case SOAP_OK:
				{
					// request successfully
					sentOK = true;
					notifyNewRequest(); // need next cycle to remove the request data
					_log(Log::L_INFO, CLOGFMT(ClientImpl, "post sessionNotification[%s] successfully."), rqstData.printable().c_str());
					continue;
				}
				break;

			case SOAP_FAULT:
			case SOAP_TCP_ERROR:
			case SOAP_UDP_ERROR:
			case SOAP_HTTP_ERROR:
			case SOAP_FATAL_ERROR:
			case SOAP_CLI_FAULT:
			case SOAP_SVR_FAULT:
				logSoapFault(cmeSoapClient->soap, _log, "destroying client due to");
				delete cmeSoapClient;
				cmeSoapClient = NULL;
				break;

			}
		}
		catch (...) { _log(Log::L_ERROR, CLOGFMT(ClientImpl, "caught an exception in step 2")); }

		++retryCount;
		_log(Log::L_WARNING, CLOGFMT(ClientImpl, "failed to post sessionNotification[%s], retry in [%d]msec, had retried [%d]times."),
			rqstData.printable().c_str(), waitTimeoutMSec, retryCount);

	} // while(true)

	if (NULL != cmeSoapClient)
		delete cmeSoapClient;
	cmeSoapClient = NULL;

	_log(Log::L_WARNING, CLOGFMT(ClientImpl, "soap invoker quits"));

	return 0;
}

} // namespace CMESOAPHelper
} // namespace EventGateway
