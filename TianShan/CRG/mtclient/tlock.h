#ifndef __TLOCK_DEFINE_H__
#define __TLOCK_DEFINE_H__
#include <windows.h>   

//锁接口类
class IMyLock
{
public:
	virtual ~IMyLock() {}

	virtual void Lock() const = 0;
	virtual void Unlock() const = 0;
};

//互斥对象锁类
class Mutex : public IMyLock
{
public:
	Mutex();
	~Mutex();

	virtual void Lock() const;
	virtual void Unlock() const;

private:
	HANDLE m_mutex;
};

//锁
class CLock
{
public:
	CLock(const IMyLock&);
	~CLock();

private:
	const IMyLock& m_lock;
};
////////////////
class TCCriticalSection
{
public:
	TCCriticalSection();
	~TCCriticalSection();
	void  lock();
	void  unlock();
	bool  trylock();
protected:
private:
	CRITICAL_SECTION _m_cs;
	TCCriticalSection(TCCriticalSection const& );
	TCCriticalSection& operator=(TCCriticalSection const& );
};
class CritSect 
{ 
public: 
	friend class Lock; 
	CritSect() 
	{ 
		InitializeCriticalSection(&_critSection); 
	} 
	~CritSect() 
	{ 
		DeleteCriticalSection(&_critSection); 
	} 
private: 
	void Acquire()
	{
		EnterCriticalSection(&_critSection);
	} 
	void Release()
	{
		LeaveCriticalSection(&_critSection);
	} 

	CRITICAL_SECTION _critSection; 
}; 

class Lock 
{ 
public: 
	Lock(CritSect& critSect):_critSect(critSect) 
	{    
		_critSect.Acquire(); 
	} 
	void unLock()
	{
		_critSect.Release();
	}
	~Lock()
	{
		_critSect.Release();
	} 
private: 
	CritSect& _critSect; 
}; 



#endif //__TLOCK_DEFINE_H__