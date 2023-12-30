#include "HttpFetcher.h"
#include <string.h>
#include <TimeUtil.h>

namespace C2Streamer{
static int64  HttpFetcherID = 0;
HttpFetcher::HttpFetcher(ReadHttpCallBack* callBack, std::string& proxyUrl, std::string& segmenterUrl, ZQ::common::Log& log, const int timeout /* = 10*1000 */, int retry /*= 0*/)
:_readCallBack(callBack),
_proxyUrl(proxyUrl),
_segmenterUrl(segmenterUrl),
_log(log), 
_dataSize(0),
_timeOut(timeout),
_httpComplete(false),
_reqBegin(0),
_resBegin(0),
_resEnd(0),
_querySwitch(false),
_offset(-1),
_retry(retry),
_retryStatus(0),
LibAsync::Socket(LibAsync::getLoopCenter().getLoop()),
LibAsync::HttpClient(), 
	LibAsync::Timer(Socket::getLoop())
	{
		setRecvBufSize( 2 * 1024 * 1024 );
		for (int i=0; i<1; i++)
		{
			LibAsync::AsyncBuffer buf;
			buf.len = 8 * 64 * 1024;
			buf.base = (char*)malloc(sizeof(char)* buf.len);
			if ( buf.base == NULL )
				continue;
			_recvBufs.push_back(buf);
		}
		_ID = __sync_add_and_fetch(&HttpFetcherID,1);

		_queryBody.clear();
		assert(!_recvBufs.empty() && "failed to malloc recv buffers in HttpFetcher");
		assert(_readCallBack != NULL && "the read call back is null.");
		//_headerMsg = new LibAsync::HttpMessage
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "HttpFetcher() httpfetcher[%ld] created."), _ID);
	}

HttpFetcher::~HttpFetcher()
{
	  for ( LibAsync::AsyncBufferS::iterator recvIt = _recvBufs.begin(); recvIt != _recvBufs.end(); recvIt ++ )
	  {
			if ( recvIt->base != NULL )
			{
				  free(recvIt->base);
				  recvIt->base = NULL;
			}
	  }
	  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "~HttpFetcher() httpfetcher[%ld] destoryed."), _ID);
}

void HttpFetcher::read(Buffer* buf)
{
	 _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "read() entry httpfetcher[%ld] retry[%d]."), _ID, _retry);
	 buf->setLastError(-2, Buffer::ECATE_SUCC);
	  if(!buf)
		return;
	  _dataBufer = buf;
	  _fileName = buf->filename();
	  _offset = buf->offsetInFile();
	  if( ( _retryStatus == 0 ) && (_retry > 0) )
	  {
	  	update(500);
		_retryStatus = 1;
		return;
	  }	

      setHttpProxy(_proxyUrl);
	  LibAsync::HttpMessagePtr  reqMsg = new LibAsync::HttpMessage(HTTP_REQUEST);
	  reqMsg->method(HTTP_GET);
	  reqMsg->url(_segmenterUrl);
	  reqMsg->header("HttpFetcherID",_ID);
	  int64 timeout = ZQ::common::TimeUtil::now() + _timeOut;
	  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "read() httpfetcher[%ld] ,file[%s]."), _ID, _fileName.c_str());
	  update(_timeOut);
      	  
	  if (!beginRequest(reqMsg, _segmenterUrl))
	  {
			cancel();
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "read() httpfetcher[%ld] send request usr[%s] to[%s] failed."), _ID, _segmenterUrl.c_str(), _proxyUrl.c_str());
			if (!_dataBufer)
				  return;
			_dataBufer->setLastError(-2, Buffer::ECATE_SOCKET);
			_dataBufer->setDataSize(_dataSize);
			//std::vector<C2Streamer::Buffer*> bufs;
			//bufs.push_back(_dataBufer);
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "read() httpfetcher[%ld], call onRead to return data."), _ID);
			_readCallBack->onRead(_dataBufer, true);
			_dataSize = 0;
			_dataBufer = NULL;
			return ;
	  }
	  _log(ZQ::common::Log::L_INFO, CLOGFMT(HttpFetcher, "read() httpfetcher[%ld] sent request[%s] to[%s]"), _ID, reqMsg->toRaw().c_str(), _proxyUrl.c_str() );
}

