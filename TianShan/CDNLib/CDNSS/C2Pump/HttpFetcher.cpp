#include "HttpFetcher.h"
#include <string.h>
#include <TimeUtil.h>

namespace C2Streamer{

uint64 currentTimeH() {
	struct timeval v;
	gettimeofday( &v , NULL );
	return (uint64)v.tv_sec * 1000 * 1000 + v.tv_usec;
}

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
_reqID(0),
_retry(retry),
_retryStatus(0),
_bufferDataSize(0),
_replaceC2(false),
_chunked(true),
_notChunkStartRecv(false),
LibAsync::Socket(LibAsync::getLoopCenter().getLoop()),
LibAsync::HttpClient(), 
LibAsync::Timer(Socket::getLoop())
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "HttpFetcher() try to create a new httpfetcher."));
	uint64 start = currentTimeH();
	setRecvBufSize( 200 * 1024 );
	uint64 step1 = currentTimeH();
	/*for (int i=0; i<1; i++)
	{
		LibAsync::AsyncBuffer buf;
		buf.len = 4 * 64 * 1024;
		buf.base = (char*)malloc(sizeof(char)* buf.len);
		if ( buf.base == NULL )
			continue;
		_recvBufs.push_back(buf);
	}*/
	uint64 step2 = currentTimeH();
	_ID = __sync_add_and_fetch(&HttpFetcherID,1);
	uint64 step3 = currentTimeH();
	_queryBody.clear();
	//assert(!_recvBufs.empty() && "failed to malloc recv buffers in HttpFetcher");
	assert(_readCallBack != NULL && "the read call back is null.");
	//_headerMsg = new LibAsync::HttpMessage
	uint64 step4 = currentTimeH();
	int a = (int)(step1 - start);
	int b = (int)(step2 - step1);
	int c = (int)(step3 - step2);
	int d = (int)(step4 - step3);

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "HttpFetcher() httpfetcher[%ld] bufReqId[%ld] created using[%d:%d:%d:%d]."), _ID,_reqID,a, b, c, d);
}

HttpFetcher::~HttpFetcher()
{
	  if(_chunked)
	  {
	  		for ( LibAsync::AsyncBufferS::iterator recvIt = _recvBufs.begin(); recvIt != _recvBufs.end(); recvIt ++ )
	  		{
				if ( recvIt->base != NULL )
				{
				  	free(recvIt->base);
				  	recvIt->base = NULL;
				}
	  		}
	   }
	  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "~HttpFetcher() httpfetcher[%ld] bufReqId[%ld] segsess[%s] destoryed."), _ID, _reqID, _segSess.c_str());
}

void HttpFetcher::read(BufferUser buf)
{
	if(!buf)
		return;
	buf->setLastError(0, Buffer::ECATE_SUCC);
	_reqID = buf->reqId();
	_dataBufer = buf;
	_fileName = buf->filename();
	_offset = buf->offsetInFile();
	_bufferDataSize = 0;
	if(_retry != 0 )
		_bufferDataSize = buf->getDataSize();
	_dataSize = 0;
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "read() entry httpfetcher[%ld] bufReqId[%ld] retry[%d] data[%d]."), _ID, _reqID,_retry, _bufferDataSize);
	if( ( _retryStatus == 0 ) && (_retry > 0) )
	{
		update(500);
		_retryStatus = 1;
		return;
	}	
	HttpAsyncWork::Ptr pt = new HttpAsyncWork(this, Socket::getLoop());
	pt->queueWork();
	return;
}

void HttpFetcher::queryIndexInfo(AssetAttribute::Ptr attr, bool replaceC2/* = false*/)
{
	attr->lastError(AssetAttribute::ASSET_SUCC);
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "queryIndexInfo() entry httpfetcher[%ld] retry[%d]."), _ID, _retry);
	if (attr == NULL )
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "queryIndexInfo() httpfetcher[%ld] entry with NULL asset attribute."), _ID);
		return;
	}
	_reqID = attr->reqId();
	_fileName = attr->filename();  
	_querySwitch = true;
	_replaceC2 = replaceC2;
	_fileAttr = attr;
	_offset = -1;
	if( ( _retryStatus == 0 ) && (_retry > 0) )
	{
		update(500);
		_retryStatus = 1;
		return;
	}
	HttpAsyncWork::Ptr pt = new HttpAsyncWork(this, Socket::getLoop());
	pt->queueWork();
	return;
}

