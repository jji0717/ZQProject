#include "C2Client.h"
#include <TimeUtil.h>
#include <InetAddr.h>

namespace ZQ{
namespace StreamService{

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

    C2ClientBind::C2ClientBind(ZQ::common::Log& log, RequestParams& params)
        : _log(log), _params(params)
    {

    }

    void C2ClientBind::onLocate(const LocateResponseData& resp)
    {
        ZQ::common::MutexGuard mg(_mutex);
        _log(ZQ::common::Log::L_INFO, CLOGFMT(C2ClientBind, "[%p] onLocate() locate request complete"), this);
       
        if (!&resp)
        {
            // #TODO response error
            OnError(-5, "locate response is empty");
            return;
        }

        AttrMap respMap;
        respMap[C2CLIENT_TransferPort]		=	resp.transferPort;
        respMap[C2CLIENT_TransferID]		=	resp.transferId;
        respMap[C2CLIENT_PortNum]			=	resp.portNum.empty() ? "12000" : resp.portNum;

        char timeout[8];
        itoa(resp.transferTimeout, timeout, 10);
        std::string strTimeout = timeout;
        respMap[C2CLIENT_TransferTimeout]	=	timeout;

        respMap[C2CLIENT_AvailableRange]	=	resp.availableRange;
        respMap[C2CLIENT_OpenForWrite]		=	resp.openForWrite;
        respMap[C2CLIENT_Recording]		    =	resp.recording;
        respMap[C2CLIENT_PlayTime]			=	resp.playTime;
        respMap[C2CLIENT_MuxBitrate]		=	resp.muxBitrate;
        respMap[C2CLIENT_ExtName]			=	resp.extName;
        respMap[C2CLIENT_StartOffset]		=	resp.startOffset;
        respMap[C2CLIENT_EndOffset]		    =	resp.endOffset;
        respMap[C2CLIENT_SubType]		    =   _params.subType;
        respMap[C2CLIENT_TransferRate]	    =   _params.transferRate;

        // paid pid
        respMap[C2CLIENT_AssetID] = _params.contentName.substr(0, 20);
        if ('_' == _params.contentName.at(20))
        {
            respMap[C2CLIENT_ProviderID] = _params.contentName.substr(20 + 1);
        }else{
            respMap[C2CLIENT_ProviderID] = _params.contentName.substr(20);
        }

        OnC2LocateResponse(_params.contentName, respMap);
    }

    void C2ClientBind::onData(const char* data, const size_t& size, bool error/* = false*/)
    {
        ZQ::common::MutexGuard mg(_mutex);
        _indexData.append(data, size);
    }

    void C2ClientBind::onRecvComplete()
    {
        ZQ::common::MutexGuard mg(_mutex);
        // parse index file
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ClientBind, "[%p] onRecvComplete() begin parse index file"), this);

