// thrdCleanupSession.h: interface for the thrdCleanupSession class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_thrdCleanupSession_H__D0A66959_3329_40CB_8F20_9E2F0A62EFD1__INCLUDED_)
#define AFX_thrdCleanupSession_H__D0A66959_3329_40CB_8F20_9E2F0A62EFD1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NativeThread.h"
//#include "ScLog.h"
#include "FileLog.h"

using namespace std;
using namespace ZQ::common;

class thrdCleanupSession : public NativeThread  
{
protected:
	//static ZQ::common::ScLog *		pLog;
	static ZQ::common::FileLog *		pLog;
public:
	thrdCleanupSession();
	//thrdCleanupSession(ZQ::common::ScLog * log);
	thrdCleanupSession(ZQ::common::FileLog * log);
	virtual ~thrdCleanupSession();
	void stop();

protected:
	int run();
	void final(void);
	bool init(void);

	HANDLE _hThread;
	HANDLE _hExit;


};

#endif // !defined(AFX_thrdCleanupSession_H__D0A66959_3329_40CB_8F20_9E2F0A62EFD1__INCLUDED_)
