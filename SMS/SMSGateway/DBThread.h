#if !defined (DBTHREAD_H)
#define DBTHREAD_H

#include <queue>
using namespace std;

#include "SMSDB.H"
#include "SMSService.h"
#include "SMSMsg.h"
#include "RawMsgProcessThread.h"
#include "WriteSocketThread.h"
#include "AFXMT.h"

#include "NativeThread.h"
using namespace ZQ::common;

typedef queue<LPSMSMSG>	 SMSMSGQUEUE;

class SMSService;
class WriteSocketThread;
class RawMsgProcessThread;

class DBThread : public NativeThread
{
public:
	DBThread(SMSService* pSrv);
	~DBThread();

	virtual bool init(void);
	virtual int  run(void);
	virtual void final(void);

	int		GetUID();
	bool	putInsertMsg(LPSMSMSG pSMS);
	bool	putUpdateMsg(LPSMSMSG pSMS);

	void	SelectUnfinishedFromDB();

	bool    StopThread();

private:
	bool	setDataCome();

	void	ProcessMessage();

	// 处理插入队列
	void	ProcessInsertQueue();

	// 处理更新队列
	void	ProcessUpdateQueue();

	void	DeleteOverdueMessagesInDB();
	
	void	DeleteMessage(LPSMSMSG pSMS);

private:
	SMSDB  m_npvrdb;
	SMSService* m_pService;

	SMSMSGQUEUE m_InsertQueue;
	SMSMSGQUEUE m_UpdateQueue;

	char m_dbPath[MAX_PATH + 1];

	HANDLE m_hStop;
	HANDLE m_hDataComeSem;

	// DB是否存在的标志符
	bool m_bDBExist;

	DWORD m_DBClearHours;

	DWORD m_DBOverTime;

	CCriticalSection m_criticalSection;
};

#endif