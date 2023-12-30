#include "LocateRequest.h"
#include <TimeUtil.h>

namespace C2Streamer {
	void splitFileName(const std::string& pathname, std::string& pid, std::string& paid, std::string& ext);
#ifdef FANGLI_EVIL
	std::string fixupPAIDPID(std::string& PAID, std::string& PID, const char outSeperator = '_', const char* inSeperators = "_");
#endif
};

namespace ZQ{
namespace StreamService{
    ZQ::common::AtomicInt LocateRequest::_atomicID;
    static long StringToLong(const std::string& s)
    {
#ifdef ZQ_OS_MSWIN
        return ::_strtoi64(s.c_str(), NULL, 10);
#elif defined(__x86_64)
        return strtol(s.c_str(), NULL, 10);
#else
        return strtoll(s.c_str(), NULL, 10);
#endif
    }

    LocateRequest::LocateRequest(ZQ::common::Log& log, IC2RequestCallBack::Ptr requestCB, RequestParams params, std::string instanceKey,  int64 bufferReqID)
        : LibAsync::Socket(LibAsync::getLoopCenter().getLoop()),RequestHandle(log, requestCB, params, instanceKey, bufferReqID), _paid(""), _pid("")
    {
    }

    LocateRequest::~LocateRequest()
    {
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] locate request ended, release resource"),
            _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
    }

    bool LocateRequest::process()
    {
        _status = ZQ::StreamService::NotConnected;

        _reqBody  = "";
        _respBody = "";

        // parse filename
		// remove slash
		_params.contentName = _params.filename;
        size_t lastSlashPos = _params.contentName.find_last_of(FNSEPC);
        if (std::string::npos != lastSlashPos)
        {
            _params.contentName = _params.contentName.substr(lastSlashPos + 1);
        }       

		C2Streamer::splitFileName(_params.contentName, _pid, _paid, _params.subType);
        //_paid = _params.contentName.substr(0, 20);
        //if ('_' == _params.contentName.at(20))
        //{
        //    _pid = _params.contentName.substr(20 + 1);
        //}else{
        //    _pid = _params.contentName.substr(20);
        //}

        if (_params.url.empty())
        {
            _status = ZQ::StreamService::Failure;
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] process(), send locate request failed, because of the url is empty"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
            _cb->onError(crGeneric, -5, "URL is empty");
            if( _cb ) _cb = NULL;
            return false;
        }

        if ("index" == _params.subType)
        {
            setTimeout(_params.indexTimeout);
        }else{
            setTimeout(_params.mainfileTimeout);
        }

        std::string host		= _params.props[C2CLIENT_HOST].empty() ? "None" : _params.props[C2CLIENT_HOST];
        std::string userAgent	= _params.props[C2CLIENT_UserAgent].empty() ? "EdgeFE/2.8" : _params.props[C2CLIENT_UserAgent];
        std::string contentType = _params.props[C2CLIENT_ContentType].empty() ? "text/xml-external-parsed-entity" : _params.props[C2CLIENT_ContentType];

        LibAsync::HttpMessagePtr locateMsgPtr = new LibAsync::HttpMessage(HTTP_REQUEST);
        locateMsgPtr->method(HTTP_POST);
        locateMsgPtr->url(_params.url);

        //add header
        locateMsgPtr->header(C2CLIENT_HOST, host);
        locateMsgPtr->header(C2CLIENT_UserAgent, userAgent);
        locateMsgPtr->header(C2CLIENT_ContentType, contentType);

