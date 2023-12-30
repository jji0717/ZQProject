#ifndef __ZQ_XmlRPCService_H__
#define __ZQ_XmlRPCService_H__
#include <ZQ_common_conf.h>
#ifdef ZQ_OS_MSWIN
#include <BaseZQServiceApplication.h>
#else
#include <ZQDaemon.h>
#endif
#include <HttpEngine.h>
class XmlRpcCmdFac;
class XmlRPCService : public ZQ::common::BaseZQServiceApplication
{
public:
    XmlRPCService();
    virtual ~XmlRPCService();
    virtual HRESULT OnInit(void);
    virtual HRESULT OnStop(void);
    virtual HRESULT OnStart(void);
    virtual HRESULT OnUnInit(void);		
    virtual void OnSnmpSet(const char*);
private:
    ZQHttp::Engine* _pWebServer;
    XmlRpcCmdFac* _pCmdFac;
};
#endif
