#include "stdafx.h"
#include "DODAppImpl.h"
#include "MessageChannelImpl.h"
#include "datapublisherimpl.h"
#include "global.h"
#include "ActiveMsgChannel.h"
#include "ActiveChannelMgr.h"

DataOnDemand::MessageChannelImpl::MessageChannelImpl()
{
	glog(ZQ::common::Log::L_DEBUG,
		"[ChannelName = %s]MessageChannelImpl()",myInfo.name.c_str());
}

DataOnDemand::MessageChannelImpl::~MessageChannelImpl()
{
/*	glog(ZQ::common::Log::L_DEBUG,
		"[ChannelName = %s]~MessageChannelImpl()",myInfo.name.c_str());*/
}

bool 
DataOnDemand::MessageChannelImpl::init()
{
	return true;
}

void
DataOnDemand::MessageChannelImpl::notifyMessageAdded(
	const ::std::string& messageId,
	const ::std::string& dest,
	const ::std::string& messageBody,
	::Ice::Long exprie,
	::Ice::Int groupId, 
	const Ice::Current& current)
{
	ActiveMsgChannel* pAMsgChanel = (ActiveMsgChannel*)getActiveChannel();
	if(pAMsgChanel == NULL)
		return;

    pAMsgChanel->notifyMessageAdded(messageId,dest,messageBody,exprie,groupId);	
}

void
DataOnDemand::MessageChannelImpl::notifyMessageDeleted(
	const ::std::string& messageId,
	::Ice::Int groupId, 
	const Ice::Current& current)
{
	ActiveMsgChannel* pAMsgChanel = (ActiveMsgChannel*)getActiveChannel();
	if(pAMsgChanel == NULL)
		return;
	pAMsgChanel->notifyMessageDeleted(messageId,groupId);		
}
