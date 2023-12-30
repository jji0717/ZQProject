#include "ZQSpotAgent.h"

#include "getopt.h"
#include <strstream>   // for sstream
#include "XMLRpcSerializer.h"

NAMESPACE_BEGIN

using namespace ZQ::common;

#define UDP_MAX_BUFFER_SIZE (64*1024)

/* basic message format
1) Request:
	POST <method> <Protocol>
	Source-Context: <Id>
	Dest-Context: <Id>

	<XMLRPC data>

2) Response:
	RESP <method> <Protocol>
	Source-Context: <Id>
	Dest-Context: <Id>

	<XMLRPC data>
*/

#define METHOD_NODE_HEATBEAT	"MPF::NodeSpot::Heartbeat"
#define HEADER_FIELD_NODEID		"Node-Identity"
#define HEADER_FIELD_APPNAME	"Application-Name"
#define HEADER_FIELD_ENDPOINT	"End-Point"
#define HEADER_FIELD_SRC_CTX	"Source-Context"
#define HEADER_FIELD_SRC_PID	"Source-PID"
#define HEADER_FIELD_DEST_CTX	"Dest-Context"
#define HEADER_FIELD_DEST_PID	"Dest-PID"
#define HEADER_FIELD_ORDINAL	"Ordinal"
#define HEADER_FIELD_CTNT_LEN	"Content-Length"

#pragma comment(lib, "ws2_32.lib")

// -----------------------------
// class NodeBcastProcess
// -----------------------------
/// Thread NodeBcastProcess launched by McastSniffer to read from message queue then
/// fire event to McastSubscribers
class NodeBcastProcess : public ZQ::common::ThreadRequest
{
	friend class NodeBcastSniffer;
	
protected:
	
	NodeHelper::MessageInfo		_msg;
	
	NodeBcastSniffer&			 _sniffer;
	
	// accept access only from NodeBcastSniffer
	NodeBcastProcess(NodeBcastSniffer& sniffer,
		const void* data, const int datalen,
		const ZQ::common::InetMcastAddress& group, const int gport, 
		const ZQ::common::InetHostAddress& source, int sport);
	
public:
	
	virtual ~NodeBcastProcess();
	
protected:
	
	/// primary processes to read from the queue then calls the subscribers
	virtual int run();
	
	virtual void final(int retcode =0, bool bCancelled =false)
	{
		delete this;
	}
	
};

// -----------------------------
// class NodeBcastSniffer
// -----------------------------
/// control class to sniff multicast traffic, support multiple group/port combinations.
class NodeBcastSniffer : public ZQ::common::ThreadRequest
{
	friend class NodeBcastProcess;

public:
	/// constructor
	NodeBcastSniffer(NodeSpot& node);
	/// destructor
	~NodeBcastSniffer();

	/// open a group/port of multicast traffic interested
	///@param group       the interest multicast IP address
	///@param group_port  the interest multicast port number
	///@param bind        the local IP address to bind the listener
	///@param pSubscriber pointer to the subcsriber that interest this traffic
	///@return            true if open succesfully
	bool open(ZQ::common::InetMcastAddress group, int group_port, ZQ::common::InetHostAddress bind);

	void stop();

protected:

	class recv_t
	{
	public:
		ZQ::common::UDPReceive       recv;   // the UDP receiver instance
		ZQ::common::InetMcastAddress group;
		int                          gport;
		ZQ::common::InetHostAddress  bind;
		char						 openTime[32]; // when this listener is opened
		
		recv_t(ZQ::common::InetMcastAddress mgroup, int group_port, ZQ::common::InetHostAddress bind_addr)
			: group(mgroup), gport(group_port), bind(bind_addr), recv(bind_addr, group_port)
		{
		}
	};

	typedef std::vector < recv_t* > recvs_t;
	recvs_t _recvs;
	ZQ::common::Mutex _recvsLock;

	bool _bQuit;
	HANDLE _hWakeUp;
	
	char _recvbuf[UDP_MAX_BUFFER_SIZE];

	NodeSpot& _node;

public:

	typedef recvs_t::const_iterator iterator;

	// browse the active listeners
	/// get the first listener iterator
	iterator begin()  const { return _recvs.begin(); }

	/// get the last listener iterator
	iterator end()    const { return _recvs.end(); }

