#ifndef __HSNTree_SessionContextImpl_H__
#define __HSNTree_SessionContextImpl_H__

#include "SessionContext_ice.h"
#include <IceUtil/IceUtil.h>

namespace HSNTree {

	class Environment;

	class SessionContextImpl : public SessionContext, public IceUtil::AbstractMutexI<IceUtil::Mutex>
	{
	public:
		typedef IceInternal::Handle<SessionContextImpl> Ptr;

		SessionContextImpl(Environment& env);

		virtual ~SessionContextImpl();

		virtual ::HSNTree::SessionData getSessionData(const ::Ice::Current& = ::Ice::Current());

		virtual ::Ice::Int addAnnounceSeq(const ::Ice::Current& = ::Ice::Current());

		virtual void onTimer(const ::Ice::Current& = ::Ice::Current());

		virtual void setRangePrefix(const ::std::string&, const ::Ice::Current& = ::Ice::Current());

		virtual bool canSendScaleChange(const ::std::string&, const ::Ice::Current& = ::Ice::Current());

		virtual ::std::string getProperty(const ::std::string&, const ::Ice::Current& = ::Ice::Current()) const ;

		virtual void updateProperty(const ::std::string&, const ::std::string&, const ::Ice::Current& = ::Ice::Current()) ;

		virtual void sessionTeardown(const ::Ice::Current& = ::Ice::Current());

		// functions defined in TianShanIce::Application::Purchase in file TsApplication.ICE.
		virtual ::TianShanIce::SRM::SessionPrx getSession(const ::Ice::Current& = ::Ice::Current()) const;
		virtual void provision(const ::Ice::Current& = ::Ice::Current());
		virtual void render(const ::TianShanIce::Streamer::StreamPrx&, const ::TianShanIce::SRM::SessionPrx&, const ::Ice::Current& = ::Ice::Current());	
		virtual void detach(const ::std::string&, const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current());
		virtual void bookmark(const ::std::string&, const ::TianShanIce::SRM::SessionPrx&, const ::Ice::Current& = ::Ice::Current());
		virtual ::Ice::Int getParameters(const ::TianShanIce::StrValues&, const ::TianShanIce::ValueMap&, ::TianShanIce::ValueMap&, const ::Ice::Current& = ::Ice::Current()) const;


	protected:

		Environment& _env;

	};

	typedef SessionContextImpl::Ptr SessionContextImplPtr;

} // namespace HSNTree

#endif // #define __HSNTree_SessionContextImpl_H__