void	HttpFetcher::onReqMsgSent( size_t size)
{
	  cancel();
	  if (!_dataBufer)
	  {
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onReqMsgSent() httpfetcher[%ld] bufReqId[%ld]  data bufer is null."), _ID, _reqID);
			return;
	  }
	  if(_reqBegin == 0)
	  {
			_reqBegin = ZQ::common::TimeUtil::now();
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onReqMsgSent() httpfetcher[%ld] bufReqId[%ld] set request begin time[%lld]."), _ID, _reqID, _reqBegin);
	  }
	  std::string locip="";
	  unsigned short locport=0;
	  getLocalAddress(locip, locport);
	  std::string peerip="";
	  unsigned short  peerport=0;
	  getPeerAddress(peerip, peerport);

	  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onReqMsgSent() httpfetcher[%ld] bufReqId[%ld] [%s:%d==>%s:%d]."), _ID, _reqID, locip.c_str(), locport, peerip.c_str(), peerport);
		

	  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onReqMsgSent() httpfetcher[%ld]  sent request size[%d] to[%s] successful."), _ID, size, _proxyUrl.c_str());
	  update(_timeOut);
	  if( !getResponse() )
	  {
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "onReqMsgSent() httpfetcher[%ld] bufReqId[%ld] failed to associate response"),_ID, _reqID);
			return;
	  }
	  _log(ZQ::common::Log::L_INFO, CLOGFMT(HttpFetcher, "onReqMsgSent() httpfetcher[%ld] bufReqId[%ld] associated response"), _ID, _reqID);
}

void	HttpFetcher::onHttpDataReceived( size_t size )
{
	  cancel();
	  //_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpDataReceived() httpfetcher[%ld] recv data[%d]."), _ID, size);
	  int httpStatus = _responseHeader->code();
	  if ( 2 != httpStatus / 100 )
	  {
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "onHttpDataReceived() httpfetcher[%ld] bufReqId[%ld] segsess[%s] get http error[%d]."), _ID, _reqID, _segSess.c_str(), httpStatus);
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
				  _dataBufer->setDataSize(_bufferDataSize);
				  _dataBufer->setLastError(httpStatus, Buffer::ECATE_HTTP);
				  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpDataReceived() httpfetcher[%ld] bufReqId[%ld] segsess[%s] call onRead to return data."), _ID, _reqID, _segSess.c_str() );
				  bool rty = true;
				  if(416 == httpStatus)
					  rty = false;
				  if(_bufferDataSize > 0)
				  {
					  //_dataBufer->fileSize( _offset + _bufferDataSize);
				  }
				  _readCallBack->onRead(_dataBufer, rty);
				  _dataBufer = NULL;
				  _dataSize = 0;
			} 
			return ;
	  }

	  if (!_httpComplete)
	  {
		update(_timeOut);
		if( !_querySwitch )
		{
			if( !_dataBufer )
			{
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpDataReceived() httpfetcher[%ld] bufReqId[%ld] segsess[%s] get the not chunked data bufer is null."),  _ID, _reqID, _segSess.c_str());
				return;
			}
			//_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpDataReceived() httpfetcher[%ld] bufReqId[%ld] segsess[%s] got the data[%d==>%d]."), _ID, _reqID, _segSess.c_str(), size, (int)(_bufferDataSize - _dataBufer->getDataSize()) );
			if(_notChunkStartRecv)
			{
				int64 oldBufferDataSize = _dataBufer->getDataSize();
				if( (_bufferDataSize != 0) && ((_bufferDataSize - oldBufferDataSize) != size) )
				{
					assert(false && "other data in buffer.");
				}
			}
			_dataBufer->setDataSize(_bufferDataSize);
			if( !_chunked )
			{
				_recvBufs.clear();
				LibAsync::AsyncBuffer buf;
				buf.len = _dataBufer->bufSize() - _dataBufer->getDataSize();
				buf.base = _dataBufer->buffer() + _dataBufer->getDataSize();
				//if( buf.base != NULL)
				_recvBufs.push_back(buf);
				_notChunkStartRecv = true;
			}
		}
		/*
		if((!_querySwitch) && _dataBufer)
			_dataBufer->setDataSize(_bufferDataSize);
		if(!_chunked)
		{
			if( !_dataBufer )
			{
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpDataReceived() httpfetcher[%ld] bufReqId[%ld] segsess[%s] get the not chunked data bufer is null."),  _ID, _reqID, _segSess.c_str());
				return;
			}
			_recvBufs.clear();
			LibAsync::AsyncBuffer buf;
			buf.len = _dataBufer->bufSize() - _dataBufer->getDataSize();
			buf.base = _dataBufer->buffer() + _dataBufer->getDataSize();
			if( buf.base != NULL)
				_recvBufs.push_back(buf);
		}
		*/
		if(_recvBufs.empty())
			assert(false && "the recv buffer is empty");
	  	bool ret = recvRespBody(_recvBufs);
		//_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpDataReceived() httpfetcher[%ld] bufReqId[%ld] segsess[%s] recved data[%d] and continue to recv data."), _ID, _reqID, _segSess.c_str(), size);
		assert(ret && "failed to recv body.");
	  }
}