	/// get the count of active listeners
	size_t   size()   const { return _recvs.size(); }

protected:

	/// the primary process to receive traffic on multiple interfaces
	virtual int run();
};

// -----------------------------
// class NodeWatchDog
// -----------------------------
class NodeWatchDog: public NodeHelper, public ZQ::common::ThreadRequest
{
public:

	NodeWatchDog(NodeSpot& node, SpotEnv& spotFrame, 
		const DWORD intervalHeartbeat);
	virtual ~NodeWatchDog();

	void stop();

protected:
	
	///@return false if no other helper needs to be notified
	virtual bool OnBroadcast(const Verb verb, const char* methodname, 
		const char* nodeId, const char* appName, const char* endPoint, 
		ZQ::common::Variant& params, 
		const MessageInfo* pMsgInfo, 
		const ZQ::common::InetHostAddress& source, 
		const char* localCtx = NULL, const char* sourceCtx = NULL, 
		const DWORD srcPid = 0);

	virtual bool init();
	virtual int run();

	bool _bQuit;
	HANDLE _hWakeUp;
	int _intvHeartbeat; // in msec

	ZQ::common::Variant		_sysInfo;
	SpotEnv&			_spotFrame;
};

// -----------------------------
// class NodeBcastSniffer
// -----------------------------
NodeBcastSniffer::NodeBcastSniffer(NodeSpot& node)
	:_bQuit(false), _node(node), ThreadRequest(node._thpool)
{
	_hWakeUp = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	_bQuit = (_hWakeUp ==NULL);
}

NodeBcastSniffer::~NodeBcastSniffer()
{
	if (!_bQuit)
		stop();

	ZQ::common::MutexGuard guard(_recvsLock);
	for ( recvs_t::iterator it= _recvs.begin( ); it != _recvs.end( ); it++ )
	{
		recv_t* pRecv = *it;
		if (pRecv !=NULL)
			delete pRecv;
	}
	_recvs.clear();

	if (_hWakeUp !=NULL)
		::CloseHandle(_hWakeUp);
	_hWakeUp =NULL;
}

bool NodeBcastSniffer::open(ZQ::common::InetMcastAddress group, int group_port, ZQ::common::InetHostAddress bind)
{
	if (_bQuit)
		return false;

	recv_t* newRecv =NULL;
	
	try
	{
		newRecv = new recv_t(group, group_port, bind);

		if (newRecv ==NULL)
			return false;

		newRecv->recv.setMulticast(true);

		if (newRecv->recv.join(group) != ZQ::common::Socket::errSuccess)
		{
			delete newRecv;
			return false;
		}

		newRecv->recv.setCompletion(true); // make the socket blockable
	}
	catch(...)
	{
		return false;
	}

	SOCKET so = newRecv->recv.getReceiver();

	if (so == INVALID_SOCKET)
	{
		delete newRecv;
		return false;
	}
	
	SYSTEMTIME SystemTime ;
	GetSystemTime(&SystemTime);
	sprintf(newRecv->openTime,
		"%4d-%2d-%2d %2d:%2d:%2d",
		SystemTime.wYear,
		SystemTime.wMonth,
		SystemTime.wDay,
		SystemTime.wHour,
		SystemTime.wMinute,
		SystemTime.wSecond);

	ZQ::common::MutexGuard guard(_recvsLock);
	_recvs.push_back(newRecv);
	::SetEvent(_hWakeUp);

	return true;
}

void NodeBcastSniffer::stop()
{
	_bQuit = true;
	if (_hWakeUp != NULL)
		::SetEvent(_hWakeUp);

	::Sleep(100);
}

