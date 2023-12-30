#ifndef __SpotAgent_H__
#define __SpotAgent_H__

#include "ZQ_common_conf.h"
#include "Guid.h"
#include "InetAddr.h"
#include "UDPSocket.h"
#include "Variant.h"
#include "NativeThreadPool.h"
#include "NativeThread.h"
#include <map>
#include <Ice/Ice.h>
#include "SpotMgr.h"
#include "ZQSpot.h"

#define DEFAULT_HEARTBEAT_INTV  10 // 2 sec
// #define DEBUG_ECHO

#define NAMESPACE_BEGIN namespace ZQ {	namespace Spot {
#define NAMESPACE_END   }}

//////////////////////////////////////////////////////////////////////////

NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////

class NodeSpot;

// -----------------------------
// class NodeHelper
// -----------------------------
/// A helper object to handle the node events

class NodeHelper
{
	friend class NodeBcastProcess;
public:

	NodeHelper(NodeSpot& node);
	virtual ~NodeHelper();

	typedef enum _Verb
	{
		UNKNOWN = 0,
		POST,
		RESP,
	} Verb;

	typedef struct _MessageInfo
	{
		ZQ::common::InetHostAddress  _saddr;
		int							 _sport;
		ZQ::common::InetMcastAddress _daddr;
		int							 _dport;
		int                          _datalen;
		char*                        _data;
	} MessageInfo;

protected:
	
	/// this entry will be called every time when a group message is called
	///@param[in] verb the verb of the message, i.e. POST as a request or notification, RESP as a response
	///@param[in] methodname the request method name or the notification name
	///@param[in] params the input params of the request the notification
	///@param[in] pMsg pointer to the MessageInfo struct for connection information
	///@param[in] localCtx the string desriptor to the local message context
	///@param[in] soureCtx the string desriptor to the remote message context
	///@param[in] sourePid the process Id to local message context
	///@return false if no other helpers need to be notified
	virtual bool OnBroadcast(const Verb verb, const char* methodname, 
		const char* nodeId, const char* appName, const char* endPoint, 
		ZQ::common::Variant& params, const MessageInfo* pMsgInfo, 
		const ZQ::common::InetHostAddress& source, 
		const char* localCtx=NULL, const char* sourceCtx = NULL, 
		const DWORD srcPid = 0);

	NodeSpot& _node;
};

// -----------------------------
// class NodeSpot
// -----------------------------
/// a node agent to the specify node group
class NodeSpot
{
	friend class NodeBcastProcess;
	friend class NodeBcastSniffer;
	friend class NodeHelper;
	friend class NodeWatchDog;

public:

	///constructor
	///@param Pool the thread pool where this node spot run in
	///@param groupAddress the multicast IP address that the node group notice each other
	///@param bindAddress the local address that this node bind on
	///@param nodeId       the global unqiue id to identify this node
	NodeSpot(ZQ::common::NativeThreadPool&		Pool,
			const ZQ::common::InetMcastAddress&	groupAddress,
			unsigned short						groupPort, 
			const ZQ::common::InetHostAddress&	bindAddress,			
			const ZQ::common::Guid				nodeId, 
			std::string							appName,
			int									processId, 
			std::string							endPoint,
			SpotEnv&							spotFrame);

	///destructor
	virtual ~NodeSpot();
	
	// broadcat post to the node group
	bool groupPost(const char* methodname, const char* nodeId, 
		ZQ::common::Variant& params, const char* remoteCtx = NULL, 
		const char* localCtx= NULL, const int remotePid=0);

protected:
	ZQ::common::Guid				_nodeid;
	std::string						_appName;
	std::string						_endPoint;
	ZQ::common::InetMcastAddress	_addrGroup;
	ZQ::common::InetHostAddress		_addrCtrlBind;
	ZQ::common::NativeThreadPool&	_thpool;

	int								_portHeatbeat;
	ZQ::common::UDPMulticast*		_pSock;
	NodeBcastSniffer*				_pSniffer;
	NodeWatchDog*					_pWatchDog;
	DWORD							_pid;

	typedef std::vector < NodeHelper* > Helpers;
	Helpers _helpers;
	ZQ::common::Mutex _lockHelpers;

	void regHelper(NodeHelper& helper);
	void unregHelper(NodeHelper& helper);
};

NAMESPACE_END

#endif // __SpotAgent_H__