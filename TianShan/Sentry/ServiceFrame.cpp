#include "debug.h"
#include "ServiceFrame.h"

#ifndef _NO_LOG_LIB_SUPPORT
#include "Log.h"
#endif

#include "Locks.h"

#define SF_RECV_TIMEOUT			10
static uint32 sf_max_polltime = 200;

#define MS2TV(tv, ms)					\
	do { \
		(tv).tv_sec = (ms) / 1000; \
		(tv).tv_usec = ((ms) % 1000) * 1000; \
	} while(0)
	
#ifndef _NO_NAMESPACE
namespace ZQ {
#endif // #ifndef _NO_NAMESPACE

#ifndef _NO_LOG_LIB_SUPPORT
using namespace ZQ::common;
#endif

#ifdef _DEBUG
#define new			debug_new
#endif

//////////////////////////////////////////////////////////////////////////
// global
//

// trace supprot route.
#ifndef _NOTRACE
#define SVCDBG_PREFIX				"[SrvFrm] "
int svcdbg_print(int level, char* format, va_list vlist);

#ifdef _NO_LOG_LIB_SUPPORT
class _svcdbg_init{
public:
	_svcdbg_init()
	{
		InitializeCriticalSection(&svcdbg_cs);
	}

	~_svcdbg_init()
	{
		DeleteCriticalSection(&svcdbg_cs);
	}

	static CRITICAL_SECTION	svcdbg_cs;
}static _dbg_init;

CRITICAL_SECTION _svcdbg_init::svcdbg_cs;
static ostream&	svcdbg_outStrm		= cout;

void svcdbg_setDebugOutStream(ostream& ostrm)
{
	svcdbg_outStrm = ostrm;
}

#endif

static int		svcdbg_level = SVCDBG_LEVEL_DEBUG;
static int		svcdbg_counts[SVCDBG_LEVEL_MAX + 1];

int svcdbg_setLevel(int level)
{
	if (level >= SVCDBG_LEVEL_MIN && level <= SVCDBG_LEVEL_MAX)
		svcdbg_level = level;
	else {
		svcdbg_printError("Invalid level value");
		return -1;
	}
	return svcdbg_level;
}

int svcdbg_getLevel()
{
	return svcdbg_level;
}

void svcdbg_resetCounts()
{
	for(int i = SVCDBG_LEVEL_MIN; i <= SVCDBG_LEVEL_MAX; i ++)
		svcdbg_counts[i] = 0;
}

int svcdbg_getCount(int level)
{
	return svcdbg_counts[level];
}

int svcdbg_print(int level, const char* format, va_list vlist)
{
	int result;
	char buf[1025];

	if (level < SVCDBG_LEVEL_MIN || level > SVCDBG_LEVEL_MAX) {
		svcdbg_printError("SvcDebug::_print(): Invalid Debugging level");
		return 0;
	}
	
	svcdbg_counts[level] ++;
	if (svcdbg_level < level)
		return 0;
#ifdef _NO_LOG_LIB_SUPPORT

	result = vsprintf(buf, format, vlist);
	static char* levelname[] = {
		"", 
		"Fault:   ", 
		"Error:   ",
		"Warning: ",
		"Notice	  ", 
		"Debug:   ",
	};

	EnterCriticalSection(&_dbg_init.svcdbg_cs);
	if (result)
		svcdbg_outStrm << SVCDBG_PREFIX << GetTickCount() << setw(12) << 
			levelname[level] << buf << endl;
	LeaveCriticalSection(&_dbg_init.svcdbg_cs);
#else
	strcpy(buf, SVCDBG_PREFIX);
	result = vsprintf(&buf[sizeof(SVCDBG_PREFIX) - 1], 
		format, vlist);
	int iLens =strlen(buf);
	if(buf[iLens-1] == '\n')
		buf[iLens-1] = '\0';

	Log::loglevel_t logLevel;
	switch (level) {
	case SVCDBG_LEVEL_FAULT:
		logLevel = Log::L_CRIT;
		break;
	case SVCDBG_LEVEL_ERROR:
		logLevel = Log::L_ERROR;
		break;
	case SVCDBG_LEVEL_WARNING:
		logLevel = Log::L_WARNING;
		break;
	case SVCDBG_LEVEL_NOTICE:
		logLevel = Log::L_NOTICE;
		break;
	case SVCDBG_LEVEL_DEBUG:
		logLevel = Log::L_DEBUG;
		break;
	default:
		logLevel = Log::L_WARNING;
		break;
	}
	if ((&glog) != NULL)
		glog(logLevel, buf);
#endif
	
	return result;
}

int svcdbg_dumpInternalError()
{
	return 0;
}

void svcdbg_printFault(const char* format, ...)
{
	va_list vlist;
	va_start(vlist, format);
	svcdbg_print(SVCDBG_LEVEL_FAULT, format, vlist);
}

void svcdbg_printError(const char* format, ...)
{
	va_list vlist;
	va_start(vlist, format);
	svcdbg_print(SVCDBG_LEVEL_ERROR, format, vlist);
}

void svcdbg_printWarning(const char* format, ...)
{
	va_list vlist;
	va_start(vlist, format);
	svcdbg_print(SVCDBG_LEVEL_WARNING, format, vlist);
}

void svcdbg_printDebug(const char* format, ...)
{
	va_list vlist;
	va_start(vlist, format);
	svcdbg_print(SVCDBG_LEVEL_DEBUG, format, vlist);
}

void svcdbg_printNotice(const char* format, ...)
{
	va_list vlist;
	va_start(vlist, format);
	svcdbg_print(SVCDBG_LEVEL_NOTICE, format, vlist);
}

#endif // #ifdef _NOTRACE

//////////////////////////////////////////////////////////////////////////

uint32 sf_idle_timeout = SF_IDLE_TIMEOUT;

//////////////////////////////////////////////////////////////////////////

//
// 基于 SSLSocket 的 IConn 的实现, 用于服务器在多通道时
// 支持主连接之外的连接
// 方法注解注解请参看接口定义(ServiceFrame.h)
//
class IConnectionImpl : public IConn 
{
public:
	IConnectionImpl(SSLSocket& sock);
	virtual ~IConnectionImpl();

	virtual int recv(OUT void* buf, IN int size);
	virtual int recvTimeout(OUT void* buf, IN int size, IN int timeo);
	virtual int send(IN const void* buf, IN int size);
	virtual int sendTimeout(IN const void* buf, IN int size, IN int timeo);
	virtual int close();
	virtual int upgradeSecurity(int timeout);
	virtual int degradeSecurity();
	virtual bool isSecure();
	virtual bool isActive();
	virtual void release();
	virtual int getConnId();

protected:
	SSLSocket&	_ssock;
};

//
// 基于 SSLSocket 的 IMainConn 的实现, 用于服务器实现主连接
//
#define MAINCONN_CANNOT_SEND			1
#define MAINCONN_CANNOT_RECV			2

class IMainConnImpl: public IMainConn
 {
	friend class ServiceFrmBase;
public:
	IMainConnImpl(ServiceFrmBase& frm, SSLSocket& sock);
	virtual ~IMainConnImpl();

	virtual void onConnected();
	virtual int recv(OUT void* buf, IN int size);
	virtual int recvTimeout(OUT void* buf, IN int size, IN int timeo);
	virtual int send(IN const void* buf, IN int size);
	virtual int sendTimeout(IN const void* buf, IN int size, IN int timeo);
	virtual int close();
	
	virtual int upgradeSecurity(int timeout);
	virtual int degradeSecurity();
	virtual bool isSecure();
	virtual bool isActive();
	virtual void release();
	virtual int getConnId();

	virtual int getPeerInfo(OUT PeerInfo* info);
	virtual IConn* createConn(IN const ConnID* addr, 
		IN int mode, bool inheritSec, IN int timeout);
	virtual void destroyConn(IN IConn* conn);

