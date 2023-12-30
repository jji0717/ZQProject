#include "RpcGeekServer.h"

#include "URLStr.h"
#include "NativeThreadPool.h"
#include "Locks.h"

#include "RpcGeekUtils.h"
//#include <Ws2tcpip.h>
#include <xmlrpc-c/server.h>
#include <xmlrpc-c/server_abyss.h>
#include <xmlrpc-c/abyss.h>

#pragma comment(lib, "abysshttp" XMPRPCLIBEXT)
#pragma comment(lib, "ws2_32.lib")

namespace ZQ {
namespace RpcGeek {

/// -----------------------------
/// class SkelNest
/// -----------------------------
class SkelNest : public ZQ::common::NativeThreadPool, public ZQ::common::NativeThread
{
	friend class Server;
    xmlrpc_registry* _pRegistry;
    xmlrpc_env _env;
	Server& _owner;
	
	int _maxConn;
	bool _bQuit;
	std::string _endpoint;
	TIPAddr _bindIP;
	TServer _serverparams;
	
	typedef std::vector<ServerHelper* > HelperVector;
	HelperVector _helpers;
	ZQ::common::Mutex _helpersLocker;
	
	SkelNest(Server& owner, const char* servEndpoint, const int maxConn, const char* logfilename ="/tmp/xmlrpc_log", int keepalivetimeout=0, int keepalivemaxconn=0, int timeout=0);
	~SkelNest();
	
	static xmlrpc_value* _default_method(xmlrpc_env * env,
		const char *   host,
		const char *   method_name,
		xmlrpc_value * param_array,
		void *         user_data);
	
	virtual bool init(void);
	virtual int run(void);
	virtual void final(void);
};

/// -----------------------------
/// class SkelRequest
/// -----------------------------
class SkelRequest: public ZQ::common::ThreadRequest, public TConn
{
public:
	SkelRequest(SkelNest& nest) : ThreadRequest(nest) {}
	
	int run()
	{
		try {
			ConnProcess(this);
		} catch(...) {}
		return 0;
	}
	
