#include "HttpC2Streamer.h"
#include <sstream>
#include "SimpleXMLParser.h"
#include <HttpClient.h>
#include <strHelper.h>
#include "CdnStreamerManager.h"
//#include "CdnSSConfig.h"
#include <TimeUtil.h>

static int32 str2int(const char* s) {
    return strtol(s, NULL, 10);
}
static int64 str2long(const char* s) {
#ifdef ZQ_OS_MSWIN
    return _strtoi64(s, NULL, 10);
#else
#error str2long
#endif
}
namespace C2Streamer {

HttpC2StreamerEnv::HttpC2StreamerEnv()
:logger(NULL),
mgr(NULL),
sessScaner(NULL),
eventUpdater(NULL),
statusReportSvr(NULL),
handlerFac(NULL)
{
}

HttpC2StreamerEnv gHttpSteamerEnv;
HttpC2StreamerEnv* getEnvironment() {
    return &gHttpSteamerEnv;
}
static int32 httpCommand(const std::string& resource, const std::string& req, std::string& resp);

int32 cTransferInit( const TransferInitRequestParamPtr request , TransferInitResponseParamPtr response ) {
    ZQ::common::Log* envLogger = getEnvironment()->getLogger();
#define UMG_EnvLog if (envLogger) (*envLogger)
#define UMG_ReqFMT(_MOD, _X) CLOGFMT(_MOD, "[%s] " _X), request->sessionId.c_str()
    UMG_EnvLog(ZQ::common::Log::L_DEBUG, UMG_ReqFMT(HttpC2Streamer, "TransferInit() ClientTransfer(%s), TransferAddress(%s), IngressCapacity(%lld), FileName(%s), TransferRate(%d), TransferTimeout(%d)"), request->clientTransfer.c_str(), request->transferAddress.c_str(), request->ingressCapacity, request->fileName.c_str(), request->transferRate, request->transferTimeout);
    // check the request parameter
    if(request->clientTransfer.empty() ||
       request->transferAddress.empty() ||
       request->ingressCapacity <= 0 ||
       request->fileName.empty() ||
       request->transferRate < 0 || // may be 0
       // not care the AllocatedTransferRate
       //request->allocatedTransferRate < request->transferRate ||
       request->transferTimeout <= 0) {
           response->setLastErr(request, 400, "Invalid parameter");
           UMG_EnvLog(ZQ::common::Log::L_WARNING, UMG_ReqFMT(HttpC2Streamer, "TransferInit() Invalid parameter"));
           return 400;
    }

    // build the request xml
    std::ostringstream buf;
    buf << "<TransferInitiate>";
    // client transfer
    buf << "<ClientTransfer>"
        << request->clientTransfer
        << "</ClientTransfer>";
    // transfer address
    buf << "<TransferAddress>"
        << request->transferAddress
        << "</TransferAddress>";
    // ingress capacity
    buf << "<IngressCapacity>"
        << request->ingressCapacity
        << "</IngressCapacity>";

    // extra ingress capacity
    if(request->extraIngressCapcity > 0) {
        buf << "<ExtraIngressCapacity>"
            << request->extraIngressCapcity
            << "</ExtraIngressCapacity>";
    }
    // file name
    buf << "<Filename>"
        << request->fileName
        << "</Filename>";

    // range
    if(request->requestRange.bStartValid || request->requestRange.bEndValid) {
        buf << "<Range>"
            << request->requestRange.toString()
            << "</Range>";
    }
    // transfer delay
    if(request->transferDelay != 0) { // may be negative
        buf << "<TransferDelay>"
            << request->transferDelay
            << "</TransferDelay>";
    }
    // transfer rate
    buf << "<TransferRate>"
        << request->transferRate
        << "</TransferRate>";
    // allocated transfer rate. use the TransferRate instead
    buf << "<AllocatedTransferRate>"
        << request->transferRate
        << "</AllocatedTransferRate>";
    // transfer timeout
    buf << "<TransferTimeout>"
        << request->transferTimeout
        << "</TransferTimeout>";
    buf << "</TransferInitiate>";

    std::string requestData, responseData;
    buf.str().swap(requestData);

    UMG_EnvLog.hexDump(ZQ::common::Log::L_DEBUG, requestData.c_str(), requestData.size(), ("TransferInit [" + request->sessionId + "]").c_str(), true); // text-only dump
    // send request and get response
    // may block here?
    int64 t0 = ZQ::common::now();
    int32 statusCode = httpCommand("/c2cp/transferinitiate", requestData, responseData);
    UMG_EnvLog(ZQ::common::Log::L_INFO, UMG_ReqFMT(HttpC2Streamer, "TransferInit() Send request to [%s/c2cp/transferinitiate] and get %d. cost(%d)"), getEnvironment()->endpoint.c_str(), statusCode, (ZQ::common::now() - t0));

    if(statusCode <= 0) {
        response->setLastErr(request, 400, "Http client error:%s", responseData.c_str());
        UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "TransferInit() Http client error:%s"), responseData.c_str());
        return 400;
    } else {
        UMG_EnvLog.hexDump(ZQ::common::Log::L_DEBUG, responseData.c_str(), responseData.size(), ("TransferInitResponse [" + request->sessionId + "]").c_str(), true); // text-only dump
        response->errorCode = statusCode;
        // parse the response xml
        SimpleXMLParser parser;
        try {
            parser.parse(responseData.data(), responseData.size(), 1);
        }catch(const ZQ::common::ExpatException& e) { // bad xml format
            response->setLastErr(request, 500, "XML parsing error: StatusCode[%d] ExpatException[%s]", statusCode, e.getString());
            UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "TransferInit() XML parsing error: StatusCode[%d] ExpatException[%s]"), statusCode, e.getString());
            return 500;
        }
        // check the response parameter
        const SimpleXMLParser::Node* root = findNode(&parser.document(), "TransferInitiateResponse");
        if(!root) {
            response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<TransferInitiateResponse>]", statusCode);
            UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "TransferInit() Parameter missed: StatusCode[%d] Node[<TransferInitiateResponse>]"), statusCode);
            return 500;
        }
        if(response->isSuccess()) {
            // transfer id
            const SimpleXMLParser::Node* n = findNode(root, "TransferID");
            if(n) {
                response->transferId = n->content;
            } else {
                response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<TransferID>]", statusCode);
                UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "TransferInit() Parameter missed: StatusCode[%d] Node[<TransferID>]"), statusCode);
                return 500;
            }
            // available range
            n = findNode(root, "AvailableRange");
            if(n) {
                response->availRange.parse(n->content);
            }
            // open-for-write
            n = findNode(root, "OpenForWrite");
            if(n) {
                response->openForWrite = n->content == "yes";
            }
        } else {
            // error text
            const SimpleXMLParser::Node* n = findNode(root, "ErrorText");
            if(n) {
                response->setLastErr(request, statusCode, "ErrorText:%s", n->content.c_str());
            } else {
                response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<ErrorText>]", statusCode);
                UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "TransferInit() Parameter missed: StatusCode[%d] Node[<ErrorText>]"), statusCode);
                return 500;
            }
        }
    }
    UMG_EnvLog(ZQ::common::Log::L_INFO, UMG_ReqFMT(HttpC2Streamer, "TransferInit() Complete. StatusCode(%d)"), statusCode);
    return statusCode;
}
int32 cTransferRun( const SessionTransferParamPtr request , SessionTransferResponseParamPtr response ) {
    return errorCodeOK;
}
int32 cTransferTerm( const TransferTermRequestParamPtr request , TransferTermResponseParamPtr response ) {
    ZQ::common::Log* envLogger = getEnvironment()->getLogger();
    UMG_EnvLog(ZQ::common::Log::L_DEBUG, UMG_ReqFMT(HttpC2Streamer, "TransferTerm() ClientTransfer(%s)"), request->clientTransfer.c_str());
    if(request->sessionId.empty()) {
        response->setLastErr(request, 400, "Parameter missed: TransferId");
        UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "TransferTerm() Parameter missed: TransferId"));
        return 400;
    }
    if(request->clientTransfer.empty()) {
        response->setLastErr(request, 400, "Parameter missed: ClientTransfer");
        UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "TransferTerm() Parameter missed: ClientTransfer"));
        return 400;
    }

    std::ostringstream buf;
    buf << "<TransferTerminate>"
        << "<ClientTransfer>" << request->clientTransfer << "</ClientTransfer>"
        << "<TransferID>" << request->sessionId << "</TransferID>"
        << "</TransferTerminate>";
    std::string requestData, responseData;
    buf.str().swap(requestData);
    UMG_EnvLog.hexDump(ZQ::common::Log::L_DEBUG, requestData.c_str(), requestData.size(), ("TransferTerm [" + request->sessionId + "]").c_str(), true); // text-only dump
    int64 t0 = ZQ::common::now();
    int32 statusCode = httpCommand("/c2cp/transferterminate", requestData, responseData);
    UMG_EnvLog(ZQ::common::Log::L_INFO, UMG_ReqFMT(HttpC2Streamer, "TransferTerm() Send request to [%s/c2cp/transferterminate] and get %d. cost(%d)"), getEnvironment()->endpoint.c_str(), statusCode, (ZQ::common::now() - t0));
    if(statusCode <= 0) {
        response->setLastErr(request, 400, "Http client error:%s", responseData.c_str());
        UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "TransferTerm() Http client error:%s"), responseData.c_str());
        return 400;
    } else {
        UMG_EnvLog.hexDump(ZQ::common::Log::L_DEBUG, responseData.c_str(), responseData.size(), ("TransferTermResponse [" + request->sessionId + "]").c_str(), true); // text-only dump
        response->errorCode = statusCode;
        // parse the response xml
        SimpleXMLParser parser;
        try {
            parser.parse(responseData.data(), responseData.size(), 1);
        }catch(const ZQ::common::ExpatException& e) { // bad xml format
            response->setLastErr(request, 500, "XML parsing error: StatusCode[%d] ExpatException[%s]", statusCode, e.getString());
            UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "TransferTerm() XML parsing error: StatusCode[%d] ExpatException[%s]"), statusCode, e.getString());
            return 500;
        }
        // check the response parameter
        const SimpleXMLParser::Node* root = findNode(&parser.document(), "TransferTerminateResponse");
        if(!root) {
            response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<TransferTerminateResponse>]", statusCode);
            UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "TransferTerm() XML parsing error: Parameter missed: StatusCode[%d] Node[<TransferTerminateResponse>]"), statusCode);
            return 500;
        }
        if(!response->isSuccess()) {
            // error text
            const SimpleXMLParser::Node* n = findNode(root, "ErrorText");
            if(n) {
                response->setLastErr(request, statusCode, "ErrorText:%s", n->content.c_str());
            } else {
                response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<ErrorText>]", statusCode);
                UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "TransferTerm() XML parsing error: Parameter missed: StatusCode[%d] Node[<ErrorText>]"), statusCode);
                return 500;
            }
        }
    }

    UMG_EnvLog(ZQ::common::Log::L_INFO, UMG_ReqFMT(HttpC2Streamer, "TransferTerm() Complete. StatusCode(%d)"), statusCode);
    return statusCode;
}
int32 cSessionStatus( const SessionStatusRequestParamPtr request , SessionStatusResponseParamPtr response ) {
    ZQ::common::Log* envLogger = getEnvironment()->getLogger();
    std::ostringstream buf;
    buf << "<Status>"
        << "<IncludeAggregate>"
        << (request->includeAggregate ? "1" : "0")
        << "</IncludeAggregate>";
    for(size_t i = 0; i < request->clientTransfers.size(); ++i) {
        buf << "<Client>"
            << request->clientTransfers[i]
            << "</Client>";
    }
    buf << "</Status>";
    std::string requestData, responseData;
    buf.str().swap(requestData);
    UMG_EnvLog.hexDump(ZQ::common::Log::L_DEBUG, requestData.c_str(), requestData.size(), ("SessionStatus [" + request->sessionId + "]").c_str(), true); // text-only dump

    int64 t0 = ZQ::common::now();
    int32 statusCode = httpCommand("/c2cp/status", requestData, responseData);
    UMG_EnvLog(ZQ::common::Log::L_INFO, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Send request to [%s/c2cp/status] and get %d. cost(%d)"), getEnvironment()->endpoint.c_str(), statusCode, (ZQ::common::now() - t0));

    if(statusCode <= 0) {
        response->setLastErr(request, 400, "Http client error:%s", responseData.c_str());
        UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Http client error:%s"), responseData.c_str());
        return 400;
    } else {
        UMG_EnvLog.hexDump(ZQ::common::Log::L_DEBUG, responseData.c_str(), responseData.size(), ("SessionStatusResponse [" + request->sessionId + "]").c_str(), true); // text-only dump
        response->errorCode = statusCode;
        // parse the response xml
        SimpleXMLParser parser;
        try {
            parser.parse(responseData.data(), responseData.size(), 1);
        }catch(const ZQ::common::ExpatException& e) { // bad xml format
            response->setLastErr(request, 500, "XML parsing error: StatusCode[%d] ExpatException[%s]", statusCode, e.getString());
            UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() XML parsing error: StatusCode[%d] ExpatException[%s]"), statusCode, e.getString());
            return 500;
        }
        // check the response parameter
        const SimpleXMLParser::Node* root = findNode(&parser.document(), "StatusResponse");
        if(!root) {
            response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<StatusResponse>]", statusCode);
            UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<StatusResponse>]"), statusCode);
            return 500;
        }
        if(response->isSuccess()) {
            // the aggregate statistics
            const SimpleXMLParser::Node* asNode = findNode(root, "AggregateStatistics");
            if(asNode) {
                const SimpleXMLParser::Node* n;
                n = findNode(asNode, "ActiveSessions");
                if(n) {
                    response->statistics.activeSessions = str2int(n->content.c_str());
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<AggregateStatistics>/<ActiveSessions>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<AggregateStatistics>/<ActiveSessions>]"), statusCode);
                    return 500;
                }
                n = findNode(asNode, "IdleSessions");
                if(n) {
                    response->statistics.idleSessions = str2int(n->content.c_str());
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<AggregateStatistics>/<IdleSessions>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<AggregateStatistics>/<IdleSessions>]"), statusCode);
                    return 500;
                }
                n = findNode(asNode, "TotalSessions");
                if(n) {
                    response->statistics.totalSessions = str2int(n->content.c_str());
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<AggregateStatistics>/<TotalSessions>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<AggregateStatistics>/<TotalSessions>]"), statusCode);
                    return 500;
                }
                n = findNode(asNode, "AllocatedBandwidth");
                if(n) {
                    response->statistics.allocatedBandwidth = str2long(n->content.c_str());
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<AggregateStatistics>/<AllocatedBandwidth>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<AggregateStatistics>/<AllocatedBandwidth>]"), statusCode);
                    return 500;
                }
                n = findNode(asNode, "TotalBandwidth");
                if(n) {
                    response->statistics.totalBandwidth = str2long(n->content.c_str());
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<AggregateStatistics>/<TotalBandwidth>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<AggregateStatistics>/<TotalBandwidth>]"), statusCode);
                    return 500;
                }
                n = findNode(asNode, "BytesTransmitted");
                if(n) {
                    response->statistics.bytesTransfered = str2long(n->content.c_str());
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<AggregateStatistics>/<BytesTransmitted>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<AggregateStatistics>/<BytesTransmitted>]"), statusCode);
                    return 500;
                }
                n = findNode(asNode, "Uptime");
                if(n) {
                    response->statistics.uptime = str2long(n->content.c_str());
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<AggregateStatistics>/<Uptime>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<AggregateStatistics>/<Uptime>]"), statusCode);
                    return 500;
                }
            }
            SiblingNode sNodes = childNodes(root, "Session");
            response->sessionInfos.reserve(sNodes.count());
            const SimpleXMLParser::Node* sNode = sNodes.first();
            while(sNode) {
                SessionStatusInfo sess;
                const SimpleXMLParser::Node* n;
                n = findNode(sNode, "TransferID");
                if(n) {
                    sess.transferId = n->content;
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<Session>/<TransferID>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<Session>/<TransferID>]"), statusCode);
                    return 500;
                }
                n = findNode(sNode, "Filename");
                if(n) {
                    sess.fileName = n->content;
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<Session>/<Filename>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<Session>/<Filename>]"), statusCode);
                    return 500;
                }
                n = findNode(sNode, "ClientTransfer");
                if(n) {
                    sess.clientTransfer = n->content;
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<Session>/<ClientTransfer>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<Session>/<ClientTransfer>]"), statusCode);
                    return 500;
                }
                n = findNode(sNode, "TransferAddress");
                if(n) {
                    sess.transferAddress = n->content;
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<Session>/<TransferAddress>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<Session>/<TransferAddress>]"), statusCode);
                    return 500;
                }
                n = findNode(sNode, "TransferPort");
                if(n) {
                    sess.transferPortName = n->content;
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<Session>/<TransferPort>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<Session>/<TransferPort>]"), statusCode);
                    return 500;
                }
                n = findNode(sNode, "State");
                if(n) {
                    sess.sessionState = n->content == "ACTIVE" ? SESSION_STATE_ACTIVE : SESSION_STATE_IDLE;
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<Session>/<State>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<Session>/<State>]"), statusCode);
                    return 500;
                }
                n = findNode(sNode, "TimeInState");
                if(n) {
                    sess.timeInState = str2long(n->content.c_str());
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<Session>/<TimeInState>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<Session>/<TimeInState>]"), statusCode);
                    return 500;
                }
                n = findNode(sNode, "TransferRate");
                if(n) {
                    sess.transferRate = str2long(n->content.c_str());
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<Session>/<TransferRate>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<Session>/<TransferRate>]"), statusCode);
                    return 500;
                }
                n = findNode(sNode, "BytesTransferred");
                if(n) {
                    sess.bytesTransfered = str2long(n->content.c_str());
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<Session>/<BytesTransferred>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<Session>/<BytesTransferred>]"), statusCode);
                    return 500;
                }
                response->sessionInfos.push_back(sess);
                sNode = sNodes.next();
            }
        } else {
            // error text
            const SimpleXMLParser::Node* n = findNode(root, "ErrorText");
            if(n) {
                response->setLastErr(request, statusCode, "ErrorText:%s", n->content.c_str());
            } else {
                response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<ErrorText>]", statusCode);
                UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Parameter missed: StatusCode[%d] Node[<ErrorText>]"), statusCode);
                return 500;
            }
        }
    }
    UMG_EnvLog(ZQ::common::Log::L_INFO, UMG_ReqFMT(HttpC2Streamer, "SessionStatus() Complete. StatusCode(%d)"), statusCode);
    return statusCode;
}
int32 cResourceStatus( const ResourceStatusRequestParamPtr request , ResourceStatusResponseParamPtr response ) {
    ZQ::common::Log* envLogger = getEnvironment()->getLogger();
    std::ostringstream buf;
    buf << "<ResourceStatus>";
    for(size_t i = 0; i < request->portNames.size(); ++i) {
        buf << "<PortName>"
            << request->portNames[i]
            << "</PortName>";
    }
    buf << "</ResourceStatus>";
    std::string requestData, responseData;
    buf.str().swap(requestData);
    UMG_EnvLog.hexDump(ZQ::common::Log::L_DEBUG, requestData.c_str(), requestData.size(), ("ResourceStatus [" + request->sessionId + "]").c_str(), true); // text-only dump

    int64 t0 = ZQ::common::now();
    int32 statusCode = httpCommand("/c2cp/resourcestatus", requestData, responseData);
    UMG_EnvLog(ZQ::common::Log::L_INFO, UMG_ReqFMT(HttpC2Streamer, "ResourceStatus() Send request to [%s/c2cp/resourcestatus] and get %d. cost(%d)"), getEnvironment()->endpoint.c_str(), statusCode, (ZQ::common::now() - t0));

    if(statusCode <= 0) {
        response->setLastErr(request, 400, "Http client error:%s", responseData.c_str());
        UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "ResourceStatus() Http client error:%s"), responseData.c_str());
        return 400;
    } else {
        UMG_EnvLog.hexDump(ZQ::common::Log::L_DEBUG, responseData.c_str(), responseData.size(), ("ResourceStatusResponse [" + request->sessionId + "]").c_str(), true); // text-only dump
        response->errorCode = statusCode;
        // parse the response xml
        SimpleXMLParser parser;
        try {
            parser.parse(responseData.data(), responseData.size(), 1);
        }catch(const ZQ::common::ExpatException& e) { // bad xml format
            response->setLastErr(request, 500, "XML parsing error: StatusCode[%d] ExpatException[%s]", statusCode, e.getString());
            UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "ResourceStatus() XML parsing error: StatusCode[%d] ExpatException[%s]"), statusCode, e.getString());
            return 500;
        }
        // check the response parameter
        const SimpleXMLParser::Node* root = findNode(&parser.document(), "ResourceStatusResponse");
        if(!root) {
            response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<ResourceStatusResponse>]", statusCode);
            UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "ResourceStatus() Parameter missed: StatusCode[%d] Node[<ResourceStatusResponse>]"), statusCode);
            return 500;
        }
        if (response->isSuccess()) {
            SiblingNode pNodes = childNodes(root, "Port");
            response->portInfos.reserve(pNodes.count());
            const SimpleXMLParser::Node* pNode = pNodes.first();
            while(pNode) {
                ResourceStatusInfo res;
                const SimpleXMLParser::Node* n;
                n = findNode(pNode, "Name");
                if(n) {
                    res.portName = n->content;
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<Port>/<Name>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "ResourceStatus() Parameter missed: StatusCode[%d] Node[<Port>/<Name>]"), statusCode);
                    return 500;
                }
                SiblingNode aNodes = childNodes(pNode, "Address");
                const SimpleXMLParser::Node* aNode = aNodes.first();
                while(aNode) {
                    if(aNode->content.find(':') != std::string::npos) { // IPv6
                        res.portAddressIpv6.push_back(aNode->content);
                    } else {
                        res.portAddressIpv4.push_back(aNode->content);
                    }
                    aNode = aNodes.next();
                }
                n = findNode(pNode, "TCPPortNumber");
                if(n) {
                    res.tcpPortNumber = str2int(n->content.c_str());
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<Port>/<TCPPortNumber>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "ResourceStatus() Parameter missed: StatusCode[%d] Node[<Port>/<TCPPortNumber>]"), statusCode);
                    return 500;
                }
                n = findNode(pNode, "Capacity");
                if(n) {
                    res.capacity = str2long(n->content.c_str());
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<Port>/<Capacity>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "ResourceStatus() Parameter missed: StatusCode[%d] Node[<Port>/<Capacity>]"), statusCode);
                    return 500;
                }
                n = findNode(pNode, "State");
                if(n) {
                    res.portState = n->content == "UP" ? PORT_STATE_UP : PORT_STATE_DOWN;
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<Port>/<State>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "ResourceStatus() Parameter missed: StatusCode[%d] Node[<Port>/<State>]"), statusCode);
                    return 500;
                }
                n = findNode(pNode, "ActiveTransferCount");
                if(n) {
                    res.activeTransferCount = str2int(n->content.c_str());
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<Port>/<ActiveTransferCount>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "ResourceStatus() Parameter missed: StatusCode[%d] Node[<Port>/<ActiveTransferCount>]"), statusCode);
                    return 500;
                }
                n = findNode(pNode, "ActiveBandwidth");
                if(n) {
                    res.activeBandwidth = str2long(n->content.c_str());
                } else {
                    response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<Port>/<ActiveBandwidth>]", statusCode);
                    UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "ResourceStatus() Parameter missed: StatusCode[%d] Node[<Port>/<ActiveBandwidth>]"), statusCode);
                    return 500;
                }
                response->portInfos.push_back(res);
                pNode = pNodes.next();
            }
        } else {
            // error text
            const SimpleXMLParser::Node* n = findNode(root, "ErrorText");
            if(n) {
                response->setLastErr(request, statusCode, "ErrorText:%s", n->content.c_str());
            } else {
                response->setLastErr(request, 500, "Parameter missed: StatusCode[%d] Node[<ErrorText>]", statusCode);
                UMG_EnvLog(ZQ::common::Log::L_ERROR, UMG_ReqFMT(HttpC2Streamer, "ResourceStatus() Parameter missed: StatusCode[%d] Node[<ErrorText>]"), statusCode);
                return 500;
            }
        }
    }
    UMG_EnvLog(ZQ::common::Log::L_INFO, UMG_ReqFMT(HttpC2Streamer, "ResourceStatus() Complete. StatusCode(%d)"), statusCode);
    return statusCode;
}

