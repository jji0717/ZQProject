// TestServer.h: interface for the TestServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESTSERVER_H__43D87F74_AE25_41E7_9213_238745D816A5__INCLUDED_)
#define AFX_TESTSERVER_H__43D87F74_AE25_41E7_9213_238745D816A5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "nativethread.h"
#include "Log.h"
#include "Locks.h"
#include <string>

class ConsoleLog : public ZQ::common::Log
{
public:
	ConsoleLog();
	~ConsoleLog();
	int activate();
	int deactivate();

protected:
	/// implement Log::writeMessage() 
	/// @param msg     - the log msg body to write down
	/// @param level   - log level. the verbosity testing has already been
	///                  performed when this method is called.
	virtual void writeMessage(const char *msg, int level=-1);

private:
	HANDLE _hOutput;
	ZQ::common::Mutex	_lock;
};

class TestServer : public ZQ::common::NativeThread
{
public:
	TestServer(::std::string endpoint);
	virtual ~TestServer();

	void signalStop();
protected:
	virtual bool init(void)	{ return true; }
	
	virtual int run(void);
	
	virtual void final(void) { return; }


private:
	::std::string	_endpoint;
	bool _bListening;
};

extern ConsoleLog g_ConsoleLogger;

#endif // !defined(AFX_TESTSERVER_H__43D87F74_AE25_41E7_9213_238745D816A5__INCLUDED_)
