#ifndef _SERVICEFRAME_H_
#define _SERVICEFRAME_H_
#if _MSC_VER > 1000
#pragma once
#endif // #if _MSC_VER > 1000

/*#include "commonFile.h"*/
#include "NativeThread.h"
#include "NativeThreadPool.h"
#include "XMLPreferenceEx.h"
#include "rwlock.h"
#include <vector>
#include <string>

extern "C"
{
	//#include <WinSock2.h>
#include <time.h>
// #include <assert.h>
}

#include <ssllib.h>
#include <list>
struct ConnID 
{
	int				type;
	int				addrlen;
	union
	{
		sockaddr	caddr;
		TCHAR		cname[MAX_PATH];
	};
};

struct PeerInfo 
{
	ConnID		addr;	// address of peer
	tm			ct;		// time of connection
};

class IConn 
{
public:
	/// receive a chuck of data from this connection
	///@param[out] buf the buf to receive data
	///@param[in] maxlen the max length in byte that the buffer can receive data
	///@return number of bytes have been received
	virtual int recv(OUT void* buf, IN int size) = 0;
	
	/// receive a chuck of data from this connection with timeout
	///@param[out] buf		the buf to receive data
	///@param[in] maxlen	the max length in byte that the buffer can receive data
	///@param[in] timeo		timeout interval
	///@return number of bytes have been received
	virtual int recvTimeout(OUT void* buf, IN int size, IN int timeo) = 0;
	
	/// send a chuck of data thru this connection
	///@param[in] buf data to send
	///@param[in] maxlen the number of bytes to send
	///@return number of bytes have been sent
	virtual int send(IN const void* buf, IN int size) = 0;
	
	/// send a chuck of data thru this connection with time-out
	///@param[in] buf		data to send
	///@param[in] maxlen	the number of bytes to send
	///@param[in] timeo		timeout interval
	///@return number of bytes have been sent
	virtual int sendTimeout(IN const void* buf, IN int size, IN int timeo) = 0;
	
	/// close the connection.
	///@return 0 if no error occurs, otherwise SOCKET_ERROR
	virtual int close() = 0;
	
	/// upgrade the TCP connection to SSL connection
	///param[in] timeout	time-out interval
	virtual int upgradeSecurity(int timeout) = 0;
	
	/// degread the SSL connection to TCP connection
	virtual int degradeSecurity() = 0;
	
	/// check secure status
	virtual bool isSecure() = 0;
	
	/// check active flase
	virtual bool isActive() = 0;
	
	/// release this object
	virtual void release() = 0;
	
	virtual int getConnId() = 0;
};


//////////////////////////////////////////////////////////////////////////
// implemented by SERVICE FRAME

#define MAX_RECV_BUFF				1024
#define SF_IDLE_TIMEOUT				300000	/* 5 minute */
// #define SF_IDLE_TIMEOUT				30000	/* 30 second */
#define	MAX_CONN_PER_REQ			FD_SETSIZE
#define CONN_TYPE_SOCKET_TCP		0
#define CONN_TYPE_SOCKET_UDP		1
#define CONN_TYPE_PIPE				2


#define CONN_MODE_PASSIVE		0
#define CONN_MODE_ACTIVE		1

struct IDialogue;
class IMainConn: public IConn 
{
public:
	/// get peer information
	///@patam[in] info		output the peer infomation
	virtual int getPeerInfo(OUT PeerInfo* info) = 0;

	/// when the connection becomes active status, this method will call
	virtual void onConnected() = 0;

	/// crate a additive connection for multi channel
	virtual IConn* createConn(IN const ConnID* addr, 
		IN int mode, bool inheritSec, IN int timeout) = 0;

	/// destroy a non-main conneciton
	virtual void destroyConn(IN IConn* conn) = 0;

	/// get dialogue object from main connection
	virtual IDialogue* getDialogue() = 0;
	
	virtual unsigned __int64 getConnectionIdentity() =0;
};

//////////////////////////////////////////////////////////////////////////
// implemented by protocol layer
using namespace ZQ::common;
/// Э��ʵ�ֲ�ʹ�øýӿ��� ServiceFrame �Խ�
struct IDialogue {
	/// when service frame get a income connection, 
	/// it notice the protocal layer
	///@param[IN] conn		new main connection object
	virtual void onConnected(IN IMainConn* conn) = 0;

