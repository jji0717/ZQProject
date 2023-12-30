
#include <DataStreamImpl.h>

::std::string
DataOnDemand::MuxItemImpl::getName(const Ice::Current& current)
{
    return ::std::string();
}

void
DataOnDemand::MuxItemImpl::notifyFullUpdate(const Ice::Current& current)
{
}

void
DataOnDemand::MuxItemImpl::notifyFileAdded(const ::std::string& fileName,
					const Ice::Current& current)
{
}

void
DataOnDemand::MuxItemImpl::notifyFileDeleted(const ::std::string& fileName,
					  const Ice::Current& current)
{
}

::DataOnDemand::MuxItemImplnfo
DataOnDemand::MuxItemImpl::getInfo(const Ice::Current& current)
{
    return ::DataOnDemand::MuxItemImplnfo();
}

void
DataOnDemand::MuxItemImpl::destory(const Ice::Current& current)
{
}

::std::string
DataOnDemand::DataStreamImpl::getName(const Ice::Current& current)
{
    return ::std::string();
}

::DataOnDemand::StreamInfo
DataOnDemand::DataStreamImpl::getInfo(const Ice::Current& current) const
{
    return ::DataOnDemand::StreamInfo();
}

::DataOnDemand::MuxItemPrx
DataOnDemand::DataStreamImpl::createMuxItem(const ::std::string& name,
					 const ::DataOnDemand::MuxItemImplnfo& info,
					 const Ice::Current& current)
{
    return 0;
}

::DataOnDemand::MuxItemPrx
DataOnDemand::DataStreamImpl::getMuxItem(const ::std::string& name,
				      const Ice::Current& current)
{
    return 0;
}

::TianShanIce::StrValues
DataOnDemand::DataStreamImpl::listMuxItems(const Ice::Current& current)
{
    return ::TianShanIce::StrValues();
}

::DataOnDemand::DataStreamPrx
DataOnDemand::DataStreamServiceImpl::createStreamByApp(const ::TianShanIce::AccreditedPath::PathTicketPrx& pathTicket,
						    const ::std::string& name,
						    const ::DataOnDemand::StreamInfo& info,
						    const Ice::Current& current)
{
    return 0;
}
