// ActiveMsgChannel.h: interface for the ActiveMsgChannel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ACTIVEMSGCHANNEL_H__B7DC4812_9F8B_4236_A132_C75034A96172__INCLUDED_)
#define AFX_ACTIVEMSGCHANNEL_H__B7DC4812_9F8B_4236_A132_C75034A96172__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DODAppEx.h"
#include "ActiveChannel.h"
#include "MessageChannelImpl.h"
#include "DODAppThread.h"
#include <list>
#include <string>
typedef struct Message_Info
{
	// The message body was save into the file.
	::std::string fileName;

	//It denote a order number in the queue.
	//bForever=TRUE,means= forever;else has duration
	BOOL bForever;
	
   //each message deleteTime;
   	long  deleteTime;	

	// messageid and nDataType identify message
	::std::string sMessageID;
	
	int GroupId;
}ZQCMessageInfoTINF, *PZQCMessageInfoTINF;

typedef std::list<ZQCMessageInfoTINF> zqMessageList;

class ActiveMsgChannelThread: public DODAppThread {
protected:

	virtual bool init()
	{
		return threadInit();
	}

	virtual bool threadInit() = 0;
};

class ActiveMsgChannelBase: public ActiveChannel {
public:
	virtual bool initchannel()
	{
		return activeChannelInit();
	}

	virtual bool activeChannelInit() = 0;
};

class ActiveMsgChannel: public ActiveMsgChannelBase, 
	public ActiveMsgChannelThread {

public:

	ActiveMsgChannel(DataOnDemand::MessageChannelExPrx& channelPrx);
	virtual ~ActiveMsgChannel();

	virtual void stop();
	
	void notifyMessageDeleted(const ::std::string& messageId,
		::Ice::Int groupId);

	bool notifyMessageAdded(
		const ::std::string& messageId,
		const ::std::string& dest,
		const ::std::string& messageBody,
		::Ice::Long exprie,
		::Ice::Int groupId);

protected:
	
	// member of class ActiveChannel
	virtual bool activeChannelInit();
	virtual void uninit();

	// member of class thread
	virtual bool threadInit();
	virtual int run();
	virtual void final();

	bool LoadMsgData();
	int ListFile(const char *argv, std::list<std::string> &File);

	//Add Message to List
	void  AddMsgList(ZQCMessageInfoTINF *pinfo, bool IsSet = true);
	
	bool NotityMsgMuxItemAdd(int groupId,std::string fileName,
											std::string _contentpath);
	bool NotityMsgMuxItemDel(int groupId,std::string fileName);
    bool WrapData(std::string content,std::string filename,
											std::string& _contentpath);
	
protected:
	CNotifyUpdateMsgChannel*			m_pNmp;
	DataOnDemand::MessageChannelExPrx	_channelPrx;

	//messagelist.The list was not used, because the list can not defined sort.
	zqMessageList		m_MessageList;

    HANDLE				m_hWaitEvent;

	// process mutex flag for message_vector
	CRITICAL_SECTION	m_channelCriticalSection;	
	
	long				m_UpdateTime;

	bool				m_bStop;
};

#endif // !defined(AFX_ACTIVEMSGCHANNEL_H__B7DC4812_9F8B_4236_A132_C75034A96172__INCLUDED_)