int NodeBcastSniffer::run()
{
	ZQ::common::InetHostAddress from;
	int sport;

	while (!_bQuit) // TODO: ScThreadPool doesn't support thread::terminate
	{
		struct timeval timeout;
		fd_set inp, err;
		SOCKET so;

		FD_ZERO(&inp);
		FD_ZERO(&err);

		int maxFD =0, nFD =0;

		// fill in the FD sets
		for ( recvs_t::iterator it= _recvs.begin( ); it != _recvs.end( ); it++ )
		{
			recv_t* pRecv = *it;
			if (pRecv == NULL)
				continue;

			so = pRecv->recv.getReceiver();
			if (so == INVALID_SOCKET)
				continue;

			FD_SET(so, &inp);
			FD_SET(so, &err);
			maxFD = (maxFD >= (int) so) ? maxFD : so;
			++nFD;
		}

		if (_bQuit)
			return 0;

		// if there is no valid FD yet, wait for open() to put one in
		if (nFD <=0 || maxFD<=0)
		{
			if (WAIT_FAILED == ::WaitForSingleObject(_hWakeUp, INFINITE))
				_bQuit = true;
			else
				::ResetEvent(_hWakeUp);

			if (_bQuit)
				return 0;

			continue;
		}

		// wait for 2 sec maximal 
		timeout.tv_sec  = 2;
		timeout.tv_usec = 0;

		int fn =::select(maxFD, &inp, NULL, &err, &timeout);

		if (fn >0)
		{
			for ( recvs_t::iterator it= _recvs.begin( ); !_bQuit && it != _recvs.end( ); it++ )
			{
				recv_t* pRecv = *it;
				if (pRecv == NULL)
					continue;

				so = pRecv->recv.getReceiver();
				if (so == INVALID_SOCKET)
					continue;

				if(FD_ISSET(so, &err))
				{
					//TODO: remove this node from _recvs
					continue;
				}

				if (FD_ISSET(so, &inp))
				{
					int len = pRecv->recv.receiveFrom(_recvbuf, UDP_MAX_BUFFER_SIZE, from, sport);
#ifndef DEBUG_ECHO
					if (InetHostAddress::getLocalAddress() == from)
						continue;
#endif
					if (len >0)
					{
						NodeBcastProcess* proc = new NodeBcastProcess(*this, _recvbuf, len, (*it)->group, (*it)->gport, from, sport);
						if (NULL != proc)
							proc->start();
					}
				}
			}
		}

	}

	_bQuit = true;
	return 0;
}

// -----------------------------
// class NodeBcastProcess
// -----------------------------
NodeBcastProcess::NodeBcastProcess(NodeBcastSniffer& sniffer,
		const void* data, const int datalen,
		const ZQ::common::InetMcastAddress& group, const int gport, 
		const ZQ::common::InetHostAddress& source, int sport)
: ThreadRequest(sniffer._node._thpool), _sniffer(sniffer)
//  _msg._data(NULL), _msg._datalen(0), _msg._sport(0), _msg._dport(0) // , _bQuit(false), _maxQueueSize(DEFAULT_QUEUE_SIZE)
{
	if (data ==NULL || datalen<=0 || datalen > UDP_MAX_BUFFER_SIZE)
		return;

//	if (_msgQueue.size() >= _maxQueueSize )
//		return;

	_msg._saddr = source;
	_msg._sport = sport;
	_msg._daddr = group;
	_msg._dport = gport;
	_msg._datalen = datalen;
	_msg._data = new char[_msg._datalen];

	if (_msg._data == NULL)
		return;

	memcpy(_msg._data, data, _msg._datalen);
}

NodeBcastProcess::~NodeBcastProcess()
{
	if (_msg._data != NULL)
		delete [] _msg._data;
}

