// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
// ===========================================================================

#if !defined(AFX_SCQUEUE_H__1663A3C2_22B2_4E27_8111_32ADECBA9561__INCLUDED_)
#define AFX_SCQUEUE_H__1663A3C2_22B2_4E27_8111_32ADECBA9561__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable:4786)

#include <queue>
#include <list>

//using namespace std;
class FredPtr;

class CSCMemoryBlock
{
public:
	int m_iBlockNumber;	// -liqing-
public:

	/// Ctor
	CSCMemoryBlock(CHAR* pBlock, INT32 Size)
	{
		m_iBlockNumber = -1;
		m_Block = pBlock;
		m_iBlockSize = Size;
		count_ = 0;
	}

	/// Copy Ctor
	CSCMemoryBlock(const CSCMemoryBlock& rhs)
	{
		m_iBlockNumber = rhs.m_iBlockNumber;
		m_Block = rhs.m_Block;
		m_iBlockSize = rhs.m_iBlockSize;
		count_ = rhs.count_;
	}

	CSCMemoryBlock & operator= ( const CSCMemoryBlock & rhs )
	{
		if( this == &rhs )
			return *this;

		m_iBlockNumber = rhs.m_iBlockNumber;
		m_Block = rhs.m_Block;
		m_iBlockSize = rhs.m_iBlockSize;
		count_ = rhs.count_;

		return *this;
	}

	/// Dtor
	~CSCMemoryBlock()
	{
		free(m_Block);
		m_Block = NULL;
		m_iBlockSize = 0;
	}

	/// Allocate a memory block.
	/// @param size Bytes to allocate.
	/// @return a void pointer to the allocated space, or NULL.
	static CHAR* AllocBlock(const INT32 size)
	{
		return (CHAR*)malloc(size);
	}

	/// Free a memory block.
	/// @param block previously allocated memory block to be freed.
	static VOID FreeBlock(CHAR* block)
	{
		if( block != NULL )
		{
			free(block);
			block = NULL;
		}
	}
	
	/// Get a memory block pointer.
	/// @return the memory block pointer.
	CHAR* GetBlock() const
	{
		return m_Block;
	}

	/// Get a memory block size.
	/// @return the memory block size.
	INT32 GetSize() const
	{
		return m_iBlockSize;
	}

private:
	CHAR* m_Block;
	INT32 m_iBlockSize;

	friend FredPtr;     // ÓÑÔªÀà
	LONG	count_;
};

#if !defined(_NO_FIX_DOD)

class FredPtr {
public:
	FredPtr()
	{
		_ptr = NULL;
	}

	FredPtr(CSCMemoryBlock* p) : _ptr(p)
	{
		if (_ptr)
			InterlockedIncrement(&_ptr->count_);
	}

	~FredPtr()
	{ 
		if (_ptr)
			
			if (InterlockedDecrement(&_ptr->count_) == 0) 
				delete _ptr;
	}

	CSCMemoryBlock* operator->()
	{
		return _ptr;
	}

	CSCMemoryBlock& operator* ()
	{
		return *_ptr;
	}

	bool operator == (const FredPtr& p)
	{
		return _ptr == p._ptr;
	}

	bool operator != (const FredPtr& p)
	{
		return _ptr != p._ptr;
	}

	bool operator == (CSCMemoryBlock* ptr)
	{
		return _ptr == ptr;
	}

	bool operator != (CSCMemoryBlock* ptr)
	{
		return _ptr != ptr;
	}

	FredPtr(const FredPtr& p) : _ptr(p._ptr) 
	{ 
		if (_ptr)
			InterlockedIncrement(&_ptr->count_); 
	}

	FredPtr& operator= (const FredPtr& p)
	{	
		if (p._ptr)
			InterlockedIncrement(&p._ptr->count_);

		if (_ptr)
			if(InterlockedDecrement(&_ptr->count_) == 0)
				delete _ptr;
		_ptr = p._ptr;
		return *this;
	}

protected:
	CSCMemoryBlock* _ptr;
};

template<typename T>
class AutoLock {
public:
	AutoLock(T& t): _t(t)
	{
		t.Lock();
	}

	~AutoLock()
	{
        _t.Unlock();
	}
protected:
	T&		_t;
};

template<typename T>
class WaitableQueue {
public:
	WaitableQueue(size_t size)
	{
		InitializeCriticalSection(&_cs);
		_topEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		_bottomEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		_maxSize = size;
	}