	virtual IDialogue* getDialogue();
	bool setDialogue(IDialogue* session);
	
	SSLSocket& getSSLSocket()
	{
		return _ssock;
	}

	DWORD64 getIdleTime();
	void updateIdleTime();

	bool modifyAttribute(uint32 added, uint32 removed)
	{
		_connAttr |= added;
		_connAttr &= ~removed;
		return true;
	}

	uint32 getAttribute()
	{
		return _connAttr;
	}

	unsigned __int64 getConnectionIdentity()
	{
		return _connectionIdentity;
	}
#if !defined(_SPEED_VERSION)
	void dump()
	{
		printDebug("IMainConnImpl dump: _session(Dialog) = %x, _ssock = %x", 
			_session, _ssock.getSocket());
	}
#endif

protected:
	void destroyAllConn();
	bool getCurrentTime(DWORD64* t);
protected:
	ServiceFrmBase&	_srvFrm;
	SSLSocket&		_ssock;
	IDialogue*		_session;
	ServiceFrmBase*	_service;
	PeerInfo		_peerInfo;
	typedef vector<IConnectionImpl* > VecConn;
	VecConn			_vecConn;
	uint32			_connAttr;
	DWORD64			_lastActiveTime;	
	unsigned __int64 _connectionIdentity;
};

//////////////////////////////////////////////////////////////////////////

//
// class IConnectionImpl
// IConn 接口实现类
//

IConnectionImpl::IConnectionImpl(SSLSocket& sock) :
	_ssock(sock)
{

}

IConnectionImpl::~IConnectionImpl()
{
	close();
}

int IConnectionImpl::recv(OUT void* buf, IN int size)
{
	return _ssock.recv(buf, size);
}

int IConnectionImpl::recvTimeout(OUT void* buf, IN int size, IN int timeo)
{
	int result;
	timeval t;
	MS2TV(t, timeo);
	result = _ssock.waitForIncome(&t);
	if (result <= 0) // error occurred or timeout
		return result;
	return _ssock.recv(buf, size);
}

int IConnectionImpl::send(IN const void* buf, IN int size)
{
	return _ssock.send(buf, size);
}

int IConnectionImpl::sendTimeout(IN const void* buf, IN int size, IN int timeo)
{
	int result;
	timeval t;
	MS2TV(t, timeo);
	result = _ssock.waitForOutgo(&t);
	if (result <= 0) // error occurred or timeout
		return result;
	return _ssock.send(buf, size);
}

int IConnectionImpl::close()
{
	int result = _ssock.close();
	_delete(&_ssock);
	return result;
}

int IConnectionImpl::upgradeSecurity(int timeout)
{
	return _ssock.s_accept();
}

int IConnectionImpl::degradeSecurity()
{
	return _ssock.s_shutdown();
}

bool IConnectionImpl::isSecure()
{
	return _ssock.isSecure();
}

bool IConnectionImpl::isActive()
{
	return _ssock.getSocket() != INVALID_SOCKET;
}

void IConnectionImpl::release()
{
	_delete(this);
}

int IConnectionImpl::getConnId()
{
	return (int )_ssock.getSocket();
}
//
// class IMainConnImpl
// IMainConn 接口实现类
//

IMainConnImpl::IMainConnImpl(ServiceFrmBase& frm, SSLSocket& sock) :
	_srvFrm(frm), _ssock(sock)
{
	_session = NULL;
	_connAttr = 0;
	getCurrentTime(&_lastActiveTime);
	frm.addMainConn(this);

	unsigned __int64 gConnectionIDXXX =0;
	gConnectionIDXXX ++;
	if(gConnectionIDXXX > 0xFFFFFFFFFFFFFFFF-2)
		gConnectionIDXXX=0;
	_connectionIdentity=gConnectionIDXXX;
}

IMainConnImpl::~IMainConnImpl()
{
	destroyAllConn();
	_srvFrm.delMainConn(this);
	delete &_ssock;
}

void IMainConnImpl::onConnected()
{
	time_t t;
	SOCKET s;
	time(&t);
	_peerInfo.ct = *localtime(&t);
	s = _ssock.getSocket();
	SF_ASSERT(s != INVALID_SOCKET);
	_peerInfo.addr.type = CONN_TYPE_SOCKET_TCP;
	_peerInfo.addr.addrlen = sizeof(_peerInfo.addr.caddr);
	::getpeername(s, &_peerInfo.addr.caddr, 
		&_peerInfo.addr.addrlen);
}

int IMainConnImpl::recv(OUT void* buf, IN int size)
{
	if (_connAttr & MAINCONN_CANNOT_RECV) {
		assert(false);
		printDebug("Can't read data at here.");
		return SOCKET_ERROR;
	}

	int result = _ssock.recv(buf, size);
	if (result > 0)
		getCurrentTime(&_lastActiveTime);
	return result;
}

int IMainConnImpl::recvTimeout(OUT void* buf, IN int size, IN int timeo)
{
	int result;
	timeval t;
	
	if (_connAttr & MAINCONN_CANNOT_RECV) {
		printDebug("Can't read data at here.");
		return SOCKET_ERROR;
	}

	MS2TV(t, timeo);
	result = _ssock.waitForIncome(&t);
	if (result <= 0) // error occurred or timeout
		return result;

	getCurrentTime(&_lastActiveTime);
	return _ssock.recv(buf, size);
}

int IMainConnImpl::send(IN const void* buf, IN int size)
{
	if (_connAttr & MAINCONN_CANNOT_SEND) {
		printDebug("Can't write data at here.");
		return SOCKET_ERROR;
	}

	int result = _ssock.send(buf, size);
	if (result > 0)
		getCurrentTime(&_lastActiveTime);

	return result;
}

int IMainConnImpl::sendTimeout(IN const void* buf, IN int size, IN int timeo)
{
	int result;
	timeval t;

	if (_connAttr & MAINCONN_CANNOT_SEND) {
		printDebug("Can't write data at here.");
		return SOCKET_ERROR;
	}

	MS2TV(t, timeo);
	result = _ssock.waitForOutgo(&t);
	if (result <= 0) // error occurred or timeout
		return result;
	getCurrentTime(&_lastActiveTime);
	return _ssock.send(buf, size);
}

int IMainConnImpl::close()
{
	return _ssock.close();
}

int IMainConnImpl::upgradeSecurity(int timeout)
{
	return _ssock.s_accept();
}

int IMainConnImpl::degradeSecurity()
{
	return _ssock.s_shutdown();
}

bool IMainConnImpl::isSecure()
{
	return _ssock.isSecure();
}

bool IMainConnImpl::isActive()
{
	return _ssock.getSocket() != INVALID_SOCKET;
}

void IMainConnImpl::release()
{
	_delete(this);
}

int IMainConnImpl::getConnId()
{
	return (int )_ssock.getSocket();
}

int IMainConnImpl::getPeerInfo(OUT PeerInfo* info)
{
	memcpy(info, &_peerInfo, sizeof(_peerInfo));
	return sizeof(_peerInfo);
}

IConn* IMainConnImpl::createConn(IN const ConnID* addr, 
	IN int mode, bool inheritSec, IN int timeout)
{
	SSLSocket sock(INVALID_SOCKET, _ssock.s_getConetext());
	SSLSocket * newsock;
	IConnectionImpl* conn;
	bool secure = false;
	if (inheritSec)
		secure = _ssock.isSecure();
	if (addr->type != CONN_TYPE_SOCKET_TCP)
		return NULL;

	if (mode == CONN_MODE_PASSIVE) {
		sock.socket();

		if (sock.bind(&addr->caddr, addr->addrlen) != 0) {
			sock.close();
			return NULL;
		}
		
		sock.listen();
		newsock = sock.accept(NULL, NULL, secure);
		if (newsock == NULL)
			return NULL;
		conn = new IConnectionImpl(*newsock);
		if (conn == NULL) {
			_delete(newsock);
			return NULL;
		}

		_vecConn.push_back(conn);
		return conn;
	} else if (mode == CONN_MODE_ACTIVE) {
		newsock = new SSLSocket(INVALID_SOCKET, _ssock.s_getConetext());
		newsock->socket();
		if (newsock->connect(&addr->caddr, addr->addrlen, secure) != 0)
			return NULL;
		
		conn = new IConnectionImpl(*newsock);
		if (conn == NULL) {
			_delete(newsock);
			return NULL;
		}

		_vecConn.push_back(conn);
		return conn;
	}

	return NULL;
}

void IMainConnImpl::destroyConn(IN IConn* conn)
{
	VecConn::iterator itor;
	for (itor = _vecConn.begin(); itor < _vecConn.end(); itor ++) {
		if (*itor == conn) {
			_vecConn.erase(itor);
			break;
		}
	}

	_delete(conn);	
}

IDialogue* IMainConnImpl::getDialogue()
{
	return _session;
}

bool IMainConnImpl::setDialogue(IDialogue* dlg)
{
	if (isActive() && dlg == NULL) {
		SF_ASSERT(false);
		printError("IMainConnImpl::setDialogue() faild. "
			"dlg == NULL, but conn is active.");
		return false;
	}

	_session = dlg;
	return true;
}

void IMainConnImpl::destroyAllConn()
{
	VecConn::iterator itor = _vecConn.begin(), last=_vecConn.end();
	while (itor < _vecConn.end()) {
		last = itor ++;
		_delete(*last);
		_vecConn.erase(itor);
	}

	if (_vecConn.end()!=last)
		_delete(*last);

}

bool IMainConnImpl::getCurrentTime(DWORD64* t)
{
	SYSTEMTIME st;
	GetSystemTime(&st);
	return SystemTimeToFileTime(&st, (FILETIME* )t);
}

DWORD64 IMainConnImpl::getIdleTime()
{
	DWORD64 cur;
	getCurrentTime(&cur);
	return (cur - _lastActiveTime) / 10000;
}

void IMainConnImpl::updateIdleTime()
{
	getCurrentTime(&_lastActiveTime);
}

//////////////////////////////////////////////////////////////////////////
ServiceConfig::ServiceConfig()
{
	_cfg_debugLevel = SVCDBG_LEVEL_DEBUG;
	_cfg_isSecure = false;
	_cfg_threadCount = 50;
	_cfg_maxConn = 2000;
	strcpy(_cfg_publicKeyFile, "server.pem");
	strcpy(_cfg_privateKeyFile, "server.pem");
	strcpy(_cfg_privateKeyFilePwd, "xiao");
	strcpy(_cfg_dhParamFile, "dh1024.pem");
	strcpy(_cfg_randFile, "rand.pem");

}

ServiceConfig::~ServiceConfig()
{

}

const ServiceConfig::_ConfigEntry* ServiceConfig::getBaseConfigMap()
{
	return NULL;
}

const ServiceConfig::_ConfigEntry* ServiceConfig::getConfigMap()
{
	static _ConfigEntry cfgMap[] = {
		{"publicKeyFile", _ConfigEntry::STRING_VALUE, 
			&_cfg_publicKeyFile, sizeof(_cfg_publicKeyFile)}, 
		{"privateKeyFile", _ConfigEntry::STRING_VALUE, 
			&_cfg_privateKeyFile, sizeof(_cfg_privateKeyFile)}, 
		{"privateKeyFilePwd", _ConfigEntry::STRING_VALUE, 
			&_cfg_privateKeyFilePwd, sizeof(_cfg_privateKeyFilePwd)}, 
		{"dhParamFile", _ConfigEntry::STRING_VALUE, 
			&_cfg_dhParamFile, sizeof(_cfg_dhParamFile)}, 
		{"randFile", _ConfigEntry::STRING_VALUE, 
			&_cfg_randFile, sizeof(_cfg_randFile)}, 
		{"isSecure", _ConfigEntry::USHORT_VALUE, 
				&_cfg_isSecure, sizeof(_cfg_isSecure)}, 
		{"threadCount", _ConfigEntry::USHORT_VALUE, 
				&_cfg_threadCount, sizeof(_cfg_threadCount)}, 
		{"maxConnection", _ConfigEntry::ULONG_VALUE, 
				&_cfg_maxConn, sizeof(_cfg_maxConn)},
		{"debugLevel", _ConfigEntry::LONG_VALUE, 
			&_cfg_debugLevel, sizeof(_cfg_debugLevel)},	
		{"idleTimeout", _ConfigEntry::LONG_VALUE, 
			&sf_idle_timeout, sizeof(sf_idle_timeout)},
		{"maxPollTime", _ConfigEntry::ULONG_VALUE, 
			&sf_max_polltime, sizeof(sf_max_polltime)},
		{NULL, _ConfigEntry::INVALID_TYPE, NULL, 0}, 
	};

	return cfgMap;
}

bool ServiceConfig::load(const char* cfgfile,std::vector<std::string>& pathVec)
{
	/* read configuration from XML file */
	XMLPreferenceDocumentEx root;
	XMLPreferenceEx* pref = NULL, * child = NULL;
	
	try {
		if(!root.open(cfgfile)) {
			printError("ServiceFrmBase::initByConfigureFile() failed");
			return false;
		}

		pref = root.getRootPreference();
		if (pref == NULL)
			return false;
		if(pathVec.size() <= 0)
		{
			child = pref->firstChild("Service");		
			pref->free();
		}
		else
		{
			while (pathVec.size() > 0)
			{
				child=pref->firstChild(pathVec[0].c_str());
				if(!child)
					return false;
				pathVec.erase(pathVec.begin());
				pref->free();
				pref=child;
			}
		}
		if (!child)
			return false;
		
		pref = child;
		const _ConfigEntry* cfgEntry;
		cfgEntry = getBaseConfigMap();
		if (cfgEntry != NULL)
			processEntry(pref, cfgEntry);

		cfgEntry = getConfigMap();
		if (cfgEntry != NULL)
			processEntry(pref, cfgEntry);

		pref->free();

	}  catch(Exception ) {
		if (pref)
			pref->free();
		return false;
	}

	if (sf_max_polltime < MAX_CONN_PER_REQ)
		sf_max_polltime = MAX_CONN_PER_REQ;

	return true;
}

bool ServiceConfig::processEntry(XMLPreferenceEx* pref, 
								 const _ConfigEntry* cfgEntry)
{
	XMLPreferenceEx* child = NULL;
	char buf[512];
	char* fmt;
	try {
		while(cfgEntry->type != _ConfigEntry::INVALID_TYPE) {
			child = pref->firstChild(cfgEntry->key);
			if (child == NULL)
				goto L_Next;
			if (!child->getAttributeValue("value", buf, sizeof(buf) - 1)) {
				child->free();
				goto L_Next;
			}
			switch(cfgEntry->type) {
			case _ConfigEntry::STRING_VALUE:
				fmt = "%s";
				break;
			case _ConfigEntry::SHORT_VALUE:
				fmt = "%hd";
				break;
			case _ConfigEntry::USHORT_VALUE:
				fmt = "%hu";
				break;
			case _ConfigEntry::LONG_VALUE:
				fmt = "%ld";
				break;
			case _ConfigEntry::ULONG_VALUE:
				fmt = "%lu";
				break;
			default:
				fmt = "%s";
				assert(false);
				break;
			}
			if(cfgEntry->type==_ConfigEntry::STRING_VALUE)
				strncpy((char*)cfgEntry->value,buf,cfgEntry->maxValue);
			else
				sscanf(buf, fmt, cfgEntry->value);
			child->free();
L_Next:
			cfgEntry ++;
		}

	} catch(Exception ) {
		if (child)
			child->free();
		return false;
	}

	return true;

}

//
// ServiceFrmBase
//

ServiceFrmBase::ServiceFrmBase():
	_sslCtx(VER_SSLv2v3)
{
	_dlgCtor = NULL;
	_secure = FALSE;
	_maxConn = 850;
	_acceptConn = 1;
	_threadCount = 50;
	_running = false;
	_idleTimeout = SF_IDLE_TIMEOUT;
	_serviceThread = true;
	_connListLock = create_rwlock();	
}

ServiceFrmBase::~ServiceFrmBase()
{
	destroy_rwlock(_connListLock);
}

bool ServiceFrmBase::initByConfiguration(const ServiceConfig* cfg)
{
	if (cfg == NULL) {
		assert(false);
		return false;
	}
	
	svcdbg_setLevel(cfg->_cfg_debugLevel);
	ssldbg_setLevel(cfg->_cfg_debugLevel);
	_secure = cfg->_cfg_isSecure;
	_threadCount = cfg->_cfg_threadCount;
	_maxConn = cfg->_cfg_maxConn;

	if (_threadCount == 0) {
		_threadCount = 50;
	}

	if (_threadCount > 1000) {
		_threadCount = 1000;
	}

	if (_maxConn > 1000000) {
		_maxConn = 1000000;
	}	

	if (_secure) {
		// load_randomess_file(cfg->_cfg_randFile);
		_sslCtx.loadCertificate(cfg->_cfg_publicKeyFile);
		_sslCtx.loadPrivateKeyFile(cfg->_cfg_privateKeyFile, cfg->_cfg_privateKeyFilePwd);
		// _sslCtx.loadRootCertificates("roots.pem");
		_sslCtx.loadDHParams(cfg->_cfg_dhParamFile);
	}

	return true;
}

bool ServiceFrmBase::init(const ServiceConfig* cfg /* = NULL */)
{
	WSADATA wsad;
	WSAStartup(MAKEWORD(2, 0), &wsad);
	
	if (_secure) {

		init_ssl_library();
		_sslCtx.create();
	}

	if (cfg == NULL || initByConfiguration(cfg) == false) {
		// use default value
		if (_secure) {
			// load_randomess_file("rand.pem");
			_sslCtx.loadCertificate("server.pem");
			_sslCtx.loadPrivateKeyFile("server.pem", "xiao");
			// _sslCtx.loadRootCertificates("roots.pem");
			_sslCtx.loadDHParams("dh1024.pem");
		}
	}

	if (_secure)
		_ssock.s_setConetext(&_sslCtx);

	printDebug("_secure = %d, _threadCount = %d, _maxCount = %d\n", 
			_secure, _threadCount, _maxConn);

	return true;
}

void ServiceFrmBase::uninit()
{

}

bool ServiceFrmBase::begin(const ConnID* connID)
{
	if (_running) {
		printFault("ServiceFrmBase::begin(): service is running.");
		SF_ASSERT(false);
		return false;
	}

	if (connID->type != CONN_TYPE_SOCKET_TCP) {
		printError("ServiceFrmBase::begin(): Must be TCP service.");
		SF_ASSERT(false);
		return false;
	}
	
	_ssock.socket();

#if 0
	LINGER linger;
	linger.l_onoff = 1;
	linger.l_linger = 70;
	_ssock.setsockopt(SOL_SOCKET, SO_LINGER, (const char *)&linger, 
		sizeof(linger));
#endif
	
	if (_ssock.bind(&connID->caddr, connID->addrlen) != 0) {
		printError("ServiceFrmBase::begin(): _ssock.bind() failed.");
		return false;
	}
	
	if (_ssock.listen() != 0) {
		printError("ServiceFrmBase::begin(): _ssock.listen() failed.");
		return false;
	}

	if (_serviceThread)
		return start();
	else {
		run();
		return true;
	}
}

bool ServiceFrmBase::end()
{
	// _threadMgr->cancelAll();
	this->terminate(-1);
	_ssock.close();
	_running = false;
	return true;
}

IMainConn* ServiceFrmBase::getMainConn(int index)
{
	return *(_connList.begin() + index);
}

int ServiceFrmBase::getMainConnCount()
{
	return _connList.size();
}

bool ServiceFrmBase::setDialogueCreator(IDialogueCreator* dlgCtor)
{
	if (_dlgCtor)
		return false;

	_dlgCtor = dlgCtor;
	return true;
}

IDialogueCreator* ServiceFrmBase::getDialogueCreator()
{
	return _dlgCtor;
}

IMainConnImpl* ServiceFrmBase::createMainConn(SSLSocket* sock)
{
	assert(sock);
	return new IMainConnImpl(*this, *sock);
}

IMainConnImpl* ServiceFrmBase::accept()
{
	SSLSocket* newsock;
	
	rwlock_lock_read(_connListLock, INFINITE);
	size_t connCount = _connList.size();

	rwlock_unlock_read(_connListLock);

	newsock = _ssock.accept(NULL, NULL, _secure);
	if (newsock == NULL) {
		printDebug("_ssock.accept() failed.\n");
		assert(false);
		return NULL;
	}

	printDebug("_maxCount = %d, connCount = %d\n", 
		_maxConn, connCount);

#if 0
	LINGER linger;
	linger.l_onoff = 1;
	linger.l_linger = 70;
	newsock->setsockopt(SOL_SOCKET, SO_LINGER, (const char *)&linger, 
		sizeof(linger));

	int reuse = 1;
	newsock->setsockopt(SOL_SOCKET, SO_REUSEADDR, 
		(const char *)&reuse, sizeof(reuse));
#endif

	IMainConnImpl* conn = createMainConn(newsock);
	if (conn == NULL) {
		_delete(newsock);
		printFault("ServiceFrmBase::accept(): new IMainConnImpl failed.");
		SF_ASSERT(false);
		return NULL;
	}

	return conn;
}

bool ServiceFrmBase::addMainConn(IMainConnImpl* conn)
{
	if (!conn)
		return false;

#ifdef _DEBUG
	/// 调试状态检查是否该连接已经在列表中,通常这代表有个编程错误发生了
	vector<IMainConn* >::iterator itor = _connList.begin();
	for(; itor != _connList.end(); itor ++) {
		if (*itor == conn) {
			printFault("the connect in the list alread conn = %p", conn);
			return false;
		}
	}
#endif

	size_t connCount;
	rwlock_lock_write(_connListLock, INFINITE);
	_connList.push_back(conn);
	connCount = _connList.size();

#ifdef _BSD
	if (connCount >= _maxConn) {
		_acceptConn = 0;
		_ssock.setsockopt(SOL_SOCKET, SO_ACCEPTCONN, 
			(const char* )&_acceptConn, 
			sizeof(_acceptConn));
	}
#endif
	
	rwlock_unlock_write(_connListLock);
	printDebug("ServiceFrmBase::addMainConn()\tconn = %p, "
		"Connection count = %d", conn, connCount);
	return true;
}

bool ServiceFrmBase::delMainConn(IMainConnImpl* conn)
{
	if (conn == NULL)
		return false;

	size_t connCount;

	bool deleted = false;
	rwlock_lock_write(_connListLock, INFINITE);
	vector<IMainConn* >::iterator itor = _connList.begin();
	for(; itor != _connList.end(); itor ++) {
		if (*itor == conn) {
			_connList.erase(itor);
			connCount = _connList.size();
#ifdef _BSD
			if (_acceptConn && connCount < _maxConn) {
				_acceptConn = 1;
				_ssock.setsockopt(SOL_SOCKET, SO_ACCEPTCONN, 
					(const char* )&_acceptConn, 
					sizeof(_acceptConn));
			}
#endif
			deleted = true;
			break;
		}
	}

	rwlock_unlock_write(_connListLock);

	if (deleted) {
		printDebug("ServiceFrmBase::delMainConn()\tconn = %p, "
			"Connection count = %d", conn, connCount);
	} else {
		
		printWarning("delete main conn but not found. conn = %p", conn);
	}

	return deleted;
}

int ServiceFrmBase::run()
{
	IMainConnImpl* conn;
	_running = true;
	while (true) 
	{
		conn = accept();

		rwlock_lock_read(_connListLock, INFINITE);
		size_t connCount = _connList.size();
		rwlock_unlock_read(_connListLock);

		if (connCount > _maxConn) 
		{			
			conn->close();
			_delete(conn);
			printDebug("the count of connections overed limit");
			continue;
		}

		if (conn != NULL)
		{

			if (!processConn(conn))
			{
				// may it is overload				
				printError("ServiceFrmBase::run(): processConn() failed.");
			}

		} 
		else 
		{
			printError("ServiceFrmBase::run(): accept failed.");
			return -1;
		}
	}

	_running = false;
	_ssock.close();

	return 0;
}


// 动态改变安全属性实现困难
bool ServiceFrmBase::setSecure(bool secure)
{
	return _secure = secure;
}


bool ServiceFrmBase::isSecure()
{
	return _secure;
}

bool ServiceFrmBase::getServiceThread()
{
	return _serviceThread;
}

bool ServiceFrmBase::setServiceThread(bool b)
{
	if (_running)
		return false;
	_serviceThread = b;
	return true;
}

int ServiceFrmBase::getIdleTimeout()
{
	return _idleTimeout;
}

bool ServiceFrmBase::setIdleTimeout(int timeo)
{
	if (_running)
		return false;
	_idleTimeout = timeo;
	return true;
}

uint32 ServiceFrmBase::getMaxConnection()
{
	return _maxConn;
}

bool ServiceFrmBase::setMaxConnection(uint32 maxConn)
{
	if (maxConn >= 1000000) {
		// this is a error? or you must upgrade the performance of frame
		printError("maxConnection is invalid.");
		assert(false);
		return false;
	}

	_maxConn = maxConn;
	return true;
}

uint16 ServiceFrmBase::getThreadCount()
{
	return _threadCount;
}

bool ServiceFrmBase::setThreadCount(uint16 count)
{
	/*
	if (count > 200) {
		// too many thread
		assert(false);
		return false;
	}
	_threadCount = count;
	return true;
	*/

	// 不实现动态改变线程数
	assert(false);
	return false;
}

#ifdef _WIN32_SPECIAL_VERSION
//////////////////////////////////////////////////////////////////////////
// special implementation that base on Win32 IO completion port

class ServiceFrmWin32Special;

class ServiceThreadGroup 
{
public:
	ServiceThreadGroup(ServiceFrmWin32Special& frm, uint16 threadCount);
	virtual ~ServiceThreadGroup();
	bool start();
	bool stop();
	bool requestThread(IMainConnImpl& conn);

protected:
	bool createThreads(uint16 threadCount);
	void destroyThreads();
	virtual DWORD threadProc();
	static DWORD __stdcall _threadProc(PVOID param);

protected:
	ServiceFrmWin32Special&	_serviceFrm;

