// EventAddDataThread.h: interface for the EventAddDataThread class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __EVENTADDDATATRHEAD_H
#define __EVENTADDDATATRHEAD_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <NativeThread.h>
#include <Locks.h>
#include <ZQEventsCtrl.h>

class CEventsDialog;
class CEventAddDataThread : public ZQ::common::NativeThread    
{
public:
	CEventAddDataThread();
	CEventAddDataThread(CEventsDialog *pEventDlg);
	virtual ~CEventAddDataThread();
public:
	static int CALLBACK  OnStreamEvent( const string & strCategory,const  int &iLevel, const string & strCurTime, const string & strMessage);
public:
	int  run();
	void final(void);
	bool init(void);
	void stop();
protected:
	static CEventsDialog *m_pDlg;
	ZQ::common::Mutex		m_Mutex;
	bool m_bExit;
	TCHAR  m_strDllName[150];
};

#endif // !defined(AFX_EVENTADDDATATHREAD_H__6121A8EA_EBC3_4A81_8F9D_1B9BCBEF94A7__INCLUDED_)
