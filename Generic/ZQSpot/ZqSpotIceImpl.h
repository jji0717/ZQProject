#ifndef __Zq_SpotIceImpl_H__
#define __Zq_SpotIceImpl_H__

#include "SpotMgr.h"
#include "ZQSpot.h"
#include "ZqSpotIce.h"

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

#include <time.h>
#include <stdio.h>

namespace ZQ {
namespace Spot {

class SpotBindImpl : virtual public ZqSpotIce::SpotBind
{
public:
	SpotBindImpl(SpotEnv& frame);
    virtual void OnInterfaceAdded(const ::ZqSpotIce::SpotKey&,
				  const ::std::string&,
				  const Ice::Current&);

    virtual void OnInterfaceRemoved(const ::ZqSpotIce::SpotKey&,
				    const ::std::string&,
				    const Ice::Current&);

protected:
	SpotEnv&		_spotFrame;
};

class LoadBindImpl : virtual public ZqSpotIce::LoadBind
{
public:
	LoadBindImpl(SpotEnv& frame);

    virtual void OnLoadChanged(const ::ZqSpotIce::SpotKey&,
			       const ::std::string&,
			       ::Ice::Int,
			       const Ice::Current&);
protected:
	SpotEnv&		_spotFrame;
};

class SpotQueryImpl;

class InterfaceLoadUpdateThread: public ZQ::common::NativeThread  {
public:
	InterfaceLoadUpdateThread(SpotQueryImpl& spotQuery, int sleepTime = 5000);
	virtual bool init();
	virtual int run();
	virtual void final();

	void stop();
protected:
	SpotQueryImpl&		_spotQuery;
	HANDLE				_event;
	int					_sleepTime;
	bool				_stop;
};

class SpotQueryImpl : virtual public ZqSpotIce::SpotQuery
{
	friend class InterfaceLoadUpdateThread;

public:

	SpotQueryImpl(const SpotMgr::NodeId& nodeId, 
		const std::string& appName, int processId, 
		SpotMgr& spotMgr);

	virtual ~SpotQueryImpl();

    virtual ::Ice::Int listAllInterface(::ZqSpotIce::InterfaceNames&,
					const Ice::Current&);

    virtual ::Ice::Int getInterfaceLoad(const ::std::string&,
					const Ice::Current&);

    virtual void sinkEvents(const ::ZqSpotIce::SpotBindPrx&,
			    const Ice::Current&);

    virtual bool subscribeLoadChange(const ZqSpotIce::LoadBindPrx& bind, 
					 const ::std::string&,
				     ::Ice::Int,
				     const Ice::Current&);

    virtual bool unsubscribeLoadChange(const ZqSpotIce::LoadBindPrx& bind, 
					 const ::std::string&,
				     const Ice::Current&);

    virtual ::std::string getOsInfo(const Ice::Current&);

    virtual ::std::string getProcessorInfo(const Ice::Current&);

// local routine
public:

	bool exportInterface(const ::Ice::ObjectPrx& obj, 
		const std::string& iid, int load, time_t t, 
		InterfaceLoadEvaluator*	eval);

	bool unexportInterface(const std::string& iid);

	bool updateInterface(const std::string& iid, int load, time_t t);

	void startUpdateLoad();
	void stopUpdateLoad();

	void updateLoad();
	
protected:

	typedef std::set<ZqSpotIce::SpotBindPrx> SpotBindSet;

	//////////////////////////////////////////////////////////////////////////

	struct LoadSubscriber {
		int			lastLoad;
		time_t		updateTime;
		int			loadWindow;
	};

	// a Proxy of bind to LoadSubscriber map
	typedef std::map <ZqSpotIce::LoadBindPrx, LoadSubscriber> SubscriberMap;
	typedef struct _LocalInterface {
		_LocalInterface()
		{
			// lock = NULL;
		}

		/*
		_LocalInterface(const _LocalInterface& li)
		{
			lock = NULL;
			ifName = li.ifName;
			obj = li.obj;
			eval = li.eval;
			load = li.load;
			stamp = li.stamp;
		}
		*/

		~_LocalInterface()
		{
			//if (lock)
			//	delete lock;
		}

		std::string					ifName;
		Ice::ObjectPrx				obj;
		InterfaceLoadEvaluator*		eval;
		int							load;
		time_t						stamp;
		// XRWLock*					lock;
		SubscriberMap				subscribers;
	} LocalInterface;
	
	/// a interfacename-to-info map
	typedef ::std::map <std::string, LocalInterface> LocalInterfaceMap;

	SpotMgr::SpotKey	_spotKey;
	SpotMgr::NodeId		_nodeId;			// identity of current node
	std::string			_appName;			// name of application
	SpotMgr&			_spotMgr;			// spot datatbase manager
	int					_defLoadWin;

	SpotBindSet			_spotBindSet;
	LocalInterfaceMap	_ifMap;
	XRWLock				_ifMapLock;

	InterfaceLoadUpdateThread	_loadUpdateThread;
};

//////////////////////////////////////////////////////////////////////////

// -----------------------------
// class ObjectAdapterImpl
// -----------------------------
class ObjectAdapterImpl : public ZqSpotIce::ObjectAdapter
{
public:

    ObjectAdapterImpl(const ::Ice::ObjectAdapterPtr& oa)
		: _iceAdapter(oa) {}

	virtual ~ObjectAdapterImpl() {}

public: // method implementations of ::Ice::ObjectAdapter

    virtual ::std::string getName() const;

    virtual ::Ice::CommunicatorPtr getCommunicator() const;

    virtual void activate();

    virtual void hold();

