#pragma once
#include <TsApplication.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

class PurchaseImpl  : public TianShanIce::Application::Purchase
{
public:
	PurchaseImpl(void);
	virtual ~PurchaseImpl();

public: // functions defined in TianShanIce::Application::Purchase in file TsApplication.ICE.
	virtual ::TianShanIce::SRM::SessionPrx getSession(const ::Ice::Current& = ::Ice::Current()) const;
	virtual void provision(const ::Ice::Current& = ::Ice::Current());
	virtual void render(const ::TianShanIce::Streamer::StreamPrx&, const ::TianShanIce::SRM::SessionPrx&, const ::Ice::Current& = ::Ice::Current());	
	virtual void detach(const ::std::string&, const ::TianShanIce::Properties&, const ::Ice::Current& = ::Ice::Current());
	virtual void bookmark(const ::std::string&, const ::TianShanIce::SRM::SessionPrx&, const ::Ice::Current& = ::Ice::Current());
	virtual ::Ice::Int getParameters(const ::TianShanIce::StrValues&, const ::TianShanIce::ValueMap&, ::TianShanIce::ValueMap&, const ::Ice::Current& = ::Ice::Current()) const;

};
typedef ::IceInternal::Handle< PurchaseImpl> PurchaseImplPtr;