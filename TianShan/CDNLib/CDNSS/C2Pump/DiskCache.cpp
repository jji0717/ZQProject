#include "DiskCache.h"
#include "TimeUtil.h"
#include "NativeThreadPool.h"
#include "MD5CheckSumUtil.h"

extern "C" {
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#ifdef ZQ_OS_MSWIN
	#include <direct.h>
#else
	#include <sys/uio.h>
	#include <dirent.h>
    #include <sys/vfs.h>
	#include <fcntl.h>
    #include <unistd.h>
#endif
}

#ifndef MAPSET
#  define MAPSET(_MAPTYPE, _MAP, _KEY, _VAL) if (_MAP.end() ==_MAP.find(_KEY)) _MAP.insert(_MAPTYPE::value_type(_KEY, _VAL)); else _MAP[_KEY] = _VAL
#endif // MAPSET

using namespace C2Streamer;
namespace XOR_Media {
namespace DiskCache {
// -----------------------------
// class CacheCopy
// -----------------------------
// the cache copy that to exchange with the on-disk file
CacheCopy::CacheCopy()
{
	_originKey.url = "";
	_originKey.stampAsOfOrigin = 0;
	_exchangeKey.pathName = "";
	_exchangeKey.stampAsOfDisk = 0;
}

CacheCopy::CacheCopy( const CacheCopy& ccp)
{
	_contentBody = ccp._contentBody;
	_originKey = ccp._originKey;
	_exchangeKey = ccp._exchangeKey;
}

CacheCopy& CacheCopy::operator= ( const CacheCopy& ccp )
{
	 _contentBody = ccp._contentBody;
	 _originKey = ccp._originKey;
	 _exchangeKey = ccp._exchangeKey;
	 return *this;
}

CacheCopy::~CacheCopy()
{

}

CacheCopy::Error CacheCopy::flush()
{
	// prepare the header
	Header cch;
	memset(&cch, 0x00, sizeof(cch));
	cch.signature = CacheCopy_Stamp_MagicNumber;
	cch.headerLen = sizeof(cch);
	cch.version = 0;
	cch.stampAsOfOrigin = _originKey.stampAsOfOrigin;

	//cch.contentSize = _contentBody->length(); //TODO: fix to take HQ's
	cch.contentSize = _contentBody.dataSize();
	strncpy(cch.url, _originKey.url.c_str(), sizeof(cch.url)-2);

	if (writev(_exchangeKey.pathName, cch, _contentBody) != (int)(sizeof(cch) + _contentBody.dataSize()))
		return eIOError;

	return eOK;
}
#ifdef ZQ_OS_MSWIN
#if 0
#if 0
int CacheCopy::writev(const std::string& filename, const Header& h, const BufferPtr& cont)
{
	FILE_SEGMENT_ELEMENT segs[2];
	memset(segs, 0x00, sizeof(segs));

	segs[0].Buffer = PtrToPtr64(&h);
	segs[0].Alignment = sizeof(h);

	size_t plLen =0, plSize=0;
	segs[1].Buffer    = PtrToPtr64(cont->bd(0, plLen, plSize));
	segs[1].Alignment = plLen;

	HANDLE hFile = ::CreateFile(filename.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (INVALID_HANDLE_VALUE == hFile)
		return 0;
	
	OVERLAPPED ol;
	memset(&ol, 0x00, sizeof(ol));
	if (FALSE == ::WriteFileGather(hFile, segs, (DWORD) (segs[0].Alignment + segs[1].Alignment), NULL, &ol))
		return 0;

	return (int) (segs[0].Alignment + segs[1].Alignment);
}

int CacheCopy::readv(const std::string& filename, Header& h, BufferPtr& cont)
{
	FILE_SEGMENT_ELEMENT segs[2];
	memset(segs, 0x00, sizeof(segs));

	segs[0].Buffer = PtrToPtr64(&h);
	segs[0].Alignment = sizeof(h);

	size_t plLen =0, plSize=0;
	segs[1].Buffer    = PtrToPtr64(cont->bd(0, plLen, plSize));
	segs[1].Alignment = plSize;

	HANDLE hFile = ::CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (INVALID_HANDLE_VALUE == hFile)
		return 0;
	if (FALSE == ::ReadFileScatter(hFile, segs, (DWORD) (segs[0].Alignment + segs[1].Alignment), NULL, NULL))
		return 0;

	return (int) (segs[0].Alignment + segs[1].Alignment);
}
#endif // 0

int CacheCopy::writev(const std::string& filename, const Header& h, const BufferPtr& cont)
{
	size_t plLen =0, plSize=0;
	FILE* f = fopen(filename.c_str(), "wb");
	int len = fwrite(&h, 1, sizeof(h), f);

	uint8 buf[1024*64] ;
	while((plSize = cont->read(buf, plLen, sizeof(buf))) > 0)
	{
		len += fwrite(buf, 1, plSize, f);
		plLen += plSize;
	}

	fclose(f);
	return len;
}

int CacheCopy::readv(const std::string& filename, Header& h, BufferUser cont)
{
	FILE* f = fopen(filename.c_str(), "rb");
	int size = fread(&h, 1, sizeof(h), f);
	char buf[1024];
	int len=0, bodylen=0;

	char* dataB = cont.getBuffer();
	size_t bufferlen = cont.bufferSize();
	size_t dataLen = bufferlen -  bodylen;
	while (dataLen > 0 && (len = fread(dataB, 1, dataLen, f)) >0 )
	{
		//cont->fill((uint8*)buf, bodylen, len);
		bodylen += len;
		dataLen -= len;
		dataB = dataB + len;
	}

	fclose(f);
	return (size + bodylen);
}
#endif
#else //  ZQ_OS_MSWIN

int CacheCopy::writev(const std::string& filename, const Header& h, const BufferUser& cont)
{
	// write the contentbody first
	struct iovec chunks[2];
	Header cch = h;
	BufferUser user = cont;
	chunks[0].iov_base = &cch;
	chunks[0].iov_len  = sizeof(cch);

	chunks[1].iov_base = user.getBuffer();
	chunks[1].iov_len  = user.dataSize();

	int f = open(filename.c_str(), O_CREAT | O_WRONLY);
	fchmod(f, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
	ssize_t len = ::writev(f, chunks, 2);

	fdatasync(f);
	close(f);

	if ( (ssize_t) (user.dataSize() + sizeof(cch)) > len )
		remove(filename.c_str());

	return len;
}

int CacheCopy::readv(const std::string& filename, Header& h, BufferUser cont)
{
	struct iovec chunks[2];
	chunks[0].iov_base = &h;
	chunks[0].iov_len  = sizeof(h);

	chunks[1].iov_base = cont.getBuffer();
	chunks[1].iov_len  = cont.bufferSize();

	int f = open(filename.c_str(), O_RDONLY);
	ssize_t len = ::readv(f, chunks, 2);
	if(len<0) {
		return 0;
	}
	close(f);
	int dataSize = len - sizeof(h);
	/*
	if( dataSize != cont.bufferSize() )
	{
		return 0;
	}
	*/
	cont.dataSize( dataSize );
	return len;
}

#endif // ZQ_OS_XXX

CacheCopy::Error CacheCopy::load(const std::string& pathName, BufferUser user)
{
	// initialize the header
	Header cch;
	memset(&cch, 0x00, sizeof(cch));

	// test the file size
	struct stat fst;
	if (0 != stat(pathName.c_str(), &fst))
		return eNotFound;

	if (fst.st_size < sizeof(cch))
		return eNoHeader;

	//BufferPtr cont = new ZQ::common::BufferList(); // TODO: fix to take HongQuan's buffer
	
	size_t len = readv(pathName, cch, user);

	if (len < sizeof(cch) || cch.signature != CacheCopy_Stamp_MagicNumber)
		return eBadHeader;

	_contentBody               = user;
	_originKey.url             = cch.url;
	_originKey.stampAsOfOrigin = cch.stampAsOfOrigin;

	_exchangeKey.stampAsOfDisk = ZQ::common::now();
	_exchangeKey.pathName = pathName;

	if (_originKey.url.empty())
		return eBadURL;

	return eOK;
}

CacheCopy::Error CacheCopy::unlink()
{
	::unlink(_exchangeKey.pathName.c_str());
	return eOK;
}

// -----------------------------
// class CacheSinkDispatcher
// -----------------------------
class CacheSinkDispatcher : public ZQ::common::ThreadRequest
{
public:
	static bool schedule(ZQ::common::NativeThreadPool& thpool, bool bWrote, DiskCacheSink* cbCompleted, DiskCacheSink::Error err, CacheCopy::Ptr ccp)
	{
		if (NULL == cbCompleted)
			return false;

		try {
			(new CacheSinkDispatcher(thpool, bWrote, cbCompleted, err, ccp))->start();
			return true;
		} catch(...){}

		return false;
	}

protected:

	CacheSinkDispatcher(ZQ::common::NativeThreadPool& thpool, bool bAboutWrite, DiskCacheSink* sink, DiskCacheSink::Error errCode, CacheCopy::Ptr ccopy)
		: ZQ::common::ThreadRequest(thpool), _bAboutWrite(bAboutWrite), _sink(sink), _errCode(errCode), _ccopy(ccopy)
	{
	}

	bool _bAboutWrite;
	DiskCacheSink* _sink;
	DiskCacheSink::Error _errCode;
	CacheCopy::Ptr _ccopy;

protected: // impl of ThreadRequest

	virtual int run(void)
	{
		if (!_sink)
			return -1;

		if (_bAboutWrite)
		{
			_sink->onCacheWrote(_errCode/*, _ccopy->_originKey.url*/);
			_ccopy = NULL;
			return 0;
		}

		_sink->onCacheRead(_errCode, _ccopy->_contentBody);
		_ccopy = NULL;
		return 0;
	}

	virtual void final(int retcode =0, bool bCancelled =false)
	{
		delete this;
	}
};

// -----------------------------
// class CacheLoader
// -----------------------------
int CacheLoader::run()
{
	int32 nextSleep = WAIT_INTERVAL_MIN;

	while (!_dir._bQuit)
	{
		if (nextSleep <  WAIT_INTERVAL_MIN)
			nextSleep = WAIT_INTERVAL_MIN;
		_dir._eventReadWorkers.wait(nextSleep);
		
		nextSleep = WAIT_INTERVAL_MAX;

		std::string subpath, fullpath; // full pathname of the file to read
		CacheDir::AwaitIOSink cacheAios;
		BufferUser bu;
		std::string urlOfBu;
		CacheDir::NotificationList sinksToExpire;
		std::map <std::string, uint32 > attentionCounts;
		int64 stampStart = ZQ::common::now();

		// step 1. scan the _awaitIOSinks for next file to load, identified by subpath
		//  during which also
		//    a) clean the expired sinks
		//    b) clean the orphan buffers by attentionCounts
		{
			ZQ::common::MutexGuard g(_dir._lkAwaitIOSinks);
			if (_dir._bQuit || _dir._awaitIOSinks.size() <=0)
			{
				continue;
			}

			int64 stampExp = stampStart - _dir._timeout;

			// take the first item in the queue
			//CacheDir::AwaitIOSinkList::iterator it = _dir._awaitIOSinks.begin();

			for (CacheDir::AwaitIOSinkList::iterator it = _dir._awaitIOSinks.begin(); it < _dir._awaitIOSinks.end();)
			{
				//_dir._log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheLoader, "start to do ioasync[%ld] awaitiosinks size[%d] subPath[%s]."), it->reqBufId, _dir._awaitIOSinks.size(), it->subPath.c_str() );
				int nextAttention = (int) (it->stampSink - stampExp);
				if (nextAttention <= 0)
				{
					CacheDir::Notification n;
					n.sink = *it;
#ifndef USERBUF_PER_READ_REQUEST
					CacheDir::AwaitBufMap::iterator itBuf = _dir._awaitReadBufMap.find(it->subPath);
					if (_dir._awaitReadBufMap.end() != itBuf)
						n.sink.bu  = itBuf->second;
#endif // !USERBUF_PER_READ_REQUEST

					sinksToExpire.push_back(n);
					_dir._log.debug(CLOGFMT(CacheLoader,"dir[%s] bufReqID[%ld] skip due to expiration subPath[%s]."), _dir.path().c_str(), it->reqBufId, it->subPath.c_str() );
					it = _dir._awaitIOSinks.erase(it);
					continue;
				}

				if (nextAttention >0 && nextAttention < nextSleep)
					nextSleep = nextAttention;

				do 
				{
					if (!fullpath.empty())
					{
						_dir._log.debug(CLOGFMT(CacheLoader,"dir[%s] subPath[%s] bufReqID[%ld] skip due to empty fullpath"), _dir.path().c_str(), it->subPath.c_str(), it->reqBufId);
						break;
					}

					CacheDir::LoaderMap::iterator itLoader = _dir._busyLoaders.find(it->subPath);
					if (it->bToWrite || _dir._busyLoaders.end() != itLoader )
					{
						_dir._log.debug(CLOGFMT(CacheLoader,"dir[%s] subPath[%s] bufReqID[%ld] skip loading because it is being loaded by th[%d]"), 
								_dir.path().c_str(), it->subPath.c_str(), it->reqBufId, itLoader->second );
						break; // a worker is already reading the same file, skip
					}

					// double check if the unique BufferUser is available
					CacheDir::AwaitBufMap::iterator itBuf = _dir._awaitReadBufMap.find(it->subPath);
					if (_dir._awaitReadBufMap.end() == itBuf)
					{
						_dir._log(ZQ::common::Log::L_WARNING, CLOGFMT(CacheLoader, "dir[%s] file[%s] sink[%p] bufReqID[%ld] has no await buf associated"),
								_dir.path().c_str(), it->subPath.c_str(), it->cb, it->reqBufId);
						break; // invalid because no BufferUser
					}

					subpath = it->subPath;
					fullpath = _dir.path() + subpath;
					bu = itBuf->second;
					urlOfBu = it->url;
					cacheAios = *it;
					_dir._busyLoaders[subpath] = (int)ZQ::common::getthreadid();
					_dir._log.debug(CLOGFMT(CacheLoader,"add subpath[%s] into busy loader, reqId[%ld]"), subpath.c_str(), bu.reqId() );

				} while (0);

				if (attentionCounts.end() == attentionCounts.find(it->subPath))
					attentionCounts.insert(std::make_pair<std::string, uint32 > (it->subPath, 1));
				else attentionCounts[it->subPath]++;

				it++;
			}

			// clean awaitReadBuf orphans
			int cOrphanBuf =0;
			std::string strOrphanBuf;
			for (CacheDir::AwaitBufMap::iterator itBuf = _dir._awaitReadBufMap.begin(); itBuf != _dir._awaitReadBufMap.end();)
			{
				if (attentionCounts.end() == attentionCounts.find(itBuf->first))
				{
					strOrphanBuf += itBuf->first + ",";
					_dir._awaitReadBufMap.erase(itBuf++);
					cOrphanBuf++;
					continue;
				}

				itBuf++;
			}

			_dir._log((cOrphanBuf>0) ? ZQ::common::Log::L_WARNING : ZQ::common::Log::L_DEBUG, CLOGFMT(CacheLoader, "dir[%s] found %d expired, %d await-bufs and %d sinks, focus-file[%s], cleaned %d orphan bufs: %s"), _dir.path().c_str(), 
				(int)sinksToExpire.size(), (int)_dir._awaitReadBufMap.size(), (int)_dir._awaitIOSinks.size(), subpath.c_str(), cOrphanBuf, strOrphanBuf.c_str());

		} // release the locker

		// step 2. trigger the notifications to the expired sinks
		int cExp =0;
		for (CacheDir::NotificationList::iterator it = sinksToExpire.begin(); !_dir._bQuit && it < sinksToExpire.end(); it++)
		{
			CacheCopy::Ptr ccopy = new CacheCopy();
			ccopy->_exchangeKey.stampAsOfDisk = 0;
#ifndef USERBUF_PER_READ_REQUEST
			ccopy->_contentBody = it->bu;
#else
			ccopy->_contentBody = it->sink.bu;
#endif // USERBUF_PER_READ_REQUEST
			_dir._log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheLoader,"dir[%s] expiring bufReqID[%ld]"), _dir.path().c_str(), ccopy->_contentBody.reqId() );
			if (CacheSinkDispatcher::schedule(_dir._thpool, it->sink.bToWrite, it->sink.cb, DiskCacheSink::cacheErr_Timeout, ccopy))
				cExp++;
		}

		if (sinksToExpire.size() >0)
			_dir._log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheLoader, "dir[%s] expired [%d/%d] sinks."), _dir.path().c_str(), cExp, (int)sinksToExpire.size());

