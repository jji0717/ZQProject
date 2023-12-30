#include "CMESOAPClientImpl.h"
#include <sstream>

#define CME_SOAP_CLIENT_TIMEOUT 5 // seconds
#define DEFAULT_PORT 8080

ZQ::common::Log _NULLLogger(ZQ::common::Log::L_DEBUG);
void logSoapErrorMsg(struct soap *psoap/* , log object */)
{
    std::stringstream ss;
    soap_stream_fault(psoap,ss);
    std::string errMsg = ss.str();
    std::cout << errMsg << std::endl;
}
int main(int argc, char* argv[])
{
    int _port = 0;
    if(argc == 2)
    {
        _port = atoi(argv[1]);
    }
    if(_port <= 0)
        _port = DEFAULT_PORT;

    static char serverEndpoint[1024];
    sprintf(serverEndpoint, "http://127.0.0.1:%d/services/CMEService?wsdl", _port);
    EventGateway::CMESOAPHelper::SOAPClientConfig cfg;
    cfg.serviceEndpoint = serverEndpoint;
    cfg.connectTimeout = CME_SOAP_CLIENT_TIMEOUT;
    cfg.sendTimeout = CME_SOAP_CLIENT_TIMEOUT;
    cfg.recvTimeout = CME_SOAP_CLIENT_TIMEOUT;
    cfg.retryCountMax = 3;

     EventGateway::CMESOAPHelper::ClientImpl client(_NULLLogger, cfg);
//     CMEServiceSoapBinding cmeSoapClient;
//     //cmeSoapClient.endpoint =serverEndpoint;
//     cmeSoapClient.soap->connect_timeout = CME_SOAP_CLIENT_TIMEOUT;  // parameter in second
//     cmeSoapClient.soap->send_timeout = CME_SOAP_CLIENT_TIMEOUT;
//     cmeSoapClient.soap->recv_timeout = CME_SOAP_CLIENT_TIMEOUT;

//     printf("Begin sessionNotification...\n");
// 
//     struct ns1__sessionNotificationResponse sessNotificationResponse;
//     if(cmeSoapClient.ns1__sessionNotification("no_providerID", "no_providerAssetID", 1, "no_clusterID", "no_sessionID", "no_timeStamp", sessNotificationResponse) == 0)
//     {
//         // invoke successfully
//         printf("sessionNotification succeed\n");
//     }
//     else
//     {
//         // soap level error
//         logSoapErrorMsg(cmeSoapClient.soap);
//         return -1;
//     }
    for(int i = 0; i < 10; ++i)
    {
        char c = 'a' + (char)i;
        EventGateway::CMESOAPHelper::RequestData::SessionNotification rqstData("no_providerID", "no_providerAssetID", (i % 2) + 1, "no_clusterID", std::string("sessID_") + c, "no_timeStamp");
        client.sessionNotification(rqstData);
        Sleep(1000);
    }
    
    Sleep(20000);
    return 0;
}