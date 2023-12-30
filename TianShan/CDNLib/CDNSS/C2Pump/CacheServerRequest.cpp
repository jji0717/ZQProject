#include <ZQ_common_conf.h>
#include <assert.h>
#include <sstream>
#include <strHelper.h>
#include <urlstr.h>
#include <CacheStoreImpl.h>
#undef max
#include "C2HttpHandler.h"
#include "C2StreamerEnv.h"
#include "C2StreamerService.h"
#include "C2SessionManager.h"
#include "C2Session.h"
#include <SimpleXMLParser.h>
#include <Text.h>
#include <TianShanIceHelper.h>
#include <CDNDefines.h>
#include <Guid.h>
#include <HttpClient.h>

#define SEP_PAID_PID    "_"
#define HDR_Require "Require" // com.schange.cdn.v1 for SeaChange spec
#define HDR_ForwardHop "x-ForwardHop"
#define HDR_TianShan_Notice "TianShan-Notice"

#define FANGLI_EVIL

extern ZQTianShan::ContentStore::CacheStoreImpl::Ptr cacheStore;

namespace C2Streamer{

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

/////////////////////////////
C2LocateCB::C2LocateCB(LibAsync::EventLoop* loop, HandlerC2Locate* req, const std::string connid)
:LibAsync::AsyncWork(*loop),
mLocateRequestPtr(req),
mbReqValid(true),
mbCbDone(false),
mHttpCode(500),
mConnId(connid),
mPostCount(0){
}

C2LocateCB::~C2LocateCB() {
	ZQ::common::MutexGuard gd(mLocker);
	if(mLocateRequestPtr)
	{
		mLocateRequestPtr = NULL;
	}
}

void C2LocateCB::setLocateRequestPtrToNull() {
	ZQ::common::MutexGuard gd(mLocker);
	mLocateRequestPtr = NULL;
}

void C2LocateCB::postResponse( int code, const std::string& message ) {
	ZQ::common::MutexGuard gd(mLocker);
	if(!mbReqValid) {
		//log message and return
		return;
	}
	if(!mLocateRequestPtr) {
		return;
	}
	/*
	if(mPostCount != 0)
		assert(false && "post more than once.");
	mPostCount ++;
	*/
	mHttpCode = code;
	mMessage = message;
	mbCbDone = true;
	queueWork();
}

void C2LocateCB::onAsyncWork() {
	ZQ::common::MutexGuard gd(mLocker);
	if(mLocateRequestPtr != NULL)
	{
		if(mPostCount != 0) {
	        assert(false && "post more than once.");
		}
		mPostCount ++;
		mLocateRequestPtr->postProcess(mHttpCode, mMessage,mConnId);
		mbCbDone = false;
		mLocateRequestPtr = NULL;
	}
}