		if (_dir._bQuit || fullpath.empty())
		{
			_dir._log.debug( CLOGFMT(CacheLoader,"dir[%s] bufReqID[%ld] skip due to empty fullpath"), _dir.path().c_str(), bu.reqId());
			continue;
		}

		// step 2. do the file reading
		CacheCopy::Ptr ccp = new CacheCopy();
		ccp->_exchangeKey.stampAsOfDisk = 0;
		if(!bu.valid())
		{
			assert(false && "get bad bu");
		}

		//if (!tmpBU)
		//{
		//	tmpBU = CacheDir::_cacheCenter->makeEmptyBufUser();
		//}
		//ccp->_contentBody = tmpBU; // ccp->_contentBody = bu;
		ccp->_contentBody = CacheDir::_cacheCenter->makeEmptyBufUser();

		_dir._log.debug(CLOGFMT(CacheLoader, "dir[%s] loading file[%s] with bufReqID[%ld]"), _dir.path().c_str(), subpath.c_str(), bu.reqId() );
		CacheCopy::Error errLoad = ccp->load(fullpath, ccp->_contentBody);

		if (CacheCopy::eOK != errLoad)
		{
			ccp->_originKey.url = ""; // this will lead to notify with cacheErr_Missed
			ccp->_exchangeKey.pathName = "";
			// ccp->_contentBody.setDataSize(0);
			_dir._log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheLoader, "dir[%s] file[%s] failed to load, bufReqID[%ld] err(%d)"), _dir.path().c_str(), subpath.c_str(), bu.reqId(), errLoad);
		}
		else
		{
			_dir._log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheLoader, "dir[%s] file[%s] loaded, adding to memory cache [%p]"), _dir.path().c_str(), subpath.c_str(), ccp.get());

			// insert into the LRUMap
			if (!_dir._bQuit)
			{
				ZQ::common::MutexGuard g(_dir._lkCacheCopies);
				if(ccp)
					_dir._cacheCopies[subpath] = ccp;
				_dir._log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheLoader, "dir[%s] file[%s] of url[%s] loaded, added to memory cache"), _dir.path().c_str(), subpath.c_str(), ccp->_originKey.url.c_str());
			}

