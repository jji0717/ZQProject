// ChannelPublishPointImpl.h: interface for the ChannelPublishPointImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHANNELPUBLISHPOINTIMPL_H__6ED6596D_044F_4EFE_9168_3B41ACBB9134__INCLUDED_)
#define AFX_CHANNELPUBLISHPOINTIMPL_H__6ED6596D_044F_4EFE_9168_3B41ACBB9134__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <DataAppEx.h>
#include <Freeze/Freeze.h>
#include "DataAppImpl.h"
#include "Util.h"
#include <list>

class ActiveData;
namespace TianShanIce {
namespace Application {
namespace DataOnDemand {

template <class Base>
class DataPublishPointImpl : public Base	{
public:
	DataPublishPointImpl();
	virtual ~DataPublishPointImpl();

	virtual ::std::string getType(const Ice::Current&) const;

	virtual ::std::string getName(const Ice::Current&) const;

	virtual ::std::string getDesc(const Ice::Current&) const;
	virtual void setDesc(const ::std::string&,const Ice::Current&);

	virtual ::Ice::Int getMaxBitrate(const Ice::Current&) const;
	virtual void setMaxBitrate(::Ice::Int,const Ice::Current&);

	virtual void setProperties(const ::TianShanIce::Properties&,const Ice::Current&);
	virtual ::TianShanIce::Properties getProperties(const Ice::Current&) const;

	virtual void destroy(const Ice::Current&);

	virtual void restrictReplica(const ::TianShanIce::StrValues&,
		const Ice::Current&);

	virtual ::TianShanIce::StrValues listReplica(const Ice::Current&) const;

    virtual  DataPublishPointInfo getInfo(const Ice::Current&);

	virtual void getCacheInfo(TianShanIce::Streamer::DataOnDemand::CacheType& type, std::string& addres, 
		const Ice::Current&) ;
	
	DataStreamLinks getDataStreamLinks(const Ice::Current&);

	virtual void linkDataStream(const ::std::string&,
			  const DataStreamExPrx&,
			  ::Ice::Long,
			  const Ice::Current&);

    virtual void unlinkDataStream(const ::std::string&,
			    const Ice::Current&);
	virtual void activate(const ::Ice::Current& );

	/*TianShanIce::SRM::ResourceMap 
	getResourceRequirement(const Ice::Current &) const;*/
	
public:
	bool init();
	
protected:
	DataPublishPointPrx getThisPrx(Ice::ObjectAdapterPtr adapter);
	ActiveData* getActiveData();

protected:
	DataPublishPointPrx	_thisPrx;
	ZQ::common::Mutex _mutex;
};

} /// end namespace DataOnDemand {
} /// end namespace Application 
} /// end namespace TianshanIce 

#include "DataPublishPointImpl.cpp"

#endif // !defined(AFX_CHANNELPUBLISHPOINTIMPL_H__6ED6596D_044F_4EFE_9168_3B41ACBB9134__INCLUDED_)