	// class CacheServerRequest
CacheServerRequest::CacheServerRequest(C2StreamerEnv& env, C2Service& svc, C2LocateCB::Ptr locateCB, ConnInfo connInfo, const std::string& content, const ZQHttp::IRequest* req)
: ZQ::common::ThreadRequest(*env.mLocateThreadPool), mEnv(env), mSvc(svc), mLocateCb(locateCB), mConnInfo(connInfo), mContent(content), mRequest(req)
{
}

CacheServerRequest::~CacheServerRequest()
{
}

#ifdef FANGLI_EVIL

std::string fixupPAIDPID(std::string& PAID, std::string& PID, const char outSeperator = '_', const char* inSeperators = "_")
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

bool CacheServerRequest::resolveObject(const std::string& identifier, std::string& providerId, std::string& assetId, std::string& extension) const
{
	//bug id #19414  object id = [.]00NVOD1234567890abcdef_cctv$com
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
	return true;
}

bool CacheServerRequest::parseRequestData(std::string content, CacheLocatorRequestData& reqData)
{
	SimpleXMLParser parser;
	try
	{
		parser.parse(content.data(), content.size(), 1);
	}
	catch(const ZQ::common::ExpatException& e)
	{ // bad xml format
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequest, "[%s] parseRequestData() ExpatException: [%s - %s] during parsing request body"), mConnInfo.connID.c_str(), e.getString(), "230103 Bad XML format");
		return false;
	}

	// check the content and extract the post data
	const SimpleXMLParser::Node* locNode = findNode(&parser.document(), "LocateRequest");
	if(NULL == locNode)
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequest, "[%s] parseRequestData() 230104 XML element missed: <LocateRequest>"), mConnInfo.connID.c_str());
		return false;
	}

	const SimpleXMLParser::Node* objNode = findNode(locNode, "Object");
	if(!objNode)
	{ // parameter missed
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequest, "[%s] parseRequestData() 230104 XML element missed: <Object>"), mConnInfo.connID.c_str());
		return false;
	}

	if(!objNode->content.empty())
	{ // parse the object id and get the pid/paid
		if(!resolveObject(objNode->content, reqData.providerId, reqData.assetId, reqData.extension))
		{
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequest, "[%s] parseRequestData() 230105 Invalid parameter: <ObjectIdentifier>"), mConnInfo.connID.c_str());
			return false;
		}
	}
	else
	{
		const SimpleXMLParser::Node* assetIdNode = findNode(locNode, "Object/Name/AssetID");
		reqData.assetId = assetIdNode ? assetIdNode->content : "";
		if(reqData.assetId.empty())
		{ // parameter missed
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequest, "[%s] parseRequestData() 230104 XML element missed: <AssetID>"), mConnInfo.connID.c_str());
			return false;
		}

		const SimpleXMLParser::Node* providerIdNode = findNode(locNode, "Object/Name/ProviderID");
		reqData.providerId = providerIdNode ? providerIdNode->content : "";
		if(reqData.providerId.empty())
		{ // parameter missed
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequest, "[%s] parseRequestData() 230104 XML element missed: <ProviderID>"), mConnInfo.connID.c_str());
			return false;
		}

#ifdef FANGLI_EVIL
		fixupPAIDPID(reqData.assetId, reqData.providerId); // this was due to FangLi's evil idea in Hefei EDU2016-06
#endif // FANGLI_EVIL

		const SimpleXMLParser::Node* subTypeNode = findNode(locNode, "Object/SubType");
		reqData.subType = subTypeNode ? subTypeNode->content : "";
		if(reqData.subType.empty())
		{ // parameter missed
			MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequest, "[%s] parseRequestData() 230104 XML element missed: <SubType>"), mConnInfo.connID.c_str());
			return false;
		}

		// convert the informal subtype into extension mode
		if(reqData.subType != "index" && reqData.subType != "forward/1")
		{
			reqData.extension = reqData.subType;
		}

		// fixup the bad subtype from vsis
		if(!reqData.extension.empty()) { // not standard subtype
			// if( _env._conf.vsisFixup.illegalSubtypes.find(reqData.extension) != _env._conf.vsisFixup.illegalSubtypes.end() ) {
			// 	reqData.providerId += reqData.extension;
			// 	MLOG(ZQ::common::Log::L_INFO,CLOGFMT(CacheServerRequest,"Got illegal subtype [%s]. Treat the object as main file and fixup pid to [%s]"), 		reqData.extension.c_str(), reqData.providerId.c_str());

			// 	// update the subtype fields
			// 	reqData.extension.clear();
			// 	reqData.subType = CDN_SUBTYPE_NormalForward;
			// }
		}
	}

	const SimpleXMLParser::Node* clientTransferNode = findNode(locNode, "ClientTransfer");
	reqData.clientTransfer = clientTransferNode ? clientTransferNode->content : "";
	if(reqData.clientTransfer.empty()) {
		reqData.clientTransfer = mConnInfo.peerIP;
	}

	// Not reject the request when the TransferRate missed.
	const SimpleXMLParser::Node* transferRateNode = findNode(locNode, "TransferRate");
	reqData.transferRate = transferRateNode ? StringToLong(transferRateNode->content) : -1;

	const SimpleXMLParser::Node* ingressCapacityNode = findNode(locNode, "IngressCapacity");
	reqData.ingressCapacity = ingressCapacityNode ? StringToLong(ingressCapacityNode->content): -1;
	if(-1 == reqData.ingressCapacity)
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequest, "[%s] parseRequestData() 230104 XML element missed: <IngressCapacity>"), mConnInfo.connID.c_str());
		return false;
	}

	const SimpleXMLParser::Node* exclusionListNode = findNode(locNode, "ExclusionList");
	if(exclusionListNode && !exclusionListNode->content.empty())
	{
		ZQ::common::Text::split(reqData.exclusionList, exclusionListNode->content, ", ");
#pragma message(__MSGLOC__"should we verify the format of the address here?")
	}

	const SimpleXMLParser::Node* rangeNode = findNode(locNode, "Range");
	reqData.range = rangeNode ? rangeNode->content : "";
	// nonstandard range format, support ',' and '-'
	char rangeDelimiter = reqData.range.find(',') != std::string::npos ? ',' : '-';
	if(rangeDelimiter != '-') {
		std::replace(reqData.range.begin(), reqData.range.end(), rangeDelimiter, '-');
	}

	// transfer delay
	const SimpleXMLParser::Node* transferDelayNode = findNode(locNode, "TransferDelay");
	reqData.transferDelay = 0;
	if(transferDelayNode && !transferDelayNode->content.empty())
	{
		reqData.transferDelay = StringToLong(transferDelayNode->content);
	}
	return true;
}