        AttrMap respMap;
        ZQ::IdxParser::IndexData idxData;
        bool result = parseIndex(_params.contentName, _indexData.c_str(), _indexData.size(), idxData);
        if (result)
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2QueryIndex, "[%p] onRecvComplete() parse index success"), this);
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ClientBind, "[%p] parse index success"), this);

            //set response map
            respMap[C2CLIENT_AssetID] = _params.contentName.substr(0, 20);
            if ('_' == _params.contentName.at(20))
            {
                respMap[C2CLIENT_ProviderID] = _params.contentName.substr(20 + 1);
            }else{
                respMap[C2CLIENT_ProviderID] = _params.contentName.substr(20);
            }

            std::ostringstream playTimeOSS;
            playTimeOSS<<idxData.getPlayTime();
            if (!playTimeOSS.str().empty())
            {
                respMap[C2CLIENT_PlayTime]			=	playTimeOSS.str();
            }

            std::ostringstream muxBitRateOSS;
            muxBitRateOSS<<idxData.getMuxBitrate();
            if (!muxBitRateOSS.str().empty())
            {
                respMap[C2CLIENT_MuxBitrate]		=	muxBitRateOSS.str();
            }

            std::string extName = idxData.getSubFileName(0);
            if (!extName.empty())
            {
                size_t dotPos = extName.find_first_of('.');
                if (dotPos == extName.npos)
                {
                    respMap[C2CLIENT_ExtName]			=	extName;
                }
                else{
                    respMap[C2CLIENT_ExtName]			=	extName.substr(dotPos + 1);
                }
            }

            ZQ::IdxParser::IndexData::SubFileInformation info;
            if (idxData.getSubFileInfo(0, info))
            {
                std::ostringstream startOffsetOSS;
                uint64 startOffset =info.startingByte;
                startOffsetOSS<<startOffset;
                if (!startOffsetOSS.str().empty())
                {
                    respMap[C2CLIENT_StartOffset]		=	startOffsetOSS.str();
                }

                std::ostringstream endOffsetOSS;
                uint64 endOffset   = info.endingByte;
                endOffsetOSS<<endOffset;
                if (!endOffsetOSS.str().empty())
                {
                    respMap[C2CLIENT_EndOffset]		=	endOffsetOSS.str();
                }
            }
            OnC2GetResponse(_params.contentName, respMap);
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ClientBind, "[%p] onRecvComplete() query index successful"), this);
        }else{
            // parse index failed
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientBind, "[%p] onRecvComplete() parse index file failed"), this);
            OnError(C2_ERROR_BAD_RESPONSE_CODE, C2_ERROR_BAD_RESPONSE_STRING);
            return;
        }
    }

    void C2ClientBind::onTransferDelete()
    {
        ZQ::common::MutexGuard mg(_mutex);
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2QueryIndex, "[%p] onTransferDelete() transfer delete complete"), this);
    }

    void C2ClientBind::onError(C2RequestErrorCategory category, const int& err, const std::string& msg)
    {
        ZQ::common::MutexGuard gd(_mutex);
        OnError(err, msg);
    }

    bool C2ClientBind::parseIndex(std::string& contentName, const char* indexData, size_t dataSize, ZQ::IdxParser::IndexData& idxData)
    {
        ZQ::IdxParser::IdxParserEnv			idxParserEnv;
        idxParserEnv.AttchLogger(&_log);
        ZQ::IdxParser::IndexFileParser		idxParser(idxParserEnv);

        if(!idxParser.ParseIndexFromMemory( contentName, idxData, indexData, dataSize ) ) 
        {
            _log(ZQ::common::Log::L_ERROR,CLOGFMT(C2QueryIndex,"[%p] parseIndex() failed to parse index data for[%s], data size[%u]"), this, contentName.c_str(), (uint32)dataSize);
            return false;
        }
        return true;
    }


//sync c2client
C2ClientSync::C2ClientSync(ZQ::common::Log& log, const std::string& addr, const unsigned int port, const int timeout)
:_log(log), _addr(addr), _port(port), _timeout(timeout)
{
	init();
	//setTimeout();
}
C2ClientSync::~C2ClientSync()
{
	uninit();
}

