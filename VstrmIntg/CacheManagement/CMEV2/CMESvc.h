#ifndef _CMESVC_SERVICE_H_
#define _CMESVC_SERVICE_H_

#ifdef ZQ_OS_LINUX
#include <dirent.h>
#endif

#include "SystemUtils.h"
#include "ZQ_common_conf.h"

#ifdef ZQ_OS_MSWIN
#include "BaseZQServiceApplication.h"
#else
#include "ZQDaemon.h"
#endif

#include "CMELAMSOAPClient.h"
#include "CMESOAPServiceThread.h"
#include "CacheStorage.h"
#include "VSISCacheIO.h"
#include "CMECacheAlgorithm.h"


namespace CacheManagement {

class CMEService;
class VSISConnFactory;

class CMESvc : public ZQ::common::BaseZQServiceApplication 
{
	friend class CMEService;
public:
	CMESvc ();
	virtual ~CMESvc ();
	
public:
	
	HRESULT OnStart(void);
	HRESULT OnStop(void);
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);
	virtual void OnSnmpSet(const char*);

private:
	CMEService*				_pCMEMain;

public: 
	CMEService&		getCMEMain()  { return *_pCMEMain; }
};

class Session
{
public:
	Session() {};
	virtual ~Session() {};
public:
	enum  { SESSION_START=1, SESSION_STOP=2 };
	std::string		_clusterID;		// cluster id
	std::string		_sid;			// Session ID (for all func's)
	std::string		_paid;			// Provider Asset ID
	std::string		_pid;			// Provider ID
	int64		    _startTime;		// Time of session start. If a session did not receive the max 2*playtime, it will be removed, means the session end event lost
};

typedef std::map<std::string, Session*> SESSIONS;
typedef std::map<std::string, VSISCacheIO*>	 CACHE_VSISIOS;

class ProactiveRecord
{
public:
	ProactiveRecord() {};
	virtual ~ProactiveRecord() {};

public:
	std::string		_pid;
	std::string     _paid;
	std::string     _clusterID;
	uint64          _startTime;		// local time
	uint32          _liftime;       // seconds
};
typedef std::map<std::string, ProactiveRecord*> PROACTIVE_RECORDS;
typedef std::vector<ProactiveRecord*> PROACTIVE_ARRAY;

// The purpose of this thread (instead of service app - CMEV2Svc) is:
// - to get LAM configuration after service started, and retry if until succeeded 
// with this, this thread turned to be kind of the service main thread

class CMEService : public ZQ::common::NativeThread
{
public: 
	CMEService();
	virtual ~CMEService();

public:
	bool		start( );	
	void		stop( );

protected:
	bool		init(void);
	virtual int	run(void);
	void		final(void);

private:
	bool				    _threadRunnng;
	SYS::SingleObject		_waitEvent;

protected:
	CMECacheAlgorithm       *_cmeAlgorithm;
	CMELAMSOAPClient		*_pLamSOAPClient;
	CMESOAPServiceThread	*_pCMESOAPServer;

	LAMCLUSTERS				_lamClusters;
	CACHE_STORAGES			_cacheStorages;
	CACHE_VSISIOS			_cacheIOs;

	ZQ::common::FileLog		_cacheFileLog;
protected:
	SESSIONS			_sessions;
	ZQ::common::Mutex	_sessLock;

	CacheStorage* findCacheStorage(std::string& name);
	void printHitrate();
	void printImportBW();
	void checkSessions();
public:
	bool sessionArrive(std::string& cluid, int func, std::string& pid, std::string& paid, std::string& sid, std::string& timestamp);
	bool proactiveImport(std::string& cluid, std::string& pid, std::string& paid, int increment, int lifetime, uint64 startTime=0);
	bool proactiveDelete(std::string& cluid, std::string& pid, std::string& paid);

public:
	std::string             _proactiveRecordFile;	// data file

	PROACTIVE_RECORDS       _proactiveRecords;

	bool checkNewXmlFile(std::string& path);
	bool parseProactiveXml(std::string& xmlfile);
	void submitProactiveImports();
	bool loadProactiveRecords(std::string& filename);
	bool flushProactiveRecords(std::string& filename);
};

}
#endif