int NodeBcastProcess::run()
{
	if (_msg._data == NULL || _msg._datalen<=0)
		return 0;

	char *p = _msg._data;
	char s1[40], s2[256], s3[40];

	// parse the command line
	char *q =strstr(p, "\r\n");
	if (q>p)
	{
		*q = *(q+1) ='\0';
		sscanf(p, "%[^ ]%*[ \t]%[^ ]%*[ \t]%[^ ]", s1, s2, s3);
		*q = '\r'; *(q+1) ='\n';
		p = q+2;
	}

	NodeHelper::Verb verb = NodeHelper::UNKNOWN;

	if (0 == stricmp(s1, "POST"))
		verb = NodeHelper::POST;
	else if (0 == stricmp(s1, "RESP"))
		verb = NodeHelper::RESP;

	if (NodeHelper::UNKNOWN == verb)
		return 0;

	std::string method = s2;
	std::string prot = s3;

	std::string srcCtx;
	std::string destCtx;
	std::string ordinal;
	std::string nodeId;
	std::string appName;
	std::string endPoint;

	int         contentLen = 0;
	DWORD         srcPid =0;
	DWORD         destPid =0;

	while (p - _msg._data < _msg._datalen)
	{
		// parse each header line
		q =strstr(p, "\r\n");
		if (p == q)
		{
			p +=2;
			break;
		}
		*q = *(q+1) ='\0';
		if (2 == sscanf(p, "%[^:]:%[^\0]", s1, s2) || 2 == 
			sscanf(p, "%[^ ]%*[ \t]:%[^\0]", s1, s2))
		{
			// trim front of the header value
			char *value = s2;
			while (*value && (*value ==' ' || *value =='\t'))
				value++;;
			
			if (0 ==stricmp(s1, HEADER_FIELD_SRC_CTX))
				srcCtx = value;
			else if (0 ==stricmp(s1, HEADER_FIELD_SRC_PID))
				srcPid = atol(value);
			else if (0 ==stricmp(s1, HEADER_FIELD_NODEID))
				nodeId = value;
			else if (0 ==stricmp(s1, HEADER_FIELD_APPNAME))
				appName = value;
			else if (0 ==stricmp(s1, HEADER_FIELD_ENDPOINT))
				endPoint = value;
			else if (0 ==stricmp(s1, HEADER_FIELD_DEST_CTX))
				destCtx = value;
			else if (0 ==stricmp(s1, HEADER_FIELD_DEST_PID))
				destPid = atol(value);
			else if (0 ==stricmp(s1, HEADER_FIELD_ORDINAL))
				ordinal = value;
			else if (0 ==stricmp(s1, HEADER_FIELD_CTNT_LEN))
				contentLen = atol(value);
		}
		*q = '\r'; *(q+1) ='\n';
		p = q+2;
	}

	if (destPid!=0 && destPid != _sniffer._node._pid)
		return 0;

	if (contentLen <=0 || (_msg._data + _msg._datalen -p) < contentLen)
		contentLen = (_msg._data + _msg._datalen -p);

	ZQ::common::Variant params;
	/*
	try
	{
		std::strstream ss(p, contentLen);
		ZQ::XmlRpc::XmlRpcUnserializer us(params, ss);
		us.unserialize();
	}
	catch(ZQ::common::Exception e)
	{
		std::cout << e.getString() <<std::endl;
	}
	*/

	NodeSpot::Helpers& helpers = _sniffer._node._helpers;
	ZQ::common::MutexGuard gd(_sniffer._node._lockHelpers);
	for (NodeSpot::Helpers::iterator it = helpers.begin(); it<helpers.end(); it++)
	{
		if (NULL != *it && !(*it)->OnBroadcast(verb, method.c_str(), 
			nodeId.c_str(), appName.c_str(), endPoint.c_str(), params, 
			&_msg, _msg._saddr, destCtx.c_str(), srcCtx.c_str(), srcPid))
			return 0;
	}

	return 0;
}

// -----------------------------
// class NodeSpot
// -----------------------------
NodeSpot::NodeSpot(ZQ::common::NativeThreadPool& Pool,
			const ZQ::common::InetMcastAddress&	groupAddress,
			unsigned short						groupPort, 
			const ZQ::common::InetHostAddress&	bindAddress,			
			const ZQ::common::Guid				nodeId, 
			std::string							appName, 
			int									processId, 
			std::string							endPoint,
			SpotEnv&						spotFrame): 
	_thpool(Pool), _pSniffer(NULL), _pSock(NULL), _pWatchDog(NULL), 
	_portHeatbeat(groupPort), _addrGroup(groupAddress), 
	_addrCtrlBind(bindAddress), _nodeid(nodeId), _appName(appName), 
	_endPoint(endPoint)
{
	_pid = processId;

	_pSock = new ZQ::common::UDPMulticast(_addrCtrlBind, 0);
	if (NULL != _pSock)
		_pSock->setGroup(_addrGroup, groupPort);
	
	_pSniffer = new NodeBcastSniffer(*this);
	if (_pSniffer)
	{
		_pSniffer->open(_addrGroup, groupPort, _addrCtrlBind);	
		_pSniffer->start();
	}

	_pWatchDog = new NodeWatchDog(*this, spotFrame, DEFAULT_HEARTBEAT_INTV);
	if (NULL != _pWatchDog)
		_pWatchDog->start();
}

NodeSpot::~NodeSpot()
{
	if (_pSniffer)
	{
		_pSniffer->stop();
		delete _pSniffer;
	}
	_pSniffer=NULL;
	
	if (NULL == _pSock)
		delete _pSock;
	_pSock=NULL;
	
	if (NULL == _pWatchDog)
		delete _pWatchDog;
	_pWatchDog=NULL;
}