void HttpFetcher::queryIndexInfo(AssetAttribute::Ptr attr)
{
	attr->lastError(AssetAttribute::ASSET_SUCC);
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "queryIndexInfo() entry httpfetcher[%ld]retry[%d]."), _ID, _retry);
	  if (attr == NULL )
	  {
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "queryIndexInfo() httpfetcher[%ld] entry with NULL asset attribute."), _ID);
			return;
	  }
	  _fileName = attr->filename();  
	  _querySwitch = true;
	  _fileAttr = attr;
	  _offset = -1;
	  if( ( _retryStatus == 0 ) && (_retry > 0) )
	  {
	  	update(500);
		_retryStatus = 1;
		return;
	  }

	  setHttpProxy(_proxyUrl);
	  LibAsync::HttpMessagePtr  reqMsg = new LibAsync::HttpMessage(HTTP_REQUEST);
	  reqMsg->method(HTTP_GET);
	  reqMsg->url(_segmenterUrl);
	  reqMsg->header("HttpFetcherID",_ID);
	  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "queryIndexInfo() httpfetcher[%ld],fileName[%s]."), _ID, _fileName.c_str());
	  update(_timeOut);
	  if (!beginRequest(reqMsg, _segmenterUrl))
	  {
			cancel();
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "queryIndexInfo() httpfetcher[%ld] send request usr[%s] to[%s] failed."), _ID, _segmenterUrl.c_str(), _proxyUrl.c_str());
			if (_fileAttr != NULL)
			{
				  _fileAttr->lastError(AssetAttribute::ASSET_SOCKET);
				  _readCallBack->onIndexInfo( _fileAttr, true );
				  _fileAttr = NULL;
			}
			return ;
	  }
	  _log(ZQ::common::Log::L_INFO, CLOGFMT(HttpFetcher, "queryIndexInfo() httpfetcher[%ld] sent request[%s] to[%s]"), _ID, reqMsg->toRaw().c_str(), _proxyUrl.c_str() );
}

void	HttpFetcher::onReqMsgSent( size_t size)
{
	  cancel();
	  if(_reqBegin == 0)
	  {
			_reqBegin = ZQ::common::TimeUtil::now();
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onReqMsgSent() httpfetcher[%ld] set request begin time[%lld]."), _ID, _reqBegin);
	  }
	  std::string locip="";
	  unsigned short locport=0;
	  getLocalAddress(locip, locport);
	  std::string peerip="";
	  unsigned short  peerport=0;
	  getPeerAddress(peerip, peerport);

	  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onReqMsgSent() httpfetcher[%ld] [%s:%d==>%s:%d]."), _ID, locip.c_str(), locport, peerip.c_str(), peerport);
		

	  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onReqMsgSent() httpfetcher[%ld]  sent request size[%d] to[%s] successful."), _ID, size, _proxyUrl.c_str());
	  update(_timeOut);
	  if( !getResponse() )
	  {
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "onReqMsgSent() httpfetcher[%ld] failed to associate response"),_ID);
			return;
	  }
	  _log(ZQ::common::Log::L_INFO, CLOGFMT(HttpFetcher, "onReqMsgSent() httpfetcher[%ld] associated response"), _ID);
}

void	HttpFetcher::onHttpDataReceived( size_t size )
{
	  cancel();
	  //_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpDataReceived() httpfetcher[%ld] recv data[%d]."), _ID, size);
	  int httpStatus = _responseHeader->code();
	  if ( 2 != httpStatus / 100 )
	  {
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "onHttpDataReceived() httpfetcher[%ld] get http error[%d]."), _ID, httpStatus);
			if ( _querySwitch )
			{
				  if ( _fileAttr != NULL )
				  {
						_fileAttr->lastError(AssetAttribute::ASSET_HTTP);
						_queryBody.clear();
						_readCallBack->onIndexInfo( _fileAttr, true );
						_fileAttr = NULL;

				  }
			}
			else
			{
				  if (!_dataBufer)
						return;
				  _dataBufer->setDataSize(_dataSize);
				  _dataBufer->setLastError(httpStatus, Buffer::ECATE_HTTP);
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpDataReceived() httpfetcher[%ld], call onRead to return data."), _ID);
				  _readCallBack->onRead(_dataBufer, true);
				  _dataBufer = NULL;
				  _dataSize = 0;
			} 
			return ;
	  }

	  if (!_httpComplete)
	  {
		update(_timeOut);
	  	bool ret = recvRespBody(_recvBufs);
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpDataReceived() httpfetcher[%ld],recved data[%d] and continue to recv data."), _ID, size);
		assert(ret && "failed to recv body.");
	  }
}

