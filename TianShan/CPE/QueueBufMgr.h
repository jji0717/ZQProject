

#ifndef _QUEUE_BUFFER_MANAGER_
#define _QUEUE_BUFFER_MANAGER_

#include "Locks.h"
#include "IMemAlloc.h"
#include <deque>

namespace ZQ{
	namespace common{
		class Log;
	}
}

class QueueBufMgr : public IMemAlloc
{
public:
	QueueBufMgr();
	virtual ~QueueBufMgr();
	
	int getAvailableSize();
	int getTotalSize();
	int getUsedSize();

	void setBufferSize(int nBufferBlockSize)
	{
		_nBufferSize = nBufferBlockSize;
	}

	void setMaxSize(int nMaxSize)
	{
		_nMaxSize = nMaxSize;
	}

	void setLog(ZQ::common::Log* pLog);

public:
	virtual void* alloc(size_t size);

	virtual void free(void* ptr);

protected:
	ZQ::common::Mutex	_lock;
	std::deque<void*>	_ioBuffers;	
	std::deque<void*>	_allIOBufs;	
	int					_nMaxSize;
	int					_nBufferSize;
//	ZQ::common::Log*	_log;
};

#endif
