//file to define the thread
//inherit NativeThread
//chop the RTSP message from NGOD server
//after chop, stored the message into a message list
#pragma once

#include "ngod_common_structure.h"
#include "../NSSEventSinkI.h"

//define for debug
//#define NGODPARSEDEBUG

typedef struct StreamStrList 
{
	StreamStrList()
	{
		InitializeCriticalSection(&m_CS);
		m_StreamStrList.clear();
	}
	~StreamStrList()
	{
		DeleteCriticalSection(&m_CS);
		m_StreamStrList.clear();
	}

	strlist				m_StreamStrList;
	CRITICAL_SECTION	m_CS;

	void PushBack(string &str)
	{
		//get lock
		EnterCriticalSection(&m_CS);

		m_StreamStrList.push_back(str);

		//release lock
		LeaveCriticalSection(&m_CS);

	}

	void PopFront()
	{
		//get lock
		EnterCriticalSection(&m_CS);

		if (!m_StreamStrList.empty())
			m_StreamStrList.pop_front();

		//release lock
		LeaveCriticalSection(&m_CS);
	}

	string First()
	{
		string tmp = "";
		//get lock
		EnterCriticalSection(&m_CS);

		if (!m_StreamStrList.empty())
			tmp = m_StreamStrList.front();
		m_StreamStrList.pop_front();

		//release lock
		LeaveCriticalSection(&m_CS);
		return tmp;
	}
}StreamStrList;

class ngod_parse_thread : public ZQ::common::NativeThread
{
public:
	ngod_parse_thread(::ZQ::common::FileLog *logfile, 
					  ::ZQ::common::NativeThreadPool &pool,
					  ::ZQTianShan::NSS::NssEventList &eventList,
					  StreamStrList &streamStrList);
	ngod_parse_thread(ZQ::common::FileLog *logfile, 
					  ZQ::common::NativeThreadPool &pool,
					  ::ZQTianShan::NSS::NssEventList &eventList,
					  NSSSessionGroupList &_NSSSessionGroupList,
					  StreamStrList &streamStrList);
	~ngod_parse_thread();

	void setNSSSessionGroupList(NSSSessionGroupList &_NSSSessionGroupList);

	int	run(void);

private:
	void syncSessionList(NSSSessionGroup *pGroup, GetPramameterRes_ExtHeader &pHead);
	NSSSessionGroupList	*m_NSSSessionGroupList;
	::ZQ::common::NativeThreadPool *m_pPool;
	::ZQ::common::FileLog *m_pLogFile;
	::ZQTianShan::NSS::NssEventList &_eventList;
	StreamStrList &_streamStrList;
};