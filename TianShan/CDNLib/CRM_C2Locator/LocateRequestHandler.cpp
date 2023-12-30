#include "LocateRequestHandler.h"
#include "TransferDeleteRequestHandler.h"
#include "../SimpleXMLParser.h"
#include "C2Env.h"
#include "C2LocatorImpl.h"
#include "ClientManager.h"
#include "TransferPortManager.h"
#include <strHelper.h>
#include <TianShanIceHelper.h>
#include <LAMFacade.h>
#include <Text.h>
#include <HttpClient.h>
#include <TsRepository.h>

#define FANGLI_EVIL

extern ZQTianShan::CDN::TransferDeleteRequestHandler* pTransferDeleteHandler;
namespace ZQTianShan{
namespace CDN{

static Ice::Long StringToLong(const std::string& s)
{
#ifdef ZQ_OS_MSWIN
    return ::_strtoi64(s.c_str(), NULL, 10);
#elif defined(__x86_64)
    return strtol(s.c_str(), NULL, 10);
#else
    return strtoll(s.c_str(), NULL, 10);
#endif
}

static std::string prefixOf(const std::string& s, char delimiter)
{
    return s.substr(0, s.find(delimiter));
}
bool compareString(const std::string& s1 , const std::string& s2)
{
	return s1 < s2;
}
struct LocateRequestData
{
    std::string objectId; // for the object identifier mode in comcast spec
    std::string assetId;
    std::string providerId;
    std::string subType;
    std::string extension; // file extension of the member file
    Ice::Long transferRate;

    std::string clientTransfer;
    Ice::Long ingressCapacity;

    Strings exclusionList;
    std::string range;
    Ice::Long transferDelay;
    LocateRequestData() {
        transferRate = 0;
        ingressCapacity = 0;
        transferDelay = 0;
    }
};

struct LocateResponseData
{
    std::string transferPort;
    std::string transferId;
    std::string openForWrite;
    std::string availableRange;
    std::string portNum; // listen port
	std::string idxContentGeneric;
	std::string idxContentSubfiles;
    Ice::Long transferTimeout;

	std::string reqSubType;//not for response output
	int32 exposeAssetIndexData;
    LocateResponseData():transferTimeout(-1),exposeAssetIndexData(0) {}
};

class LocateSpec
{
public:
    virtual bool parseRequest(const SimpleXMLParser::Node* root, LocateRequestData& reqData, std::string& error) const = 0;
    virtual void buildForwardRequest(const SimpleXMLParser::Node* origReq, const std::string& clientTransfer, std::string& doc) const = 0;
    virtual void buildResponse(const LocateResponseData& respData, std::string& doc) const = 0;
	void embedClipInfo( const LocateResponseData& respData, std::ostringstream& oss) const {
		if( respData.exposeAssetIndexData <= 0 )
			return;
		if( respData.reqSubType == CDN_SUBTYPE_Index || respData.reqSubType == CDN_SUBTYPE_NormalForward) {
			oss << "<ClipInfo>"<<std::endl;
			oss << "<Residential recording=\""<< (respData.openForWrite == "yes" ? 1 : 0) <<"\" />" << std::endl;
			oss << respData.idxContentGeneric << std::endl;
			if( respData.reqSubType == CDN_SUBTYPE_Index ) {
				oss << respData.idxContentSubfiles << std::endl;
			}
			oss << "</ClipInfo>"<<std::endl;
		}
	}
};

#ifdef FANGLI_EVIL

static std::string fixupPAIDPID(std::string& PAID, std::string& PID, const char outSeperator = '_', const char* inSeperators = "_")
{
	std::string contentName = PAID + PID;
	size_t pos;
	if (NULL != inSeperators)
	{
		pos = contentName.find_first_of(inSeperators);
		if (std::string::npos != pos)
		{
			PAID = contentName.substr(0, pos);
			PID = contentName.substr(pos + 1);
		}
	}

	pos = PAID.find_first_of("123456789");

	if (0 != pos && PAID.find_first_not_of("0") == pos)
		PAID = PAID.substr(pos);

	if (!isprint(outSeperator))
		contentName = PAID + PID;
	else
		contentName = PAID + outSeperator + PID;

	return contentName;
}
#endif // FANGLI_EVIL

class NGODC2LocateSpec : public LocateSpec
{
public:
	virtual bool parseRequest(const SimpleXMLParser::Node* root, LocateRequestData& reqData, std::string& error) const {
		
        typedef SimpleXMLParser::Node Node;
        // step 1: check the content and extract the post data
        const Node* locNode = findNode(root, "LocateRequest");
        if(NULL == locNode)
        { // bad xml content
            error = "230104 XML element missed: <LocateRequest>";
            return false;
        }

        const Node* objNode = findNode(locNode, "Object");
        if(!objNode)
        { // parameter missed
            error = "230104 XML element missed: <Object>";
            return false;
        }

        if(!objNode->content.empty())
        {
            reqData.objectId = objNode->content;
        }
        else
        {
            const Node* assetIdNode = findNode(locNode, "Object/Name/AssetID");
            reqData.assetId = assetIdNode ? assetIdNode->content : "";
            if(reqData.assetId.empty())
            { // parameter missed
                error = "230104 XML element missed: <AssetID>";
                return false;
            }

            const Node* providerIdNode = findNode(locNode, "Object/Name/ProviderID");
            reqData.providerId = providerIdNode ? providerIdNode->content : "";
            if(reqData.providerId.empty())
            { // parameter missed
                error = "230104 XML element missed: <ProviderID>";
                return false;
            }

            const Node* subTypeNode = findNode(locNode, "Object/SubType");
            reqData.subType = subTypeNode ? subTypeNode->content : "";
            if(reqData.subType.empty())
            { // parameter missed
                error = "230104 XML element missed: <SubType>";
                return false;
            }			

#ifdef FANGLI_EVIL
			fixupPAIDPID(reqData.assetId, reqData.providerId); // this was due to FangLi's evil idea in Hefei EDU2016-06
#endif // FANGLI_EVIL
        }

        const Node* clientTransferNode = findNode(locNode, "ClientTransfer");
        reqData.clientTransfer = clientTransferNode ? clientTransferNode->content : "";

        // Not reject the request when the TransferRate missed.
        const Node* transferRateNode = findNode(locNode, "TransferRate");
        reqData.transferRate = transferRateNode ? StringToLong(transferRateNode->content) : -1;

        const Node* ingressCapacityNode = findNode(locNode, "IngressCapacity");
        reqData.ingressCapacity = ingressCapacityNode ? StringToLong(ingressCapacityNode->content): -1;
        if(-1 == reqData.ingressCapacity)
        {
            // parameter missed
            error =  "230104 XML element missed: <IngressCapacity>";
            return false;
        }

        const Node* exclusionListNode = findNode(locNode, "ExclusionList");
        if(exclusionListNode && !exclusionListNode->content.empty())
        {
            ZQ::common::Text::split(reqData.exclusionList, exclusionListNode->content, ", ");

#pragma message(__MSGLOC__"should we verify the format of the address here?")
        }

        const Node* rangeNode = findNode(locNode, "Range");
        reqData.range = rangeNode ? rangeNode->content : "";

        // transfer delay
        const Node* transferDelayNode = findNode(locNode, "TransferDelay");
        reqData.transferDelay = 0;
        if(transferDelayNode && !transferDelayNode->content.empty())
        {
            reqData.transferDelay = StringToLong(transferDelayNode->content);
        }
        return true;
    }
    virtual void buildForwardRequest(const SimpleXMLParser::Node* origReq, const std::string& clientTransfer, std::string& doc) const {
        // fixup the content with original ClientTransfer
        SimpleXMLParser::Node origClient;
        origClient.name = "ClientTransfer";
        origClient.content = clientTransfer;
        buildXMLDocument(doc, *origReq, "/LocateRequest/ClientTransfer", origClient);
    }
    virtual void buildResponse(const LocateResponseData& respData, std::string& doc) const {
        std::ostringstream buf;
        buf << "<LocateResponse>\n";
        if(!respData.transferPort.empty())
            buf << "  <TransferPort>" << respData.transferPort << "</TransferPort>\n";
        if(!respData.transferId.empty())
            buf << "  <TransferID>" << respData.transferId << "</TransferID>\n";
        if(respData.transferTimeout > 0)
            buf << "  <TransferTimeout>" << respData.transferTimeout << "</TransferTimeout>\n";
        if(!respData.availableRange.empty())
            buf << "<AvailableRange>" << respData.availableRange << "</AvailableRange>\n";
        if(!respData.openForWrite.empty())
            buf << "<OpenForWrite>" << respData.openForWrite << "</OpenForWrite>\n";
		
		embedClipInfo(respData, buf );
		
        buf << "</LocateResponse>";

        buf.str().swap(doc);
    }
};


class NGBB1Spec : public NGODC2LocateSpec
{
public:
	virtual bool parseRequest(const SimpleXMLParser::Node* root, LocateRequestData& reqData, std::string& error) const {
		typedef SimpleXMLParser::Node Node;
		// step 1: check the content and extract the post data
		const Node* locNode = findNode(root, "LocateRequest");
		if(NULL == locNode)
		{ // bad xml content
			error = "230104 XML element missed: <LocateRequest>";
			return false;
		}

		const Node* objNode = findNode(locNode, "Object");
		if(!objNode)
		{ // parameter missed
			error = "230104 XML element missed: <Object>";
			return false;
		}

		if(!objNode->content.empty())
		{
			reqData.objectId = objNode->content;
			(reqData.subType).clear();//reqData.extension geted by regular expression.
		}
		else
		{
			const Node* assetIdNode = findNode(locNode, "Object/Name");
			if( NULL == assetIdNode || (assetIdNode->content).empty() )
			{ // parameter missed
				error = "230104 XML element missed: <Name>";
				return false;
			}

			reqData.objectId = assetIdNode->content;
			std::string subNodeContent;
			const Node* subTypeNode = findNode(locNode, "Object/SubType");
			if( NULL == subTypeNode || (subTypeNode->content).empty() )
			{ // parameter missed
				const Node* subFileNode = findNode(locNode, "Object/SubFile");
				if( NULL == subFileNode || (subFileNode->content).empty() )
				{
					error = "230104 XML element missed: <SubType> or <SubFile>";
					return false;
				}

				subNodeContent = subFileNode->content;
			}else{
				subNodeContent = subTypeNode->content;
			}

			if (std::string::npos == subNodeContent.find("."))
			    reqData.subType = ".";

			reqData.subType  += subNodeContent;
			reqData.objectId += reqData.subType; // for using resolveObject(...)  to parse
		}

		const Node* clientTransferNode = findNode(locNode, "ClientTransfer");
		reqData.clientTransfer = clientTransferNode ? clientTransferNode->content : "";

		// Not reject the request when the TransferRate missed.
		const Node* transferRateNode = findNode(locNode, "TransferRate");
		reqData.transferRate = transferRateNode ? StringToLong(transferRateNode->content) : -1;

		const Node* ingressCapacityNode = findNode(locNode, "IngressCapacity");
		reqData.ingressCapacity = ingressCapacityNode ? StringToLong(ingressCapacityNode->content): -1;
		if(-1 == reqData.ingressCapacity)
		{
			// parameter missed
			error =  "230104 XML element missed: <IngressCapacity>";
			return false;
		}

		const Node* exclusionListNode = findNode(locNode, "ExclusionList");
		if(exclusionListNode && !exclusionListNode->content.empty())
		{
			ZQ::common::Text::split(reqData.exclusionList, exclusionListNode->content, ", ");

#pragma message(__MSGLOC__"should we verify the format of the address here?")
		}

		const Node* rangeNode = findNode(locNode, "Range");
		reqData.range = rangeNode ? rangeNode->content : "";

		// transfer delay
		const Node* transferDelayNode = findNode(locNode, "TransferDelay");
		reqData.transferDelay = 0;
		if(transferDelayNode && !transferDelayNode->content.empty())
		{
			reqData.transferDelay = StringToLong(transferDelayNode->content);
		}

		return true;
	}
};

class SeaChangeLocateSpec : public LocateSpec
{
public:
    virtual bool parseRequest(const SimpleXMLParser::Node* root, LocateRequestData& reqData, std::string& error) const
    {
        typedef SimpleXMLParser::Node Node;
        // step 1: check the content and extract the post data
        const Node* locNode = findNode(root, "Query");
        if(NULL == locNode)
        { // bad xml content
            error = "230104 XML element missed: <Query>";
            return false;
        }
        const Node* assetIdNode = findNode(locNode, "AssetID");
        reqData.assetId = assetIdNode ? assetIdNode->content : "";
        if(reqData.assetId.empty())
        { // parameter missed
            error = "230104 XML element missed: <AssetID>";
            return false;
        }

        const Node* providerIdNode = findNode(locNode, "ProviderID");
        reqData.providerId = providerIdNode ? providerIdNode->content : "";
        if(reqData.providerId.empty())
        { // parameter missed
            error = "230104 XML element missed: <ProviderID>";
            return false;
        }

        const Node* subTypeNode = findNode(locNode, "SubFile");
		if (NULL == subTypeNode) 
			subTypeNode = findNode(locNode, "SubType"); // the bug of SeaHTTP 6.3BL21

        reqData.subType = subTypeNode ? subTypeNode->content : "";
        if(reqData.subType.empty())
        { // parameter missed
            error = "230104 XML element missed: <SubFile>";
            return false;
        }

        const Node* clientTransferNode = findNode(locNode, "ClientTransfer");
        reqData.clientTransfer = clientTransferNode ? clientTransferNode->content : "";

        // Not reject the request when the TransferRate missed.
        const Node* transferRateNode = findNode(locNode, "TransferRate");
        reqData.transferRate = transferRateNode ? StringToLong(transferRateNode->content) : -1;

        const Node* ingressCapacityNode = findNode(locNode, "IngressCapacity");
        reqData.ingressCapacity = ingressCapacityNode ? StringToLong(ingressCapacityNode->content): -1;
        if(-1 == reqData.ingressCapacity)
        {
            // parameter missed
            error =  "230104 XML element missed: <IngressCapacity>";
            return false;
        }

        const Node* rangeNode = findNode(locNode, "Range");
        reqData.range = rangeNode ? rangeNode->content : "";

        // transfer delay
        const Node* transferDelayNode = findNode(locNode, "TimeAdjust");
        reqData.transferDelay = 0;
        if(transferDelayNode && !transferDelayNode->content.empty())
        {
            reqData.transferDelay = StringToLong(transferDelayNode->content);
        }
        return true;
    }
    virtual void buildForwardRequest(const SimpleXMLParser::Node* origReq, const std::string& clientTransfer, std::string& doc) const
    {
        // fixup the content with original ClientTransfer
        SimpleXMLParser::Node origClient;
        origClient.name = "ClientTransfer";
        origClient.content = clientTransfer;
        buildXMLDocument(doc, *origReq, "/Query/ClientTransfer", origClient);
    }
    virtual void buildResponse(const LocateResponseData& respData, std::string& doc) const {
        std::ostringstream buf;
        buf << "<QueryResponse>\n";
        if(!respData.transferPort.empty()) {
            buf << "  <SourceAddress>" << respData.transferPort;
            if(!respData.portNum.empty())
                buf << ":" << respData.portNum;
            buf << "</SourceAddress>\n";
        }
        if(!respData.transferId.empty())
            buf << "  <Cookie>" << respData.transferId << "</Cookie>\n";
        if(respData.transferTimeout > 0)
            buf << "  <TransferTimeout>" << respData.transferTimeout << "</TransferTimeout>\n";
        if(!respData.availableRange.empty())
            buf << "<AvailableSize>" << respData.availableRange << "</AvailableSize>\n";
        if(!respData.openForWrite.empty())
            buf << "<PWE>" << respData.openForWrite << "</PWE>\n";
		
		embedClipInfo(respData, buf );
		
        buf << "</QueryResponse>";

        buf.str().swap(doc);
    }
};

static NGODC2LocateSpec gC2Spec;
static NGBB1Spec        gB1Spec;
static SeaChangeLocateSpec gSeacSpec;

namespace Helper
{
    class Action
    {
    public:
        virtual ~Action(){}
        virtual void perform(){}
    };

