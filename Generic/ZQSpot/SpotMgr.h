// SpotMgr.h: interface for the SpotMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPOTMGR_H__2D9FE4D8_2E6B_417F_939E_47C0A43964C5__INCLUDED_)
#define AFX_SPOTMGR_H__2D9FE4D8_2E6B_417F_939E_47C0A43964C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZQ_common_conf.h"
#include "Guid.h"
/*
#include "UDPSocket.h"
#include "InetAddr.h"
#include "Variant.h"
#include "NativeThreadPool.h"
*/
#include "ZQSpotIce.h"

#include "rwlock.h"

namespace ZQ {
namespace Spot {

class SpotStatusChangeBind;
typedef SpotStatusChangeBind SpotDataEvent;

class SpotMgr
{
public:
	SpotMgr();
	virtual ~SpotMgr();

	bool init();
	void uninit();

public:

	//////////////////////////////////////////////////////////////////////////
	// typedef

	typedef ZQ::common::Guid NodeId;

	// key to index the ZQ Spot
	struct SpotKey: public ZqSpotIce::SpotKey
	{
		SpotKey()
		{

		}

		SpotKey(const ZqSpotIce::SpotKey& spotKey)
		{
			processId = spotKey.processId;
			nodeguid = spotKey.nodeguid;
		}

		SpotKey(int pid, const std::string& nid)
		{
			processId = pid;
			nodeguid = nid;
		}

		SpotKey(int pid, const NodeId& nid)
		{
			processId = pid;
			char id[128];
			nid.toString(id, sizeof(id));
			nodeguid = id;
		}

		bool operator==(const SpotKey& spotKey) const
		{
			return nodeguid == spotKey.nodeguid && 
				processId == spotKey.processId;
		}

		const SpotKey& operator=(const SpotKey& spotKey)
		{
			nodeguid = spotKey.nodeguid;
			processId = spotKey.processId;
			return *this;
		}

		bool operator<(const SpotKey& spotKey) const
		{
			if (processId == spotKey.processId) {
				return nodeguid < spotKey.nodeguid;
			} else
				return processId < spotKey.processId;
		}

	};

	// Information about an interface registered on remote spot
	typedef struct _InterfaceInfo
	{
		// the name of infterface
		std::string ifName;		// key
		
		// the key to SpotInfo
		SpotKey		spotKey;
		
		// a load value ranged within 0-10000
		//    0     - highly available,
		//    10000 - fully allocated, 10000+ means unavailable
		int			load;
		
		// the timestamp of last load update
		time_t		lastUpdate;

	} InterfaceInfo;

	typedef std::multimap<std::string, InterfaceInfo> InterfaceMap;
	typedef std::pair<InterfaceMap::iterator, 
		InterfaceMap::iterator> interface_range;
	
	// Information about a server application or process
	typedef struct _SpotInfo
	{
		// the unique key
		SpotKey		spotKey;	// key
		
		// the name of the executable
		std::string application;

		// end point of the spot
		std::string	endPoint;
		
		// the timestamp of last spotinfo update
		time_t		updateTime;
		
		// the query interface that also that contains the 
		// default adapter endpoint. 
		ZqSpotIce::SpotQueryPrx spotQuery;
	} SpotInfo;
	
	typedef std::map<SpotKey, SpotInfo> SpotMap;

	// Information about an &IceGrid; node.
	typedef struct _NodeInfo
	{
		// the unique key: guid of the node
		NodeId		nodeId;		// key
		
		// The operating system name and version
		std::string osInfo;
		
		// the network name of the node
		std::string hostName;
		
		// The description of processors.
		std::string procDesc;
	
	} NodeInfo;

	typedef std::map<NodeId, NodeInfo> NodeMap;


//////////////////////////////////////////////////////////////////////////
// methods
public:
	bool addNode(const NodeInfo& info);
	bool delNode(const NodeId& nodeId);
	bool updateNode(const NodeInfo& info);

	bool addSpot(const SpotInfo& info);
	bool delSpot(const SpotKey& spotKey);
	bool delSpot(const NodeId& nodeId);
	bool updateSpot(const SpotInfo& info);

	bool addInterface(const InterfaceInfo& ifInfo);
	bool delInterface(const std::string& ifName, 
		const SpotKey& spotKey);
	bool delInterface(const SpotKey& spotKey);
	bool updateInterface(const InterfaceInfo& ifInfo);

	int findInterface(const std::string& ifName, 
		std::vector<InterfaceInfo >& result);
	int findInterface(const SpotKey& spotKey, 
		std::vector<InterfaceInfo >& result);
	bool findInterface(const std::string& ifName, const SpotKey& spotKey, 
		InterfaceInfo* info);

	bool getSpot(const SpotKey& spotKey, SpotInfo& info);
	int findSpotByAppName(const std::string& appName, 
		std::vector<SpotInfo >& result) const;
	bool getNode(const NodeId& nodeId, NodeInfo& info) const;

	typedef std::vector<NodeInfo> NodeVector;
	typedef std::vector<SpotInfo> SpotVector;
	typedef std::vector<InterfaceInfo> InterfaceVector;
	int createNodeSnap(NodeVector& nodeVec) const;
	int createSpotSnap(const NodeId& nodeId, SpotVector& spotVec);
	int createInterfaceSnap(const SpotKey& spotKey, 
		InterfaceVector& ifVec);
	int createAllInterfaceSnap(
		const char* ifName OPTIONAL, 
		InterfaceVector& ifVec, 
		bool nameUnique) const;

	bool sinkDataChangeEvent(SpotDataEvent* event);
	bool unsinkDataChangeEvent(SpotDataEvent* event);

protected:
	bool _getSpot(const SpotKey& spotKey, SpotInfo& info, bool& erased);	
	bool checkSpotTimeout(const SpotInfo& nodeInfo) const;

protected:
	NodeMap					_nodeMap;
	SpotMap					_spotMap;
	InterfaceMap			_ifMap;
	
	mutable XRWLock			_nodeMapLock;
	mutable XRWLock			_spotMapLock;
	mutable XRWLock			_ifMapLock;

	typedef std::vector<SpotDataEvent* > SpotDataEventVec;
	SpotDataEventVec		_dataEventVec;

	time_t					_spotTimeo;
};

} // namespace ZQ {
} // namespace Spot {

#endif // !defined(AFX_SPOTMGR_H__2D9FE4D8_2E6B_417F_939E_47C0A43964C5__INCLUDED_)
