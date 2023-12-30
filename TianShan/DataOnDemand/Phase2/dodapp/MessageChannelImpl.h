#pragma once

#include <DODAppEx.h>
#include <Freeze/Freeze.h>
#include "DODAppImpl.h"
#include "Util.h"
#include "ChannelPublishPointImpl.h"

class CNotifyUpdateMsgChannel;

namespace DataOnDemand {

class MessageChannelImpl : 
	public ChannelPublishPointImpl<MessageChannelEx>, 
	public IceUtil::AbstractMutexI<IceUtil::RecMutex>
{
public:
	MessageChannelImpl();
	virtual ~MessageChannelImpl();

    virtual void notifyMessageAdded(const ::std::string&,
				    const ::std::string&,
				    const ::std::string&,
				    ::Ice::Long,
					::Ice::Int, 
				    const Ice::Current&);

    virtual void notifyMessageDeleted(const ::std::string&, 
		::Ice::Int, const Ice::Current&);

public:
	virtual bool init();
	
};
	
}