    class AutoExecuter
    {
    public:
        AutoExecuter(Action& act)
            :_act(act), _cancelled(false)
        {
        }
        ~AutoExecuter()
        {
            if(!_cancelled)
            {
                try{ _act.perform(); } catch(...) {}
            }
        }

        void cancel() { _cancelled = true; }
    private:
        Action& _act;
        bool _cancelled;
    };
}
///////////////
class RemoveClientReservation : public Helper::Action
{
public:
	RemoveClientReservation(ClientManager& clientMgr, const std::string& clientTransfer, const std::string reqId, Ice::Long allocatedBw)
        :_clientMgr(clientMgr), _clientTransfer(clientTransfer), _allocatedBw(allocatedBw), _reqId(reqId)
    {

    }
    virtual void perform()
    {
        _clientMgr.removeReservation(_clientTransfer, _reqId, _allocatedBw);
    }

private:
    ClientManager& _clientMgr;
    std::string _clientTransfer;
    Ice::Long   _allocatedBw;
	std::string _reqId;
};

class RemovePortReservation : public Helper::Action
{
public:
    RemovePortReservation(TransferPortManager& tpMgr, const std::string& portName, const std::string& reqId, Ice::Long allocatedBw)
        :_tpMgr(tpMgr), _portName(portName), _allocatedBw(allocatedBw), _reqId(reqId)
    {
    }
    virtual void perform()
    {
        _tpMgr.removeReservation(_portName, _reqId, _allocatedBw);
    }
private:
    TransferPortManager& _tpMgr;
    std::string _portName, _reqId;
    Ice::Long _allocatedBw;
};

class RemoveObjectFromEvictor : public Helper::Action
{
public:
	RemoveObjectFromEvictor(Freeze::EvictorPtr evi, Ice::Identity ident)
        :_evi(evi), _ident(ident)
    {
    }
    virtual void perform()
    {
        try{ _evi->remove(_ident); } catch(...) {}
    }
private:
    Freeze::EvictorPtr _evi;
    Ice::Identity _ident;
};
////////////////////////////////////////
//
LocateRequestHandler::LocateRequestHandler(C2Env& env, TianShanIce::SCS::C2LocatorImpl& locator)
    :_log(*env._pLog), _env(env), _locator(locator),
     _clientMgr(locator.getClientManager()),
     _tpMgr(locator.getTransferPortManager())
{
    // init the object resolvers
    std::vector<ObjectResolution>::const_iterator it = _env._conf.objectResolutions.begin();
    for(; it != _env._conf.objectResolutions.end(); ++it)
    {
        ObjectResolver resolver;
        try
        {
            resolver.matcher.assign(it->identifier);
        }catch(const boost::bad_expression& e)
        { // ignore this resolution
            _log(ZQ::common::Log::L_WARNING, CLOGFMT(LocateRequestHandler, "Bad expression [%s]. position: %d, type: %d, detail: %s"), it->identifier.c_str(), e.position(), e.code(), e.what());
            _log(ZQ::common::Log::L_WARNING, CLOGFMT(LocateRequestHandler, "Ignore the object resolution of type %s"), it->type.c_str());
            continue;
        }
        resolver.type = it->type;
        resolver.providerId = it->providerId;
        resolver.assetId = it->assetId;
        resolver.extension = it->extension;
        _objResolvers.push_back(resolver);
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateRequestHandler, "Objcet resolver of type %s is created"), resolver.type.c_str());
    }
    // init the forward url
    forwardUrl_ = _env._conf.forwardUrl;
    ZQ::common::Text::split(forwardExcludeStates_, _env._conf.forwardExcludeStates, ", ;");
    // verify the states? No necessary.
    _log(ZQ::common::Log::L_INFO, CLOGFMT(LocateRequestHandler, "Locate request handler created with %d resolvers. forward url=%s, forward exclude states=%s"), _objResolvers.size(), forwardUrl_.c_str(), _env._conf.forwardExcludeStates.c_str());

}