	HANDLE		_completionPort;
	uint16		_threadCount;
	PHANDLE		_threadHandles;
};

struct SvcCompletionPortKey {
	OVERLAPPED		overlapped;
	IMainConnImpl*	mainConn;
	byte			buf[MAX_RECV_BUFF];
};

ServiceThreadGroup::ServiceThreadGroup(ServiceFrmWin32Special& frm, 
									   uint16 threadCount):
	_serviceFrm(frm), _threadCount(threadCount)
{
	_completionPort = NULL;
	_threadHandles = NULL;
}

ServiceThreadGroup::~ServiceThreadGroup()
{

}

bool ServiceThreadGroup::start()
{
	_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 
		NULL, 0, _threadCount);
	if (!_completionPort) 
	{
		printFault("ServiceThreadGroup::start()\t"
			"CreateIoCompletionPort() failed.\n");
		return false;
	}
	
	if (!createThreads(_threadCount))
		return false;

	return true;
}

bool ServiceThreadGroup::stop()
{
	return false;
}

bool ServiceThreadGroup::requestThread(IMainConnImpl& conn)
{
	//HttpD中不需要在这个地方接收数据
	SOCKET s = (SOCKET )conn.getConnId();
	SvcCompletionPortKey* key = new SvcCompletionPortKey;
	memset(key, 0, sizeof(*key));
	key->mainConn = &conn;
	if (!CreateIoCompletionPort((HANDLE )s, _completionPort, 
		(ULONG_PTR )key, 0))
	{
		printFault("ServiceThreadGroup::requestThread()\t"
						"CreateIoCompletionPort() failed.\n");
		return false;
	}

	DWORD readBytes;
	WSABUF wsaBuf;
	DWORD wsaFlag = 0;
	IDialogue* dlg;
	
	wsaBuf.buf = (char *)key->buf;
	wsaBuf.len = sizeof(key->buf);
	memset(&key->overlapped, 0, sizeof(key->overlapped));
	if (!WSARecv(s, &wsaBuf, 1, &readBytes, &wsaFlag,  
								&key->overlapped, NULL) == 0) 
	{
		if (WSAGetLastError() != ERROR_IO_PENDING) 
		{
			dlg = conn.getDialogue();
			conn.close();
			conn.setDialogue(NULL);
			dlg->onConnectionDestroyed(&conn);
			_serviceFrm.getDialogueCreator()->releaseDialogue(dlg);
			_delete(key);
			printWarning("ServiceThreadGroup::requestThread():\t"
				"WSARecv() failed.");
			return false;
		}
	}
	
	return true;
}

