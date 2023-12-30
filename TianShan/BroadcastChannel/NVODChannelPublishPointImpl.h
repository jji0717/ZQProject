#ifndef __NVODChannelPublishPointImpl_h__
#define __NVODChannelPublishPointImpl_h__
#include "BroadCastChannelEnv.h"
#include <BcastChannelEx.h>
#include <Freeze/Freeze.h>
#include <IceUtil/IceUtil.h>

namespace ZQBroadCastChannel
{ 
class BroadCastChannelEnv;
class NVODChannelPublishPointImpl : public TianShanIce::Application::Broadcast::NVODChannelPublishPointEx,
	                                //public IceUtil::AbstractMutexWriteI<IceUtil::RWRecMutex>
	                                public ICEAbstractMutexWLock
{
public:
	typedef ::IceInternal::Handle< NVODChannelPublishPointImpl> Ptr;
	NVODChannelPublishPointImpl(BroadCastChannelEnv& bcastChenv);
	~NVODChannelPublishPointImpl();
public:
	virtual TianShanIce::Application::Broadcast::NVODSupplementalChannels getSupplementalChannels(const Ice::Current&)const;
protected:
	BroadCastChannelEnv& _env;
public:
	///implement publishpoint
	virtual ::std::string getType(const Ice::Current&)const;

	virtual ::std::string getName(const Ice::Current&)const;

	virtual ::std::string getDesc(const Ice::Current&)const;

	virtual ::Ice::Int getMaxBitrate(const Ice::Current&)const;

	virtual void setMaxBitrate(::Ice::Int,
		const Ice::Current&);

	virtual void setProperties(const TianShanIce::Properties&,
		const Ice::Current&);

	virtual void setDesc(const ::std::string&,
		const Ice::Current&);

	virtual TianShanIce::Properties getProperties(const Ice::Current&)const;

	virtual void destroy(const Ice::Current&);

	virtual void restrictReplica(const TianShanIce::StrValues&,
		const Ice::Current&);

	virtual TianShanIce::StrValues listReplica(const Ice::Current&)const;

	virtual ::Ice::Long requireResource(TianShanIce::SRM::ResourceType,
		const TianShanIce::SRM::Resource&,
		const Ice::Current&);

	///implement broadcast publishpoint
	virtual TianShanIce::SRM::ResourceMap getResourceRequirement(const Ice::Current&)const;

	virtual void withdrawResourceRequirement(TianShanIce::SRM::ResourceType,
		const Ice::Current&);

	virtual void setup(const Ice::Current&);

	virtual TianShanIce::SRM::SessionPrx getSession(const Ice::Current&);

	virtual void start(const Ice::Current&);

	virtual void stop(const Ice::Current&);

	virtual ::Ice::Long getUpTime(const Ice::Current&);

	virtual void renew(Ice::Long TTL, const Ice::Current &);

	virtual ::Ice::Long getExpiration(const Ice::Current &);
	///implement channelpublishpoint
	virtual TianShanIce::StrValues getItemSequence(const Ice::Current&)const;

	virtual TianShanIce::Application::ChannelItem findItem(const ::std::string&,
		const Ice::Current&)const;

	virtual void appendItem(const TianShanIce::Application::ChannelItem&,
		const Ice::Current&);

	virtual void appendItemAs(const TianShanIce::Application::ChannelItem&,
		const ::std::string&,
		const Ice::Current&);

	virtual void insertItem(const ::std::string&,
		const TianShanIce::Application::ChannelItem&,
		const Ice::Current&);

	virtual void insertItemAs(const ::std::string&,
		const TianShanIce::Application::ChannelItem&,
		const ::std::string&,
		const Ice::Current&);

	virtual void replaceItem(const ::std::string&,
		const TianShanIce::Application::ChannelItem&,
		const Ice::Current&);

	virtual void removeItem(const ::std::string&,
		const Ice::Current&);

	///implement NVODChannelPublisPointEx
	virtual ::Ice::Int getInterval(const Ice::Current&);
	virtual ::Ice::Short getIteration(const Ice::Current&);
	virtual bool isInService(const ::Ice::Current&);
	virtual bool activate(const ::Ice::Current& = ::Ice::Current());
};

}
#endif ///end define  __NVODChannelPublishPointImpl_h__
