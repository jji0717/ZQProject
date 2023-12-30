#include "GetRequest.h"
#include <TimeUtil.h>

namespace ZQ{
namespace StreamService{
    ZQ::common::AtomicInt GetRequest::_atomicID;

    GetRequest::GetRequest(ZQ::common::Log& log, IC2RequestCallBack::Ptr requestCB, RequestParams params, std::string instanceKey, int64 bufferReqID)
        : RequestHandle(log, requestCB, params, instanceKey, bufferReqID), 
		LibAsync::Socket(LibAsync::getLoopCenter().getLoop()),
        _dataSize(0), 
        _bWaitBuffer(false), 
        _receivedDataSize(0)
    {
    }

    GetRequest::~GetRequest()
    {
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(GetRequest, "[%s/%s] bufReqID[%ld] get request ended, release resource"),
            _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
    }

    void GetRequest::waitBuffer(int64 timeout)
    {
        _bWaitBuffer = true;
        setTimeout(timeout);
    }

    void GetRequest::continueRecv()
    {
        if (ZQ::StreamService::Timeout == _status) return;
        //ZQ::common::MutexGuard gd(_mutex);
        recvRespBody(_recvBufs);
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(GetRequest, "[%s/%s] bufReqID[%ld] continueRecv() continue to recv data buffer."), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
        _status = ZQ::StreamService::Receiving;
    }

