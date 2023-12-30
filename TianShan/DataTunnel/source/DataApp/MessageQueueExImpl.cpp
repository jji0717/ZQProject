#include "stdafx.h"
#include "MessageQueueExImpl.h"
#include "DataPointPublisherImpl.h"
#include "global.h"
#include "ActiveMsgQueueData.h"
#include "ActiveDataMgr.h"
namespace TianShanIce {
namespace Application {
namespace DataOnDemand {
MessageQueueExImpl::MessageQueueExImpl()
{
/*	glog(ZQ::common::Log::L_DEBUG,
		"[ChannelName: %s]MessageQueueExlImpl()",myInfo.name.c_str());*/
}

MessageQueueExImpl::~MessageQueueExImpl()
{
/*	glog(ZQ::common::Log::L_DEBUG,
		"[ChannelName: %s]~MessageQueueExlImpl()",myInfo.name.c_str());*/
}

bool 
MessageQueueExImpl::init()
{
	return true;
}

void
MessageQueueExImpl::onMessageAdded(
			 ::Ice::Int groupId,
			 const ::std::string& messageId,
			 const ::TianShanIce::Application::DataOnDemand::Message& msgInfo,
			 const Ice::Current& current)
{
	ActiveMsgQueueData* pAMsgChanel = (ActiveMsgQueueData*)getActiveData();
	if(pAMsgChanel == NULL)
		return;

    pAMsgChanel->onMessageAdded(groupId, messageId, msgInfo);	
}

void
MessageQueueExImpl::onMessageDeleted(
	                        ::Ice::Int groupId,
							const ::std::string& messageId,
							const Ice::Current& current)
{
	ActiveMsgQueueData* pAMsgChanel = (ActiveMsgQueueData*)getActiveData();
	if(pAMsgChanel == NULL)
		return;
	pAMsgChanel->onMessageDeleted(groupId, messageId);		
}
} // END DataOnDemand
} // END Application
} // END TianshanICE