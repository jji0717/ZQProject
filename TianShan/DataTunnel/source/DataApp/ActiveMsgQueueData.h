// ActiveMsgQueueData.h: interface for the ActiveMsgQueueData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ActiveMsgQueueData_H__B7DC4812_9F8B_4236_A132_C75034A96172__INCLUDED_)
#define AFX_ActiveMsgQueueData_H__B7DC4812_9F8B_4236_A132_C75034A96172__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DataAppEx.h"
#include "ActiveData.h"
#include "MessageQueueExImpl.h"
#include "DataAppThread.h"
#include "messagemanage.h"
#include <list>
#include <string>
class ActiveMsgQueueDataThread: public DataTunnelAppThread {
protected:

	virtual bool init()
	{
		return threadInit();
	}

	virtual bool threadInit() = 0;
};

class ActiveMsgQueueDataBase: public ActiveData {
public:
	virtual bool initchannel()
	{
		return ActiveDataInit();
	}

	virtual bool ActiveDataInit() = 0;
};

class ActiveMsgQueueData: public ActiveMsgQueueDataBase, 
	public ActiveMsgQueueDataThread {

public:

	ActiveMsgQueueData(TianShanIce::Application::DataOnDemand::MessageQueueExPrx& channelPrx);
	virtual ~ActiveMsgQueueData();

	virtual void stop();
	
	void onMessageDeleted(::Ice::Int groupId , const ::std::string& messageId);

	bool onMessageAdded(::Ice::Int groupId,
		const ::std::string& messageId,
		const TianShanIce::Application::DataOnDemand::Message& message);

protected:
	
	// member of class ActiveData
	virtual bool ActiveDataInit();
	virtual void uninit();

	// member of class thread
	virtual bool threadInit();
	virtual int run();
	virtual void final();
	bool NotityMsgMuxItemAdd(int groupId,std::string _contentpath);
	bool NotityMsgMuxItemDel(int groupId,std::string fileName);
    bool WrapData(std::string content,std::string filename,
											std::string& _contentpath);
	
protected:
	TianShanIce::Application::DataOnDemand::MessageQueueExPrx	_dataPPPrx;
    HANDLE				m_hWaitEvent;		
	long				m_UpdateTime;
	bool				m_bStop;
public:
	MessageManage* m_pmessagemanage;
};

#endif // !defined(AFX_ActiveMsgQueueData_H__B7DC4812_9F8B_4236_A132_C75034A96172__INCLUDED_)