bool ServiceThreadGroup::createThreads(uint16 threadCount)
{
	_threadHandles = new HANDLE[threadCount];
	memset(_threadHandles, 0, sizeof(HANDLE) * threadCount);

	if (!_threadHandles) {
		printFault("ServiceThreadGroup::createThreads()\t"
			"malloc() failed.\n");
		return false;
	}

	for (int i = 0; i < threadCount; i ++) {
		_threadHandles[i] = CreateThread(NULL, 0, &_threadProc, 
			this, 0, NULL);
		if (!_threadHandles[i]) {
			printFault("ServiceThreadGroup::createThreads()\t" 
				"CreateThread() failed.\n");
			destroyThreads();
			return false;
			_delete(_threadHandles);
			_threadHandles = NULL;
		}
	}

	return true;
}

void ServiceThreadGroup::destroyThreads()
{
	// _delete(_threadHandles);
}

// wait for completion port and process request.
DWORD ServiceThreadGroup::threadProc()
{
	DWORD byteCount;
	IMainConnImpl* mainConn;
	LPOVERLAPPED overlapped;
	SvcCompletionPortKey* key;
	DWORD readBytes;
	WSABUF wsaBuf;
	DWORD wsaFlag = 0;
	BOOL rc;
	IDialogue* dlg;
	
	while(true) 
	{
		rc = GetQueuedCompletionStatus(_completionPort, &byteCount, 
											(PULONG_PTR)&key, &overlapped, INFINITE);

		if (overlapped == NULL)
		{
			printFault("occurred a error on the completion prot.");
			break;
		}

		if (!rc || byteCount == 0)
		{
			mainConn = key->mainConn;
			dlg = mainConn->getDialogue();
			mainConn->close();
			mainConn->setDialogue(NULL);
			dlg->onConnectionDestroyed(mainConn);
			_serviceFrm.getDialogueCreator()->releaseDialogue(dlg);
			_delete(key);
			printDebug("ServiceThreadGroup::threadProc():\t", 
					"GetQueuedCompletionStatus() failed.");
			continue;
		}		

		mainConn = key->mainConn;
		SOCKET s = (SOCKET )mainConn->getConnId();

		dlg = mainConn->getDialogue();
		mainConn->modifyAttribute(MAINCONN_CANNOT_RECV, 0);
		try
		{
			dlg->onRequest(mainConn, key->buf, byteCount);
		}
		catch(...)
		{
			printDebug("ServiceReq::run():\tonRequest() occurred a exception.");
		}
		mainConn->modifyAttribute(0, MAINCONN_CANNOT_RECV);

		wsaBuf.buf = (char *)key->buf;
		wsaBuf.len = sizeof(key->buf);
		memset(&key->overlapped, 0, sizeof(key->overlapped));
		if (!WSARecv(s, &wsaBuf, 1, &readBytes, &wsaFlag,  
								&key->overlapped, NULL) == 0)
		{

			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				dlg = mainConn->getDialogue();
				mainConn->close();
				mainConn->setDialogue(NULL);
				dlg->onConnectionDestroyed(mainConn);
				_serviceFrm.getDialogueCreator()->releaseDialogue(dlg);
				_delete(key);
				printDebug("ServiceThreadGroup::threadProc():\t", 
					"WSARecv() failed.");
			}
		}
	}

	return 0;
}