C2EventSinkerPtr updateEventReceiver( C2EventSinkerPtr& newEventReceiver , C2Method mask ) {
    //return gHttpSteamerEnv.setSinker(newEventReceiver, mask);
}
static int32 httpCommand(const std::string& resource, const std::string& req, std::string& resp) {
    resp.clear();
    ZQ::common::HttpClient client;
    client.init();
    if(0 != client.httpConnect((gHttpSteamerEnv.endpoint + resource).c_str(), ZQ::common::HttpClient::HTTP_POST)) {
#define SaveClientError(Action) {                                   \
            std::ostringstream buf;                                 \
            buf << Action " error: code(" << client.getErrorcode()  \
                << ") message(" << client.getErrorstr() << ")";     \
            buf.str().swap(resp);                                   \
        }
        SaveClientError("Connect");
        return -1;
    }
    if(0 != client.httpSendContent(req.data(), req.size())) {
        SaveClientError("Send");
        return -1;
    }
    if(0 != client.httpEndSend()) {
        SaveClientError("EndSend");
        return -1;
    }
    if(0 != client.httpBeginRecv()) {
        SaveClientError("BeginRecv");
        return -1;
    }

    while(!client.isEOF()) {
        if(0 != client.httpContinueRecv()) {
            SaveClientError("ContinueRecv");
            return -1;
        }
    }
    if(0 != client.httpEndRecv()) {
        SaveClientError("EndRecv");
        return -1;
    }
    client.getContent(resp);
    int statusCode =  client.getStatusCode();
    client.uninit();