	virtual ~WaitableQueue()
	{
		DeleteCriticalSection(&_cs);
		CloseHandle(_topEvent);
		CloseHandle(_bottomEvent);
	}

	void Lock()
	{
		EnterCriticalSection(&_cs);
	}

	void Unlock()
	{
		LeaveCriticalSection(&_cs);
	}

	bool Push(const T& item, DWORD timeo)
	{
		if (WaitForSingleObject(_topEvent, timeo) != WAIT_OBJECT_0) {
			return false;
		}

		_AutoLock cs(*this);
		
		size_t size = _queue.size();
		if (size >= _maxSize)
			return false;

		_queue.push(item);
		
		size ++;

		if (size >= _maxSize) {
            ResetEvent(_topEvent);
		}

		if (size == 1) {
			SetEvent(_bottomEvent);
		}
		
		return true;
	}

	bool Pop(T& item, DWORD timeo)
	{	
		if (WaitForSingleObject(_bottomEvent, timeo) != WAIT_OBJECT_0) {
			return false;
		}

		_AutoLock cs(*this);
		size_t size = _queue.size();

		if (size <= 0)
			return false;
		
		item = _queue.front();
        _queue.pop();

		size --;

		if (size == 0) {
			ResetEvent(_bottomEvent);
		}

		if (size == _maxSize - 1) {
			SetEvent(_topEvent);
		}

		return true;
	}

	size_t Size()
	{
		_AutoLock cs(*this);
		return _queue.size();
	}

	bool Empty()
	{
		_AutoLock cs(*this);
		return _queue.empty();
	}

	static void* CreateMultiQueues(WaitableQueue* queues[], size_t queueNum)
	{
		HANDLE* handles = new HANDLE[queueNum + 1];
        handles[0] = (HANDLE )queueNum;
		for (size_t i = 0; i < queueNum; i ++) {
			queues[i]->Lock();
			handles[i + 1] = queues[i]->_topEvent;
			queues[i]->Unlock();
		}

		return handles;
	}

	static DWORD WaitForMultiQueues(void* waitHandle, DWORD timeo)
	{
		HANDLE* handles = (HANDLE* )waitHandle;
        size_t size = (size_t )handles[0];
		DWORD r = WaitForMultipleObjects(size, &handles[1], FALSE, timeo);
		if (r >= WAIT_OBJECT_0 && r < WAIT_OBJECT_0 + size)
			return r - WAIT_OBJECT_0;
		return r;
	}

	static void CloseMultiQueues(void* waitHandle)
	{
		HANDLE* handles = (HANDLE* )waitHandle;
		delete handles;
	}

protected:
	typedef AutoLock<WaitableQueue> _AutoLock;
	CRITICAL_SECTION	_cs;
	HANDLE				_topEvent;
	HANDLE				_bottomEvent;
	size_t				_maxSize;
	std::queue<T>		_queue;
};

//////////////////////////////////////////////////////////////////////////
#include <assert.h>

template<typename T>
class RawQueue {
protected:
	void rawPush(const T& x)
	{
		_queue.push(x);
	}

	void rawPop(T& item)
	{
		item = _queue.front();
		_queue.pop();
	}

	size_t rawSize()
	{
		return _queue.size();
	}

protected:
	std::queue<T>		_queue;
};

template<typename T, typename Comp = std::less<T> >
class RawPriQueue {
protected:
	void rawPush(const T& x)
	{
		_queue.push(x);
	}

	void rawPop(T& item)
	{
		item = _queue.top();
		_queue.pop();
	}

	size_t rawSize()
	{
		return _queue.size();
	}

protected:
	std::priority_queue<T, std::vector<T>, Comp>	_queue;
};

template<typename T, typename Q = RawQueue<T> >
class WaitableQueue2: protected Q {

public:
	class QueueGroup;
	friend QueueGroup;

public:
	WaitableQueue2(size_t size = 0):
	  _maxSize(size)
	  {
		  InitializeCriticalSection(&_cs);
		  _usedSem = CreateSemaphore(NULL, 0, size, NULL);
		  _unusedSem = CreateSemaphore(NULL, size, size, NULL);
	  }

	  virtual ~WaitableQueue2()
	  {
		  DeleteCriticalSection(&_cs);
		  CloseHandle(_usedSem);
		  CloseHandle(_unusedSem);
	  }

	  void Lock()
	  {
		  EnterCriticalSection(&_cs);
	  }

	  void Unlock()
	  {
		  LeaveCriticalSection(&_cs);
	  }