DWORD __stdcall ServiceThreadGroup::_threadProc(PVOID param)
{
	ServiceThreadGroup* me = (ServiceThreadGroup* )param;
	return me->threadProc();
}

//////////////////////////////////////////////////////////////////////////
class _WriteDataEvent {
public:
	_WriteDataEvent()
	{
		_eventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	~_WriteDataEvent()
	{
		CloseHandle(_eventHandle);
	}

	HANDLE		_eventHandle;
};

static _WriteDataEvent writeDataEvent;

class IMainConnImpl2: public IMainConnImpl {
public:
	IMainConnImpl2(ServiceFrmBase& frm, SSLSocket& sock):
	  IMainConnImpl(frm, sock)
	{

	}

	virtual int send(IN const void* buf, IN int size)
	{
		DWORD writtenBytes;
		SOCKET s = (SOCKET )_ssock.getSocket();
		OVERLAPPED overlapped;
		memset(&overlapped, 0, sizeof(overlapped));
		overlapped.hEvent = 
			(HANDLE )((DWORD )writeDataEvent._eventHandle | 0x1);
		WSABUF wsabuf;
		wsabuf.buf = (char* )buf;
		wsabuf.len = size;
		WSASend(s, &wsabuf, 1, &writtenBytes, 0, &overlapped, NULL);
		DWORD sendBytes;
		if (!GetOverlappedResult((HANDLE)s, &overlapped, &sendBytes, TRUE))
			return SOCKET_ERROR;

		return sendBytes;
	}