bool	HttpFetcher::onHttpMessage( const LibAsync::HttpMessagePtr msg)
{
	  if( _resBegin == 0 )
	  {
			_resBegin = ZQ::common::TimeUtil::now();
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpMessage() httpfetcher[%ld] bufReqId[%ld] set response begin time[%lld]."), _ID, _reqID, _resBegin);
		        if (!_querySwitch)	
					_readCallBack->onLatency(_fileName, _offset, _resBegin);
	  }
	  _segSess = msg->header("X-SessionId");
	  if( !msg->chunked() )
		  _chunked = true;
	  if(_chunked || _querySwitch)
	  {
		    /*
		    if( !_recvBufs.empty() )
			{
	  			for(LibAsync::AsyncBufferS::iterator recvIt = _recvBufs.begin(); recvIt != _recvBufs.end(); recvIt ++ )
	  			{
					if ( recvIt->base != NULL )
					{
				  		free(recvIt->base);
				  		recvIt->base = NULL;
					}
	  			}	
			}*/
	  		for (int i=0; i<1; i++)
			{
				LibAsync::AsyncBuffer buf;
				buf.len = 2 * 64 * 1024;
				buf.base = (char*)malloc(sizeof(char)* buf.len);
				if ( buf.base == NULL )
					continue;
				_recvBufs.push_back(buf);
			}
	  }
	  else
	  {
		  	if(_dataBufer)
			{
				_dataBufer->setDataSize(0);
				_bufferDataSize = 0;
	  			LibAsync::AsyncBuffer buf;
				buf.len = _dataBufer->bufSize();
				buf.base = _dataBufer->buffer();
				if( buf.base != NULL)
					_recvBufs.push_back(buf);
				_notChunkStartRecv = false;
			}
	  }
	  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpMessage() httpfetcher[%ld] bufReqId[%ld] segsess[%s] got the response chunked[%s] contentlegth[%ld] message[%s]."), _ID, _reqID, _segSess.c_str(), _chunked ? "true" : "false" , msg->contentLength(), msg->toRaw().c_str());
	  _responseHeader = msg;
	  return true;
}
/*
bool   HttpFetcher::checkFileData( const char* data, size_t size)
{
	int ot = _offset + _bufferDataSize;
	int pos = 188 - ot % 188;
	for(; pos < size ; pos += 188 * 3 )
	{
		char ch = *(data + pos);
		 if(ch != 0x47){
			assert(false && "onhttpbody recv err data.");
		 }
	}
	return true;	
}
*/
bool	HttpFetcher::onHttpBody( const char* data, size_t size)
{
	  int httpStatus = _responseHeader->code();
	  if ( 2 != httpStatus / 100 )
			return false;
	  if ( _querySwitch )
	  {
			_queryBody.append(data, size);
	  }
	  else
	  {
		  //  checkFileData(data, size);
			if(!_dataBufer)
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "onHtttpBody(), httpfetcher[%ld] bufReqId[%ld] segsess[%s] the data buff is null."),_ID, _reqID, _segSess.c_str());
				return false;
			}
			if (_dataSize + size > _dataBufer->bufSize())
			{
				  _log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "onHttpBody() httpfetcher[%ld] bufReqId[%ld] segsess[%s] recv data size [%d] greater the buff size[%d]."), _ID, _reqID, _segSess.c_str(), _dataSize + size, _dataBufer->bufSize());
				  return false;
			}
			_dataSize += size;
			if( _dataSize <= _bufferDataSize)
			{
				 _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpBody() httpfetcher[%ld] bufReqId[%ld] segsess[%s] recv[%d] data[%d] less than data size[%d]."), _ID, _reqID, _segSess.c_str(), size,  _dataSize, _bufferDataSize);
				return true;
			}
			if( !_chunked && _notChunkStartRecv )
			{
				_bufferDataSize += size;
				//_dataSize = _bufferDataSize;
				return true;
			}
			int writeSize = _dataSize - _bufferDataSize; //writedSize = size need write into buffer
			if( writeSize > size )
				assert(false && "http fetcher write data failed");
			int dataSize = size -writeSize; // dataSize = size already write int buffer 
			if( writeSize > 0 )
			{
				 memcpy(_dataBufer->buffer() + _bufferDataSize, data + dataSize, writeSize);
				_bufferDataSize += writeSize;
				if(_bufferDataSize != _dataSize)
					assert(false && "http fetcher write data failed");
			}
			//size_t delta = size > (_dataBufer->bufSize() - _dataSize ) ? (_dataBufer->bufSize() - _dataSize) : size;
			//memcpy(_dataBufer->buffer()+_dataSize, data, delta );
			//_dataSize += size;
	  }
	  return true;
}