	  bool Push(const T& item, DWORD timeo = INFINITE)
	  {
		  DWORD wr = WaitForSingleObject(_unusedSem, timeo);
		  if (wr != WAIT_OBJECT_0) {

			  assert (wr != WAIT_FAILED);
			  return false;
		  }

		  bool r = ipush(item, timeo);
		  if (!r) {
			  ReleaseSemaphore(_unusedSem, 1, NULL);
		  }

		  return r;
	  }

	  bool Pop(T& item, DWORD timeo = INFINITE)
	  {
		  DWORD wr = WaitForSingleObject(_usedSem, timeo);
		  if (wr != WAIT_OBJECT_0) {

			  assert (wr != WAIT_FAILED);
			  return false;
		  }

		  bool r = ipop(item, timeo);
		  if (!r) {
			  ReleaseSemaphore(_unusedSem, 1, NULL);
		  }

		  return r;
	  }

	  size_t Size()
	  {
		  _AutoLock lock(*this);
		  return rawSize();
	  }

	  bool empty()
	  {
		  _AutoLock lock(*this);
		  return _queue.empty();
	  }

public:

	class QueueGroup {
		friend WaitableQueue2;
	protected:
		QueueGroup(WaitableQueue2* queues[], size_t size, 
			bool forPop = true, bool forPush = true): 
		_size(size)
		{
			assert(forPop | forPush);

			HANDLE* handles;

			if (forPop) {
				_usedHandles = (HANDLE* )malloc(sizeof(HANDLE) * size);
				handles = _usedHandles;

				for (size_t i = 0; i < size; i ++) {

					WaitableQueue2* queue = queues[i];
					queue->Lock();
					handles[i] = queue->_usedSem;
					queue->Unlock();
				}

			} else {

				_usedHandles = NULL;
			}

			if (forPush) {
				_unusedHandles = (HANDLE* )malloc(sizeof(HANDLE) * size);

				handles = _unusedHandles;

				for (size_t i = 0; i < size; i ++) {

					WaitableQueue2* queue = queues[i];
					queue->Lock();
					handles[i] = queue->_unusedSem;
					queue->Unlock();
				}
			} else {
				_unusedHandles = NULL;
			}

			_queues = (WaitableQueue2** )malloc(
				sizeof(WaitableQueue2* ) * size);

			memcpy(_queues, queues, sizeof(WaitableQueue2*) * size);
		}

		virtual ~QueueGroup()
		{
			if (_usedHandles) {
				free(_usedHandles);
			}

			if (_unusedHandles) {
				free(_unusedHandles);
			}

			free(_queues);
		}

	public:
		DWORD pop(T& item, DWORD timeo)
		{
			if (_usedHandles == NULL) {
				assert(false);
				return -1;
			}

			DWORD r = WaitForMultipleObjects(_size, _usedHandles, 
				FALSE, timeo);

			DWORD index;

			if (r >= WAIT_OBJECT_0 && r < WAIT_OBJECT_0 + _size) {

				index = r - WAIT_OBJECT_0;
			} else {
				return -1;
			}

			if (!_queues[index]->ipop(item, timeo)) {
				ReleaseSemaphore(_usedHandles[index], 1, NULL);
			}

			return index;
		}

		DWORD push(const T& item, DWORD timeo = INFINITE)
		{
			if (_unusedHandles == NULL) {
				assert(false);
				return -1;
			}

			DWORD r = WaitForMultipleObjects(_size, _unusedHandles, 
				FALSE, timeo);

			DWORD index;

			if (r >= WAIT_OBJECT_0 && r < WAIT_OBJECT_0 + _size) {

				index = r - WAIT_OBJECT_0;
			} else {
				return -1;
			}

			if (!_queues[index]->ipush(item, timeo)) {
				ReleaseSemaphore(_usedHandles[index], 1, NULL);
			}

			return index;
		}

		DWORD preparePush(DWORD timeo = INFINITE)
		{
			if (_unusedHandles == NULL) {
				assert(false);
				return -1;
			}

			DWORD r = WaitForMultipleObjects(_size, _unusedHandles, 
				FALSE, timeo);

			if (r >= WAIT_OBJECT_0 && r < WAIT_OBJECT_0 + _size) {

				return r - WAIT_OBJECT_0;
			} else {
				return r;
			}
		}

		bool cancelPush(DWORD index)
		{
			return ReleaseSemaphore(_unusedHandles[index], 1, NULL) == TRUE;
		}