    virtual void waitForHold();

    virtual void deactivate();

    virtual void waitForDeactivate();

    virtual ::Ice::ObjectPrx add(const ::Ice::ObjectPtr&, const ::Ice::Identity&);

    virtual ::Ice::ObjectPrx addFacet(const ::Ice::ObjectPtr&, const ::Ice::Identity&, const ::std::string&);

    virtual ::Ice::ObjectPrx addWithUUID(const ::Ice::ObjectPtr&);

    virtual ::Ice::ObjectPrx addFacetWithUUID(const ::Ice::ObjectPtr&, const ::std::string&);

    virtual ::Ice::ObjectPtr remove(const ::Ice::Identity&);

    virtual ::Ice::ObjectPtr removeFacet(const ::Ice::Identity&, const ::std::string&);

    virtual ::Ice::FacetMap removeAllFacets(const ::Ice::Identity&);

    virtual ::Ice::ObjectPtr find(const ::Ice::Identity&) const;

    virtual ::Ice::ObjectPtr findFacet(const ::Ice::Identity&, const ::std::string&) const;

    virtual ::Ice::FacetMap findAllFacets(const ::Ice::Identity&) const;

    virtual ::Ice::ObjectPtr findByProxy(const ::Ice::ObjectPrx&) const;

    virtual void addServantLocator(const ::Ice::ServantLocatorPtr&, const ::std::string&);

    virtual ::Ice::ServantLocatorPtr findServantLocator(const ::std::string&) const;

    virtual ::Ice::ObjectPrx createProxy(const ::Ice::Identity&) const;

    virtual ::Ice::ObjectPrx createDirectProxy(const ::Ice::Identity&) const;

    virtual ::Ice::ObjectPrx createIndirectProxy(const ::Ice::Identity&) const;

    virtual ::Ice::ObjectPrx createReverseProxy(const ::Ice::Identity&) const;

    virtual void addRouter(const ::Ice::RouterPrx&);

    virtual void removeRouter(const ::Ice::RouterPrx&);

    virtual void setLocator(const ::Ice::LocatorPrx&);

protected:

	::Ice::ObjectAdapterPtr _iceAdapter;

	typedef struct _LocalInterfaceInfo
	{
		const ::Ice::ObjectPtr& obj;
		InterfaceLoadEvaluator* eval;
		int				load;
		time_t			stamp;
	} LocalInterfaceInfo;

	/// a interfacename-to-info map
	typedef ::std::map <std::string, LocalInterfaceInfo> LocalInterfaceInfoMap;

	LocalInterfaceInfoMap _localInterface;

};

typedef ::IceUtil::Handle<ObjectAdapterImpl> ObjectAdapterImplPtr;

/*
// -----------------------------
// class InterfaceSelect
// -----------------------------
class InterfaceSelect
{
	typedef struct _InterfaceOption
	{
		std::string endpoint;
		int			load;
	} InterfaceOption;

	typedef std::vector <InterfaceOption> InterfaceOptions;

	/// select an endpoint from the options by evaluating the given scores
	///@param options a list of potential interface providers
	///@return the selected interface
	virtual InterfaceOptions::iterator select(InterfaceOptions options)
	{
		// the default dummy Select always choose the first available interface
		for (InterfaceOptions::iterator it = options.begin(); it < options.end(); it++)
		{
			if (it->load < MAX_LOAD)
				return it;
		}
		
		return options.end();
	}
};
*/

// -----------------------------
// class CommunicatorImpl
// -----------------------------
class CommunicatorImpl : public ZqSpotIce::Communicator
{

public:

	// constructor
	CommunicatorImpl(const Ice::CommunicatorPtr& ic);

	virtual ~CommunicatorImpl();

public: // method implementations of ::Ice::Communicator

    virtual void destroy();

    virtual void shutdown();

    virtual void waitForShutdown();

    virtual ::Ice::ObjectPrx stringToProxy(const ::std::string&) const;

    virtual ::std::string proxyToString(const ::Ice::ObjectPrx&) const;

    virtual ::Ice::ObjectAdapterPtr createObjectAdapter(const ::std::string&);

    virtual ::Ice::ObjectAdapterPtr createObjectAdapterWithEndpoints(const ::std::string&, const ::std::string&);

    virtual void addObjectFactory(const ::Ice::ObjectFactoryPtr&, const ::std::string&);

    virtual void removeObjectFactory(const ::std::string&);

    virtual ::Ice::ObjectFactoryPtr findObjectFactory(const ::std::string&) const;

    virtual void setDefaultContext(const ::Ice::Context&);

    virtual ::Ice::Context getDefaultContext() const;

    virtual ::Ice::PropertiesPtr getProperties() const;

    virtual ::Ice::LoggerPtr getLogger() const;

    virtual void setLogger(const ::Ice::LoggerPtr&);

    virtual ::Ice::StatsPtr getStats() const;

    virtual void setStats(const ::Ice::StatsPtr&);

    virtual ::Ice::RouterPrx getDefaultRouter() const;

    virtual void setDefaultRouter(const ::Ice::RouterPrx&);

    virtual ::Ice::LocatorPrx getDefaultLocator() const;

    virtual void setDefaultLocator(const ::Ice::LocatorPrx&);

    virtual ::Ice::PluginManagerPtr getPluginManager() const;

    virtual void flushBatchRequests();

protected:

	void cleanup();
	bool bCleanedup;

	::Ice::CommunicatorPtr _ic;
	// std::string _defaultEndpoint;
};

} //namespace Spot
} // namespace ZQ


#endif // __Zq_SpotIceImpl_H__