void NodeSpot::regHelper(NodeHelper& helper)
{
	ZQ::common::MutexGuard gd(_lockHelpers);
	for (Helpers::iterator it = _helpers.begin(); it<_helpers.end(); it++)
	{
		if (*it == &helper)
			return;
	}
	_helpers.push_back(&helper);
}

void NodeSpot::unregHelper(NodeHelper& helper)
{
	ZQ::common::MutexGuard gd(_lockHelpers);
	for (Helpers::iterator it = _helpers.begin(); it<_helpers.end(); it++)
	{
		if (*it == &helper)
			_helpers.erase(it);
	}
}

bool NodeSpot::groupPost(const char* methodname, const char* nodeId, 
	ZQ::common::Variant& params, const char* remoteCtx, 
	const char* localCtx, const int remotePid)
{
	if (NULL == _pSock || NULL == methodname)
		return false;
	
	char header[512];
	char* msg = NULL;

	{
		/*
		std::strstream ss;
		{
			ZQ::XmlRpc::XmlRpcSerializer s(params, ss);
			s.serialize();
		}
		*/
		
		sprintf(header, "POST %s MPF/0.1\r\n"
				HEADER_FIELD_NODEID ": %s\r\n"
				HEADER_FIELD_APPNAME ": %s\r\n"
				HEADER_FIELD_ENDPOINT ": %s\r\n"
			    HEADER_FIELD_DEST_CTX ": %s\r\n" HEADER_FIELD_DEST_PID ": %d\r\n" 
				HEADER_FIELD_SRC_CTX ": %s\r\n" HEADER_FIELD_SRC_PID ": %d\r\n" 
				HEADER_FIELD_CTNT_LEN ": %d\r\n\r\n", 
			    methodname, nodeId, _appName.c_str(), _endPoint.c_str(), 
				(NULL !=remoteCtx ? remoteCtx :""), remotePid,
			    (NULL !=localCtx ? localCtx:""), _pid,
				0 /* ss.pcount() */);
		
		//msg = new char[strlen(header) + ss.pcount()+2];
		//char *p = msg;

		/*		
		if (NULL != p)
		{
			strcpy(p, header);
			p+=strlen(p);
			strncpy(p, ss.str(), ss.pcount());
			p+=strlen(p);
			*p=0;
		}
		*/
	}
	
	//if (NULL != msg)
	{
//		printf("%s\n", msg);
		_pSock->send(header, strlen(header));
		//delete [] msg;
	}
	
	return true;
}

// -----------------------------
// class NodeHelper
// -----------------------------
NodeHelper::NodeHelper(NodeSpot& node)
: _node(node)
{
	_node.regHelper(*this);
}

NodeHelper::~NodeHelper()
{
	_node.unregHelper(*this);
}

// -----------------------------
// class NodeWatchDog
// -----------------------------
NodeWatchDog::NodeWatchDog(NodeSpot& node, SpotEnv& spotFrame, 
						   const DWORD intervalHeartbeat): 
	NodeHelper(node), ThreadRequest(node._thpool), 
	_spotFrame(spotFrame), _bQuit(false), _hWakeUp(NULL), 
	_intvHeartbeat(intervalHeartbeat)
{
	_hWakeUp = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	_bQuit = (_hWakeUp ==NULL);
}

NodeWatchDog::~NodeWatchDog()
{
	if (!_bQuit)
		stop();
}

void NodeWatchDog::stop()
{
	_bQuit = true;
	if (_hWakeUp != NULL)
		::SetEvent(_hWakeUp);
	
	::Sleep(100);
}

extern bool analyseEndpoint(const char endpoint[], char* addr, 
	int* addrlen, unsigned short& port);

