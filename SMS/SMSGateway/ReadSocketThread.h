#if !defined (READSOCKETTHREAD_H)
#define READSOCKETTHREAD_H

#include "NPVRSocket.h"
#include "SMSService.h"
#include "SMSMsg.h"
#include "DBThread.h"
#include "WriteSocketThread.h"
#include "NativeThread.h"
using namespace ZQ::common;

class SMSService;
class DBThread;
class WriteSocketThread;

class ReadSocketThread : public NativeThread
{
public:
	ReadSocketThread(SMSService* pSrv);
	~ReadSocketThread();

	virtual bool init(void);
	virtual int run(void);
	virtual void final(void);

	bool	sendLoginMsg();
	
	void	ReConnect();

	bool	StopThread();

	NPVRSocket* GetNPVRSocket() { return m_pNSocket; };
	
private:
	// �����½�ظ���Ϣ
	void	ProcessLoginResp();

	void	Classify(int packagelength, int cmd, int uid, char* content);

	// ������½�ظ�
	bool	ParseLoginResponse(LPMessage pMsg);

	// ��������ȴ�ʱ��
	void	clearHeartBeatTime() { m_HeartbeatWaitTime = 0; };

	// �����������
	void	clearHeartBeatCount() { m_HeartbeatCount = 0; };

	// ��ն�ʧ����������
	void	clearLostHeartBeatCount() { m_LostHeartbeatCount = 0; };

	// ��������
	void	SendHeatbeat();

private:
	SMSService* m_pService;
	NPVRSocket* m_pNSocket;

private:
	HANDLE	 m_hStop;

	WSAEVENT m_hWSAEvent;

	// TM�������Ķ˿�
	DWORD	m_port;
	
	// TM��������ip��ַ
	char	m_ip[20];
	
	// ��¼�µȴ�ʱ��
	DWORD  m_WaitForTmTime;

	// ��־�ѷ���һ��heartbeat��û���յ��ظ�ȷ��
	DWORD  m_HeartbeatWaitTime;

	int	   m_heatbeatUID;

	// TM �� Username �� Password
	char   m_TmUsername[30];
	char   m_TmPassword[30];

	// ����TM��ServiceID
	char   m_ServiceID[20];

	// heartbeat�ļ�¼����
	DWORD  m_HeartbeatCount;

	// ��ʧ��heartbeat�ĸ���
	DWORD  m_LostHeartbeatCount;
};

#endif