	virtual int sendTimeout(IN const void* buf, IN int size, IN int timeo)
	{
		DWORD writtenBytes;
		SOCKET s = (SOCKET )_ssock.getSocket();
		OVERLAPPED overlapped;
		memset(&overlapped, 0, sizeof(overlapped));
		overlapped.hEvent = 
			(HANDLE )((DWORD )writeDataEvent._eventHandle | 0x1);
		WSABUF wsabuf;
		wsabuf.buf = (char* )buf;
		wsabuf.len = size;
		WSASend(s, &wsabuf, 1, &writtenBytes, 0, &overlapped, NULL);
		DWORD sendBytes;
		if (WaitForSingleObject((HANDLE)s, timeo) != WAIT_OBJECT_0) {
			return SOCKET_ERROR;
		}

		if (!GetOverlappedResult((HANDLE)s, &overlapped, &sendBytes, FALSE))
			return SOCKET_ERROR;

		return sendBytes;
	}
};


//////////////////////////////////////////////////////////////////////////
// class ServiceFrmWin32Special

ServiceFrmWin32Special::ServiceFrmWin32Special()
{
	_serviceGroup = NULL;
}

ServiceFrmWin32Special::~ServiceFrmWin32Special()
{

}

bool ServiceFrmWin32Special::init(const ServiceConfig* cfg)
{
	if (cfg->_cfg_isSecure) {
		printFault("ServiceFrmWin32Special::init():\t"
			"couldn't support secure at current version.\n");
		return false;
	}
	
	if (!ServiceFrmBase::init(cfg))
		return false;

	if (_serviceGroup == NULL) {
		_serviceGroup = createThreadGroup(_threadCount);
		if (_serviceGroup == NULL) {
			printFault("ServiceFrmWin32Special::init():\t"
				"createThreadGroup() failed.\n");
			return false;
		}
	}

	SetProcessWorkingSetSize(GetCurrentProcess(), 0xa00000, 0x14000000);
	_serviceGroup->start();

	return true;
}

void ServiceFrmWin32Special::uninit()
{
	if (_serviceGroup) {
		_serviceGroup->stop();
		_delete(_serviceGroup);
	}

	ServiceFrmBase::uninit();
}

bool ServiceFrmWin32Special::begin(const ConnID* connID)
{
	if (_running) {
		printFault("ServiceFrmWin32Special::begin(): service is running.");
		SF_ASSERT(false);
		return false;
	}

	if (connID->type != CONN_TYPE_SOCKET_TCP) {
		printError("ServiceFrmWin32Special::begin(): Must be TCP service.");
		SF_ASSERT(false);
		return false;
	}
	
	SOCKET s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,  
		WSA_FLAG_OVERLAPPED);
	if (s == INVALID_SOCKET) {
		printError("ServiceFrmWin32Special::begin(): WSASocket() failed.");
		return false;
	}

	_ssock.attach(s);

#if 0
	LINGER linger;
	linger.l_onoff = 1;
	linger.l_linger = 70;
	_ssock.setsockopt(SOL_SOCKET, SO_LINGER, (const char *)&linger, 
		sizeof(linger));
#endif
	
	if (_ssock.bind(&connID->caddr, connID->addrlen) != 0) {
		printError("ServiceFrmWin32Special::begin(): _ssock.bind() failed.");
		return false;
	}
	
	if (_ssock.listen() != 0) {
		printError("ServiceFrmWin32Special::begin(): _ssock.listen() failed.");
		return false;
	}

	if (_serviceThread)
		return start();
	else {
		run();
		return true;
	}
}

ServiceThreadGroup* ServiceFrmWin32Special::createThreadGroup(uint16 threadCount)
{
	return new ServiceThreadGroup(*this, threadCount);
}

bool ServiceFrmWin32Special::processConn(IMainConnImpl* conn)
{
	IDialogue* dlg;
	dlg = _dlgCtor->createDialogue();
	if (dlg == NULL) 
	{
		printFault("ServiceFrmWin32Special::processConn(): "
			"_dlgCtor->createDialogue() failed.");
		_delete(conn);
		return false;
	}
	conn->setDialogue(dlg);
	conn->onConnected();
	dlg->onConnected(conn);
	return _serviceGroup->requestThread(*conn);

}

IMainConnImpl* ServiceFrmWin32Special::createMainConn(SSLSocket* sock)
{
	assert(sock);
	return new IMainConnImpl(*this, *sock);
}

// special implementation that base on Win32 IO completion port
//////////////////////////////////////////////////////////////////////////
#else // #ifdef _WIN32_SPECIAL_VERSION
//////////////////////////////////////////////////////////////////////////
// Generic implementation

typedef NativeThreadPool ThreadPool;

/* ++
class:
	ServiceReq
Desc:
	线程池请求对象, 代表一个客户请求的执行绪.
-- */