    bool GetRequest::process()
    {
        _status = ZQ::StreamService::NotConnected;
        _reqBody  = "";
        _respBody = "";

        if (_params.transferID.empty())
        {
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(GetRequest, "[%s/%s] bufReqID[%ld] process(), url is empty"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
            _status = ZQ::StreamService::Failure;
            _cb->onError(crGeneric, -10, "TransferID is empty");
            if( _cb ) _cb = NULL;
            return false;
        }

        LibAsync::HttpMessagePtr getMsgPtr = new LibAsync::HttpMessage(HTTP_REQUEST);
        getMsgPtr->method(HTTP_GET);

        std::string strUrl = _params.transferID;
        if (strUrl.at(0) != '/')
        {
            strUrl = "/" + strUrl;
        }
        getMsgPtr->url(strUrl);

        getMsgPtr->header(C2CLIENT_Range, _params.props[C2CLIENT_Range]);
        getMsgPtr->header(C2CLIENT_UserAgent, _params.props[C2CLIENT_UserAgent]);
        getMsgPtr->header(C2CLIENT_HOST, _params.props[C2CLIENT_HOST]);
        getMsgPtr->header("Transfer-Delay", _params.props[C2CLIENT_TransferDelay]);
        getMsgPtr->header("Ingress-Capacity", _params.props[C2CLIENT_IngressCapacity]);

        _startTime = ZQ::common::TimeUtil::now();

        //bind up stream ip
        if (!bind(_params.clientTransfer, 0))
        {
            _status = ZQ::StreamService::Failure;
#ifdef ZQ_OS_LINUX
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(GetRequest, "[%s/%s] bufReqID[%ld] process(), bind up stream ip[%s] failed, error[%d:%s]"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, _params.upstreamIP.c_str(), errno, strerror(errno));
#else
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(GetRequest, "[%s/%s] bufReqID[%ld] process(), bind up stream ip[%s] failed"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, _params.upstreamIP.c_str());
#endif
            _cb->onError(crGeneric, -11, "bind up stream ip failed");
            if( _cb ) _cb = NULL;
            return false;
        }

        std::string localIP;
        unsigned short localPort;
        if (getLocalAddress(localIP, localPort))
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(GetRequest, "[%s/%s] bufReqID[%ld] local socket info[%s:%d] get address[%s:%d]"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, localIP.c_str(), localPort, _params.getAddr.c_str(), _params.getPort);
        }else{
            _log(ZQ::common::Log::L_WARNING, CLOGFMT(GetRequest, "[%s/%s] bufReqID[%ld] get local address failed"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
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
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(GetRequest, "[%s/%s] bufReqID[%ld] process(), begin send get request endpoint[%s:%d]"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, _params.getAddr.c_str(), _params.getPort);
        if (!beginRequest(getMsgPtr, _params.getAddr, _params.getPort))
        {
            _status = ZQ::StreamService::Failure;
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(GetRequest, "[%s/%s] bufReqID[%ld] process(), failed to send get request to [%s:%d]"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, _params.getAddr.c_str(), _params.getPort);
            if (_cb)    
            {
                _cb->onError(crGeneric, -12, "send get request message failed");
            }
            if( _cb ) _cb = NULL;
            return false;
        }

        return true;
    }

    bool GetRequest::onHttpBody( const char* data, size_t size)
    {
        if (ZQ::StreamService::Timeout == _status) return false;
        _receivedDataSize += size;
        size_t currBufLeftSize = RECV_BUFFER_SIZE - _dataSize;

        if (currBufLeftSize > size)
        {
            char *startWritePoint = _data + _dataSize;
            memcpy(startWritePoint, data, size);
            _dataSize += size;
        }else{
            //the free space less than received data size
            size_t leftDataSize = size;
            size_t allCopySize = 0;
            while (leftDataSize > 0)
            {
                char *startDstPoint = _data + _dataSize;
                char *startSrcPoint = (char*)data + allCopySize;

                size_t copySize;
                if (leftDataSize > currBufLeftSize)
                {
                    copySize = currBufLeftSize;
                    currBufLeftSize = 0;
                }else{
                    copySize = leftDataSize;
                    currBufLeftSize = currBufLeftSize - copySize;
                }

                memcpy(startDstPoint, startSrcPoint, copySize);
                _dataSize += copySize;

                leftDataSize = leftDataSize - copySize;
                allCopySize += copySize;

                if (currBufLeftSize == 0)
                {
                    // return _instanceKey.c_str(), _xSessionID.c_str() buffer and fill next buffer
                    _cb->onData(_data, _dataSize);
                    memset(_data, 0, _dataSize);
                    _dataSize = 0;


                    if (leftDataSize > 0)
                    {
                        currBufLeftSize = RECV_BUFFER_SIZE - _dataSize;
                    }
                }
            }
        }
        return true;
    }

    void GetRequest::onHttpDataReceived( size_t size )
    {
        if (ZQ::StreamService::Timeout == _status) return;
        cancel();
        if (ZQ::StreamService::Completed != _status)
        {
            //check if timeout
            if(!checkTimeout()) return;

            if (!_bWaitBuffer){
                recvRespBody(_recvBufs);
                //_log(ZQ::common::Log::L_DEBUG, CLOGFMT(GetRequest, "[%s] onHttpDataReceived() current request received data[%ld]"), _instanceKey.c_str(), _xSessionID.c_str(), _receivedDataSize);
                //_log(ZQ::common::Log::L_DEBUG, CLOGFMT(GetRequest, "[%s] onHttpDataReceived() current request received data[%d], continue"), _instanceKey.c_str(), _xSessionID.c_str(), size);
                _status = ZQ::StreamService::Receiving;
            }else{
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(GetRequest, "[%s/%s] bufReqID[%ld] onHttpDataReceived() no enough buffer, wait for new buffer"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
                _status = ZQ::StreamService::WaitBuffer;
                update(_timeout);
            }
        }
        else{
            // received all data
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(GetRequest, "[%s/%s] bufReqID[%ld] onHttpDataReceived() received all data[%ld]."), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, _receivedDataSize);
            _cb->onData(_data, _dataSize);
            _cb->onRecvComplete();
        }
    }

    void GetRequest::onHttpError( int error )
    {
        if (ZQ::StreamService::Timeout == _status) return;
        _cb->onData(_data, _dataSize, true);
        RequestHandle::onHttpError(error);
    }

    void GetRequest::onTimer()
    {
        if (ZQ::StreamService::Timeout == _status) return;
        cancel();

        ZQ::StreamService::TransferStatus preStatus = _status;
        _status = ZQ::StreamService::Timeout;

        std::string status = ZQ::StreamService::getStatusStr(preStatus);
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(GetRequest, "[%s/%s] bufReqID[%ld] timeout, status[%s],timeout[%d]"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, status.c_str(), _timeout);

        _cb->onData(_data, _dataSize, true);
        if (_status == ZQ::StreamService::WaitBuffer)
        {
            _cb->onError(crWaitBufferTimeout, -5, "wait buffer timeout");
        }else{
            _cb->onError(crTimeout, -4, "timeout");
        }
        if (_cb)
        {
            _cb = NULL;
        }
    }
}}