bool CacheServerRequest::getStreamProperty(::TianShanIce::Streamer::StreamPrx stream, ::TianShanIce::Properties& props)
{
	try
	{
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(CacheServerRequest, "[%s] get stream Properties"), mConnInfo.connID.c_str());
		props = stream->getProperties();
	}
	catch (const TianShanIce::BaseException& ex)
	{
		char mesbuf[1024]={0};
		sprintf(mesbuf, "caught %s  errorcode[%d] errmessage[%s]",ex.ice_name().c_str(),ex.errorCode,ex.message.c_str());
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequest,"[%s] %s"), mConnInfo.connID.c_str(), mesbuf);
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		char mesbuf[512]={0};
		sprintf(mesbuf, "caught %s ,(%s)",ex.ice_name().c_str(),ex.what());
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequest, "[%s] %s"), mConnInfo.connID.c_str(), mesbuf );
		return false;
	}
	catch (...)
	{
		char mesbuf[512]={0};
		sprintf(mesbuf, "caught unkonwn exception by call invoke getProperties  .");
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequest, "[%s] %s"), mConnInfo.connID.c_str(), mesbuf);
		return false;
	}
	return true;
}

std::string CacheServerRequest::generateResponseXML(const CacheLocatorResponseData& respData)
{
	std::ostringstream buf;
	buf << "<LocateResponse>\n"
	<< "  <TransferPort>" << respData.transferPort << "</TransferPort>\n"
	<< "  <TransferID>" << respData.transferID << "</TransferID>\n"
	<< "  <TransferTimeout>" <<respData.transferTimeout << "</TransferTimeout>\n"
	<< "  <AvailableRange>" << respData.availableRange << "</AvailableRange>\n"
	<< "  <OpenForWrite>" << respData.openForWrite << "</OpenForWrite>\n"
	<< "  <PortNum>" << respData.portNumber << "</PortNum>\n";

	if( /*_env._conf.exposeAssetIndexData > 0*/ false) {
		if(reqData.subType == CDN_SUBTYPE_Index || reqData.subType == CDN_SUBTYPE_NormalForward) {
			buf << "<ClipInfo>"<<std::endl;
			buf << "<Residential recording=\""<< (respData.openForWrite == "yes" ? 1 : 0) <<"\" />" << std::endl;
			buf << respData.idxContentGeneric << std::endl;
			if( reqData.subType == CDN_SUBTYPE_Index ) {
				buf << respData.idxContentSubfiles << std::endl;
			}
			buf << "</ClipInfo>"<<std::endl;
		}
	}
	buf << "</LocateResponse>";
	return buf.str();
}