#ifndef USERBUF_PER_READ_REQUEST
			// double check if the received buffer is same of the requested
			if (ccp->_contentBody && ccp->_originKey.url == urlOfBu)
			{
				bu.copyFrom(ccp->_contentBody);
				ccp->_contentBody = NULL;
			}
#endif // USERBUF_PER_READ_REQUEST
		}

		int durLoad = (int)(ZQ::common::now() - stampStart);

		// step 3. notify those sinks about the file
		CacheDir::NotificationList sinksToNotify;
		std::string buIds;
		
		{
			ZQ::common::MutexGuard g(_dir._lkAwaitIOSinks);

			if (_dir._bQuit)
				break;

			for (CacheDir::AwaitIOSinkList::iterator it = _dir._awaitIOSinks.begin(); it < _dir._awaitIOSinks.end(); )
			{
				if (it->bToWrite || it->subPath != subpath)
				{
					// ignore the sinks not about this reading
					it++;
					continue;
				}

				CacheDir::Notification n;
				n.sink = *it;
				char tmp[20];
#ifdef USERBUF_PER_READ_REQUEST
				snprintf(tmp, sizeof(tmp), "%ld,", n.sink.bu.reqId());
#else
				n.bu = bu;
				snprintf(tmp, sizeof(tmp), "%ld,", bu.reqId());
#endif // USERBUF_PER_READ_REQUEST
				buIds += tmp;

				//if (tmpBU)
				//{
				//	bu.copyFrom(tmpBU);
				//	tmpBU = NULL; // no more useful as it has been duplicated to bu
				//}

				sinksToNotify.push_back(n);
				it = _dir._awaitIOSinks.erase(it);
			}

			_dir._busyLoaders.erase(subpath);
			if (_dir._awaitIOSinks.size() >0)
				nextSleep = WAIT_INTERVAL_MIN;
			_dir._awaitReadBufMap.erase(subpath);
		} // release the locker

		_dir._log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheLoader, "dir[%s] file[%s] of url[%s] loaded[%c] err(%d), found %d sinks, leaving %d await-bufs and %d sinks, bufReqID[%s]"), _dir.path().c_str(), 
			subpath.c_str(), ccp->_originKey.url.c_str(), (CacheCopy::eOK == errLoad) ? 'T' : 'F', (int)errLoad, (int)sinksToNotify.size(), (int)_dir._awaitReadBufMap.size(), (int)_dir._awaitIOSinks.size(), buIds.c_str());

		int cNoticeHit =0 , cNoticeMissed = 0;
		for (CacheDir::NotificationList::iterator it = sinksToNotify.begin(); !_dir._bQuit && it < sinksToNotify.end(); it++)
		{
			CacheCopy::Ptr tmpccp = new CacheCopy();
			tmpccp->_exchangeKey = ccp->_exchangeKey;
			tmpccp->_originKey = ccp->_originKey;
			tmpccp->_cntAccess = ccp->_cntAccess;
#ifdef USERBUF_PER_READ_REQUEST
			tmpccp->_contentBody = it->sink.bu;
#else
			tmpccp->_contentBody = bu;
#endif // USERBUF_PER_READ_REQUEST

			if (ccp->_originKey.url == it->sink.url && ccp->_contentBody)
			{
#ifdef USERBUF_PER_READ_REQUEST
				tmpccp->_contentBody.copyFrom(ccp->_contentBody);
#endif // USERBUF_PER_READ_REQUEST

				_dir._log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheLoader,"dir[%s] url[%s] bufReqID[%ld] scheduling HIT notification"), 
				_dir.path().c_str(), ccp->_originKey.url.c_str(), tmpccp->_contentBody.reqId());
				if (CacheSinkDispatcher::schedule(_dir._thpool, false, it->sink.cb, DiskCacheSink::cacheErr_Hit, tmpccp))
					cNoticeHit++;
			}
			else
			{
				_dir._log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheLoader,"dir[%s] url[%s] bufReqID[%ld] scheduling MISSED notification"), 
					_dir.path().c_str(), ccp->_originKey.url.c_str(), tmpccp->_contentBody.reqId());
				if (CacheSinkDispatcher::schedule(_dir._thpool, false, it->sink.cb, DiskCacheSink::cacheErr_Missed, tmpccp))
					cNoticeMissed++;
			}
		}

		_dir._log((durLoad > 200) ? ZQ::common::Log::L_WARNING : ZQ::common::Log::L_INFO, CLOGFMT(CacheLoader, "dir[%s] file[%s]=>url[%s] loaded[%c], notified (%dH+%dM)/%d sinks, expired %d/%d sinks, took %d/%dmsec ccp ref[%p=>%d]."), _dir.path().c_str(),
			subpath.c_str(), ccp->_originKey.url.c_str(), (CacheCopy::eOK == errLoad)?'T':'F', cNoticeHit, cNoticeMissed, (int)sinksToNotify.size(), cExp, (int)sinksToExpire.size(), durLoad, (int) (ZQ::common::now() - stampStart), ccp._ptr, ccp->__getRef());

		ccp = NULL; // no more using this local var, it should has already be in _dir._cacheCopies[]
	} // while (!_dir._bQuit)

	return 0;
}

