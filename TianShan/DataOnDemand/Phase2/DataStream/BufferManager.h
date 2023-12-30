// BufferManager.h: interface for the BufferManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUFFERMANAGER_H__3C756C4F_1C86_472D_A62C_951800DA6E97__INCLUDED_)
#define AFX_BUFFERMANAGER_H__3C756C4F_1C86_472D_A62C_951800DA6E97__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZQThreadPool.h"

namespace DataStream {

class BufferPool;
class BufferManager;
class AllocMemWorkItem;
//friend class BufferBlock;

class BufferBlock {
public:

	BufferBlock(void* ptr, size_t size):
	  _ptr(ptr), _size(size)
	{

	}

	virtual ~BufferBlock()
	{
		
	}

	size_t size() const
	{
		return _size;
	}

	void* getPtr() const
	{
		return _ptr;
	}
	
	virtual void release() = 0;

	virtual BufferBlock* getSubBlock(size_t)
	{
		assert(false);
		return NULL;
	}

protected:
	size_t		_size;
	void*		_ptr;
};

class GenBufferBlock: public BufferBlock {
public:
	GenBufferBlock(void* ptr, size_t size):
	  BufferBlock(ptr, size)
	{

	}

	virtual void release()
	{
		
	}
};

template <class T>
class FastStack {
public:
	void init()
	{

	}

	void push(T n)
	{
		_stack.push_back(n);
	}

	T pop()
	{
		T result = _stack.back();
		_stack.pop_back();
		return result;
	}

	size_t size() const
	{
		return _stack.size();
	}

protected:
	std::vector<T>	_stack;
};

class MainBufferBlock;

class BufferPool: protected ZQLIB::LightLock {
	friend class MainBufferBlock;
	friend class AllocMemWorkItem;
	friend class BufferManager;
public:

	bool init();

	BufferBlock* allocate();
	inline size_t blockSize() const;
	inline size_t initBlocks() const;
	inline size_t maxBlocks() const;
	
	inline long refer();
	inline long release();

protected:

	BufferPool(BufferManager& mgr, size_t blockSize, 
		size_t initBlocks, size_t maxBlocks);
	
	virtual ~BufferPool();

	void free(BufferBlock* block);
	void requestMemory();

	virtual void addMem(void* p);


protected:

	typedef ZQLIB::AbsAutoLock AutoLock;

	BufferManager&	_bufferMgr;
	size_t			_blockSize;
	size_t			_initBlocks;
	size_t			_maxBlocks;
	
	typedef FastStack<void*> MemBlocks;

	MemBlocks		_sysMemDescs;
	MemBlocks		_freeBlocks;

	HANDLE			_gainEvent;
	long			_ref;
};

class BufferManager: public ZQLIB::ZQThreadPool {
public:
	BufferManager();
	virtual ~BufferManager();

	BufferPool* createBufferPool(size_t blockSize, 
		size_t initBlocks, size_t maxBlocks);

	void destroyBufferPool(BufferPool* pool);
	bool exist(BufferPool* pool);

protected:
	std::set<BufferPool* >		_bufferPools;
	ZQLIB::XRWLockEx			_rwlock;
};

} // namespace DataStream {

#endif // !defined(AFX_BUFFERMANAGER_H__3C756C4F_1C86_472D_A62C_951800DA6E97__INCLUDED_)
