

#include "QueueBufMgr.h"
#include "Log.h"
#include "SystemUtils.h"

using namespace ZQ::common;

#define MOLOG	(glog)
QueueBufMgr::QueueBufMgr()
{
//		_log = &ZQ::common::NullLogger;
	_nMaxSize = 0;
	_nBufferSize = 65536;
}


QueueBufMgr::~QueueBufMgr()
{
	void* buf;
	
	ZQ::common::Guard<ZQ::common::Mutex> op(_lock);
	
	while(!_allIOBufs.empty())
	{
		buf = _allIOBufs.front();
		
		if (buf)
		{
			free(buf);
		}
		
		_allIOBufs.pop_front();
	}
}

int QueueBufMgr::getAvailableSize()
{
	Guard<Mutex> op(_lock);
	return _ioBuffers.size();
}

int QueueBufMgr::getTotalSize()
{
	Guard<Mutex> op(_lock);
	return _allIOBufs.size();
}

void* QueueBufMgr::alloc(size_t size)
{
	void* ioBuf;
	
	// get buffer from queue if have
	_lock.enter();
	
	if (_ioBuffers.size())
	{
		ioBuf = _ioBuffers.front();
		_ioBuffers.pop_front();
	}
	else
	{
		if (!_nMaxSize || _allIOBufs.size() < (size_t)_nMaxSize)
		{
			ioBuf = ::malloc(size);
			if (ioBuf)
			{
				_allIOBufs.push_back(ioBuf);
			}
			else
			{
				MOLOG(Log::L_ERROR, CLOGFMT(QueueBufMgr, "malloc() failed to allocate a buffer with size[%d], error[%d], total allocated[%d]"),
					size, SYS::getLastErr(), _allIOBufs.size());

#ifdef ZQ_OS_MSWIN
				// Memory
				MEMORYSTATUS ms;
				memset(&ms, 0x00, sizeof(ms));
				ms.dwLength = sizeof(ms);
				::GlobalMemoryStatus(&ms);
				MOLOG(Log::L_INFO, CLOGFMT(QueueBufMgr, "physical memory: free[%u] total[%u], virtual memory: free[%u] total[%u]"),
					(unsigned int)(ms.dwAvailPhys), (unsigned int)(ms.dwTotalPhys), (unsigned int)(ms.dwAvailVirtual), (unsigned int)(ms.dwTotalVirtual));
#endif
			}
		}
		else
		{
			ioBuf = 0;

			MOLOG(Log::L_ERROR, CLOGFMT(QueueBufMgr, "failed to alloc(), max buffer queue size[%d] exceeded"), _nMaxSize);
		}
	}

	_lock.leave();
	
	return ioBuf;
}

void QueueBufMgr::free(void* ptr)
{
	if (!ptr)
	{
		MOLOG(Log::L_WARNING, CLOGFMT(QueueBufMgr, "free() a buffer with NULL pointer"));
		return;
	}

	_lock.enter();
	_ioBuffers.push_front(ptr);
	_lock.leave();
}
void QueueBufMgr::setLog( ZQ::common::Log* pLog )
{
	ZQ::common::setGlogger(pLog);
// 	if (pLog)
// 	{
// 		_log = pLog;
// 	}
}

int QueueBufMgr::getUsedSize()
{
	Guard<Mutex> op(_lock);
	return _allIOBufs.size() - _ioBuffers.size();
}
