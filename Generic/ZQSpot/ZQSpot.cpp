#include "ZQSpot.h"
#include "ZQSpotIceImpl.h"
#include "SpotMgr.h"
#include "SpotData.h"
#include "ZQSpotAgent.h"
#include "InetAddr.h"

using namespace ZQ::common;
using namespace ZqSpotIce;

namespace ZQ {
namespace Spot {

SpotMgr spotManager;

static InterfaceLoadEvaluator dummyEvaluator;
static InterfaceSelect defIfSel;

bool analyseEndpoint(const char endpoint[], char* addr, int* addrlen, 
	unsigned short& port)
{
	const char* c = endpoint;
	int len = 0;
	bool colonFound = false;
	while(*c) {
		
		if (*c == ':') {
			port = atoi(c + 1);
			colonFound = true;
			break;
		} else {
			addr[len] = *c;
			len ++;
			if (len >= *addrlen)
				break;
		}
		c ++;
	}

	if (!colonFound)
		return false;

	addr[len] = 0;
	*addrlen = len;
	return true;
}

//////////////////////////////////////////////////////////////////////////

SpotEnv::SpotEnv(const SpotEnv::InitParams& init): 
		_threadPool(10), _initParams(init)
{
	_defLoadEvaluator = &dummyEvaluator;
	_defIfSel = &defIfSel;
	_hookIce = true;
	_processId = GetCurrentProcessId();
	_hostDesc = "Cary's host, CPU speed: 1.88TBHz, "
		"Memory 60G. Haha, It's a super computer.";
}

bool SpotEnv::initialize(int argc , char* argv[])
{
	Ice::CommunicatorPtr ic = Ice::initialize(argc, argv);
	if (ic == NULL)
		return false;

	InetMcastAddress groupAddr(_initParams.multicastGroupAddr.c_str());
	InetHostAddress bindAddr(_initParams.multicastBindAddr.c_str());
	Guid nodeId(_initParams.nodeId.c_str());
	_spotMgr = &spotManager;
	_spotData = new SpotStatusQuery(*_spotMgr);

	_hookIce = true;
	if (_hookIce) {
		_communicator = new ZQ::Spot::CommunicatorImpl(ic);
	} else {
		_communicator = ic;
	}

	char spotEndPoint[128];
	sprintf(spotEndPoint, "%s:%d", _initParams.iceServiceAddr.c_str(), 
		_initParams.iceServicePort);

	_spotQuery = new ZQ::Spot::SpotQueryImpl(nodeId, _initParams.appName, 
		_processId, *_spotMgr);
	if (!_spotQuery) {
		if (_hookIce)
			delete _communicator.get();
		return false;
	}

	char endpoint[256];
	sprintf(endpoint, "default -p %u", _initParams.iceServicePort);
	_objAdapter = _communicator->createObjectAdapterWithEndpoints(
		_initParams.appName, endpoint);
	if (_objAdapter == NULL) {
		_spotQuery = NULL;
		if (_hookIce)
			delete _communicator.get();
		return false;
	}

	Ice::ObjectPrx spotQueryPrx = _objAdapter->add(_spotQuery, 
		Ice::stringToIdentity("SpotQuery"));

	Ice::ObjectPtr lb = new ZQ::Spot::LoadBindImpl(*this);
	if (!lb) {
		_spotQuery = NULL;
		_objAdapter->remove(Ice::stringToIdentity("SpotQuery"));
		if (_hookIce)
			delete _communicator.get();
		return false;
	}

	_loadBind = ZqSpotIce::LoadBindPrx::checkedCast(
		_objAdapter->addWithUUID(lb));

	SpotMgr::NodeInfo nodeInfo;
	nodeInfo.nodeId = nodeId; 
	nodeInfo.procDesc = _hostDesc;
	_spotMgr->addNode(nodeInfo);

	SpotMgr::SpotInfo spotInfo;
	spotInfo.spotKey = SpotMgr::SpotKey(_processId, 
		_initParams.nodeId);
	spotInfo.application = _initParams.appName;
	spotInfo.endPoint = spotEndPoint;
	spotInfo.spotQuery = ZqSpotIce::SpotQueryPrx::checkedCast(spotQueryPrx);
	time(&spotInfo.updateTime);
	_spotMgr->addSpot(spotInfo);
	_objAdapter->activate();

	_nodeSpot = new ZQ::Spot::NodeSpot(_threadPool, groupAddr, 
		_initParams.multicastPort, bindAddr, nodeId, _initParams.appName, 
		_processId, spotEndPoint, *this);
	if (!_nodeSpot) {
		_spotQuery = NULL;
		if (_hookIce)
			delete _communicator.get();
		return false;
	}

	return _communicator;
}

void SpotEnv::shutdown()
{
	if (_nodeSpot)
		delete _nodeSpot;

	if (_spotQuery)
		_spotQuery = NULL;

	_communicator->shutdown();

	if (_hookIce) {
		ZQ::Spot::CommunicatorImpl* impl = 
			dynamic_cast<ZQ::Spot::CommunicatorImpl* >(
			_communicator.get());

		delete impl;
	}
}

const Ice::CommunicatorPtr SpotEnv::getCommunicator()
{
	return _communicator;
}

const std::string& SpotEnv::getNodeId()
{
	return _initParams.nodeId;
}

/* virtual */ SpotMgr* SpotEnv::getSpotMgr()
{
	return _spotMgr;
}

const Ice::ObjectPrx& SpotEnv::getLoadBind()
{
	return _loadBind;
}

const std::string& SpotEnv::getCurrentMachineDesc()
{
	return _hostDesc;
}

/*
void SpotEnv::setCurrentMachineDesc(const std::string& desc)
{
	_hostDesc = desc;
}
*/

InterfaceLoadEvaluator* SpotEnv::getDefaultEvaluator()
{
	return _defLoadEvaluator;
}

void SpotEnv::setDefaultEvaluator(
	InterfaceLoadEvaluator* evaluator)
{
	if (evaluator == NULL)
		evaluator = &dummyEvaluator;
	else
		_defLoadEvaluator = evaluator;
}

ZQ::Spot::SpotQueryImpl* SpotEnv::getSpotQuery()
{
	return _spotQuery;
}

bool SpotEnv::exportInterface(::Ice::ObjectPtr obj, 
	const std::string& iid, InterfaceLoadEvaluator* eval /* = NULL */)
{
	SpotMgr::NodeId nodeId(_initParams.nodeId.c_str());
	SpotMgr::SpotKey spotKey(_processId, nodeId);

	// already exported.
	if (_spotMgr->findInterface(iid, spotKey, NULL))
		return false;

	Ice::ObjectPrx objPrx = _objAdapter->add(obj, 
		Ice::stringToIdentity(iid));
	if (objPrx == NULL)
		return false;

	if (eval == NULL) // set default evaluator
		eval = _defLoadEvaluator;

	SpotMgr::InterfaceInfo ifInfo; 
	ifInfo.ifName = iid;
	ifInfo.spotKey = spotKey;
	ifInfo.load = eval->calcLoad(objPrx);
	time(&ifInfo.lastUpdate);

	if (!_spotMgr->addInterface(ifInfo)) {
		_objAdapter->remove(Ice::stringToIdentity(iid));
		return false;
	}

	// export interface to spot query
	bool r = _spotQuery->exportInterface(objPrx, iid, ifInfo.load, 
		ifInfo.lastUpdate, eval);
	assert(r);
	return true;
}

::Ice::ObjectPrx SpotEnv::openInterface(const ::std::string& iid, 
	InterfaceSelect* sel /* = NULL */)
{
	std::vector<SpotMgr::InterfaceInfo> ifInfoVector;
	std::vector<SpotMgr::InterfaceInfo>::iterator itor, selIt;
	if (!_spotMgr->findInterface(iid, ifInfoVector))
		return NULL;
	
	SpotMgr::SpotInfo spotInfo;
	InterfaceSelect::InterfaceOptions ifOptions;
	InterfaceSelect::InterfaceOption ifOp;

	for (itor = ifInfoVector.begin(); itor != ifInfoVector.end(); itor ++) {
		if (_spotMgr->getSpot(itor->spotKey, spotInfo)) {
			ifOp.load = itor->load;
			ifOp.endPoint = spotInfo.endPoint;
			ifOptions.push_back(ifOp);
		}
	}

	InterfaceSelect::InterfaceOptions::iterator ifOptsIt;
	if (sel == NULL)
		sel = _defIfSel;

selInterface:
	ifOptsIt = sel->select(ifOptions);

	if (ifOptsIt == ifOptions.end())
		return NULL;
	int pos = ifOptsIt - ifOptions.begin();
	selIt = ifInfoVector.begin() + pos;

	char buf[256];	
	char addr[64];
	int addrlen = sizeof(addr);
	unsigned short port;
	if (analyseEndpoint(ifOptsIt->endPoint.c_str(), 
		addr, &addrlen, port)) {

		::Ice::ObjectPrx objBase;
		sprintf(buf, "%s:default -h %s -p %d", iid.c_str(), addr, port);
		try {
			objBase = _communicator->stringToProxy(buf);			
			objBase->ice_ping();
		} catch (Ice::Exception& ) {
			_spotMgr->delInterface(iid, selIt->spotKey);
			ifOptions.erase(ifOptions.begin() + pos);
			goto selInterface;
		} catch(...) {
			_spotMgr->delInterface(iid, selIt->spotKey);
			ifOptions.erase(ifOptions.begin() + pos);
			goto selInterface;
		}
		
		return objBase;
	}

	return NULL;
}

Ice::ObjectPrx SpotEnv::getSpotQueryFromEndPoint(
	std::string endPoint)
{
	char addr[64];
	int addrlen = sizeof(addr);
	unsigned short port;
	if (!analyseEndpoint(endPoint.c_str(), 
		addr, &addrlen, port)) {

		return NULL;
	}
	
	char buf[512];
	sprintf(buf, "SpotQuery:default -h %s -p %d", addr, port);
	ZqSpotIce::SpotQueryPrx spotQuery;
	try {
		spotQuery = ZqSpotIce::SpotQueryPrx::checkedCast(
			_communicator->stringToProxy(buf));
	} catch(Ice::Exception& e) {
		std::cerr << "SpotEnv::getSpotQueryFromEndPoint() " << e 
			<< std::endl;
	}

	return spotQuery;
}

SpotStatusQuery* SpotEnv::getStatusQuery()
{
	return _spotData;
}

} // namespace Spot
} // namespace ZQ