LocateRequestHandler::~LocateRequestHandler()
{
}
// calculate the priority value of the content state
static int getStatePriorityValue(const std::string& st) {
    static const char* ContentStateTbl[] = {
        "InService",
        "ProvisioningStreamable",
        "Provisioning",
        "NotProvisioned"
    };
    size_t i = 0;
    for(; i < sizeof ContentStateTbl / sizeof ContentStateTbl[0]; ++i) {
        if(0 == strcmp(st.c_str(), ContentStateTbl[i])) {
            break;
        }
    }
    return i;
}

#define HDR_Require "Require" // com.schange.cdn.v1 for SeaChange spec
#define HDR_ForwardHop "x-ForwardHop"
#define HDR_TianShan_Notice "TianShan-Notice"
#define ERROR_RESPONSE(RESP, CODE, REASON, NOTICE) RESP->setStatus(CODE, REASON);\
    if(_env._conf.tianshanNoticeEnabled) {RESP->setHeader(HDR_TianShan_Notice, NOTICE);}\
    _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "errorResponse() cost(%d) AssetId(%s)ProviderId(%s)SubType(%s)DemandedBy(%s)RequestedFrom(%s:%d)StatusCode(%d)ReasonPhrase(%s)Notice(%s)"), (int)(ZQ::common::now() - t1), reqData.assetId.c_str(), reqData.providerId.c_str(), (reqData.subType.empty() ? reqData.extension.c_str() : reqData.subType.c_str()), reqData.clientTransfer.c_str(), reqFrom.c_str(), reqFromPort, CODE, REASON, NOTICE)

