#include "TransferDelete.h"
#include <TimeUtil.h>

namespace ZQ{
namespace StreamService{
    ZQ::common::AtomicInt TransferDelete::_atomicID;

    TransferDelete::TransferDelete(ZQ::common::Log& log, IC2RequestCallBack::Ptr requestCB, RequestParams params, std::string instanceKey, int64 bufferReqID)
        : RequestHandle(log, requestCB, params, instanceKey, bufferReqID), LibAsync::Socket(LibAsync::getLoopCenter().getLoop())
    {
        
    }

    TransferDelete::~TransferDelete()
    {
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(TransferDelete, "[%s] bufReqID[%ld] transfer delete request ended, release resource"),
            _instanceKey.c_str(), _bufferReqID);
    }

    bool TransferDelete::process()
    {
        _status = ZQ::StreamService::NotConnected;

        std::string host		= "None";
        std::string userAgent	= "ToInfinityAndBeyond";
        std::string contentType = "text/xml-external-parsed-entity";

        LibAsync::HttpMessagePtr transferDeleteMsgPtr = new LibAsync::HttpMessage(HTTP_REQUEST);
        transferDeleteMsgPtr->method(HTTP_POST);
        transferDeleteMsgPtr->url("*");

        //add header
        transferDeleteMsgPtr->header(C2CLIENT_HOST, host);
        transferDeleteMsgPtr->header(C2CLIENT_UserAgent, userAgent);
        transferDeleteMsgPtr->header(C2CLIENT_ContentType, contentType);

        _reqBody = generateTransferDeleteBody(_params.transferID);
        transferDeleteMsgPtr->contentLength(_reqBody.size());

        _startTime = ZQ::common::TimeUtil::now();

        //bind up stream ip
        if (!bind(_params.upstreamIP, 0))
        {
            _status = ZQ::StreamService::Failure;
#ifdef ZQ_OS_LINUX
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(TransferDelete, "[%s/%s] bufReqID[%ld] process(), bind up stream ip[%s] failed, error[%d:%s]"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, _params.upstreamIP.c_str(), errno, strerror(errno));
#else
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(TransferDelete, "[%s/%s] bufReqID[%ld] process(), bind up stream ip[%s] failed"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, _params.upstreamIP.c_str());
#endif
            _cb->onError(crGeneric, -11, "bind up stream ip failed");
            if( _cb ) _cb = NULL;
            return false;
        }

        std::string localIP;
        unsigned short localPort;
        if (getLocalAddress(localIP, localPort))
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(TransferDelete, "[%s/%s] bufReqID[%ld] local socket info[%s:%d] transfer delete address[%s:%d]"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, localIP.c_str(), localPort, _params.locateIP.c_str(), _params.locatePort);
        }else{
            _log(ZQ::common::Log::L_WARNING, CLOGFMT(TransferDelete, "[%s/%s] bufReqID[%ld] get local address failed"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
        }

        if ("index" == _params.subType)
        {
            setTimeout(_params.indexTimeout);
        }else{
            setTimeout(_params.mainfileTimeout);
        }

        if (!checkTimeout())
        {
            return false;
        }

        _bBodySend = false;
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(TransferDelete, "[%s] bufReqID[%ld] process(), begin send transfer delete request for[%s] to [%s:%d]"), _instanceKey.c_str(), _bufferReqID, _params.transferID.c_str(), _params.locateIP.c_str(), _params.locatePort);
        if (!beginRequest(transferDeleteMsgPtr, _params.locateIP, _params.locatePort))
        {
            _status = ZQ::StreamService::Failure;
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(TransferDelete, "[%s] bufReqID[%ld] process(), failed to send transfer delete request to [%s:%d]"), _instanceKey.c_str(), _bufferReqID, _params.locateIP.c_str(), _params.locatePort);
            if (_cb)    
            {
                _cb->onError(crGeneric, -12, "send get request message failed");
            }
            if( _cb ) _cb = NULL;
            return false;
        }
        return true;
    }

    std::string TransferDelete::generateTransferDeleteBody(const std::string& transferID)
    {
        std::ostringstream sendTransferDeleteBody;

        sendTransferDeleteBody << "<LocateRequest>" << "\r\n";

        if (_params.clientTransfer.empty()) return 0;
        sendTransferDeleteBody << "<ClientTransfer>" << _params.clientTransfer << "</ClientTransfer>" << "\r\n";

        if (transferID.empty()) return 0;
        sendTransferDeleteBody << "<TransferIDDelete>" << transferID << "</TransferIDDelete>" << "\r\n";

        sendTransferDeleteBody << "</LocateRequest>";

        return sendTransferDeleteBody.str();
    }

    bool TransferDelete::onHttpBody( const char* data, size_t size)
    {
        if (ZQ::StreamService::Timeout == _status) return false;
        _respBody.append(data, size);
        return true;
    }

    void TransferDelete::onHttpDataReceived( size_t size )
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

            //_log(ZQ::common::Log::L_DEBUG, CLOGFMT(TransferDelete, "[%s] onHttpDataReceived() current request received data[%d], continue"), _instanceKey.c_str(), size);
            _status = ZQ::StreamService::Receiving;
        }
        else{
            int statusCode = _headerMsg->code();
            if (2 != statusCode/100 && 404 != statusCode)
            {
                _log(ZQ::common::Log::L_ERROR, CLOGFMT(TransferDelete, "[%s] bufReqID[%ld] onHttpDataReceived(), send locate request failure, error[%d:%s]"), _instanceKey.c_str(), _bufferReqID, statusCode, _headerMsg->status().c_str());		
                _cb->onError(crHttpError, statusCode, _headerMsg->status());
                if( _cb ) _cb = NULL;
                return;
            }

            _cb->onTransferDelete();
        }
    }
}}