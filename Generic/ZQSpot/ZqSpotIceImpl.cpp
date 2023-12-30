#include "ZqSpotIceImpl.h"
#include <IceUtil/Exception.h>

using namespace ZqSpotIce;

namespace ZQ {
namespace Spot {

//////////////////////////////////////////////////////////////////////////
// class SpotQueryImpl

SpotQueryImpl::SpotQueryImpl(const SpotMgr::NodeId& nodeId, 
	const std::string& appName, int processId, SpotMgr& spotMgr):
	_nodeId(nodeId), _appName(appName), _spotMgr(spotMgr), 
	_loadUpdateThread(*this)
{
	// 10%
	_defLoadWin = 1000;

	_spotKey = SpotMgr::SpotKey(processId, nodeId);

	_loadUpdateThread.start();

}

SpotQueryImpl::~SpotQueryImpl()
{
	_loadUpdateThread.stop();
}

::Ice::Int
SpotQueryImpl::listAllInterface(ZqSpotIce::InterfaceNames& names,
					const Ice::Current& current)
{
	/*
	std::vector<SpotMgr::InterfaceInfo > result;
	std::vector<SpotMgr::InterfaceInfo >::iterator itor;
	_spotMgr.findInterface(_spotKey, result);
	for (itor = result.begin(); itor < result.end(); itor ++) {
		names.push_back(itor->ifName);
	}
	*/

	printf("SpotQueryImpl::listAllInterface()\n");

	LocalInterfaceMap::iterator itor;
	RWLockGuard gruard(_ifMapLock, RWLockGuard::LOCK_READ);

	for (itor = _ifMap.begin(); itor != _ifMap.end(); itor ++) {
		names.push_back(itor->first);
	}

    return names.size();
}

::Ice::Int
SpotQueryImpl::getInterfaceLoad(const ::std::string& ifName,
					const Ice::Current& current)
{
	LocalInterfaceMap::iterator itor;
	RWLockGuard gruard(_ifMapLock, RWLockGuard::LOCK_READ);

	itor = _ifMap.find(ifName);
	if (itor == _ifMap.end())
		return -1;
	return itor->second.load;
}

void
SpotQueryImpl::sinkEvents(const SpotBindPrx& bind,
				  const Ice::Current& current)
{
	_spotBindSet.insert(bind);
}

bool
SpotQueryImpl::subscribeLoadChange(const LoadBindPrx& bind, 
					   const ::std::string& ifName,
					   ::Ice::Int loadWindow,
					   const Ice::Current& current)
{
	LocalInterfaceMap::iterator itor;
	RWLockGuard gruard(_ifMapLock);
	itor = _ifMap.find(ifName);
	if (itor == _ifMap.end())
		return false;
	LoadSubscriber loadInfo;
	loadInfo.lastLoad = 0;
	loadInfo.loadWindow = loadWindow;
	time(&loadInfo.updateTime);
	std::pair<SubscriberMap::iterator, bool> insRet;
	insRet = itor->second.subscribers.insert(
		SubscriberMap::value_type(bind, loadInfo));
	if (!insRet.second)
		return false;

    return true;
}

bool
SpotQueryImpl::unsubscribeLoadChange(const LoadBindPrx& bind, 
						const ::std::string& ifName,
						const Ice::Current& current)
{
	LocalInterfaceMap::iterator itor;
	RWLockGuard gruard(_ifMapLock);

	itor = _ifMap.find(ifName);
	if (itor == _ifMap.end())
		return false;
	return itor->second.subscribers.erase(bind) > 0;
}

::std::string
SpotQueryImpl::getOsInfo(const Ice::Current& current)
{
    return "unavailable";
}

::std::string
SpotQueryImpl::getProcessorInfo(const Ice::Current& current)
{
    return "unavailable";
}

bool SpotQueryImpl::exportInterface(const ::Ice::ObjectPrx& obj, 
	const std::string& iid, int load, time_t t, 
	InterfaceLoadEvaluator*	eval)
{
	assert(eval);
	RWLockGuard gruard(_ifMapLock);

	LocalInterface ifInfo;
	ifInfo.ifName = iid;
	ifInfo.obj = obj;
	ifInfo.load = load;	
	ifInfo.stamp = t;
	ifInfo.eval = eval;

	std::pair<LocalInterfaceMap::iterator, bool> insRet;
	insRet = _ifMap.insert(std::pair<std::string, LocalInterface>(
		iid, ifInfo));
	if (!insRet.second)
		return false;
	// insRet.first->second.lock = new XRWLock;
	return true;
}

bool SpotQueryImpl::unexportInterface(const std::string& iid)
{
	RWLockGuard gruard(_ifMapLock, RWLockGuard::LOCK_READ);
	return _ifMap.erase(iid) > 0;
}

bool SpotQueryImpl::updateInterface(const std::string& iid, 
	int load, time_t t)
{
	RWLockGuard gruard(_ifMapLock, RWLockGuard::LOCK_WRITE);
	LocalInterfaceMap::iterator itor = _ifMap.find(iid);
	if (itor == _ifMap.end())
		return false;
	LocalInterface& ifInfo = itor->second;

	// RWLockGuard gruard2(*ifInfo.lock);
	ifInfo.load = load;
	ifInfo.stamp = t;

	SpotMgr::InterfaceInfo ifInfo2;
	ifInfo2.ifName = iid;
	ifInfo2.spotKey = _spotKey;
	ifInfo2.load = load;
	ifInfo2.lastUpdate = t;
	_spotMgr.updateInterface(ifInfo2);
	return true;
}

void SpotQueryImpl::startUpdateLoad()
{
	_loadUpdateThread.start();
}

void SpotQueryImpl::stopUpdateLoad()
{
	_loadUpdateThread.stop();
}

void SpotQueryImpl::updateLoad()
{
	int newLoad;
	LocalInterfaceMap& ifMap = _ifMap;
	LocalInterfaceMap::iterator ifMapIt;
	
	RWLockGuard gruard(_ifMapLock, RWLockGuard::LOCK_WRITE);

	for (ifMapIt = ifMap.begin(); ifMapIt != ifMap.end(); ifMapIt ++) {
		LocalInterface& ifInfo = ifMapIt->second;
		// ifInfo.lock->lockWrite();
		newLoad = ifInfo.eval->calcLoad(ifInfo.obj);
		if (newLoad != ifInfo.load) {
			ifInfo.load = newLoad;
			SpotMgr::InterfaceInfo ifInfo2;
			ifInfo2.ifName = ifInfo.ifName;
			ifInfo2.spotKey = _spotKey;
			ifInfo2.load = newLoad;
			time(&ifInfo2.lastUpdate);
			_spotMgr.updateInterface(ifInfo2);

			SubscriberMap& subMap = ifInfo.subscribers;
			SubscriberMap::iterator subMapIt;
			for (subMapIt = subMap.begin(); subMapIt != subMap.end(); 
				subMapIt ++) {

				LoadSubscriber& ls = subMapIt->second;
				
				if (abs(newLoad - ls.lastLoad) >= ls.loadWindow) {
					const LoadBindPrx& loadBindPrx = subMapIt->first;
					try {
						loadBindPrx->OnLoadChanged(_spotKey, 
							ifInfo.ifName, newLoad);						
					} catch(Ice::Exception& ) {
						subMap.erase(subMapIt ++);
						OutputDebugString("subMap.erase(subMapIt ++) done\n");
						if (subMapIt == subMap.end())
							break;
						continue;
					} catch(...) {
						subMap.erase(subMapIt ++);
						OutputDebugString("subMap.erase(subMapIt ++) done\n");
						if (subMapIt == subMap.end())
							break;
						continue;
					}

					ls.lastLoad = newLoad;
					ls.updateTime = ifInfo2.lastUpdate;
				}
			}
		}

		// ifInfo.lock->unlockWrite();
	}

}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// class SpotBindImpl

SpotBindImpl::SpotBindImpl(SpotEnv& frame): _spotFrame(frame)
{

}

void
SpotBindImpl::OnInterfaceAdded(const SpotKey& spot,
				       const ::std::string& interfacename,
				       const Ice::Current& current)
{
	
}

void
SpotBindImpl::OnInterfaceRemoved(const SpotKey& spot,
					 const ::std::string& interfacename,
					 const Ice::Current& current)
{

}

//////////////////////////////////////////////////////////////////////////
// LoadBindImpl

LoadBindImpl::LoadBindImpl(SpotEnv& frame): _spotFrame(frame)
{

}

void
LoadBindImpl::OnLoadChanged(const SpotKey& spotKey,
				    const ::std::string& ifName,
				    ::Ice::Int newLoad,
				    const Ice::Current& current)
{
	printf("LoadBindImpl::OnLoadChanged():\t Node: %s, ProcessId: %d, "
		"IfName: %s, newLoad: %d\n", spotKey.nodeguid.c_str(), 
		spotKey.processId, ifName.c_str(), newLoad);
	SpotMgr* spotMgr = _spotFrame.getSpotMgr();
	SpotMgr::InterfaceInfo ifInfo;
	ifInfo.ifName = ifName;
	ifInfo.spotKey = spotKey;
	ifInfo.load = newLoad;
	spotMgr->updateInterface(ifInfo);
}

//////////////////////////////////////////////////////////////////////////
// class InterfaceLoadUpdateThread
InterfaceLoadUpdateThread::InterfaceLoadUpdateThread(
	SpotQueryImpl& spotQuery, int sleepTime /* = 5000 */): 
	_spotQuery(spotQuery), _sleepTime(sleepTime)
	
{
	_stop = false;
}

bool InterfaceLoadUpdateThread::init()
{
	_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	return true;
}

int InterfaceLoadUpdateThread::run()
{
	DWORD dwState;
	
	while (!_stop){
		dwState = WaitForSingleObject(_event, _sleepTime);
		if (dwState == WAIT_OBJECT_0) { // quit
			return 0;
		} else if (dwState == WAIT_TIMEOUT) {
			_spotQuery.updateLoad();
		} else {
			// error
			return -1;
		}
	}

	return 0;
}

void InterfaceLoadUpdateThread::final()
{
	CloseHandle(_event);
}

void InterfaceLoadUpdateThread::stop()
{
	_stop = true;
	SetEvent(_event);
	waitHandle(INFINITE);
}

//////////////////////////////////////////////////////////////////////////

// -----------------------------
// class CommunicatorImpl
// -----------------------------
/*
CommunicatorImpl::CommunicatorImpl(::Ice::CommunicatorPtr& ic)
:_ic(ic) // , bSelfInited(false) //, _zqLocator(NULL)
{
	_backupLocator = _ic->getDefaultLocator();

	::Ice::ObjectAdapterPtr selfAdapter = _ic->createObjectAdapter("ZqSpot.Communicator.Internal");

	DWORD pid = ::GetProcessId(NULL);
	time_t now = ::time(NULL);
	char insIdstr[32];
	sprintf(insIdstr, "%08x%08x", pid, now);

   //
    // Setup a internal locator to be used by the IceGrid registry itself. This locator is 
    // registered with the registry object adapter which is using an independant threadpool.
    //
    ::Ice::ObjectPtr locator = new LocatorImpl(_backupLocator);
	::Ice::Identity insId = ::Ice::stringToIdentity(insIdstr);
    selfAdapter->add(locator, insId);
    ::Ice::ObjectPrx obj = selfAdapter->createDirectProxy(insId);
    _ic->setDefaultLocator(::Ice::LocatorPrx::uncheckedCast(obj->ice_collocationOptimization(false)));
}
*/

CommunicatorImpl::CommunicatorImpl(const Ice::CommunicatorPtr& ic)
	:_ic(ic)
{

}

CommunicatorImpl::~CommunicatorImpl()
{

}

// methods to redirect to Ice::Communicator
// -------------------------------------------
void CommunicatorImpl::destroy()
{
	_ic->destroy();
}

void CommunicatorImpl::shutdown()
{
	_ic->shutdown();
}

void CommunicatorImpl::waitForShutdown()
{
	_ic->waitForShutdown();
}

::Ice::ObjectPrx CommunicatorImpl::stringToProxy(const ::std::string& s) const
{
	return _ic->stringToProxy(s);
}

::std::string CommunicatorImpl::proxyToString(const ::Ice::ObjectPrx& o) const
{
	return _ic->proxyToString(o);
}

::Ice::ObjectAdapterPtr CommunicatorImpl::createObjectAdapter(const ::std::string& s)
{
	return new ObjectAdapterImpl(_ic->createObjectAdapter(s));
}

::Ice::ObjectAdapterPtr CommunicatorImpl::createObjectAdapterWithEndpoints(const ::std::string&s, const ::std::string& ep)
{
	return new ObjectAdapterImpl(_ic->createObjectAdapterWithEndpoints(s, ep));
}

void CommunicatorImpl::addObjectFactory(const ::Ice::ObjectFactoryPtr& of, const ::std::string& s)
{
	_ic->addObjectFactory(of, s);
}

void CommunicatorImpl::removeObjectFactory(const ::std::string& s)
{
	_ic->removeObjectFactory(s);
}

::Ice::ObjectFactoryPtr CommunicatorImpl::findObjectFactory(const ::std::string& s) const
{
	return _ic->findObjectFactory(s);
}

void CommunicatorImpl::setDefaultContext(const ::Ice::Context& c)
{
	_ic->setDefaultContext(c);
}

::Ice::Context CommunicatorImpl::getDefaultContext() const
{
	return _ic->getDefaultContext();
}

::Ice::PropertiesPtr CommunicatorImpl::getProperties() const
{
	return _ic->getProperties();
}

::Ice::LoggerPtr CommunicatorImpl::getLogger() const
{
	return _ic->getLogger();
}

void CommunicatorImpl::setLogger(const ::Ice::LoggerPtr& l)
{
	_ic->setLogger(l);
}

::Ice::StatsPtr CommunicatorImpl::getStats() const
{
	return _ic->getStats();
}

void CommunicatorImpl::setStats(const ::Ice::StatsPtr& s)
{
	_ic->setStats(s);
}

::Ice::RouterPrx CommunicatorImpl::getDefaultRouter() const
{
	return _ic->getDefaultRouter();
}

void CommunicatorImpl::setDefaultRouter(const ::Ice::RouterPrx& r)
{
	_ic->setDefaultRouter(r);
}

::Ice::LocatorPrx CommunicatorImpl::getDefaultLocator() const
{
	return _ic->getDefaultLocator();
}

void CommunicatorImpl::setDefaultLocator(const ::Ice::LocatorPrx& l)
{
	_ic->setDefaultLocator(l);
}


::Ice::PluginManagerPtr CommunicatorImpl::getPluginManager() const
{
	return _ic->getPluginManager();
}

void CommunicatorImpl::flushBatchRequests()
{
	_ic->flushBatchRequests();
}

// -----------------------------
// class ObjectAdapterImpl
// -----------------------------
::std::string ObjectAdapterImpl::getName() const
{
	return _iceAdapter->getName();
}

::Ice::CommunicatorPtr ObjectAdapterImpl::getCommunicator() const
{
	return _iceAdapter->getCommunicator();
}

void ObjectAdapterImpl::activate()
{
	_iceAdapter->activate();
}

void ObjectAdapterImpl::hold()
{
	_iceAdapter->hold();
}

void ObjectAdapterImpl::waitForHold()
{
	_iceAdapter->waitForHold();
}

void ObjectAdapterImpl::deactivate()
{
	_iceAdapter->deactivate();
}

void ObjectAdapterImpl::waitForDeactivate()
{
	_iceAdapter->waitForDeactivate();
}

::Ice::ObjectPrx ObjectAdapterImpl::add(const ::Ice::ObjectPtr& obj, const ::Ice::Identity& id)
{
	return _iceAdapter->add(obj, id);
}

::Ice::ObjectPrx ObjectAdapterImpl::addFacet(const ::Ice::ObjectPtr& obj, const ::Ice::Identity& id, const ::std::string& s)
{
	return _iceAdapter->addFacet(obj, id, s);
}


::Ice::ObjectPrx ObjectAdapterImpl::addWithUUID(const ::Ice::ObjectPtr& obj)
{
	return _iceAdapter->addWithUUID(obj);
}

::Ice::ObjectPrx ObjectAdapterImpl::addFacetWithUUID(const ::Ice::ObjectPtr& obj, const ::std::string& s)
{
	return _iceAdapter->addFacetWithUUID(obj, s);
}

::Ice::ObjectPtr ObjectAdapterImpl::remove(const ::Ice::Identity& id)
{
	return _iceAdapter->remove(id);
}

::Ice::ObjectPtr ObjectAdapterImpl::removeFacet(const ::Ice::Identity& id, const ::std::string& s)
{
	return _iceAdapter->removeFacet(id, s);
}

::Ice::FacetMap ObjectAdapterImpl::removeAllFacets(const ::Ice::Identity& id)
{
	return _iceAdapter->removeAllFacets(id);
}

::Ice::ObjectPtr ObjectAdapterImpl::find(const ::Ice::Identity& id) const
{
	return _iceAdapter->find(id);
}

::Ice::ObjectPtr ObjectAdapterImpl::findFacet(const ::Ice::Identity& id, const ::std::string& s) const
{
	return _iceAdapter->findFacet(id, s);
}

::Ice::FacetMap ObjectAdapterImpl::findAllFacets(const ::Ice::Identity& id) const
{
	return _iceAdapter->findAllFacets(id);
}

::Ice::ObjectPtr ObjectAdapterImpl::findByProxy(const ::Ice::ObjectPrx& obj) const
{
	return _iceAdapter->findByProxy(obj);
}

void ObjectAdapterImpl::addServantLocator(const ::Ice::ServantLocatorPtr& sl, const ::std::string& s)
{
	_iceAdapter->addServantLocator(sl, s);
}

::Ice::ServantLocatorPtr ObjectAdapterImpl::findServantLocator(const ::std::string& s) const
{
	return _iceAdapter->findServantLocator(s);
}

::Ice::ObjectPrx ObjectAdapterImpl::createProxy(const ::Ice::Identity& id) const
{
	return _iceAdapter->createProxy(id);
}

::Ice::ObjectPrx ObjectAdapterImpl::createDirectProxy(const ::Ice::Identity& id) const
{
	return _iceAdapter->createDirectProxy(id);
}

::Ice::ObjectPrx ObjectAdapterImpl::createIndirectProxy(const ::Ice::Identity& id) const
{
	return _iceAdapter->createIndirectProxy(id);
}

::Ice::ObjectPrx ObjectAdapterImpl::createReverseProxy(const ::Ice::Identity& id) const
{
	return _iceAdapter->createReverseProxy(id);
}

void ObjectAdapterImpl::addRouter(const ::Ice::RouterPrx& r)
{
	_iceAdapter->addRouter(r);
}

void ObjectAdapterImpl::removeRouter(const ::Ice::RouterPrx& r)
{
	_iceAdapter->removeRouter(r);
}

void ObjectAdapterImpl::setLocator(const ::Ice::LocatorPrx& l)
{
	_iceAdapter->setLocator(l);
}

} // namespace Spot
} // namespace ZQ