// return next hop & cur hop
// return "" for reaching the max hop limit
static std::string getNextForwardHop(const char* hop, int maxHop, int& curHop) {
    curHop = NULL == hop ? 0 : atoi(hop);
    if(curHop < 0)
        curHop = 0;
    if(curHop < maxHop) {
        char hdrBuf[64];
        sprintf(hdrBuf, "%d", (curHop + 1));
        return hdrBuf;
    } else { // reach the max limit
        return "";
    }
}
void LocateRequestHandler::onRequest(const CRG::IRequest* req, CRG::IResponse* resp)
{
	std::string reqContent;
	req->getContent(reqContent);
	if (NULL != strstr(reqContent.c_str(), "<TransferIDDelete>"))
		return pTransferDeleteHandler->onRequest(req, resp);

    int64 t1 = ZQ::common::now(); // record the request cost
    std::string reqFrom;
    int reqFromPort;
    req->getClientEndpoint(reqFrom, reqFromPort);

    // got the spec
    std::string cdnType = "NGODC2";
    const LocateSpec* locateSpec = &gC2Spec;
    const char* requireString = req->header(HDR_Require);

    //if(NULL != strstr(req->uri(), "sccdn")) {
    if(NULL != requireString && NULL != strstr(requireString, "com.schange.cdn")) 
	{
        cdnType = "SeaChange";
        locateSpec = &gSeacSpec;
		resp->setHeader(HDR_Require, requireString); 
    }
	else if (NULL != strstr(req->uri(), "ngbBOne"))
    {
		cdnType = "ngbBOne";
		locateSpec = &gB1Spec;
    }

    std::string reqId = _env.reqIdGen.create();
    _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "LocateRequest from %s:%d, connetion:%lld"), reqFrom.c_str(), reqFromPort, req->getConnectionId());

    typedef SimpleXMLParser::Node Node;

    std::string reqDumpHint = "[req-" + reqId + "] LocateRequest:";
    _log.hexDump(ZQ::common::Log::L_DEBUG, reqContent.c_str(), reqContent.size(), reqDumpHint.c_str(), true); // text-only dump

    LocateRequestData reqData;
    SimpleXMLParser parser;
    try
    {
        parser.parse(reqContent.data(), reqContent.size(), 1);
    }catch(const ZQ::common::ExpatException& e)
    { // bad xml format
        _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "ExpatException: [%s] during parsing request body"), e.getString());
        ERROR_RESPONSE(resp, 400, "Bad Request", "230103 Bad XML format");
        return;
    }

    std::string error;
	if(!_env._auth.auth(&parser.document(),reqId)) {
		ERROR_RESPONSE(resp, 401, "Unauthorized", "Unauthorized");
		return;
	}
    // step 1: check the content and extract the post data
    if(!locateSpec->parseRequest(&parser.document(), reqData, error)) {
        ERROR_RESPONSE(resp, 400, "Bad Request", error.c_str());
        return;
    }

    if(!reqData.objectId.empty())
    { // parse the object id and get the pid/paid
        if(!resolveObject(reqData.objectId, reqData.providerId, reqData.assetId, reqData.extension))
        {
            ERROR_RESPONSE(resp, 400, "Bad Request", "230105 Invalid parameter: <ObjectIdentifier>");
            return;
        }
    }

    // convert the informal subtype into extension mode
	if(reqData.subType != CDN_SUBTYPE_Index && reqData.subType != CDN_SUBTYPE_NormalForward)
	{
		if (! reqData.subType.empty())
		{
			reqData.extension = reqData.subType;
			reqData.subType.clear();
		}
	}

    // fixup the bad subtype from vsis
    if(!reqData.extension.empty()) { // not standard subtype
        if( _env._conf.vsisFixup.illegalSubtypes.find(reqData.extension) != _env._conf.vsisFixup.illegalSubtypes.end() ) {
            reqData.providerId += reqData.extension;
            _log(ZQ::common::Log::L_INFO,ReqLOGFMT(LocateRequestHandler,"Got illegal subtype [%s]. Treat the object as main file and fixup pid to [%s]"), reqData.extension.c_str(), reqData.providerId.c_str());

            // update the subtype fields
            reqData.extension.clear();
            reqData.subType = CDN_SUBTYPE_NormalForward;
        }
    }

    // fixup the client transfer
    if(reqData.clientTransfer.empty()) {
        reqData.clientTransfer = reqFrom;
    }

    // nonstandard range format
    // support ',' and '-'
    char rangeDelimiter = reqData.range.find(',') != std::string::npos ? ',' : '-';
    if(rangeDelimiter != '-') {
        std::replace(reqData.range.begin(), reqData.range.end(), rangeDelimiter, '-');
    }

    std::string contentName;
    Ice::Long contentBitrate = 0;
    std::vector<std::string> volumeList;

    std::string localContentState; // keep the max state of the local content
    if(_env.ignoreLamWithTestContent) {
        ZQ::common::Text::Properties params;
        params["PID"] = reqData.providerId;
        params["PAID"] = reqData.assetId;
        try
        {
            contentName = ZQ::common::Text::format(_env.testContent.name, params);
        }
        catch(const ZQ::common::Text::FormattingException& e)
        {
            // log and return
            _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "Ignore LAM but failed to format the content name from local. msg:%s"), e.getString());
            ERROR_RESPONSE(resp, 500, "Internal Server Error", "230132 TestContent formatting error");
            return;
        }

        contentBitrate = _env.testContent.bandwidth;
        volumeList = _env.testContent.volumeList;
        _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "Ignore LAM with: content(%s), bandwidth(%lld), volumelist(%s)"), contentName.c_str(), contentBitrate, ZQ::common::Text::join(volumeList).c_str());
    } else if (!_env._conf.lamEndpoint.empty()) {
        // step 2: ask LAM for content location and bitrate
        com::izq::am::facade::servicesForIce::LAMFacadePrx lamPrx;
        com::izq::am::facade::servicesForIce::AEInfo3Collection ae3s;

        _log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(LocateRequestHandler, "Query content from LAM(%s): PID(%s), PAID(%s)"), _env._conf.lamEndpoint.c_str(), reqData.providerId.c_str(), reqData.assetId.c_str());
        int64 tQuery = ZQ::common::now();
		try
        {
            lamPrx = com::izq::am::facade::servicesForIce::LAMFacadePrx::uncheckedCast(_env._communicator->stringToProxy(_env._conf.lamEndpoint));
            ae3s = lamPrx->getAEListByPIdPAIdSId(reqData.providerId, reqData.assetId, "");
            if(ae3s.empty())
            {
                _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "Got empty aelist from LAM. PID=%s, PAID=%s"), reqData.providerId.c_str(), reqData.assetId.c_str());
                ERROR_RESPONSE(resp, 404, "Not Found", "230113 Got empty content list from AM");
                return;
            }
        }
        catch(const com::izq::am::facade::servicesForIce::LogicError& e)
        {
            _log(ZQ::common::Log::L_ERROR, ReqLOGFMT(LocateRequestHandler, "Caught LogicError from LAM for content: pid=%s paid=%s. code=%d, msg=%s"), reqData.providerId.c_str(), reqData.assetId.c_str(), e.errorCode, e.errorMessage.c_str());
            ERROR_RESPONSE(resp, 404, "Not Found", "230114 LogicError from AM");
            return;
        }
        catch(const Ice::Exception& e)
        {
            _log(ZQ::common::Log::L_ERROR, ReqLOGFMT(LocateRequestHandler, "Cautht %s from LAM for content: pid=%s paid=%s."), e.ice_name().c_str(), reqData.providerId.c_str(), reqData.assetId.c_str());
            char msgbuf[256];
            sprintf(msgbuf, "230132 Got unexpected %s from AM", e.ice_name().c_str());
            ERROR_RESPONSE(resp, 500, "Internal Server Error", msgbuf);
            return;
        }
        catch(...)
        {
            _log(ZQ::common::Log::L_ERROR, ReqLOGFMT(LocateRequestHandler, "Caught unknown exception from LAM for content: pid=%s paid=%s."), reqData.providerId.c_str(), reqData.assetId.c_str());
            ERROR_RESPONSE(resp, 500, "Internal Server Error", "230132 Got unknown exception from AM");
            return;
        }

        // only care the first entry of the aelist
        contentName = ae3s[0].name;
        contentBitrate = ae3s[0].bandWidth;
        volumeList.clear();
        if(/*!forwardUrl_.empty()*/1) // I am not sure if this is correct? HongQuan
		{
            const ::com::izq::am::facade::servicesForIce::StringCollection& vList = ae3s[0].volumeList;
            for(size_t i = 0; i < vList.size(); ++i) {
                std::string stateKey = "replicaStateOfVol_" + vList[i];
                ::com::izq::am::facade::servicesForIce::AttributesMap::const_iterator itState;
                itState = ae3s[0].attributes.find(stateKey);
                if(itState != ae3s[0].attributes.end()) {
                    if(localContentState.empty() || getStatePriorityValue(itState->second) < getStatePriorityValue(localContentState)) {
                        localContentState = itState->second;
                    }
                    if(std::find(forwardExcludeStates_.begin(), forwardExcludeStates_.end(), itState->second) != forwardExcludeStates_.end()) {
                        volumeList.push_back(vList[i]);
                    }
                }
            }
            {
                std::ostringstream attrsbuf;
                ::com::izq::am::facade::servicesForIce::AttributesMap::const_iterator itAttr;
                for(itAttr = ae3s[0].attributes.begin(); itAttr != ae3s[0].attributes.end (); ++itAttr) {
                    attrsbuf << itAttr->first << "=" << itAttr->second << ";";
                }
                std::string attrsstr;
                attrsbuf.str().swap(attrsstr);

                _log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(LocateRequestHandler, "Filter the volume list with (%s) state got %d volumes. attributes(%s)"), _env._conf.forwardExcludeStates.c_str(), volumeList.size(), attrsstr.c_str());
            }
        } else {
            volumeList = ae3s[0].volumeList;
        }
        _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "Query content from LAM got: content(%s), bandwidth(%lld), volumelist(%s), cost(%d)"), contentName.c_str(), contentBitrate, ZQ::common::Text::join(volumeList).c_str(), (int)(ZQ::common::now() - tQuery));
    } else if (!_env._conf.contentLibEndpoint.empty()) {
        // query content from content library
        TianShanIce::Repository::ContentLibPrx contentLib;
        TianShanIce::Repository::MetaObjectInfos metaObjs;
        _log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(LocateRequestHandler, "Query content from ContentLibrary(%s): PID(%s), PAID(%s)"), _env._conf.contentLibEndpoint.c_str(), reqData.providerId.c_str(), reqData.assetId.c_str());
        int64 tQuery = ZQ::common::now();
        try
        {
            contentLib = TianShanIce::Repository::ContentLibPrx::uncheckedCast(_env._communicator->stringToProxy(_env._conf.contentLibEndpoint));

            TianShanIce::StrValues metaNames;
            metaNames.push_back("sys.BitRate");
            metaObjs = contentLib->locateContentByPIDAndPAID("", "", reqData.providerId, reqData.assetId, metaNames);
            if(metaObjs.empty())
            {
                _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "No content return from ContentLibrary. PID=%s, PAID=%s"), reqData.providerId.c_str(), reqData.assetId.c_str());
                ERROR_RESPONSE(resp, 404, "Not Found", "230113 Got empty content list from ContentLib");
                return;
            }
        } catch (const Ice::Exception& e) {
            _log(ZQ::common::Log::L_ERROR, ReqLOGFMT(LocateRequestHandler, "Cautht %s from ContentLibrary for content: pid=%s paid=%s."), e.ice_name().c_str(), reqData.providerId.c_str(), reqData.assetId.c_str());
            char msgbuf[256];
            sprintf(msgbuf, "230132 Got unexpected %s from ContentLib", e.ice_name().c_str());
            ERROR_RESPONSE(resp, 500, "Internal Server Error", msgbuf);
            return;
        } catch(...) {
            _log(ZQ::common::Log::L_ERROR, ReqLOGFMT(LocateRequestHandler, "Caught unknown exception from ContentLibrary for content: pid=%s paid=%s."), reqData.providerId.c_str(), reqData.assetId.c_str());
            ERROR_RESPONSE(resp, 500, "Internal Server Error", "230132 Got unknown exception from ContentLib");
            return;
        }

        // gather the result
        TianShanIce::Repository::MetaObjectInfos::const_iterator it;
        for(it = metaObjs.begin(); it != metaObjs.end(); ++it) {
            if(contentBitrate == 0) {
                TianShanIce::Repository::MetaDataMap::const_iterator itMeta;
                itMeta = it->metaDatas.find("sys.BitRate");
                if(itMeta != it->metaDatas.end()) {
                    contentBitrate = StringToLong(itMeta->second.value);
                }
            }

            // format:ContentName@NetId$Volume
            std::string tmpNetId, tmpVolume;
            std::string::size_type off1 = it->id.find('@');
            if(off1 != std::string::npos) {
                if(contentName.empty()) {
                    contentName = it->id.substr(0, off1);
                }
                std::string::size_type off2 = it->id.find('$', off1 + 1);
                if(off2 != std::string::npos) {
                    tmpNetId = it->id.substr(off1 + 1, off2 - off1 - 1);
                    tmpVolume = it->id.substr(off2 + 1);
                }
            }
            if(off1 != 0 && !tmpNetId.empty() && !tmpVolume.empty()) {
                volumeList.push_back(tmpNetId + (tmpVolume[0] == '/' ? "" : "/") + tmpVolume);
            } else {
                _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "Bad format of content id(%s) from ContentLibrary"), it->id.c_str());
            }
        }

        _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "Query content from ContentLibrary got: content(%s), bandwidth(%lld), volumelist(%s), cost(%d)"), contentName.c_str(), contentBitrate, ZQ::common::Text::join(volumeList).c_str(), (int)(ZQ::common::now() - tQuery));
    } else {
        // bad configuration
        _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "Bad configuration: No content query method."));
        ERROR_RESPONSE(resp, 500, "Internal Server Error", "230133 Can't query content due to bad configuration");
    }

    // forward the request if need
    if(volumeList.empty()) {
        if(!forwardUrl_.empty()) {
        _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "Need forward the request to %s"), forwardUrl_.c_str());
        } else {
            _log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(LocateRequestHandler, "forwardRequest() No forward url is configured."));
            ERROR_RESPONSE(resp, 404, "Not Found", "230115 Content doesn't exist locally and the forward function is disabled");
            return;
        }

        std::map<std::string, std::string> forwardHeaders;

        // check the x-ForwardHop
        int nHop = 0;
        std::string nextForwardHop = getNextForwardHop(req->header(HDR_ForwardHop), _env._conf.maxHop, nHop);
        if(!nextForwardHop.empty()) {
            // include this header
            forwardHeaders[HDR_ForwardHop] = nextForwardHop;
        } else { // reject it
            _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "forwardRequest() Can't forward the request because the forward hop reach the max limit. " HDR_ForwardHop ":%d, MaxHop:%d"), nHop, _env._conf.maxHop);
            ERROR_RESPONSE(resp, 400, "Bad Request", "230106 The request had been forwarded too many");
            return;
        }
        if(NULL != requireString) {
            forwardHeaders[HDR_Require] = requireString;
        }
        // build the forward document
        std::string forwardDoc;
        locateSpec->buildForwardRequest(&parser.document(), reqData.clientTransfer, forwardDoc);
        int64 tForwardRequest = ZQ::common::now();
        if(forwardRequest(forwardDoc, resp, reqId, forwardHeaders)) {
            tForwardRequest = ZQ::common::now() - tForwardRequest;
            // log the successful forwarding
            _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "Forward the request and get: %d %s. cost(%lld)"), resp->getStatusCode(), resp->getReasonPhrase(), tForwardRequest);
            if(200 <= resp->getStatusCode() && resp->getStatusCode() < 300) {
                _env.hitCounter.recordHit(false); // remote hit
                _log(ZQ::common::Log::L_NOTICE, ReqLOGFMT(LocateRequestHandler, "RemoteAssetResolved: cost(%d) AssetId(%s)ProviderId(%s)SubType(%s)ContentName(%s)DemandedBy(%s)RemoteUrl(%s)RequestedFrom(%s:%d)StatusCode(%d)LocalState(%s)OutOfResource(false)"), (int)(ZQ::common::now() - t1), reqData.assetId.c_str(), reqData.providerId.c_str(), (reqData.subType.empty() ? reqData.extension.c_str() : reqData.subType.c_str()), contentName.c_str(), reqData.clientTransfer.c_str(), forwardUrl_.c_str(), reqFrom.c_str(), reqFromPort, resp->getStatusCode(), localContentState.c_str());
            }
        } else {
            ERROR_RESPONSE(resp, 404, "Not Found", "230116 Failed to forward the request");
        }
        return;
    } else {
        _env.hitCounter.recordHit(true); // local hit
    }

    // step 3: check client transfer restriction with allocated capacity
    //      3.1: compute allocated bandwidth
    Ice::Long allocatedBw = reqData.transferRate;
    if(reqData.transferRate <= 0)
    {
        if(reqData.subType == CDN_SUBTYPE_Index)
        {
            reqData.transferRate = 0;
            allocatedBw = _env._conf.indexFileTransferRate;
        }
        else
        { // use the encoded MEPG bitrate
            allocatedBw = reqData.transferRate = contentBitrate;
        }
    }

    if(reqData.transferDelay < 0 && reqData.subType != CDN_SUBTYPE_Index)
    {
        allocatedBw = reqData.transferRate * _env._conf.transferAheadRatePercent / 100;
    }

    Ice::Long clientIngressCapacity = reqData.ingressCapacity;
    if(_env._conf.ignoreIngressCapacity) {
        _log(ZQ::common::Log::L_INFO,ReqLOGFMT(LocateRequestHandler,"Overwrite IngressCapacity: %lld->%lld"), reqData.ingressCapacity, CDN_IngressCapacity_NoLimit);
        clientIngressCapacity = CDN_IngressCapacity_NoLimit;
    }
    //      3.2: try reserve the transfer
    if(_clientMgr.reserveTransfer(reqId, reqData.clientTransfer, clientIngressCapacity, allocatedBw))
    { // create the transfer session
    }
    else
    { // exceed the client capacity
        ERROR_RESPONSE(resp, 400, "Bad Request", "230101 Client IngressCapacity exceeded");
        return;
    }
    // Note: all error quit handling should include the removing reservation after the reserve
    RemoveClientReservation removeClientReservation(_clientMgr, reqData.clientTransfer, reqId, allocatedBw);
    // we need cancel the executer if no error occur
    Helper::AutoExecuter clientReservationRemover(removeClientReservation);

    TianShanIce::SCS::TransferSessionImpl::Ptr psess = new TianShanIce::SCS::TransferSessionImpl(_env);
    // add to the evictor
    Ice::Identity sessIdent;
    sessIdent.name = "S" + reqId;
    sessIdent.category = TransferSessionCategory;
    psess->ident = sessIdent;
    int64 tCreateSession = ZQ::common::now();
    TianShanIce::SCS::TransferSessionPrx sess = TianShanIce::SCS::TransferSessionPrx::uncheckedCast(_env._eC2TransferSession->add(psess, sessIdent));
    tCreateSession = ZQ::common::now() - tCreateSession;
    _log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(LocateRequestHandler, "created TransferSession:%s. cost(%lld)"), sessIdent.name.c_str(), tCreateSession);
    RemoveObjectFromEvictor removeTransferSession(_env._eC2TransferSession, sessIdent);
    Helper::AutoExecuter transferSessionRemover(removeTransferSession);
    TianShanIce::SCS::TransferSessionProp props;
    TianShanIce::SCS::TransferSessionImpl::initProperties(props);
    props.clientTransfer = reqData.clientTransfer;
    props.transferDelay = reqData.transferDelay;
    props.transferRate = reqData.transferRate;
    props.allocatedBW = allocatedBw;

    // append other properties
    ZQTianShan::Util::updateValueMapData(props.others, CDN_PID, reqData.providerId);
    ZQTianShan::Util::updateValueMapData(props.others, CDN_PAID, reqData.assetId);
    ZQTianShan::Util::updateValueMapData(props.others, CDN_SUBTYPE, reqData.subType);
    ZQTianShan::Util::updateValueMapData(props.others, CDN_EXTENSIONNAME, reqData.extension);


	bool bAssetStack = false;
	///add it for multi UML, choose one UML by paid 
	///////////////////////////////////////////////
	if(_env._conf.assetStack.enable)
	{  
		std::string strVolumes = "";
		uint32 nVolumeIndex = 0;

		std::vector<std::string>::iterator itorPID = std::find(_env._conf.assetStack.paids.begin(), _env._conf.assetStack.paids.end(), reqData.providerId);
		if (_env._conf.assetStack.paids.end() != itorPID && volumeList.size() > 1)
		{
			std::sort(volumeList.begin(), volumeList.end(), compareString);
			for(uint i = 0; i < volumeList.size(); i++)
			{
				strVolumes += volumeList[i] + ";";
			}
			_log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(LocateRequestHandler, "Pid[%s]Paid[%s]volume lists [%s]"),reqData.providerId.c_str(),reqData.assetId.c_str(), strVolumes.c_str());

			uint32 nPaidXOR = 0;
			if( reqData.assetId.size() > 0)
			{
				nPaidXOR =  reqData.assetId[0];
			}		
			for(uint i = 1; i < reqData.assetId.size(); i++)
			{
				nPaidXOR ^=  reqData.assetId[i];
			}

			nVolumeIndex = nPaidXOR % volumeList.size();

			///if the volumeIndex == 0, don't need adjust sequence of volume list
			if(nVolumeIndex != 0)
			{
				std::vector<std::string> volumeListTemp(volumeList.begin(), volumeList.end());
				volumeList.clear();
				for(uint i = nVolumeIndex; i < volumeListTemp.size(); i++)
				{
					volumeList.push_back(volumeListTemp[i]);
				}

				for(uint i = 0; i < nVolumeIndex; i++)
				{
					volumeList.push_back(volumeListTemp[i]);
				}
			}

			strVolumes.clear();
			for(uint i = 0; i < volumeList.size(); i++)
			{
				strVolumes += volumeList[i] + ";";
			}
			_log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "Pid[%s]Paid[%s] VolumeIndex[%d], adjusted volume lists [%s] per asset stack"), reqData.providerId.c_str(),reqData.assetId.c_str(),nVolumeIndex, strVolumes.c_str());
			bAssetStack = true;
		}
	}
	/////////////////////////////////////////

    // step 4: select a proper transfer port
    //      4.1: find out the ports that can serve the transfer
    //      4.2: select a port base on the selection algorithm
    // 4.1, 4.2 are included in the TransferPortManager's logic
    Strings portGroupList, exclusionList, lowPrioritySSList;
    {
        com::izq::am::facade::servicesForIce::StringCollection::const_iterator itVol;
        for(itVol = volumeList.begin(); itVol != volumeList.end(); ++itVol)
        {
            std::string netid = prefixOf(*itVol, '/');
            if(!netid.empty())
            {
                portGroupList.push_back(netid);
            }
        }
    }
    exclusionList = reqData.exclusionList;
    // ignore the exclusion if need
    bool ignoreExclusion = (!exclusionList.empty() && _env._conf.ignoreExclusion);
    ::TianShanIce::SCS::TransferPort port;
    SubnetFilter ipFilter;
    ipFilter.target4 = reqData.clientTransfer; // only support IPv4 request at present
    ipFilter.mask4 = _env._conf.subnetMask4;

    // retry logic here
    int maxRetryLimit = _env._conf.selectionRetryMax;
    for(int nTring = 1; nTring <= maxRetryLimit; ++nTring)
    {
        if(_tpMgr.selectAndReserve(reqId, port, allocatedBw, portGroupList, exclusionList, lowPrioritySSList, ipFilter, bAssetStack))
        {
            // fill the transfer session with the selected port info
            props.transferPort = port.name;
            sess->setProps2(props);
            // fill the ticket srm info
            TianShanIce::SRM::Resource res;
            res.status = TianShanIce::SRM::rsRequested;
            res.attr = TianShanIce::SRM::raMandatoryNonNegotiable;

            ZQTianShan::Util::updateValueMapData(res.resourceData, "Type", "NetworkId");
            // get the netid from port name
            // format: [csid/]ssid/portid
            std::string networkId;
            std::vector<std::string> portInfo;

            ZQ::common::Text::split(portInfo, port.name, "/");
            if(portInfo.size() <= 2)
                networkId = port.name;
            else
                networkId = portInfo[portInfo.size() - 2] + "/" + portInfo[portInfo.size() - 1];
            ZQTianShan::Util::updateValueMapData(res.resourceData, "NetworkId", networkId);

            TianShanIce::SRM::ResourceMap resMap;
            resMap[TianShanIce::SRM::rtStreamer] = res;
            sess->setResources(resMap);
        }
        else
        { // no port available
            if(ignoreExclusion) {
                ignoreExclusion = false;
                _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "Retry with the excluded ports:%s"), ZQ::common::Text::join(reqData.exclusionList).c_str());
                // remove the original ExclusionList
                if(exclusionList.size() >= reqData.exclusionList.size()) {
                    exclusionList = Strings(exclusionList.begin() + reqData.exclusionList.size(), exclusionList.end());
                    maxRetryLimit += reqData.exclusionList.size();
                } else {
                    _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "abnormal exclusion list:%s in the %d tries"), ZQ::common::Text::join(exclusionList).c_str(), nTring);
                    exclusionList.clear();
                }
                continue;
            } else {
                _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "No transfer port available.(tried %d times)"), nTring);
                if(!forwardUrl_.empty() && _env._conf.forwardOnOutOfResource) {
                    _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "Resource run out. Need forward the request to %s"), forwardUrl_.c_str());