class ServiceReq : public ThreadRequest {
	friend class ThreadMgr;
	friend class ServiceFrmBase;
public:
	ServiceReq(ThreadMgr& threadMgr, ServiceFrmBase& mgr);
	virtual ~ServiceReq();


	/// get count of connection in the this request object.
	int getConnCount();

	/// test overload
	bool isOverload();

protected:
	/// method of super class
	virtual int run();
	virtual bool init(void);
	virtual void final(int retcode = 0, bool bCancelled = false);

	/// add a connection into current req
	/// call by ServiceFrmBase
	int addConn(IMainConnImpl& conn);

	/// clear invalide connection
	int cleanError();

	/// cleck the timeout interval of all connection
	int checkTimeout();

	// int removeConn(IMainConnImpl* conn);	
	
protected:
	PollSSLSocket	_pollSSocks[MAX_CONN_PER_REQ];
	IMainConnImpl*	_mainConns[MAX_CONN_PER_REQ];
	size_t			_pollSSockCount;
	
	ServiceFrmBase&		_service;
	ThreadMgr&		_threadMgr;
	bool			_isOver;
	bool			_lastTimeo;

	//////////////////////////////////////////////////////////////////////////
	
#if !defined(_SPEED_VERSION)
public:
	long			_reqID;
	long			_processTime;

	/// dump this object
	void dump()
	{
		printDebug("ServiceReq dump: _pollSSockCount = %d, _isOver = %d", 
			_pollSSockCount, _isOver);
	}

protected:
#endif // #if !defined(_SPEED_VERSION)

};

/// the thread manager, it's base on thread pool
class ThreadMgr: public ThreadPool {
	friend class ServiceReq;
public:
	ThreadMgr(ServiceFrmBase& service, uint16 threadCount);

	/// insert a request into the thread pool
	bool requestThread(IMainConnImpl& conn);

	/// get the count of thread in thread pool
	int getThreadCount();

	/*
	/// cancel a request.
	bool cancel(ServiceReq* req);
  
	*/
	/// cancel all request in the thread manager
	// void cancelAll();

protected:
	/// 请求开始处理时通知
	virtual void onRequestBegin(ServiceReq* req);

	/// 请求处理完成是通知
	virtual void onRequestEnd(ServiceReq* req);

	/// 获取一个请求对象, 用于负载客户端的连接
	ServiceReq* obtainRequest(bool& isNew);

	/// 获取当前正在处理的请求数
	size_t getThreadRequestCount();

	/// 将一个线程请求加入到池中
	void addReq(ServiceReq* req);

	/// 将一个请求从线程池中删除
	void delReq(ServiceReq* req);
protected:
	ServiceFrmBase&		_service;

	typedef std::vector<ServiceReq* > SrvReqVec;
	SrvReqVec		_reqs;
	Mutex			_reqsMutex;
	int				_nextReq;
};

//
// class ServiceReq
//

ServiceReq::ServiceReq(ThreadMgr& threadMgr, ServiceFrmBase& mgr) :
	ThreadRequest(threadMgr), _threadMgr(threadMgr), _service(mgr)
{

#if !defined(_SPEED_VERSION)
	static volatile long _sID =0;
	_reqID = InterlockedIncrement(&_sID);
	printDebug("ServiceReq::ServiceReq():\tReqID = %d", _reqID);
	_processTime = GetTickCount();
#endif

	threadMgr.addReq(this);
	_isOver = false;
	_pollSSockCount = 0;
	_lastTimeo = true;
}

ServiceReq::~ServiceReq()
{
	_threadMgr.delReq(this);
}

int ServiceReq::run()
{
	byte buf[MAX_RECV_BUFF];
	int size;

	timeval t;

#if !defined(_SPEED_VERSION)
	if (_pollSSockCount == 0) {
		_isOver = true;
		assert(false);
		return 0;
	}
#endif
	
	// calculate the dynamic value of time out
	if (_lastTimeo)
		MS2TV(t, sf_max_polltime / MAX_CONN_PER_REQ * _pollSSockCount);
	else
		MS2TV(t, 0);

	int result = SSLSocket_poll(_pollSSocks, _pollSSockCount, &t);
	
	// error
	if (result < 0) {
		cleanError();
		if (_pollSSockCount == 0)
			_isOver = true;
		return result;
	}
	
	if (result > 0) {
		_lastTimeo = false;
		// cleanError();
		for (size_t i = 0; i < _pollSSockCount; i ++) {
			PollSSLSocket* pollSSock = &_pollSSocks[i];
			assert(pollSSock->revents != SSL_POLLEXCEPTION);
			
			if (pollSSock->revents == SSL_POLLREAD) {

				IMainConnImpl* mainConn = _mainConns[i];
				memset(buf, 0 , sizeof(buf));

				// poll 函数确定已有数据
				size = mainConn->recv(buf, sizeof(buf));
				// size = mainConn->recvTimeout(buf, sizeof(buf), 
				//	SF_RECV_TIMEOUT);

				if (size < 0) {

					// mark it, and delete it in checkError
					pollSSock->revents |= SSL_POLLEXCEPTION;
					// assert(false);

				} else if (size > 0) {

					IDialogue* dlg = mainConn->getDialogue();
					if (dlg) {

						// 阻止在处理时, 服务器内向客户发送信息.
						mainConn->modifyAttribute(MAINCONN_CANNOT_RECV, 0);

						try {

							dlg->onRequest(mainConn, buf, size);
						} catch(...) {

							printDebug(
								"ServiceReq::run():\tonRequest() "
								"occurred a exception.");
						}

						mainConn->modifyAttribute(0, MAINCONN_CANNOT_RECV);
					}

					if (!mainConn->isActive()) {

						// mark it, and delete it in checkError
						pollSSock->revents |= SSL_POLLEXCEPTION;
					}

				} else { // size = 0, connection is closed

					pollSSock->revents |= SSL_POLLEXCEPTION;
					mainConn->close();
				}
			}			
		}

	} else { // result == 0
		_lastTimeo = true;
	}

	// 最后检查一遍错误与超时
	cleanError();
	checkTimeout();

	// have no connection
	if (_pollSSockCount == 0)
		_isOver = true;

	return result;
}

bool ServiceReq::init(void)
{
	_threadMgr.onRequestBegin(this);

#if !defined(_SPEED_VERSION)
	printDebug("ServiceReq::init():\tReqID = %d, processTime = %d", 
		_reqID, GetTickCount() - _processTime);
	_processTime = GetTickCount();
#endif

	return true;
}

void ServiceReq::final(int retcode /* = 0 */, 
					   bool bCancelled /* = false */)
{
#if !defined(_SPEED_VERSION)
	printDebug("ServiceReq::final():\tReqID = %d, processTime = %d" , 
		_reqID, GetTickCount() - _processTime);
	_processTime = GetTickCount();
#endif
	
	/// 一次请求流程已经完成, 通知 thread manager.
	_threadMgr.onRequestEnd(this);
	if (_isOver) {
		printDebug("ServiceReq::final():\treq is over(this = %p)", this);
		_delete(this);
	} else {
#ifdef	_DEBUG
		if (IsBadReadPtr(this, 1))
			DebugBreak();
#endif

		_threadMgr.pushRequest(*this);
	}

}

int ServiceReq::addConn(IMainConnImpl& conn)
{
	ZQ::common::MutexGuard(*this);
	assert(_pollSSockCount < MAX_CONN_PER_REQ);
	if (_pollSSockCount == MAX_CONN_PER_REQ - 1)
		return 0;
	
	_mainConns[_pollSSockCount] = &conn;
	INIT_POLLSSOCK(&_pollSSocks[_pollSSockCount], 
		&conn.getSSLSocket(), SSL_POLLREAD);

	_pollSSockCount ++;
	
	return _pollSSockCount;
}

int ServiceReq::getConnCount()
{
	return _pollSSockCount;
}