bool C2ClientSync::sendLocateRequest(const std::string& url, const std::string& contentName, const std::string& subType, AttrMap& reqProp, AttrMap& respProp)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ClientSync, "sendLocateRequest() entry"));

	int64 startTime = ZQ::common::TimeUtil::now();
	setConnectTimeout(-_timeout*1000); // -10*1000*1000us

	//content name: 1. assetid(20)provideid
	//				2. /.*/assetid(20)_provideid
	size_t startAssetID;
	size_t startProvideID;
	size_t lastSlashPos = contentName.find_last_of('/');
	if (std::string::npos == lastSlashPos)
	{//no slash
		startAssetID = 0;
	}
	else{
		startAssetID = lastSlashPos + 1;
	}
	std::string assetId = contentName.substr(startAssetID, 20);

	if ('_' == contentName.at(startAssetID + 20))
	{
		startProvideID = startAssetID + 20 + 1;
	}else{
		startProvideID = startAssetID + 20;
	}
	std::string providerId = contentName.substr(startProvideID);

	std::ostringstream oss;
	oss<<"http://" << _addr << ":" << _port << url;
	_sendMap[C2CLIENT_URI] = oss.str();
	_sendMap[C2CLIENT_AssetID] = assetId;
	_sendMap[C2CLIENT_ProviderID] = providerId;
	_sendMap[C2CLIENT_SubType] = subType;

	_sendMap[C2CLIENT_HOST] = reqProp[C2CLIENT_HOST].empty() ? "None" : reqProp[C2CLIENT_HOST];

	_sendMap[C2CLIENT_UserAgent] = reqProp[C2CLIENT_UserAgent].empty() \
		? "ToInfinityAndBeyond" : reqProp[C2CLIENT_UserAgent];

	_sendMap[C2CLIENT_ContentType] = reqProp[C2CLIENT_ContentType].empty() \
		? "text/xml-external-parsed-entity" : reqProp[C2CLIENT_ContentType];

	_sendMap[C2CLIENT_TransferRate] = reqProp[C2CLIENT_TransferRate].empty() \
		? "20000000" : reqProp[C2CLIENT_TransferRate];

	_sendMap[C2CLIENT_IngressCapacity] = reqProp[C2CLIENT_IngressCapacity].empty() \
		? "16512000000" : reqProp[C2CLIENT_IngressCapacity];

	std::string strClientTransfer = ZQ::common::InetHostAddress::getLocalAddress().getHostAddress();

	_sendMap[C2CLIENT_ClientTransfer] = reqProp[C2CLIENT_ClientTransfer].empty() \
		? strClientTransfer : reqProp[C2CLIENT_ClientTransfer];

	_sendMap[C2CLIENT_ExclusionList] = reqProp[C2CLIENT_ExclusionList];

	_sendMap[C2CLIENT_Range] = reqProp[C2CLIENT_Range];

	_sendMap[C2CLIENT_TransferDelay] = reqProp[C2CLIENT_TransferDelay].empty() \
		? "-2000" : reqProp[C2CLIENT_TransferDelay];

	//set header
	//setHeader(C2CLIENT_HOST, (char*)_sendMap[C2CLIENT_HOST].c_str());
	setHeader(C2CLIENT_UserAgent, (char*)_sendMap[C2CLIENT_UserAgent].c_str());
	setHeader(C2CLIENT_ContentType, (char*)_sendMap[C2CLIENT_ContentType].c_str());

	//connect
	int ret = httpConnect(_sendMap[C2CLIENT_URI].c_str(),  HTTP_POST);
	int64 connectEndTime = ZQ::common::TimeUtil::now();
	int64 connTime  = connectEndTime - startTime;
	if (_timeout < connTime)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "timeout when connect via uri[%s]"), _sendMap[C2CLIENT_URI].c_str());
		return false;
	}
	else if (ret)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "failed to connect via uri[%s]"), _sendMap[C2CLIENT_URI].c_str());
		return false;
	}
	setSendTimeout(-(_timeout - connTime)*1000);
	
	//set body
	ret = setLocateRequestBody();
	int64 sendContentTime = ZQ::common::TimeUtil::now();
	int64 sendTime  = sendContentTime - startTime;
	if (_timeout < sendTime)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "timeout when send body data to server"));
		return false;
	}
	else if (ret)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "failed to send body data to server"));
		return false;
	}

	ret = httpEndSend();
	int64 endSendTime = ZQ::common::TimeUtil::now();
	sendTime = endSendTime - startTime;
	if (_timeout < sendTime)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "timeout when complete sending body data to server"));
		return false;
	}else if (ret)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "failed to complete sending body data to server"));
		return false;
	}
	setRecvTimeout(-(_timeout - sendTime)*1000);

	//receive
	ret = httpBeginRecv();
	int64 recvContentTime = ZQ::common::TimeUtil::now();
	int recvTime = recvContentTime - startTime;
	if (_timeout < recvTime)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "timeout when recv data from server"));
		return false;
	}else if (ret)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "failed to recv data from server"));
		return false;
	}

	while(!isEOF())
	{
		ret = httpContinueRecv();
		int64 endRecvTime = ZQ::common::TimeUtil::now();
		recvTime = endRecvTime - startTime;
		if (_timeout < recvTime)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "timeout when recv data from server"));
			return false;
		}else if (ret)
		{
			_log(ZQ::common::Log::L_ERROR,CLOGFMT(C2ClientSync,"failed to received data from server with errorCode[%d]"),
				getErrorcode());
		}
		break;
	}

	int64 endRecvTime = ZQ::common::TimeUtil::now();
	recvTime = endRecvTime - startTime;
	if (_timeout < recvTime)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "timeout when receive data"));
		return false;
	}

	int statusCode = getStatusCode();

	if (201 != statusCode && 206 != statusCode)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "status code is not 201 or 206, but %d, %s"), statusCode, getMsg());		
		return false;
	}

	std::string responseMsg;
	getContent(responseMsg);
	if (responseMsg.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "content is empty"));		
		return false;
	}

	if (201 == statusCode)
	{
		//TODO: parse xml
		LocateResponseData respData;
		SimpleXMLParser parser;
		try
		{
			parser.parse(responseMsg.data(), responseMsg.size(), 1);
		}
		catch (const ZQ::common::ExpatException& e)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "parse response body catch exception, ExpatException: [%s] during parsing http response body"), e.getString());
			return false;
		}


		std::string error;
		error.reserve(2048);
		if(!parseResponse(&parser.document(), respData, error))
		{//error
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "parse response body failure"));			
			return false;
		}

		setRevcMap(respData);

		//set response map
		respProp = _recvMap;
		respProp[C2CLIENT_AssetID]		= _sendMap[C2CLIENT_AssetID];
		respProp[C2CLIENT_ProviderID]   = _sendMap[C2CLIENT_ProviderID];
		respProp[C2CLIENT_SubType]		= _sendMap[C2CLIENT_SubType];
		respProp[C2CLIENT_TransferRate]	= _sendMap[C2CLIENT_TransferRate];		
	}

	if (206 == statusCode)
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ClientSync, "statusCode[%d]"), statusCode);
	}

	uninit();
	return true;
}

