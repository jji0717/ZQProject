#pragma once

#include <Ice/Ice.h>
#include "DataStreamImpl.h"

class DODStreamerServer;

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
	DODStreamerServer*		_server;
	HANDLE					_exitEvent;
};