	/// when a request arrived, service notice the protocal layer
	///@param[in] conn		main connection object who is there a new request
	///@param[in] buf		request package data
	///@param[in] size		size fo the request package
	virtual void onRequest(IN IMainConn* conn, IN const void* buf, IN int size) = 0;

	/// when a dialgue was timeout
	/// service frame notice the protocal layer
	///@param[in] conn		main connection object
	virtual void onIdlenessTimeout(IN IMainConn* conn) = 0;

	/// when a main connection destroyed, service notice the protocol layer
	///@param[in] conn		main connection object
	virtual void onConnectionDestroyed(IN IMainConn* conn) = 0;
};

struct IDialogueCreator {
	/// crate a dialogue object.
	///@return the a dialogue object
	virtual IDialogue* createDialogue() = 0;
	
	/// release a dialogue object
	///@param[in] dlg	this dialogue object will release
	virtual void releaseDialogue(IN IDialogue* dlg) = 0;
};

//////////////////////////////////////////////////////////////////////////
// some implementation
#define SVCDBG_LEVEL_MIN				SVCDBG_LEVEL_DISABLE
#define SVCDBG_LEVEL_DISABLE			0
#define SVCDBG_LEVEL_FAULT				1
#define SVCDBG_LEVEL_ERROR				2
#define SVCDBG_LEVEL_WARNING			3
#define SVCDBG_LEVEL_NOTICE				4
#define SVCDBG_LEVEL_DEBUG				5
#define SVCDBG_LEVEL_MAX				SVCDBG_LEVEL_DEBUG

#define printFault						svcdbg_printFault
#define printError						svcdbg_printError
#define printWarning					svcdbg_printWarning
#define printNotice						svcdbg_printNotice
#define printDebug						svcdbg_printDebug

#ifdef _DEBUG
#define	SF_ASSERT(b)					assert(b);
#else
#define	SF_ASSERT(b)
#endif

#ifndef _NOTRACE
void svcdbg_setDebugOstream(ostream& ostrm);
int svcdbg_dumpInternalError();

void svcdbg_printFault(const char* format, ...);
void svcdbg_printError(const char* format, ...);
void svcdbg_printWarning(const char* format, ...);
void svcdbg_printNotice(const char* format, ...);
void svcdbg_printDebug(const char* format, ...);

int svcdbg_setLevel(int level);
int svcdbg_getLevel();
void svcdbg_resetCounts();
int svcdbg_getCount(int level);

#else // #ifndef _NOTRACE

#define svcdbg_setDebugOstream(ostrm)
#define svcdbg_dumpInternalError()			0

#define svcdbg_printFault
#define svcdbg_printError
#define svcdbg_printWarning
#define svcdbg_printNotice
#define svcdbg_printDebug

#define svcdbg_setLevel(n)					(SVCDBG_LEVEL_MAX + 1)
#define svcdbg_getLevel()					(SVCDBG_LEVEL_MAX + 1)
#define svcdbg_resetCounts()
#define svcdbg_getCount(n)					0

#endif // #ifndef _NOTRACE

/// �����������ù�����
class ServiceConfig {
public:
	
	/// ������Ϣ����������
	struct _ConfigEntry {
		const char* key;
		enum {
			INVALID_TYPE, 
			STRING_VALUE, 
			SHORT_VALUE, 
			USHORT_VALUE, 
			LONG_VALUE,
			ULONG_VALUE,
			DISPATCH_ENTRY, 
		} type;
		
		void* value;
		
		int	maxValue;
	};

	ServiceConfig();
	virtual ~ServiceConfig();

	/// ��������ñ�
	virtual const _ConfigEntry* getBaseConfigMap();

	/// ��ǰ���ñ�
	virtual const _ConfigEntry* getConfigMap();