bool NodeWatchDog::OnBroadcast(const Verb verb, 
							   const char* methodname, 
							   const char* nid, 
							   const char* appName, 
							   const char* endPoint, 
							   ZQ::common::Variant& params, 
							   const MessageInfo* pMsgInfo, 
							   const InetHostAddress& source, 
							   const char* localCtx, 
							   const char* sourceCtx, 
							   const DWORD srcPid)
{
	if (POST != verb || 0 != stricmp(methodname, METHOD_NODE_HEATBEAT))
		return true;

	SpotMgr& spotMgr = *_spotFrame.getSpotMgr();
	Guid nodeId(nid);
	SpotMgr::NodeInfo nodeInfo;

	// discover a new node
	if (!spotMgr.getNode(nodeId, nodeInfo)) {
		nodeInfo.nodeId = nodeId;
		nodeInfo.hostName = "unavailable";
		nodeInfo.osInfo = "unavailable";
		nodeInfo.procDesc = "unavailable";
		bool r = spotMgr.addNode(nodeInfo);
		assert(r);
		printf("node(nid: %s) added.\n", nid);
	}

	SpotMgr::SpotInfo spotInfo;
	SpotMgr::SpotKey spotKey(srcPid, nodeId);
	if (!spotMgr.getSpot(spotKey, spotInfo)) {
		spotInfo.spotKey = spotKey;
		spotInfo.application = appName;
		spotInfo.endPoint = endPoint;
		time(&spotInfo.updateTime);
		spotInfo.spotQuery = ZqSpotIce::SpotQueryPrx::checkedCast(
			_spotFrame.getSpotQueryFromEndPoint(endPoint));
		bool r = spotMgr.addSpot(spotInfo);
		assert(r);
		printf("spot(nid: %s, pid: %d) added.\n", nid, srcPid);

		std::vector<std::string> ifNames;
		std::vector<std::string>::iterator nameIt;
		bool listOk = false;
		try {
			spotInfo.spotQuery->listAllInterface(ifNames);
			listOk = true;
		} catch(Ice::Exception& e) {
			std::cerr << "NodeWatchDog::OnBroadcast() " << e << std::endl;
		}

		if (listOk) {
			SpotMgr::InterfaceInfo ifInfo;
			for (nameIt = ifNames.begin(); nameIt != ifNames.end(); 
				nameIt++ ) {

				ifInfo.ifName = *nameIt;
				ifInfo.spotKey = spotKey;
				ifInfo.load = spotInfo.spotQuery->getInterfaceLoad(
					ifInfo.ifName);
				time(&ifInfo.lastUpdate);
				bool r = spotMgr.addInterface(ifInfo);
				spotInfo.spotQuery->subscribeLoadChange(
					ZqSpotIce::LoadBindPrx::checkedCast(
						_spotFrame.getLoadBind()), 
					ifInfo.ifName, 100);
				assert(r);
				printf("interface(nid: %s, pid: %d ifname: %s) added.\n", 
					nid, srcPid, ifInfo.ifName);
			}
		}

	} else {
		// the spot already in the spotMgr
		time(&spotInfo.updateTime);
		spotMgr.updateSpot(spotInfo);
	}

	
	// ignore illegal node hb or those from the local machine
	// tstring tmpstr = params["NodeGuid"];
	// ZQ::common::Guid remoteId(tmpstr.c_str());
	/*
	NodeItem* nodeItem = _nodeMap.findNode(remoteId);
	if (nodeItem == NULL) {
		NodeItem* nodeItem = new NodeItem(remoteId);

		// InetHostAddress::inetaddr_t addr = source.getAddress();
		if (!_nodeMap.addNode(nodeItem)) {
			delete nodeItem;
			return false;
		}
	} else {
		nodeItem->release();
	}
	*/
	
//	if (remoteId.isNil() || _node._nodeid == remoteId)
//	{
//		printf("ignore node heartbeat from %s\n", tmpstr.c_str());
//		return false; 
//	}

	NodeHelper::OnBroadcast(verb, methodname, nid, appName, endPoint, 
		params, pMsgInfo, source, localCtx, sourceCtx, srcPid);

	// TODO:
	
	return false;
}