bool C2ClientSync::sendGetRequest(const std::string& url, const std::string& contentName, AttrMap& reqProp, AttrMap& respProp)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ClientSync, "sendGetRequest() entry"));

	int64 startTime = ZQ::common::TimeUtil::now();
	setConnectTimeout(-_timeout*1000); // -10*1000*1000us

	std::ostringstream oss;
	oss<<"http://" << _addr << ":" << _port <<"/"<< url;
	_sendMap[C2CLIENT_URI] = oss.str();

	_sendMap[C2CLIENT_HOST] = reqProp[C2CLIENT_HOST].empty() ? "None" : reqProp[C2CLIENT_HOST];

	_sendMap[C2CLIENT_UserAgent] = reqProp[C2CLIENT_UserAgent].empty() \
		? "ToInfinityAndBeyond" : reqProp[C2CLIENT_UserAgent];

	_sendMap[C2CLIENT_Range] = reqProp[C2CLIENT_Range];

	_sendMap[C2CLIENT_GET_REQUEST_Transfer_Delay] = reqProp[C2CLIENT_GET_REQUEST_Transfer_Delay].empty() \
		? "-2000" : reqProp[C2CLIENT_GET_REQUEST_Transfer_Delay];

	_sendMap[C2CLIENT_GET_REQUEST_Ingress_Capacity] = reqProp[C2CLIENT_GET_REQUEST_Ingress_Capacity].empty() \
		? "16512000000" : reqProp[C2CLIENT_GET_REQUEST_Ingress_Capacity];

	//set header
	//setHeader(C2CLIENT_HOST, (char*)_sendMap[C2CLIENT_HOST].c_str());
	setHeader(C2CLIENT_Range, (char*)_sendMap[C2CLIENT_Range].c_str());
	setHeader(C2CLIENT_UserAgent, (char*)_sendMap[C2CLIENT_UserAgent].c_str());
	setHeader(C2CLIENT_HOST, (char*)_sendMap[C2CLIENT_HOST].c_str());
	setHeader(C2CLIENT_GET_REQUEST_Transfer_Delay, (char*)_sendMap[C2CLIENT_GET_REQUEST_Transfer_Delay].c_str());
	setHeader(C2CLIENT_GET_REQUEST_Ingress_Capacity, (char*)_sendMap[C2CLIENT_GET_REQUEST_Ingress_Capacity].c_str());

	//connect
	int ret = httpConnect(_sendMap[C2CLIENT_URI].c_str(),  HTTP_GET);
	int64 connectEndTime = ZQ::common::TimeUtil::now();
	int64 connTime  = connectEndTime - startTime;
	if (_timeout < connTime)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "timeout when connect via uri[%s]"), _sendMap[C2CLIENT_URI].c_str());
		return false;
	}
	else if (ret)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "failed to connect via uri[%s]"), _sendMap[C2CLIENT_URI].c_str());
		return false;
	}
	setSendTimeout(-(_timeout - connTime)*1000);

	ret = httpEndSend();
	int64 endSendTime = ZQ::common::TimeUtil::now();
	int64 sendTime = endSendTime - startTime;
	if (_timeout < sendTime)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "timeout when complete sending body data to server"));
		return false;
	}else if (ret)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "failed to complete sending body data to server"));
		return false;
	}
	setRecvTimeout(-(_timeout - sendTime)*1000);

	//receive
	ret = httpBeginRecv();
	int64 recvContentTime = ZQ::common::TimeUtil::now();
	int recvTime = recvContentTime - startTime;
	if (_timeout < recvTime)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "timeout when recv data from server"));
		return false;
	}else if (ret)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "failed to recv data from server"));
		return false;
	}

	while(!isEOF())
	{
		ret = httpContinueRecv();
		int64 endRecvTime = ZQ::common::TimeUtil::now();
		recvTime = endRecvTime - startTime;
		if (_timeout < recvTime)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "timeout when recv data from server"));
			return false;
		}else if (ret)
		{
			_log(ZQ::common::Log::L_ERROR,CLOGFMT(C2ClientSync,"failed to received data from server with errorCode[%d]"),
				getErrorcode());
		}
		break;
	}

	int64 endRecvTime = ZQ::common::TimeUtil::now();
	recvTime = endRecvTime - startTime;
	if (_timeout < recvTime)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "timeout when receive data"));
		return false;
	}

	int statusCode = getStatusCode();

	if (200 != statusCode)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "status code is not 201 or 206, but %d, %s"), statusCode, getMsg());		
		return false;
	}

	std::string responseMsg;
	getContent(responseMsg);
	//int msgLen = responseMsg.size();
	if (responseMsg.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "content is empty"));		
		return false;
	}

	ZQ::IdxParser::IndexData idxData;
	bool result = parseIndex(contentName, responseMsg.c_str(), responseMsg.size(), idxData);
	if (result)
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ClientSync, "parse index success"));
		
		//set response map
		respProp[C2CLIENT_AssetID] = contentName.substr(0, 20);
		respProp[C2CLIENT_ProviderID] = contentName.substr(20);

		std::ostringstream playTimeOSS;
		playTimeOSS<<idxData.getPlayTime();
		if (!playTimeOSS.str().empty())
		{
			respProp[C2CLIENT_PlayTime]			=	playTimeOSS.str();
		}

		std::ostringstream muxBitRateOSS;
		muxBitRateOSS<<idxData.getMuxBitrate();
		if (!muxBitRateOSS.str().empty())
		{
			respProp[C2CLIENT_MuxBitrate]		=	muxBitRateOSS.str();
		}

		std::string extName = idxData.getSubFileName(0);
		if (!extName.empty())
		{
			size_t dotPos = extName.find_first_of('.');
			if (dotPos == extName.npos)
			{
				respProp[C2CLIENT_ExtName]			=	extName;
			}
			else{
				respProp[C2CLIENT_ExtName]			=	extName.substr(dotPos + 1);
			}
		}
		
		ZQ::IdxParser::IndexData::SubFileInformation info;
		if (idxData.getSubFileInfo(0, info))
		{
			std::ostringstream startOffsetOSS;
			uint64 startOffset =info.startingByte;
			startOffsetOSS<<startOffset;
			if (!startOffsetOSS.str().empty())
			{
				respProp[C2CLIENT_StartOffset]		=	startOffsetOSS.str();
			}

			std::ostringstream endOffsetOSS;
			uint64 endOffset   = info.endingByte;
			endOffsetOSS<<endOffset;
			if (!endOffsetOSS.str().empty())
			{
				respProp[C2CLIENT_EndOffset]		=	endOffsetOSS.str();
			}
		}
	}
	else
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "parse index failure"));
		return false;
	}

	uninit();
	return true;
}

