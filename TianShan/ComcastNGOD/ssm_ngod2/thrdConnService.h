// thrdConnService.h: interface for the thrdConnService class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_thrdConnService_H__460822AC_7E7A_4E02_8E5A_11B8572CA4CF__INCLUDED_)
#define AFX_thrdConnService_H__460822AC_7E7A_4E02_8E5A_11B8572CA4CF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NativeThread.h"
#include "FileLog.h"

class NGODEnv;
class thrdConnService : public ZQ::common::NativeThread  
{
protected:
	NGODEnv*										_pSsmNGODr2c1;
	ZQ::common::FileLog*								_pLog;
public:
	thrdConnService();
	void open(NGODEnv* pSsmNGODr2c1,ZQ::common::FileLog* pLog);
	virtual ~thrdConnService();
	void stop();
protected:
	int run();
	void final(void);
	bool init(void);
	HANDLE _event;
	bool b_exit;

};

#endif // !defined(AFX_thrdConnService_H__460822AC_7E7A_4E02_8E5A_11B8572CA4CF__INCLUDED_)
