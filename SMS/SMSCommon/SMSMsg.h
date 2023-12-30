#if !defined(AFX_SMSMSG_H)
#define AFX_SMSMSG_H

#define CMD_LOGIN			0x00000001
#define CMD_LOGIN_RESP		0x80000001
#define CMD_MSG				0x00000002
#define CMD_MSG_RESP		0x80000002
#define CMD_HEARTBEAT		0x00000003
#define CMD_HEARTBEAT_RESP	0x80000003

#define KEY_LENGTH			20
#define LENGTH_LENGTH		4

// sms flags
#define SERVICE_CODE		"SPNumber"
#define TELLPHONE_NUMBER	"UserPhone"
#define SMS_CONTENT			"Content"
#define SMS_TIME			"recvTime"
#define LOGIN_STATE			"state"

class Message
{
public:
	Message (int packLength, int cmd, int uid);
	~Message ();

	int		GetPackageLength() {return m_PackLength;};
	bool	SetPackageLength(int packLength);// maybe not need
	
	bool	SetCmd(int cmd);
	int		GetCmd(){return m_Cmd;};
	
	void	SetUID(int uid) { m_UID = uid;};
	int		GetUID() { return m_UID;};

	void    TicpFinished(bool TicpFinished) { m_bTicpFinished = TicpFinished;};
	bool	IsTicpFinished() { return m_bTicpFinished; };

	void	SMSFinished(bool SMSFinished) { m_bSMSFinished = SMSFinished;};
	bool	IsSMSFinished()  { return m_bSMSFinished; };

	void	SetTicpTimes(int ticpTimes) { m_TicpProcTimes = ticpTimes;};
	int		GetTicpTimes() { return m_TicpProcTimes;};

	void	SetSMSTimes(int SMSTimes) { m_SMSProcTimes = SMSTimes;};
	int		GetSMSTimes() { return m_SMSProcTimes;};

	// Æ´½ÓMessage Content
	void	CombineMessage(char* content);

	void	SetLoginState(int loginState) {m_LoginState = loginState;};
	int		GetLoginState() {return m_LoginState;};

protected:
	int m_PackLength;
	int m_Cmd;
	int m_UID;

private:
	// ¼ÇÂ¼µÇÂ½½á¹û
	// µÇÂ½³É¹¦   m_LoginState = 0
	// µÇÂ½Ê§°Ü   m_LoginState = 1
	int m_LoginState;
	
	// this message has been finished by TICP
	bool m_bTicpFinished;
	
	// the times which this message has been processed by TICP
	int  m_TicpProcTimes;

	// this message has been finished by  SMS service
	bool m_bSMSFinished;

	// the times which this message has been processed by TM
	int m_SMSProcTimes;
};

typedef Message* LPMessage;

//////////////////////////////////////////////////////////////////////////
class SMSMsg;
// Content
class Content
{
	friend class SMSMsg;
private:
	char m_Key[21];
	char m_Length[5];
	char* m_Value;

public:
	Content();
	Content(char*, char*, char*);
	~Content();
	
	Content* GetNode(char*, char*, char*, Content*);

	void SetKey(char*);
	void SetLength(char*);
	void SetValue(char*);

	char* GetKey() {return m_Key;};
	char* GetLength() {return m_Length;};
	char* GetValue() {return m_Value;};

	void TraceContent(Content*);
	
public:
	Content* m_next;
};


// SMSMsg
class SMSMsg : public Message
{
public:
	SMSMsg(int packLength, int cmd, int uid);
	~SMSMsg();
	
	char*	GetContent() {return m_content;};
	char*	AddContent(char* key, char* length, char* value);
	char*	AddContent(char* con);

	// sms content operation
	char*	AddSMSContent(char* SMSContent);
	char*	GetSMSContent() { return m_SMSContent;};

	// content which send to TICP operation
	char*  AddSendContent(char* SendContent);
	char*  GetSendContent() { return m_SendContent;};

	// content which receive from TICP operation
	char*   AddTicpContent(char* TicpContent);
	char*	GetTicpContent() { return m_SMSTICPContent; };

	// call number operation
	void	SetCallNumber(char* callNumber);
	char*	GetCallNumber() { return m_CallerPhoneNumber;};

	// send time operation
	void	SetSendTime(char* sendTime);
	char*	GetSendTime()   { return m_SendTime;};

	// service code operation
	void	SetServiceCode(char* serviceCode);
	char*	GetServiceCode() {return m_ServiceCode;};

	// parse raw message content
	void	ParseContent();

	void	TraceSMS();

	// parse left message content from DB
	void	ParseLeftContentFromDB(char* leftContent, int leftContentLength);

	//combine message
	// return value is the combined content length
	// use function GetContent() to get content
	int		CombineContentMessage();

	// left content length operation
	void	SetLeftContentLength(int leftContentLength) { m_LeftContentLength = leftContentLength;};
	int		GetLeftContentLength() { return m_LeftContentLength;};

	// time which send to TM operation
	void	SetSendToTmTime(DWORD current) { m_SendToTM_Time = current;};
	DWORD	GetSendToTmTime() { return m_SendToTM_Time;};

private:
	int		IsNeeded(char* key);
	void	PutValueIntoSuitableArray(int need, char* value);
	
	// add key content
	void	AddKeyContent(char* des, char* key, char* length, char* value);

private:
	char  m_content[2000];

	int	  m_ContentLength;// the length of m_content

	int   m_LeftContentLength;//the length of content without the values I want

	char  m_ServiceCode[20];
	char  m_CallerPhoneNumber[50];
	char  m_SendTime[20];

	// store the short message content
	//char* m_SMSContent;
	char m_SMSContent[1000]; 
	
	// store the message content which send to TICP
	//char* m_SendContent;
	char m_SendContent[500];

	// store the TICP message content
	//char* m_SMSTICPContent;
	char m_SMSTICPContent[500];
	
	// store time which send to TM
	DWORD m_SendToTM_Time;
	
public:
	Content* m_first;
};

typedef SMSMsg* LPSMSMSG;

#endif