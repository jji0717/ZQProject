#ifndef   _READHTTP_H
#define  _READHTTP_H

#include "AioFile.h"
#include "HttpFetcher.h"

namespace C2Streamer{
class ReadHttp :  public IDataReader, public ReadHttpCallBack, virtual public ZQ::common::SharedObject
{
public:
	  typedef ZQ::common::Pointer<ReadHttp> Ptr;
	  ReadHttp(IReaderCallback* callBack, const std::string& proxyUrl, const std::string& segmenterUrl, ZQ::common::Log& log, const int timeout = 10*1000, int retrytime = 0, bool replacec2 = false);
	  ~ReadHttp();
public:
	  virtual bool read( const std::vector<BufferUser>& bufs );
	  virtual bool queryIndexInfo( const std::string& filename, AssetAttribute::Ptr attr );
public:
	virtual void onRead(BufferUser buf, bool retry = false);
	virtual void onLatency(std::string& fileName, int64 offset,int64 time);
    virtual void onError( int err );
    virtual void onIndexInfo( AssetAttribute::Ptr attr, bool retry = false);
private:
	  HttpFetcher::Ptr addClient(const std::string& fileName, int64 offset, std::string resUrl, int retry = 0, int type = 0);
	  HttpFetcher::Ptr delClient(const std::string& fileName, int64 offset, int type = 0);                
private:
	  typedef std::map<std::string,HttpFetcher::Ptr> FileClientMap; 
	  ZQ::common::Log&                    _log;
	  IReaderCallback*                        _callBack;
	  std::string                                   _proxyUrl;
	  std::string                                   _segmenterUrl;
	  int                                              _timeOut;
	  ZQ::common::Mutex                   _clientMapLock;
	  FileClientMap                       _clientMap; 
	  int                                 _retryTimes;
	  bool                                _replaceC2;
};
}


#endif
