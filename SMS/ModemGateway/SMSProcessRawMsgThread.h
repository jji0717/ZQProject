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

	//��ȡDB��ID
	int  getDBID();

	//delete sms
	void deleteSMS();
	
private:
	
	bool IsQueueRawMsgEmpty();//�ж�QueueRawMsg�Ƿ�Ϊ��//need?

	void setDataCome();//֪ͨ�߳�����������
	
	//list SMS
	void listSMS();

	//list db
	void listDB();
	
	//�����������ȡ���ݿ��е���Ϣ
	void LoadUnprocessedMsgFromDB();

	//��ʼ��SMS Modem
	void InitializeSMSModem();

	//����SMS�ĺ���
	void ProcessRawMsgStr();

	//������Ź����ı�־ CMTI
	void ProcessSMSComeFlag(const char* pRawMsg);
	int  getSMSIndex(char* pCMTI);
	void readSMS(int SIMIndex);

	//������ŵ����� CMGR
	void ProcessSMSContent(const char* pRawMsg);
	int getRawContentLength(char* pCMGR);
	
	//������ŵ��о����� CMGL
	void ProcessSMSListCmd(const char* pRawMsg);
	int getListSMSLength(char* pCMGL, int* flag);

	//�������OK������
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
	// ���ڼ�¼list�����ʣ����ַ���
	int  m_list;
	// store list SMS
	char m_listRawSMS[320];

	// read SMS
	// ���ڼ�¼read�����ʣ����ַ���
	int  m_read;
	// store read SMS
	char m_readRawSMS[320];

	int m_overtime;

	int m_echo;
	
};
#endif // !defined(SMSPROCESSRAWMSGTHREAD_H)