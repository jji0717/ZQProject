#include "stdafx.h"
#include "JmsUI.h"
#include "Queue1Thread.h"
#include "sys\timeb.h"
#include "time.h"

Queue1Thread::Queue1Thread(const char* desname, 
						   TianShanIce::common::JMSPublisherManager* jmsPublisherMgr)
: m_Message(0),
  m_hStop(NULL), 
  m_hDataCome(NULL), 
  m_bStopSend(false),
  m_JmsPublisherMgr(jmsPublisherMgr)
{
	strnset(m_destName, 0, MAX_PATH);
	strncpy(m_destName, desname, MAX_PATH - 1);

	m_hStop = CreateEvent(NULL, false, false, NULL);
	m_hDataCome  = CreateEvent(NULL, false, false, NULL);
}

Queue1Thread::~Queue1Thread()
{
	CloseHandle(m_hStop);
	CloseHandle(m_hDataCome);
}

int Queue1Thread::run()
{
	DWORD status = 0;
	bool  bContinued = true;
	HANDLE handles[2] = { m_hStop, m_hDataCome };

	while (bContinued)
	{
		status = WaitForMultipleObjects(2, handles, false, INFINITE);
		switch(status)
		{
		case WAIT_OBJECT_0:
			bContinued = false;
			break;
			
		case WAIT_OBJECT_0 + 1:
			g_Queue1_Busy = true;
			if (m_Message == 1)
			{
				sendMessage1();
			}
			else if (m_Message == 2)
			{
				sendMessage2();
			}
			g_Queue1_Busy = false;
			break;

		default:
			break;
		}
	}

	return 1;
}

void Queue1Thread::sendMessage1()
{
	m_bStopSend = false;
	
	char time[30];
	char xml[1024];
	
	for (DWORD i = 0; i < m_SendTimes; i++)
	{
		if (m_bStopSend == true)
		{
			return;
		}
		
		strnset(xml, 0, 1024);
		createTime(time);

		sprintf(xml, "<DODMessage>\n"
					 "<DODMessageHeader Time=\" %s \" MessageCode=\"3004\"/>\n"
					 "<DODMessageBody DataType=\" %s \" GroupID=\" %s \">\n"
					 "<FileOperation Root=\" %s \" UpdateMode=\" %s \"/>\n"
					 "</DODMessageBody>\n"
					 "</DODMessage>", 
				time,
				m_DataType,
				m_GroupID,
				m_RootPath,
				m_UpdateMode);

		m_JmsPublisherMgr->publishMsg(m_destName, xml);
		
		Sleep(m_SendInterval);
	}
}

void Queue1Thread::sendMessage2()
{
	m_bStopSend = false;
	
	char time[30];
	char xml[1024];
	
	if (strlen(m_MessageID) == 0)
	{
		for (DWORD i = 0; i < m_SendTimes; i++)
		{
			if (m_bStopSend == true)
			{
				return;
			}

			strnset(xml, 0, 1024);
			createTime(time);

			sprintf(xml, "<DODMessage>\n"
						 "<DODMessageHeader Time=\" %s \" MessageCode=\"3005\"/>\n"
						 "<DODMessageBody DataType=\" %s \" GroupID=\" %s \">\n"
						 "<Message Datatype=\" %s \" Destination=\" %s \" DestinationType=\" %s \"\n"
						 "ExpiredTime=\" %s \" TimeStamp=\" %s \"></Message>\n"
						 "</DODMessageBody>\n"
						 "</DODMessage>",
					time,
					m_DataType,
					m_GroupID,
					m_DataType,
					m_Destination,
					m_DestinationType,
					m_ExpiredTime,
					time);

			m_JmsPublisherMgr->publishMsg(m_destName, xml);
			
			Sleep(m_SendInterval);
		}
	}
	else
	{
		for (DWORD i = 0; i < m_SendTimes; i++)
		{
			if (m_bStopSend == true)
			{
				return;
			}

			strnset(xml, 0, 1024);
			createTime(time);

			sprintf(xml, "<DODMessage>\n"
						 "<DODMessageHeader Time=\" %s \" MessageCode=\"3005\"/>\n"
						 "<DODMessageBody DataType=\" %s \" GroupID=\" %s \">\n"
						 "<Message MessageID = \" %s \" Datatype=\" %s \" Destination=\" %s \" DestinationType=\" %s \"\n"
						 "ExpiredTime=\" %s \" TimeStamp=\" %s \"></Message>\n"
						 "</DODMessageBody>\n"
						 "</DODMessage>",
					time,
					m_DataType,
					m_GroupID,
					m_MessageID,
					m_DataType,
					m_Destination,
					m_DestinationType,
					m_ExpiredTime,
					time);

			m_JmsPublisherMgr->publishMsg(m_destName, xml);
			
			Sleep(m_SendInterval);
		}
	}	
}

void Queue1Thread::createTime(char* strTime)
{
	strnset(strTime, 0, 30);
	
	_timeb timebuffer;
	_ftime(&timebuffer);

	char timeline[30];
	char date[10];
	char time[10];
	strnset(timeline, 0, 30);
	strnset(date, 0, 10);
	strnset(time, 0, 10);

	strcpy(timeline, ctime(&timebuffer.time));

	_strdate(date);
	_strtime(time);

	sprintf(strTime, "%s %s:%03d", date, time, timebuffer.millitm);
}

void Queue1Thread::pushMessage1(DWORD sendTimes,
							    DWORD sendInterval,
							    const char* groupID,
							    const char* dataType,
							    const char* updateMode,
							    const char* rootPath)
{
	m_Message = 1;
	m_SendTimes = sendTimes;
	m_SendInterval = sendInterval;
	
	strnset(m_GroupID, 0, 20);
	strncpy(m_GroupID, groupID, 20);

	strnset(m_DataType, 0, 20);
	strncpy(m_DataType, dataType, 20);
	
	strnset(m_UpdateMode, 0, 20);
	strncpy(m_UpdateMode, updateMode, 20);
	
	strnset(m_RootPath, 0, MAX_PATH);
	strncpy(m_RootPath, rootPath, MAX_PATH - 1);

	SetEvent(m_hDataCome);
}

void Queue1Thread::pushMessage2(DWORD sendTimes,
							    DWORD sendInterval,
							    const char* groupID,
							    const char* dataType,
							    const char* destination,
							    const char* destinationType,
							    const char* expiredTime,
								const char* messageID)
{
	m_Message = 2;
	m_SendTimes = sendTimes;
	m_SendInterval = sendInterval;
	
	strnset(m_GroupID, 0, 20);
	strncpy(m_GroupID, groupID, 20);

	strnset(m_DataType, 0, 20);
	strncpy(m_DataType, dataType, 20);

	strnset(m_Destination, 0, 20);
	strncpy(m_Destination, destination, 20);

	strnset(m_ExpiredTime, 0, 20);
	strncpy(m_ExpiredTime, expiredTime, 20);
	
	strnset(m_DestinationType, 0, 20);
	strncpy(m_DestinationType, destinationType, 20);

	strnset(m_MessageID, 0, 20);
	if (messageID)
	{
		strncpy(m_MessageID, messageID, 20);
	}

	SetEvent(m_hDataCome);
}