int  C2ClientSync::setLocateRequestBody()
{
	std::ostringstream sendBodyMsg;

	sendBodyMsg << "<LocateRequest>" << "\r\n";
	sendBodyMsg << "<Object>" << "\r\n";
	sendBodyMsg << "<Name>" << "\r\n";

	if (_sendMap[C2CLIENT_AssetID].empty()) return 0;
	sendBodyMsg << "<AssetID>" << _sendMap[C2CLIENT_AssetID] << "</AssetID>" << "\r\n";

	if (_sendMap[C2CLIENT_ProviderID].empty()) return 0;
	sendBodyMsg << "<ProviderID>" << _sendMap[C2CLIENT_ProviderID] << "</ProviderID>" << "\r\n";

	sendBodyMsg << "</Name>" << "\r\n";

	if (_sendMap[C2CLIENT_SubType].empty()) return 0;
	sendBodyMsg << "<SubType>" << _sendMap[C2CLIENT_SubType] << "</SubType>" << "\r\n";

	sendBodyMsg << "</Object>" << "\r\n";

	if (_sendMap[C2CLIENT_TransferRate].empty()) return 0;
	sendBodyMsg << "<TransferRate>" << _sendMap[C2CLIENT_TransferRate] << "</TransferRate>" << "\r\n";

	if (_sendMap[C2CLIENT_IngressCapacity].empty()) return 0;
	sendBodyMsg << "<IngressCapacity>" << _sendMap[C2CLIENT_IngressCapacity] << "</IngressCapacity>" << "\r\n";

	if (_sendMap[C2CLIENT_ClientTransfer].empty()) return 0;
	sendBodyMsg << "<ClientTransfer>" << _sendMap[C2CLIENT_ClientTransfer] << "</ClientTransfer>" << "\r\n";

	//if (_sendMap[C2CLIENT_LOCATE_REQUEST_ExclusionList].empty()) return 0;
	sendBodyMsg << "<ExclusionList>" << _sendMap[C2CLIENT_ExclusionList] << "</ExclusionList>" << "\r\n";

	//if (_sendMap[C2CLIENT_LOCATE_REQUEST_Range].empty()) return 0;
	sendBodyMsg << "<Range>" << _sendMap[C2CLIENT_Range] << "</Range>" << "\r\n";

	//if (_sendMap[C2CLIENT_LOCATE_REQUEST_TransferDelay].empty()) return 0;
	sendBodyMsg << "<TransferDelay>" << _sendMap[C2CLIENT_TransferDelay] << "</TransferDelay>" << "\r\n";

	sendBodyMsg << "</LocateRequest>";

	/*char len[8];
	itoa(sendBodyMsg.str().size(), len, 10);
	std::string strLength = len;*/

	return httpSendContent(sendBodyMsg.str().c_str(), sendBodyMsg.str().size());
}