#undef SaveClientError
    return statusCode;
}

class StatusUpdateHandler : public ZQHttp::IRequestHandler {
public:
    virtual ~StatusUpdateHandler(){}
    /// on*() return true for continue, false for break.
    virtual bool onConnected(ZQHttp::IConnection&){ return true; }
    virtual bool onRequest(const ZQHttp::IRequest& req, ZQHttp::IResponse& resp) {
        req_ = &req;
        resp_ = &resp;
        reqData_.clear();
        const char* lenHint = req.header("Content-Length");
        if(lenHint) {
            reqData_.reserve(atoi(lenHint));
        }
        return true;
    }
    virtual bool onPostData(const ZQHttp::PostDataFrag& frag) {
        reqData_ += std::string(frag.data, frag.len);
        return true;
    }
    virtual bool onPostDataEnd() { return true; }
    virtual void onRequestEnd() {
        std::string respData;
        int statusCode = processRequest(reqData_, respData);
        resp_->setStatus(statusCode);
        char buf[32];
        itoa(respData.size(), buf, 10);
        resp_->setHeader("Content-Length", buf);
        resp_->headerPrepared();
        resp_->addContent(respData.c_str(), respData.size());
        resp_->complete();
    }

    // break the current request processing
    virtual void onBreak() {
        return;
    }

