#pragma once

#include <DataAppEx.h>
#include <Freeze/Freeze.h>
#include "DataAppImpl.h"
#include "Util.h"
#include "DataPublishPointImpl.h"

namespace TianShanIce {
namespace Application {
namespace DataOnDemand {

class MessageQueueExImpl : 
	public DataPublishPointImpl<MessageQueueEx>, 
	public IceUtil::AbstractMutexI<IceUtil::RecMutex>
{
public:
	MessageQueueExImpl();
	virtual ~MessageQueueExImpl();

	virtual void onMessageAdded(::Ice::Int,
		const ::std::string&,
		const ::TianShanIce::Application::DataOnDemand::Message&,
		const Ice::Current&);

	virtual void onMessageDeleted(::Ice::Int,
		const ::std::string&,
		const Ice::Current&);

public:
	virtual bool init();
	
};
	
} // END DataOnDemand
} // END Application
} // END TianshanICE
