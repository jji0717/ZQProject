#include <afx.h>
#include "SMSMsg.h"

#include "Log.h"
#include "ScLog.h"
using namespace ZQ::common;

//////////  Content ///////////////////////////////////////////////////////
Content::Content()
{
	memset(m_Key, 0x00, 21*sizeof(char));
	memset(m_Length, 0x00, 5*sizeof(char));
	m_Value = NULL;
	m_next = NULL;
}

Content::Content(char* key, char* length, char* value)
{
	memset(m_Key,	0x00, 21*sizeof(char));
	memset(m_Length, 0x00, 5*sizeof(char));
	SetKey(key);
	SetLength(length);
	SetValue(value);
	m_next = NULL;
}

Content::~Content()
{
	if (m_Value)
	{
		//glog(Log::L_INFO, "Content::delete m_Value <%s>", m_Value);
		delete m_Value;
	}
	m_Value = NULL;
}

Content* Content::GetNode(char* key, char* length, char* value, Content* nextlink)
{
	Content* newContent = new Content(key, length, value);
	newContent->m_next = nextlink;
	return newContent;
}

void Content::SetKey(char* key)
{
	if (strlen(key) > KEY_LENGTH)
	{
		return;
	}
	strcpy(m_Key, key);
}

void Content::SetLength(char* length)
{
	if (strlen(length) > LENGTH_LENGTH)
	{
		return;
	}
	strcpy(m_Length, length);
}

void Content::SetValue(char* value)
{
	m_Value = new char[strlen(value) + 1];
	strcpy(m_Value, value);
}

void Content::TraceContent(Content* p)
{
	if (p != NULL)
	{
		TraceContent(p->m_next);
		glog(Log::L_DEBUG, "Key <%s>, Length <%s>, Value <%s>", p->m_Key,
															    p->m_Length,
															    p->m_Value);
	}
}

//////////  SMSMsg ///////////////////////////////////////////////////////

SMSMsg::SMSMsg(int packLength, int cmd, int uid)
: Message(packLength, cmd, uid)
{
	m_first = NULL;
//	m_content = NULL;

//	m_SMSContent = NULL;
	memset(m_SMSContent, 0x00, 1000*sizeof(char));

//	m_SendContent = NULL;
	memset(m_SendContent, 0x00, 500*sizeof(char));

//	m_SMSTICPContent = NULL;
	memset(m_SMSTICPContent, 0x00, 500*sizeof(char));

	memset(m_ServiceCode, 0x00, 20*sizeof(char));

	memset(m_CallerPhoneNumber, 0x00, 50*sizeof(char));

	memset(m_SendTime, 0x00, 20*sizeof(char));
	
	m_SendToTM_Time = GetTickCount();
	m_ContentLength = m_PackLength - 12;
}

SMSMsg::~SMSMsg()
{
	Content* p;

	while (m_first)
	{
		p = m_first;
		m_first = p->m_next;
		delete p;
		p = NULL;
	}
/*	
	if (m_content != NULL)
	{
		glog(Log::L_INFO, "SMSMsg:: delete m_content");
		delete m_content;
	}
	
	if (m_SMSContent)
	{
		delete m_SMSContent;
	}
	
	if (m_SMSTICPContent)
	{
		delete m_SMSTICPContent;
	}

	if (m_SendContent)
	{
		delete m_SendContent;
	}
*/	
	m_first = NULL;
}

// call number set 
void SMSMsg::SetCallNumber(char* callNumber) 
{ 
	memset(m_CallerPhoneNumber, 0x00, 50*sizeof(char));
	strcpy(m_CallerPhoneNumber, callNumber);
}

// send time set
void SMSMsg::SetSendTime(char* sendTime) 
{
	memset(m_SendTime, 0x00, 20*sizeof(char));
	strcpy(m_SendTime, sendTime);
}

// service code set
void SMSMsg::SetServiceCode(char* serviceCode)
{
	memset(m_ServiceCode, 0x00, 20*sizeof(char));
	strcpy(m_ServiceCode, serviceCode);
}

// sms content set
char* SMSMsg::AddSMSContent(char* SMSContent)
{
//	int len = strlen(SMSContent);
//	m_SMSContent = new char[len + 1];
	memset(m_SMSContent, 0x00, 100*sizeof(char));
	strcpy(m_SMSContent, SMSContent);
//	m_SMSContent[strlen(SMSContent)] = 0;
	return m_SMSContent;
}