#pragma message(__MSGLOC__"These code are copied from other place!!")


                    std::map<std::string, std::string> forwardHeaders;
                    // check the x-ForwardHop
                    int nHop = 0;
                    std::string nextForwardHop = getNextForwardHop(req->header(HDR_ForwardHop), _env._conf.maxHop, nHop);
                    if(!nextForwardHop.empty()) {
                        // include this header
                        forwardHeaders[HDR_ForwardHop] = nextForwardHop;
                    } else { // reject it
                        _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "forwardRequest() Can't forward the request because the forward hop reach the max limit. " HDR_ForwardHop ":%d, MaxHop:%d"), nHop, _env._conf.maxHop);
                        ERROR_RESPONSE(resp, 400, "Bad Request", "230106 The request had been forwarded too many");
                        return;
                    }
                    if(NULL != requireString) {
                        forwardHeaders[HDR_Require] = requireString;
                    }
                    // build the forward document
                    std::string forwardDoc;
                    locateSpec->buildForwardRequest(&parser.document(), reqData.clientTransfer, forwardDoc);
                    int64 tForwardRequest = ZQ::common::now();
                    if(forwardRequest(forwardDoc, resp, reqId, forwardHeaders)) {
                        tForwardRequest = ZQ::common::now() - tForwardRequest;
                        // log the successful forwarding
                        _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "Forward the request and get: %d %s. cost(%lld)"), resp->getStatusCode(), resp->getReasonPhrase(), tForwardRequest);
                        if(200 <= resp->getStatusCode() && resp->getStatusCode() < 300) {
                            _log(ZQ::common::Log::L_NOTICE, ReqLOGFMT(LocateRequestHandler, "RemoteAssetResolved: cost(%d) AssetId(%s)ProviderId(%s)SubType(%s)ContentName(%s)DemandedBy(%s)RemoteUrl(%s)RequestedFrom(%s:%d)StatusCode(%d)LocalState(%s)OutOfResource(true)"), (int)(ZQ::common::now() - t1), reqData.assetId.c_str(), reqData.providerId.c_str(), (reqData.subType.empty() ? reqData.extension.c_str() : reqData.subType.c_str()), contentName.c_str(), reqData.clientTransfer.c_str(), forwardUrl_.c_str(), reqFrom.c_str(), reqFromPort, resp->getStatusCode(), localContentState.c_str());
                        }
                    } else {
                        ERROR_RESPONSE(resp, 404, "Not Found", "230116 Failed to forward the request");
                    }
                } else {
                    ERROR_RESPONSE(resp, 503, "Service Unavailable", "230141 No transfer port is available for a new locate");
                }
                return;
            }
        }

        RemovePortReservation removeTransferPortReservation(_tpMgr, port.name, reqId, allocatedBw);
        Helper::AutoExecuter transferPortReservationRemover(removeTransferPortReservation);

        // chose an address
        std::string portAddress = (!port.addressListIPv4.empty() ? port.addressListIPv4[0] : (!port.addressListIPv6.empty() ? port.addressListIPv6[0] : ""));
        if(portAddress.empty())
        {
            _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "Transfer port selected (%s) but no address info. (tried %d times)"), port.name.c_str(), nTring);
            if(!port.streamService.empty())
            {
                lowPrioritySSList.push_back(port.streamService);
            }
            continue;
        }

        // chosen a volume
        std::string vol;
        std::string selectedGroup = prefixOf(port.name, '/');
        {
            std::string volPrefix = selectedGroup + "/";
            com::izq::am::facade::servicesForIce::StringCollection::const_iterator itVol;
            for(itVol = volumeList.begin(); itVol != volumeList.end(); ++itVol)
            {
#define startsWith(s1, s2) (0 == (s1).compare(0, (s2).size(), (s2)))
                if(startsWith(*itVol, volPrefix))
                {
                    vol = itVol->substr(volPrefix.size());
					_log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(LocateRequestHandler, "chosen volume[%s] by selectedGroup[%s], portname[%s]"), itVol->c_str(), volPrefix.c_str(), port.name.c_str());

                    break;
                }
            }
        }
        _log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(LocateRequestHandler, "Selection and reservation are prepared."));
        // step 5: reserve the transfer resource on the streamer
        std::string contentFullName;
        try
        {
            Ice::ObjectPrx prx = _env._communicator->stringToProxy(port.streamService);
            TianShanIce::Streamer::StreamServicePrx ss = TianShanIce::Streamer::StreamServicePrx::uncheckedCast(prx);
            TianShanIce::SCS::TransferSessionPrx sessPrx = C2IdentityToObjEnv(_env, TransferSession, sessIdent);

            // set the timeout
            sessPrx->setExpiration(_env.iTransferSessionTimeout);
            TianShanIce::Streamer::StreamPrx stream;
            try {
                int64 tCreateStream = ZQ::common::now();
                //stream = ss->createStream(sessPrx);
				stream = ss->createStream( NULL );// do not use pathticket to create stream
                sess->setStream(stream);
                tCreateStream = ZQ::common::now() - tCreateStream;
                _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "stream(%s) created at %s. cost(%lld)"), _env._communicator->proxyToString(stream).c_str(), port.streamService.c_str(), tCreateStream);
                _tpMgr.recordStreamCreated(port.name, true);
            }
			catch( const Ice::SocketException& ex)
			{
				_tpMgr.recordStreamCreated(port.name, false);
				_tpMgr.addPenalty(port.name);
				_log(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "[%s] catch [%s] when createStream [%s]"), _env._communicator->proxyToString(stream).c_str(), ex.ice_name().c_str(), port.streamService.c_str());
				throw;
			}
			catch( const Ice::TimeoutException& ex)
			{
				_tpMgr.recordStreamCreated(port.name, false);
				_tpMgr.addPenalty(port.name);
				_log(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "[%s] catch [%s] when createStream [%s]"),  _env._communicator->proxyToString(stream).c_str(), ex.ice_name().c_str(), port.streamService.c_str());
				throw;
			}
			catch(const TianShanIce::BaseException& e)
			{
				_tpMgr.recordStreamCreated(port.name, false);
				_tpMgr.addPenalty(port.name);
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2LocatorImpl, "[%s] TianShan Exception: %s during createStream [%s]. category=%s code=%d message=%s."),  _env._communicator->proxyToString(stream).c_str(), e.ice_name().c_str(), port.streamService.c_str(), e.category.c_str(), e.errorCode, e.message.c_str());
				throw;
			}
			catch (::Ice::Exception& ex)
			{
				_log(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "[%s] catch [%s] when createStream [%s]"), _env._communicator->proxyToString(stream).c_str(), ex.ice_name().c_str(), port.streamService.c_str());
				throw;
			}
			catch (...) {
				_log(::ZQ::common::Log::L_ERROR,CLOGFMT(C2LocatorImpl, "[%s] catch [unknown] when createStream [%s]"), _env._communicator->proxyToString(stream).c_str(), port.streamService.c_str());
                throw;
            }
            TianShanIce::Streamer::PlaylistPrx pl = TianShanIce::Streamer::PlaylistPrx::uncheckedCast(stream);
            TianShanIce::Streamer::PlaylistItemSetupInfo plInfo;

            try
            {
                ZQ::common::Text::Properties params;
                params["Volume"] = vol;
                params["Name"] = contentName;
                contentFullName = ZQ::common::Text::format(_env._conf.contentFullNameFmt, params);
                plInfo.contentName = contentFullName;
                _log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(LocateRequestHandler, "Request file %s"), plInfo.contentName.c_str());
            }
            catch(const ZQ::common::Text::FormattingException& e)
            {
                // the format string is bad, and this situation is considered unrecoverable
                _log(ZQ::common::Log::L_ERROR, ReqLOGFMT(LocateRequestHandler, "Can't format the content full name. msg:%s"), e.getString());
                ERROR_RESPONSE(resp, 500, "Internal Server Error", "230134 FullContentName format error");
                return;
            }

            plInfo.inTimeOffset = 0;
            plInfo.outTimeOffset = 0;
            plInfo.criticalStart = 0;
            plInfo.spliceIn = false;
            plInfo.spliceOut = false;
            plInfo.forceNormal = false;
            plInfo.flags = 0;
            // fill the fields in the private data
            ZQTianShan::Util::updateValueMapData(plInfo.privateData, CDN_CLIENTTRANSFER, reqData.clientTransfer);
            ZQTianShan::Util::updateValueMapData(plInfo.privateData, CDN_TRANSFERADDRESS, portAddress);
            ZQTianShan::Util::updateValueMapData(plInfo.privateData, CDN_INGRESSCAPACITY, reqData.ingressCapacity);
            ZQTianShan::Util::updateValueMapData(plInfo.privateData, CDN_ALLOCATEDCAPACITY, allocatedBw);
            ZQTianShan::Util::updateValueMapData(plInfo.privateData, CDN_TRANSFERRATE, reqData.transferRate);
            ZQTianShan::Util::updateValueMapData(plInfo.privateData, CDN_DELAY, reqData.transferDelay);

            ZQTianShan::Util::updateValueMapData(plInfo.privateData, CDN_SUBTYPE, reqData.subType);
            ZQTianShan::Util::updateValueMapData(plInfo.privateData, CDN_EXTENSIONNAME, reqData.extension);

            if(!reqData.range.empty())
                ZQTianShan::Util::updateValueMapData(plInfo.privateData, CDN_RANGE, reqData.range);

            // set the CDN Type
            ZQTianShan::Util::updateValueMapData(plInfo.privateData, CDN_CDNTYPE, cdnType);
            int64 tPushBack = ZQ::common::now();
            pl->pushBack(0, plInfo);
            tPushBack = ZQ::common::now() - tPushBack;
            _log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(LocateRequestHandler, "push back item cost(%lld)"), tPushBack);
        } catch(const TianShanIce::InvalidParameter& e) {
            // the content not found on this cndss
			if( e.errorCode == TianShanIce::errcodeAssetNotfound ) {
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(LocateRequestHandler, "Exception: AssetNotFound during create stream at [%s]. (tried %d times)"), port.streamService.c_str(), nTring);
				if(!portAddress.empty())
				{
					exclusionList.push_back(portAddress);
				}
				if(!port.streamService.empty())
				{
					lowPrioritySSList.push_back(port.streamService);
				}
				continue;
			} else {
				_log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "create stream on [%s]%d caught exception[%s]:%s"), port.streamService.c_str(), nTring, e.ice_name().c_str(), e.message.c_str());
				char msgbuf[256];
				sprintf(msgbuf, "230111 Got InvalidParameter from streaming service. desc:%s", e.message.c_str());
				ERROR_RESPONSE(resp, 404, "Not Found", msgbuf);
				return;
			}
        } catch(const TianShanIce::BaseException& e) {
            _log(ZQ::common::Log::L_ERROR, ReqLOGFMT(LocateRequestHandler, "create stream on [%s]%d caught exception[%s]:%s"), port.streamService.c_str(), nTring, e.ice_name().c_str(), e.message.c_str());
            if(!portAddress.empty())
            {
                exclusionList.push_back(portAddress);
            }
            if(!port.streamService.empty())
            {
                lowPrioritySSList.push_back(port.streamService);
            }
            continue;
        }
        catch(const Ice::Exception& e)
        {
            _log(ZQ::common::Log::L_ERROR, ReqLOGFMT(LocateRequestHandler, "Exception: %s during create stream at [%s]. (tried %d times)"), e.ice_name().c_str(), port.streamService.c_str(), nTring);
            if(!portAddress.empty())
            {
                exclusionList.push_back(portAddress);
            }
            if(!port.streamService.empty())
            {
                lowPrioritySSList.push_back(port.streamService);
            }
            continue;
        }
        catch(...)
        {
            _log(ZQ::common::Log::L_ERROR, ReqLOGFMT(LocateRequestHandler, "unknown exception during create stream at [%s]. (tried %d times)"), port.streamService.c_str(), nTring);
            if(!portAddress.empty())
            {
                exclusionList.push_back(portAddress);
            }
            if(!port.streamService.empty())
            {
                lowPrioritySSList.push_back(port.streamService);
            }
            continue;
        }
        int64 tCommit = ZQ::common::now();
        int err = 0;
        if(_locator.commit(reqId, sess, err))
        {
            tCommit = ZQ::common::now() - tCommit;
            _log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(LocateRequestHandler, "commit and play. cost(%lld)"), tCommit);

            // step 6: response OK
            // Content sample:
            //
            // <LocateResponse>
            //   <TransferPort>192.20.10.1</TransferPort>
            //   <TransferID>123456789ABCDEF0123456789ABCDEF0</TransferID>
            //   <TransferTimeout>500</TransferTimeout>
            // </LocateResponse>

            LocateResponseData respData;
			respData.reqSubType = reqData.subType;  //record request subtype
			respData.exposeAssetIndexData = _env._conf.exposeAssetIndexData;

            props = sess->getProps();
            respData.transferPort = portAddress;
            respData.transferId = props.transferId;
            respData.transferTimeout = props.transferTimeout;
            ZQTianShan::Util::getValueMapDataWithDefault(props.others, CDN_TRANSFERPORTNUM, "", respData.portNum);

            // check the AvailableRange and OpenForWrite