int  C2ClientSync::setGetRequestBody()
{
	return 0;
}

int64 C2ClientSync::setTimeout(int64 timeout)
{
	_timeout = timeout;
	return _timeout;
}

bool C2ClientSync::parseResponse(const SimpleXMLParser::Node* root, LocateResponseData& respData, std::string& error)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ClientSync, "parseResponse() entry"));

	typedef SimpleXMLParser::Node Node;
	// step 1: check the content and extract the response data
	const Node* locNode = findNode(root, "LocateResponse");
	if(NULL == locNode)
	{ // bad xml content
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "parseResponse() XML element missed: <LocateResponse>"));
		return false;
	}

	const Node* transferPortNode = findNode(locNode, "TransferPort");
	if(!transferPortNode)
	{ // parameter missed
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "parseResponse() XML element missed: <TransferPort>"));
		return false;
	}
	respData.transferPort = transferPortNode->content;

	const Node* transferIDNode = findNode(locNode, "TransferID");
	if (!transferIDNode)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "parseResponse() XML element missed: <TransferID>"));
		return false;
	}
	respData.transferId = transferIDNode->content;

	const Node* transferTimeoutNode = findNode(locNode, "TransferTimeout");
	if (!transferTimeoutNode)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "parseResponse() XML element missed: <TransferTimeout>"));
		return false;
	}
	respData.transferTimeout = StringToLong(transferTimeoutNode->content);

	const Node* availableRangeNode = findNode(locNode, "AvailableRange");
	if (!availableRangeNode)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "parseResponse() XML element missed: <AvailableRange>"));
		return false;
	}
	respData.availableRange = availableRangeNode->content;

	const Node* openForWriteNode = findNode(locNode, "OpenForWrite");
	if (!openForWriteNode)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "parseResponse() XML element missed: <OpenForWrite>"));
		return false;
	}
	respData.openForWrite = openForWriteNode->content;

	const Node* portNumNode = findNode(locNode, "PortNum");
	if (portNumNode)
	{
		//_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "parseResponse() XML element missed: <PortNum>"));
		//return false;
		respData.portNum = portNumNode->content;
	}
	
	const Node* residentialNode = findNode(locNode, "ClipInfo/Residential");
	if (residentialNode)
	{
		//_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "parseResponse() XML element missed: <Residential>"));
		//return false;
		std::map<std::string, std::string> residentialNodeAttrs = residentialNode->attrs;
		respData.recording = residentialNodeAttrs["recording"];
	}
	
	const Node* encodingInfoNode = findNode(locNode, "ClipInfo/EncodingInfo");
	if (encodingInfoNode)
	{
		/*_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "parseResponse() XML element missed: <EncodingInfo>"));
		return false;*/

		std::map<std::string, std::string> encodingInfoNodeAttrs = encodingInfoNode->attrs;
		respData.playTime = encodingInfoNodeAttrs["playTime"];
		respData.muxBitrate = encodingInfoNodeAttrs["muxBitrate"];
	}
	
	const Node* membersNode = findNode(locNode, "ClipInfo/Members");
	if (membersNode)
	{
		//_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "parseResponse() XML element missed: <MembersNode>"));
		//return false;

		std::list<Node>::const_iterator it = membersNode->children.begin();
		if (it == membersNode->children.end())
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "parseResponse() ClipInfo/Members is empty "));
			return false;
		}

		std::map<std::string, std::string> subFileAttrs = it->attrs;
		std::string ext = subFileAttrs["extName"];

		respData.extName = ext;
		respData.startOffset = subFileAttrs["startOffset"];
		respData.endOffset = subFileAttrs["endOffset"];
	}
	
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ClientSync, "parseResponse() parse response success"));
	return true;
}

