#include "NativeThread.h"
//#include "Log.h" // from macro SCRTRACE, DON'T log to file in base thread

//xiaobai
#ifndef SCRTRACE
#define SCRTRACE
#endif

extern "C" {

#ifdef WIN32
#  include <process.h>
#  include <stdio.h>
#else
#  include <unistd.h>
#  include <errno.h>
#endif

#include <time.h>
}


namespace ZQ {
namespace common {

// typedef	unsigned (__stdcall *exec_t)(void *);
#ifdef WIN32
NativeThread::NativeThread(int stacksize)
       :_thrdID(0), _status(stDeferred),_bCancel(false)
{
	_flags.B = 0;
	
//	_hThread = (HANDLE)CreateThread(NULL, stacksize, _execute, (void *)this, CREATE_SUSPENDED, (unsigned long *)&_thrdID);
	_hThread = (HANDLE)_beginthreadex(NULL, stacksize, _execute, (void *)this, CREATE_SUSPENDED, (unsigned*) &_thrdID);

	if(_hThread >0)
		return;	

	setStatus(stInitial);
}

NativeThread::~NativeThread()
{
	terminate();
	try
	{
		CloseHandle(_hThread);
	}
	catch(...){}

	_hThread = INVALID_HANDLE_VALUE;
}

void NativeThread::exit()
{
	if (isThread())
	{
		setStatus(stDisabled);
		ExitThread(0);
	}
}

bool NativeThread::isRunning(void)
{
	return (_status == stRunning) ? true : false;
}

bool NativeThread::isThread(void)
{
	return ((_thrdID == GetCurrentThreadId())) ? true : false;
}

void NativeThread::terminate(int code /* = 0 */)
{
#ifdef _DEBUG
//	printf(LOGFMT("tid=%d\n"), _thrdID);
#endif // _DEBUG

	if(!_thrdID)
	{
		return;
	}

	SCRTRACE;

	if(_thrdID == GetCurrentThreadId())
		return;

	SCRTRACE;

	bool terminated = false;

	try
	{
		if(!_flags.b.active)
			ResumeThread(_hThread);
	}
	catch(...){}

	SCRTRACE;

	try
	{
		TerminateThread(_hThread, code);
		terminated = true;
	}
	catch(...){}

	WaitForSingleObject(_hThread, INFINITE);

	if (terminated)
	{
		SCRTRACE;
		this->final();
	}

	_thrdID = 0;
}

// unsigned long __stdcall NativeThread::_execute(void *thread)
unsigned __stdcall NativeThread::_execute(void *thread)
{
	NativeThread *th = (NativeThread *)thread;

	int ret = -1;
	if( th->_bCancel )
	{
		th->_thrdID = 0 ;
		th->setStatus(stDisabled);
		return 0;
	}

	try
	{
		// initialize the rand seed as the srand()/rand() is per-thread
		::srand((uint32) (::time(NULL)<<6 ^ ((uint32) thread & 0xffffffff)) ^ 0x93da8fae);

		if (th->init())
		{
			th->setStatus(stRunning);
			ret = th->run();
		}

	} catch(...) {}

	th->setStatus(stDisabled);

	try
	{
		
		if (th->_thrdID > 0)
		{
			th->_thrdID = 0;

			SCRTRACE;
			th->final();
		}
		else ret = -2; // this thread was terminated

	} catch(...) {}

	return ret;
}

void NativeThread::setStatus(const status_t st)
{
	_status = st;
}

void NativeThread::sleep(timeout_t msec)
{
	::SleepEx(msec, false);
}

bool NativeThread::start()
{
	DWORD ret = ResumeThread(_hThread);
	::Sleep(1); // yield for other threads
	return (ret!=-1);
}

void NativeThread::final(void)
{ 
	SCRTRACE;
	return; 
}

void NativeThread::suspend(void)
{
	SuspendThread(_hThread);
}

void NativeThread::resume(void)
{
	ResumeThread(_hThread);
}

uint32 NativeThread::waitHandle(timeout_t timeout)
{
	if( _status == stDeferred)
	{
		_bCancel = true;
		resume();
	}
	return WaitForSingleObject(_hThread,timeout);
}

#else//non-win32

#define panic(str) \
    char* error = new char[128]; \
    memset(error, '\0', sizeof(error)); \
    sprintf(error, str": [%d][%s]", errno, strerror(errno)); \
    throw error; 

NativeThread::NativeThread(int stacksize):_thrdID(0), _status(stDeferred){
	_flags.B = 0;

	if(sem_init(&_thsem, 0, 0)) {
        panic("failed to initialize thread semaphore");
	}
	if(sem_init(&_suspend, 0, 0)) {
        panic("failed to initialize suspend semaphore");
	}

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, stacksize);
	if(pthread_create(&_thrdID, &attr, _execute,(void*)this)) {
		sem_destroy(&_thsem);
		sem_destroy(&_suspend);
		pthread_attr_destroy(&attr);

        panic("failed to create thread");
	}

