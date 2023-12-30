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

#define DB_CLEAR_HOURS 2//2小时

#define DB_OVER_TIME 20000//20分钟

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

	// TICP IP 地址的列表
	IPList*		m_ipList;

private:	
	// 与TM服务器交互等待的时间
	DWORD m_waitTime;

	// 与TM服务器交互的 id
	int   m_UID;
	int   m_MsgUID;

	// 与TICP服务器交互的 sequence id
	DWORD m_sequenceID;

	// 与TICP服务器连接的最大等待时间
	DWORD m_timeout;

	// 和TICP服务器重做的最大次数
	DWORD m_TicpRedoTimes;

	// 和TM服务器重做的最大次数
	DWORD m_TMRedoTimes;

	// 清数据库，清多少小时之前的数据
	DWORD m_DBClearHours;//以（小时）为单位

	// 读取数据库，读多少分钟前的数据
	DWORD m_DBOverTime;//以（分钟）为单位

	// data base path
	char  m_dbPath[MAX_PATH + 1];

	// 媒体控制对应表
	CMapStringToString* m_StreamCtrlFlgMap;

	// 反馈信息表
	std::vector <ReturnText> m_vec;

	// TM 的 Username 和 Password
	char   m_TmUsername[30];
	char   m_TmPassword[30];

	// 发给TM的ServiceID
	char   m_ServiceID[20];

	// 存放心跳最大个数
	DWORD  m_HeartbeatLogWindows;

	// 丢失心跳的最大允许个数
	DWORD  m_LostHeartbeatMaxCount;

	// 存放SPNumber的表
	CMapStringToString* m_SPNumberMap;

	// SMS 配置的初始化信息
	SMSConfigInfo m_info;
};

#endif // !defined(SMSSERVICE_H)