// -----------------------------
// class CacheSaver
// -----------------------------
int CacheSaver::run()
{
	int64 stampLastFlush = 0;
	std::string lastPathOfCounter;
	int countersToFlush =0;

	while (!_dir._bQuit)
	{
#pragma message ( __MSGLOC__ "TODO: CacheSaver pass access counters to _counterFlow")
#ifdef ENABLE_COUNTERFLOW
		if (countersToFlush >0)
		{
			_dir._log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheLoader, "submitting %d access counters to CountFlow start from subPath[%s]"), countersToFlush, lastPathOfCounter.c_str());
			ZQ::common::MutexGuard g(_dir._lkCacheCopies);
			CacheCopy::LRUMap::iterator it = _dir._cacheCopies.lower_bound(lastPathOfCounter);
			if (_dir._cacheCopies.end() == it)
				it = _dir._cacheCopies.begin();
			bool bSpinned = false;
			while (countersToFlush)
			{
				if (it->second->_cntAccess.c >0)
				{
					lastPathOfCounter = it->first;
					if (it->second->_exchangeKey.stampAsOfDisk >0)
						_dir._counterFlow.countHit(lastPathOfCounter, it->second->_cntAccess);
					else _dir._counterFlow.countMissed(lastPathOfCounter, it->second->_cntAccess);
					
					countersToFlush--;
				}

				if (++it == _dir._cacheCopies.end())
				{
					countersToFlush--; // dummy decreasing to avoid dead loop if _cacheCopies has nothing to submit

					if (bSpinned)
						countersToFlush =0;
					else
					{
						bSpinned = true;
						it = _dir._cacheCopies.begin();
					}
				}
			}
		}

#endif // ENABLE_COUNTERFLOW

		countersToFlush =0; // reset countersToStepSubmit anyway
		
		int32 nextSleep = FLUSH_INTERVAL_DEFAULT;
		if (_dir._flushInterval >0)
			nextSleep = (int32) (stampLastFlush + _dir._flushInterval - ZQ::common::now());

		if (nextSleep > FLUSH_INTERVAL_MAX)
			nextSleep = FLUSH_INTERVAL_MAX;

		if (nextSleep < FLUSH_INTERVAL_MIN)
			nextSleep = FLUSH_INTERVAL_MIN;

		_dir._eventWriteWorkers.wait(nextSleep);

		// nextSleep = WAIT_INTERVAL_MAX;

		std::string subpath, fullpath; // full pathname of the file to read
		int64 stampStart = ZQ::common::now();
		CacheCopy::Ptr ccp;

		{
			ZQ::common::MutexGuard g(_dir._lkCacheCopies);
			if (_dir._bQuit)
				continue;

			if (_dir._dirtyCopies.empty())
			{
#pragma message ( __MSGLOC__ "TODO: when CacheSaver idle, determin how many access counters to pass to _counterFlow")
#ifdef ENABLE_COUNTERFLOW
				CounterFlow::attenuate(_dir._cntDirAccess, stampStart - DIR_ACCESS_COUNT_TIME_WIN);
				int c = _dir._cntDirAccess.c;
				c = DIR_ACCESS_COUNTS_SUBMIT_MAX - _dir._cntDirAccess.c;

				while (c >0)
				{
					c >>=2;
					countersToFlush +=10;
				}
#endif // ENABLE_COUNTERFLOW

				continue;
			}

			// step 1. pop from _dirtyCopies
			subpath = _dir._dirtyCopies.front();
			_dir._dirtyCopies.pop();
			// step 2. finding the CacheCopy in the memory
			if(_dir._cacheCopies.find(subpath) != _dir._cacheCopies.end() )
				ccp =  _dir._cacheCopies[subpath];

			if (!ccp)
				continue;

			//if (!_dir._dirtyCopies.empty())
			//	nextSleep = WAIT_INTERVAL_MIN;
		}
		
		// step 3. do the flushing
		_dir._log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheSaver,"run() saving file[%s]=>url[%s], ccp[%p=>%d]"), subpath.c_str(), ccp->_originKey.url.c_str(), ccp.get(), ccp->__getRef());
		stampLastFlush = ZQ::common::now();
		CacheCopy::Error errFlush = ccp->flush(); 
		int64 stampFlushed = ZQ::common::now();
		int durFlush = (int)(stampFlushed - stampStart);

		if (CacheCopy::eOK != errFlush)
			_dir._log(ZQ::common::Log::L_ERROR, CLOGFMT(CacheSaver, "dir[%s] failed to flush file[%s]=>url[%s], took %dmsec err(%d)"), _dir.path().c_str(), subpath.c_str(), ccp->_originKey.url.c_str(), durFlush, errFlush);
#ifdef _DEBUG
		else
			_dir._log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheSaver, "dir[%s] flushed file[%s]=>url[%s], took %dmsec"), _dir.path().c_str(), subpath.c_str(), ccp->_originKey.url.c_str(), durFlush);
#endif // _DEBUG

		// step 4. scan await IO sinks for expired and those-to-notify
		int64 stampExp = stampFlushed - _dir._timeout;
		CacheDir::AwaitIOSinkList sinksToExpire, sinksToNotify;

		if(0)
		{
			ZQ::common::MutexGuard g(_dir._lkAwaitIOSinks);

			// take the first item in the queue
			for (CacheDir::AwaitIOSinkList::iterator it = _dir._awaitIOSinks.begin(); !_dir._bQuit && it < _dir._awaitIOSinks.end(); )
			{
				int nextAttention = (int) (it->stampSink - stampExp);
				if (nextAttention <= 0)
				{
					sinksToExpire.push_back(*it);
					it = _dir._awaitIOSinks.erase(it);
					continue;
				}

				if (nextAttention >0 && nextAttention < nextSleep)
					nextSleep = nextAttention;

				if (!it->bToWrite || it->subPath != subpath)
				{
					it++;
					continue;
				}

				sinksToNotify.push_back(*it);
				it = _dir._awaitIOSinks.erase(it);
			}
		} // release the locker

#ifdef _DEBUG
		_dir._log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheSaver, "dir[%s] file[%s]=>url[%s] found %d to-notify, %d expired"),
						_dir.path().c_str(), subpath.c_str(), ccp->_originKey.url.c_str(), (int)sinksToNotify.size(), (int)sinksToExpire.size());
