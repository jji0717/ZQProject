#include "RequestHandle.h"
#include <TimeUtil.h>

namespace ZQ
{
	namespace StreamService
	{
        std::string getStatusStr(TransferStatus stat)
        {
            switch(stat)
            {
            case Initialize:
                return "Initialize";
            case NotConnected:
                return "NotConnected";
            case Connected:
                return "Connected";
            case HeaderSendCompleted:
                return "HeaderSendCompleted";
            case BodySendCompleted:
                return "BodySendCompleted";
            case Receiving:
                return "Receiving";
            case WaitBuffer:
                return "WaitBuffer";
            case Completed:
                return "Completed";
            case Failure:
                return "Failure";
            default:
                return "Timeout";
            }
        }

        RequestHandle::RequestHandle(ZQ::common::Log& log, IC2RequestCallBack::Ptr cb, RequestParams params, std::string instanceKey, int64 bufferReqID)
            : LibAsync::Socket(LibAsync::getLoopCenter().getLoop()),LibAsync::HttpClient(), LibAsync::Timer(getLoop()), _instanceKey(instanceKey), _log(log), _cb(cb), _params(params),
            _timeout(0), _startTime(0), _startCalcLatency(0), _bIndex(false), _bPrintConnInfo(true),
            _reqBody(""), _respBody(""), _status(ZQ::StreamService::Initialize), _bBodySend(false), _bufferReqID(bufferReqID), _xSessionID("")
        {
            for (int i=0; i<8; i++)
            {
                LibAsync::AsyncBuffer buf;
                buf.len = 8* 1024;
                buf.base = (char*)malloc(sizeof(char)* buf.len);
                _recvBufs.push_back(buf);
            }
        }

        RequestHandle::~RequestHandle()
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(RequestHandle, "[%s/%s] bufReqID[%ld] release resource"),
                _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
            LibAsync::AsyncBufferS::iterator it = _recvBufs.begin();
            for (; it != _recvBufs.end(); it++)
            {
                free(it->base);
                it->base = NULL;
            }
			if(_cb)
				_cb = NULL;
        }

        int64 RequestHandle::setTimeout(int64 timeout)
        {
            _timeout = timeout;
            return _timeout;
        }

        bool RequestHandle::checkTimeout()
        {
            int64 now = ZQ::common::TimeUtil::now();
            int64 remainTime = _timeout - (now - _startTime);

            if (remainTime > 0)
            {
                _startTime = ZQ::common::TimeUtil::now();
                update(_timeout);
                return true;
            }
            else{
                onTimer();
                return false;
            }
        }

        void RequestHandle::onReqMsgSent( size_t size)
        {
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(RequestHandle, "[%s/%s] bufReqID[%ld] entry onReqMsgSent(), send size[%ld]"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID,  size);

            if (ZQ::StreamService::Timeout == _status) 
				return;

            if (_bPrintConnInfo)
            {
                std::string localIP, peerIP;
                unsigned short localPort, peerPort;
                if (getLocalAddress(localIP, localPort) && getPeerAddress(peerIP, peerPort))
                {
                    _log(ZQ::common::Log::L_DEBUG, CLOGFMT(RequestHandle, "[%s/%s] bufReqID[%ld] connection info[%s:%d => %s:%d]"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, localIP.c_str(), localPort, peerIP.c_str(), peerPort);
                    _bPrintConnInfo = false;
                }
            }

            _status = ZQ::StreamService::HeaderSendCompleted;

            cancel();

            if(!_bBodySend && !_reqBody.empty()) {
                LibAsync::AsyncBuffer buf;
                buf.base = (char*)_reqBody.c_str();
                buf.len = _reqBody.length();
                _bBodySend = true;

                //check if timeout
                if(!checkTimeout())
                {
                    return;
                }

                sendReqBody(buf);
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(RequestHandle, "[%s/%s] bufReqID[%ld] start send body"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
            } else if(_bBodySend && !_reqBody.empty()) {
                _reqBody.clear();
                //mRequest = NULL;
                //check if timeout
                if(!checkTimeout())
                {
                    return;
                }
                endRequest();
                _startCalcLatency = ZQ::common::TimeUtil::now();
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(RequestHandle, "[%s/%s] bufReqID[%ld] end send body"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
            } else {
                _status = ZQ::StreamService::BodySendCompleted;
                //check if timeout
                if(!checkTimeout())
                {
                    return;
                }
                getResponse();
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(RequestHandle, "[%s/%s] bufReqID[%ld] get response"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
            }
        }

        bool RequestHandle::onHttpMessage( const LibAsync::HttpMessagePtr msg)
        {
            if (ZQ::StreamService::Timeout == _status) return false;
            _headerMsg = msg;
            _xSessionID = _headerMsg->header("X-SessionId");

            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(RequestHandle, "[%s/%s] bufReqID[%ld] onHttpMessage(), received http header"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
            return true;
        }

        void RequestHandle::onHttpComplete()
        {
            if (ZQ::StreamService::Timeout == _status) return;
            _log(ZQ::common::Log::L_DEBUG, CLOGFMT(RequestHandle, "[%s/%s] bufReqID[%ld] onHttpComplete() entry"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID);
            _status = ZQ::StreamService::Completed;
        }

        void RequestHandle::onHttpError( int error )
        {
            if (ZQ::StreamService::Timeout == _status) return;
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(RequestHandle, "[%s/%s] bufReqID[%ld] onHttpError() error[%d]"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, error);
            cancel();

            _status = ZQ::StreamService::Failure;
            if (_cb)
            {
                _cb->onError(crSocketError, error, "");
                _cb  = NULL;
            }
        }

        void RequestHandle::onTimer()
        {
            if (ZQ::StreamService::Timeout == _status) return;
            cancel();
            std::string status = ZQ::StreamService::getStatusStr(_status);
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(RequestHandle, "[%s/%s] bufReqID[%ld] timeout, status[%s],timeout[%d]"), _instanceKey.c_str(), _xSessionID.c_str(), _bufferReqID, status.c_str(), _timeout);

            _status = ZQ::StreamService::Timeout;
            if (_cb)
            {
                _cb->onError(crTimeout, -4, "timeout");
                _cb = NULL;
            }
        }
	}
}
