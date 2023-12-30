#include "CacheServerRequestHandle.h"
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
#include  "../ContentStore/CacheStore.h"

#define HDR_Require "Require" // com.schange.cdn.v1 for SeaChange spec
#define HDR_ForwardHop "x-ForwardHop"
#define HDR_TianShan_Notice "TianShan-Notice"
#define ERROR_RESPONSE(RESP, CODE, REASON, NOTICE) RESP->setStatus(CODE, REASON);\
    if(_env._conf.tianshanNoticeEnabled) {RESP->setHeader(HDR_TianShan_Notice, NOTICE);}

#define FANGLI_EVIL

extern struct GlobalResource
{
	ZQ::common::Config::Loader<ZQTianShan::CDN::C2LocatorConf> conf;
	ZQ::common::FileLog* pLog;
	ZQ::common::FileLog* pIceLog;
	ZQ::common::NativeThreadPool* pThreadPool;
	Ice::CommunicatorPtr communicator;
	ZQTianShan::CDN::C2Env* pEnv;
	IceInternal::Handle<TianShanIce::SCS::C2LocatorImpl> pLocator;
//	ZQ::Snmp::Subagent* pAgent;
//	PortSnmpManager portSnmpMgr;

	GlobalResource();
	void clear();
} gResource;

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

namespace ZQTianShan
{
	namespace CDN
	{
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

		CacheServerRequestHandle::CacheServerRequestHandle(C2Env& env, TianShanIce::SCS::C2LocatorImpl& locator)
			:_log(*env._pLog), _env(env), _locator(locator),_clientMgr(locator.getClientManager())
		{
			_bCQuit=true;
		}

		CacheServerRequestHandle::~CacheServerRequestHandle()
		{}
		// define 
		bool CacheServerRequestHandle::stop()
		{
			_bCQuit= true;
			_semaphore.post();//no wait
			return true;
		}
		int CacheServerRequestHandle::run()
		{
			while (!_bCQuit) 
			{
				CacheSessionMap expiredCacheSessMap;
				long nextSleep = 10*60*1000; // 10min
				int64 stampNow = ZQ::common::now();

				{
					int64 stampExpire = stampNow - _env._conf.transferSessionTimeOut;//sessionTimeout;

					ZQ::common::MutexGuard g(_lkCacheSessMap);
					for (CacheSessionMap::iterator it = _cacheSessMap.begin(); !_bCQuit && it != _cacheSessMap.end(); it++)
					{
						long timeleft = (long) (it->second.stampLastTouch - stampExpire);
						if (timeleft <0)
						{
							MAPSET(CacheSessionMap, expiredCacheSessMap, it->first, it->second);
							it->second.stampLastTouch = stampNow; // temporary reset the stampLastTouch
							continue;
						}
						//#define min(a,b)            (((a) < (b)) ? (a) : (b))
						nextSleep = timeleft < nextSleep ? timeleft : nextSleep;//min(timeleft, nextSleep);
					}
				}

				if (_bCQuit)
					break;

				TianShanIce::StrValues tranferIdToDelete, transferIdTouched;
				stampNow = ZQ::common::now();
				int64 stampExpireDirect = stampNow - _env._conf.transferSessionTimeOut*3; //conf.sessionTimeout*3;

				for (CacheSessionMap::iterator itE = expiredCacheSessMap.begin(); !_bCQuit && itE != expiredCacheSessMap.end(); itE++)
				{
					if (itE->second.stampLastTouch < stampExpireDirect)
					{
						tranferIdToDelete.push_back(itE->first);
						continue;
					}

					try {
						itE->second.stream->ice_ping() ;//->ping();
						transferIdTouched.push_back(itE->first);//
					}
					catch(::Ice::ObjectNotExistException& ex)
					{
						tranferIdToDelete.push_back(itE->first);
						_log(ZQ::common::Log::L_WARNING, "CacheServerRequestHandle[%s] ice_ping cdnss session caught %s", itE->second.transferId.c_str(), ex.ice_name().c_str());
					}
					catch(Ice::Exception& ex)
					{
						_log(ZQ::common::Log::L_WARNING, "CacheServerRequestHandle[%s] ice_ping cdnss session caught %s", itE->second.transferId.c_str(), ex.ice_name().c_str());
					}

				}
				if (_bCQuit)
					break;

				/////////////////////////////////////
				{
					ZQ::common::MutexGuard g(_lkCacheSessMap);
					for (TianShanIce::StrValues::iterator itD = tranferIdToDelete.begin(); !_bCQuit && itD < tranferIdToDelete.end(); itD++)
						_cacheSessMap.erase(*itD);			//_cacheSessMap.erase(*it);

					for (TianShanIce::StrValues::iterator itT = transferIdTouched.begin(); !_bCQuit && itT < tranferIdToDelete.end(); itT++)
					{
						CacheSessionMap::iterator it = _cacheSessMap.find(*itT);
						it->second.stampLastTouch = stampNow;
					}
				}

				if (_bCQuit)
					break;

				nextSleep -= ZQ::common::now() - stampNow;

				if (nextSleep < 1000)
					nextSleep = 1000;
				_semaphore.timedWait(nextSleep*1000);//				//::Sleep(nextSleep); // TODO should be a WaitForObject()

			} // while()
			return 0;
		}

		bool CacheServerRequestHandle::resolveObject(const std::string& identifier, std::string& providerId, std::string& assetId, std::string& extension) const
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
						_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheServerRequestHandle, "resolveObject() resolved object identifier [%s] into: pid [%s], paid [%s], ext [%s]"), identifier.c_str(), providerId.c_str(), assetId.c_str(), extension.c_str());
						return true;
					}catch(const std::exception& e)
					{
						_log(ZQ::common::Log::L_ERROR, CLOGFMT(CacheServerRequestHandle, "resolveObject() resolution [%s] matched but get exception:[%s] during formatting the attributes"), it->type.c_str(), e.what());
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
			//_log(ZQ::common::Log::L_WARNING, CLOGFMT(CacheServerRequestHandle, "resolveObject() failed to resolve object identifier [%s]"), identifier.c_str());
			return true;
		}
		typedef SimpleXMLParser::Node Node;

