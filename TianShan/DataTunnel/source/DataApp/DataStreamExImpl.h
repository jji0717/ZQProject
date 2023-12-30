#pragma once

#include <DataAppEx.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include "DataAppImpl.h"
#include ".\DataStream.h"
#include "Locks.h"
namespace TianShanIce {
namespace Application {
namespace DataOnDemand {

class DataStreamExImpl : public DataStreamEx , 
	public IceUtil::AbstractMutexI<IceUtil::RecMutex> {
public:
	DataStreamExImpl();
    ~DataStreamExImpl();

	typedef ::IceInternal::Handle<DataStreamExImpl>DestinationImplPtr;

	/// impl interface ::TianShanIce::Application::PublishPoint
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

	virtual ::TianShanIce::StrValues listReplica(const Ice::Current&)const;
 
	///impl interface ::TianShanIce::Application::BroadcastPublishPoint
	virtual ::Ice::Long requireResource(::TianShanIce::SRM::ResourceType,
		const ::TianShanIce::SRM::Resource&,
		const Ice::Current&);

	virtual ::TianShanIce::SRM::ResourceMap getResourceRequirement(const Ice::Current&) const;

	virtual void withdrawResourceRequirement(::TianShanIce::SRM::ResourceType,
		const Ice::Current&);

	virtual void setup(const Ice::Current&);

	virtual ::TianShanIce::SRM::SessionPrx getSession(const Ice::Current&);

	virtual void start(const Ice::Current&);

	virtual void stop(const Ice::Current&);

	virtual ::Ice::Long getUpTime(const Ice::Current&);

	///impl interface ::TianShanIce::Application::DataStream

	virtual ::TianShanIce::State getState(const Ice::Current&);

	virtual ::TianShanIce::Application::DataOnDemand::DataAttachInfos listAttachments(const ::std::string&,
		const Ice::Current&);

	virtual void attachDataPublishPoint(
		const ::std::string&, 
		const ::TianShanIce::Application::DataOnDemand::DataAttachInfo& , 
		const Ice::Current&);

	virtual void detachDataPublishPoint(const ::std::string&,
		const Ice::Current&);

	virtual void pause(const Ice::Current&);

	///impl interface ::TianShanIce::Application::DataStreamEx
	virtual void removeDataPublishPoint(const ::std::string&,
		const Ice::Current&);

	virtual void activate(const Ice::Current&);
	 ::TianShanIce::Application::DataOnDemand::DataStreamInfo getInfo(const Ice::Current&);

public:
	bool init();
	TianShanIce::Streamer::DataOnDemand::DataStreamPrx getStreamPrx();
protected:
	inline TianShanIce::Application::DataOnDemand::DataStreamExPrx getThisProxy(
		const Ice::ObjectAdapterPtr& adapter);

	void rebuildMuxItem(TianShanIce::Streamer::DataOnDemand::DataStreamPrx& strm);
	void notifyMuxItem();

protected:
	TianShanIce::Application::DataOnDemand::DataStreamExPrx	_thisPrx;
};

} // END DataOnDemand
} // END Application
} // END TianshanICE