void	HttpFetcher::onHttpComplete()
{
	  cancel();
	  if ( _resEnd == 0){
			_resEnd = ZQ::common::TimeUtil::now();
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] bufReqId[%ld] segsess[%s] set response end [%lld]"), _ID, _reqID, _segSess.c_str(), _resEnd);
	  }
	  _httpComplete = true;
	  int buffSize = 0;
	//if the http result code  is not 2**, then 
	  int httpStatus = _responseHeader->code();
	  if ( 2 != httpStatus / 100 )
	  {
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] bufReqId[%ld] segsess[%s] get http error[%d]."), _ID, _reqID, _segSess.c_str(), httpStatus);
			if ( _querySwitch )
			{
				  if ( _fileAttr != NULL )
				  {
						_fileAttr->lastError( httpStatus == 404 ? AssetAttribute::ASSET_NOTFOUND: AssetAttribute::ASSET_HTTP);
						_queryBody.clear();
						_readCallBack->onIndexInfo( _fileAttr, true );
						_fileAttr = NULL;

				  }
			}
			else
			{
				  if (!_dataBufer)
						return;
				  _dataBufer->setDataSize(_bufferDataSize);
				  _dataBufer->setLastError(httpStatus, Buffer::ECATE_HTTP);
				  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] bufReqId[%ld] segsess[%s] call onRead to return data."), _ID, _reqID, _segSess.c_str());
				  if(_bufferDataSize > 0)
				  {
				  	//_dataBufer->fileSize( _offset + _bufferDataSize);
				  }
				  _readCallBack->onRead(_dataBufer, true);
				  _dataBufer = NULL;
				  _dataSize = 0;
			} 
			return ;
	  }
	  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] bufReqId[%ld] segsess[%s] the get the http response[%d]."), _ID, _reqID, _segSess.c_str(), httpStatus);
	  if ( _querySwitch )
	  {
	      bool retry = false;
		  if ( _queryBody.empty() )
		  {
			  _log(ZQ::common::Log::L_WARNING, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] bufReqId[%ld] segsess[%s] the query index body is empty."), _ID, _reqID, _segSess.c_str());
			  _readCallBack->onIndexInfo( _fileAttr, true);
			  _fileAttr = NULL;
			  return;
		  }
		  if ( _fileAttr == NULL )
		  {
			  _log(ZQ::common::Log::L_WARNING, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] bufReqId[%ld] segsess[%s] the assert attribute is NULL."), _ID, _reqID, _segSess.c_str());
			  return;
		  }
		  if( _replaceC2 )
		  {
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] bufReqId[%ld] segsess[%s] try to parse index file."), _ID, _reqID, _segSess.c_str());
				ZQ::IdxParser::IndexData idxData;
				bool result = parseIndex(_fileName, _queryBody.c_str(), _queryBody.size(), idxData);
				if ( result )
				{
					  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] bufReqId[%ld] segsess[%s] parse index success mainFileSize[%ld]."), _ID, _reqID, _segSess.c_str(), idxData.getMainFileSize());
					  _fileAttr->pwe(idxData.getMainFileSize() == 0);
					  //_fileAttr->pwe(false);
					  ZQ::IdxParser::IndexData::SubFileInformation info;
					  if (idxData.getSubFileInfo(0, info))
					  {
							std::ostringstream startOffsetOSS;
							uint64 startOffset =info.startingByte;
							startOffsetOSS<<startOffset;
							std::ostringstream endOffsetOSS;
							uint64 endOffset   = info.endingByte;
							endOffsetOSS<<endOffset;
							_fileAttr->range(atol(startOffsetOSS.str().c_str()), atol(endOffsetOSS.str().c_str()));
					  }

					  _fileAttr->assetBaseInfo(idxData.baseInfoToXML());
					  _fileAttr->assetMemberInfo(idxData.memberFileToXML());
				}//if ( result )
				else
				{
					  _fileAttr->lastError(C2Streamer::AssetAttribute::ASSET_DATAERR);
					  retry = true;
					  _log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] bufReqId[%ld] segsess[%s] parse index failed."), _ID, _reqID, _segSess.c_str());
					  return;
				}
		  }//if( _replaceC2 )
		  else
		  {	  
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
						  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] bufReqId[%ld] segsess[%s] pwe value[%s]."), _ID, _reqID, _segSess.c_str(), value.c_str());
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
						  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] bufReqId[%ld] segsess[%s] baseinfo value[%s]."), _ID, _reqID, _segSess.c_str(), value.c_str());
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
			  if (_queryBody.empty())
			  {
				  _fileAttr->lastError(AssetAttribute::ASSET_DATAERR);
				  retry = true;
			  }
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
			_dataBufer->setDataSize(_bufferDataSize);
			//std::vector<C2Streamer::Buffer*> bufs;
			//bufs.push_back(_dataBufer);
			buffSize = _bufferDataSize;
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld], call onRead to return data."), _ID);
			bool retry = false;
			if(buffSize == 0)
			{
				//_dataBufer->setLastError(-2, Buffer::ECATE_FILEIO);
				retry = true;
			}
			if(_bufferDataSize > 0)
			{
				//_dataBufer->fileSize( _offset + _bufferDataSize);
			}
			_readCallBack->onRead(_dataBufer, retry);
			_dataBufer = NULL;
			_dataSize = 0;
	  }
	  int reqTranTime = (int)(_resBegin - _reqBegin);
	  int resSendTime = (int)(_resEnd - _resBegin);
	  _log(ZQ::common::Log::L_INFO, CLOGFMT(HttpFetcher, "onHttpComplete() httpfetcher[%ld] bufReqId[%ld] segsess[%s] got content[%d]bytes, took [%d]ms to send, [%d]ms to receive"), _ID, _reqID, _segSess.c_str(), buffSize, reqTranTime, resSendTime);
}