#endif // _DEBUG
		int cNotified = 0, cExpired = 0;
		for (CacheDir::AwaitIOSinkList::iterator it = sinksToNotify.begin(); !_dir._bQuit && it < sinksToNotify.end(); it++)
		{
			// determin the errCode
			DiskCacheSink::Error err = DiskCacheSink::cacheErr_StoreFail;
			if (CacheCopy::eOK == errFlush)
			{
				err = DiskCacheSink::cacheErr_OK;
				if ((ccp->_originKey.url != it->url))
					err = DiskCacheSink::cacheErr_SwapMissed;
			}

			if (CacheSinkDispatcher::schedule(_dir._thpool, true, it->cb, err, ccp))
				cNotified++;
		}

		for (CacheDir::AwaitIOSinkList::iterator it = sinksToExpire.begin(); !_dir._bQuit && it < sinksToExpire.end(); it++)
		{
			// CacheCopy::Ptr ccopy = new CacheCopy();
			// ccopy->_exchangeKey.stampAsOfDisk = 0;
			// ccopy->_contentBody = it->bu;
			if (CacheSinkDispatcher::schedule(_dir._thpool, true, it->cb, DiskCacheSink::cacheErr_Timeout, NULL))
				cExpired++;
		}

		_dir._log((durFlush > 200) ? ZQ::common::Log::L_WARNING : ZQ::common::Log::L_INFO, CLOGFMT(CacheSaver, "dir[%s] file[%s]=>url[%s] flushed, notified %d/%d sinks, expired %d/%d sinks, buf[%ld] took %d/%dmsec ccp ref[%p=>%d]"),
			_dir.path().c_str(), subpath.c_str(), ccp->_originKey.url.c_str(), cNotified, (int)sinksToNotify.size(), cExpired, (int)sinksToExpire.size(),
			ccp->_contentBody.reqId(), durFlush, (int)(ZQ::common::now() - stampStart), ccp._ptr, ccp->__getRef());
		
		// no matter if there are external evict policy, some file must be evicted if the diskspace is about empty
		uint64 totalMB=0, freeMB=0;
		if(_dir.getSpace(totalMB, freeMB) && freeMB < totalMB /10)
		{
			_dir._log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheSaver, "start to evict cache[%ldMB/%ldMB]."), freeMB, totalMB);
			uint32 expectedMB = 2; // 2MB by default
			if (ccp->_contentBody)
				expectedMB = ( ccp->_contentBody.dataSize() + sizeof(CacheCopy::Header)) * 2/1024/1024; // twice of just wrote

			if (expectedMB <2)
				expectedMB =2;

			_dir.evictByFileToSwap(subpath, expectedMB);
		}
		ccp=NULL;
	} // while (!_dir._bQuit)

	return 0;
}

// -----------------------------
// class CacheDir
// -----------------------------
// a CacheDir may maps to a mount point of a disk
bool CacheDir::_calcHashKey(hkey_t& hkey, const char* buf, uint len)
{
	if (NULL == buf)
		return false;

	if (len <=0)
		len = strlen(buf);

	memset(&hkey, 0x00, sizeof(hkey));

	ZQ::common::md5 encoder;
	encoder.Update((unsigned char*)buf, len);
	encoder.Finalize();
	memcpy(&hkey, encoder.Digest(), sizeof(uint64)*2);

	return true;
}

uint64 CacheDir::_distance(const hkey_t& hkey1, const hkey_t& hkey2)
{
	uint64 sqrSum =0;
	for (int i= (sizeof(hkey_t) / sizeof(uint16)) -1; i>=0; i--)
	{
		int32 diff = hkey1.w[i] - hkey2.w[i];
		sqrSum += diff*diff;
	}

	return sqrSum;
}

uint64 CacheDir::distance(const std::string& url) const
{
	hkey_t hkey;
	_calcHashKey(hkey, url.c_str(), url.length());
	return distance(hkey); 
}

CacheDir::CacheDir(ZQ::common::Log& log, const std::string& homePath, uint64 mbQuota, int threadsLoad, int threadsFlush)
: _log(log), _homePath(homePath), _mbQuota(mbQuota), _threadsLoad(threadsLoad), _threadsFlush(threadsFlush),
  _cdirsTop(SUBDIR_TOP_MIN), _cdirsLeaf(SUBDIR_LEAF_MIN), _flushInterval(FLUSH_INTERVAL_DEFAULT)
{
	if (_threadsLoad < IO_THREADS_MIN)
		_threadsLoad = IO_THREADS_MIN;
	if (_threadsFlush < IO_THREADS_MIN)
		_threadsFlush = IO_THREADS_MIN;

	if (FNSEPC != _homePath[_homePath.length()-1])
		_homePath += FNSEPC;

	_buildDirs();

	int i, cLoaders =0, cFlushers =0;
	for (i =0; i <_threadsLoad; i++)
	{
		try {
			CacheLoader::Ptr worker = new CacheLoader(*this);
			worker->start();
			_loaders.push_back(worker);
			cLoaders++;
		}
		catch(...) {}
	}

	for (i =0; i <_threadsFlush; i++)
	{
		try {
			CacheSaver::Ptr worker = new CacheSaver(*this);
			worker->start();
			_flushers.push_back(worker);
			cFlushers++;
		}
		catch(...) {}
	}
	//setLRUsize(800);
	setMaxPending(8, 8);
	_timeout = 3500; 

	_calcHashKey(_hkey, _homePath.c_str(), (uint) _homePath.length());

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheDir, "dir[%s] initialized %d/%d loaders, %d/%d flushers"), _homePath.c_str(), cLoaders, _threadsLoad, cFlushers, _threadsFlush);
	_threadsLoad = cLoaders;
	_threadsFlush = cFlushers;
}

CacheDir::~CacheDir()
{
}

uint64 CacheDir::distance(const hkey_t& hkeyURL) const
{
	return _distance(_hkey, hkeyURL); 
}

void CacheDir::_buildDirs()
{
#ifdef ZQ_OS_MSWIN
#  define mkdir(DIR) _mkdir(DIR)
#else
#  define mkdir(DIR) mkdir(DIR, 0x770)
#endif 
	for (int i=0; i < _cdirsTop; i++)
	{
		char buf[10];
		snprintf(buf, sizeof(buf)-2, "%02X" FNSEPS, i);
		std::string tdir = _homePath + buf;
		mkdir(tdir.c_str());
		for (int j=0; j < _cdirsLeaf; j++)
		{
			snprintf(buf, sizeof(buf)-2, "%02X" FNSEPS, j);
			mkdir((tdir +buf).c_str());
		}
	}

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheDir, "dir[%s] _buildDirs() %dx%d folder structure built"), _homePath.c_str(), _cdirsTop, _cdirsLeaf);
}