		bool doPush(T& item, DWORD index)
		{
			if (!_queues[index]->ipush(item, INFINITE)) {
				ReleaseSemaphore(_usedHandles[index], 1, NULL);
				return false;
			}

			return true;
		}

		DWORD preparePop(DWORD timeo = INFINITE)
		{
			if (_usedHandles == NULL) {
				assert(false);
				return -1;
			}

			DWORD r = WaitForMultipleObjects(_size, _usedHandles, 
				FALSE, timeo);

			if (r >= WAIT_OBJECT_0 && r < WAIT_OBJECT_0 + _size) {

				return r - WAIT_OBJECT_0;
			} else {
				return r;
			}
		}

		bool cancelPop(DWORD index)
		{
			return ReleaseSemaphore(_usedHandles[index], 1, NULL)) == TRUE;
		}

		bool doPop(const T& item, DWORD index)
		{
			if (!_queues[index]->ipop(item, INFINITE)) {
				ReleaseSemaphore(_usedHandles[index], 1, NULL);
				return false;
			}

			return true;
		}

	protected:		
		HANDLE*				_usedHandles;
		HANDLE*				_unusedHandles;
		WaitableQueue2**	_queues;
		size_t				_size;
	};


	static QueueGroup* createQueueGroup(WaitableQueue2* queues[], 
		size_t queueNum, bool forPop = true, bool forPush = true)
	{

		QueueGroup* qg = new QueueGroup(queues, queueNum, forPop, 
			forPush);

		return qg;
	}

	static void closeQueueGroup(QueueGroup* queueGroup)
	{
		delete queueGroup;
	}


protected:

	bool ipop(T& item, DWORD timeo = INFINITE)
	{
		_AutoLock lock(*this);

		rawPop(item);

		BOOL res = ReleaseSemaphore(_unusedSem, 1, NULL);
		if (!res) {			
			assert(false);
			return false;
		}

		return true;
	}

	bool ipush(const T& item, DWORD timeo = INFINITE)
	{
		_AutoLock lock(*this);

		rawPush(item);

		BOOL res = ReleaseSemaphore(_usedSem, 1, NULL);
		if (!res) {			
			assert(false);
			return false;
		}

		return true;
	}

protected:
	typedef AutoLock<WaitableQueue2> _AutoLock;
	CRITICAL_SECTION	_cs;
	HANDLE				_usedSem;
	HANDLE				_unusedSem;
	size_t				_maxSize;
};

//////////////////////////////////////////////////////////////////////////

// #define _USE_OLD_QUEUE

#ifdef _USE_OLD_QUEUE

class CSCMemoryBlockQueue: public WaitableQueue<FredPtr> {
public:
	CSCMemoryBlockQueue(size_t maxSize):
      WaitableQueue<FredPtr>(maxSize)
	{
		m_WFileOffset = 0;
	}

	DWORD		m_WFileOffset;
};

#else

class CSCMemoryBlockQueue: public WaitableQueue2<FredPtr> {
public:
	CSCMemoryBlockQueue(size_t maxSize):
	  WaitableQueue2<FredPtr>(maxSize)
	  {
		  m_WFileOffset = 0;
	  }

	  DWORD		m_WFileOffset;
};

#endif

//////////////////////////////////////////////////////////////////////////

#else // #if !defined(_NO_FIX_DOD)

class FredPtr {
public:
   CSCMemoryBlock* operator->() { return p_; }
   CSCMemoryBlock& operator* ()  { return *p_; }

   FredPtr(CSCMemoryBlock* p) : p_(p) { ++p_->count_; }  // p can't be NULL
  ~FredPtr(){ if (--p_->count_ == 0) delete p_; }

   FredPtr(const FredPtr& p) : p_(p.p_) { ++p_->count_; }
   FredPtr& operator= (const FredPtr& p)
         { // 
           // 
           ++p.p_->count_;
           if(--p_->count_ == 0) delete p_;
           p_ = p.p_;
           return *this;
         }
private:
   CSCMemoryBlock* p_;    // p_ can't be NULL never
};

/////////////////////////////////////////////
template<typename T> 
class CSCQueue  
{
public:

	/// Adds an element to the back of the queue.
	/// @param t element added to the queue.
	VOID Push(const T& t)
	{
		m_Queue.push(t);
	}

	/// Removes an element from the front of the queue.
	VOID Pop()
	{
		m_Queue.pop();
	}

	/// Tests if the queue is empty.
	/// @return TRUE if empty, otherwise return FALSE.
	BOOL Empty( ) const
	{
		return m_Queue.empty();
	}

