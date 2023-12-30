// thrdConnService.h: interface for the thrdConnService class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_thrdConnService_H__460822AC_7E7A_4E02_8E5A_11B8572CA4CF__INCLUDED_)
#define AFX_thrdConnService_H__460822AC_7E7A_4E02_8E5A_11B8572CA4CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NativeThread.h"
//#include "ScLog.h"
#include "FileLog.h"
using namespace ZQ::common;

class thrdConnService : public ZQ::common::NativeThread  
{
protected:
	//static ZQ::common::ScLog *		pLog;
	static ZQ::common::FileLog *		pLog;
public:
	thrdConnService();
	//thrdConnService(ZQ::common::ScLog * log);
	thrdConnService(ZQ::common::FileLog * log);
	virtual ~thrdConnService();
	void stop();
protected:
	int run();
	void final(void);
	bool init(void);
	HANDLE _event;

};

#endif // !defined(AFX_thrdConnService_H__460822AC_7E7A_4E02_8E5A_11B8572CA4CF__INCLUDED_)