BufferUser CacheDir::load_async(const std::string& subPath, const std::string& fileName, const int64 offsetInFile, const int64 stampAsOfOrign, int64 bitrate, long opaqueData, DiskCacheSink* cbCompletion)
{
	std::string url = CacheDir::getURL(fileName, offsetInFile);
	CacheCopy::Ptr ccp = NULL;
	BufferUser bu;
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheDir, "dir[%s] load_async() [%s]=>[%s] cb[%p]"), _homePath.c_str(), url.c_str(), subPath.c_str(), cbCompletion);
	size_t memCacheSize = 0;

	{
		ZQ::common::MutexGuard g(_lkCacheCopies);
#ifdef ENABLE_COUNTERFLOW
		CounterFlow::inc(_cntDirAccess);
#endif // ENABLE_COUNTERFLOW

		CacheCopy::LRUMap::iterator it = _cacheCopies.find(subPath);
		if( it != _cacheCopies.end() )
		{
			ccp = it->second;
			bu = ccp->_contentBody;
			//_cacheCopies[subPath] = ccp;
		}
		memCacheSize = _cacheCopies.size();
	}

	if(NULL != ccp && ccp->_contentBody.valid())
	{
#ifdef ENABLE_COUNTERFLOW
		CounterFlow::inc(ccp->_cntAccess);
#endif // ENABLE_COUNTERFLOW

		//BufferUser user = ccp->_contentBody;
		if (ccp->_originKey.url == url)
		{
#ifdef USERBUF_PER_READ_REQUEST
			/*
			bu = cbCompletion->getCacheBufUser(fileName, offsetInFile, bitrate, opaqueData);
			if( !bu.valid() )
			{
				assert(false && "CacheDir get bad bu from cachecenter.");
			}

			bu.copyFrom(ccp->_contentBody);
			*/
			BufferUser bu(ccp->_contentBody);
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheDir, "dir[%s] load_async() HIT in memory[%s]=>[%s] cb[%p], duplicated bufReqID[%ld] to bufReqID[%ld]"), 
				_homePath.c_str(), url.c_str(), subPath.c_str(), cbCompletion, ccp->_contentBody.reqId(), bu.reqId());
#else
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheDir, "dir[%s] load_async() HIT in memory[%s]=>[%s] cb[%p], bufReqID[%ld]"), _homePath.c_str(), url.c_str(), subPath.c_str(), cbCompletion, bu.reqId());
#endif // USERBUF_PER_READ_REQUEST

#ifdef READ_INSTANT_CALLBACK
			if (cbCompletion)
			{
				cbCompletion->onCacheRead(DiskCacheSink::cacheErr_Hit, bu);
			}
#else
			ccp = new CacheCopy();
			ccp->_contentBody = bu;
			CacheSinkDispatcher::schedule(_thpool, false, cbCompletion, DiskCacheSink::cacheErr_Hit, ccp);
#endif // READ_INSTANT_CALLBACK
			return bu;
		}

		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheDir, "dir[%s] load_async() swap-missed in memory [%s]=>[%s] cb[%p], cached-copy[%s]mem cached[%ld] ccp ref[%p=>%d]."), _homePath.c_str(), url.c_str(), subPath.c_str(), cbCompletion, ccp->_originKey.url.c_str(), memCacheSize, ccp._ptr, ccp->__getRef());

#ifdef READ_INSTANT_CALLBACK
		if (cbCompletion)
			cbCompletion->onCacheRead(DiskCacheSink::cacheErr_SwapMissed, bu);
#else
		ccp = new CacheCopy();
		ccp->_contentBody = bu;
		CacheSinkDispatcher::schedule(_thpool, false, cbCompletion, DiskCacheSink::cacheErr_SwapMissed, ccp);
#endif // READ_INSTANT_CALLBACK

		return bu;
	}

	BufferUser user = scheduleRead(subPath, fileName, offsetInFile, stampAsOfOrign, bitrate, opaqueData, cbCompletion);
	_log.debug(CLOGFMT(CacheDir, "dir[%s] load_async() not in memory [%s]=>[%s] cb[%p], scheduled to load mem cache[%ld]."), _homePath.c_str(), url.c_str(), subPath.c_str(), cbCompletion, memCacheSize);
	//_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheDir, "dir[%s] read() not in memory [%s]=>[%s] cb[%p], scheduled to load"), _homePath.c_str(), url.c_str(), subPath.c_str(), cbCompletion);
	return user;
}

BufferUser CacheDir::scheduleRead(const std::string& subPath, const std::string& fileName, const int64 offsetInFile, const int64 stampAsOfOrign, int64 bitrate, long opaqueData, DiskCacheSink* cbCompletion)
{
	BufferUser bu;// = cbCompletion->getCacheBufUser(fileName, offsetInFile);
	AwaitIOSink aios;
	aios.bToWrite = false;
	aios.url = CacheDir::getURL(fileName, offsetInFile);
	aios.cb = cbCompletion;
	aios.stampSink = ZQ::common::now();
	aios.subPath = subPath;

	{
		ZQ::common::MutexGuard g(_lkAwaitIOSinks);
#ifdef USERBUF_PER_READ_REQUEST
		bu = aios.bu = cbCompletion->getCacheBufUser(fileName, offsetInFile, bitrate, opaqueData);
		if( !bu.valid() )
		{
			assert(false && "CacheDir get bad bu from cachecenter.");
		}

#else
		AwaitBufMap::iterator iter = _awaitReadBufMap.find(aios.subPath);
		if (_awaitReadBufMap.end() != iter )
		{
			bu = iter->second;
			if( !bu.valid() )
				assert(false && "CacheDir get bad bu in _awaitReadBufMap");
		}

		bu = cbCompletion->getCacheBufUser(fileName, offsetInFile, bitrate, opaqueData );
		if( !bu.valid() )
		{
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(AwaitBufMap,"dir[%s] failed to get valid buffer from cachecenter, all buffs maybe used."), path().c_str());
			return bu;
		}

		if ( _awaitReadBufMap.size() >= _maxLoadingCopies )
		{
			CacheCopy::Ptr ccp = new CacheCopy();
			ccp->_contentBody = bu;
			CacheSinkDispatcher::schedule(_thpool, false, cbCompletion, DiskCacheSink::cacheErr_SwapMissed, ccp);
			return bu;
		}
#endif // USERBUF_PER_READ_REQUEST

		if (_awaitReadBufMap.end() == _awaitReadBufMap.find(aios.subPath) )
			_awaitReadBufMap.insert(AwaitBufMap::value_type(aios.subPath, bu));

		aios.reqBufId = bu.reqId();
		_awaitIOSinks.push_back(aios);
		_eventReadWorkers.signal();
		// _log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheDir, "awaitReadBufMap-size[%d] awaitIOSinks-size[%u] bufReqID[%ld] subPath[%s]."),(int)_awaitReadBufMap.size(), _awaitIOSinks.size(), bu.reqId(), subPath.c_str());
	}

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheDir, "awaitReadBufMap-size[%d] awaitIOSinks-size[%u] bufReqID[%ld] start reading subPath[%s] for url[%s:%ld] "), (int)_awaitReadBufMap.size(), _awaitIOSinks.size(), bu.reqId(), subPath.c_str(), fileName.c_str(), bu.offsetInFile());
	return bu;
}