    virtual int processRequest(const std::string&, std::string&) {
        return 200;
    }
private:
    const ZQHttp::IRequest* req_;
    ZQHttp::IResponse* resp_;
    std::string reqData_;
};
class IngressCapacityUpdateHandler: public StatusUpdateHandler {
public:
    virtual int processRequest(const std::string& req, std::string& resp) {
        resp = "<TransferIngressCapacityUpdateResponse/>";
        return 200;
    }
};

#define UMG_BuildErrorResponse(Root, ErrorText) { std::ostringstream buf;\
    buf << "<" << Root << "><ErrorText>" << ErrorText << "</ErrorText></" << Root << ">";\
    buf.str().swap(resp); }
class TransferStateUpdateHandler: public StatusUpdateHandler {
public:
    virtual int processRequest(const std::string& req, std::string& resp) {
        SimpleXMLParser parser;
        try {
            parser.parse(req.data(), req.size(), 1);
        }catch(const ZQ::common::ExpatException& e) { // bad xml format
            UMG_BuildErrorResponse("TransferStateUpdateResponse", (std::string("XML parsing error: ") + e.getString()));
            return 400;
        }
        const SimpleXMLParser::Node* root = findNode(&parser.document(), "TransferStateUpdate");
        if(!root) {
            UMG_BuildErrorResponse("TransferStateUpdateResponse", "Parameter missed: TransferStateUpdate");
            return 400;
        }
        SiblingNode tNodes = childNodes(root, "Transfer");
        const SimpleXMLParser::Node* tNode = tNodes.first();
        while(tNode) {
            const SimpleXMLParser::Node* n = findNode(tNode, "TransferID");
            if(!n) {
                UMG_BuildErrorResponse("TransferStateUpdateResponse", "Parameter missed: TransferID");
                return 400;
            }
            std::string transferId = n->content;
            n = findNode(tNode, "State");
            if(!n) {
                UMG_BuildErrorResponse("TransferStateUpdateResponse", "Parameter missed: State");
                return 400;
            }
            if(n->content == "DELETED") {
                ZQ::StreamService::StreamParams paras;
                TianShanIce::Properties props;
                getEnvironment()->mgr->getSsImpl()->OnStreamEvent(ZQ::StreamService::SsServiceImpl::seGone, transferId, paras , props );

            }
            tNode = tNodes.next();
        }
        resp = "<TransferStateUpdateResponse/>";
        return 200;
    }
};

