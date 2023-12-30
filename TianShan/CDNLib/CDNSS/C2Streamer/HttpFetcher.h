#ifndef  _HTTP_FETCHER_H
#define  _HTTP_FETCHER_H

#include <http.h>
#include <Log.h>
#include <AioFile.h>


namespace C2Streamer{

class ReadHttpCallBack :virtual public ZQ::common::SharedObject{
 public:
	virtual void onRead( Buffer* buf, bool retry = false ) = 0;
	virtual void onLatency(std::string& fileName, int64 offset,int64 time) = 0;
    virtual void onError( int err ) = 0;
    virtual void onIndexInfo( AssetAttribute::Ptr attr, bool retry = false ) = 0;

};

class HttpFetcher : public virtual  LibAsync::HttpClient, public virtual LibAsync::Timer
{
public:
	  typedef ZQ::common::Pointer<HttpFetcher> Ptr;
	  HttpFetcher(ReadHttpCallBack* callBack, std::string& proxyUrl, std::string& segmenterUrl, ZQ::common::Log& log, const int timeout = 10*1000, int retry = 0);
	  virtual ~HttpFetcher();
public:
	  void read(Buffer* buf);
	  void queryIndexInfo(AssetAttribute::Ptr attr);
	  int getRetry() {return _retry;}
	  std::string getSegmenterUrl() { return _segmenterUrl;}
private:

	  virtual void	onReqMsgSent( size_t size);
	  virtual void	onHttpDataReceived( size_t size );
	  virtual bool	onHttpMessage( const LibAsync::HttpMessagePtr msg);
	  virtual bool	onHttpBody( const char* data, size_t size);
	  virtual void	onHttpComplete();
	  virtual void	onHttpError( int error );

	  virtual void	onTimer();

	  
private:
	  
	  LibAsync::AsyncBufferS		_recvBufs;
	  LibAsync::HttpMessagePtr	_responseHeader;

	  ZQ::common::Log&                    _log;
	  ReadHttpCallBack*                        _readCallBack;
	  std::string                                   _proxyUrl;
	  std::string                                   _segmenterUrl;

	  Buffer*                                       _dataBufer;
	  int                                              _dataSize;
	  int                                              _timeOut;

	  bool                                            _httpComplete;

	  int64                                           _reqBegin;
	  int64                                           _resBegin;
	  int64                                           _resEnd;

	  AssetAttribute::Ptr                     _fileAttr;
	  bool                                          _querySwitch;
	  std::string                                  _queryBody;
	  int64                                       _ID;
          std::string                                 _fileName;
	  int64                                       _offset;
	  int                                         _retry;
	  int                                         _retryStatus;
};

}
#endif
