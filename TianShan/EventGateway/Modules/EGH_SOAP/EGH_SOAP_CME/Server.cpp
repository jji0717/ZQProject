#include <fstream>
// import the namespace map
#include "CMEServiceSoapBinding.nsmap"
#include "CMESOAPCMEServiceSoapBindingObject.h"
static std::ofstream out;
static CRITICAL_SECTION _csOut; // protect the out file

int ns1__cacheNotification(struct soap*, std::string _providerID, std::string _providerAssetID, std::string _clusterID, LONG64 _status, struct ns1__cacheNotificationResponse &_param_1)
{
    EnterCriticalSection(&_csOut);
    using std::endl;
    static int rqstCount = 0;
    ++ rqstCount;
    out << time(NULL) << "\t";
    out << "cache(" << rqstCount << "):"
        << "  P=" << _providerID
        << "  PA=" << _providerAssetID
        << "  C=" << _clusterID
        << "  St=" << _status
        << endl;
    LeaveCriticalSection(&_csOut);
    return SOAP_OK;
}

int ns1__sessionNotification(struct soap*, std::string providerID, std::string providerAssetID, int func, std::string clusterID, std::string sessionID, std::string timeStamp, struct ns1__sessionNotificationResponse &_param_2)
{
    EnterCriticalSection(&_csOut);
    using std::endl;
    static int rqstCount = 0;
    ++ rqstCount;
    out << time(NULL) << "\t";
    out << "sess(" << rqstCount << "):"
        << "  P=" << providerID
        << "  PA=" << providerAssetID
        << "  f=" << func
        << "  C=" << clusterID
        << "  S=" << sessionID
        << "  T=" << timeStamp << endl;;
    LeaveCriticalSection(&_csOut);
    return SOAP_OK;
}

#define TIMEOUT			(24*60*60) // timeout after 24hrs of inactivity
#define DEFAULT_PORT    8080
#define BACKLOG			100
int main(int argc, char* argv[])
{
    using std::cout;
    using std::endl;
    const char* _ip = NULL;
    int _port = 0;
    switch(argc)
    {
    case 3:
        _port = atoi(argv[2]);
    case 2:
        _ip = argv[1];
        break;
    default:
        cout << "usage: CMESOAPServer.exe <bind_ip> [port(default 8080)]\n";
        return -1;
    }
    if(_port <= 0)
        _port = DEFAULT_PORT;

    InitializeCriticalSection(&_csOut);
    out.open("CMESOAPServer.log");

    struct soap soapServer;
    soap_init2(&soapServer, SOAP_IO_KEEPALIVE, SOAP_IO_KEEPALIVE);

    soapServer.accept_timeout = TIMEOUT;
    soapServer.bind_flags |= SO_REUSEADDR;	/* don't use this in unsecured environments */

    out << "CMESOAPServer bind at [" << _ip << ":" << _port << "]" << endl;
    cout << "CMESOAPServer bind at [" << _ip << ":" << _port << "]\n";
    int ret = soap_bind(&soapServer, _ip, _port, BACKLOG);
    if (ret < 0)
    {
        soap_stream_fault(&soapServer, out);
        return -1;
    }

    while (true)
    {
        int ret = soap_accept(&soapServer);
        if (ret < 0)
        {
            if (soapServer.errnum)
                soap_stream_fault(&soapServer, out);
            else
                out << "timed out\n";	// should really wait for threads to terminate
        }

        struct soap* soapConn = soap_copy(&soapServer);
        soap_serve(soapConn);

        soap_destroy(soapConn);
        soap_end(soapConn);
        soap_done(soapConn);
    }

    soap_destroy(&soapServer);
    soap_end(&soapServer);
    soap_done(&soapServer);



    DeleteCriticalSection(&_csOut);
    return 0;
}