void	HttpFetcher::onHttpError( int error )
{
	  cancel();
	  
	  std::string locip="";
	  unsigned short locport=0;
	  getLocalAddress(locip, locport);
	  std::string peerip="";
	  unsigned short  peerport=0;
	  getPeerAddress(peerip, peerport);
	  _log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "onHttpError() httpfetcher[%ld] bufReqId[%ld] segsess[%s] get error[%d] sysErr[%d] con[%s:%d==>%s:%d], url[%s]."),
			  _ID, _reqID, _segSess.c_str(), error, errno, locip.c_str(), locport, peerip.c_str(), peerport, _segmenterUrl.c_str() );
	  
	  //_log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "onHttpError() httpfetcher[%ld] bufReqId[%ld] segsess[%s] get error[%d] sysErr[%d]."), _ID, _reqID, _segSess.c_str(), error, errno);
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
			_dataBufer->setDataSize(_bufferDataSize);
			_dataBufer->setLastError(error, Buffer::ECATE_SOCKET);
			//std::vector<C2Streamer::Buffer*> bufs;
			//bufs.push_back(_dataBufer);
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onHttpError() httpfetcher[%ld],call onRead to return data."), _ID);
			if(_bufferDataSize > 0)
			{
				//_dataBufer->fileSize( _offset + _bufferDataSize);
			}
			_readCallBack->onRead(_dataBufer, true);
			_dataBufer = NULL;
			_dataSize = 0;
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
			queryIndexAction();
		else
			readAction();
        return;
	  }
	  _log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "onTimer()httpfetcher[%ld] bufReqId[%ld] segsess[%s] entry client[%p]."), _ID, _reqID, _segSess.c_str(), this);
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
			_dataBufer->setDataSize(_bufferDataSize);
			_dataBufer->setLastError(-4, Buffer::ECATE_TIMEOUT);
			//std::vector<C2Streamer::Buffer*> bufs;
			//bufs.push_back(_dataBufer);
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "onTimer() httpfetcher[%ld], call onRead to return data."), _ID);
			if(_bufferDataSize > 0)
			{
				//_dataBufer->fileSize( _offset + _bufferDataSize);
			}
			_readCallBack->onRead(_dataBufer, true);
			_dataBufer = NULL;
			_dataSize = 0;
	  }
}

