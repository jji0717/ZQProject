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
	// 处理登陆回复信息
	void	ProcessLoginResp();

	void	Classify(int packagelength, int cmd, int uid, char* content);

	// 解析登陆回复
	bool	ParseLoginResponse(LPMessage pMsg);

	// 清空心跳等待时间
	void	clearHeartBeatTime() { m_HeartbeatWaitTime = 0; };

	// 清空心跳个数
	void	clearHeartBeatCount() { m_HeartbeatCount = 0; };

	// 清空丢失的心跳个数
	void	clearLostHeartBeatCount() { m_LostHeartbeatCount = 0; };

	// 发送心跳
	void	SendHeatbeat();

private:
	SMSService* m_pService;
	NPVRSocket* m_pNSocket;

private:
	HANDLE	 m_hStop;

	WSAEVENT m_hWSAEvent;

	// TM服务器的端口
	DWORD	m_port;
	
	// TM服务器的ip地址
	char	m_ip[20];
	
	// 记录下等待时间
	DWORD  m_WaitForTmTime;

	// 标志已发送一个heartbeat但没有收到回复确认
	DWORD  m_HeartbeatWaitTime;

	int	   m_heatbeatUID;

	// TM 的 Username 和 Password
	char   m_TmUsername[30];
	char   m_TmPassword[30];

	// 发给TM的ServiceID
	char   m_ServiceID[20];

	// heartbeat的记录个数
	DWORD  m_HeartbeatCount;

	// 丢失的heartbeat的个数
	DWORD  m_LostHeartbeatCount;
};

#endif