#pragma once
#include <NativeThread.h>
#include <set>

class DataTunnelAppServer;

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


class DataTunnelAppMainThread: public ZQ::common::NativeThread {
	
	friend class DataTunnelAppThread;
public:
	DataTunnelAppMainThread();
	virtual ~DataTunnelAppMainThread();

	void clear();
	void stop();
	void destory();
	
	HANDLE getThreadHandle()
	{
		NativeThreadEx* pthis = (NativeThreadEx* )this;
		return pthis->_hThread;
	}
	ZQ::common::Log*		m_pIceLog;
	
protected:

	virtual bool init();
	virtual int run();
	virtual void final();

	void addSubthread(DataTunnelAppThread* thread);
	void removeSubthread(DataTunnelAppThread* thread);

	void stopSubthreads();

protected:
	DataTunnelAppServer*	_appSvr;
	typedef std::set<DataTunnelAppThread*> ThreadSet;

	ThreadSet		_subthreads;
	Ice::LoggerPtr			m_iceLogger;
};
