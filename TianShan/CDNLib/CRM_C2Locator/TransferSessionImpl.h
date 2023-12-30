#ifndef __ZQTianShan_TransferSessionImpl_H__
#define __ZQTianShan_TransferSessionImpl_H__

#include "C2Env.h"
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include "C2Locator.h"

namespace TianShanIce{
namespace SCS{

class TransferSessionImpl : public TransferSession,//public ::IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
                                                   public ICEAbstractMutexRLock
{
public:
    typedef ::IceInternal::Handle< TransferSessionImpl > Ptr;
	TransferSessionImpl(::ZQTianShan::CDN::C2Env &env);
	~TransferSessionImpl();
    static void initProperties(TransferSessionProp& p);
	//impl of TransferSession
	virtual ::TianShanIce::Streamer::StreamPrx getStream(const ::Ice::Current& = Ice::Current());
    virtual void setStream(const ::TianShanIce::Streamer::StreamPrx& s, const ::Ice::Current& = Ice::Current());

    virtual TransferSessionProp getProps(const ::Ice::Current& = Ice::Current());
    virtual void setProps(const ::TianShanIce::ValueMap &value, const ::Ice::Current& c);
    virtual void setProps2(const TransferSessionProp& p, const ::Ice::Current& = Ice::Current());
	virtual void setProps3(const TianShanIce::Properties& p, const ::Ice::Current& = Ice::Current());
	


	//impl of PathTicket
    virtual ::Ice::Identity getIdent(const ::Ice::Current& = Ice::Current()) const;

	virtual ::TianShanIce::State getState(const ::Ice::Current& = Ice::Current()) const;

    virtual void setState(::TianShanIce::State st, const ::Ice::Current& = ::Ice::Current());

    virtual ::TianShanIce::SRM::ResourceMap getResources(const ::Ice::Current&) const ;

    virtual void setResources(const TianShanIce::SRM::ResourceMap& rs, const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::ValueMap getPrivateData(const ::Ice::Current&) const;

	virtual void renew(::Ice::Int, const ::Ice::Current&);

	virtual ::TianShanIce::Transport::StorageLinkPrx getStorageLink(const ::Ice::Current&) const;

	virtual ::TianShanIce::Transport::StreamLinkPrx getStreamLink(const ::Ice::Current&) const;

	virtual ::Ice::Int getCost(const ::Ice::Current&) const;

	virtual ::Ice::Int getLeaseLeft(const ::Ice::Current&) const;
    virtual void setExpiration(Ice::Long timeoutMsec, const ::Ice::Current& = ::Ice::Current());

	virtual void narrow_async(const ::TianShanIce::Transport::AMD_PathTicket_narrowPtr&, const ::TianShanIce::SRM::SessionPrx&, const ::Ice::Current&);

	virtual void commit_async(const ::TianShanIce::Transport::AMD_PathTicket_commitPtr&, const ::TianShanIce::SRM::SessionPrx&, const ::Ice::Current&);

	virtual void destroy(const ::Ice::Current&);

	virtual void setPenalty(::Ice::Byte, const ::Ice::Current&);

	//impl of TimeoutObj
	virtual void OnTimer(const ::Ice::Current& c);
protected:
private:
	::ZQTianShan::CDN::C2Env& _env;
};

#define envExtLog (*(_env._pLog))
#define C2IdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::SCS::_CLASS##Prx::uncheckedCast((_ENV)._adapter->createProxy(_ID))

}//namespace SCS
}// namespace TianShanIce
#endif //__ZQTianShan_TransferSessionImpl_H__