bool	HttpFetcher::onHttpMessage( const LibAsync::HttpMessagePtr msg)
{
	  if( _resBegin == 0 )
	  {
			_resBegin = ZQ::common::TimeUtil::now();
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpMessage() httpfetcher[%ld] set response begin time[%lld]."), _ID, _resBegin);
		        if (!_querySwitch)	
					_readCallBack->onLatency(_fileName, _offset, _resBegin);
	  }
	  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpMessage() httpfetcher[%ld] got the response message[%s]."), _ID, msg->toRaw().c_str());
	  _responseHeader = msg;
	  return true;
}

bool	HttpFetcher::onHttpBody( const char* data, size_t size)
{
	  if ( _querySwitch )
	  {
			_queryBody.append(data, size);
	  }
	  else
	  {
			if(!_dataBufer)
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "onHtttpBody(), httpfetcher[%ld] the data buff is null."),_ID);
				return false;
			}
			if (_dataSize + size > _dataBufer->bufSize())
			{
				  _log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "onHttpBody() httpfetcher[%ld] recv data size [%d] greater the buff size[%d]."), _ID, _dataSize + size, _dataBufer->bufSize());
				  return false;
			}
			size_t delta = size > (_dataBufer->bufSize() - _dataSize ) ? (_dataBufer->bufSize() - _dataSize) : size;
			memcpy(_dataBufer->buffer()+_dataSize, data, delta );
			_dataSize += size;
	  }
	  return true;
}

void	HttpFetcher::onHttpComplete()
{
	  cancel();
	  if ( _resEnd == 0){
			_resEnd = ZQ::common::TimeUtil::now();
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] set response end [%lld]"), _ID, _resEnd);
	  }
	  _httpComplete = true;
	  int buffSize = 0;
	//if the http result code  is not 2**, then 
	  int httpStatus = _responseHeader->code();
	  if ( 2 != httpStatus / 100 )
	  {
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] get http error[%d]."), _ID, httpStatus);
			if ( _querySwitch )
			{
				  if ( _fileAttr != NULL )
				  {
						_fileAttr->lastError(AssetAttribute::ASSET_HTTP);
						_queryBody.clear();
						_readCallBack->onIndexInfo( _fileAttr, true );
						_fileAttr = NULL;

				  }
			}
			else
			{
				  if (!_dataBufer)
						return;
				  _dataBufer->setDataSize(_dataSize);
				  _dataBufer->setLastError(httpStatus, Buffer::ECATE_HTTP);
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld], call onRead to return data."), _ID);
				  _readCallBack->onRead(_dataBufer, true);
				  _dataBufer = NULL;
				  _dataSize = 0;
			} 
			return ;
	  }
	  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] the get the http response[%d]."), _ID, httpStatus);
	  if ( _querySwitch )
	  {
			if ( _queryBody.empty() )
			{
				  _log(ZQ::common::Log::L_WARNING, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] the query index body is empty."), _ID);
				  _readCallBack->onIndexInfo( _fileAttr, true);
				  _fileAttr = NULL;
				  return;
			}
			if ( _fileAttr == NULL )
			{
				  _log(ZQ::common::Log::L_WARNING, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] the assert attribute is NULL."), _ID);
				  return;
			}
			//get pwe
			bool getPwe = false;
			int n_pos = _queryBody.find("pwe");
			if (n_pos != std::string::npos)
			{
				  if (_queryBody.at(n_pos+3) == ':')
				  {
						int s_pos = n_pos + 5;
						int e_pos = _queryBody.find("\n", s_pos);
						if (e_pos != std::string::npos )
						{
							  std::string value = _queryBody.substr(s_pos, e_pos - s_pos);
							  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] pwe value[%s]."), _ID, value.c_str());
							  getPwe = true;
							  if ( !value.compare("true"))
									_fileAttr->pwe(true);
						}
				  }
			}
			//get baseinfo
			bool getBaseinfo = false;
			n_pos = _queryBody.find("baseinfo");
			if (n_pos != std::string::npos)
			{
				  if (_queryBody.at(n_pos+8) == ':')
				  {
						int s_pos = n_pos + 10;
						int e_pos = _queryBody.find("\n", s_pos);
						if (e_pos != std::string::npos )
						{
							  std::string value = _queryBody.substr(s_pos, e_pos - s_pos);
							  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] baseinfo value[%s]."), _ID, value.c_str());
							  getBaseinfo = true;
							  _fileAttr->assetBaseInfo(value);
						}
				  }
			}
			//get memberinfo
			bool getMemBerinfo = false;
			n_pos = _queryBody.find("memberinfo");
			if (n_pos != std::string::npos)
			{
				  if (_queryBody.at(n_pos+10) == ':')
				  {
						int s_pos = n_pos + 12;
						int e_pos = _queryBody.find("\n", s_pos);
						if (e_pos != std::string::npos )
						{
							  std::string value = _queryBody.substr(s_pos, e_pos - s_pos);
							  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] baseinfo value[%s]."), _ID, value.c_str());
							  getMemBerinfo = true;
							  _fileAttr->assetMemberInfo(value);
						}
				  }
			}
			if ( !( getPwe && getBaseinfo && getMemBerinfo ) )
			{
				  _fileAttr->lastError(AssetAttribute::ASSET_DATAERR);
				  _log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] invalid index data."), _ID);
			}
			bool retry = false;
			if (_queryBody.empty())
			{
				//_fileAttr->lastError(AssetAttribute::ASSET_DATAERR);
				retry = true;
			}
			_readCallBack->onIndexInfo( _fileAttr, retry);
			_queryBody.clear();
			_fileAttr = NULL;
	  }
	  else
	  {
			if( !_dataBufer) {
				  _log(ZQ::common::Log::L_WARNING, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] the data buffer is null."), _ID);
				  return;
			}
			_dataBufer->setDataSize(_dataSize);
			//std::vector<C2Streamer::Buffer*> bufs;
			//bufs.push_back(_dataBufer);
			buffSize = _dataSize;
			_dataSize = 0;
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld], call onRead to return data."), _ID);
			bool retry = false;
			if(buffSize == 0)
			{
				//_dataBufer->setLastError(-2, Buffer::ECATE_FILEIO);
				retry = true;
			}
			_readCallBack->onRead(_dataBufer, retry);
			_dataBufer = NULL;
	  }
	  int reqTranTime = (int)(_resBegin - _reqBegin);
	  int resSendTime = (int)(_resEnd - _resBegin);
	  _log(ZQ::common::Log::L_INFO, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] got content[%d]bytes, took [%d]ms to send, [%d]ms to receive"), _ID, buffSize, reqTranTime, resSendTime);
}

