#ifndef _C2_REQUEST_HANDLE_H
#define _C2_REQUEST_HANDLE_H

#include <ZQ_common_conf.h>
#include <libasync/http.h>
#include <HttpClient.h>
#include "RequestInc.h"

#ifdef ZQ_OS_LINUX
#include <errno.h>
#endif

#define RECV_BUFFER_SIZE 1024*1024  // need to be configurable

namespace ZQ
{
	namespace StreamService
	{
        typedef std::map<std::string, std::string> AttrMap;

        enum C2RequestErrorCategory
        {
            crGeneric,
            crSocketError,
            crHttpError,
            crParseXmlDataError,
            crTimeout,
            crWaitBufferTimeout
        };

        enum TransferStatus{
            Initialize,
            NotConnected,
            Connected,
            HeaderSendCompleted,
            BodySendCompleted,
            Receiving,
            WaitBuffer,
            Completed,
            Failure,
            Timeout
        };

        struct RequestParams{
            std::string  locateIP;
            unsigned int locatePort;
            std::string  url;
            std::string  upstreamIP;
            std::string  clientTransfer;
            unsigned int defaultGetPort;
            std::string  transferRate;
            std::string  ingressCapacity;
            std::string  exclusionList;
            std::string  transferDelay;
            std::string  filename;
            std::string  contentName;
            std::string  subType;
            std::string  range;
            int          alignment;

            std::string  transferID;
            std::string  getAddr;
            uint         getPort;

            int          waitBufferTime;
            int          indexTimeout;
            int          indexRetryTimes;
            int          mainfileTimeout;
            int          mainfileRetryTimes;

            bool         transferDelete;

            AttrMap      props;

            RequestParams()
            {
                locateIP            = "";
                locatePort          = 10080;         
                url                 = "";
                upstreamIP          = "";
                clientTransfer      = "";
                defaultGetPort      = 12000;
                transferRate        = "3750000";
                ingressCapacity     = "16512000000";
                exclusionList       = "";
                transferDelay       = "-2000";
                filename            = "";
                contentName         = "";
                subType             = "";
                range               = "";
                alignment           = 4;

                transferID          = "";
                getAddr             = "";
                getPort             = 0;

                waitBufferTime      = 10*1000;
                indexTimeout        = 5*1000;
                indexRetryTimes     = 5;
                mainfileTimeout     = 15*1000;
                mainfileRetryTimes  = 5;
                transferDelete      = false;
            }

            void operator=(const RequestParams& rhs)
            {
                locateIP            = rhs.locateIP;
                locatePort          = rhs.locatePort;         
                url                 = rhs.url;
                upstreamIP          = rhs.upstreamIP;
                clientTransfer      = rhs.clientTransfer;
                defaultGetPort      = rhs.defaultGetPort;
                transferRate        = rhs.transferRate;
                ingressCapacity     = rhs.ingressCapacity;
                exclusionList       = rhs.exclusionList;
                transferDelay       = rhs.transferDelay;
                filename            = rhs.filename;
                contentName         = rhs.contentName;
                subType             = rhs.subType;
                range               = rhs.range;
                alignment           = rhs.alignment;

                waitBufferTime      = rhs.waitBufferTime;
                indexTimeout        = rhs.indexTimeout;
                indexRetryTimes     = rhs.indexRetryTimes;
                mainfileTimeout     = rhs.mainfileTimeout;
                mainfileRetryTimes  = rhs.mainfileRetryTimes;
                transferDelete      = rhs.transferDelete;
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
            std::string recording;
            std::string startOffset;
            std::string endOffset;
            std::string extName;
            std::string playTime;
            std::string muxBitrate;
            long		transferTimeout;

            std::string reqSubType;//not for response output
            int32		exposeAssetIndexData;
            LocateResponseData():transferTimeout(-1),exposeAssetIndexData(0) {}
        };

        extern std::string getStatusStr(TransferStatus stat);

        class IC2RequestCallBack : virtual public ZQ::common::SharedObject
		{
		public:
            typedef ZQ::common::Pointer<IC2RequestCallBack> Ptr;

            virtual ~IC2RequestCallBack() {};
		    virtual void onLocate(const LocateResponseData& resp) = 0;
		    virtual void onData(const char* data, const size_t& size, bool error = false) = 0;
            virtual void onRecvComplete() = 0;
		    virtual void onTransferDelete() = 0;
		    virtual void onError(C2RequestErrorCategory category, const int& err, const std::string& msg) = 0;
		};
		
		class RequestHandle : public LibAsync::HttpClient, public LibAsync::Timer
		{
        protected:
            RequestHandle(ZQ::common::Log& log, IC2RequestCallBack::Ptr cb, RequestParams params, std::string instanceKey, int64 bufferReqID);

        private:
            RequestHandle( const RequestHandle& );
            RequestHandle& operator=( const RequestHandle&);

		public:
            typedef ZQ::common::Pointer<RequestHandle> Ptr;
		    virtual ~RequestHandle();

        public:
            virtual bool process(){ return true; }
            virtual void waitBuffer(int64 timeout){}

        protected:
            virtual bool checkTimeout();
            int64 setTimeout(int64 timeout);
		
		public:	
		    virtual void onReqMsgSent( size_t size);
		    virtual bool onHttpMessage( const LibAsync::HttpMessagePtr msg);
		    virtual void onHttpComplete();
		    virtual void onHttpError( int error );	
		    virtual void onTimer();

            // need implement by derived class
            virtual bool onHttpBody( const char* data, size_t size) { return true;}
            virtual void onHttpDataReceived( size_t size ) { }
		
        public:
            std::string         _instanceKey;
		    ZQ::common::Log&    _log;
            uint64              _timeout;
            int64               _startTime;
            int64               _startCalcLatency;
            bool                _bIndex;
            bool                _bPrintConnInfo;

            LibAsync::AsyncBufferS		_recvBufs;
            LibAsync::HttpMessagePtr	_headerMsg;
            std::string					_reqBody;
            std::string					_respBody;
            TransferStatus				_status;
            bool						_bBodySend;

            RequestParams               _params;
            IC2RequestCallBack::Ptr     _cb;
            int64                       _bufferReqID;
            std::string                 _xSessionID;
		};
	}
}
#endif // _C2_REQUEST_HANDLE_H