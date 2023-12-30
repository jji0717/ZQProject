// ChannelPublishPointImpl.cpp: implementation of the ChannelPublishPointImpl class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "global.h"

#include "DataPublishPointImpl.h"

#ifndef _CHANNEL_PUBLISHPOINT_IMPL_CPP_
#define _CHANNEL_PUBLISHPOINT_IMPL_CPP_

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace TianShanIce {
namespace Application {
namespace DataOnDemand {
template <class Base>
DataPublishPointImpl<Base>::DataPublishPointImpl()
{

}

template <class Base>
DataPublishPointImpl<Base>::~DataPublishPointImpl()
{

}

template <class Base>
::std::string
DataPublishPointImpl<Base>::getDesc(const Ice::Current& current) const
{
	return  desc;
}

template <class Base>
::Ice::Int
DataPublishPointImpl<Base>::getMaxBitrate(const Ice::Current& current) const
{
	return maxBitrate;
}

template <class Base>
void
DataPublishPointImpl<Base>::setMaxBitrate(::Ice::Int maxBit,
													   const Ice::Current& current)
{
	ZQ::common::MutexGuard guard(_mutex);
	maxBitrate  = maxBit;
}
template <class Base>
void
DataPublishPointImpl<Base>::setDesc(const ::std::string& description,
												 const Ice::Current& current)
{
	ZQ::common::MutexGuard guard(_mutex);
	desc = description;
}
template <class Base>
void
DataPublishPointImpl<Base>::restrictReplica(const ::TianShanIce::StrValues& contentStoreNetIds,
														 const Ice::Current& current)
{
	ZQ::common::MutexGuard guard(_mutex);
	replicas = contentStoreNetIds;
}
template <class Base>
::TianShanIce::StrValues
DataPublishPointImpl<Base>::listReplica(const Ice::Current& current) const
{
	return replicas;
}

template <class Base>
::std::string
DataPublishPointImpl<Base>::getName(
	const Ice::Current& current) const
{
    return myInfo.name;
}

template <class Base>
::std::string 
DataPublishPointImpl<Base>::getType(
	const Ice::Current& current) const
{
    return type;
}

template <class Base>
DataPublishPointInfo
DataPublishPointImpl<Base>::getInfo(
	const Ice::Current& current)
{
    return myInfo;
}

template <class Base>
::TianShanIce::Properties
DataPublishPointImpl<Base>::getProperties(
	const Ice::Current& current) const
{
    return properties;
}

template <class Base>
void
DataPublishPointImpl<Base>::setProperties(
						  const ::TianShanIce::Properties& props,
						  const Ice::Current& current)
{
	ZQ::common::MutexGuard guard(_mutex);
	properties = props;
}

template <class Base>
void
DataPublishPointImpl<Base>::destroy(const Ice::Current& current)
{
	ZQ::common::MutexGuard guard(_mutex);
	DataStreamLinks::iterator iter;
	DataStreamExPrx datastreamprx;
    DataPPToDataStreamAssoc datastreaminfo;
	std::string datastreamname;
	try
	{
		while(!myDataStreamLinks.empty())
		{
			iter = myDataStreamLinks.begin();
			datastreamname = iter->first;
			datastreaminfo = iter->second;
			datastreamprx = datastreaminfo.dest;
			try
			{	
				datastreamprx->detachDataPublishPoint(myInfo.name);	
			}
			catch (::TianShanIce::InvalidParameter& ex)
			{
				glog(ZQ::common::Log::L_ERROR,CLOGFMT(DataPublishPointImpl,  "[%s] failed to destroy caught excepiton(%s)"),myInfo.name.c_str(), ex.ice_name().c_str());
			}	
		}
		myParent->removeDataPublishPoint(myInfo.name);
		ActiveDataManager.remove(myInfo.name);
	}
	catch (const Ice::ObjectNotExistException &ex)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(DataPublishPointImpl,  "[%s] failed to destroy caught excepiton(%s)"),myInfo.name.c_str(), ex.ice_name().c_str());
		throw Ice::Exception();
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(DataPublishPointImpl,  "[%s] failed to destroy caught excepiton(%d)"),myInfo.name.c_str(), GetLastError());

		throw Ice::Exception();
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(DataPublishPointImpl,  "[%s] destroy success"),myInfo.name.c_str());
	// delete this;
}

template <class Base>
DataStreamLinks
DataPublishPointImpl<Base>::getDataStreamLinks(const Ice::Current&)
{
	return myDataStreamLinks;
}

template <class Base>
void
DataPublishPointImpl<Base>::linkDataStream(const ::std::string& name,
					 const DataStreamExPrx& dest,
					 ::Ice::Long lastUpdate,
					 const Ice::Current& current)
{
	ZQ::common::MutexGuard guard(_mutex);

    DataPPToDataStreamAssoc datapptodestassoc;
	datapptodestassoc.destName = name;
	datapptodestassoc.dest = dest;
	datapptodestassoc.lastUpdate = lastUpdate;
	std::pair<DataStreamLinks::iterator, bool> ir;
	ir = myDataStreamLinks.insert(DataStreamLinks::value_type(
		name, datapptodestassoc));
	if (!ir.second) {
		throw ObjectExistException();
	}
}

template <class Base>
void
DataPublishPointImpl<Base>::unlinkDataStream(const ::std::string& name,
					const Ice::Current& current)
{
	ZQ::common::MutexGuard guard(_mutex);
	myDataStreamLinks.erase(name);
}


template <class Base>
void 
DataPublishPointImpl<Base>::activate(
	const ::Ice::Current& current)
{
	ZQ::common::MutexGuard guard(_mutex);
	ActiveDataManager.create(getThisPrx(current.adapter));
}

template <class Base>
void 
DataPublishPointImpl<Base>::getCacheInfo(TianShanIce::Streamer::DataOnDemand::CacheType& type, 
										std::string& addres, const Ice::Current& current)
{
	
}

template <class Base>
DataPublishPointPrx DataPublishPointImpl<Base>::getThisPrx(Ice::ObjectAdapterPtr adapter)
{
	if (_thisPrx == NULL) {
		_thisPrx = DataPublishPointPrx::uncheckedCast(adapter->createProxy(createDataPublishPointIdentity(myInfo.name)));
	}

	return _thisPrx;
}

template <class Base>
ActiveData* 
DataPublishPointImpl<Base>::getActiveData()
{
	ZQ::common::MutexGuard guard(_mutex);
	ActiveData* ActiveData = ActiveDataManager.get(myInfo.name);
	assert(ActiveData);

	return ActiveData;
}

template <class Base>
bool 
DataPublishPointImpl<Base>::init()
{
	return true;
}
/*template <class Base>
TianShanIce::SRM::ResourceMap 
DataPublishPointImpl<Base>::getResourceRequirement(const Ice::Current &) const
{
	return resources;
}*/
} /// end namespace DataOnDemand {
} /// end namespace Application 
} /// end namespace TianshanIce 
#endif // #ifdef _CHANNEL_PUBLISHPOINT_IMPL_CPP_
