#if !defined(SMSCOMPORTWRITETHREAD_H)
#define SMSCOMPORTWRITETHREAD_H

#include "afx.h"

#include "NativeThread.h"
using namespace ZQ::common;

#include <queue>
#include <string>
using namespace std;

#include "ComCommunication.h"
#include "Locks.h"
#include "ModemGateway.h"

typedef queue<string>    STRCMDQUEUE;

class ModemGateway;

class SMSComPortWriteThread : public NativeThread
{
public:
	SMSComPortWriteThread(ModemGateway* pSMSSrv, int comPort);
	~SMSComPortWriteThread();

	virtual bool init(void);
	virtual int  run(void);
	virtual void final(void);

	void AddWriteMsg(string msg);
	
	CComCommunication* GetComCommunication();

	/// stop the write com thread
	bool     stopWriteThd();

	//this function supply other thread to notify this thread that new data in queue
	void     setDataCome();

private:

	// set PDU mode
	void     setPduMode();
	
	//call ComCommunication write functions to send cmd to ComPort
	void     sendCmdToComPort();

private:
	STRCMDQUEUE m_QueueWriteMsgStr;

	HANDLE	 m_hStopEvent;
	HANDLE   m_hDataCome;
	
	CComCommunication* m_pCommunication;

	ModemGateway* m_pSMSSrv;

	DWORD m_calc;
};
#endif  //!defined(SMSCOMPORTWRITETHREAD_H)