#pragma message(__MSGLOC__"May need more check for these parameters")
            std::string availableRange, openForWrite;
            // TODO: query AvailableRange and OpenForWrite from MCS
            std::string csep = getContentStore(selectedGroup);
            if(!csep.empty()) {
                TianShanIce::Storage::ContentStorePrx cs;
                cs = TianShanIce::Storage::ContentStorePrx::uncheckedCast(_env._communicator->stringToProxy(csep));

                if(cs) {
                    try {
                        // open content
                        TianShanIce::Storage::ContentPrx content = cs->openContentByFullname(contentFullName);
                        // check state
                        TianShanIce::Storage::ContentState state = content->getState();
                        openForWrite = (state == TianShanIce::Storage::csProvisioning || state == TianShanIce::Storage::csProvisioningStreamable) ? "yes" : "no";
                        // get metadata
                        TianShanIce::Properties md = content->getMetaData();
                        std::string subfileExt;
                        if(reqData.subType.empty()) {
                            subfileExt = reqData.extension;
                        } else if (reqData.subType == CDN_SUBTYPE_Index) {
                            subfileExt = md["sys.IndexFileExt"];
                            _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "Got Metadata: sys.IndexFileExt(%s)"), subfileExt.c_str());
                        } // else: main file, keep the ext empty
                        // standardize the ext
                        ZQ::common::Text::trim(subfileExt, ". ");

                        // check the subfile, EMPTY value works here
                        if(std::string::npos != md["sys.memberFileExts"].find(subfileExt)) {
                            std::string firstOffsetKey = "sys.FirstOffset" + (subfileExt.empty() ? "" : ("." + subfileExt));
                            std::string fileSizeKey = "sys.FileSize" + (subfileExt.empty() ? "" : ("." + subfileExt));
                            _log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(LocateRequestHandler, "Got Metadata: %s(%s), %s(%s)"), firstOffsetKey.c_str(), md[firstOffsetKey].c_str(), fileSizeKey.c_str(), md[fileSizeKey].c_str());
                            availableRange = md[firstOffsetKey] + "-" + md[fileSizeKey];
                            _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "Got AvailableRange(%s) OpenForWrite(%s) of [%s@%s@%s]"),availableRange.c_str(), openForWrite.c_str(),(subfileExt.empty() ? "Main" : subfileExt.c_str()), contentFullName.c_str(), csep.c_str());
                            if(md[firstOffsetKey].empty() || md[fileSizeKey].empty()) { // ignore invalid value
                                _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "Ignore invalid AvailableRange(%s)"), availableRange.c_str());
                                availableRange.clear();
                            }
                        } else { // invalid subfile
                            _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "Failed to query the attributes of content[%s@%s]: invalid subfile(%s). valid subfile set [%s]"), contentFullName.c_str(), csep.c_str(), subfileExt.c_str(), md["sys.memberFileExts"].c_str());
                        }
                    } catch (const Ice::Exception& e) {
                        _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "Failed to query the attributes of content[%s@%s]: %s"), contentFullName.c_str(), csep.c_str(), e.ice_name().c_str());
                        openForWrite.clear();
                        availableRange.clear();
                    } catch (...) {
                        _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "Failed to query the attributes of content[%s@%s]: Unexpected exception"), contentFullName.c_str(), csep.c_str());
                        openForWrite.clear();
                        availableRange.clear();
                    }
                }
            }
            if(_env._conf.overwriteAvailableRange || availableRange.empty()) {
                try{
                    ZQTianShan::Util::getValueMapData(props.others, CDN_AVAILRANGE, availableRange);
                }catch(...){}
            }
            if(openForWrite.empty()) {
                try{
                    ZQTianShan::Util::getValueMapData(props.others, CDN_OPENFORWRITE, openForWrite);
                }catch(...){}
            }

            if(!availableRange.empty()) {
                // nonstandard range format
                if(rangeDelimiter != '-') {
                    std::replace(availableRange.begin(), availableRange.end(), '-', rangeDelimiter);
                }
                respData.availableRange = availableRange;
            }
            respData.openForWrite = openForWrite;

            resp->setStatus(201, "Created");
            std::string respContent;
            locateSpec->buildResponse(respData, respContent);
            resp->setContent(respContent.data(), respContent.size());

            std::string respDumpHint = "[req-" + reqId + "] LocateResponse:";
            _log.hexDump(ZQ::common::Log::L_DEBUG, respContent.c_str(), respContent.size(), respDumpHint.c_str(), true); // text-only dump

            // cancel the resource remover
            clientReservationRemover.cancel();
            transferPortReservationRemover.cancel();
            transferSessionRemover.cancel();
            _log(ZQ::common::Log::L_NOTICE, ReqLOGFMT(LocateRequestHandler, "TransferCreated: cost(%d) AssetId(%s)ProviderId(%s)SubType(%s)ContentName(%s)DemandedBy(%s)TransferPort(%s)TransferId(%s)RequestedFrom(%s:%d)StatusCode(%d)ReservedBw(%lld)ReservedAt(%s)"), (int)(ZQ::common::now() - t1), reqData.assetId.c_str(), reqData.providerId.c_str(), (reqData.subType.empty() ? reqData.extension.c_str() : reqData.subType.c_str()), contentName.c_str(), reqData.clientTransfer.c_str(), portAddress.c_str(), props.transferId.c_str(), reqFrom.c_str(), reqFromPort, resp->getStatusCode(), allocatedBw, port.name.c_str());
            return;
        }
        else
        {
            _log(ZQ::common::Log::L_ERROR, ReqLOGFMT(LocateRequestHandler, "failed to commit the transfer session %s. errorCode=%d (tried %d times)"), sessIdent.name.c_str(), err, nTring);
            // just response on the famous error
            if(err == 400) {
                ERROR_RESPONSE(resp, 400, "Bad Request", "230102 Rejected by streaming service");
                return;
            } else if (err == 404) {
                ERROR_RESPONSE(resp, 404, "Not Found", "230112 Rejected by streaming service");
                return;
            } else if (err == 416) {
                ERROR_RESPONSE(resp, 416, "Requested Range Not Satisfiable", "230121 Rejected by streaming service");
                // set the <AvailableRange> for seahttp
                const char* userAgent = req->header("User-Agent");
                if(userAgent != NULL && 0 == strcmp(userAgent, "ToInfinityAndBeyond")) {
                    props = sess->getProps();
                    std::string availableRange;
                    try{
                        ZQTianShan::Util::getValueMapData(props.others, CDN_AVAILRANGE, availableRange);
                    }catch(...){}

                    if(!availableRange.empty()) {
                        // nonstandard range format
                        if(rangeDelimiter != '-') {
                            std::replace(availableRange.begin(), availableRange.end(), '-', rangeDelimiter);
                        }
                        LocateResponseData respData;
                        respData.availableRange = availableRange;
                        std::string respContent;
                        locateSpec->buildResponse(respData, respContent);
                        resp->setContent(respContent.data(), respContent.size());
                        _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "Provide AvailableRange(%s) for seahttp in 416 response"), availableRange.c_str());
                    } else {
                        _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "Can't get AvailableRange for seahttp in 416 response"));
                    }
                }
                return;
            } 
			else {

                if(err < 0 )
				{
					_tpMgr.addPenalty(port.name);//only add penalty when encounter a socket exception or timeout exception
					_log(ZQ::common::Log::L_NOTICE, ReqLOGFMT(LocateRequestHandler, "Unexpected error(%d) when committing, add penalty to %s"), err, port.name.c_str());
				}
				else _log(ZQ::common::Log::L_NOTICE, ReqLOGFMT(LocateRequestHandler, "Unexpected error(%d) when committing to %s"), err, port.name.c_str());
            }

            if(!portAddress.empty())
            {
                exclusionList.push_back(portAddress);
            }
            if(!port.streamService.empty())
            {
                lowPrioritySSList.push_back(port.streamService);
            }
            continue;
        }
    }
    // after max tries but can't located the transfer successfully
    _log(ZQ::common::Log::L_INFO, ReqLOGFMT(LocateRequestHandler, "Can't locate the transfer of session %s after %d tried"), sessIdent.name.c_str(), maxRetryLimit);
    ERROR_RESPONSE(resp, 500, "Internal Server Error", "230131 Can't locate afert the max retries.");
    return;
}
bool LocateRequestHandler::resolveObject(const std::string& identifier, std::string& providerId, std::string& assetId, std::string& extension) const
{
    for(ObjectResolvers::const_iterator it = _objResolvers.begin(); it != _objResolvers.end(); ++it)
    {
        boost::smatch m;
        if(boost::regex_match(identifier, m, it->matcher))
        {
            try
            {
                providerId = m.format(it->providerId);
                assetId = m.format(it->assetId);
                extension = m.format(it->extension);
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateRequestHandler, "resolveObject() resolved object identifier [%s] into: pid [%s], paid [%s], ext [%s]"), identifier.c_str(), providerId.c_str(), assetId.c_str(), extension.c_str());
                return true;
            }catch(const std::exception& e)
            {
                _log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateRequestHandler, "resolveObject() resolution [%s] matched but get exception:[%s] during formatting the attributes"), it->type.c_str(), e.what());
                continue;
            }
        }
    }
	//bug id #19414
	{
		// object id = [.]00NVOD1234567890abcdef_cctv$com
		std::string suffix;
		size_t nDotPos = identifier.rfind('.');
		if (std::string::npos != nDotPos)
		{
			suffix = identifier.substr(nDotPos + 1);
		}else{
			suffix = identifier;
		}
		
		boost::regex reg("#");
		suffix = boost::regex_replace(suffix, reg, ".");
		if ("index" == suffix) return false;

		extension = identifier;

		size_t nUnderlinePos = suffix.rfind('_');
		providerId = suffix.substr(nUnderlinePos + 1);

		assetId = suffix.substr(2, nUnderlinePos - 2);

	}
    //_log(ZQ::common::Log::L_WARNING, CLOGFMT(LocateRequestHandler, "resolveObject() failed to resolve object identifier [%s]"), identifier.c_str());
    return true;
}

