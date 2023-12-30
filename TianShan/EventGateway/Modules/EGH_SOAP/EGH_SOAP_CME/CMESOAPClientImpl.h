#ifndef __TianShan_EventGw_Module_CMESOAP_ClientImpl_H__
#define __TianShan_EventGw_Module_CMESOAP_ClientImpl_H__

#include <ZQ_common_conf.h>
#include "CMESOAPCMEServiceSoapBindingProxy.h"
#include "CMESOAPConfig.h"
#include <Log.h>
#include <NativeThread.h>
#include <Locks.h>
#include <deque>
#include "SystemUtils.h"

#define CMEClient_RetryInterval_MIN 1 // seconds 
#define CMEClient_RetryInterval_MAX 5 // seconds 
#define CMEClient_RetryCountMax_MIN 1
#define CMEClient_ExitTimeoutMSec   5000 // millisecond

namespace EventGateway{
namespace CMESOAPHelper{

namespace RequestData{

struct SessionNotification
{
    std::string providerID;
    std::string providerAssetID;
    int func;
    std::string clusterID;
    std::string sessionID;
    std::string timeStamp;

    SessionNotification();
    SessionNotification(
        std::string theProviderID,
        std::string theProviderAssetID,
        int theFunc,
        std::string theClusterID,
        std::string theSessionID,
        std::string theTimeStamp
        );
    std::string printable();
};

} // namespace RequestData



class ClientImpl : public ZQ::common::NativeThread
{
public:
    explicit ClientImpl(ZQ::common::Log &log, const SOAPClientConfig&);
    ~ClientImpl();
    void sessionNotification(const RequestData::SessionNotification &rqstData);
private:
    virtual int run(void);
    void notifyNewRequest()
    {
        _hNotifyNewRequest.signal();
    }

private:
    std::deque<RequestData::SessionNotification> _requests;
    ZQ::common::Mutex _lockRqsts;

    ZQ::common::Log &_log;

    bool _quit;
    SYS::SingleObject _hNotifyNewRequest;
    SOAPClientConfig _config;
};
} // namespace CMESOAPHelper
} // namespace EventGateway

#endif
