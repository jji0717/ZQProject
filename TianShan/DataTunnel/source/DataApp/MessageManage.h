#if !defined(AFX_MESSAGEMANAGE_H__95E43CAF_5C0C_407F_9C54_A3DBE6E2220C__INCLUDED_)
#define AFX_MESSAGEMANAGE_H__95E43CAF_5C0C_407F_9C54_A3DBE6E2220C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "MessageListDict.h"
class MessageManage
{
public:
	MessageManage(Ice::CommunicatorPtr&	communicator);
	virtual ~MessageManage();
	
public:
	int  addMessage(const std::string& datappname, const TianShanIce::Application::DataOnDemand::messageinfo& message);
	bool deleteMessage(const std::string& datappname, const std::string& messageId);
	bool modityMessage(const std::string& datappname, const std::string& messageId);
    bool getMessage(const std::string& datappname, TianShanIce::Application::DataOnDemand::messageinfo& message);	
	long getMessageCount(const std::string& datappname);
	bool removeDataPublishPointMessage(const std::string& datappname);
	bool init();
protected:
	
protected:
	Freeze::ConnectionPtr		_connCh;
	Ice::CommunicatorPtr&		_communicator;
	TianShanIce::Application::DataOnDemand::MessageListDict* _pmessageDict;
	IceUtil::AbstractMutexI<IceUtil::RecMutex>	_dictLock;
public:
	enum InsertPos
	{
       first = 0,
	   middel,
	   last,
	   unknown
	};
};
		

#endif // !defined(AFX_MESSAGEMANAGE_H__95E43CAF_5C0C_407F_9C54_A3DBE6E2220C__INCLUDED_)