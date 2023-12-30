// SpotMgr.cpp: implementation of the SpotMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "SpotMgr.h"
#include <time.h>
#include "SpotData.h"

using namespace ZQ::common;

namespace ZQ {
namespace Spot {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SpotMgr::SpotMgr()
{
	_spotTimeo = 60;	// 10 minute
}

SpotMgr::~SpotMgr()
{

}

bool SpotMgr::init()
{
	return true;
}

void SpotMgr::uninit()
{

}

bool SpotMgr::addNode(const NodeInfo& info)
{
	std::pair<NodeMap::iterator, bool> insRet;

	{
		RWLockGuard gurad(_nodeMapLock);
		insRet = _nodeMap.insert(std::pair<NodeId, NodeInfo>(
			info.nodeId, info));
	}

	SpotDataEventVec::iterator itor;
	if (insRet.second) {
		for (itor = _dataEventVec.begin(); itor != _dataEventVec.end(); 
			itor ++) {
			char nodeId[128];
			info.nodeId.toString(nodeId, sizeof(nodeId));
			(*itor)->onNodeAdded(nodeId);
		}
	}

	return insRet.second;
}

bool SpotMgr::delNode(const SpotMgr::NodeId& nodeId)
{
	bool result;

	{
		RWLockGuard gurad(_nodeMapLock);
		result = _nodeMap.erase(nodeId) > 0;
	}

	SpotDataEventVec::iterator itor;
	if (result) {
		for (itor = _dataEventVec.begin(); itor != _dataEventVec.end(); 
			itor ++) {
			char nid[128];
			nodeId.toString(nid, sizeof(nid));
			(*itor)->onNodeRemoved(nid);
		}
			
		delSpot(nodeId);
	}

	return result;
}

bool SpotMgr::updateNode(const NodeInfo& info)
{
	return false;
}

bool SpotMgr::addSpot(const SpotMgr::SpotInfo& info)
{
	std::pair<SpotMap::iterator, bool> insRet;

	{
		RWLockGuard gurad(_spotMapLock);
		insRet = _spotMap.insert(std::pair<SpotKey, SpotInfo>(
			info.spotKey, info));
	}

	if (insRet.second) {
		SpotDataEventVec::iterator itor;
		for (itor = _dataEventVec.begin(); itor != _dataEventVec.end(); 
			itor ++) {
			(*itor)->onSpotAdded(info.spotKey.nodeguid, 
				info.spotKey.processId);
		}
	}
	return insRet.second;
}

bool SpotMgr::delSpot(const SpotMgr::SpotKey& spotKey)
{
	bool result;

	{
		RWLockGuard gurad(_spotMapLock);
		result = _spotMap.erase(spotKey) > 0;
	}
	
	if (result) {
		SpotDataEventVec::iterator itor;

		for (itor = _dataEventVec.begin(); itor != _dataEventVec.end(); 
			itor ++) {

			(*itor)->onSpotRemoved(spotKey.nodeguid, spotKey.processId);
		}

		delInterface(spotKey);
	}

	return result;
}

bool SpotMgr::delSpot(const NodeId& nodeId)
{
	typedef std::vector<SpotKey> SpotKeyVec;
	SpotKeyVec spotKeys;

	{
		RWLockGuard gurad(_spotMapLock);
		SpotMap::iterator itor;
		for(itor = _spotMap.begin(); itor != _spotMap.end(); itor ++) {
			if (Guid(itor->first.nodeguid.c_str()) == nodeId) {
				delInterface(itor->first);
				spotKeys.push_back(itor->first);
				_spotMap.erase(itor ++);
			}
		}
	}

	for (SpotKeyVec::iterator skIt = spotKeys.begin(); skIt != spotKeys.end();
		skIt ++) {
		
		SpotDataEventVec::iterator itor;
		for (itor = _dataEventVec.begin(); itor != _dataEventVec.end(); 
			itor ++) {

			(*itor)->onSpotRemoved(skIt->nodeguid, skIt->processId);
		}

	}

	return true;
}

bool SpotMgr::updateSpot(const SpotInfo& info)
{
	{
		RWLockGuard gurad(_spotMapLock);
		SpotMap::iterator itor = _spotMap.find(info.spotKey);
		if (itor == _spotMap.end())
			return false;
		itor->second = info;
	}

	SpotDataEventVec::iterator itor;

	for (itor = _dataEventVec.begin(); itor != _dataEventVec.end(); 
		itor ++) {

		(*itor)->onSpotChanged(info.spotKey.nodeguid, 
			info.spotKey.processId);
	}

	return true;
}

bool SpotMgr::addInterface(const InterfaceInfo& ifInfo)
{
	{
		RWLockGuard gurad(_ifMapLock);
		_ifMap.insert(
			std::pair<std::string, InterfaceInfo>(ifInfo.ifName, ifInfo)
			);
	}

	SpotDataEventVec::iterator itor;
	for (itor = _dataEventVec.begin(); itor != _dataEventVec.end(); 
		itor ++) {
		(*itor)->onInterfaceAdded(ifInfo.spotKey.nodeguid, 
			ifInfo.spotKey.processId, ifInfo.ifName);
	}

	return true;
}

bool SpotMgr::delInterface(const std::string& ifName, 
	const SpotMgr::SpotKey& spotKey)
{
	typedef std::pair<InterfaceMap::iterator, 
		InterfaceMap::iterator> interface_range;

	bool result = false;

	{
		RWLockGuard gurad(_ifMapLock);
		interface_range range = _ifMap.equal_range(ifName);
		for (InterfaceMap::iterator itor = range.first; 
			itor != range.second; itor ++) {
			if (itor->second.spotKey == spotKey) {
				_ifMap.erase(itor ++);

				result = true;
				break;
			}
		}
	}

	if (result) {
		SpotDataEventVec::iterator eventIt;
		for (eventIt = _dataEventVec.begin(); 
			eventIt != _dataEventVec.end(); 
			eventIt ++) {

			(*eventIt)->onInterfaceRemoved(spotKey.nodeguid, 
				spotKey.processId, ifName);
		}
	}

	return result;
}

bool SpotMgr::delInterface(const SpotKey& spotKey)
{
	typedef std::vector<std::string> IfNameVec;
	IfNameVec ifNames;
	bool result = false;

	{
		RWLockGuard gurad(_ifMapLock);
		for (InterfaceMap::iterator itor = _ifMap.begin(); 
			itor != _ifMap.end(); itor ++) {

			if (itor->second.spotKey == spotKey) {
				ifNames.push_back(itor->second.ifName);
				_ifMap.erase(itor ++);
				result = true;
				break;
			}
		}
	}

	if (result) {
		SpotDataEventVec::iterator eventIt;
		for (eventIt = _dataEventVec.begin(); 
			eventIt != _dataEventVec.end(); 
			eventIt ++) {
			
			for (IfNameVec::iterator nameIt = ifNames.begin();
				nameIt != ifNames.end(); nameIt ++) {
				
				(*eventIt)->onInterfaceRemoved(spotKey.nodeguid, 
					spotKey.processId, *nameIt);
			}

			
		}
	}

	return result;
}

bool SpotMgr::updateInterface(const InterfaceInfo& ifInfo)
{
	typedef std::pair<InterfaceMap::iterator, 
		InterfaceMap::iterator> interface_range;

	SpotKey spotKey;
	bool result = false;

	{
		RWLockGuard gurad(_ifMapLock, RWLockGuard::LOCK_WRITE);
		interface_range range = _ifMap.equal_range(ifInfo.ifName);
		for (InterfaceMap::iterator itor = range.first; 
			itor != range.second; itor ++) {
			if (itor->second.spotKey == ifInfo.spotKey) {
				InterfaceInfo& info = itor->second;
				info.load = ifInfo.load;
				time(&info.lastUpdate);
				spotKey = spotKey;
				result = true;
				break;			
			}
		}
	}

	if (result) {
		SpotDataEventVec::iterator eventIt;
		for (eventIt = _dataEventVec.begin(); 
			eventIt != _dataEventVec.end(); 
			eventIt ++) {

			(*eventIt)->onInterfaceChanged(ifInfo.spotKey.nodeguid, 
				ifInfo.spotKey.processId, ifInfo.ifName);
		}
	}

	return result;
}

int SpotMgr::findInterface(const std::string& ifName, 
	std::vector<SpotMgr::InterfaceInfo >& result)
{
	typedef std::vector<SpotKey> SpotKeyVec;
	SpotKeyVec spotKeys;

	{
		bool erased;
		RWLockGuard gurad(_ifMapLock, RWLockGuard::LOCK_WRITE);
		interface_range range = _ifMap.equal_range(ifName);
		for (InterfaceMap::iterator itor = range.first; 
			itor != range.second; itor ++) {

			InterfaceInfo& ifInfo = itor->second;
			SpotInfo spotInfo;
			if (_getSpot(ifInfo.spotKey, spotInfo, erased))
				result.push_back(itor->second);
			else {
				spotKeys.push_back(itor->second.spotKey);
				_ifMap.erase(itor ++);			
			}
		}
	}

	for (SpotKeyVec::iterator skIt = spotKeys.begin();
		skIt != spotKeys.end(); skIt ++) {
		
		SpotDataEventVec::iterator eventIt;
		for (eventIt = _dataEventVec.begin(); 
			eventIt != _dataEventVec.end(); 
			eventIt ++) {

			(*eventIt)->onInterfaceRemoved(skIt->nodeguid,  
				skIt->processId, ifName);

		}
	}

	return result.size();
}

int SpotMgr::findInterface(const SpotMgr::SpotKey& spotKey, 
		std::vector<SpotMgr::InterfaceInfo >& result)
{
	InterfaceMap::const_iterator itor;

	SpotInfo spotInfo;
	if (!getSpot(spotKey, spotInfo))
		return 0;

	RWLockGuard gurad(_ifMapLock, RWLockGuard::LOCK_READ);
	for (itor = _ifMap.begin(); itor != _ifMap.end(); itor ++) {
		if (itor->second.spotKey == spotKey)
			result.push_back(itor->second);
	}

	return result.size();
}

bool SpotMgr::findInterface(const std::string& ifName, 
		const SpotKey& spotKey, 
		InterfaceInfo* info)
{
	typedef std::pair<InterfaceMap::const_iterator, 
		InterfaceMap::const_iterator> interface_range;


	SpotInfo spotInfo;
	if (!getSpot(spotKey, spotInfo))
		return 0;

	RWLockGuard gurad(_ifMapLock, RWLockGuard::LOCK_READ);
	interface_range range = _ifMap.equal_range(ifName);
	for (InterfaceMap::const_iterator itor = range.first; 
		itor != range.second; itor ++) {
		if (itor->second.spotKey == spotKey) {
			if (info)
				*info = itor->second;
			return true;
		}

	}

	return false;
}

bool SpotMgr::_getSpot(const SpotKey& spotKey, SpotInfo& info, bool& erased)
{
	SpotMap::iterator itor = _spotMap.find(spotKey);
	
	if (itor == _spotMap.end())
		return false;
	if (checkSpotTimeout(itor->second)) {
		info = itor->second;
		return true;
	} else {
		erased = _spotMap.erase(spotKey) > 0;
		return false;
	}
}

bool SpotMgr::getSpot(const SpotMgr::SpotKey& spotKey, 
	SpotMgr::SpotInfo& info)
{
	bool result;
	bool erased = false;

	{
		RWLockGuard gurad(_spotMapLock, RWLockGuard::LOCK_WRITE);
		result = _getSpot(spotKey, info, erased);
	}

	assert(!(result && erased));

	if (!result) {
		if (erased){
			SpotDataEventVec::iterator eventIt;
			for (eventIt = _dataEventVec.begin(); 
				eventIt != _dataEventVec.end(); 
				eventIt ++) {

				(*eventIt)->onSpotRemoved(spotKey.nodeguid, 
					spotKey.processId);
			}
		}
	}

	return result;
}

int SpotMgr::findSpotByAppName(const std::string& appName, 
	std::vector<SpotMgr::SpotInfo >& result) const
{
	SpotMap::const_iterator itor;
	RWLockGuard gurad(_spotMapLock, RWLockGuard::LOCK_READ);
	for (itor = _spotMap.begin(); itor != _spotMap.end(); itor ++) {
		if (itor->second.application == appName)
			result.push_back(itor->second);
	}

	return result.size();
}

bool SpotMgr::getNode(const SpotMgr::NodeId& nodeId, 
	SpotMgr::NodeInfo& info) const
{
	RWLockGuard gurad(_nodeMapLock, RWLockGuard::LOCK_READ);
	NodeMap::const_iterator itor = _nodeMap.find(nodeId);
	if (itor == _nodeMap.end())
		return false;
	info = itor->second;
	return true;
}

bool SpotMgr::sinkDataChangeEvent(SpotDataEvent* event)
{
	unsinkDataChangeEvent(event);
	_dataEventVec.push_back(event);
	return true;
}

bool SpotMgr::unsinkDataChangeEvent(SpotDataEvent* event)
{
	SpotDataEventVec::iterator itor;
	for (itor = _dataEventVec.begin(); itor != _dataEventVec.end(); 
		itor ++) {

		if (*itor == event) {
			_dataEventVec.erase(itor ++);
			return true;
		}

	}
	return false;
}

int SpotMgr::createNodeSnap(NodeVector& nodeVec) const
{
	nodeVec.clear();
	RWLockGuard gurad(_nodeMapLock, RWLockGuard::LOCK_READ);
	int nodeCount = _nodeMap.size();
	if (nodeCount <= 0)
		return 0;

	NodeMap::const_iterator itor;
	for(itor = _nodeMap.begin(); itor != _nodeMap.end(); itor ++) {
		nodeVec.push_back(itor->second);
	}

	return nodeCount;
}

bool SpotMgr::checkSpotTimeout(const SpotMgr::SpotInfo& nodeInfo) const
{
	time_t t;
	time(&t);
	return abs(t - nodeInfo.updateTime) < _spotTimeo;
}

int SpotMgr::createSpotSnap(const NodeId& nodeId, 
	SpotVector& spotVec)
{
	SpotMap::const_iterator itor;
	int i = 0;
	spotVec.clear();
	typedef std::vector<SpotKey> SpotKeyVec;
	SpotKeyVec spotKeys;
	{
		RWLockGuard gurad(_spotMapLock, RWLockGuard::LOCK_READ);
		for(itor = _spotMap.begin(); itor != _spotMap.end(); itor ++) {
			if (nodeId == NodeId(itor->first.nodeguid.c_str()))
				if (checkSpotTimeout(itor->second))
					spotVec.push_back(itor->second);
				else 
					spotKeys.push_back(itor->first);
		}
	}

	for (SpotKeyVec::iterator skIt = spotKeys.begin(); 
		skIt != spotKeys.end(); skIt ++) {

		delSpot(*skIt);
	}

	return spotVec.size();
}

int SpotMgr::createInterfaceSnap(const SpotKey& spotKey, 
	InterfaceVector& ifVec)
{
	InterfaceMap::const_iterator itor;
	ifVec.clear();

	SpotInfo spotInfo;
	if (!getSpot(spotKey, spotInfo))
		return 0;

	RWLockGuard gurad(_ifMapLock, RWLockGuard::LOCK_READ);
	for (itor = _ifMap.begin(); itor != _ifMap.end(); itor ++) {
		if (itor->second.spotKey == spotKey)
			ifVec.push_back(itor->second);
	}

	return ifVec.size();
}

int SpotMgr::createAllInterfaceSnap(
	const char* ifName OPTIONAL, InterfaceVector& ifVec, 
	bool nameUnique) const
{
	InterfaceMap::const_iterator itor;
	ifVec.clear();

	typedef std::pair<InterfaceMap::const_iterator, 
		InterfaceMap::const_iterator> if_range;

	RWLockGuard gurad(_ifMapLock, RWLockGuard::LOCK_READ);
	if (ifName) {
		if_range range = _ifMap.equal_range(ifName);
		for (InterfaceMap::const_iterator itor = range.first; 
			itor != range.second; itor ++) {
			ifVec.push_back(itor->second);
		}
	} else {
		for (itor = _ifMap.begin(); itor != _ifMap.end(); itor ++) {
			if (nameUnique) {
				bool foundName = false;
				InterfaceVector::const_iterator ifVecIt;
				for (ifVecIt = ifVec.begin(); ifVecIt != ifVec.end(); 
					ifVecIt ++) {

					if (ifVecIt->ifName == itor->first) {
						foundName = true;
						break;
					}
				}

				if (!foundName)
					ifVec.push_back(itor->second);
				
			} else
				ifVec.push_back(itor->second);
		}
	}

	return ifVec.size();
}

} // namespace ZQ {
} // namespace Spot {
