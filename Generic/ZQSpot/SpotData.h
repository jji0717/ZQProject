// SpotData.h: interface for the SpotData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPOTDATA_H__A2BECF3A_E00F_4EA2_8818_53B171AD0A10__INCLUDED_)
#define AFX_SPOTDATA_H__A2BECF3A_E00F_4EA2_8818_53B171AD0A10__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <time.h>
#include "Variant.h"

#ifndef ZQSPOT_API
#ifdef ZQSPOT_EXPORTS
#define ZQSPOT_API __declspec(dllexport)
#else
#define ZQSPOT_API __declspec(dllimport)
#endif // ZQSPOT_EXPORTS
#endif // ZQSPOT_API

namespace ZQ {
namespace Spot {

class ZQSPOT_API SpotStatusQuery;
class ZQSPOT_API SpotStatusChangeBind;

// -----------------------------
// class SpotStatusChangeBind
// -----------------------------
/// a interface for receive the event of status changing

class SpotStatusChangeBind
{
public:
	virtual void onNodeAdded(const std::string& nodeId)	{}
	virtual void onNodeRemoved(const std::string& nodeId) {}
	virtual void onNodeChanged(const std::string& nodeId) {}

	virtual void onSpotAdded(const std::string& nodeId, int pid) {}
	virtual void onSpotRemoved(const std::string& nodeId, int pid) {}
	virtual void onSpotChanged(const std::string& nodeId, int pid) {}

	virtual void onInterfaceAdded(const std::string& nodeId, int pid, 
		const std::string& ifName) {}
	virtual void onInterfaceRemoved(const std::string& nodeId, int pid, 
		const std::string& ifName) {}
	virtual void onInterfaceChanged(const std::string& nodeId, int pid, 
		const std::string& ifName) {}
};

#define INVALID_PID					0
#define INFO_FIELD_GRIDCOLNAME		"GridColName"
#define INFO_FIELD_GRIDCELLS		"GridCells"

class SpotMgr;

// -----------------------------
// class SpotStatusQuery
// -----------------------------
/// a interface for querying spot status

class SpotStatusQuery {
	friend class SpotEnv;
protected:
	SpotStatusQuery(SpotMgr& spotMgr);
	virtual ~SpotStatusQuery();

public:

	/// get the node desc information that the current spot environment 
	/// has already collected
	///@param[out]	nodeInfo an array of node description info
	///@param[in]	nodeId the Guid of the node about to be queried, 
	///				NULL means return all the nodes
	///@return		the count of the node info
	///@remark every node desc info is a struct-ed variant that contains two part:
	/// key=value pairs
	/// nodeinfo[INFO_FIELD_GRIDCOLNAME]
	/// nodeinfo[INFO_FIELD_GRIDCELLS] => a 2d array of the cells
	int getNodeInfo(ZQ::common::Variant& nodeInfo, const char* nodeId = NULL);

	/// get the spot desc information that the current spot environment 
	/// has already collected
	///@param[out]	nodeInfo	an array of spot description info
	///@param[in]	nodeId		the Guid of the node about to be queried, 
	///							NULL means return all the nodes
	///@param[in]	processId	the process id of the spot
	///@return		the count of the spot info
	///@remark every node desc info is a struct-ed variant that contains two part:
	/// key=value pairs
	/// nodeinfo[INFO_FIELD_GRIDCOLNAME]
	/// nodeinfo[INFO_FIELD_GRIDCELLS] => a 2d array of the cells
	int getSpotInfo(ZQ::common::Variant& spotInfo, const char* nodeId, 
		int processId = INVALID_PID);
	
	/// get the interface desc information that the current spot environment 
	/// has already collected
	///@param[out]	ifInfo		an array of interface description info
	///@param[in]	nodeId		the Guid of the node about to be queried, 
	///@param[in]	processId	the process id of the spot
	///							INVALID_PID means return all the processes
	///@param[in]	interfaceName	the name of the interface. 
	///								NULL means return all the interfaces
	///@return		the count of the interface info
	///@remark every node desc info is a struct-ed variant that contains two part:
	/// key=value pairs
	/// nodeinfo[INFO_FIELD_GRIDCOLNAME]
	/// nodeinfo[INFO_FIELD_GRIDCELLS] => a 2d array of the cells
	int getInterfaceInfo(ZQ::common::Variant& ifInfo, const char* nodeId, 
		int processId, const char* interfaceName = NULL);

	/// list the information of interface
	///@param[out]	ifInfo		an array of interface description info
	///@param[in]	interfaceName	the name of the interface. 
	///								NULL means return all the interfaces
	///@return		the count of the interface info
	///@remark every node desc info is a struct-ed variant that contains two part:
	/// key=value pairs
	/// nodeinfo[INFO_FIELD_GRIDCOLNAME]
	/// nodeinfo[INFO_FIELD_GRIDCELLS] => a 2d array of the cells
	int listInterfaces(ZQ::common::Variant& ifInfo, 
		const char* interfaceName = NULL);

	/// subscrbe the event status changing
	///@param[in]	event		a binding of status changing
	///@return		If the function succeeds, the return value is true.
	///				If the function fails, the return value is false.
	bool sinkStatusChangeEvents(SpotStatusChangeBind* event);

	/// cancel the subscribing
	///@param[in]	event		a binding of status changing
	///@return		if the function succeeds, the return value is true.
	///				if the function fails, the return value is false.
	bool unsinkStatusChangeEvents(SpotStatusChangeBind* event);

protected:
	int createNodeSnap(const char* nodeId OPTIONAL, 
		bool recursion, ZQ::common::Variant& var);

	int createSpotSnap(const char* nodeId, 
		int processId OPTIONAL, bool recursion, 
		ZQ::common::Variant& var);

	int createInterfaceSnap(const char* nodeId, 
		int processId, 
		const char* interfaceName OPTIONAL, 
		ZQ::common::Variant& var);

protected:
	SpotMgr&		_spotMgr;
};

} // namespace Spot
} // namespace ZQ

#endif // !defined(AFX_SPOTDATA_H__A2BECF3A_E00F_4EA2_8818_53B171AD0A10__INCLUDED_)