void HttpFetcher::readAction()
{
	setHttpProxy(_proxyUrl);
	LibAsync::HttpMessagePtr  reqMsg = new LibAsync::HttpMessage(HTTP_REQUEST);
	reqMsg->method(HTTP_GET);
	reqMsg->url(_segmenterUrl);
	reqMsg->header("HttpFetcherID",_ID);
	int64 timeout = ZQ::common::TimeUtil::now() + _timeOut;
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "readAction() httpfetcher[%ld] bufReqId[%ld] file[%s] client[%p] url[%s]."), 
			_ID, _reqID, _fileName.c_str(), this, _segmenterUrl.c_str() );
	//update(_timeOut);
	update(500);
	if (!beginRequest(reqMsg, _segmenterUrl))
	{
		cancel();
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "readAction() httpfetcher[%ld] bufReqId[%ld] send request segmenterUrl[%s] proxyUrl[%s] sysErr[%d] failed."), _ID, _reqID, _segmenterUrl.c_str(), _proxyUrl.c_str(), errno);
		if (!_dataBufer)
			return;
		_dataBufer->setLastError(-2, Buffer::ECATE_SOCKET);
		_dataBufer->setDataSize(_bufferDataSize);
		//std::vector<C2Streamer::Buffer*> bufs;
		//bufs.push_back(_dataBufer);
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "readAction() httpfetcher[%ld] bufReqId[%ld] call onRead to return data."), _ID, _reqID);
		if(_bufferDataSize > 0)
		{
			//_dataBufer->fileSize( _offset + _bufferDataSize);
		}
		_readCallBack->onRead(_dataBufer, true);
		_dataBufer = NULL;
		_dataSize = 0;
		return ;
	}
	std::string locip="";
	unsigned short locport=0;
	getLocalAddress(locip, locport);
	std::string peerip="";
	unsigned short  peerport=0;
	getPeerAddress(peerip, peerport);
	
	_log(ZQ::common::Log::L_INFO, CLOGFMT(HttpFetcher, "readAction() httpfetcher[%ld] bufReqId[%ld] sent to[%s] conn[%s:%d==>%s:%d] request[%s] client[%p]"), _ID, _reqID, _proxyUrl.c_str(), locip.c_str(), locport, peerip.c_str(), peerport, reqMsg->toRaw().c_str(), this);
	return;
}

