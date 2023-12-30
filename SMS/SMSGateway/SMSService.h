#if !defined(SMSSERVICE_H)
#define SMSSERVICE_H

#include "afx.h"
#include "BaseSchangeServiceApplication.h"

#include "RawMsgProcessThread.h"
#include "ReadSocketThread.h"
#include "WriteSocketThread.h"
#include "RespMsgProcessThread.h"
#include "DBThread.h"

#include "NPVRSocket.h"
#include "SMSMsg.h"
#include "SMSXmlProc.h"
#include "IpList.h"

#include <vector>
using namespace std;

#define MESSAGE_HEAD_LENGTH		12

#define SMSSRV_DEFAULT_WAIT_TIME INFINITE//just for test

#define TICP_PORT		64232
#define TICP_IP			_T("127.0.0.1")
#define SELECT_TIME_OUT		500000//just for test

#define TICP_REDO_TIMES 5

#define TM_REDO_TIMES 5

#define DB_CLEAR_HOURS 2//2Сʱ

#define DB_OVER_TIME 20000//20����

#define TM_SERVICE_ID 1001

#define MAX_HEARTBEAT 100

#define MAX_LOST_HEARTBEAT	10

#define SLEEP_TIME 4000

class WriteSocketThread;
class ReadSocketThread;
class RawMsgProcessThread;
class RespMsgProcessThread;
class DBThread;

struct ReturnText
{
	int		m_index;
	char	m_text[100];
	int		m_successFlag;
	
	void	setText(char* text)
	{
		memset(m_text, 0x00, 100*sizeof(char));
		strcpy(m_text, text);
	}
};

class SMSService : public ZQ::common::BaseSchangeServiceApplication
{
public:
	SMSService();
	~SMSService();

	HRESULT OnInit(void);
	HRESULT OnUnInit(void);

	HRESULT	OnStart(void);
	HRESULT OnStop(void);

	DBThread*				GetDBThread()    { return m_dbThread;};
	ReadSocketThread*		GetReadSocket()  { return m_pReadSocketThd; };
	WriteSocketThread*		GetWriteSocket() { return m_pWriteSocketThd; };
	RespMsgProcessThread*	GetRspMsgProc()  { return m_pRespMsgThd; };
	RawMsgProcessThread*	GetRawProc()     { return m_pRawMsgProcThd; };

	char*	GetACFlag()                { return m_info._ACFlag; };
	
	char*	GetCTFlag()                { return m_info._CTFlag; };
	
	char*	GetRGFlag()                { return m_info._RGFlag; };

	char*   GetNCFlag()                { return m_info._NCFlag; };

	char*	GetDBPath()                { return m_dbPath; };

	char*	GetServiceID()             { return m_ServiceID;};

	DWORD	GetWaitTime()              { return m_waitTime; };

	DWORD	GetSelectTimeOut()         { return m_timeout;};

	DWORD	GetTicpRedoTimes()         { return m_TicpRedoTimes;};

	DWORD   GetTMRedoTimes()           { return m_TMRedoTimes;};

	DWORD   GetDBClearHours()          { return m_DBClearHours;};

	DWORD   GetDBOverTime()            { return m_DBOverTime;};

	DWORD   GetResponseFlag()          { return m_info._response;};

	DWORD   GetErrorResponseFlag()     { return m_info._errorResponse;};

	DWORD   GetReplyHistoryFlag()      { return m_info._replyHistory;};

	DWORD	GetHeartbeatLogWindows()   { return m_HeartbeatLogWindows;};

	DWORD	GetLostHeartbeatMaxCount() { return m_LostHeartbeatMaxCount;};

	int		GetUID(bool content = false);
	DWORD	GetSequenceID();

	int		GetTicpFlag(char* contentFlag, char* realFlag);

	bool	GetReturnCode(int actionCode, char* returnText);

	int		GetTicpIP(char* ip, long& port);

	char*	GetTMIP() {return m_info._TMIp;};

	DWORD   GetTMPort() {return m_info._TMPort;};

	void	SetUID(int uid) { m_MsgUID = uid;};
	
	bool	IsReturnCodeSuccess(int ret);

	bool	GetTelephoneNumberProfix(char* SPNumber, char* telephoneProfix);

private:
	bool	ReadConfig();
	bool	ReadConfigXmlFile();
	bool	SetIPList();
	
private:
	DBThread*				m_dbThread;
	ReadSocketThread*		m_pReadSocketThd;
	WriteSocketThread*		m_pWriteSocketThd;
	RawMsgProcessThread*	m_pRawMsgProcThd;
	RespMsgProcessThread*	m_pRespMsgThd;

	SMSXmlProc  m_pXmlProc;

	// TICP IP ��ַ���б�
	IPList*		m_ipList;

private:	
	// ��TM�����������ȴ���ʱ��
	DWORD m_waitTime;

	// ��TM������������ id
	int   m_UID;
	int   m_MsgUID;

	// ��TICP������������ sequence id
	DWORD m_sequenceID;

	// ��TICP���������ӵ����ȴ�ʱ��
	DWORD m_timeout;

	// ��TICP������������������
	DWORD m_TicpRedoTimes;

	// ��TM������������������
	DWORD m_TMRedoTimes;

	// �����ݿ⣬�����Сʱ֮ǰ������
	DWORD m_DBClearHours;//�ԣ�Сʱ��Ϊ��λ

	// ��ȡ���ݿ⣬�����ٷ���ǰ������
	DWORD m_DBOverTime;//�ԣ����ӣ�Ϊ��λ

	// data base path
	char  m_dbPath[MAX_PATH + 1];

	// ý����ƶ�Ӧ��
	CMapStringToString* m_StreamCtrlFlgMap;

	// ������Ϣ��
	std::vector <ReturnText> m_vec;

	// TM �� Username �� Password
	char   m_TmUsername[30];
	char   m_TmPassword[30];

	// ����TM��ServiceID
	char   m_ServiceID[20];

	// �������������
	DWORD  m_HeartbeatLogWindows;

	// ��ʧ����������������
	DWORD  m_LostHeartbeatMaxCount;

	// ���SPNumber�ı�
	CMapStringToString* m_SPNumberMap;

	// SMS ���õĳ�ʼ����Ϣ
	SMSConfigInfo m_info;
};

#endif // !defined(SMSSERVICE_H)