void CacheDir::update_async(const std::string& subPath, BufferUser buf, const std::string& url, int64 stampAsOfOrigin, DiskCacheSink* cbCompletion)
{
	CacheCopy::Ptr ccp = NULL;
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheDir, "dir[%s] write() [%s]=>[%s] cb[%p]"), _homePath.c_str(), url.c_str(), subPath.c_str(), cbCompletion);
	bool bDirtyCopyAdded =false;
	{
		ZQ::common::MutexGuard g(_lkCacheCopies);
		// step 1. locate the CacheCopy
		if( _cacheCopies.end() != _cacheCopies.find(subPath))
			ccp = _cacheCopies[subPath];
		else
		{
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheDir, "dir[%s] write() new cache-copy [%s]=>[%s] cb[%p]"), _homePath.c_str(), url.c_str(), subPath.c_str(), cbCompletion);
			ccp = new CacheCopy();
			ccp->_exchangeKey.pathName = path() + subPath;
			ccp->_originKey.stampAsOfOrigin =0;
			ccp->_exchangeKey.stampAsOfDisk =0;
			//_cacheCopies[subPath] = ccp;
		}

		if (ccp->_originKey.url != url || stampAsOfOrigin > ccp->_originKey.stampAsOfOrigin) // test if need to overwrite
		{
			if (ccp->_originKey.url.empty())
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheDir, "dir[%s] write() filling cache-copy [%s]=>[%s] cb[%p]"), _homePath.c_str(), url.c_str(), subPath.c_str(), cbCompletion);
			else
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheDir, "dir[%s] write() swapping cache-copy [%s]=>[%s] cb[%p], prev[%s]"), _homePath.c_str(), url.c_str(), subPath.c_str(), cbCompletion, ccp->_originKey.url.c_str());
			
			ccp->_originKey.url = url;

			ccp->_contentBody = buf;
			ccp->_originKey.stampAsOfOrigin = stampAsOfOrigin;
			ccp->_exchangeKey.stampAsOfDisk =0;
		}

		if (ccp->_exchangeKey.stampAsOfDisk <=0)
		{
			size_t c=0;
			while (_maxDirtyCopies >0 && _dirtyCopies.size() >= _maxDirtyCopies)
			{
				_dirtyCopies.pop();
				c++;
			}
			
			_dirtyCopies.push(subPath);
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheDir, "update_async() push buffer[%s] into dirtyCopies[%d/%d] by evicting [%d] dirties"),url.c_str(), (int)_dirtyCopies.size(), _maxDirtyCopies, c);
			bDirtyCopyAdded = true;
			_eventWriteWorkers.signal();
		}
	    _cacheCopies[subPath] = ccp;
	}

	DiskCacheSink::Error err = DiskCacheSink::cacheErr_OK;
	if (ccp->_exchangeKey.stampAsOfDisk >0) // cache copy has previous saved
		err = DiskCacheSink::cacheErr_Hit;
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheDir, "dir[%s] write() updated [%s]=>[%s] cb[%p]: %s, dirty-added[%c] cachesize[%d]"),
		_homePath.c_str(), url.c_str(), subPath.c_str(), cbCompletion, DiskCacheSink::errorToA(err), bDirtyCopyAdded ? 'T' : 'F', _cacheCopies.size());
	CacheSinkDispatcher::schedule(_thpool, true, cbCompletion, err, ccp);
}

uint32 CacheDir::evict(const std::string& url)
{
#pragma message ( __MSGLOC__ "TODO: impl evict()")
	return 0;
}

void  CacheDir::subDirs(uint8 topDirs, uint8 leafDirs)
{
	if(topDirs >0)
		_cdirsTop =topDirs; 
	if(leafDirs > 0)
		_cdirsLeaf =leafDirs;
	_buildDirs();
}

void  CacheDir::delBuffer() 
{
	//BufferUser user;
	int cacheC = 0;
	//int64 buffId = -1;
	{
		ZQ::common::MutexGuard guard(_lkCacheCopies); 
		cacheC = _cacheCopies.size();
		if( cacheC > 0)
			_cacheCopies.erase_eldest();//->_contentBody;
		//refC = ccp->__getRef();
		//buffId = ccp->_contentBody.reqId();
	}
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CacheDir, "delBuffer dir[%s] erase eldest buffer from LRUmap size[%d]."), _homePath.c_str(), cacheC);
}

// -----------------------------
// CacheDir Aggregator
// -----------------------------
// the main class the access the disk-cache that may manages multiple CacheDirs
bool CacheDir::_bQuit = false;
CacheDir::DirMap CacheDir::_dirMap;
ZQ::common::Mutex CacheDir::_lkDirMap;
ZQ::common::NativeThreadPool CacheDir::_thpool(20); // for callback dispatchers
C2Streamer::CacheCenter* CacheDir::_cacheCenter = NULL;
//ZQ::common::Event CacheDir::_eventWorkers;

void CacheDir::setCacheCenter( CacheCenter * cc ) {
	_cacheCenter = cc;
}

CacheDir::Ptr CacheDir::addCacheDir(ZQ::common::Log& log, const std::string& homePath, uint64 mbTotal, int threadsLoad, int threadsFlush, int pendingsToYield, uint32 timeout, int32 LRUSize)
{
#pragma message ( __MSGLOC__ "TODO: fixup rootPath")

	ZQ::common::MutexGuard g(_lkDirMap);
	DirMap::iterator it= _dirMap.find(homePath);
	if (_dirMap.end() != it && it->second)
		return it->second;

	Ptr dir = new CacheDir(log, homePath, mbTotal, threadsLoad, threadsFlush);
	if (!dir)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CacheDir, "dir[%s] failed to add"), dir->_homePath.c_str());
		return NULL;
	}
	dir->setTimeout(timeout);
	dir->setLRUsize(LRUSize);
	MAPSET(DirMap, _dirMap, homePath, dir);
	log(ZQ::common::Log::L_INFO, CLOGFMT(CacheDir, "dir[%s] added: quota[%llu]MB, [%d]loaders, [%d]flushers"), 
		dir->_homePath.c_str(), dir->_mbQuota, dir->_threadsLoad, dir->_threadsFlush);

	return dir;
}
	
CacheDir::Ptr CacheDir::locateCacheDir(const std::string& url, std::string& subPath)
{
	hkey_t keyURL;
	if (!_calcHashKey(keyURL, url.c_str(), (uint) url.length()))
		return NULL;

	subPath = "";
	Ptr dir;
	uint64 distMin = ~((uint64) 0);
	{
		ZQ::common::MutexGuard g(_lkDirMap);
		for(DirMap::iterator it= _dirMap.begin(); it != _dirMap.end() && it->second; it++)
		{
			uint64 dist = it->second->distance(keyURL);
			if (dist > distMin)
				continue;
			dir = it->second;
			distMin = dist;
		}
	}

	if (dir)
	{
		uint64 d = keyURL.q[0] ^ keyURL.q[1];
		uint8 tdir = (uint8) ((d & 0xff) % dir->_cdirsTop);
		uint8 ldir = (uint8) (((d>>8) & 0xff) % dir->_cdirsLeaf);
		char buf[32];
		snprintf(buf, sizeof(buf)-2, "%02X" FNSEPS "%02X" FNSEPS "%012lX", tdir, ldir, d>>16);
		subPath = buf;
	}

	return dir;
}
CacheDir::DirMap  CacheDir::listCacheDir()
{
	ZQ::common::MutexGuard g(_lkDirMap);
	return _dirMap;
}
void CacheDir::stop(void)
{
	_bQuit = true;
	ZQ::common::MutexGuard g(_lkDirMap);
	for(DirMap::iterator it= _dirMap.begin(); it != _dirMap.end() && it->second; it++)
	{
		it->second->_eventReadWorkers.signal();
		it->second->_eventWriteWorkers.signal();
	}
	//::Sleep(200);
	SYS::sleep(200);
}

void CacheDir::releaseBuffer()
{
	//int rNum = rand();
	Ptr dir;
	{
		ZQ::common::MutexGuard g(_lkDirMap);
		//int pos = rNum %  _dirMap.size();
		CacheDir::DirMap::iterator mIt = _dirMap.begin();
		for(;mIt != _dirMap.end();mIt ++)
		{
			dir =  mIt->second;
			if( dir != NULL )
				dir->delBuffer();
		} 
		/*
		for(int i = 0; i<pos; i++)
			mIt ++;
			
		dir =  mIt->second;
		*/
	}
	if( dir != NULL)
		dir->delBuffer();
}

