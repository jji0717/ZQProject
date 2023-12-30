// BufferManager.cpp: implementation of the BufferManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BufferManager.h"
#include <typeinfo>

// #define _FAST_BUFFER_POOL

namespace DataStream {

//////////////////////////////////////////////////////////////////////////

#ifndef _FAST_BUFFER_POOL

class MainBufferBlock: public BufferBlock {
public:
	MainBufferBlock(void* ptr, size_t size):
	  BufferBlock(ptr, size)
	{
		_pos = 0;
		_ref = 1;
	}

	virtual void release()
	{
		long r = InterlockedDecrement(&_ref);
		if (r == 0) {
			free(_ptr);
			delete this;
		}
	}

	BufferBlock* getSubBlock(size_t size);
	
protected:
	long refer()
	{
		return InterlockedIncrement(&_ref);
	}

protected:
	unsigned int	_pos;
	long			_ref;
};

class SubBufferBlock: public BufferBlock {
public:
	SubBufferBlock(MainBufferBlock& mainBlock, void* ptr, size_t size):
	  BufferBlock(ptr, size), _mainBlock(mainBlock)
	{

	}

	virtual ~SubBufferBlock()
	{

	}
	
	virtual void release()
	{
		_mainBlock.release();
		delete this;
	}

protected:
	MainBufferBlock&	_mainBlock;
};


BufferBlock* MainBufferBlock::getSubBlock(size_t size)
{
	assert(_pos < _size);
	BYTE* ptr = (BYTE* )getPtr();
	ptr += _pos;
	SubBufferBlock* subBlock = new SubBufferBlock(*this, ptr, size);		
	_pos += size;	
	refer();
	return subBlock;
}

BufferPool::BufferPool(BufferManager& mgr, size_t blockSize, 
					   size_t initBlocks, size_t maxBlocks): 
						_bufferMgr(mgr), _blockSize(blockSize), 
						_initBlocks(initBlocks), _maxBlocks(maxBlocks)
{
	assert(blockSize);
	assert(initBlocks >= 2);
	assert(maxBlocks >= initBlocks);
}

BufferPool::~BufferPool()
{

}

bool BufferPool::init()
{
	_gainEvent = NULL;
	return true;
}

BufferBlock* BufferPool::allocate()
{
	void* ptr = malloc(_blockSize);
	return new MainBufferBlock(ptr, _blockSize);
}

inline size_t BufferPool::blockSize() const
{
	return _blockSize;
}

inline size_t BufferPool::initBlocks() const
{
	return _initBlocks;
}

inline size_t BufferPool::maxBlocks() const
{
	return _maxBlocks;
}

long BufferPool::refer()
{
	return 1;
}

long BufferPool::release()
{
	delete this;
	return 0;
}

void BufferPool::free(BufferBlock* block)
{
	assert(false);
}

void BufferPool::requestMemory()
{
	// assert(false);
}

void BufferPool::addMem(void* p)
{
	assert(false);
}

//////////////////////////////////////////////////////////////////////////

#else // #ifndef _FAST_BUFFER_POOL

class MainBufferBlock: public BufferBlock {
public:
	MainBufferBlock(BufferPool&	pool, void* ptr):
	  _pool(pool), BufferBlock(ptr, pool.blockSize())
	{
		pool.refer();
		_pos = 0;
		_ref = 1;
	}

	virtual ~MainBufferBlock()
	{
		_pool.release();
	}
	
	virtual void release()
	{
		long r = InterlockedDecrement(&_ref);
		if (r == 0)
			_pool.free(this);
	}

	virtual BufferBlock* getSubBlock(size_t size);

protected:
	long refer()
	{
		return InterlockedIncrement(&_ref);
	}

protected:
	BufferPool&		_pool;
	unsigned int	_pos;
	long			_ref;
};

class SubBufferBlock: public BufferBlock {
public:
	SubBufferBlock(MainBufferBlock&	mainBlock, void* ptr, size_t size):
	  BufferBlock(ptr, size), _mainBlock(mainBlock)
	{

	}

	virtual ~SubBufferBlock()
	{

	}
	
