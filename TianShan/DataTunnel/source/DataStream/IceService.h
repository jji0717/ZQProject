#pragma once

#include <Ice/Ice.h>
#include "DataStreamServiceImpl.h"
#include "TianShanDefines.h"
class DataStreamServer;

class IceService {
public:
	IceService();

	virtual ~IceService();

	bool start();

	bool stop();

protected:

	static DWORD __stdcall win32ThreadProc(void* param);

	virtual bool init ();
	virtual unsigned long run();
	virtual void final();


protected:
	unsigned long			_threadId;
	HANDLE					_threadHandle;
	DataStreamServer*		_server;
	HANDLE					_exitEvent;
	bool					_stopped;
};