	/// Returns the number of elements in the queue.
	/// @return the number of elements in the queue.
	INT32 Size() const
	{
		return m_Queue.size();
	}

	/// Returns a reference to the first element at the front of the queue.
	/// @return the element.
	T& Front( )
	{
		return m_Queue.front();
	}

	/// Returns a reference to the first element at the front of the queue.
	/// @return the element.
	const T& Front( ) const
	{
		return m_Queue.front();
	}

#if 0
	/// Returns a reference to the last and most recently added element at 
	/// the back of the queue.
	/// @return the element.
	T& Back( )
	{
		return m_Queue.back();
	}

	/// Returns a reference to the last and most recently added element at 
	/// the back of the queue.
	/// @return the element.
	const T& Back( ) const
	{
		return m_Queue.back();
	}
#endif
protected:
	std::queue<T> m_Queue;
};

/// Mutex aspect
template <class T>
class Mutex_Aspect: public CSCQueue<T>
{
public:
	Mutex_Aspect()
	{
		InitializeCriticalSection(&m_CriticalSection);
		m_WFileOffset=0;
	}

	~Mutex_Aspect()
	{
		DeleteCriticalSection(&m_CriticalSection);
	}

	VOID Push(const T& t)
	{
		EnterCriticalSection(&m_CriticalSection);
		CSCQueue<T>::Push(t);
		LeaveCriticalSection(&m_CriticalSection);
	}

	VOID Pop()
	{
		EnterCriticalSection(&m_CriticalSection);
		CSCQueue<T>::Pop();
		LeaveCriticalSection(&m_CriticalSection);
	}

	T& Front()
	{
		return CSCQueue<T>::Front();
	}

	const T& Front() const
	{
		return CSCQueue<T>::Front();
	}
public:
	DWORD m_WFileOffset;

private:
	CRITICAL_SECTION m_CriticalSection;
};

/*
/// Mutex aspect
template <class T, template <class> class Q>
class Mutex_Aspect: public Q<T>
{
public:
	Mutex_Aspect()
	{
		InitializeCriticalSection(&m_CriticalSection);
	}

	~Mutex_Aspect()
	{
		DeleteCriticalSection(&m_CriticalSection);
	}

	VOID Push(const T& t)
	{
		EnterCriticalSection(&m_CriticalSection);
		Q::push(t);
		LeaveCriticalSection(&m_CriticalSection);
	}

	VOID Pop()
	{
		EnterCriticalSection(&m_CriticalSection);
		Q::pop();
		LeaveCriticalSection(&m_CriticalSection);
	}

	T& Front( )
	{
		return Q::front();
	}

	const T& Front( ) const
	{
		return Q::front();
	}

private:
	CRITICAL_SECTION m_CriticalSection;
};


/// Signal aspect
template <class T, template <class> class Q>
class Signal_Aspect: public Q<T>
{
public:
	Signal_Aspect()
	{
		m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if(m_hEvent == NULL)
		{
			throw NULL;
		}
	}

	~Signal_Aspect()
	{
		if(m_hEvent)
		{
			CloseHandle(m_hEvent);
			m_hEvent = NULL;
		}
	}

	VOID Push(const T& t)
	{
		Q::push(t);
		SetEvent(m_hEvent);
	}

	VOID Pop()
	{
		Q::pop();
		if(m_Queue.empty())
		{
			ResetEvent(m_hEvent);
		}
	}

	T& Front( )
	{
		WaitForSingleObject(m_hEvent,INFINITE);
		return Q::front();
	}

	/// Returns a reference to the first element at the front of the queue.
	/// @return the element.
	const T& Front( ) const
	{
		WaitForSingleObject(m_hEvent,INFINITE);
		return Q::front();
	}

private:
	HANDLE m_hEvent;
};
*/
/// Common queue
//typedef CSCQueue<CSCMemoryBlock> CSCMemoryBlockQueue;

typedef Mutex_Aspect<FredPtr> CSCMemoryBlockQueue;
/*
/// Signal purpose queue
typedef Signal_Aspect<CSCMemoryBlock, CSCQueue> CSCSignalMemoryBlockQueue;

/// Mutex purpose queue
typedef Mutex_Aspect<CSCMemoryBlock, CSCQueue> CSCMutexMemoryBlockQueue;
*/

#endif // #if !defined(_NO_FIX_DOD)

#endif // !defined(AFX_SCQUEUE_H__1663A3C2_22B2_4E27_8111_32ADECBA9561__INCLUDED_)