bool NodeWatchDog::init()
{
	char buf[2048];
	// cpu info
	try
	{
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);

		ZQ::common::Variant processorInfo;

		processorInfo.set("num", itoa(sysinfo.dwNumberOfProcessors, buf, 10));
		processorInfo.set("pagesize", itoa(sysinfo.dwPageSize, buf, 10));
		processorInfo.set("activemask", itoa(sysinfo.dwActiveProcessorMask, buf, 16));

		switch (sysinfo.dwProcessorType)
		{
		case PROCESSOR_INTEL_386:
			processorInfo.set("type", "Intel 386");
			break;
		case PROCESSOR_INTEL_486:
			processorInfo.set("type", "Intel 486");
			break;
		case PROCESSOR_INTEL_PENTIUM:
			if (sysinfo.wProcessorLevel == 6)
			{
				buf[0]=buf[1] =0x00;
				int iMod = (sysinfo.wProcessorRevision >> 8) & 0xff;
				int iStp = sysinfo.wProcessorRevision & 0xff;

				switch(iMod)
				{
				case 1:
					processorInfo.set("type", "Intel Pentium Pro");
					break;
				case 3:
					processorInfo.set("type", "Intel Pentium II");
					break;
				case 5:
					processorInfo.set("type", "Intel Pentium II Xeon or Celeron");
					break;
				case 6:
					processorInfo.set("type", "Intel Celeron");
					break;
				case 7:
				case 8:
					processorInfo.set("type", "Intel Pentium III or Pentium III Xeon");
					break;
				default:
					processorInfo.set("type", "Other Pentium processor");
					break;
				}
			}
			else
				processorInfo.set("type", "Pentium");
			break;
		case PROCESSOR_MIPS_R4000:
			processorInfo.set("type", "MIPS R4000");
			break;
		case PROCESSOR_ALPHA_21064:
			processorInfo.set("type", "Alpha 21064");
			break;
			
		default:
			if (sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_PPC)
				processorInfo.set("type", "PowerPC");
			else
				processorInfo.set("type", "Unknown");
			break;
		}

		// Get the processor speed info.
		HKEY hKey;
		LONG result = ::RegOpenKeyEx (HKEY_LOCAL_MACHINE,
			"Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey);

		// Check if the function has succeeded.
		if (result == ERROR_SUCCESS)
		{
			DWORD data, dataSize = MAX_PATH;
			result = ::RegQueryValueEx (hKey, "~MHz", NULL, NULL,
				(LPBYTE)&data, &dataSize);
			
			if (result == ERROR_SUCCESS)
				processorInfo.set("speed", itoa(data, buf, 10));

			dataSize = sizeof (buf);
			result = ::RegQueryValueEx (hKey, "VendorIdentifier", NULL, NULL,
						(LPBYTE)buf, &dataSize);

			if (result == ERROR_SUCCESS)
				processorInfo.set("vendor", buf);
			RegCloseKey (hKey);
		}

		_sysInfo.set("Processor", processorInfo);
	}
	catch(...) {}

	// os info
	try
	{
		ZQ::common::Variant osInfo;

		OSVERSIONINFOEX osinfo;
		memset(&osinfo, 0, sizeof(osinfo));
		osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		
		bool bEx = false;
		if( !(bEx = ::GetVersionEx((OSVERSIONINFO *)&osinfo)))
		{
			osinfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
			if(!::GetVersionEx((OSVERSIONINFO *)&osinfo))
				throw "";
		}

		buf[0]=buf[1] =0x00;

		sprintf(buf, "%d.%d", osinfo.dwMajorVersion, osinfo.dwMinorVersion);
		osInfo.set("version", buf);
		osInfo.set("build", itoa(osinfo.dwBuildNumber, buf, 10));

		std::string suit, edition;
		switch(osinfo.dwPlatformId)
		{
		case VER_PLATFORM_WIN32_NT:
			// Windows NT
			osInfo.set("type", "Windows NT");
			break;
			
		case VER_PLATFORM_WIN32_WINDOWS:
			if (osinfo.dwMajorVersion > 4 || ((osinfo.dwMajorVersion = 4) && (osinfo.dwMinorVersion > 0)))
				osInfo.set("type", "Windows 98");
			else
				osInfo.set("type", "Windows 95");
			break;
			
		case VER_PLATFORM_WIN32s:
			//Windows 3.1
			osInfo.set("type", "Windows 3.x");
			break;
			
		default:
			// Unknown OS
			osInfo.set("type", "Unknown Microsoft Windows");
			break;
		} // switch

		_sysInfo.set("OS", osInfo);
	}
	catch(...) {}

	// network info
	try
	{
		ZQ::common::Variant nicInfo;
	SOCKET sd;

    if ((sd = WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0)) == SOCKET_ERROR)
		throw "";

    INTERFACE_INFO InterfaceList[MAX_INTERFACES];
    unsigned long nBytesReturned;

    if (WSAIoctl(sd,
				SIO_GET_INTERFACE_LIST,
				0,
				0,
				&InterfaceList,
				sizeof(InterfaceList),
				&nBytesReturned,
				0,
				0) == SOCKET_ERROR)
		throw "";

	int ifcount=nBytesReturned / sizeof(INTERFACE_INFO);

	if (ifcount <=0)
		throw "";

	char buf[256];
    for (int i = 0; i < ifcount; ++i)
	{
		ZQ::common::Variant nic;

	    sockaddr_in *pAddress;

		// ----
		pAddress = (sockaddr_in *) &(InterfaceList[i].iiAddress);
		nic.set("ip", inet_ntoa(pAddress->sin_addr));

		uint64 ipaddr = pAddress->sin_addr.S_un.S_addr;

		// ----
		pAddress = (sockaddr_in *) &(InterfaceList[i].iiNetmask);
		nic.set("netmask", inet_ntoa(pAddress->sin_addr));
		
		uint64 netmask = pAddress->sin_addr.S_un.S_addr;

		// ----
		uint64 subnet = ipaddr & netmask;
		uint64 broadcastIpExpected = (~netmask & 0xffffffff) | subnet;

		pAddress = (sockaddr_in *) &(InterfaceList[i].iiBroadcastAddress);
		pAddress->sin_addr.S_un.S_addr &=broadcastIpExpected;
		nic.set("broadcast", inet_ntoa(pAddress->sin_addr));

		nic.set("flags", ltoa(InterfaceList[i].iiFlags, buf, 16));

		sprintf(buf, "NetIf%02x", i);
		nicInfo.set(buf, nic);
    }
	nicInfo.set("num", ifcount);

	shutdown(sd, 2);
	closesocket(sd);

	_sysInfo.set("NetIf", nicInfo);
	}
	catch(...) {}

	_node._nodeid.toString(buf, sizeof(buf));
	_sysInfo.set("NodeGuid", buf);
	return true;
}

