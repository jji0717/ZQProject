#include "TransferDeleteRequestHandler.h"
#include "C2Env.h"
#include "C2LocatorImpl.h"
#include "ClientManager.h"
#include "TransferPortManager.h"
#include "../SimpleXMLParser.h"
#include <Text.h>

namespace ZQTianShan{
namespace CDN{

TransferDeleteRequestHandler::TransferDeleteRequestHandler(C2Env& env, TianShanIce::SCS::C2LocatorImpl& locator)
    :_log(*env._pLog), _env(env), _locator(locator),
     _clientMgr(locator.getClientManager()),
     _tpMgr(locator.getTransferPortManager())
{
}

#define ERROR_RESPONSE(RESP, CODE, REASON) RESP->setStatus(CODE, REASON);\
    _log(ZQ::common::Log::L_INFO, ReqLOGFMT(TransferDeleteRequestHandler, "errorResponse() ReqestFrom(%s:%d)StatusCode(%d)ReasonPhrase(%s)"), reqFrom.c_str(), reqFromPort, CODE, REASON)

void TransferDeleteRequestHandler::onRequest(const CRG::IRequest* req, CRG::IResponse* resp)
{
    std::string reqFrom;
    int reqFromPort;
    req->getClientEndpoint(reqFrom, reqFromPort);

    std::string reqId = _env.reqIdGen.create();
    _log(ZQ::common::Log::L_INFO, ReqLOGFMT(TransferDeleteRequestHandler, "TransferDeleteRequest from %s:%d"), reqFrom.c_str(), reqFromPort);

    typedef SimpleXMLParser::Node Node;
    std::string content;
    req->getContent(content);
    std::string reqDumpHint = "[req-" + reqId + "] TransferDeleteRequest:";
    _log.hexDump(ZQ::common::Log::L_DEBUG, content.c_str(), (const int)content.size(), reqDumpHint.c_str(), true); // text-only dump
    SimpleXMLParser parser;
    try
    {
        parser.parse(content.data(), (const int)content.size(), 1);
    }
	catch(const ZQ::common::ExpatException& e)
    { // bad xml format
        _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(TransferDeleteRequestHandler, "ExpatException: [%s] during parsing request body"), e.getString());
        ERROR_RESPONSE(resp, 400, "Bad XML format");
        return;
    }

    // check the content and extract the post data
    const Node* locNode = findNode(&parser.document(), "LocateRequest");
    if(NULL == locNode)
    { // bad xml content
        ERROR_RESPONSE(resp, 400, "Bad XML content");
        return;
    }

    const Node* clientTransferNode = findNode(locNode, "ClientTransfer");
    std::string clientTransfer = clientTransferNode ? clientTransferNode->content : "";
    if(clientTransfer.empty())
    {
        int clientPort;
        req->getClientEndpoint(clientTransfer, clientPort);
    }

    const Node* transferIdNode = findNode(locNode, "TransferIDDelete");
    std::string transferId = transferIdNode ? transferIdNode->content : "";
    if(transferId.empty())
    {
        ERROR_RESPONSE(resp, 400, "Parameter missed: TransferIDDelete");
        return;
    }

    // the TransferIDDelete is a comma separated list of transfer IDs
    std::vector<std::string> transferIds;
    ZQ::common::Text::split(transferIds, transferId, ", ");
    if(transferIds.empty())
    {
        ERROR_RESPONSE(resp, 400, "Bad Parameter: TransferIDDelete");
        return;
    }

    size_t nDeleted = 0;
    for(std::vector<std::string>::const_iterator it = transferIds.begin(); it != transferIds.end(); ++it)
    {
        TianShanIce::SCS::TransferSessionPrx sess = _locator.openSessionByTransferId(*it, Ice::Current());
        if(sess)
        {
            Ice::Identity id = sess->getIdent();
            _locator.destroy(id);
            _log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(TransferDeleteRequestHandler, "Deleted transfer id [%s] successfully."), it->c_str());
            ++nDeleted;
        }
        else
        {
#pragma message(__MSGLOC__"Is it a good idea that ignore the non-exist id?")
            _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(TransferDeleteRequestHandler, "No session is bound with transfer id [%s]."), it->c_str());
        }
    }

    _log(ZQ::common::Log::L_INFO, ReqLOGFMT(TransferDeleteRequestHandler, "%d ids processed. %d deleted, %d not found"), transferIds.size(), nDeleted, transferIds.size() - nDeleted);
    if(nDeleted != 0)
    { // response
        resp->setStatus(200, "OK");
        _log(ZQ::common::Log::L_INFO, ReqLOGFMT(TransferDeleteRequestHandler, "Response 200 OK"));
        return;
    }

	ERROR_RESPONSE(resp, 404, "Content Not Found");
	return;
}

}} // namepace ZQTianShan::CDN