bool CacheServerRequest::sendResponseData(const ::TianShanIce::Properties& props)
{
	try
	{
		ZQTianShan::Util::getPropertyData(props,CDN_TRANSFERPORT,respData.transferPort);     //cdn.transferPort
		ZQTianShan::Util::getPropertyData(props,CDN_TRANSFERID,respData.transferID);         //cdn.transferId
		ZQTianShan::Util::getPropertyData(props,CDN_TRANSFERTIMEOUT,respData.transferTimeout);//cdn.transferTimeout
		ZQTianShan::Util::getPropertyData(props,CDN_AVAILRANGE,respData.availableRange);		//cdn.availableRange
		ZQTianShan::Util::getPropertyData(props,CDN_OPENFORWRITE,respData.openForWrite);		//cdn.openForWrite
		ZQTianShan::Util::getPropertyData(props,CDN_TRANSFERPORTNUM,respData.portNumber);			//cdn.transferPortNum
		ZQTianShan::Util::getPropertyDataWithDefault(props,CDN_IDXCONTENT_GENERIC,"",respData.idxContentGeneric);
		ZQTianShan::Util::getPropertyDataWithDefault(props,CDN_IDXCONTENT_SUBFILES,"",respData.idxContentSubfiles);
	}
	catch (const TianShanIce::InvalidParameter& ex)
	{
		char mesbuf[512]={0};
		sprintf(mesbuf, "getPropertyData() invalidParameter exception is caught:%s",ex.message.c_str());
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequest, "[%s] %s"), mConnInfo.connID.c_str(), mesbuf);
		mLocateCb->postResponse(500, "Internal  CacheServer Error");
		return false;
	}
	catch (...)
	{
		char mesbuf[512]={0};
		sprintf(mesbuf, "caught unkonwn exception via  getPropertyData()  .");
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequest, "[%s] %s"), mConnInfo.connID.c_str(), mesbuf );
		mLocateCb->postResponse(500, "Internal  CacheServer Error");
		return false;
	}
	
	mForwardUrl = mEnv.getConfig().forwardURL;

	if(respData.openForWrite == "yes" && !mForwardUrl.empty())
	{	
		std::string strResponse = "";	
		bool res = locateForward(strResponse);
		//mRespData = generateResponseXML(strResponse);
		mLocateCb->postResponse(mForwardStatus, strResponse);
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheServerRequest, "Get forward response [%s]"), strResponse.c_str());
		return true;
	}

	mRespData = generateResponseXML(respData);
	mLocateCb->postResponse(201, mRespData);
	return true;
}
    