	setStatus(stInitial);
}

NativeThread::~NativeThread() {
	terminate();

    sem_destroy(&_thsem);
    sem_destroy(&_suspend);
}

void NativeThread::exit() {
	if(isThread()) {
		setStatus(stDisabled);
		pthread_exit(EXIT_SUCCESS);
	}
}

bool NativeThread::isRunning() {
	return (_status == stRunning) ? true : false;
}

bool NativeThread::isThread() {
    return pthread_equal(_thrdID, pthread_self());
}

void NativeThread::terminate(int code /* = 0 */) {
    /* called from the same thread */
	if(isThread()) {
		return;
    }
    
    if(_status == stRunning) {
//      pthread_cancel(_thrdID);

        void* res;
        pthread_join(_thrdID, &res);
    
/*
        if(res == PTHREAD_CANCELED) {
            this->final();
        }
*/

        _thrdID = 0;
    }
}

void* NativeThread::_execute(void *thread) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

	NativeThread *th = (NativeThread *)thread;

	/* suspend */
sus:
	int res =sem_wait(&th->_suspend);
	if(res == (-1) && errno == EINTR) {
		goto sus;
	}

	try {
		if (th->init()) {
			th->setStatus(stRunning);
			th->run();
		}
	} catch(...) {}

	try {
        th->final();
    } catch(...) {}

	sem_post(&th->_thsem);
    
	th->setStatus(stDisabled);

    return (void*)(0);
}

void NativeThread::setStatus(const status_t st) {
	_status = st;
}

void NativeThread::sleep(timeout_t msec) {
    struct timespec ts;
    ts.tv_sec = msec/1000L;
    ts.tv_nsec = (msec%1000L) * 1000000L;
    nanosleep(&ts, 0);
}

bool NativeThread::start() {
	return (sem_post(&_suspend) == 0);
}

void NativeThread::final(void) { 
}

void NativeThread::suspend(void) {
}

void NativeThread::resume(void) {
}

uint32 NativeThread::waitHandle(timeout_t timeout) {
    if(!isRunning()) {
        return (0);
    }
	int res = 0;
	if(timeout <= 0 || timeout == (timeout_t)(-1)) {
		void* val;
        res = pthread_join(_thrdID, &val);
	}
	else {
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);

		long long nsec = ts.tv_nsec + timeout*1000000LL;
		ts.tv_sec += nsec/1000000000L;
		ts.tv_nsec = nsec%1000000000L;

con:
        if(sem_timedwait(&_thsem, &ts) == (-1)) {
            if(errno == EINTR) {
                goto con;
            }
            res = errno;
        }
	}
	return res;
}

#endif

/// Gets the status code of the current thread object.
/// @return a status code
NativeThread::status_t NativeThread::getStatus(void) const {
    return _status;
}

/// Gets the id of the current thread object.
/// @return  thread id
uint32 NativeThread::id(void) const {
    return _thrdID;
}

} // namespace common
} // namespace ZQ