// content which receive from TICP set
char* SMSMsg::AddTicpContent(char* TicpContent)
{
//	int len = strlen(TicpContent);
//	m_SMSTICPContent = new char[len + 1];
	memset(m_SMSTICPContent, 0x00, 500 * sizeof(char));
	strcpy(m_SMSTICPContent, TicpContent);
	return m_SMSTICPContent;
}

// content which send to TICP set
char* SMSMsg::AddSendContent(char* SendContent)
{
//	int len = strlen(SendContent);
//	m_SendContent = new char[strlen(SendContent) + 1];
	memset(m_SendContent, 0x00, 500 * sizeof(char));
	strcpy(m_SendContent, SendContent);
	return m_SendContent;
}

char* SMSMsg::AddContent(char* content)
{
	if (m_ContentLength <= 0)
	{
		m_ContentLength = m_PackLength - 12;
		if (m_ContentLength <= 0)
		{
			//m_content = NULL;
			//m_SMSContent = NULL;
			m_first = NULL;
			return NULL;
		}
	}

/*	m_content = new char[m_ContentLength + 1];
	memset(m_content, 0x00, (m_ContentLength + 1)*sizeof(char));
*/
	memcpy(m_content, content, m_ContentLength);
	return m_content;
}

char* SMSMsg::AddContent(char* key, char* length, char* value)
{
	if (strlen(key) > KEY_LENGTH)
	{
		return NULL;
	}
	if (strlen(length) > LENGTH_LENGTH)
	{
		return NULL;
	}

	int len = m_ContentLength + KEY_LENGTH + LENGTH_LENGTH + strlen(value);
/*	char *temp = new char[len + 1];
	memset(temp, 0x00, (len + 1)*sizeof(char));
	if (m_content)
	{
		memcpy(temp, m_content, m_ContentLength);
	}
	AddKeyContent(temp + m_ContentLength, key, length, value);
	delete m_content;
	m_content = temp;
*/	
	AddKeyContent(m_content + m_ContentLength, key, length, value);
	m_ContentLength = len;
//	int packLength = 12 + m_ContentLength;
	m_PackLength = 12 + m_ContentLength;
	return m_content;
}

void SMSMsg::AddKeyContent(char* des, char* key, char* length, char* value)
{
	strncpy(des, key, KEY_LENGTH);
	strncpy(des + KEY_LENGTH, length, LENGTH_LENGTH);
	strcpy(des + KEY_LENGTH + LENGTH_LENGTH, value);
}

void SMSMsg::ParseLeftContentFromDB(char* leftContent, int leftContentLength)
{
	m_ContentLength = leftContentLength;

//	m_content = new char[m_ContentLength + 1];

	memset(m_content, 0x00, 2000*sizeof(char));

	memcpy(m_content, leftContent, m_ContentLength);
	
	ParseContent();
}

void SMSMsg::ParseContent()
{	
//	glog(Log::L_INFO, "SMSMsg::ParseContent()");

	int currentAt = 0;
	char key[KEY_LENGTH + 1];
	char length[LENGTH_LENGTH + 1];
	char* value;
	Content* p = NULL;

	m_LeftContentLength = m_ContentLength;
	
	int need = 0;
	
	while(currentAt < m_ContentLength)
	{
		strncpy(key, m_content + currentAt, KEY_LENGTH);
		currentAt += KEY_LENGTH;

		need = IsNeeded(key);	

		strncpy(length, m_content + currentAt, LENGTH_LENGTH);
		currentAt += LENGTH_LENGTH;
		
		int len = atoi(length);
		value = new char[len + 1];
		memset(value, 0x00, (len + 1)*sizeof(char));
		strncpy(value, m_content + currentAt, len);
		currentAt += len;

		if (need == 0)
		{
			p = p->GetNode(key, length, value, p);
		}
		else
		{
			PutValueIntoSuitableArray(need ,value);
			m_LeftContentLength -= (KEY_LENGTH + LENGTH_LENGTH + len);
		}
		
		delete value;

/*		//just for test
		if (need == 1)
			glog(Log::L_DEBUG, "ServiceCode = %s", m_ServiceCode);
		else if (need == 2)
			glog(Log::L_DEBUG, "CallerPhoneNumber= %s", m_CallerPhoneNumber);
		else if (need == 3)
			glog(Log::L_DEBUG, "SMSContent= %s", m_SMSContent);
		else if (need == 4)
			glog(Log::L_DEBUG, "SendTime= %s", m_SendTime);

		int ret = GetLastError();
		////////////////////////////////////////
		ret = 0;
*/		need = 0;
	}
	m_first = p;
//	glog(Log::L_DEBUG, "SMSMsg:: delete m_content");

	memset(m_content, 0x00, 2000*sizeof(char));
//	delete m_content;
//	m_content = NULL;
}

