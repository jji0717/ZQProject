#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#define _WINSOCK2API_

#include "NGODr2c1.h"
#include <IceUtil/IceUtil.h>

class ssmNGODr2c1;

namespace NGODr2c1
{
	
class ContextImpl : public Context, public IceUtil::AbstractMutexI<IceUtil::Mutex>
{
public:

	typedef IceInternal::Handle<ContextImpl> Ptr;

	ContextImpl(ssmNGODr2c1& env);
	~ContextImpl();
	
	virtual ::NGODr2c1::ctxData getState(const ::Ice::Current& c = ::Ice::Current());
	virtual void renew(::Ice::Long ttl, const ::Ice::Current& = ::Ice::Current());
	virtual void increaseAnnSeq(const ::Ice::Current& = ::Ice::Current());
	virtual void onTimer(const ::Ice::Current& = ::Ice::Current());

protected:
	ssmNGODr2c1& _env;
};

typedef ContextImpl::Ptr ContextImplPtr;

}

#endif // __CONTEXT_H__