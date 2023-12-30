#if !defined(SMSPROCESSRAWMSGTHREAD_H)
#define SMSPROCESSRAWMSGTHREAD_H
#include "afx.h"

#include "NativeThread.h"
using namespace ZQ::common;

#include <queue>
using namespace std;
#include <string>
using namespace std;

//#include "ModemGateway.h"
#include "NativeThreadPool.h"
#include "SMSContentProcessReq.h"
#include "SMSComPortWriteThread.h"
#include "SMSDB.h"

typedef queue<string> CMDSQUEUE;//test

class ModemGateway;

class SMSProcessRawMsgThread : public NativeThread
{
public:
	SMSProcessRawMsgThread(SMSComPortWriteThread* pWriteThread, ModemGateway* pModemGateway);
	~SMSProcessRawMsgThread();
	
public:	
	virtual bool init(void);
	virtual int run(void);
	virtual void final(void);
	
	void AddRawMsg(string cstRawMsg);

	bool stopProcessRawMsgThd();
	
	int  getSequenceID();
	
	SMSDB*  getDB() {return m_db;};

	//获取DB的ID
	int  getDBID();

	//delete sms
	void deleteSMS();
	
private:
	
	bool IsQueueRawMsgEmpty();//判断QueueRawMsg是否为空//need?

	void setDataCome();//通知线程有数据来了
	
	//list SMS
	void listSMS();

	//list db
	void listDB();
	
	//服务重启后读取数据库中的信息
	void LoadUnprocessedMsgFromDB();

	//初始化SMS Modem
	void InitializeSMSModem();

	//处理SMS的函数
	void ProcessRawMsgStr();

	//处理短信过来的标志 CMTI
	void ProcessSMSComeFlag(const char* pRawMsg);
	int  getSMSIndex(char* pCMTI);
	void readSMS(int SIMIndex);

	//处理短信的内容 CMGR
	void ProcessSMSContent(const char* pRawMsg);
	int getRawContentLength(char* pCMGR);
	
	//处理短信的列举命令 CMGL
	void ProcessSMSListCmd(const char* pRawMsg);
	int getListSMSLength(char* pCMGL, int* flag);

	//处理短信OK的命令
	void ProcessOK(const char* pRawMsg);

private:
	SMSComPortWriteThread* m_comportWriteThread;
	
	ModemGateway* m_ModemGateway;

	CMDSQUEUE m_QueueRawMsgStr;
	
	HANDLE  m_hStopEvent;
	HANDLE  m_hDataComeSem;
	HANDLE  m_timeOut;

	SMSDB*  m_db;

	int m_sequenceID;

	int m_dbId;

	// list SMS
	// 用于记录list命令的剩余的字符数
	int  m_list;
	// store list SMS
	char m_listRawSMS[320];

	// read SMS
	// 用于记录read命令的剩余的字符数
	int  m_read;
	// store read SMS
	char m_readRawSMS[320];

	int m_overtime;

	int m_echo;
	
};
#endif // !defined(SMSPROCESSRAWMSGTHREAD_H)