int SMSMsg::IsNeeded(char* key)
{
	if (strcmp(key, SERVICE_CODE) == 0)
	{
		return 1;
	}
	else if (strcmp(key, TELLPHONE_NUMBER) == 0)
	{
		return 2;
	}
	else if (strcmp(key, SMS_CONTENT) == 0)
	{
		return 3;
	}
	else if (strcmp(key, SMS_TIME) == 0)
	{
		return 4;
	}
	else if (_stricmp(key, LOGIN_STATE) == 0)
	{
		return 5;
	}
	return 0;
}

void SMSMsg::PutValueIntoSuitableArray(int need, char* value)
{
	if (need == 1)
	{
		SetServiceCode(value);
		//memset(m_ServiceCode, 0x00, 5*sizeof(char));
		//strcpy(m_ServiceCode, value);
	}
	else if (need == 2)
	{
		SetCallNumber(value);
		//memset(m_CallerPhoneNumber, 0x00, 12*sizeof(char));
		//strcpy(m_CallerPhoneNumber, value);
	}
	else if (need == 3)
	{
		AddSMSContent(value);
		//m_SMSContent = new char[strlen(value) + 1];
		//strcpy(m_SMSContent, value);
		//m_SMSContent[strlen(value)] = 0;
	}
	else if (need == 4)
	{
		SetSendTime(value);
		//memset(m_SendTime, 0x00, 20*sizeof(char));
		//strcpy(m_SendTime, value);
	}
	else if (need = 5)
	{
		int loginState = atoi(value);
		SetLoginState(loginState);
	}
}

void SMSMsg::TraceSMS()
{
	glog(Log::L_DEBUG, "PackLength <%d>, CMD <%d>, UID <%d>", m_PackLength,
															 m_Cmd,
															 m_UID);

	if (strlen(m_ServiceCode) > 0)
	{
		glog(Log::L_DEBUG, "SPNumber = %s", m_ServiceCode);
	}

	if (strlen(m_CallerPhoneNumber) > 0)
	{
//		glog(Log::L_DEBUG, "CallerPhoneNumber = %s", m_CallerPhoneNumber);
		glog(Log::L_DEBUG, "手机号码 = %s", m_CallerPhoneNumber);
	}

	if (strlen(m_SMSContent) > 0)
	{
//		glog(Log::L_DEBUG, "SMSContent = %s", m_SMSContent);
		glog(Log::L_DEBUG, "短信内容 = %s", m_SMSContent);
	}

	if (strlen(m_SendTime) > 0)
	{
//		glog(Log::L_DEBUG, "SendTime = %s", m_SendTime);
		glog(Log::L_DEBUG, "发送时间 = %s", m_SendTime);
	}

	glog(Log::L_DEBUG, "LeftContentLength = %d", m_LeftContentLength);
	
	m_first->TraceContent(m_first);
}

