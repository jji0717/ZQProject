#pragma once

#include <NativeThread.h>
#include <set>

class DODAppServer;

class DODAppThread: public ZQ::common::NativeThread {

public:
	DODAppThread();
	virtual ~DODAppThread();
	virtual void stop() = 0;
};


class NativeThreadEx
{
public:
	virtual void vf()
	{

	}

	typedef enum _status
	{
		stDeferred	=0,
		stInitial		=1,	
		stRunning,
		stDisabled,
		stDefault     =stDeferred
	} status_t;

#ifdef WIN32
//	unsigned long _thrdID;
	unsigned _thrdID;
#else
	DWORD _thrdID;
#endif
	status_t _status;

	// log information
	int _msgpos;
	char _msgbuf[128];

// #ifdef WIN32

	union _flag
	{
		struct
		{
			bool active :1;
		} b;

		DWORD B;
	} _flags;

//	static unsigned long __stdcall _execute(void *th);
	static unsigned __stdcall _execute(void *th);
	HANDLE	_hThread;
// #endif
};


class DODAppMainThread: public ZQ::common::NativeThread {
	
	friend class DODAppThread;
public:
	DODAppMainThread();
	virtual ~DODAppMainThread();

	void clear();
	void stop();
	void destory();
	
	HANDLE getThreadHandle()
	{
		NativeThreadEx* pthis = (NativeThreadEx* )this;
		return pthis->_hThread;
	}
	
protected:

	virtual bool init();
	virtual int run();
	virtual void final();

	void addSubthread(DODAppThread* thread);
	void removeSubthread(DODAppThread* thread);

	void stopSubthreads();

protected:
	DODAppServer*	_appSvr;
	typedef std::set<DODAppThread*> ThreadSet;

	ThreadSet		_subthreads;
};
