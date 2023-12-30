#ifndef __TianShanS1_SessionContextImpl_H__
#define __TianShanS1_SessionContextImpl_H__

#include "SessionContext_ice.h"
#include <IceUtil/IceUtil.h>

namespace TianShanS1 {

	class Environment;

	class SessionContextImpl : public SessionContext, public IceUtil::AbstractMutexI<IceUtil::Mutex>
	{
	public:
		typedef IceInternal::Handle<SessionContextImpl> Ptr;

		SessionContextImpl(Environment& env);

		virtual ~SessionContextImpl();

		virtual ::TianShanS1::SessionData getSessionData(const ::Ice::Current& = ::Ice::Current());

		virtual ::Ice::Int addAnnounceSeq(const ::Ice::Current& = ::Ice::Current());

		virtual void onTimer(const ::Ice::Current& = ::Ice::Current());

		virtual void setRangePrefix(const ::std::string&, const ::Ice::Current& = ::Ice::Current());

		virtual bool canSendScaleChange(const ::std::string&, const ::Ice::Current& = ::Ice::Current());

		virtual ::std::string getProperty(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const ;

		virtual void updateProperty(const ::std::string&, const ::std::string&, const ::Ice::Current& = ::Ice::Current()) ;


	protected:

		Environment& _env;

	};

	typedef SessionContextImpl::Ptr SessionContextImplPtr;

} // namespace TianShanS1

#endif // #define __TianShanS1_SessionContextImpl_H__

