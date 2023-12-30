#ifndef __TianShanS1_SessionView_H__
#define __TianShanS1_SessionView_H__

#include <IceUtil/IceUtil.h>
#include "./SessionContext_ice.h"
#include <vector>

namespace TianShanS1
{
	class Environment;
	class SessionViewImpl : public SessionView, public IceUtil::AbstractMutexI<IceUtil::Mutex>
	{
	public: 
		typedef IceInternal::Handle<SessionViewImpl> Ptr;

		SessionViewImpl(Environment& env);
		virtual ~SessionViewImpl();
		virtual ::Ice::Int getAllContext(::Ice::Int, ::Ice::Int&, const ::Ice::Current& = ::Ice::Current());
		virtual SessionDatas getRange(::Ice::Int, ::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current()) const;
		virtual void unregister(::Ice::Int, const ::Ice::Current& = ::Ice::Current());
		
	private: 
		Environment& _env;
		std::vector<SessionDatas*> _vSessDatas;
		
	}; // class SessionView

	typedef SessionViewImpl::Ptr SessionViewImplPtr;
}

#endif // __TianShanS1_SessionView_H__

