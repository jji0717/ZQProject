#if !defined(SMSCOMPORTREADTHREAD_H)
#define SMSCOMPORTREADTHREAD_H

#include "afx.h"
#include "NativeThread.h"
using namespace ZQ::common;

class CComCommunication;
class SMSComPortWriteThread;
class SMSProcessRawMsgThread;

class SMSComPortReadThread : public NativeThread
{
public:
	SMSComPortReadThread(CComCommunication* pComm, SMSProcessRawMsgThread* processThread, SMSComPortWriteThread* pWriteCom);
	~SMSComPortReadThread();

public:	
	virtual bool init(void);
	virtual int run(void);
	virtual void final(void);

	bool  stopReadThd();// stop this thread

private:
	CComCommunication*      m_pCommunication;
	SMSComPortWriteThread*  m_pWriteCom;
	SMSProcessRawMsgThread* m_pProcessRawMsg;

	HANDLE  m_hStopEvent;
	HANDLE  m_hDataComeFromComPort;

	DWORD   m_dwBytesToRead;
	
   	char m_dataBuffer[1024];
};
#endif  //!defined(SMSCOMPORTREADTHREAD_H)