bool ServiceReq::isOverload()
{
	return _pollSSockCount == MAX_CONN_PER_REQ - 1;
}

int ServiceReq::cleanError()
{
	common::MutexGuard(*this);

	/// 扫描当前连接列表, 删除已经出错的连接
	IDialogueCreator* dlgctor = _service.getDialogueCreator();

	for (size_t i = 0; i < _pollSSockCount; i ++) {
		if (_pollSSocks[i].revents & SSL_POLLEXCEPTION) {

			IMainConnImpl* mainConn = _mainConns[i];
			assert(mainConn);
			mainConn->close();

			IDialogue* dlg = mainConn->getDialogue();
			if (dlg) {

				mainConn->setDialogue(NULL);
				
				// onConnectionDestroyed 完成后 mainConn 可能已经释放
				dlg->onConnectionDestroyed(mainConn);

				// releaseDialogue 后 dlg 不可用
				dlgctor->releaseDialogue(dlg);
			}
			
			if (i < _pollSSockCount - 1) {
				int lastCount = (_pollSSockCount - i - 1);
				memmove(&_mainConns[i], &_mainConns[i + 1], 
					sizeof(_mainConns[i]) * lastCount);

				memmove(&_pollSSocks[i], &_pollSSocks[i + 1], 
					sizeof(_pollSSocks[i]) * lastCount);
				i --;
			}

			_pollSSockCount --;
		}
	}

	return _pollSSockCount;
}

int ServiceReq::checkTimeout()
{
	common::MutexGuard(*this);
	IDialogueCreator* dlgctor = _service.getDialogueCreator();
	IMainConnImpl* mainConn;

	for (size_t i = 0; i < _pollSSockCount; i ++) {

		mainConn = _mainConns[i];

		if (mainConn->getIdleTime() >= sf_idle_timeout) {

			IDialogue* dlg = mainConn->getDialogue();
			if (dlg) {

				dlg->onIdlenessTimeout(mainConn);
				if (!mainConn->isActive()) {
					mainConn->setDialogue(NULL);
					
					// onConnectionDestroyed 完成后 mainConn 可能已经释放
					dlg->onConnectionDestroyed(mainConn);

					// releaseDialogue 后 dlg 不可用
					dlgctor->releaseDialogue(dlg);					
				} else {
					// 没有关闭, 更新 idle time
					mainConn->updateIdleTime();
					continue;
				}

			}
			
			if (i < _pollSSockCount - 1) {
				int lastCount = (_pollSSockCount - i - 1);
				memmove(&_mainConns[i], &_mainConns[i + 1], 
					sizeof(_mainConns[i]) * lastCount);

				memmove(&_pollSSocks[i], &_pollSSocks[i + 1], 
					sizeof(_pollSSocks[i]) * lastCount);
				i --;
			}
			
			_pollSSockCount --;

		}
	} // end for
	
	return 0;
}


//
// class ThreadMgr
//

ThreadMgr::ThreadMgr(ServiceFrmBase& service, uint16 threadCount):
	_service(service), ThreadPool(threadCount)
{
	_nextReq = 0;
}

ServiceReq* ThreadMgr::obtainRequest(bool& isNew)
{
	common::MutexGuard guard(_reqsMutex);

	// 在请求数小于线程数时总是新建请求
	if ((size_t )size() < _reqs.size()) {
		if ((size_t )_nextReq >= _reqs.size())
			_nextReq = 0;

		SrvReqVec::iterator itor;
		itor = _reqs.begin() + _nextReq;
		if (!(*itor)->isOverload()) {
			isNew = false;
			_nextReq ++;
			return *itor;
		}

		_nextReq = 0;
		for (itor = _reqs.begin(); itor != _reqs.end(); itor ++) {
			if (!(*itor)->isOverload()) {
				isNew = false;
				_nextReq ++;
				return *itor;
			}
			_nextReq ++;
		}
	}
	
	ServiceReq* req = new ServiceReq(*this, _service);
	printDebug("ThreadMgr::obtainRequest(): new req = %p, req count = %d", 
		req, _reqs.size());
	isNew = req != NULL;
	return req;
}

void ThreadMgr::addReq(ServiceReq* req)
{
	common::MutexGuard guard(_reqsMutex);
	_reqs.push_back(req);
}

void ThreadMgr::delReq(ServiceReq* req)
{
	common::MutexGuard guard(_reqsMutex);
	SrvReqVec::iterator itor;
	for (itor = _reqs.begin(); itor != _reqs.end(); itor ++) {
		if (*itor == req) {
			_reqs.erase(itor);
			break;
		}
	}
}

size_t ThreadMgr::getThreadRequestCount()
{
	common::MutexGuard guard(_reqsMutex);
	return _reqs.size();
}

bool ThreadMgr::requestThread(IMainConnImpl& conn)
{
	MutexGuard guard(*this);
	bool isNew;
	ServiceReq* req;
	while (true) 
	{
		req = obtainRequest(isNew);
		if (req == NULL)
			return false;

		if (req->addConn(conn))
			break;
	}
	
	if (isNew) 
	{
#ifdef	_DEBUG
		if (IsBadReadPtr(this, 1))
			DebugBreak();

		if (IsBadReadPtr(req, 1))
			DebugBreak();
#endif
		// req->start();
		pushRequest(*req);
	}

	return true;
}

int ThreadMgr::getThreadCount()
{
	return this->size();
}

void ThreadMgr::onRequestBegin(ServiceReq* req)
{
#if !defined(_SPEED_VERSION)
	printDebug("ThreadMgr::onRequestBegin(): Req = %d", req->_reqID);
#endif
}

void ThreadMgr::onRequestEnd(ServiceReq* req)
{

#if !defined(_SPEED_VERSION)
	printDebug("ThreadMgr::onRequestBegin(): Req = %d", req->_reqID);
#endif
}

/*
bool ThreadMgr::cancel(ServiceReq* req)
{
	SF_ASSERT(false);
	printDebug("ThreadMgr::cancel(): no implementation.");
	return false;
}
*/

/*
void ThreadMgr::cancelAll()
{
	printDebug("ThreadMgr::cancelAll().");
	_requests.empty();
}
*/

//////////////////////////////////////////////////////////////////////////
// class ServiceFrmGeneric

ServiceFrmGeneric::ServiceFrmGeneric()
{
	_threadMgr = NULL;
}

ServiceFrmGeneric::~ServiceFrmGeneric()
{
	if (_threadMgr)
		_delete(_threadMgr);
}

bool ServiceFrmGeneric::init(const ServiceConfig* cfg)
{
	if (!ServiceFrmBase::init(cfg))
		return false;

	if (_threadMgr == NULL)
		_threadMgr = createThreadMgr();
	if (_threadMgr == NULL) {
		printFault("createThreadMgr() failed.\n");
		return false;
	}

	return true;
}

void ServiceFrmGeneric::uninit()
{
	if (_threadMgr) {
		_delete(_threadMgr);
		_threadMgr = NULL;
	}

	ServiceFrmBase::uninit();
}

ThreadMgr* ServiceFrmGeneric::createThreadMgr()
{
	return new ThreadMgr(*this, _threadCount);
}

bool ServiceFrmGeneric::processConn(IMainConnImpl* conn)
{
	IDialogue* dlg;

	dlg = _dlgCtor->createDialogue();
	if (dlg == NULL) {
		printFault("ServiceFrmGeneric::processConn(): "
			"_dlgCtor->createDialogue() failed.");
		return false;
	}
	conn->setDialogue(dlg);
	conn->onConnected();
	dlg->onConnected(conn);
	return _threadMgr->requestThread(*conn);
}

#endif // #ifdef _WIN32_SPECIAL_VERSION

#ifndef _NO_NAMESPACE
}
#endif // #ifndef _NO_NAMESPACE