#define HDR_TianShan_Notice "TianShan-Notice"
#define CACHEDELETER_ERROR_RESPONSE(RESP, CODE, REASON, NOTICE) RESP->setStatus(CODE, REASON);\
	if(_env._conf.tianshanNoticeEnabled) {RESP->setHeader(HDR_TianShan_Notice, NOTICE);}\
	_log(ZQ::common::Log::L_INFO, ReqLOGFMT(CacheDeleteRequestHandle, "errorResponse() cost(%d)  RequestedFrom(%s:%d)StatusCode(%d)ReasonPhrase(%s)Notice(%s)"), (int)(ZQ::common::now() - t1), reqFrom.c_str(), reqFromPort, CODE, REASON, NOTICE)

#define CACHEREQUESTR_ERROR_RESPONSE(RESP, CODE, REASON, NOTICE) RESP->setStatus(CODE, REASON);\
	if(_env._conf.tianshanNoticeEnabled) {RESP->setHeader(HDR_TianShan_Notice, NOTICE);}\
	_log(ZQ::common::Log::L_INFO, ReqLOGFMT(CacheServerRequestHandle, "errorResponse() cost(%d) AssetId(%s)ProviderId(%s)SubType(%s)DemandedBy(%s)RequestedFrom(%s:%d)StatusCode(%d)ReasonPhrase(%s)Notice(%s)"), (int)(ZQ::common::now() - t1), reqData.assetId.c_str(), reqData.providerId.c_str(), (reqData.subType.empty() ? reqData.extension.c_str() : reqData.subType.c_str()), reqData.clientTransfer.c_str(), reqFrom.c_str(), reqFromPort, CODE, REASON, NOTICE)

		void CacheServerRequestHandle::addSession(ZQTianShan::CDN::CacheServerSession& sess)
		{
			if (sess.transferId.empty())
				return;

			sess.stampCreated = sess.stampLastTouch = ZQ::common::now();
			ZQ::common::MutexGuard gl(_lkCacheSessMap);//gl(_env._lkCacheSessMap);
			MAPSET(CacheSessionMap, _cacheSessMap, sess.transferId, sess);//_env._cacheSessMap,
		}
		void CacheServerRequestHandle::deleteSession(const std::string& transferId)
		{
			if (transferId.empty())
				return;

			ZQ::common::MutexGuard gl(_lkCacheSessMap);//gl(_env._lkCacheSessMap);
			CacheSessionMap::iterator it = _cacheSessMap.find(transferId);
			if (_cacheSessMap.end() == it)
				return;

			try {
				if (it->second.stream)
					it->second.stream->destroy();
			}
			catch(...)
			{
				//TODO
			}
			_cacheSessMap.erase(it);;//_env._cacheSessMap.erase(it); 
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

		bool CacheServerRequestHandle::cache_forwardRequest(const CRG::IRequest* req, CRG::IResponse* resp, const std::string& reqId, SimpleXMLParser& parser, CacheServerLocatorRequestData reqData)
		{	
			_log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(CacheServerRequestHandle, "cache_forwardRequest() entry."));
			//1, prepare data
			/*std::string reqContent;
			req->getContent(reqContent);
			SimpleXMLParser parser;
			try
			{
				parser.parse(reqContent.data(), reqContent.size(), 1);
			}catch(const ZQ::common::ExpatException& e)
			{ // bad xml format
				_log(ZQ::common::Log::L_WARNING, ReqLOGFMT(CacheServerRequestHandle, "cache_forwardRequest() ExpatException: [%s] during parsing request body"), e.getString());
				ERROR_RESPONSE(resp, 400, "Bad Request", "230103 Bad XML format");
				return false;
			}
			*/
			
			const char* requireString = req->header(HDR_Require);
			
			std::map<std::string, std::string> forwardHeaders;
			// check the x-ForwardHop
			int nHop = 0;
			std::string nextForwardHop = getNextForwardHop(req->header(HDR_ForwardHop), _env._conf.maxHop, nHop);
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
			ZQ::common::HttpClient client(&_log);
			// set parameters
			client.init();
			if(0 != client.httpConnect(_forwardUrl.c_str(), ZQ::common::HttpClient::HTTP_POST)) {
				_log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(CacheServerRequestHandle, "cache_forwardRequest() failed to connect the forward url [%s]. error[%d:%s]"), _forwardUrl.c_str(), client.getErrorcode(), client.getErrorstr());
				return false;
			}
			for(std::map<std::string, std::string>::const_iterator it = forwardHeaders.begin(); it != forwardHeaders.end(); ++it) {
				client.setHeader((char*)(it->first.c_str()), (char*)(it->second.c_str()));
			}

			if(0 != client.httpSendContent(forwardDoc.data(), forwardDoc.size())) {
				_log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(CacheServerRequestHandle, "cache_forwardRequest() failed to send data to the forward url [%s]. data size[%d], error[%d:%s]"), _forwardUrl.c_str(), forwardDoc.size(), client.getErrorcode(), client.getErrorstr());
				return false;
			}
			if(0 != client.httpEndSend()) {
				_log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(CacheServerRequestHandle, "cache_forwardRequest() failed to send data to the forward url [%s] (EndSend). error[%d:%s]"), _forwardUrl.c_str(), client.getErrorcode(), client.getErrorstr());
				return false;
			}
			_log(ZQ::common::Log::L_DEBUG,ReqLOGFMT(CacheServerRequestHandle, "cache_forwardRequest() http client end send request"));
			if(0 != client.httpBeginRecv()) {
				_log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(CacheServerRequestHandle, "cache_forwardRequest() failed to receive response data from forward url [%s] (BeginRecv). error[%d:%s]"), _forwardUrl.c_str(), client.getErrorcode(), client.getErrorstr());
			}
            _log(ZQ::common::Log::L_DEBUG,ReqLOGFMT(CacheServerRequestHandle, "cache_forwardRequest() http client end recv request"));					
			while(!client.isEOF()) {
				if(0 != client.httpContinueRecv()) {
					_log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(CacheServerRequestHandle, "cache_forwardRequest() failed to receive response data from forward url [%s] (ContinueRecv). error[%d:%s]"), _forwardUrl.c_str(), client.getErrorcode(), client.getErrorstr());
					return false;
				}
			}
			if(0 != client.httpEndRecv()) {
				_log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(CacheServerRequestHandle, "cache_forwardRequest() failed to receive response data from forward url [%s] (EndRecv). error[%d:%s]"), _forwardUrl.c_str(), client.getErrorcode(), client.getErrorstr());
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
				_log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(CacheServerRequestHandle, "cache_forwardRequest() Get response:\n%s"), result.c_str());
			}
			_log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(CacheServerRequestHandle, "cache_forwardRequest() request forward successful"));
			return true;
		}

		void CacheServerRequestHandle::cache_locatorequest(const CRG::IRequest* prequest, CRG::IResponse* presponse)
		{
			//step 1 :  parse xml 
			int64 t1 = ZQ::common::now(); // record the request cost
			std::string reqFrom;
			int reqFromPort;
			prequest->getClientEndpoint(reqFrom, reqFromPort);
			std::string strFullUrl=prequest->uri();
			std::string reqId = _env.reqIdGen.create();
			_log(ZQ::common::Log::L_INFO, ReqLOGFMT(CacheServerRequestHandle, "LocateRequest from %s:%d"), reqFrom.c_str(), reqFromPort);

			typedef SimpleXMLParser::Node Node;
			std::string reqContent;
			prequest->getContent(reqContent);
			std::string reqDumpHint = "[req-" + reqId + "] CahceServerLocateRequest:";
			_log.hexDump(ZQ::common::Log::L_DEBUG, reqContent.c_str(), reqContent.size(), reqDumpHint.c_str(), true); // text-only dump

			CacheServerLocatorRequestData reqData;
			SimpleXMLParser parser;
			try
			{
				parser.parse(reqContent.data(), reqContent.size(), 1);
			}
			catch(const ZQ::common::ExpatException& e)
			{ // bad xml format
				_log(ZQ::common::Log::L_WARNING, ReqLOGFMT(CacheServerRequestHandle, "ExpatException: [%s] during parsing request body"), e.getString());
				CACHEREQUESTR_ERROR_RESPONSE(presponse, 400, "Bad Request", "230103 Bad XML format");
				return;
			}

			if(!_env._auth.auth(&parser.document(),reqId)) {
				CACHEREQUESTR_ERROR_RESPONSE(presponse, 401, "Unauthorized", "Unauthorized");
				return;
			}

			// check the content and extract the post data
			const Node* locNode = findNode(&parser.document(), "LocateRequest");
			if(NULL == locNode)
			{ // bad xml content
				CACHEREQUESTR_ERROR_RESPONSE(presponse, 400, "Bad Request", "230104 XML element missed: <LocateRequest>");
				return;
			}

			const Node* objNode = findNode(locNode, "Object");
			if(!objNode)
			{ // parameter missed
				CACHEREQUESTR_ERROR_RESPONSE(presponse, 400, "Bad Request", "230104 XML element missed: <Object>");
				return;
			}

			if(!objNode->content.empty())
			{ // parse the object id and get the pid/paid
				if(!resolveObject(objNode->content, reqData.providerId, reqData.assetId, reqData.extension))
				{
					CACHEREQUESTR_ERROR_RESPONSE(presponse, 400, "Bad Request", "230105 Invalid parameter: <ObjectIdentifier>");
					return;
				}
			}
			else
			{
				const Node* assetIdNode = findNode(locNode, "Object/Name/AssetID");
				reqData.assetId = assetIdNode ? assetIdNode->content : "";
				if(reqData.assetId.empty())
				{ // parameter missed
					CACHEREQUESTR_ERROR_RESPONSE(presponse, 400, "Bad Request", "230104 XML element missed: <AssetID>");
					return;
				}

				const Node* providerIdNode = findNode(locNode, "Object/Name/ProviderID");
				reqData.providerId = providerIdNode ? providerIdNode->content : "";
				if(reqData.providerId.empty())
				{ // parameter missed
					CACHEREQUESTR_ERROR_RESPONSE(presponse, 400, "Bad Request", "230104 XML element missed: <ProviderID>");
					return;
				}

				const Node* subTypeNode = findNode(locNode, "Object/SubType");
				reqData.subType = subTypeNode ? subTypeNode->content : "";
				if(reqData.subType.empty())
				{ // parameter missed
					CACHEREQUESTR_ERROR_RESPONSE(presponse, 400, "Bad Request", "230104 XML element missed: <SubType>");
					return;
				}

				// convert the informal subtype into extension mode
				if(reqData.subType != CDN_SUBTYPE_Index && reqData.subType != CDN_SUBTYPE_NormalForward)
				{
					reqData.extension = reqData.subType;
					//TODO
					//reqData.subType.clear();
				}

				// fixup the bad subtype from vsis
				if(!reqData.extension.empty()) { // not standard subtype
					if( _env._conf.vsisFixup.illegalSubtypes.find(reqData.extension) != _env._conf.vsisFixup.illegalSubtypes.end() ) {
						reqData.providerId += reqData.extension;
						_log(ZQ::common::Log::L_INFO,ReqLOGFMT(CacheServerRequestHandle,"Got illegal subtype [%s]. Treat the object as main file and fixup pid to [%s]"), reqData.extension.c_str(), reqData.providerId.c_str());

						// update the subtype fields
						reqData.extension.clear();
						reqData.subType = CDN_SUBTYPE_NormalForward;
					}
				}

#ifdef FANGLI_EVIL
				fixupPAIDPID(reqData.assetId, reqData.providerId); // this was due to FangLi's evil idea in Hefei EDU2016-06
#endif // FANGLI_EVIL
			}

			const Node* clientTransferNode = findNode(locNode, "ClientTransfer");
			reqData.clientTransfer = clientTransferNode ? clientTransferNode->content : "";
			if(reqData.clientTransfer.empty()) {
				reqData.clientTransfer = reqFrom;
			}

			// Not reject the request when the TransferRate missed.
			const Node* transferRateNode = findNode(locNode, "TransferRate");
			reqData.transferRate = transferRateNode ? StringToLong(transferRateNode->content) : -1;

			const Node* ingressCapacityNode = findNode(locNode, "IngressCapacity");
			reqData.ingressCapacity = ingressCapacityNode ? StringToLong(ingressCapacityNode->content): -1;
			if(-1 == reqData.ingressCapacity)
			{
				// parameter missed
				CACHEREQUESTR_ERROR_RESPONSE(presponse, 400, "Bad Request", "230104 XML element missed: <IngressCapacity>");
				return;
			}

			const Node* exclusionListNode = findNode(locNode, "ExclusionList");
			if(exclusionListNode && !exclusionListNode->content.empty())
			{
				ZQ::common::Text::split(reqData.exclusionList, exclusionListNode->content, ", ");

#pragma message(__MSGLOC__"should we verify the format of the address here?")
			}

			const Node* rangeNode = findNode(locNode, "Range");
			reqData.range = rangeNode ? rangeNode->content : "";
			// nonstandard range format
			// support ',' and '-'
			char rangeDelimiter = reqData.range.find(',') != std::string::npos ? ',' : '-';
			if(rangeDelimiter != '-') {
				std::replace(reqData.range.begin(), reqData.range.end(), rangeDelimiter, '-');
			}

			// transfer delay
			const Node* transferDelayNode = findNode(locNode, "TransferDelay");
			reqData.transferDelay = 0;
			if(transferDelayNode && !transferDelayNode->content.empty())
			{
				reqData.transferDelay = StringToLong(transferDelayNode->content);
			}

			//
			::TianShanIce::SRM::ResourceMap resMapObj;

			//1 subtype  [Will be passed via the parameter subfile of the API
			//2 URL      [The original URI in the client’s request.Passed thru the parameter resourceRequirement[rtURI].resourceData[“uri”]]
			ZQTianShan::Util::updateResourceData(resMapObj,TianShanIce::SRM::rtURI,"uri",strFullUrl);

			//3 Client TransferAddress  ----Will be passed via the parameter resourceRequirement[rtEthernetInterface].resourceData[destIP]
			ZQTianShan::Util::updateResourceData(resMapObj,TianShanIce::SRM::rtEthernetInterface,"destIP",reqFrom);
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
			CacheServerSession sess;
			::TianShanIce::Storage::CacheStorePrx  cacheStore;

			const char* pUserAgent = prequest->header("User-Agent");
			if(pUserAgent && pUserAgent[0] != NULL) {
				propertiesobj["User-Agent"] = pUserAgent;
			}
						
			ZQTianShan::Util::updatePropertyData(propertiesobj,"Range",reqData.range.empty()? "0-" : reqData.range);
			ZQTianShan::Util::updatePropertyData(propertiesobj,CDN_RANGE,reqData.range.empty()? "0-" : reqData.range);
			ZQTianShan::Util::updatePropertyData(propertiesobj,"CDNType", "");
			ZQTianShan::Util::updatePropertyData(propertiesobj,"TansferDeplay", "0");
			if( _env._conf.exposeAssetIndexData > 0 ) {
				ZQTianShan::Util::updatePropertyData(propertiesobj,SYS_PROP("exposeIndex"),"1");
			}
			
			char msgbuf[1024] ={0};
			snprintf(msgbuf, sizeof(msgbuf)-2, "%s", reqData.clientTransfer.c_str());
			ZQTianShan::Util::updatePropertyData(propertiesobj, SYS_PROP("client"), msgbuf);
			snprintf(msgbuf, sizeof(msgbuf)-2, "%p", &reqData);
			ZQTianShan::Util::updatePropertyData(propertiesobj, SYS_PROP("clientSession"), msgbuf);

			try
			{	////get the proxy of cache store
				_log(ZQ::common::Log::L_INFO,ReqLOGFMT(CacheServerRequestHandle,"trying to connect to [%s]"),
					_env._conf.cacheserverendpoint.c_str() );
				std::string connId;
				int nMod = 0;
				{
					static ZQ::common::Mutex _mutex;
					static int _lLoop = 0;
					ZQ::common::MutexGuard guard(_mutex);
					nMod = _lLoop++ % 10;
				}
				char strtemp[32];
				sprintf(strtemp, "conn_%lld\0", nMod);
				connId = strtemp;

				Ice::ObjectPrx prx = _env._communicator->stringToProxy(_env._conf.cacheserverendpoint);
				Ice::ObjectPrx connectPrx = prx->ice_connectionId(strtemp);
				cacheStore=::TianShanIce::Storage::CacheStorePrx::uncheckedCast( connectPrx );
			}
			catch( const TianShanIce::BaseException& ex)
			{
				char msgbuf[512]={0};
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheServerRequestHandle, "Caught %s while connecting to CacheServer [%s] ."), 
					ex.ice_name().c_str(), _env._conf.cacheserverendpoint.c_str() );
				sprintf(msgbuf, "230888 Got unexpected %s Caught %s while connecting to CacheServer ", ex.ice_name().c_str(),_env._conf.cacheserverendpoint.c_str());
				CACHEREQUESTR_ERROR_RESPONSE(presponse, 500, "Internal  CacheServer Error", msgbuf);
				return;
			}
			catch( const Ice::Exception& ex)
			{
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheServerRequestHandle, "Cautht %s while connecting to CacheServer [%s] ."), ex.ice_name().c_str(), _env._conf.cacheserverendpoint.c_str());
				sprintf(msgbuf, "230888 Got unexpected %s Caught %s while connecting to CacheServer .", ex.ice_name().c_str(),_env._conf.cacheserverendpoint.c_str());
				CACHEDELETER_ERROR_RESPONSE(presponse, 500, "Internal  CacheServer Error", msgbuf);
				return;
			}

			//::TianShanIce::Streamer::StreamPrx stream = store->exportContentAsStream("cdntest1234567890127zq.com", "forward/1", 60*1000,0, resources, params);
			try
			{
				std::string contentname=reqData.assetId+reqData.providerId;//exportContentAsStream  cachestore module
				int64 itexportCAS =ZQ::common::now();			
				sess.stream = cacheStore->exportContentAsStream(contentname, 
					reqData.subType,
					idleStreamTimeoutobj,
					cacheStoreDepthobj,
					resMapObj,propertiesobj);
				
				if (NULL == sess.stream)
				{
					_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheServerRequestHandle, "failed to exportContentAsStream for content [%s]"), contentname.c_str());
					CACHEREQUESTR_ERROR_RESPONSE(presponse, 500, "Internal  CacheServer Error", "failed to exportContentAsStream");
					return;
				}
				else
					_log(ZQ::common::Log::L_INFO, ReqLOGFMT(CacheServerRequestHandle, "exportContentAsStream [%s] for content [%s] successfully took %dms"),
					_env._communicator->proxyToString(cacheStore).c_str(), contentname.c_str(),  (int)(ZQ::common::now() - itexportCAS));

			}
			catch(const TianShanIce::InvalidParameter& ex)
			{
				char mesbuf[512]={0};
				sprintf(mesbuf, "failed to exportContentAsStream caught exception [%s]  errorcode[%d] errmessage(%s]",ex.ice_name().c_str(),ex.errorCode,ex.message.c_str());
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheServerRequestHandle,"%s"), mesbuf);
				CACHEREQUESTR_ERROR_RESPONSE(presponse, 400, "Bad Request",mesbuf);
				return;
			}
			catch(const TianShanIce::ClientError& ex)
			{
				char mesbuf[512]={0};
				sprintf(mesbuf, "failed to exportContentAsStream caught exception [%s]  errorcode[%d] errmessage(%s]",ex.ice_name().c_str(),ex.errorCode,ex.message.c_str());
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheServerRequestHandle,"%s"), mesbuf);
				CACHEREQUESTR_ERROR_RESPONSE(presponse, 400, "Bad Request",mesbuf);
				return;
			}
			catch(const TianShanIce::ServerError& ex)
			{
				char mesbuf[512]={0};
				sprintf(mesbuf, "failed to exportContentAsStream caught exception [%s]  errorcode[%d] errmessage(%s]",ex.ice_name().c_str(),ex.errorCode,ex.message.c_str());
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheServerRequestHandle,"%s"), mesbuf);
				switch(ex.errorCode)
				{
				case 400:
					CACHEREQUESTR_ERROR_RESPONSE(presponse, 400, "Bad Request",mesbuf);
				case 500:
					CACHEREQUESTR_ERROR_RESPONSE(presponse, 500, "Internal CacheServer Error",mesbuf);
				case 503:
					CACHEREQUESTR_ERROR_RESPONSE(presponse, 503, "Service Unavailable",mesbuf);
				case 404:
					CACHEREQUESTR_ERROR_RESPONSE(presponse, 404, "Not Found",mesbuf);
				case 416:
					CACHEREQUESTR_ERROR_RESPONSE(presponse, 416, "Requested Range Not Satisfiable",mesbuf);
				default:
					CACHEREQUESTR_ERROR_RESPONSE(presponse, 400, "Internal  CacheServer Error",mesbuf);
				}
				return;
			}
			catch(const TianShanIce::BaseException& ex)
			{
				char mesbuf[1024]={0};
				sprintf(mesbuf, "failed to exportContentAsStream caught exception [%s]  errorcode[%d] errmessage(%s]",ex.ice_name().c_str(),ex.errorCode,ex.message.c_str());
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheServerRequestHandle,"%s"), mesbuf);
				CACHEREQUESTR_ERROR_RESPONSE(presponse, 500, "Internal  CacheServer Error",mesbuf);
				return;
			}
			catch(const Ice::Exception& ex)
			{
				char mesbuf[512]={0};
				sprintf(mesbuf, "failed to exportContentAsStream caught ice exception[%s]",ex.ice_name().c_str());
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheServerRequestHandle, "%s"),mesbuf );
				CACHEREQUESTR_ERROR_RESPONSE(presponse, 500, "Internal  CacheServer Error",mesbuf);
				return;
			}
			catch (...)
			{
				char mesbuf[512]={0};
				sprintf(mesbuf, "failed to exportContentAsStream caught unkonwn exception");
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheServerRequestHandle, "%s"),mesbuf );
				CACHEREQUESTR_ERROR_RESPONSE(presponse, 500, "Internal  CacheServer Error",mesbuf);
				return;
			}

			//get the properties of this stream
			::TianShanIce::Properties propertiess;
			try
			{
				_log(ZQ::common::Log::L_INFO, ReqLOGFMT(CacheServerRequestHandle, "get stream Properties"));
				propertiess = sess.stream ->getProperties();
			}
			catch (const TianShanIce::BaseException& ex)
			{
				char mesbuf[1024]={0};
				sprintf(mesbuf, "caught %s  errorcode[%d] errmessage(%s]) by call getProperties [%s] .",ex.ice_name().c_str(),ex.errorCode,ex.message.c_str(), _env._conf.cacheserverendpoint.c_str());
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheServerRequestHandle,"%s"), mesbuf);
				CACHEREQUESTR_ERROR_RESPONSE(presponse, 500, "Internal  CacheServer Error",mesbuf);
				return;
			}
			catch (const Ice::Exception& ex)
			{
				char mesbuf[512]={0};
				sprintf(mesbuf, "caught %s ,(%s) by call getProperties [%s] .",ex.ice_name().c_str(),ex.what(), _env._conf.cacheserverendpoint.c_str());
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheServerRequestHandle, "%s"),mesbuf );
				CACHEREQUESTR_ERROR_RESPONSE(presponse, 500, "Internal  CacheServer Error",mesbuf);
				return;
			}
			catch (...)
			{
				char mesbuf[512]={0};
				sprintf(mesbuf, "caught unkonwn exception by call invoke getProperties  .");
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheServerRequestHandle, "%s"),mesbuf );
				CACHEREQUESTR_ERROR_RESPONSE(presponse, 500, "Internal  CacheServer Error",mesbuf);
				return;
			}

			std::string sPropertyDataContent;
			::TianShanIce::Properties::iterator itor;
			for(itor = propertiess.begin(); itor != propertiess.end(); itor++)
				sPropertyDataContent+=itor->first + "=" + itor->second+ " ;"; 
			_log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(CacheServerRequestHandle, "property-content:%s"),sPropertyDataContent.c_str());
			_log(ZQ::common::Log::L_DEBUG, ReqLOGFMT(CacheServerRequestHandle, "prepare response message"));

			std::string transferporttmp,transferidtmp,transfertimeouttmp,availablerangetmp,openforwritetmp,portnumber;
			std::string idxContentGeneric, idxContentSubfiles;

			//TianShanIce::Properties
			try
			{
				ZQTianShan::Util::getPropertyData(propertiess,CDN_TRANSFERPORT,transferporttmp);     //cdn.transferPort
				ZQTianShan::Util::getPropertyData(propertiess,CDN_TRANSFERID,transferidtmp);         //cdn.transferId
				ZQTianShan::Util::getPropertyData(propertiess,CDN_TRANSFERTIMEOUT,transfertimeouttmp);//cdn.transferTimeout
				ZQTianShan::Util::getPropertyData(propertiess,CDN_AVAILRANGE,availablerangetmp);		//cdn.availableRange
				ZQTianShan::Util::getPropertyData(propertiess,CDN_OPENFORWRITE,openforwritetmp);		//cdn.openForWrite
				ZQTianShan::Util::getPropertyData(propertiess,CDN_TRANSFERPORTNUM,portnumber);			//cdn.transferPortNum
				ZQTianShan::Util::getPropertyDataWithDefault(propertiess,CDN_IDXCONTENT_GENERIC,"",idxContentGeneric);
				ZQTianShan::Util::getPropertyDataWithDefault(propertiess,CDN_IDXCONTENT_SUBFILES,"",idxContentSubfiles);
			}
			catch (const TianShanIce::InvalidParameter& ex)
			{
				char mesbuf[512]={0};
				sprintf(mesbuf, "getPropertyData() invalidParameter exception is caught:%s",ex.message.c_str());
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheServerRequestHandle, "%s"),mesbuf);		
				CACHEREQUESTR_ERROR_RESPONSE(presponse, 500, "Internal  CacheServer Error",mesbuf);
				return ;
			}
			catch (...)
			{
				char mesbuf[512]={0};
				sprintf(mesbuf, "caught unkonwn exception via  getPropertyData()  .");
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheServerRequestHandle, "%s"),mesbuf );
				CACHEREQUESTR_ERROR_RESPONSE(presponse, 500, "Internal  CacheServer Error",mesbuf);
				return;
			}
			if(openforwritetmp == "yes" && (!gResource.conf.cacheforwardurl.empty()) )
			{
				_forwardUrl = gResource.conf.cacheforwardurl;
				bool res = cache_forwardRequest(prequest, presponse, reqId, parser, reqData);	
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheServerRequestHandle, "forward return [%s]"),res?"true":"false" );
				return;
			}
			
			sess.transferId = transferidtmp;
			// ::TianShanIce::Streamer::StreamPrx streamPrx = ::TianShanIce::Streamer::StreamPrx::uncheckedcast( _env._communicator->stringToProxy(( transfer-Id))

			//add CacheServerSession to CacheSessionMap
			if (sess.stream && !sess.transferId.empty())
			{
				sess.stampCreated = sess.stampLastTouch = ZQ::common::now();
				ZQ::common::MutexGuard gl(_lkCacheSessMap);//gl(_env._lkCacheSessMap);
				MAPSET(CacheSessionMap, _cacheSessMap, sess.transferId, sess);//_env._cacheSessMap,
			}
			else
			{
				_log(ZQ::common::Log::L_WARNING, ReqLOGFMT(CacheServerRequestHandle,"failed to invoke exportContentAsStream by parameters:contentName[%s],subFile[%s],idleStreamTimeout[%ld],cacheStoreDepth[%d],resources[uri[%s]],destIP[%s],bandwidth0[%ld],bandwidth1[%ld]"),reqData.assetId.c_str(),reqData.subType.c_str(),idleStreamTimeoutobj,cacheStoreDepthobj,strFullUrl.c_str(),reqFrom.c_str(),reqData.transferRate,reqData.ingressCapacity);
				CACHEREQUESTR_ERROR_RESPONSE(presponse, 500, "Internal CacheServer Error", "230132 failed to invoke exportContentAsStream.");
				return;
			}
			//5 compose XML fields
			std::ostringstream buf;
			buf << "<LocateResponse>\n" 
				<< "  <TransferPort>" << transferporttmp << "</TransferPort>\n" 
				<< "  <TransferID>" << transferidtmp << "</TransferID>\n"
				<< "  <TransferTimeout>" <<transfertimeouttmp << "</TransferTimeout>\n"
				<< "  <AvailableRange>" << availablerangetmp << "</AvailableRange>\n"
				<< "  <OpenForWrite>" << openforwritetmp << "</OpenForWrite>\n"
				<< "  <PortNum>" << portnumber << "</PortNum>\n";
			
			if( _env._conf.exposeAssetIndexData > 0 ) {
				if(reqData.subType == CDN_SUBTYPE_Index || reqData.subType == CDN_SUBTYPE_NormalForward) {
					buf << "<ClipInfo>"<<std::endl;
					buf << "<Residential recording=\""<< (openforwritetmp == "yes" ? 1 : 0) <<"\" />" << std::endl;
					buf << idxContentGeneric << std::endl;
					if( reqData.subType == CDN_SUBTYPE_Index ) {
						buf << idxContentSubfiles << std::endl;
					}
					buf << "</ClipInfo>"<<std::endl;
				}
			}
			buf << "</LocateResponse>";
			presponse->setStatus(201, "Created");
			std::string respContent = buf.str();
			presponse->setContent(respContent.data(), respContent.size());
			_log(ZQ::common::Log::L_INFO, ReqLOGFMT(CacheServerRequestHandle, "client[%s:%d],AssetID[%s],response content:%s"),reqFrom.c_str(),reqFromPort, reqData.assetId.c_str(), respContent.c_str());

			std::string respDumpHint = "[req-" + reqId + "] CahceServerLocateResponse:";
			_log.hexDump(ZQ::common::Log::L_DEBUG, respContent.c_str(), respContent.size(), respDumpHint.c_str(), true); // text-only dump

			_log(ZQ::common::Log::L_NOTICE, ReqLOGFMT(CacheServerRequestHandle, "TransferCreated: cost(%d) AssetId(%s)ProviderId(%s)SubType(%s)TransferPort(%s)TransferId(%s)RequestedFrom(%s:%d)StatusCode(%d)"), (int)(ZQ::common::now() - t1), reqData.assetId.c_str(), reqData.providerId.c_str(), (reqData.subType.empty() ? reqData.extension.c_str() : reqData.subType.c_str()), transferporttmp.c_str(), transferidtmp.c_str(), reqFrom.c_str(), reqFromPort,presponse->getStatusCode());

		}

		void CacheServerRequestHandle::cache_deleterequest(const CRG::IRequest* prequest, CRG::IResponse* presponse)
		{
			int64 t1= ZQ::common::now() ;
			std::string reqFrom  ;
			int reqFromPort =0 ;
			std::string reqId = _env.reqIdGen.create() ;
			prequest->getClientEndpoint(reqFrom,reqFromPort) ;
			_log(ZQ::common::Log::L_INFO, ReqLOGFMT(CacheDeleteRequestHandle, "Cache_deleterequest from %s:%d"), reqFrom.c_str(), reqFromPort) ;

			typedef SimpleXMLParser::Node  NodeCDR ;
			std::string reqContent ;
			prequest->getContent(reqContent) ;
			std::string reqDumpHint = "[req-" + reqId + "] CacheDeleteRequest:" ;
			_log.hexDump(ZQ::common::Log::L_DEBUG, reqContent.c_str(), reqContent.size(), reqDumpHint.c_str(), true) ;

			CacheDeleteRequestData reqData;
			//
			SimpleXMLParser  parser ;
			try
			{
				parser.parse(reqContent.data(), reqContent.size(), 1) ;
			}
			catch (ZQ::common::ExpatException& e)
			{
				_log(ZQ::common::Log::L_WARNING, ReqLOGFMT(CacheDeleteRequestHandle, "ExpatException: [%s] during parsing request body"), e.getString());
				CACHEDELETER_ERROR_RESPONSE(presponse, 400, "Bad Request", "230103 Bad XML format") ;
				return ;
			}
			//step 1:  check the content and extract the post data
			const Node* locNode = findNode(&parser.document(), "LocateRequest") ;
			if ( NULL ==  locNode)
			{
				CACHEDELETER_ERROR_RESPONSE(presponse, 400, "Bad Request", "230104 XML element missed: <LocateRequest>");
				return ;
			}
			const Node* clienttransferNode = findNode(locNode,"ClientTransfer") ;
			reqData.clientTransfer = clienttransferNode ? clienttransferNode->content : "" ;
			if (reqData.clientTransfer.empty())
				reqData.clientTransfer = reqFrom ;

			//
			const Node* transferiddeleteNode = findNode(locNode, "TransferIDDelete") ;
			reqData.transferIdDelete = transferiddeleteNode ? transferiddeleteNode->content : "" ;
			if (reqData.transferIdDelete.empty())
			{
				CACHEDELETER_ERROR_RESPONSE(presponse, 400, "Bad Request", "230104 XML element-value missed: <TransferIDDelete>");
				return ;
			}

			char msgbuf[256] ={0};

			//2   via transfer-Id  get stream proxy query the map to find out stream proxy string by transferId . 
			//3  delete
			ZQ::common::MutexGuard gl(_lkCacheSessMap);
			CacheSessionMap::iterator it = _cacheSessMap.find(reqData.transferIdDelete);
			if (_cacheSessMap.end() == it)
			{
				_log(ZQ::common::Log::L_DEBUG,ReqLOGFMT(CacheDeleteRequestHandle,"not found [%s] in CacheServerSessionMap"),reqData.transferIdDelete.c_str());
				return;
			}
			try {
				if (it->second.stream)
					it->second.stream->destroy();
			}
			catch( const TianShanIce::BaseException& ex)
			{
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheDeleteRequestHandle, "Cautht %s from LAM for content: ClientTransfer=%s TransferIDDelete=%s."), ex.ice_name().c_str(), reqData.clientTransfer.c_str(), reqData.transferIdDelete.c_str());
				sprintf(msgbuf, "230132 Got unexpected %s from CacheServer", ex.ice_name().c_str());
				CACHEDELETER_ERROR_RESPONSE(presponse, 500, "Internal Server Error", msgbuf);
				return;
			}
			catch( const Ice::ObjectNotExistException& ex)
			{
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheDeleteRequestHandle, "Cautht %s from LAM for content: ClientTransfer=%s TransferIDDelete=%s."), ex.ice_name().c_str(), reqData.clientTransfer.c_str(), reqData.transferIdDelete.c_str());
				sprintf(msgbuf, "230132 Got unexpected %s from CacheServer", ex.ice_name().c_str());
				CACHEDELETER_ERROR_RESPONSE(presponse, 500, "Internal Server Error", msgbuf);
				return;
			}
			catch( const Ice::SocketException& ex)
			{
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheDeleteRequestHandle, "Cautht %s from LAM for content: ClientTransfer=%s TransferIDDelete=%s."), ex.ice_name().c_str(), reqData.clientTransfer.c_str(), reqData.transferIdDelete.c_str());
				sprintf(msgbuf, "230132 Got unexpected %s from CacheServer", ex.ice_name().c_str());
				CACHEDELETER_ERROR_RESPONSE(presponse, 500, "Internal Server Error", msgbuf);
				return;
			}
			catch( const Ice::TimeoutException& ex)
			{
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheDeleteRequestHandle, "Cautht %s from LAM for content: ClientTransfer=%s TransferIDDelete=%s."), ex.ice_name().c_str(), reqData.clientTransfer.c_str(), reqData.transferIdDelete.c_str());
				sprintf(msgbuf, "230132 Got unexpected %s from CacheServer", ex.ice_name().c_str());
				CACHEDELETER_ERROR_RESPONSE(presponse, 500, "Internal Server Error", msgbuf);
				return;
			}
			catch( const Ice::Exception& ex)
			{
				_log(ZQ::common::Log::L_ERROR, ReqLOGFMT(CacheDeleteRequestHandle, "Cautht %s from LAM for content: ClientTransfer=%s TransferIDDelete=%s."), ex.ice_name().c_str(), reqData.clientTransfer.c_str(), reqData.transferIdDelete.c_str());
				sprintf(msgbuf, "230132 Got unexpected %s from CacheServer", ex.ice_name().c_str());
				CACHEDELETER_ERROR_RESPONSE(presponse, 500, "Internal Server Error", msgbuf);
				return;
			}
			_cacheSessMap.erase(it);;//_env._cacheSessMap.erase(it); 

			//4  response 
			presponse->setStatus(200, "OK");
			std::string respContent = "" ;//= "Server: TestCacheServer/1.1.0 Comcast";
			presponse->setContent(respContent.data(), respContent.size() );
			_log(ZQ::common::Log::L_INFO, ReqLOGFMT(CacheDeleteRequestHandle, "client[%s:%d],terminateID[%s] response content:%s"), reqFrom.c_str(), reqFromPort,reqData.transferIdDelete.c_str(), respContent.c_str());

			_log(ZQ::common::Log::L_INFO, ReqLOGFMT(CacheDeleteRequestHandle, "cost %d ms"), (int)(ZQ::common::now() - t1));
			return  ;
		}
		void CacheServerRequestHandle::onRequest(const CRG::IRequest* req, CRG::IResponse* resp)
		{
			std::string  strFullURL = req->uri() ;
			//ZQ::common::URLStr urlstr(strFullURL.c_str());
			//std::string urlPath = urlstr.getVar("/cacheserver");//urlstr.getPath();

			_log(ZQ::common::Log::L_INFO, CLOGFMT(CacheServerRequestHandle, "onRequest() : receive request uri=%s"), strFullURL.c_str());
			/*if ("/cacheserver" == strFullURL)
			{
				return cache_locatorequest(req, resp);
			}
			else if ("/cacheserver/transferterminate" == strFullURL)
			{
				return cache_deleterequest(req, resp);
			}*/
			boost::regex cacheserverRegex(gResource.conf.cacheserveruri);
			//boost::regex cacheserverTerminateRegex(gResource.conf.cacheserveruri + "/transferterminate");

			if (boost::regex_match(strFullURL.c_str(), cacheserverRegex))
			{
				std::string strContent;
				req->getContent(strContent);

				if (NULL != strstr(strContent.c_str(), "<TransferIDDelete>"))
				{
					return cache_deleterequest(req, resp);
				}
				return cache_locatorequest(req, resp);
			}		
			else
			{
				glog(ZQ::common::Log::L_INFO, CLOGFMT(CacheServerRequestHandle, "onRequest() : ingore this request uri=%s"), strFullURL.c_str());
			}
		}
	} 
}