bool LocateRequestHandler::forwardRequest(const std::string& fReq, CRG::IResponse* resp, const std::string& reqId, const std::map<std::string, std::string>& headers) {
    using namespace ZQ::common;
    // build the request content
    HttpClient client(&_log);
    // set parameters
    client.init();
    if(0 != client.httpConnect(forwardUrl_.c_str(), HttpClient::HTTP_POST)) {
        _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "forwardRequest() failed to connect the forward url [%s]. error[%d:%s]"), forwardUrl_.c_str(), client.getErrorcode(), client.getErrorstr());
        return false;
    }
    for(std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        client.setHeader((char*)(it->first.c_str()), (char*)(it->second.c_str()));
    }
    if(0 != client.httpSendContent(fReq.data(), fReq.size())) {
        _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "forwardRequest() failed to send data to the forward url [%s]. data size[%d], error[%d:%s]"), forwardUrl_.c_str(), fReq.size(), client.getErrorcode(), client.getErrorstr());
        return false;
    }
    if(0 != client.httpEndSend()) {
        _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "forwardRequest() failed to send data to the forward url [%s] (EndSend). error[%d:%s]"), forwardUrl_.c_str(), client.getErrorcode(), client.getErrorstr());
        return false;
    }

    if(0 != client.httpBeginRecv()) {
        _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "forwardRequest() failed to receive response data from forward url [%s] (BeginRecv). error[%d:%s]"), forwardUrl_.c_str(), client.getErrorcode(), client.getErrorstr());
    }
    while(!client.isEOF()) {
        if(0 != client.httpContinueRecv()) {
            _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "forwardRequest() failed to receive response data from forward url [%s] (ContinueRecv). error[%d:%s]"), forwardUrl_.c_str(), client.getErrorcode(), client.getErrorstr());
            return false;
        }
    }
    if(0 != client.httpEndRecv()) {
        _log(ZQ::common::Log::L_WARNING, ReqLOGFMT(LocateRequestHandler, "forwardRequest() failed to receive response data from forward url [%s] (EndRecv). error[%d:%s]"), forwardUrl_.c_str(), client.getErrorcode(), client.getErrorstr());
        return false;
    }

    resp->setStatus(client.getStatusCode());
    std::string result;
    client.getContent(result);
    std::string noticeMsg = client.getHeader()[HDR_TianShan_Notice];
    if(!noticeMsg.empty()) {
        resp->setHeader(HDR_TianShan_Notice, noticeMsg.c_str());
    }
    if(!result.empty()) {
        resp->setContent(result.data(), result.size());
        _log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(LocateRequestHandler, "forwardRequest() Get response:\n%s"), result.c_str());
    }
    return true;
}
std::string LocateRequestHandler::getContentStore(const std::string& netid) const {
    std::map<std::string, std::string>::const_iterator it;
    it = _env._conf.storageMap.find(netid);
    if(it != _env._conf.storageMap.end())
        return it->second;
    else
        return "";
}

}} // namespace ZQTianShan::CDN
