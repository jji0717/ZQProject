#ifndef __DummyApp_H__
#define __DummyApp_H__

#include "../../common/TianShanDefines.h"
#include "../../Ice/TsApplication.h"

#define OBJ_NAME_DummyPurchase		"DummyPurchase"
#define SERVICE_NAME_DummyApp		"DummyApp"

class WeiwooAdminConsole;

// -----------------------------
// class DummyPurchase
// -----------------------------
//class DummyPurchase : public ::TianShanIce::Application::Purchase, public IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class DummyPurchase : public ::TianShanIce::Application::Purchase, public ICEAbstractMutexRLock
{
	friend class DummyApp;

public:

    DummyPurchase(WeiwooAdminConsole& env);
	typedef ::IceInternal::Handle< DummyPurchase > Ptr;

public:	// impls of Purchase

    virtual ::TianShanIce::Weiwoo::SessionPrx getSession(const ::Ice::Current& c);
    virtual void provision(const ::Ice::Current& c);
    virtual void render(const ::TianShanIce::Streamer::StreamPrx& stream, const ::TianShanIce::Weiwoo::SessionPrx& sess, const ::Ice::Current& c);
    virtual void detach(const ::TianShanIce::Weiwoo::SessionPrx& sess, const ::Ice::Current& c) {WLock sync(*this);} // do nothing here for DummyPurchase
    virtual void bookmark(const ::std::string& title, const ::TianShanIce::Weiwoo::SessionPrx&, const ::Ice::Current& c) {WLock sync(*this);}


protected:

	WeiwooAdminConsole& _env;
};

// -----------------------------
// service DummyApp
// -----------------------------
class DummyApp : public ::TianShanIce::Application::AppService
{
public:
    DummyApp(WeiwooAdminConsole& env);
	virtual ~DummyApp();

	typedef ::IceInternal::Handle< DummyApp > Ptr;

public:	// impls of BaseService

	virtual ::std::string getAdminUri(const ::Ice::Current& c);
    virtual ::TianShanIce::State getState(const ::Ice::Current& c);

public:	// impls of AppService

    virtual ::TianShanIce::Application::PurchasePrx createPurchase(const ::TianShanIce::Weiwoo::SessionPrx&, const ::TianShanIce::Properties& serverProps, const ::Ice::Current& c);

protected:
	
	WeiwooAdminConsole& _env;
	::TianShanIce::Application::PurchasePrx _thePurchase;
};

#endif // __DummyApp_H__