class TransferResourceUpdateHandler: public StatusUpdateHandler {
public:
    TransferResourceUpdateHandler() {}
    virtual ~TransferResourceUpdateHandler() {}
    virtual int processRequest(const std::string& req, std::string& resp) {

        SimpleXMLParser parser;
        try {
            parser.parse(req.data(), req.size(), 1);
        }catch(const ZQ::common::ExpatException& e) { // bad xml format
            UMG_BuildErrorResponse("TransferResourceUpdateResponse", (std::string("XML parsing error: ") + e.getString()));
            return 400;
        }
        const SimpleXMLParser::Node* root = findNode(&parser.document(), "TransferResourceUpdate");
        if(!root) {
            UMG_BuildErrorResponse("TransferResourceUpdateResponse", "Parameter missed: TransferResourceUpdate");
            return 400;
        }
        SiblingNode pNodes = childNodes(root, "Port");
        const SimpleXMLParser::Node* pNode = pNodes.first();
        while(pNode) {
            const SimpleXMLParser::Node* n = findNode(pNode, "Name");
            if(!n) {
                UMG_BuildErrorResponse("TransferResourceUpdateResponse", "Parameter missed: Name");
                return 400;
            }
            std::string name = n->content;

            n = findNode(pNode, "Address");
            if(!n) {
                UMG_BuildErrorResponse("TransferResourceUpdateResponse", "Parameter missed: Address");
                return 400;
            }
            std::string addr = n->content;

            n = findNode(pNode, "TCPPortNumber");
            if(!n) {
                UMG_BuildErrorResponse("TransferResourceUpdateResponse", "Parameter missed: TCPPortNumber");
                return 400;
            }
            std::string tcpPortNum = n->content;

            n = findNode(pNode, "Capacity");
            if(!n) {
                UMG_BuildErrorResponse("TransferResourceUpdateResponse", "Parameter missed: Capacity");
                return 400;
            }
            std::string capacity = n->content;

            n = findNode(pNode, "State");
            if(!n) {
                UMG_BuildErrorResponse("TransferResourceUpdateResponse", "Parameter missed: State");
                return 400;
            }
            std::string state = n->content;

            n = findNode(pNode, "ActiveTransferCount");
            if(!n) {
                UMG_BuildErrorResponse("TransferResourceUpdateResponse", "Parameter missed: ActiveTransferCount");
                return 400;
            }
            std::string activeTransferCount = n->content;

            n = findNode(pNode, "ActiveBandwidth");
            if(!n) {
                UMG_BuildErrorResponse("TransferResourceUpdateResponse", "Parameter missed: ActiveBandwidth");
                return 400;
            }
            std::string activeBandwidth = n->content;

            ZQ::StreamService::CdnStreamerManager::StreamerAttr streamer;
            streamer.portName = name;
            if(addr.find(':') != std::string::npos) {
                streamer.transferAddressIpv6.push_back(addr);
            } else {
                streamer.transferAddressIpv4.push_back(addr);
            }
            streamer.transferTcpPort = str2int(tcpPortNum.c_str());
            streamer.capacity = str2long(capacity.c_str());
            streamer.bUp = state == "UP";
            streamer.activeTransferCount = str2int(activeTransferCount.c_str());
            streamer.activeBandwidth = str2long(activeBandwidth.c_str());
            getEnvironment()->mgr->reportStreamerState(streamer);

            pNode = pNodes.next();
        }
        resp = "<TransferResourceUpdateResponse/>";
        return 200;
    }
};
class UpdateHandlerFactory: public ZQHttp::IRequestHandlerFactory {
public:
    virtual ~UpdateHandlerFactory(){}
    virtual ZQHttp::IRequestHandler* create(const char* uri) {
        if(strstr(uri, "transferingresscapacityupdate")) {
            return new IngressCapacityUpdateHandler();
        } else if (strstr(uri, "transferstateupdate")) {
            return new TransferStateUpdateHandler();
        } else if (strstr(uri, "transferterresourceupdate")) {
            return new TransferResourceUpdateHandler();
        } else {
            return NULL;
        }
    }
    virtual void destroy(ZQHttp::IRequestHandler* p) {
        if(p) {
            delete p;
        }
    }
};

