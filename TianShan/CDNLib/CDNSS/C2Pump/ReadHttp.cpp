#include "ReadHttp.h"
#include "C2SessionHelper.h"

namespace C2Streamer{
ReadHttp::ReadHttp(IReaderCallback* callBack, const std::string& proxyUrl, const std::string& segmenterUrl, ZQ::common::Log& log, const int timeout /* = 10*1000 */, int retrytime /*=0*/, bool replacec2 /*= false*/)
:_callBack(callBack),
_proxyUrl(proxyUrl),
_segmenterUrl(segmenterUrl),
_log(log),
_retryTimes(retrytime),
_timeOut(timeout),
_replaceC2(replacec2)
{
}

ReadHttp::~ReadHttp()
{

}

bool ReadHttp::read( const std::vector<BufferUser>& bufs )
{
	  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttp, "read() entry with bufs[%d]."), bufs.size());
	  for (int i = 0; i < bufs.size(); i++ )
	  {
			BufferUser buf = bufs[i];
			if ( !buf.valid() ) {
				  assert(false && "get the null buffer from cache center.");
			}
			std::string  segUrl  = _segmenterUrl + "/" + buf->filename();
			char buffer[256];
			memset(buffer, '\0', sizeof(buffer));
			snprintf(buffer, sizeof(buffer), "%s-%ldL%d", segUrl.c_str(), buf->offsetInFile( ), buf->bufSize());
			std::string resUrl(buffer);
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttp, "read get url [%s]."), resUrl.c_str());
		    //HttpFetcher::Ptr pt = new HttpFetcher(_callBack, _proxyUrl, resUrl, _log, _timeOut);
			HttpFetcher::Ptr pt = addClient(buf->filename(), buf->reqId(), resUrl, 0, 0);
			pt->read(buf);
	  }
	  return true;
}

bool ReadHttp::queryIndexInfo( const std::string& filename, AssetAttribute::Ptr attr )
{
	  _log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttp, "queryIndexInfo() entry with file[%s]."), filename.c_str() );
	  std::string resUrl = _segmenterUrl;
	  int pos = resUrl.find("getfile");
	  if(pos == std::string::npos )
	  {
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(ReadHttp, "queryIndexInfo() can not find getfile in _segmenterurl[%s]."), _segmenterUrl.c_str());
			return false;
	  }
	  if( _replaceC2 )
	  {
			resUrl += filename;
			resUrl += "-0L4095";
	  }
	  else
	  {
			resUrl.replace(resUrl.begin() + pos , resUrl.end(),"queryassetinfo");
			resUrl += "?filename=";
			resUrl += filename;
	  }
	//HttpFetcher::Ptr queryIndexPtr = new HttpFetcher(_callBack, _proxyUrl, resUrl, _log, _timeOut);
	  HttpFetcher::Ptr queryIndexPtr = addClient(filename, attr->reqId(),resUrl, 0, 1);
	  queryIndexPtr->queryIndexInfo(attr, _replaceC2);
	  return true;
}

uint64 currentTimeC() {
	    struct timeval v;
		gettimeofday( &v , NULL );
		return (uint64)v.tv_sec * 1000 * 1000 + v.tv_usec;
}

// create a new HttpFetcher client if type=0  then for read, 1 for querIndex
HttpFetcher::Ptr ReadHttp::addClient(const std::string& fileName, int64 offset, std::string resUrl, int retry /*= 0*/, int type /*=0*/)
{
	uint64 start = currentTimeC();
	std::ostringstream oss;
	oss<<fileName<<":"<<offset<<":"<<type;
	std::string key = oss.str();
	uint64 step1 = currentTimeC();
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttp, "addClient() try to create a new client."));
	HttpFetcher::Ptr httpPtr=new HttpFetcher(this, _proxyUrl, resUrl, _log, _timeOut, retry);
	uint64 step2 = currentTimeC();
	{
		ZQ::common::MutexGuard gd(_clientMapLock);
		if(_clientMap.find(key) != _clientMap.end())
			assert(false && "the key is already in client map.");
		_clientMap[key] = httpPtr;
	}
	uint64 step3 = currentTimeC();
	int a = (int)(step1 - start);
	int b = (int)(step2 - step1);
	int c = (int)(step3 - step2);
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttp, "addClient() create a new client to request file[%s], offset[%ld] usint[%d:%d:%d]."), fileName.c_str(), offset, a, b, c);
	return httpPtr;
}

HttpFetcher::Ptr ReadHttp::delClient(const std::string& fileName, int64 offset, int type)
{
	HttpFetcher::Ptr pt = NULL;
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ReadHttp, "delClient() try to delete client for file[%s], offset[%ld]."), fileName.c_str(), offset);
	std::ostringstream oss;
	oss<<fileName<<":"<<offset<<":"<<type;
	std::string key = oss.str();
	{
		ZQ::common::MutexGuard gd(_clientMapLock);
		/*FileClientMap::iterator iter = _clientMap.find(key);
		if( iter  == _clientMap.end())
			assert(false && "can not find the client in map.");
		pt = iter->second;
		iter->second = NULL;
		_clientMap.erase(iter);*/
		pt = _clientMap[key];
		_clientMap[key] = NULL;
		_clientMap.erase(key);
	}
	return pt;
}

void ReadHttp::onRead( BufferUser buf, bool retry /*=false*/ )
{
	HttpFetcher::Ptr pt = delClient(buf->filename(), buf->reqId(), 0);
	assert(pt != NULL);
	if(retry && (pt->getRetry() <  _retryTimes ))
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(ReadHttp, "onRead() [%d]times to retry for file[%s] offset[%ld]."), pt->getRetry() + 1, buf->filename().c_str(), buf->offsetInFile());
		HttpFetcher::Ptr ptNew = addClient(buf->filename(), buf->reqId(), pt->getSegmenterUrl(), pt->getRetry()+1, 0);
		ptNew->read(buf);
		pt=NULL;
		ptNew=NULL;
		return;
	}
	pt = NULL;
	std::vector<C2Streamer::BufferUser> bufs;
    bufs.push_back(buf);
	_callBack->onRead(bufs);
	return;
}

void ReadHttp::onLatency(std::string& fileName, int64 offset,int64 time)
{
	_callBack->onLatency(fileName, offset, time);
}

void ReadHttp::onError( int err )
{
	_callBack->onError(err);
}

void ReadHttp::onIndexInfo( AssetAttribute::Ptr attr , bool retry /*=false*/)
{
	HttpFetcher::Ptr pt = delClient(attr->filename(), attr->reqId(), 1);
	assert(pt != NULL);
	if(retry && (pt->getRetry() <  _retryTimes ))
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(ReadHttp, "onIndexInfo() [%d]times to retry for file[%s] offset[%ld]."), pt->getRetry() + 1, attr->filename().c_str(),attr->rangeStart());
		HttpFetcher::Ptr ptNew = addClient(attr->filename(), attr->reqId(), pt->getSegmenterUrl(), pt->getRetry()+1, 1);
		ptNew->queryIndexInfo(attr);
		pt=NULL;
		ptNew=NULL;
		return;
	}
	pt = NULL;
	_callBack->onIndexInfo(attr);
	return;

}


}