BufferUser CacheDir::read_async( const std::string& fileName, const int64 offsetInFile, const int64 stampAsOfOrign, int64 bitrate, long opaqueData, DiskCacheSink* cbCompletion)
{
	// TODO: change to take async processing
	std::string url = CacheDir::getURL(fileName, offsetInFile);
	std::string subPath;
	Ptr dir = locateCacheDir(url, subPath);
	if (NULL == dir || subPath.empty())
	{
		BufferUser user = cbCompletion->getCacheBufUser(fileName, offsetInFile, bitrate, opaqueData);
		CacheCopy::Ptr ccp = new CacheCopy();
		ccp->_contentBody = user;
#ifdef READ_INSTANT_CALLBACK
		if (cbCompletion)
			cbCompletion->onCacheRead(DiskCacheSink::cacheErr_StoreFail, ccp->_contentBody);
#else
		//CacheCopy::Ptr ccp = NULL;
		CacheSinkDispatcher::schedule(_thpool, false, cbCompletion, DiskCacheSink::cacheErr_StoreFail, ccp);
#endif // READ_INSTANT_CALLBACK

		//BufferUser  usr ;//= cbCompletion->makeEmptyBufUser();
		return user;
	}

	return dir->load_async(subPath, fileName, offsetInFile, stampAsOfOrign, bitrate, opaqueData, cbCompletion);
}

void CacheDir::write_async(BufferUser buf, int64 stampAsOfOrign, DiskCacheSink* cbCompletion)
{
	if(buf.dataSize() <= 0)
		return ;
	std::string subPath;
	//int64 stampAsOfOrigin = //ZQ::common::now();//buf.offsetInFile();
	std::string url = CacheDir::getURL(buf.fileName(), buf.offsetInFile());
	Ptr dir = locateCacheDir(url, subPath);
	CacheCopy::Ptr ccp = NULL;
	if (NULL == dir || subPath.empty())
	{
		CacheSinkDispatcher::schedule(_thpool, true, cbCompletion, DiskCacheSink::cacheErr_StoreFail, ccp);
		return;
	}

	dir->update_async(subPath, buf, url, stampAsOfOrign, cbCompletion);
}

std::string CacheDir::getURL(const std::string& fileName, const int64 offsetInFile)
{
	std::ostringstream os;
	os << fileName<< ":" << offsetInFile;
	return os.str();
}

#ifndef WIN32
static bool lessAtime( CacheDir::FILETIME ft1, CacheDir::FILETIME ft2)
{
		  //return memcmp(&d1, &d2, sizeof(d1)) <0;
		  return ft1.lastReadTime < ft2.lastReadTime;
		  //return true;
}

uint32 CacheDir::evictByFileToSwap(const std::string& subPathToSwap, uint32 expectedMB)
{
	std::string dir = dirOfFile(subPathToSwap);
	std::string fn  = subPathToSwap.substr(dir.length());

	DIR* dir_info;
	struct dirent* dir_entry;
	//std::vector <struct dirent> falist;
	FileLastTimeList    falist;
	dir_info = opendir((_homePath + dir).c_str());
	if (NULL == dir_info )
		return 0;

	while (NULL != (dir_entry = readdir(dir_info)))
	{
		if('.' == dir_entry->d_name[0] || 0 == fn.compare(dir_entry->d_name) || (8 != dir_entry->d_type) )
			continue;
		std::string filePath = _homePath + dir;
		filePath.append( dir_entry->d_name );
		struct stat  fileSt;
		if( 0!= stat( filePath.c_str() ,&fileSt))
			continue;
		FILETIME flt;
		flt.fileName.assign( dir_entry->d_name );
		flt.fileSize = fileSt.st_size;
		flt.lastReadTime = fileSt.st_atime;
		falist.push_back(flt);
	} // while

    closedir(dir_info);
	std::sort(falist.begin(), falist.end(), lessAtime);

	uint64 freedSize =0;
	uint64 expectedSize = expectedMB;
	expectedSize <<=20;
	std::string fns;
	for (size_t i =0; freedSize < expectedSize && i< falist.size(); i++)
	{
		unlink((_homePath + dir + falist[i].fileName).c_str());
		fns += std::string(falist[i].fileName) + ",";
		freedSize += falist[i].fileSize;//2*1024*1024; //todo: (falist[i].nFileSizeHigh <<32) + falist[i].nFileSizeLow;
	}

	_log.info(CLOGFMT(CacheDir, "dir[%s] sub[%s] evictByFileToSwap() %lluB free-ed for %uMB: %s"), 
		_homePath.c_str(), dir.c_str(), freedSize, expectedMB, fns.c_str());

	return ((freedSize>>10) +512)>>10;
}

bool CacheDir::getSpace(uint64& totalMB, uint64& freeMB)
{
	struct statfs diskInfo;
    if (0 !=statfs(_homePath.c_str(), &diskInfo))
	{	
		_log.debug(CLOGFMT(CacheDir, "failed to status[%s]."), _homePath.c_str());
		return false;
	}

    totalMB = (diskInfo.f_bsize * diskInfo.f_blocks) >>20; // total MB
    freeMB  = (diskInfo.f_bsize * diskInfo.f_bfree) >>20; // free MB
	float percent = (totalMB >0) ? (((float) freeMB *100) /totalMB) : 0.0;
	if (percent >100.0)
		percent = 100.0;

	_log.debug(CLOGFMT(CacheDir, "dir[%s] got disk size[%ldMB/%ldMB] %.1f%%free"), _homePath.c_str(), freeMB, totalMB, percent);
	return true;
}

#else
static bool lessAtime(WIN32_FIND_DATA faA, WIN32_FIND_DATA faB)
{
	return memcmp(&faA.ftLastAccessTime, &faB.ftLastAccessTime, sizeof(FILETIME)) <0;
}

uint32 CacheDir::evictByFileToSwap(const std::string& subPathToSwap, uint32 expectedMB)
{
	std::string dir = dirOfFile(subPathToSwap);
	std::string fn  = subPathToSwap.substr(dir.length());

	WIN32_FIND_DATA fa;
	std::vector <WIN32_FIND_DATA> falist;
	HANDLE hFind = FindFirstFile((_homePath + dir + "*").c_str(), &fa);
	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	do {
		if ('.' == fa.cFileName[0] || 0== fn.compare(fa.cFileName)) // ignore some unexpected files
			continue;

		falist.push_back(fa);
		if (TRUE != FindNextFile(hFind, &fa))
			break;
	} while(1);
	FindClose(hFind);

	std::sort(falist.begin(), falist.end(), lessAtime);
	uint64 freedSize =0;
	uint64 expectedSize = expectedMB;
	expectedSize <<=20;
	std::string fns;
	for (size_t i =0; freedSize < expectedSize && i< falist.size(); i++)
	{
		unlink((_homePath + dir + falist[i].cFileName).c_str());
		fns += falist[i].cFileName + ",";
		freedSize += (falist[i].nFileSizeHigh <<32) + falist[i].nFileSizeLow; // (fa.size /1024 +500)/1024; // round to MB
	}

	log(ZQ::common::Log::L_INFO, CLOGFMT(CacheDir, "dir[%s] sub[%s] evictByFileToSwap() %lldB free-ed for %dMB: %s"), 
		_homePath.c_str(), dir.c_str(), freedSize, expectedMB, fns.c_str());

	return ((freedSize>>10) +512)>>10;
}

bool CacheDir::getSpace(uint64& totalMB, uint64& freeMB)
{
	totalMB = 1000;
	freeMB  =  960;
	return true;
}

#endif // WIN32

std::string CacheDir::dirOfFile(const std::string& filepath)
{
	if (FNSEPC == filepath[filepath.length() -1] || LOGIC_FNSEPC == filepath[filepath.length() -1])
		return filepath;

	size_t pos = filepath.find_last_of(FNSEPS LOGIC_FNSEPS);
	if (pos >=0)
		return filepath.substr(0, pos) + FNSEPS;
	return "";
}

}} // namespace