	/// ����һ�������ļ�
	virtual bool load(const char* fileName,std::vector<std::string>& path=std::vector<std::string>());

protected:
	/// ���������ļ�
	bool processEntry(XMLPreferenceEx* pref, const _ConfigEntry* cfgEntry);
public:
	int32		_cfg_debugLevel;
	uint16		_cfg_isSecure;
	uint16		_cfg_threadCount;
	uint32		_cfg_maxConn;
	char		_cfg_publicKeyFile[MAX_PATH];
	char		_cfg_privateKeyFile[MAX_PATH];
	char		_cfg_privateKeyFilePwd[64];
	char		_cfg_dhParamFile[MAX_PATH];
	char		_cfg_randFile[MAX_PATH];
};

//typedef NativeThread Thread;
class IMainConnImpl;

class ServiceFrmBase: protected NativeThread {
	friend class IMainConnImpl;
public:
	ServiceFrmBase();
	virtual ~ServiceFrmBase();

	/// ��ʼ�� service frame
	virtual bool init(const ServiceConfig* cfg);
	virtual void uninit();

	/// ��ʼ����
	virtual bool begin(const ConnID* connID);

	/// ��������
	virtual bool end();

	/// �õ��ܹ���������
	int getMainConnCount();
	
	/// �õ�ָ��������
	IMainConn* getMainConn(int index);

	/// dialogCreator ����
	bool setDialogueCreator(IDialogueCreator* dlgCtor);
	IDialogueCreator* getDialogueCreator();

	/// �������İ�ȫ����. ��ȫ������ʱ,ʹ�� SSL ��Ϊͨ���ŵ�
	bool setSecure(bool secure);
	bool isSecure();

	/// �������Ƿ��Ե������߳̿�ʼ���շ���
	bool getServiceThread();
	bool setServiceThread(bool b);

	/// ����ʱ����ļ��
	int getIdleTimeout();
	bool setIdleTimeout(int timeo);
	
	/// ���������
	uint32 getMaxConnection();
	bool setMaxConnection(uint32 maxConn);
	
	/// �̳߳ص��߳���
	uint16 getThreadCount();
	bool setThreadCount(uint16 count);

protected:
	// NativeThread methods
	virtual int run(); 

	virtual IMainConnImpl* createMainConn(SSLSocket* sock);

	/// ���ڽ������Կͻ�������
	IMainConnImpl* accept();

	/// ���������б�
	bool addMainConn(IMainConnImpl* conn);
	bool delMainConn(IMainConnImpl* conn);

	/// ��������
	virtual bool processConn(IMainConnImpl* conn) = 0;

	bool initByConfiguration(const ServiceConfig* cfg);
	
protected:
	
	vector<IMainConn* >			_connList;
	_rwlock_t					_connListLock;
	IDialogueCreator*			_dlgCtor;
	SSLContext					_sslCtx;
	SSLSocket					_ssock;
	bool						_secure;
	bool						_running;
	int							_idleTimeout;
	bool						_serviceThread;
	uint32						_maxConn;
	int32						_acceptConn;
	uint16						_threadCount;

};

#ifdef _WIN32_SPECIAL_VERSION

class ServiceThreadGroup;

class ServiceFrmWin32Special: public ServiceFrmBase {
public:
	ServiceFrmWin32Special();
	virtual ~ServiceFrmWin32Special();
	virtual bool init(const ServiceConfig* cfg);
	virtual void uninit();
	virtual bool begin(const ConnID* connID);

protected:
	virtual IMainConnImpl* createMainConn(SSLSocket* sock);
	virtual ServiceThreadGroup* createThreadGroup(uint16 threadCount);
	virtual bool processConn(IMainConnImpl* conn);

protected:
	ServiceThreadGroup*	_serviceGroup;
};

typedef ServiceFrmWin32Special ServiceFrm;

#else // #ifdef _WIN32_SPECIAL_VERSION

class ThreadMgr;

class ServiceFrmGeneric: public ServiceFrmBase {
	friend class ServiceReq;
public:
	ServiceFrmGeneric();
	virtual ~ServiceFrmGeneric();
	virtual bool init(const ServiceConfig* cfg);
	virtual void uninit();

protected:

	/// �̹߳������Ĺ�������
	virtual ThreadMgr* createThreadMgr();
	virtual bool processConn(IMainConnImpl* conn);

protected:
	ThreadMgr*					_threadMgr;
};

typedef ServiceFrmGeneric ServiceFrm;

#endif // #ifdef _WIN32_SPECIAL_VERSION

#endif // #ifndef _SERVICEFRAME_H_