int NodeWatchDog::run()
{
	int count = 0;
	char nodeId[64];
	_node._nodeid.toString(nodeId, sizeof(nodeId));

	while (!_bQuit)
	{
		if (_intvHeartbeat <0 || _intvHeartbeat>1000*DEFAULT_HEARTBEAT_INTV)
			_intvHeartbeat = DEFAULT_HEARTBEAT_INTV;

		if (WAIT_FAILED == ::WaitForSingleObject(_hWakeUp, _intvHeartbeat))
			_bQuit = true;
		else
			::ResetEvent(_hWakeUp);
		
		if (_bQuit)
			return 0;
		
		try
		{
			ZQ::common::Variant params;
			/*
			if (++count % 10 ==0)
			{
				count = 0;
				params =_sysInfo;
			}
			else {
				char buf[128];
				_node._nodeid.toString(buf, sizeof(buf));
				params.set("NodeGuid", buf);
			}
			
			params.set("HeartbeatInterval", ZQ::common::Variant(_intvHeartbeat));
			params.set("CtrlNetIf", ZQ::common::Variant( _node._addrCtrlBind.getHostAddress()));
			*/
			
			_node.groupPost(METHOD_NODE_HEATBEAT, nodeId, params);

			// “ª√Î“ª¥Œ
			Sleep(1000);
		}
		catch(...) {}
	}
	return 0;
}

// -----------------------------
// class NodeHelper
// -----------------------------
///@return false if no other helper needs to be notified
bool NodeHelper::OnBroadcast(const Verb verb, const char* methodname, 
							 const char* nodeId, 
							 const char* appName, 
							 const char* endPoint, 
							 ZQ::common::Variant& params, 
							 const MessageInfo* pMsgInfo, 
							 const ZQ::common::InetHostAddress& source, 
							 const char* remoteCtx, const char* localCtx, 
							 const DWORD srcPid)
{

#if 0
	printf("%s from NodeId: %s AppName: %s PID: %d END-POINT: %s p(%d)\n", 
		methodname, nodeId, appName, srcPid, endPoint, 
		_node._thpool.activeCount());
#endif

#if 0
	std::strstream ss;
	{
		ZQ::XmlRpc::XmlRpcSerializer s(params, ss);
		s.serialize();
	}
	
	printf("%s\n", ss.str());
#endif _DEBUG
	
	return false;
}

NAMESPACE_END
