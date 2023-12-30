#ifndef __C2_LocateRequestHandler_H__
#define __C2_LocateRequestHandler_H__

#include "ZQ_common_conf.h"
#include "CRMInterface.h"

#undef max
#include <boost/regex.hpp>

namespace TianShanIce{
namespace SCS{
class C2LocatorImpl;
}}

namespace ZQTianShan{
namespace CDN{

class C2Env;
class ClientManager;
class TransferPortManager;

class LocateRequestHandler: public CRG::IContentHandler
{
public:
    LocateRequestHandler(C2Env& env, TianShanIce::SCS::C2LocatorImpl& locator);
    virtual ~LocateRequestHandler();
    virtual void onRequest(const CRG::IRequest* req, CRG::IResponse* resp);
private:
    bool resolveObject(const std::string& identifier, std::string& porviderId, std::string& assetId, std::string& extentsion) const;
    bool forwardRequest(const std::string& fReq, CRG::IResponse* resp, const std::string& reqId, const std::map<std::string, std::string>& headers);
    std::string getContentStore(const std::string& netid) const;
private:
    ZQ::common::Log& _log;
    C2Env& _env;
    TianShanIce::SCS::C2LocatorImpl& _locator;
    ClientManager& _clientMgr;
    TransferPortManager& _tpMgr;
    struct ObjectResolver
    {
        std::string type;
        boost::regex matcher;
        std::string providerId;
        std::string assetId;
        std::string extension;
    };
    typedef std::vector<ObjectResolver> ObjectResolvers;
    ObjectResolvers _objResolvers;

    std::string forwardUrl_;
    std::vector<std::string> forwardExcludeStates_;
};
}} // namespace ZQTianShan::CDN

#endif
