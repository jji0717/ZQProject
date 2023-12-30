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
//
// Ident : $Id: Locks.h,v 1.9 2004/06/30 07:24:54 jshen Exp $
// Branch: $Name:  $
// Author: mwang
// Desc  : Define simple mutex and semaphore
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/SMS/ModemGateway/Locks.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 1     06-11-14 18:23 Shuai.chen
// 
// 3     05-06-24 14:58 Bernie.zhao
// modified 'MutexGuard' to  'ZQ::common::Guard<ZQ::common::Mutex>' for
// convinient use between namespaces
// Revision 1.9  2004/06/30 07:24:54  jshen
// remove some warnings
//
// Revision 1.8  2004/06/23 07:37:39  wli
// add coede to avoid redefine WIN32_LEAN_AND_MEAN
//
// Revision 1.7  2004/06/13 03:46:50  mwang
// no message
//
// Revision 1.6  2004/06/13 03:42:41  mwang
// no message
//
// Revision 1.5  2004/06/07 13:33:30  shao
// no message
//
// Revision 1.4  2004/06/07 10:58:25  mwang
// no message
//
// Revision 1.3  2004/06/07 10:54:08  mwang
// add WaitThread impl
//
// Revision 1.2  2004/06/07 10:46:13  shao
// added Semaphore
//
// ===========================================================================

#ifndef __ZQ_COMMON_LOCKS_H__
#define __ZQ_COMMON_LOCKS_H__

#include "ZQ_common_conf.h"
#include "Exception.h"

#ifndef WIN32
#error here
#endif

#ifndef WIN32
#error here2
#  if (defined(__FreeBSD__) && __FreeBSD__ <= 3) || defined(_AIX)
#    define	_SYSV_SEMAPHORES
#  endif
#  ifndef HAVE_PTHREAD_H
#error here3
#    include <pthread.h>
#    ifndef _SYSV_SEMAPHORES
#      include <semaphore.h>
#    endif
#  endif
#endif // !WIN32

#ifndef WIN32
#  include <time.h>
#  include <signal.h>
#  include <unistd.h>
typedef	unsigned long	timeout_t;
#else // WIN32
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#  include <windows.h>
typedef DWORD   timeout_t;
#  define	MAX_SEM_VALUE	1000000
#endif // !WIN32

namespace ZQ {
namespace common  {

// -----------------------------
// class Mutex
// -----------------------------
///a wrap of CRITICAL_SECTION on windows
///suppose the functions will never fail!!!(if it fails, the OS will be ...)
class Mutex
{
public:
	///initializes the critical section object
	Mutex()
		{ ::InitializeCriticalSection(&_mutex); }

	///releases all resources used by an unowned critical section object.
	~Mutex()
		{ ::DeleteCriticalSection(&_mutex); }

	///waits for ownership of the specified critical section object,The function returns when the calling thread is granted ownership.
	void enter()
		{::EnterCriticalSection(&_mutex);}

	///releases ownership of the specified critical section object.
	void leave()
		{::LeaveCriticalSection(&_mutex);}
		
#if _WIN32_WINNT >=0x0400
	bool tryEnter()
		{return (::TryEnterCriticalSection(&_mutex) == TRUE);}
#endif //_WIN32_WINNT 

private:

	CRITICAL_SECTION  _mutex;
	///can't be copied!!!
	Mutex(const Mutex &);
	Mutex &operator=(const Mutex &);
};

// -----------------------------
// template Guard
// -----------------------------
///a simple wrap for locks,for exception safe
template <typename _LOCK >
class Guard
{
public:
	Guard(_LOCK& lock)
		:_lock(lock) { _lock.enter(); }

	~Guard()
		{ _lock.leave(); }
private:
	///need to be const???
	_LOCK& _lock;
	///also ,can't be copied!!!
	Guard(const Guard &);
	Guard &operator=(const Guard &);
};

typedef ZQ::common::Guard<ZQ::common::Mutex> MutexGuard;

// -----------------------------
// class Semaphore
// -----------------------------
/// The semaphore has a counter which only permits access by one or more threads when
/// the value of the semaphore is non-zero. Each access reduces the current value of
/// the semaphore by 1. One or more threads can wait on a semaphore until it is no
/// longer 0, and hence the semaphore can be used as a simple thread synchronization
/// object to enable one thread to pause others until the thread is ready or has
/// provided data for them. Semaphores are typically used as a counter for protecting
/// or limiting concurrent access to a given resource, such as to permitting at most
/// "x" number of threads to use resource "y", for example.   
class Semaphore
{
private:
#ifndef WIN32
#ifdef	_SYSV_SEMAPHORES
	int _semaphore;
#else
	sem_t _semaphore;
#endif
#else // WIN32
	HANDLE	semObject;
#endif // !WIN32

public:

	/// The initial value of the semaphore can be specified. An initial value is often
	/// used When used to lock a finite resource or to specify the maximum number of
	/// thread instances that can access a specified resource.
	/// @param resource specify initial resource count or 0 default.
	Semaphore(size_t resource = 0);

	/// Destroying a semaphore also removes any system resources associated with it.
	/// If a semaphore has threads currently waiting on it, those threads will all
	/// continue when a semaphore is destroyed.
	virtual ~Semaphore();

	/// Wait is used to keep a thread held until the semaphore counter is greater than
	/// 0. If the current thread is held, then another thread must increment the
	/// semaphore.  Once the thread is accepted, the semaphore is automatically
	/// decremented, and the thread continues execution. The pthread semaphore object
	/// does not support a timed "wait", and hence to maintain consistancy, neither
	/// the posix nor win32 source trees support "timed" semaphore objects.
	void wait(void);

	/// TryWait is a non-blocking variant of Wait. If the semaphore counter is greater
	/// than 0, then the thread is accepted and the semaphore counter is decreased. If
	/// the semaphore counter is 0 TryWait returns immediately with false, NO decrement.
	/// @return true if thread is accepted otherwise false
	bool tryWait(void);

	/// Posting to a semaphore increments its current value and releases the first
	/// thread waiting for the semaphore if it is currently at 0. multiple increments
	/// must perform multiple post operations.
	void post(void);

	/// TODO: how implement getValue for posix compatibility ?
	/// Get the current value of a semaphore.
	/// @return current value.
#ifndef WIN32
#ifndef __CYGWIN32__
	int getValue(void);
#endif
#endif
};


}
}

#endif //__ZQ_COMMON_LOCKS_H__