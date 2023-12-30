#include "tlock.h"

Mutex::Mutex()
{
	m_mutex = ::CreateMutex(NULL, FALSE, NULL);
}


Mutex::~Mutex()
{
	::CloseHandle(m_mutex);
}


void Mutex::Lock() const
{
	DWORD d = WaitForSingleObject(m_mutex, INFINITE);
}


void Mutex::Unlock() const
{
	::ReleaseMutex(m_mutex);
}


CLock::CLock(const IMyLock& m) : m_lock(m)
{
	m_lock.Lock();
}


CLock::~CLock()
{
	m_lock.Unlock();
}
////////////////////////////////////////////////////////
TCCriticalSection::TCCriticalSection()
{
	ZeroMemory(&_m_cs,sizeof(_m_cs));
	InitializeCriticalSection(&_m_cs);
}
TCCriticalSection::~TCCriticalSection()
{
	DeleteCriticalSection(&_m_cs);
}
void TCCriticalSection::lock()
{
	EnterCriticalSection(&_m_cs);
}
void TCCriticalSection::unlock()
{
	LeaveCriticalSection(&_m_cs);
}
bool TCCriticalSection::trylock()
{
	return TryEnterCriticalSection(&_m_cs);
}