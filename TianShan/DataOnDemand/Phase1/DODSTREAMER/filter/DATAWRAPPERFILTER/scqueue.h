
#ifndef SCQUEUE_H
#define SCQUEUE_H

#include <queue>
#include <list>

using namespace std;

class FredPtr;

class CSCMemoryBlock
{
public:
	int m_iBlockNumber;	// -liqing-
public:

	/// Ctor
	CSCMemoryBlock(CHAR* pBlock, INT32 Size, int Number)
		: m_Block(pBlock), m_iBlockSize(Size), m_iBlockNumber(Number), count_(0)
	{
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
		//free(m_Block);
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

	friend FredPtr;     // friend class
	unsigned count_;
};

// smart pointer
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

#endif