void HttpFetcher::queryIndexAction()
{
	setHttpProxy(_proxyUrl);
	LibAsync::HttpMessagePtr  reqMsg = new LibAsync::HttpMessage(HTTP_REQUEST);
	reqMsg->method(HTTP_GET);
	reqMsg->url(_segmenterUrl);
	reqMsg->header("HttpFetcherID",_ID);
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpFetcher, "queryIndexAction() httpfetcher[%ld] bufReqId[%ld] fileName[%s] client[%p]."), _ID, _reqID, _fileName.c_str(), this);
	//update(_timeOut);
	update(500);
	if (!beginRequest(reqMsg, _segmenterUrl))
	{
		cancel();
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(HttpFetcher, "queryIndexAction() httpfetcher[%ld] bufReqId[%ld] send request usr[%s] to[%s] failed."), _ID, _reqID, _segmenterUrl.c_str(), _proxyUrl.c_str());
		if (_fileAttr != NULL)
		{
			_fileAttr->lastError(AssetAttribute::ASSET_SOCKET);
			_readCallBack->onIndexInfo( _fileAttr, true );
			_fileAttr = NULL;
		}
		return ;
	}
	  std::string locip="";
	  unsigned short locport=0;
	  getLocalAddress(locip, locport);
	  std::string peerip="";
	  unsigned short  peerport=0;
	  getPeerAddress(peerip, peerport);

	_log(ZQ::common::Log::L_INFO, CLOGFMT(HttpFetcher, "queryIndexAction() httpfetcher[%ld] bufReqId[%ld] sent to[%s] conn[%s:%d==>%s:%d] arequest[%s] client[%p]."), _ID, _reqID, _proxyUrl.c_str(), locip.c_str(), locport, peerip.c_str(), peerport, reqMsg->toRaw().c_str(), this);

	return;
}

bool HttpFetcher::parseIndex(std::string& contentName, const char* indexData, size_t dataSize, ZQ::IdxParser::IndexData& idxData)
{
	  ZQ::IdxParser::IdxParserEnv			idxParserEnv;
	  idxParserEnv.AttchLogger(&_log);
	  ZQ::IdxParser::IndexFileParser		idxParser(idxParserEnv);

	  if(!idxParser.ParseIndexFromMemory( contentName, idxData, indexData, dataSize ) ) 
	  {
			_log(ZQ::common::Log::L_ERROR,CLOGFMT(HttpFetcher,"parseIndex() httpfetcher[%ld] bufReqId[%ld] segsess[%s] failed to parse index data for[%s], data size[%u]"),  _ID, _reqID, _segSess.c_str(), _fileName.c_str(), (uint32)dataSize);
			return false;
	  }
	  return true;
}
HttpAsyncWork::HttpAsyncWork(HttpFetcher::Ptr ptr, LibAsync::EventLoop& loop)
	:HttpClientPtr(ptr),
	LibAsync::AsyncWork(loop)
{

}

HttpAsyncWork::~HttpAsyncWork()
{
	HttpClientPtr = NULL;
}

void HttpAsyncWork::onAsyncWork()
{
	if(HttpClientPtr->getClientType())
		HttpClientPtr->queryIndexAction();
	else
		HttpClientPtr->readAction();

}

}// name space