	void final(int retcode =0, bool bCancelled =false)
	{
		delete this;
	}
};

SkelNest::SkelNest(Server& owner, const char* servEndpoint, const int maxConn,
				   const char* logfilename, int keepalivetimeout, int keepalivemaxconn, int timeout)
				   :_owner(owner), _maxConn(maxConn), _bQuit(false),
				   NativeThreadPool((maxConn<=0) ? DEFAULT_MAX_CONN:maxConn)
{
	_endpoint = (NULL == servEndpoint) ? DEFAULT_SERV_END_POINT : servEndpoint;
	ZQ::common::URLStr url(_endpoint.c_str());
	
	xmlrpc_env_init(&_env);
	
	_pRegistry = xmlrpc_registry_new(&_env);
	if (NULL == _pRegistry)
		throw ZQ::common::IOException("SkelNest() out of memory for _pRegistry");
	
	xmlrpc_registry_set_default_method(&_env, _pRegistry, _default_method, &_owner);
	
	if (_maxConn<=0)
		_maxConn = DEFAULT_MAX_CONN;
	
	// get the bind IP and port from endpoint URL
	int port = url.getPort();
	if (port <=0)
		port = DEFAULT_SERV_PORT;
	
	memset(&_bindIP, 0x00, sizeof(_bindIP));
	std::string ip = url.getHost();
	struct hostent* hp = gethostbyname(ip.c_str());
	if (hp)
	{
		struct in_addr **bptr = (struct in_addr **)hp->h_addr_list;
		_bindIP = *bptr[0];
	}
	
	/*
	ADDRINFO hints, *AddrInfo;
	memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo("www.google.com", NULL, &hints, &AddrInfo) >0)
	{
	const sockaddr_in* sin = reinterpret_cast<const sockaddr_in*>(AddrInfo->ai_addr);
	_bindIP = sin->sin_addr;
	}
	
	  // count the addresses getaddrinfo returned
	  int c =0;
	  ADDRINFO* ai =NULL;
	  for (ai =AddrInfo; ai != NULL; ai = ai->ai_next)
	  if (ai->ai_family == PF_INET || ai->ai_family == PF_INET6)
	  c ++;
	  
	*/
	
    // prepare the server params
	ServerCreate(&_serverparams, "XmlRpcServer", port, DEFAULT_DOCS, logfilename);
	
	// additional server params
	if (timeout >0 )
		_serverparams.timeout = timeout;
	if (keepalivemaxconn >0 )
		_serverparams.keepalivemaxconn = keepalivemaxconn;
	if (keepalivetimeout >0 )
		_serverparams.keepalivetimeout = keepalivetimeout;
	
    // connect the handler
	xmlrpc_server_abyss_set_handlers(&_serverparams, _pRegistry);
}

SkelNest::~SkelNest()
{
	if (NULL != _pRegistry)
		xmlrpc_registry_free(_pRegistry);
	_pRegistry = NULL;
	
}

xmlrpc_value* SkelNest::_default_method(xmlrpc_env * env,
										const char *   host,
										const char *   method_name,
										xmlrpc_value * param_array,
										void *         user_data)
{
	Server* pOwner = (Server*)user_data;
	if (NULL == pOwner)
		return NULL;
	
	//convert paramters to variant
	ZQ::common::Variant params, result;
	if (!_convert2variant(env, param_array, params))
		return NULL;
	
	bool bExec = false;
	for (HelperVector::iterator it = pOwner->_nest->_helpers.begin();
	!bExec && it < pOwner->_nest->_helpers.end(); it++)
	{
		ServerHelper *phelper = (ServerHelper*) (*it);
		bExec = phelper->execMethod(host, method_name, params, result);
	}
	
	if (!bExec)
	{
		xmlrpc_env_set_fault_formatted(env, XMLRPC_NO_SUCH_METHOD_ERROR, "no such method: %s", method_name);
		return NULL;
	}

	return _convert2xmlrpc(env, result);
}

bool SkelNest::init(void)
{
    // Must check errors from these functions
    return (SocketCreate(&_serverparams.listensock)
		&& SocketBind(&_serverparams.listensock, &_bindIP, _serverparams.port)
		&& SocketListen(&_serverparams.listensock, _maxConn));
}

int SkelNest::run(void)
{
	//    setupSignalHandlers();
	
    while (!_bQuit)
	{
		//        printf("Waiting for next RPC...\n");
		try {
			TSocket connectedSocket;
			TIPAddr remoteAddr;
			
			if (SocketAccept(&_serverparams.listensock, &connectedSocket, &remoteAddr))
			{
				SkelRequest* conn = new SkelRequest(*this);
				if (NULL != conn)
				{
					conn->connected = FALSE;
					conn->inUse = FALSE;
					abyss_bool success = ConnCreate2(conn, &_serverparams, connectedSocket, remoteAddr, 
						&ServerFunc,
						ABYSS_FOREGROUND);
					if (success)
						conn->start();
					else delete conn;
				}
				closeParentSocketCopy(&connectedSocket);
			} else
				TraceMsg("Socket Error=%d\n", SocketError());
			
			// ServerRunOnce2(&_serverparams, ABYSS_BACKGROUND/*ABYSS_FOREGROUND*/);
			// ServerRunOnce2(&_serverparams, ABYSS_FOREGROUND);
		}catch(...) {}
    }
	
	return 0;
}
void SkelNest::final(void)
{
}

/// -----------------------------
/// class Server
/// -----------------------------
Server::Server(const char* servEndpoint, const int maxConn)
:_nest(NULL)
{
	_nest = new SkelNest(*this, servEndpoint, maxConn);
	if (NULL == _nest)
        throw ZQ::common::IOException("Server() out of memory for _nest");
}

Server::~Server()
{
	stop();
	if (_nest)
		delete _nest;
	
	_nest = NULL;
}

void Server::addHelper(ServerHelper& helper)
{
	if (NULL == _nest)
		return;
	
	ZQ::common::MutexGuard guard(_nest->_helpersLocker);
	
	SkelNest::HelperVector::iterator it = _nest->_helpers.begin();
	for (; it < _nest->_helpers.end() && (*it) != &helper; it++);
	
	if (it == _nest->_helpers.end())
		_nest->_helpers.push_back(&helper);
}

void Server::removeHelper(ServerHelper& helper)
{
	if (NULL == _nest)
		return;
	
	ZQ::common::MutexGuard guard(_nest->_helpersLocker);
	
	SkelNest::HelperVector::iterator it = _nest->_helpers.begin();
	for (; it < _nest->_helpers.end() && (*it) != &helper; it++);
	
	if (it != _nest->_helpers.end())
		_nest->_helpers.erase(it);
}

void Server::serv()
{
	if (NULL == _nest)
		return;
	_nest->start();
}

void Server::stop()
{
	if (NULL == _nest)
		return;
	_nest->_bQuit =true;
	Sleep(1); // yield
}

/// -----------------------------
/// class ServerHelper
/// -----------------------------
ServerHelper::ServerHelper(Server& skel)
:_skel(skel)
{
	// register self into the skel
	_skel.addHelper(*this);
}

ServerHelper::~ServerHelper()
{
	// unregister self into the skel
	_skel.removeHelper(*this);
}

/// -----------------------------
/// class ServerObject
/// -----------------------------
ServerObject::ServerObject(ServerObjectHelper& helper, const ZQ::common::Guid& guid)
: _helper(helper), _guid(guid)
{
	renew();
}

ServerObject::~ServerObject()
{
}

void ServerObject::renew(void)
{
	PollingTimer::setTimer(_helper.getLeaseTerm());
}

//const char* ServerObject::guidStr(void)
//{
//	return _guid.toString().c_str();
//}


/// -----------------------------
/// class ServerObjectMap
/// -----------------------------
//TODO: object life-cycle management
class ServerObjectMap
{
	friend class ServerObjectHelper;
	ServerObjectMap(ServerObjectHelper& helper) : _helper(helper) {}
	~ServerObjectMap();