void	HttpFetcher::onHttpError( int error )
{
	  cancel();
	  _log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "onHttpError() httpfetcher[%ld] get error[%d]."), _ID, error);
	  if ( _querySwitch )
	  {
			if (_fileAttr != NULL )
			{
				  _fileAttr->lastError(AssetAttribute::ASSET_SOCKET);
				  _queryBody.clear();
				  _readCallBack->onIndexInfo( _fileAttr, true);
				  _fileAttr = NULL;
			}
	  }
	  else
	  {
			if( !_dataBufer) {
				  _log(ZQ::common::Log::L_WARNING, CLOGFMT(HttpFetcher, "onHttpError() httpfetcher[%ld] the data buffer is null."), _ID);
				  return;
			}
			_dataBufer->setDataSize(_dataSize);
			_dataBufer->setLastError(error, Buffer::ECATE_SOCKET);
			//std::vector<C2Streamer::Buffer*> bufs;
			//bufs.push_back(_dataBufer);
			_dataSize = 0;
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpError() httpfetcher[%ld],call onRead to return data."), _ID);
			_readCallBack->onRead(_dataBufer, true);
			_dataBufer = NULL;
	  }
}

void	HttpFetcher::onTimer()
{
	  cancel();
	  if (_retryStatus == 1)
	  {
	  	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onTimer() httpfetcher[%ld] to retry file[%s] offsert[%ld]."), _ID, _fileName.c_str(), _offset);
		_retryStatus = 2;
		if(_querySwitch)
			queryIndexInfo(_fileAttr);
		else
			read(_dataBufer);
        return;
	  }
	  _log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "onTimer()httpfetcher[%ld] entry."), _ID);
	  if ( _querySwitch )
	  {
			if( _fileAttr != NULL )
			{
				  _fileAttr->lastError(AssetAttribute::ASSET_TIMEOUT);
				  _queryBody.clear();
				  _readCallBack->onIndexInfo( _fileAttr , true);
				  _fileAttr = NULL;
			}
	  }
	  else
	  {
			if( !_dataBufer) {
				  _log(ZQ::common::Log::L_WARNING, CLOGFMT(HttpFetcher, "onTimer() httpfetcher[%ld] the data buffer is null."), _ID);
				  return;
			}
			_dataBufer->setDataSize(_dataSize);
			_dataBufer->setLastError(-4, Buffer::ECATE_TIMEOUT);
			//std::vector<C2Streamer::Buffer*> bufs;
			//bufs.push_back(_dataBufer);
			_dataSize = 0;
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onTimer() httpfetcher[%ld], call onRead to return data."), _ID);
			_readCallBack->onRead(_dataBufer, true);
			_dataBufer = NULL;
	  }
}

}
