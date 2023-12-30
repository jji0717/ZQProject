/**********************************************************
 $Id: Thread.h,v 1.6 2003/06/18 20:43:11 shao Exp $
 $Author: Admin $
 $Revision: 1 $
 $Date: 10-11-12 16:01 $

Copyright (C) 2002-2003 Shao Hui.
All Rights Reserved, Shao Hui.
**********************************************************/

#ifndef __Thread_h__
#define __Thread_h__

#include "MPFCommon.h"
using namespace ZQ::MPF::utils;

MPF_NAMESPACE_BEGIN  

class __declspec(dllexport) Thread
{
public:

	typedef enum _status
	{
		DEFERRED	=0,
		INITIAL	,
		RUNNING,
		DISABLED,
		DEFAULT     =DEFERRED
	} status_t;

	Thread();
	virtual ~Thread();

	bool start();

private:

	status_t _Status;

	// log information
	int _msgpos;
	char _msgbuf[512];

	static DWORD __stdcall  _execute(void *th);

	HANDLE	_hThread;

	HANDLE  _evtExit;
protected:

	virtual bool init(void)	{ return true; }
	virtual void run(void) = 0;
	virtual void final(void){}

	void terminate(void);

	inline void setStatus(const status_t st){_Status=st;}

	inline status_t getStatus(void){return _Status;}
	
	bool isRunning(void);

	bool waitExit(long timeout);

};


MPF_NAMESPACE_END

#endif // __Thread_h__