	ServerObject* findObject(const ZQ::common::Guid& obj_id);
	bool insert(const ZQ::common::Guid& obj_id, ServerObject* pSO);
	bool erase(const ZQ::common::Guid& obj_id);

	ServerObjectHelper& _helper;

	typedef std::map<ZQ::common::Guid, ServerObject* > Map;
	Map _serverObjectMap;
	ZQ::common::Mutex _serverObjectMap_locker;
};

ServerObjectMap::~ServerObjectMap()
{
	ZQ::common::MutexGuard guard(_serverObjectMap_locker);
	std::vector<ZQ::common::Guid> idlist;
	for (Map::iterator it= _serverObjectMap.begin(); it !=_serverObjectMap.end(); it++)
		idlist.push_back(it->first);

	for (std::vector<ZQ::common::Guid>::iterator it2= idlist.begin(); it2 !=idlist.end(); it2++)
		erase(*it2);
}

ServerObject* ServerObjectMap::findObject(const ZQ::common::Guid& obj_id)
{
	if (obj_id.isNil())
		return NULL;
	
	Map::iterator it = _serverObjectMap.find(obj_id);
	
	if (_serverObjectMap.end() != it)
		return (it->second);
	
	return NULL;
}

bool ServerObjectMap::insert(const ZQ::common::Guid& obj_id, ServerObject* pSO)
{
	if (obj_id.isNil() || NULL == pSO)
		return false;

	// add it into the map for future operations
	ZQ::common::MutexGuard guard(_serverObjectMap_locker);
	_serverObjectMap.insert(std::make_pair<ZQ::common::Guid, ServerObject*>(obj_id, pSO));
	return true;
}

bool ServerObjectMap::erase(const ZQ::common::Guid& obj_id)
{
	if (obj_id.isNil())
		return false;

	ServerObject* pSO = findObject(obj_id);
	if (NULL == pSO)
		return false;

	ZQ::common::MutexGuard guard(_serverObjectMap_locker);
	_serverObjectMap.erase(obj_id);

	// delete the user server object instance
	_helper.releaseEx(pSO);
	pSO = NULL;
	return true;
}


/// -----------------------------
/// class ServerObjectHelper
/// -----------------------------
ServerObjectHelper::ServerObjectHelper(ZQ::RpcGeek::Server& skel, timeout_t objLeaseTerm)
: ServerHelper(skel), _pMap(NULL), _objLeaseTerm(objLeaseTerm)
{
	_pMap = new ServerObjectMap(*this);
}

ServerObjectHelper::~ServerObjectHelper()
{
	if (_pMap)
		delete _pMap;
	_pMap=NULL;
}

ServerObject* ServerObjectHelper::findObject(ZQ::common::Variant& params, ZQ::common::Guid& objGuid)
{
	objGuid = ZQ::common::Guid();

	if (!isValid())
		return NULL;
	
	if (ZQ::common::Variant::T_ARRAY != params.type() || params.size() <=0
		|| ZQ::common::Variant::T_STRUCT != params[0].type() || !params[0].has(OBJ_GUID))
		return NULL;
	
	if (ZQ::common::Variant::T_STRING != params[0][OBJ_GUID].type())
		return NULL;
	
	try
	{
		tstring obj_id_str = params[0][OBJ_GUID];
		objGuid = obj_id_str.c_str();
		ServerObject* pSO = _pMap->findObject(objGuid);
		if (NULL == pSO)
			pSO->renew();
		return pSO;
	}
	catch(...) {}
	return NULL;
}

size_t ServerObjectHelper::size(void)
{
	if (!isValid())
		return 0;
	return _pMap->_serverObjectMap.size();
}

timeout_t ServerObjectHelper::getLeaseTerm(void)
{
	return _objLeaseTerm;
}

void ServerObjectHelper::create(ZQ::common::Variant& params, ZQ::common::Variant& result, const char* host)
{
	if (!isValid())
	{
		setError(result, true, ERROR_INTERNAL, "ServerObjectHelper internal error: invalid helper body");
		return;
	}

	ZQ::common::Guid objGuid;
	ServerObject* pSO = findObject(params, objGuid);
	if (NULL != pSO || objGuid.isNil())
	{
		setError(result, true, ERROR_UID, "ServerObject(%s) failed to create, already exists", objGuid.toString().c_str());
		return;
	}
	
	// now create a new playlist instance
	pSO = createEx(objGuid);
	if (NULL == pSO)
	{
		setError(result, true, ERROR_MEM, "ServerObject(%s) failed to create, out of memory", objGuid.toString().c_str());
		return;
	}
	
	// add it into the map for future operations
	_pMap->insert(objGuid, pSO);

	setError(result, false, ERROR_SUCC, "ServerObject(%s) created", objGuid.toString().c_str());
}

void ServerObjectHelper::release(ZQ::common::Variant& params, ZQ::common::Variant& result, const char* host)
{
	if (!isValid())
	{
		setError(result, true, ERROR_INTERNAL, "ServerObjectHelper internal error: invalid helper body");
		return;
	}

	ZQ::common::Guid objGuid;
	ServerObject* pSO = findObject(params, objGuid);
	if (NULL == pSO)
	{
		setError(result, true, ERROR_NOTFOUND, "ServerObject(%s) not found", objGuid.toString().c_str());
		return;
	}
	
	// remove from the map
	_pMap->erase(objGuid);

	setError(result, false, ERROR_SUCC, "ServerObject(%s) released", objGuid.toString().c_str());
}

void ServerObjectHelper::lookup(ZQ::common::Variant& params, ZQ::common::Variant& result, const char* host)
{
	if (!isValid())
	{
		setError(result, true, ERROR_INTERNAL, "ServerObjectHelper internal error: invalid helper body");
		return;
	}

	ZQ::common::Guid objGuid;
	ServerObject* pSO = findObject(params, objGuid);
	if (NULL == pSO)
	{
		setError(result, true, ERROR_NOTFOUND, "ServerObject(%s) not found", objGuid.toString().c_str());
		return;
	}
	
	setError(result, false, ERROR_SUCC, "ServerObject(%s) found", objGuid.toString().c_str());
}

void ServerObjectHelper::setError(ZQ::common::Variant& result, bool bReset, const char* errcode, const char* fmt, ...)
{
	if (NULL == errcode)
		errcode = "N/A";
	if (bReset)
		result = ZQ::common::Variant();
	
	if (ZQ::common::Variant::T_STRUCT !=result.type() && ZQ::common::Variant::T_NIL !=result.type())
		result = ZQ::common::Variant();
	
	char msg[2048];
	va_list args;
	
	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);
	
#ifdef _DEBUG
	printf("ServerObj::setError(%s), %s\n", errcode, msg);
#endif // _DEBUG
	result.set(ERROR_CODE, errcode);
	result.set(ERROR_MSG,  msg);
}

} // namespace RpcGeek
} // namespace ZQ

#if defined(WIN32) && defined(SKEL_ALONE) // Winsock processing
// -----------------------------
// class init_WSA
// -----------------------------
// class init_WSA used to initalise windows sockets specfifc stuff : there is
// an MS - specific init sequence for Winsock 2 this class attempts to 
// initalise Winsock 2.2 - needed for non - blocking I/O. It will fall back 
// on 1.2 or lower if 2.0 or higher is not available,  but < 2.0 does not 
// support non - blocking I/O
// TO DO : might be an idea to have a method that reports version of Winsock in
// use or a predicate to test if non - blocking is OK
class init_WSA
{
public:
	init_WSA()
	{
		//-initialize OS socket resources!
		WSADATA	wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData))
		{
			abort();
		}
	};
	
	~init_WSA() 
	{ 
		WSACleanup(); 
	} 
};

init_WSA iWSA;
#endif // WIN32 && SKEL_ALONE
