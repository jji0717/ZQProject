#if !defined (TICPMSGPROCESSREQUEST_H)
#define TICPMSGPROCESSREQUEST_H

#include "SMSService.h"
#include "NativeThreadPool.h"
#include "SMSMsg.h"
#include "WriteSocketThread.h"
#include "DBThread.h"
#include "TicpProc.h"

#define OPERATION_SUCCESSFUL 1

class SMSService;
class TicpProc;

class TICPMsgProcessRequest : public ZQ::common::ThreadRequest
{
public:
	TICPMsgProcessRequest(ZQ::common::NativeThreadPool& Pool, SMSService* pSrv, LPMessage pMsg);
	~TICPMsgProcessRequest();

protected:
	int run();
	void final(int retcode, bool bCancelled);

private:
	// ���� TICP �ķ���ֵ �����ø� TM �Ķ�������
	void SetMesssageContent(int ret);

	// ����Ҫ���͸� TICP �Ķ�������
	bool ProcSendContent();

	// ������ Request
	void EndRequest();
	
	// put content into WriteSocketThread
	void PutContentIntoWriteThrd();

	// ���� Response �� ErrorResponse ���������
	void LastProcessMessage();

	// �������ݿ�
	void InsertDB();
	
	// �������ݿ�
	void UpdateDB();

	// ��ȡҪ���͸�TICP�ĺ���
	void GetRealTelephoneNumber(char* realTelephoneNumber);

private:
	SMSMsg* m_pSMSMsg;
	SMSService* m_pService;

	// �� TICP ����ʱ�� sequence id
	DWORD m_sequenceID;

	// TICP �� IP Address �� port
	char m_ip[20];
	long m_port;

	// �� TICP ����ʱ���ȴ�ʱ��
	DWORD m_timeout;

	// TICP �ķ���ֵ
	int m_ret;

	// ���յ��Ķ��������еĿ�ͷ��־
	char m_ACFlag[10];//stream control
	char m_CTFlag[10];//chat control
	char m_RGFlag[10];//register control
	int  m_mode;

	// this request has been dealed times
	int  m_processTimes;
	DWORD m_MaxRedoTimes;

	// this content which send to TICP
	char m_SendContent[100];

	// �㲥����ظ� TM �ı�־
	DWORD m_Response;

	// �㲥�������ظ� TM �ı�־
	DWORD m_ErrorResponse;

	TicpProc* m_ticpProc;

	// �ǳƱ�־
	char m_NCFlag[10];
};

#endif