        //generate body
        _reqBody = generateLocateBody();
		if( _reqBody.length() == 0) {
			_log.error(CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] process() no request Body generated. pid[%s] paid[%s] subtype[%s] transferRate[%s] ingressCapacity[%s] clientTransfer[%s] "),
					_instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, _pid.c_str(), _paid.c_str() , _params.subType.c_str(), _params.transferRate.c_str(),
					_params.ingressCapacity.c_str(), _params.clientTransfer.c_str());
		}
        locateMsgPtr->contentLength(_reqBody.size());

        _startTime = ZQ::common::TimeUtil::now();

		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] process() request Body:%s"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID,_reqBody.c_str());
        //bind up stream ip
        if (!bind(_params.upstreamIP, 0))
        {
            _status = ZQ::StreamService::Failure;
#ifdef ZQ_OS_LINUX
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] process(), bind up stream ip[%s] failed, error[%d:%s]"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, _params.upstreamIP.c_str(), errno, strerror(errno));
#else
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] process(), bind up stream ip[%s] failed"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, _params.upstreamIP.c_str());
#endif
            _cb->onError(crSocketError, -8, "bind up stream ip failed");
            if( _cb ) _cb = NULL;
            return false;
        }

        std::string localIP;
        unsigned short localPort;
        if (getLocalAddress(localIP, localPort))
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] local socket info[%s:%d] locate address[%s:%d]"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, localIP.c_str(), localPort, _params.locateIP.c_str(), _params.locatePort);
        }else{
            _log(ZQ::common::Log::L_WARNING, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] get local address failed"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
        }

        if (!checkTimeout())
        {
            return false;
        }

        _bBodySend = false;
		std::string sendingMsg = locateMsgPtr->toRaw();
		removeCRLF(sendingMsg);
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] process(), begin sending locate request endpoint[%s:%d]: %s"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID,  _params.locateIP.c_str(), _params.locatePort, sendingMsg.c_str());
        if (!beginRequest(locateMsgPtr, _params.locateIP, _params.locatePort))
        {
            _status = ZQ::StreamService::Failure;
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] process(), failed to send locate request to [%s:%d]"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, _params.locateIP.c_str(), _params.locatePort);
            if (_cb)
            {
                _cb->onError(crSocketError, -9, "send locate message failed");
            }
            if( _cb ) _cb = NULL;
            return false;
        }
        return true;
    }

    std::string LocateRequest::generateLocateBody()
    {
        std::map<std::string, std::string> reqProp = _params.props;
        std::ostringstream sendBodyMsg;

        std::string transferRate    = _params.transferRate;
        std::string clientTransfer  = _params.clientTransfer;
        std::string range           = _params.range;
        std::string ingressCapacity = _params.ingressCapacity;
        std::string exclusionList   = _params.exclusionList;
        std::string transferDelay   = _params.transferDelay;

        sendBodyMsg << "<LocateRequest>" << "\r\n";
        sendBodyMsg << "<Object>" << "\r\n";
        sendBodyMsg << "<Name>" << "\r\n";

		if (_pid.empty()) return "";
		sendBodyMsg << "<ProviderID>" << _pid << "</ProviderID>" << "\r\n";

		if (_paid.empty()) return "";
		sendBodyMsg << "<AssetID>" << _paid << "</AssetID>" << "\r\n";

        sendBodyMsg << "</Name>" << "\r\n";

        if (_params.subType.empty()) return "";
        sendBodyMsg << "<SubType>" << _params.subType << "</SubType>" << "\r\n";

        sendBodyMsg << "</Object>" << "\r\n";

        if (transferRate.empty()) return "";
        sendBodyMsg << "<TransferRate>" << transferRate << "</TransferRate>" << "\r\n";

        if (ingressCapacity.empty()) return "";
        sendBodyMsg << "<IngressCapacity>" << ingressCapacity << "</IngressCapacity>" << "\r\n";

        if (clientTransfer.empty()) return "";
        sendBodyMsg << "<ClientTransfer>" << clientTransfer << "</ClientTransfer>" << "\r\n";

        sendBodyMsg << "<ExclusionList>" << exclusionList << "</ExclusionList>" << "\r\n";

        sendBodyMsg << "<Range>" << range << "</Range>" << "\r\n";

        sendBodyMsg << "<TransferDelay>" << transferDelay << "</TransferDelay>" << "\r\n";

        sendBodyMsg << "</LocateRequest>";

		_log.debug(CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] generated locate body [%s]"),
				_instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, sendBodyMsg.str().c_str());

        return sendBodyMsg.str();
    }

    bool LocateRequest::parseResponse(const SimpleXMLParser::Node* root, LocateResponseData& respData, std::string& error)
    {
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] parseResponse() entry"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);

        typedef SimpleXMLParser::Node Node;
        // step 1: check the content and extract the response data
        const Node* locNode = findNode(root, "LocateResponse");
        if(NULL == locNode)
        { // bad xml content
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] parseResponse() XML element missed: <LocateResponse>"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
            return false;
        }

        const Node* transferPortNode = findNode(locNode, "TransferPort");
        if(!transferPortNode)
        { // parameter missed
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] parseResponse() XML element missed: <TransferPort>"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
            return false;
        }
        respData.transferPort = transferPortNode->content;

        const Node* transferIDNode = findNode(locNode, "TransferID");
        if (!transferIDNode)
        {
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] parseResponse() XML element missed: <TransferID>"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
            return false;
        }
        respData.transferId = transferIDNode->content;

        const Node* transferTimeoutNode = findNode(locNode, "TransferTimeout");
        if (!transferTimeoutNode)
        {
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] parseResponse() XML element missed: <TransferTimeout>"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
            return false;
        }
        respData.transferTimeout = StringToLong(transferTimeoutNode->content);

        const Node* availableRangeNode = findNode(locNode, "AvailableRange");
        if (!availableRangeNode)
        {
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] parseResponse() XML element missed: <AvailableRange>"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
            return false;
        }
        respData.availableRange = availableRangeNode->content;

        const Node* openForWriteNode = findNode(locNode, "OpenForWrite");
        if (!openForWriteNode)
        {
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] parseResponse() XML element missed: <OpenForWrite>"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
            return false;
        }
        respData.openForWrite = openForWriteNode->content;

        const Node* portNumNode = findNode(locNode, "PortNum");
        if (portNumNode)
        {
            respData.portNum = portNumNode->content;
        }

        if (_bIndex) //if call read(), with no need for these properties
        {
            const Node* residentialNode = findNode(locNode, "ClipInfo/Residential");
            if (!residentialNode)
            {
                _log(ZQ::common::Log::L_WARNING, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] parseResponse() XML element missed: <Residential>"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
            }else{
                std::map<std::string, std::string> residentialNodeAttrs = residentialNode->attrs;
                respData.recording = residentialNodeAttrs["recording"];
            }

            const Node* encodingInfoNode = findNode(locNode, "ClipInfo/EncodingInfo");
            if (!encodingInfoNode)
            {
                _log(ZQ::common::Log::L_WARNING, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] parseResponse() XML element missed: <EncodingInfo>"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
            }else{
                std::map<std::string, std::string> encodingInfoNodeAttrs = encodingInfoNode->attrs;
                respData.playTime = encodingInfoNodeAttrs["playTime"];
                respData.muxBitrate = encodingInfoNodeAttrs["muxBitrate"];
            }

            const Node* membersNode = findNode(locNode, "ClipInfo/Members");
            if (!membersNode)
            {
                _log(ZQ::common::Log::L_WARNING, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] parseResponse() XML element missed: <MembersNode>"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
            }else{
                std::list<Node>::const_iterator it = membersNode->children.begin();
                if (it == membersNode->children.end())
                {
                    _log(ZQ::common::Log::L_WARNING, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] parseResponse() ClipInfo/Members is empty "), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
                }

                std::map<std::string, std::string> subFileAttrs = it->attrs;
                std::string ext = subFileAttrs["extName"];

                respData.extName = ext;
                respData.startOffset = subFileAttrs["startOffset"];
                respData.endOffset = subFileAttrs["endOffset"];
            }    
        }

        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] parseResponse() parse response success"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
        return true;
    }

    void LocateRequest::onHttpDataReceived( size_t size )
    {
        if (ZQ::StreamService::Timeout == _status) return;
        cancel();

        if (ZQ::StreamService::Completed != _status)
        {
            //check if timeout
            if(!checkTimeout())
            {
                return;
            }

            recvRespBody(_recvBufs);
            //_log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateRequest, "[%s] onHttpDataReceived() current request received data[%d], continue"), _instanceKey.c_str(), _xSessionID.c_str(), size);
            _status = ZQ::StreamService::Receiving;
        }else{
            int statusCode = _headerMsg->code();
            //if (2 != statusCode/100)
            if (2 != statusCode/100 && statusCode != 416)
            {
                _log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] filename[%s] onHttpDataReceived(), send locate request failure, error[%d:%s]"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, _params.filename.c_str(), statusCode, _headerMsg->status().c_str());		
				C2RequestErrorCategory cate = crGeneric;
				if(statusCode == 404 || statusCode == 400) {
					cate = crNotFound;
				} else if( statusCode == 503 ) {
					cate = crServiceUnavailable;
				} else {
					statusCode = crHttpError;
				}
                _cb->onError( cate, statusCode, _headerMsg->status());
                if( _cb ) _cb = NULL;
                return;
            }

            // locate successful
            _log(ZQ::common::Log::L_INFO, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] onHttpDataReceived(), locate request latency[%ld], respBody: %s"),
					_instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, ZQ::common::TimeUtil::now()-_startCalcLatency, _respBody.c_str());

            // parse xml
            LocateResponseData respData;
            SimpleXMLParser parser;
            try
            {
                parser.parse(_respBody.data(), _respBody.size(), 1);
            }
            catch (const ZQ::common::ExpatException& e)
            {
                _log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] onHttpDataReceived(), parse response body catch exception, ExpatException: [%s] during parsing http response body: %s"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, e.getString(), _respBody.c_str());
                _cb->onError(crParseXmlDataError, C2_ERROR_BAD_RESPONSE_CODE, C2_ERROR_BAD_RESPONSE_STRING);
                if( _cb ) _cb = NULL;
                return;
            }

            std::string error;
            error.reserve(2048);
            if(!parseResponse(&parser.document(), respData, error))
            {//error
                _log(ZQ::common::Log::L_ERROR, CLOGFMT(LocateRequest, "[%s/%s] bufReqID[%ld] onHttpDataReceived(), parse response body failure, %s"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, error.c_str());
                _cb->onError(crParseXmlDataError, C2_ERROR_BAD_RESPONSE_CODE, C2_ERROR_BAD_RESPONSE_STRING);
                if( _cb ) _cb = NULL;
                return;
            }
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateRequest, "onHttpDataReceived() [%s/%s] bufReqID[%ld] response body %s"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID,_respBody.c_str());
            _cb->onLocate(respData);
			_cb = NULL; // _cb must be valid, clear it here to resolve the cyclic reference between LocateRequest and it's callback sink
						// It'd be better to use std::shared_ptr and std::weak_ptr
        }
        return;
    }

    bool LocateRequest::onHttpBody( const char* data, size_t size)
    {
        if (ZQ::StreamService::Timeout == _status) return false;
        _respBody.append(data, size);
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(LocateRequest, "onHttpBody() [%s/%s] bufReqID[%ld] append data [%s] response body %s"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, data,_respBody.c_str());
		
        return true;
    }
}}