int SMSMsg::CombineContentMessage()
{
/*	if (m_content)
	{
		return m_ContentLength;
	}
*/	
	// 说明 m_content 里面有内容，清空重新拼接
	if (strlen(m_content) > 0)
	{
		memset(m_content, 0x00, 2000*sizeof(char));
	}
	m_ContentLength = 0;
	
	// 初始化 packagelength
	int packageLength = m_LeftContentLength;
	
/*	if (strlen(m_SMSTICPContent) <= 0)
	{
		return -1;
	}
*/
	// 回复的时候只要再加上 回复短信内容 和 电话号码
	bool bTicpContent = false;
	if (strlen(m_SMSTICPContent) > 0)
	{
		packageLength += 20 + 4 + strlen(m_SMSTICPContent);	// 加上回复短信内容的Key，Length，Value的长度
		bTicpContent = true;
	}
	
	bool bPhoneNumber = false;
	if (strlen(m_CallerPhoneNumber) > 0)
	{
		bPhoneNumber = true;
		packageLength += 20 + 4 + strlen(m_CallerPhoneNumber); // 加上电话号码的Key，Length，Value的长度
	}

	bool bServiceCode = false;
	if (strlen(m_ServiceCode) > 0)
	{
		bServiceCode = true;
		packageLength += 20 + 4 + strlen(m_ServiceCode); //加上ServiceCode(SPNumber)的Key，Length，Value的长度
	}

	packageLength += 12; // 加上报头的长度(12) 等于 总长度

/*	m_content = new char[packageLength + 1];
	memset(m_content, 0x00, (packageLength + 1)*sizeof(char));
*/
	memset(m_content, 0x00, 2000*sizeof(char));

	int netPackageLength = htonl(packageLength);
	memcpy(m_content, &netPackageLength, 4);

	int netCmd = htonl(GetCmd());
	memcpy(m_content + 4, &netCmd, 4);

	int netUid = htonl(m_UID);
	memcpy(m_content + 8, &netUid, 4);
	
	//glog(Log::L_DEBUG, "CombineMessage() : packageLength <%d>, cmd <%d>, uid <%d>", packageLength, CMD_MSG, m_UID);
	//glog(Log::L_DEBUG, "CombineMessage() : net  packLen  <%d>, cmd <%d>, uid <%d>", netPackageLength, netCmd, netUid);
	
	// 全局长度变量 初始化为 报头长度
	m_ContentLength = 12;

	int  len;
	char length[5];

	// add phone number
	if (bPhoneNumber)
	{
		len = strlen(m_CallerPhoneNumber);
		memset(length, 0x00, 5*sizeof(char));
		sprintf(length, "%d", len);
		AddKeyContent(m_content + m_ContentLength, "UserPhone", length, m_CallerPhoneNumber);
		m_ContentLength += 20 + 4 + len; // 全局长度变量 再加上 电话号码 的长度
	}

	// add sms response content
	if (bTicpContent)
	{
		len = strlen(m_SMSTICPContent);
		memset(length, 0x00, 5*sizeof(char));
		sprintf(length, "%d", len);
		AddKeyContent(m_content + m_ContentLength, "Content", length, m_SMSTICPContent);
		m_ContentLength += 20 + 4 + len; // 全局长度变量 再加上 回复短信 的长度
	}

	// add ServiceCode 
	if (bServiceCode)
	{
		len = strlen(m_ServiceCode);
		memset(length, 0x00, 5*sizeof(char));
		sprintf(length, "%d", len);
		AddKeyContent(m_content + m_ContentLength, SERVICE_CODE, length, m_ServiceCode);
		m_ContentLength += 20 + 4 + len; // 全局长度变量 再加上 ServiceCode 的长度
	}

	// add other data
	Content* p = m_first;
	while (p)
	{
		AddKeyContent(m_content + m_ContentLength, p->GetKey(), p->GetLength(), p->GetValue());
		m_ContentLength += 20 + 4 + strlen(p->GetValue());// 全局长度变量 再加上 其他内容 的长度
		p = p->m_next;
	}

	if (packageLength == m_ContentLength)
	{
		return m_ContentLength;
	}
	return -1;
}

////////// Message //////////////////////////////////////////////////////

Message::Message (int packLength, int cmd, int uid)
:m_PackLength(packLength), m_Cmd(cmd), m_UID(uid)
{
	m_bTicpFinished = false;
	m_TicpProcTimes = 0;
	m_bSMSFinished	= false;
	m_SMSProcTimes  = 0;

	// 初始登陆状态为 1
	m_LoginState = 1;
}

Message::~Message()
{
}

bool Message::SetPackageLength(int packLength)
{
	if (packLength < 12)
	{
		return false;
	}
	m_PackLength = packLength;
	return true;
}

bool Message::SetCmd(int cmd)
{
	if ( (cmd == CMD_LOGIN)	   || (cmd == CMD_LOGIN_RESP) || (cmd == CMD_MSG) || 
		 (cmd == CMD_MSG_RESP) || (cmd == CMD_HEARTBEAT)  || (cmd == CMD_HEARTBEAT_RESP) )
	{
		m_Cmd = cmd;
		return true;
	}
	return false;
}

void Message::CombineMessage(char* content)
{
	int netPackageLength = htonl(m_PackLength);
	memcpy(content, &netPackageLength, 4);

	int netCmd = htonl(m_Cmd);
	memcpy(content + 4, &netCmd, 4);

	int netUid = htonl(m_UID);
	memcpy(content + 8, &netUid, 4);
}