bool CacheServerRequest::locateForward( std::string& sResponse )
{
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheServerRequest, "locateForward() entry."));
	//1, prepare data
	SimpleXMLParser parser;
	try
	{
		parser.parse(mContent.data(), mContent.size(), 1);
	}catch(const ZQ::common::ExpatException& e)
	{ // bad xml format
		MLOG(ZQ::common::Log::L_WARNING, CLOGFMT(CacheServerRequest, "locateForward() ExpatException: [%s] during parsing request body"), e.getString());
		//ERROR_RESPONSE(resp, 400, "Bad Request", "230103 Bad XML format");
		mForwardStatus = 400;
		return false;
	}
	const char* requireString = mRequest->header(HDR_Require);
	std::map<std::string, std::string> forwardHeaders;
	// check the x-ForwardHop
	
	int nHop = 0;

	std::string nextForwardHop = getNextForwardHop(mRequest->header(HDR_ForwardHop), 0, nHop);
	if(!nextForwardHop.empty()) {
		// include this header
		forwardHeaders[HDR_ForwardHop] = nextForwardHop;
	}
	
	if(NULL != requireString) {
		forwardHeaders[HDR_Require] = requireString;
	}
	
	std::string forwardDoc;
	SimpleXMLParser::Node origClient;
	origClient.name = "ClientTransfer";
	origClient.content = reqData.clientTransfer;
	buildXMLDocument(forwardDoc, parser.document(), "/LocateRequest/ClientTransfer", origClient);
			
	//2, start to send forward request
	// build the request content
	ZQ::common::HttpClient client(mEnv.getLogger());
	// set parameters
	client.init();
	if(0 != client.httpConnect(mForwardUrl.c_str(), ZQ::common::HttpClient::HTTP_POST)) {
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheServerRequest, "locateForward() failed to connect the forward url [%s]. error[%d:%s]"), mForwardUrl.c_str(), client.getErrorcode(), client.getErrorstr());
		mForwardStatus = 500;
		return false;
	}
	for(std::map<std::string, std::string>::const_iterator it = forwardHeaders.begin(); it != forwardHeaders.end(); ++it) {
		client.setHeader((char*)(it->first.c_str()), (char*)(it->second.c_str()));
	}

	if(0 != client.httpSendContent(forwardDoc.data(), forwardDoc.size())) {
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheServerRequest, "locateForward() failed to send data to the forward url [%s]. data size[%d], error[%d:%s]"), mForwardUrl.c_str(), forwardDoc.size(), client.getErrorcode(), client.getErrorstr());
		mForwardStatus = 500;
		return false;
	}
	if(0 != client.httpEndSend()) {
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheServerRequest, "locateForward() failed to send data to the forward url [%s] (EndSend). error[%d:%s]"), mForwardUrl.c_str(), client.getErrorcode(), client.getErrorstr());
		mForwardStatus = 500;
		return false;
	}
	MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(CacheServerRequest, "locateForward() http client end send request"));
	if(0 != client.httpBeginRecv()) {
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheServerRequest, "locateForward() failed to receive response data from forward url [%s] (BeginRecv). error[%d:%s]"), mForwardUrl.c_str(), client.getErrorcode(), client.getErrorstr());
		mForwardStatus = 500;
		return false;
	}
    MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(CacheServerRequest, "locateForward() http client end recv request"));					
	while(!client.isEOF()) {
		if(0 != client.httpContinueRecv()) {
			MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheServerRequest, "locateForward() failed to receive response data from forward url [%s] (ContinueRecv). error[%d:%s]"), mForwardUrl.c_str(), client.getErrorcode(), client.getErrorstr());
			mForwardStatus = 500;
			return false;
		}
	}
	if(0 != client.httpEndRecv()) {
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheServerRequest, "locateForward() failed to receive response data from forward url [%s] (EndRecv). error[%d:%s]"), mForwardUrl.c_str(), client.getErrorcode(), client.getErrorstr());
		mForwardStatus = 500;
		return false;
	}

	//resp->setStatus(client.getStatusCode());
	mForwardStatus = client.getStatusCode();
	//std::string result;
	client.getContent(sResponse);
	/*std::string noticeMsg = client.getHeader()[HDR_TianShan_Notice];
	if(!noticeMsg.empty()) {
		resp->setHeader(HDR_TianShan_Notice, noticeMsg.c_str());
	}
	
	if(!mRespData.empty()) {
		//resp->setContent(result.data(), result.size());
		MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheServerRequest, "locateForward() Get response:\n%s"), result.c_str());
	}
	*/
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheServerRequest, "locateForward() request forward successful"));
	mForwardStatus = 201;
	return true;
}

