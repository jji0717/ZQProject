#ifndef __TianShan_EventGw_Module_CMESOAP_Config_H__
#define __TianShan_EventGw_Module_CMESOAP_Config_H__
#include <ConfigHelper.h>
namespace EventGateway{
namespace CMESOAPHelper{

struct SOAPClientConfig
{
    std::string serviceEndpoint;

    int32 connectTimeout;
    int32 sendTimeout;
    int32 recvTimeout;

    int32 retryIntervalSec;
    int32 retryCountMax;
    static void structure(ZQ::common::Config::Holder<SOAPClientConfig>& holder)
    {
        using namespace ZQ::common::Config;
        holder.addDetail("CMEService", "endpoint", &SOAPClientConfig::serviceEndpoint, "", optReadOnly);
        holder.addDetail("Timeout", "connection", &SOAPClientConfig::connectTimeout, "5", optReadOnly);
        holder.addDetail("Timeout", "send", &SOAPClientConfig::sendTimeout, "5", optReadOnly);
        holder.addDetail("Timeout", "receive", &SOAPClientConfig::recvTimeout, "5", optReadOnly);
        holder.addDetail("Retry", "intevalSec", &SOAPClientConfig::retryIntervalSec, "1", optReadOnly);
        holder.addDetail("Retry", "maxCount", &SOAPClientConfig::retryCountMax, "10", optReadOnly);
    }
};

struct CMESOAPConfig
{
    int32 logLevel;
    int32 logFileSize;
    int32 logCount;

    int32 ignoreLocal;

    ZQ::common::Config::Holder<SOAPClientConfig> client;
    static void structure(ZQ::common::Config::Holder<CMESOAPConfig>& holder)
    {
        using namespace ZQ::common::Config;
        holder.addDetail("EGH_SOAP_CME/Log", "level", &CMESOAPConfig::logLevel, "7", optReadOnly);
        holder.addDetail("EGH_SOAP_CME/Log", "size", &CMESOAPConfig::logFileSize, "10240000", optReadOnly);
        holder.addDetail("EGH_SOAP_CME/Log", "count", &CMESOAPConfig::logCount, "5", optReadOnly);
        holder.addDetail("EGH_SOAP_CME/SOAPClient", &CMESOAPConfig::readClient, &CMESOAPConfig::registerClient, Range(1, 1));
        holder.addDetail("EGH_SOAP_CME/Options", "ignoreLocal", &CMESOAPConfig::ignoreLocal, "1", optReadOnly);
    }

    void readClient(ZQ::common::XMLUtil::XmlNode node, const ZQ::common::Preprocessor* hPP)
    {
        client.read(node, hPP);
    }
    void registerClient(const std::string &full_path)
    {
        client.snmpRegister(full_path);
    };
};

} // namespace CMESOAPHelper
} // namespace EventGateway

extern ZQ::common::Config::Loader <EventGateway::CMESOAPHelper::CMESOAPConfig> gConfig;
#endif

