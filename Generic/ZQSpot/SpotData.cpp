// SpotData.cpp: implementation of the SpotData class.
//
//////////////////////////////////////////////////////////////////////

#include <map>
#include "SpotMgr.h"
#include "SpotData.h"
#include "guid.h"
#pragma warning(disable: 4786)
using namespace ZQ::common;

namespace ZQ {
namespace Spot {

//////////////////////////////////////////////////////////////////////////
// class SpotStatusQuery
SpotStatusQuery::SpotStatusQuery(SpotMgr& spotMgr):
	_spotMgr(spotMgr)
{

}

SpotStatusQuery::~SpotStatusQuery()
{

}

void setNodeVariant(const SpotMgr::NodeInfo& nodeInfo, 
	ZQ::common::Variant& var)
{
	char nodeId[64];
	nodeInfo.nodeId.toString(nodeId, sizeof(nodeId));
	var.set("nodeId", tstring(nodeId));
	var.set("osInfo", nodeInfo.osInfo);
	var.set("hostName", nodeInfo.hostName);
	var.set("processerDesc", nodeInfo.procDesc);
}

void setSpotVariant(const SpotMgr::SpotInfo& spotInfo, 
	ZQ::common::Variant& var)
{
	var.set("processId", spotInfo.spotKey.processId);
	var.set("application", spotInfo.application);
	var.set("endPoing", spotInfo.endPoint);
}

void setInterfaceVariant(const SpotMgr::InterfaceInfo& ifInfo, 
	ZQ::common::Variant& var, bool spotKey = false)
{
	if (spotKey) {
		var.set("NodeId", ifInfo.spotKey.nodeguid);
		var.set("processId", ifInfo.spotKey.processId);
	}
	var.set("ifName", ifInfo.ifName);
	var.set("load", ifInfo.load);
	var.set("lastUpdate", ctime(&ifInfo.lastUpdate));
}

int SpotStatusQuery::createNodeSnap(const char* nodeId OPTIONAL, 
	bool recursion, ZQ::common::Variant& nodeSnapVar)
{
	Variant itemVar;
	int vecSize;
	if (nodeId == NULL) {
		SpotMgr::NodeVector nodeVec;
		vecSize = _spotMgr.createNodeSnap(nodeVec);
		if (!vecSize)
			return 0;

		nodeSnapVar.setSize(vecSize);
		SpotMgr::NodeVector::iterator itor;
		int i = 0;
		for (itor = nodeVec.begin(); itor != nodeVec.end(); itor ++) {
			setNodeVariant(*itor, itemVar);
			if (recursion) {
				itemVar.set(INFO_FIELD_GRIDCELLS, 0);
				Variant& subItemVar = itemVar[INFO_FIELD_GRIDCELLS];
				createSpotSnap(nodeId, INVALID_PID,  false, subItemVar);
			}

			nodeSnapVar[i ++] = itemVar;
		}
		
	} else {
		vecSize = 1;
		SpotMgr::NodeInfo nodeInfo;
		if (!_spotMgr.getNode(SpotMgr::NodeId(nodeId), nodeInfo))
			return 0;

		nodeSnapVar.setSize(vecSize);
		Variant& item = nodeSnapVar[0];
		setNodeVariant(nodeInfo, item);
		if (recursion) {
			item.set(INFO_FIELD_GRIDCELLS, 0);
			Variant& subItemVar = item[INFO_FIELD_GRIDCELLS];
			createSpotSnap(nodeId, INVALID_PID,  false, subItemVar);
		}
	}

	return vecSize;

}

int SpotStatusQuery::createSpotSnap(const char* nodeId, 
		int processId OPTIONAL, bool recursion, 
		ZQ::common::Variant& spotSnapVar)
{
	int vecSize;
	if (processId == INVALID_PID) {
		SpotMgr::SpotVector spotVec;
		vecSize = _spotMgr.createSpotSnap(nodeId, spotVec);
		if (!vecSize)
			return 0;

		spotSnapVar.setSize(vecSize);

		SpotMgr::SpotVector::iterator itor;
		int i = 0;
		for (itor = spotVec.begin(); itor != spotVec.end(); itor ++) {
			Variant& itemVar = spotSnapVar[i];
			setSpotVariant(*itor, itemVar);
			if (recursion) {
				itemVar.set(INFO_FIELD_GRIDCELLS, 0);
				Variant& subItemVar = itemVar[INFO_FIELD_GRIDCELLS];
				createInterfaceSnap(nodeId, itor->spotKey.processId, 
					NULL, subItemVar);
			}
			i ++;
		}

	} else {
		vecSize = 1;
		SpotMgr::SpotInfo spotInfo;
		SpotMgr::SpotKey spotKey(processId, Guid(nodeId));
		if (!_spotMgr.getSpot(spotKey, spotInfo))
			return 0;

		spotSnapVar.setSize(vecSize);
		Variant& item = spotSnapVar[0];
		setSpotVariant(spotInfo, item);
		if (recursion) {
			item.set(INFO_FIELD_GRIDCELLS, 0);
			Variant& subItemVar = item[INFO_FIELD_GRIDCELLS];
			createInterfaceSnap(nodeId, spotInfo.spotKey.processId,  NULL, 
				subItemVar);
		}
	}

	return vecSize;
}

int SpotStatusQuery::createInterfaceSnap(const char* nodeId, 
	int processId, 
	const char* ifName OPTIONAL, 
	ZQ::common::Variant& ifSnapVar)
{
	int vecSize;
	SpotMgr::SpotKey spotKey(processId, Guid(nodeId));
	if (ifName == NULL) {		
		SpotMgr::InterfaceVector ifVec;
		vecSize = _spotMgr.createInterfaceSnap(spotKey, ifVec);
		if (vecSize == 0)
			return 0;
		ifSnapVar.setSize(vecSize);
		SpotMgr::InterfaceVector::iterator itor;
		int i = 0;
		for (itor = ifVec.begin(); itor != ifVec.end(); itor ++) {
			setInterfaceVariant(*itor, ifSnapVar[i ++]);
		}
		return true;
	} else {
		vecSize = 1;
		SpotMgr::InterfaceInfo ifInfo;
		if (!_spotMgr.findInterface(ifName, spotKey, &ifInfo))
			return 0;
		ifSnapVar.setSize(vecSize);
		Variant& item = ifSnapVar[0];
		setInterfaceVariant(ifInfo, item);
	}

	return vecSize;
}

//////////////////////////////////////////////////////////////////////////

int SpotStatusQuery::getNodeInfo(ZQ::common::Variant& nodeInfo, 
	const char* nodeId /* = NULL */)
{
	return createNodeSnap(nodeId, false, nodeInfo);
}

int SpotStatusQuery::getSpotInfo(ZQ::common::Variant& spotInfo, 
	const char* nodeId, int processId /* = INVALID_PID */)
{
	return createSpotSnap(nodeId, processId, false, spotInfo);
}

int SpotStatusQuery::getInterfaceInfo(ZQ::common::Variant& ifInfo, 
	const char* nodeId, int processId, const char* interfaceName /* = NULL */)
{
	return createInterfaceSnap(nodeId, processId, interfaceName, ifInfo);
}

int SpotStatusQuery::listInterfaces(ZQ::common::Variant& ifInfo, 
	const char* interfaceName /* = NULL */)
{
	SpotMgr::InterfaceVector ifVec;
	int vecSize = _spotMgr.createAllInterfaceSnap(interfaceName, ifVec, true);
	if (!vecSize)
		return 0;

	SpotMgr::InterfaceVector::iterator itor;
	int i = 0;
	ifInfo.setSize(vecSize);
	
	for (itor = ifVec.begin(); itor != ifVec.end(); itor ++) {
		Variant& item = ifInfo[i ++];
		setInterfaceVariant(*itor, item, true);
	}

	return vecSize;
}

bool SpotStatusQuery::sinkStatusChangeEvents(SpotStatusChangeBind* event)
{
	return _spotMgr.sinkDataChangeEvent(event);
}

bool SpotStatusQuery::unsinkStatusChangeEvents(SpotStatusChangeBind* event)
{
	return _spotMgr.unsinkDataChangeEvent(event);
}

} // namespace Spot
} // namespace ZQ