bool C2ClientSync::setRevcMap(LocateResponseData& respData)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ClientSync, "setRevcMap() entry"));

	if (!&respData)
	{
		return false;
	}

	_recvMap[C2CLIENT_TransferPort]		=	respData.transferPort;
	_recvMap[C2CLIENT_TransferID]		=	respData.transferId;
	_recvMap[C2CLIENT_PortNum]			=	respData.portNum;

	char timeout[8];
	itoa(respData.transferTimeout, timeout, 10);
	std::string strTimeout = timeout;
	_recvMap[C2CLIENT_TransferTimeout]	=	timeout;

	_recvMap[C2CLIENT_AvailableRange]	=	respData.availableRange;
	_recvMap[C2CLIENT_OpenForWrite]		=	respData.openForWrite;

	if (!respData.recording.empty())
	{
		_recvMap[C2CLIENT_Recording]		=	respData.recording;
	}
	
	if (!respData.playTime.empty())
	{
		_recvMap[C2CLIENT_PlayTime]			=	respData.playTime;
	}

	if (!respData.muxBitrate.empty())
	{
		_recvMap[C2CLIENT_MuxBitrate]		=	respData.muxBitrate;
	}

	if (!respData.extName.empty())
	{
		_recvMap[C2CLIENT_ExtName]			=	respData.extName;
	}

	if (!respData.startOffset.empty())
	{
		_recvMap[C2CLIENT_StartOffset]		=	respData.startOffset;
	}

	if (!respData.endOffset.empty())
	{
		_recvMap[C2CLIENT_EndOffset]		=	respData.endOffset;
	}

	return true;
}

bool C2ClientSync::parseIndex(std::string contentName, const char* indexData, size_t dataSize, ZQ::IdxParser::IndexData& idxData)
{
	ZQ::IdxParser::IdxParserEnv			idxParserEnv;
	idxParserEnv.AttchLogger(&_log);
	ZQ::IdxParser::IndexFileParser		idxParser(idxParserEnv);

	if(!idxParser.ParseIndexFromMemory( contentName, idxData, indexData, dataSize ) ) 
	{
		_log(ZQ::common::Log::L_ERROR,CLOGFMT(C2ClientSync,"parseIndex() failed to parse index data for[%s], data size[%u]"),
			contentName.c_str(), (uint32)dataSize);
		return false;
	}
	return true;
}
} // namespace StreamService
} // namespace ZQ