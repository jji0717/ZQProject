// ConnChecker.h: interface for the ConnChecker class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "NativeThread.h"


class ConnChecker: public ZQ::common::NativeThread
{
public:
	ConnChecker(DWORD TimeOut/*ms*/);
	virtual ~ConnChecker();
	void StopChecker(void);
	
	// this function is invoked by outside, and let this thread to check and reconnection
	void TriggerCheck();

	bool isBroken();
private:
	int run(void);
	void Check(void);

private :
	bool m_bQuit;

	HANDLE m_hNofity;		// this event is set by outside to the connection check
	HANDLE m_EventStop;
	DWORD m_dwTimeOut;

	bool  m_isBroken;
};


