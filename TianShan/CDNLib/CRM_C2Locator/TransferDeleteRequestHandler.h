#ifndef __C2_TransferDeleteRequestHandler_H__
#define __C2_TransferDeleteRequestHandler_H__
#include <ZQ_common_conf.h>
#include <CRMInterface.h>

namespace TianShanIce{
namespace SCS{
class C2LocatorImpl;
}}

namespace ZQTianShan{
namespace CDN{

class C2Env;
class ClientManager;
class TransferPortManager;

class TransferDeleteRequestHandler: public CRG::IContentHandler
{
public:
    TransferDeleteRequestHandler(C2Env& env, TianShanIce::SCS::C2LocatorImpl& locator);

    virtual void onRequest(const CRG::IRequest* req, CRG::IResponse* resp);
private:
    ZQ::common::Log& _log;
    C2Env& _env;
    TianShanIce::SCS::C2LocatorImpl& _locator;
    ClientManager& _clientMgr;
    TransferPortManager& _tpMgr;
};
}} // namespace ZQTianShan::CDN
#endif