bool HttpC2StreamerEnv::start() {
    if(NULL == logger || NULL == mgr) {
        return false;
    }
    endpoint = gCdnSSConfig.c2StreamerConfig.httpBindIp + ":" + gCdnSSConfig.c2StreamerConfig.httpBindPort;
    sessScaner = new ZQ::StreamService::CdnSessionScaner(mgr->getCdnSsEnv(), mgr->getSsImpl());
    eventUpdater = new C2EventUpdater();
    statusReportSvr = new ZQHttp::Engine(*C2Streamer::getEnvironment()->logger);
    statusReportSvr->setEndpoint(gCdnSSConfig.c2StreamerConfig.statusReportSvrIp, gCdnSSConfig.c2StreamerConfig.statusReportSvrPort);
    statusReportSvr->setCapacity(gCdnSSConfig.c2StreamerConfig.statusReportSvrThreadCount);
    bool bOK = sessScaner->start();
    (*logger)(ZQ::common::Log::L_INFO, CLOGFMT(HttpC2StreamerEnv, "Start the SessionScaner...%s"), (bOK ? "OK" : "Failed"));

    bOK = eventUpdater->start();
    (*logger)(ZQ::common::Log::L_INFO, CLOGFMT(HttpC2StreamerEnv, "Start the EventUpdater...%s"), (bOK ? "OK" : "Failed"));
    
    handlerFac = new UpdateHandlerFactory();
    statusReportSvr->registerHandler(".c2cp.*", handlerFac);
    bOK = statusReportSvr->start();
    (*logger)(ZQ::common::Log::L_INFO, CLOGFMT(HttpC2StreamerEnv, "Start the StatusReportServer...%s"), (bOK ? "OK" : "Failed"));
    return true;
}
void HttpC2StreamerEnv::stop() {
    if(statusReportSvr) {
        statusReportSvr->stop();
        (*logger)(ZQ::common::Log::L_INFO, CLOGFMT(HttpC2StreamerEnv, "Stop the  StatusReportServer"));
    }
    if(eventUpdater) {
        eventUpdater->stop();
        (*logger)(ZQ::common::Log::L_INFO, CLOGFMT(HttpC2StreamerEnv, "Stop the  EventUpdater"));
    }
    if(sessScaner) {
        sessScaner->stop();
        (*logger)(ZQ::common::Log::L_INFO, CLOGFMT(HttpC2StreamerEnv, "Stop the  SessionScaner"));
    }

#define UMG_Clear_Field(f) if(f) { delete f; f = NULL; }
    UMG_Clear_Field(eventUpdater);
    UMG_Clear_Field(sessScaner);
    UMG_Clear_Field(statusReportSvr);
    UMG_Clear_Field(handlerFac);
}
};