	virtual void release()
	{
		_mainBlock.release();
		delete this;
	}

protected:
	MainBufferBlock&	_mainBlock;
};

BufferBlock* MainBufferBlock::getSubBlock(size_t size)
{
	assert(_pos < _size);

	BYTE* ptr = (BYTE* )getPtr();
	ptr += _pos;
	SubBufferBlock* subBlock = new SubBufferBlock(*this, ptr, size);		
	_pos += size;
	refer();
	return subBlock;
}

//////////////////////////////////////////////////////////////////////////

BufferPool::BufferPool(BufferManager& mgr, size_t blockSize, 
					   size_t initBlocks, size_t maxBlocks): 
						_bufferMgr(mgr), _blockSize(blockSize), 
						_initBlocks(initBlocks), _maxBlocks(maxBlocks)
{
	_ref = 1;
	assert(blockSize);
	assert(initBlocks >= 2);
	assert(maxBlocks >= initBlocks);
}

BufferPool::~BufferPool()
{
	while (_sysMemDescs.size()) {
		void* p = _sysMemDescs.pop();
		::free(p);
	}
}

bool BufferPool::init()
{
	_gainEvent = CreateEvent(NULL, TRUE, FALSE, NULL);	
	return true;
}

BufferBlock* BufferPool::allocate()
{
	WaitForSingleObject(_gainEvent, INFINITE);

	AutoLock autoLock(*this);
	
	void* p = NULL;
	p = _freeBlocks.pop();

	BufferBlock* block = new(p) MainBufferBlock(*this, 
		(PBYTE )p + sizeof(MainBufferBlock));

	if (_freeBlocks.size() <= 0) {
		ResetEvent(_gainEvent);
	}

	if (_freeBlocks.size() <= _initBlocks / 2) {
		requestMemory();
	}

	return block;
}

inline size_t BufferPool::blockSize() const
{
	return _blockSize;
}

inline size_t BufferPool::initBlocks() const
{
	return _initBlocks;
}

inline size_t BufferPool::maxBlocks() const
{
	return _maxBlocks;
}

long BufferPool::refer()
{
	return InterlockedIncrement(&_ref);
}

long BufferPool::release()
{
	long r = InterlockedDecrement(&_ref);
	if (r == 0) {
		delete this;
	}

	return r;
}

void BufferPool::free(BufferBlock* block)
{
	AutoLock autoLock(*this);
	bool sig = _freeBlocks.size() <= 0;	
	_freeBlocks.push(block);
	if (sig)
		SetEvent(_gainEvent);
}

class AllocMemWorkItem: public ZQLIB::ZQWorkItem {
public:
	AllocMemWorkItem(BufferManager& bufMgr, BufferPool* bufPool): 
	  ScWorkItem(bufMgr), _bufPool(bufPool)
	{
		  assert(bufPool);		
	}

	int run()
	{
		BufferManager* bufMgr;

		try {
			bufMgr = (BufferManager* )(&_pool);
		} catch (bad_cast& ) {

			assert(false);
			return -1;
		}

		if (!bufMgr->exist(_bufPool)){

			return 0;
		}

		size_t fragment = _bufPool->blockSize() + sizeof(MainBufferBlock);
		void* p = malloc( fragment * _bufPool->initBlocks() );

		if (p == NULL) {
			// log error
			return -1;
		}

		_bufPool->addMem(p);

		return 0;
	}

	virtual void final(int )
	{
		delete this;
	}

protected:
	BufferPool*	_bufPool;
};

void BufferPool::requestMemory()
{
	AllocMemWorkItem* item = new AllocMemWorkItem(_bufferMgr, this);
	item->start();
}

void BufferPool::addMem(void* p)
{
	AutoLock autoLock(*this);

	_sysMemDescs.push(p);
	size_t fragment = _blockSize + sizeof(MainBufferBlock);
	bool sig = _freeBlocks.size() <= 0;	

	for (size_t i = 0; i < _initBlocks; i ++) {
		void* blk = (PBYTE )p + i * fragment;
		_freeBlocks.push(blk);
	}

	if (sig)
		SetEvent(_gainEvent);
}

#endif // #ifndef _FAST_BUFFER_POOL

//////////////////////////////////////////////////////////////////////////

BufferManager::BufferManager():
#ifndef _FAST_BUFFER_POOL
	ZQThreadPool(1, 1, 0)
#else
	ZQThreadPool(3, 30, 3)
#endif
{

}

BufferManager::~BufferManager()
{

}

BufferPool* BufferManager::createBufferPool(size_t blockSize, 
		size_t initBlocks, size_t maxBlocks)
{
	BufferPool* bufferPool = new BufferPool(*this, 
		blockSize, initBlocks, maxBlocks);

	if (!bufferPool->init()) {
		// log error
		return NULL;
	}

	{
		ZQLIB::AbsAutoLock autoLock(_rwlock.wlock());
		_bufferPools.insert(bufferPool);
	}

	bufferPool->requestMemory();

	return bufferPool;
}

void BufferManager::destroyBufferPool(BufferPool* pool)
{
	ZQLIB::AbsAutoLock autoLock(_rwlock.wlock());
	std::set<BufferPool* >::iterator it;
	it = _bufferPools.find(pool);
	if (it == _bufferPools.end()) {
		assert(false);
		return;
	}

	_bufferPools.erase(it);
	pool->release();
}

bool BufferManager::exist(BufferPool* pool)
{
	ZQLIB::AbsAutoLock autoLock(_rwlock.rlock());

	return _bufferPools.find(pool) != _bufferPools.end();
}

} // namespace DataStream {