int CacheServerRequest::run()
{
    int64 t1 = ZQ::common::now(); // record the request cost
    std::string reqId = mConnInfo.connID;
	std::string reqDumpHint = "[req-" + reqId + "] CacheServerRequest:";
	MLOG.hexDump(ZQ::common::Log::L_DEBUG, mContent.c_str(), mContent.size(), reqDumpHint.c_str(), true); // text-only dump

	if (!parseRequestData(mContent, reqData))
	{
		// parse data failed
		mLocateCb->postResponse(400, "Bad Request");
		return 0;
	}

	::TianShanIce::SRM::ResourceMap resMapObj;

	//1 subtype  [Will be passed via the parameter subfile of the API
	//2 URL      [The original URI in the client’s request.Passed thru the parameter resourceRequirement[rtURI].resourceData[“uri”]]
	ZQTianShan::Util::updateResourceData(resMapObj,TianShanIce::SRM::rtURI,"uri", mConnInfo.uri);

	//3 Client TransferAddress  ----Will be passed via the parameter resourceRequirement[rtEthernetInterface].resourceData[destIP]
	ZQTianShan::Util::updateResourceData(resMapObj,TianShanIce::SRM::rtEthernetInterface,"destIP", mConnInfo.peerIP);
	//4 Server TransferAddress  ----Optional in CacheServer, will be passed via parameter resourceRequirement[rtEthernetInterface].resourceData[srcIP]

	//5 TransferRate  --Will be passed via parameter resourceRequirement[rtTsDownstreamBandwidth].resourceData[bandwidth][0]
	//6 Client IngressCapacity ---Optional. If this is applicable, the parameter resourceRequirement[rtTsDownstreamBandwidth] will be a range. Besides the minimal value resourceData[bandwidth][0] indicates the requested TransferRate, the maximal value resourceData[bandwidth][1] indicates the client’s IngressCapacity
	std::vector<Ice::Long> vectric;vectric.push_back(reqData.transferRate);vectric.push_back(reqData.ingressCapacity);
	ZQTianShan::Util::updateResourceData(resMapObj,TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth",vectric,true);

	//7 Range  ---Optional. If this is applicable, the parameter params[“Range”] passes the value
	//8 CDN Type ---Optional, can potentially be recognized from the URI. When it is applicable, it is passed thru params[“CDNType”]
	//9 TransferDelay ---Optional. When it is applicable, it is passed thru params[“TansferDeplay”] in milliseconds
	ZQTianShan::Util::updateResourceData(resMapObj,TianShanIce::SRM::rtTsUpstreamBandwidth,"bandwidth", vectric, true);
	//::TianShanIce::StrValues sessionInterface;
	//sessionInterface.push_back("$");
	//ZQTianShan::Util::updateResourceData(resMapObj,TianShanIce::SRM::rtTsUpstreamBandwidth,"sessionInterface", sessionInterface, false);

	Ice::Int idleStreamTimeoutobj=500000;
	Ice::Int cacheStoreDepthobj=0;
	TianShanIce::Properties propertiesobj;

	ZQTianShan::Util::updatePropertyData(propertiesobj,"Range",reqData.range.empty()? "0-" : reqData.range);
	ZQTianShan::Util::updatePropertyData(propertiesobj,CDN_RANGE,reqData.range.empty()? "0-" : reqData.range);
	ZQTianShan::Util::updatePropertyData(propertiesobj,"CDNType", "");
	ZQTianShan::Util::updatePropertyData(propertiesobj,"TansferDeplay", "0");
	if( /*_env._conf.exposeAssetIndexData > 0*/ true ) {
		//FIXME: add a configuration here to enable exposeAssetIndexData
		// set exposeIndex to false temporarily
		ZQTianShan::Util::updatePropertyData(propertiesobj,SYS_PROP("exposeIndex"), "0");
	}

	char msgbuf[1024] ={0};
	snprintf(msgbuf, sizeof(msgbuf)-2, "%s", reqData.clientTransfer.c_str());
	ZQTianShan::Util::updatePropertyData(propertiesobj, SYS_PROP("client"), msgbuf);
	snprintf(msgbuf, sizeof(msgbuf)-2, "%p", &reqData);
	ZQTianShan::Util::updatePropertyData(propertiesobj, SYS_PROP("clientSession"), msgbuf);

	// std::string contentname = reqData.assetId + reqData.providerId;//exportContentAsStream  cachestore module
	std::string contentname = reqData.assetId + SEP_PAID_PID + reqData.providerId; //exportContentAsStream  cachestore module
	int64 itexportCAS = ZQ::common::now();
	::TianShanIce::Streamer::StreamPrx stream = NULL;
	try {
		::Ice::Current ic;
		ic.ctx["hintSessionID"] =  mConnInfo.connID;
		stream = cacheStore->exportContentAsStream(contentname, reqData.subType,
				idleStreamTimeoutobj, cacheStoreDepthobj, resMapObj, propertiesobj, ic);
		//stream->play();
	} catch( const TianShanIce::InvalidParameter& ex ) {
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequest,"[%s] failed to exportContentAsStream for content [%s] due to [%s]"),
				mConnInfo.connID.c_str(), contentname.c_str(), ex.ice_name().c_str() );
		mLocateCb->postResponse(404, "Not Found");
		return 0;
	} catch( const TianShanIce::BaseException& ex ) {
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequest,"[%s] failed to exportContentAsStream for content [%s] due to [%s][%d]"),
				mConnInfo.connID.c_str(), contentname.c_str(), ex.ice_name().c_str(), (int)ex.errorCode );
		if(ex.errorCode == 404) {
			mLocateCb->postResponse(404, "Not Found");
		} else {
			mLocateCb->postResponse(500, "Internal  CacheServer Error");
		}
		return 0;
	} catch( const std::exception& ex ) {
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequest,"[%s] failed to exportContentAsStream for content [%s] due to [%s]"),
				mConnInfo.connID.c_str(), contentname.c_str(), ex.what());
		mLocateCb->postResponse(500, "Internal  CacheServer Error");
		return 0;
	}

	if (NULL == stream)
	{
		MLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequest, "[%s] failed to exportContentAsStream for content [%s]"), mConnInfo.connID.c_str(), contentname.c_str());
		mLocateCb->postResponse(500, "Internal  CacheServer Error");
		return 0;
	}else{
		MLOG(ZQ::common::Log::L_INFO, CLOGFMT(CacheServerRequest, "[%s] exportContentAsStream for content [%s] successfully took %dms"), mConnInfo.connID.c_str(),
			contentname.c_str(),  (int)(ZQ::common::now() - itexportCAS));
	}

	//get the properties of this stream
	::TianShanIce::Properties propertiess;
	if (!getStreamProperty(stream, propertiess))
	{
		// failed to get stream properties
		mLocateCb->postResponse(500, "Internal  CacheServer Error");
		return 0;
	}

	std::string sPropertyDataContent;
	::TianShanIce::Properties::iterator itor;
	for(itor = propertiess.begin(); itor != propertiess.end(); itor++)
		sPropertyDataContent+=itor->first + "=" + itor->second+ " ;";
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheServerRequest, "[%s] property-content:%s"), mConnInfo.connID.c_str(), sPropertyDataContent.c_str());
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheServerRequest, "[%s] prepare response message"), mConnInfo.connID.c_str());

    if(!sendResponseData(propertiess))
    {
        return 0;
    }

	MLOG(ZQ::common::Log::L_INFO, CLOGFMT(CacheServerRequest, "[%s] client[%s:%d],AssetID[%s],response content:%s"), mConnInfo.connID.c_str(), mConnInfo.peerIP.c_str(), mConnInfo.peerPort, reqData.assetId.c_str(), mRespData.c_str());

	std::string respDumpHint = "[req-" + reqId + "] CahceServerLocateResponse:";
	MLOG.hexDump(ZQ::common::Log::L_DEBUG, mRespData.c_str(), mRespData.size(), respDumpHint.c_str(), true); // text-only dump

	MLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(CacheServerRequest, "[%s] TransferCreated: cost(%d) AssetId(%s)ProviderId(%s)SubType(%s)TransferPort(%s)TransferId(%s)RequestedFrom(%s:%d)"), mConnInfo.connID.c_str(), (int)(ZQ::common::now() - t1), reqData.assetId.c_str(), reqData.providerId.c_str(), (reqData.subType.empty() ? reqData.extension.c_str() : reqData.subType.c_str()), respData.transferPort.c_str(), respData.transferID.c_str(), mConnInfo.peerIP.c_str(), mConnInfo.peerPort);

	return 1;

}
}//namespace C2Streamer
