#include "DummyApp.h"
#include "admin.h"
#include "urlstr.h"

// -----------------------------
// service DummyApp
// -----------------------------
DummyApp::DummyApp(WeiwooAdminConsole& env)
: _env (env)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(DummyApp, "add the interface \"%s\" on to the adapter"), SERVICE_NAME_DummyApp);
    _env._adapter->add(this, _env._communicator->stringToIdentity(SERVICE_NAME_DummyApp));
	Ice::PropertiesPtr proper=_env._communicator->getProperties();
	proper->setProperty("DummyApp.ThreadPool.Size","5");
	proper->setProperty("Ice.ThreadPool.Server.Size","5");
	DummyPurchase::Ptr purchase= new DummyPurchase(_env);
    _thePurchase = ::TianShanIce::Application::PurchasePrx::checkedCast(_env._adapter->add(purchase, _env._communicator->stringToIdentity(OBJ_NAME_DummyPurchase)));
}

DummyApp::~DummyApp()
{
	_thePurchase = NULL;
//	_env._adapter->remove(_env._communicator->stringToIdentity(OBJ_NAME_DummyPurchase));
}

::std::string DummyApp::getAdminUri(const ::Ice::Current& c)
{
	throw TianShanIce::NotImplemented(__MSGLOC__ "getAdminUri() not supported in DummyApp");
}

::TianShanIce::State DummyApp::getState(const ::Ice::Current& c)
{
	throw TianShanIce::NotImplemented(__MSGLOC__ "getState() not supported in DummyApp");
}

::TianShanIce::Application::PurchasePrx DummyApp::createPurchase(const ::TianShanIce::Weiwoo::SessionPrx& sess, const ::TianShanIce::Properties& serverProps, const ::Ice::Current& c)
{
	printf("\nDummyApp::createPurchase(sess[%s])\n", sess->getId().c_str());
	// start with a new line on console

	// set a bw for test
	::TianShanIce::ValueMap bwdata;
	::TianShanIce::Variant  val;
	val.bRange = false;
	val.type = TianShanIce::vtLongs;
	val.lints.clear();
	val.lints.push_back(4000000);
	bwdata["bandwidth"] = val;
	val.lints.clear();
	//sess->addResource(::TianShanIce::Weiwoo::rtTsDownstreamBandwidth, TianShanIce::Weiwoo::raMandatoryNonNegotiable, bwdata);
	
/*
	char buf[256];
	char prompt[256], *p=prompt;
	sprintf(prompt, "DummyApp:sess[%s]> ", sess->getId().c_str());
	p = prompt + strlen(prompt);

#pragma message ( __MSGLOC__ "TODO: impl here")

	strcpy(p, "bandwidth: ");
	_env.readInput(buf, sizeof(buf)-1, prompt, false);

	strcpy(p, "content: ");
	_env.readInput(buf, sizeof(buf)-1, prompt, false);
*/
	return _thePurchase;
}

// -----------------------------
// class DummyPurchase
// -----------------------------
DummyPurchase::DummyPurchase(WeiwooAdminConsole& env)
:_env(env)
{
	WLock sync(*this);
}

::TianShanIce::Weiwoo::SessionPrx DummyPurchase::getSession(const ::Ice::Current& c)
{
	RLock sync(*this);
	throw TianShanIce::NotImplemented(__MSGLOC__ "getSession() not supported in DummyPurchase");
}

void DummyPurchase::provision(const ::Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: impl here")
}

void DummyPurchase::render(const ::TianShanIce::Streamer::StreamPrx& stream, const ::TianShanIce::Weiwoo::SessionPrx& sess, const ::Ice::Current& c)
{
	WLock sync(*this);
	if (!sess)
	{
		printf("\nDummyApp::render() NULL session, quit");
		return;
	}

	if (!stream)
	{
		printf("\nDummyApp::render() NULL stream, quit");
		return;
	}

	::TianShanIce::Weiwoo::ResourceMap resMap = sess->getReources();
	std::string uristr;
	// try to get the URI from the session context
	if (resMap.end() != resMap.find(::TianShanIce::Weiwoo::rtURI))
	{
		::TianShanIce::Weiwoo::Resource& resUri = resMap[::TianShanIce::Weiwoo::rtURI];
		::TianShanIce::Variant& uri = resUri.resourceData["uri"];
		if (resUri.resourceData.end() != resUri.resourceData.find("uri") && !resUri.resourceData["uri"].bRange && resUri.resourceData["uri"].strs.size() >0)
			uristr = resUri.resourceData["uri"].strs[0];
	}

	ZQ::common::URLStr url(uristr.c_str());
	::TianShanIce::Streamer::PlaylistItemSetupInfo info;
	info.contentName = url.getVar("item0");
	info.criticalStart = info.inTimeOffset = info.outTimeOffset =0;
	info.spliceIn = info.spliceOut = info.forceNormal = false;
	info.flags = 0;

	try
	{
		::TianShanIce::Streamer::PlaylistPrx pl = ::TianShanIce::Streamer::PlaylistPrx::checkedCast(stream);
		if (pl && !info.contentName.empty())
			pl->pushBack(1, info);
	}
	catch(const ::Ice::Exception& e)
	{
		printf("\n%s caught", e.ice_name().c_str());
	}


	
	printf("\nDummyApp::render(sess[%s])\n", sess->getId().c_str());

#pragma message ( __MSGLOC__ "TODO: impl here")
}

