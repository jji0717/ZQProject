
#ifndef __CRG_HTTPCRGSVC_H__
#define __CRG_HTTPCRGSVC_H__

#include <ZQDaemon.h>
#include "ClientRequestGw.h"

namespace ZQTianShan{
namespace HttpCRG{

class HttpCRGSvc : public ZQ::common::ZQDaemon
{
public:
    HttpCRGSvc();
    ~HttpCRGSvc();

    virtual bool OnInit(void);
    virtual bool OnStart(void);
    virtual void OnStop(void);
    virtual void OnUnInit(void);

private:
 	::std::string                   _strProgramRootPath;
    ::std::string                   _strPluginFolder;
    ::std::string                   _strLogFolder;

    ::ZQ::common::FileLog           _fLog;

    //CRGateway obj
     CRG::CRGateway*                _pcrg;

};

}
}

#endif //__CRG_HTTPCRGSVC_H__

