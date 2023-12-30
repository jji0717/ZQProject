#ifndef __ZQTianShan_C2Env_H__
#define __ZQTianShan_C2Env_H__

#include <ZQ_common_conf.h>
#include "CDNDefines.h"

//include ice header
#include "SessionIdx.h"

//include tianshan header
#include "TianShanDefines.h"
#include "EventChannel.h"

//include project header
#include "C2Factory.h"

#include "C2LocatorConf.h"

#include "SystemUtils.h"

#include "../SimpleXMLParser.h"

#include <TimeUtil.h>

#include <auth5i.h>
#include "ZQSnmp.h"

namespace ZQTianShan{
namespace CDN{

class InMemoryObjects: public virtual Ice::ServantLocator
{
public:
    explicit InMemoryObjects(const std::string& category);
    const std::string& category() const;

    virtual Ice::ObjectPtr locate(const Ice::Current& c,
                                  Ice::LocalObjectPtr& cookie);
    virtual void finished(const Ice::Current& c,
                          const Ice::ObjectPtr& servant,
                          const Ice::LocalObjectPtr& cookie);
    virtual void deactivate(const std::string& category);

    Ice::Identity add(Ice::ObjectPtr obj);
    Ice::ObjectPtr remove(const Ice::Identity& ident);
    void clear();
public:
    std::string category_;
    ZQ::common::Mutex lock_;
    typedef std::map<std::string, Ice::ObjectPtr> Objects;
    Objects objs_;
};
typedef IceInternal::Handle<InMemoryObjects> InMemoryObjectsPtr;

// help class for id generating
class IdGen {
public:
    IdGen(int from = 1):next_(from) {
        seed_ = ZQ::common::now();
    }
    std::string create() {
        ZQ::common::MutexGuard gd(lock_);
        char buf[64];
        sprintf(buf, FMT64"-%04d", seed_, next_++);
        return buf;
    }
private:
    ZQ::common::Mutex lock_;
    int next_;
    int64 seed_;
};

#define MAX_TICKET_LEASETERM				(6*3600*1000)	// 6hour
#define DEFAULT_TICKET_LEASETERM	        (60*1000)		// 60s
#define MIN_TICKET_LEASETERM                (10*1000)       // 10s

#define DEFAULT_ENDPOINT_C2Locator "default -p 10001"
#define ADAPTER_NAME_C2Locator "C2Locator"

#define TransferSessionCategory "TransferSession"

struct PortPerfData {
    Ice::Long totalBw;
    Ice::Long activeBw;
    int32 count;
    int32 sessions;

    void clear() {
        totalBw = 0;
        activeBw = 0;
        count = 0;
        sessions = 0;
    }
    PortPerfData() { clear(); }
};

struct ClientPerfData {
    int32 count;

    void clear() {
        count = 0;
    }
    ClientPerfData() { clear(); }
};
class HitCounter {
public:
    HitCounter():hitCountTotal_(0), hitCountLocal_(0){}
    void recordHit(bool isLocal) {
        ZQ::common::MutexGuard guard(lock_);
        hitCountTotal_ += 1;
        if(isLocal) {
            hitCountLocal_ += 1;
        }
    }
    double getHitRate() const {
        ZQ::common::MutexGuard guard(lock_);
        if(hitCountTotal_ > 0) {
            return (double)hitCountLocal_ / (double)hitCountTotal_;
        } else {
            return 0.0;
        }
    }
    void reset() {
        ZQ::common::MutexGuard guard(lock_);
        hitCountTotal_ = 0;
        hitCountLocal_ = 0;
    }
private:
    ZQ::common::Mutex lock_;
    int64 hitCountTotal_;
    int64 hitCountLocal_;
};

class C2Env;

class RequestAuth {
public:
	RequestAuth( C2Env& env );
	~RequestAuth( );
	bool loadKeyfile( const std::string& keyfile );
	bool auth( const SimpleXMLParser::Node* root, const std::string& sessId );

private:
	bool getSessionContent( const SimpleXMLParser::Node* session, const std::string& sessId,
		std::string& paid, std::string& pid, std::string& txnId, 
		std::string& clientIp, std::string& clientSess,
		std::string& expiration, std::string& signature);

private:
	C2Env&				_env;
	ZQ::common::Log&	_log;
	Authen5i			mAuth;	
};

class TransferPortManager;
class ClientManager;
class C2Env
{
public:
    C2Env(::ZQ::common::FileLog& filelog, ::ZQ::common::FileLog& icelog, ::ZQ::common::NativeThreadPool& threadPool, ::Ice::CommunicatorPtr& communicator, C2LocatorConf& conf);
	~C2Env();

	void initialize();
	void uninitial();
    void setClientManager(ClientManager* clientMgr){ _clientMgr = clientMgr;}
    void setPortManager(TransferPortManager* portMgr){ _portMgr = portMgr;}

    C2LocatorConf& _conf;
	//for configurations
	::std::string _dbPath;          //database dir
	::std::string _endpoint;        //ice endpoint
	::std::string _dbRuntimePath;   //runtime database dir
	::std::string _programRootPath; //program root dir
	int32 iTransferSessionTimeout;
    // transfer option
    int32 replicaReportIntervalSec; // interval of the transfer port info update

    // test content info
    bool ignoreLamWithTestContent;
    ContentInfo testContent;

	//for ice
	C2Factory::Ptr                     _factory;
	Freeze::EvictorPtr                 _eC2TransferSession;
	::Ice::CommunicatorPtr             _communicator;
	ZQADAPTER_DECLTYPE                 _adapter;
	::TianShanIce::SCS::SessionIdxPtr  _idxSessionIdx;//session index ptr

    // for timers
    InMemoryObjectsPtr timers_;

	//for multi-thread use
	::ZQ::common::NativeThreadPool&     _pool;
	::ZQ::common::FileLog*              _pLog;
    ::ZQ::common::FileLog*              _pIceLog;

	//timer watch dog
	::ZQTianShan::TimerWatchDog         _watch;

	RequestAuth							_auth;

	::std::string						_eventChannelEndpoint;
	::TianShanIce::Events::EventChannelImpl::Ptr _pEventChannel;

    // expired session notification
    SYS::SingleObject hExpiredSessionNotifier;

    // request id simulator
    IdGen reqIdGen;

    // global performance data
    PortPerfData portPerf;
    ClientPerfData clientPerf;
    HitCounter hitCounter;
private:
    ClientManager*         _clientMgr;
    TransferPortManager*   _portMgr;
    ZQ::SNMP::ModuleMIB                     _mmib;
    ZQ::SNMP::SubAgent                      _snmpSA;

    ZQ::common::Mutex                       _registeresLock;
    std::vector<std::string>                _registeres;
    int                                     _portCount;

public:
    void updatePort(TianShanIce::SCS::TransferPort port);

    void   registerSnmpVariables();
    void   refreshTransferPortTable();
    void   refreshClientTransferTable();

    uint32 snmp_getLogLevel_Main() { return _pLog->getVerbosity(); }
    void   snmp_setLogLevel_Main(const uint32& newLevel) { _pLog->setLevel(newLevel); }
    uint32 snmp_getLogLevel_Ice() { return _pIceLog->getVerbosity(); }
    void   snmp_setLogLevel_Ice(const uint32& newLevel) { _pIceLog->setLevel(newLevel); }

    uint32 snmp_getHitRate(){  return (uint32)(hitCounter.getHitRate() * 100); }
    void   snmp_setHitRate(const uint32&) { hitCounter.reset(); }

    void   snmp_c2locPortTable(TianShanIce::SCS::TransferPort& info, uint32 index);

public:
    void reportSessionExpired(const Ice::Identity& ident);
    std::vector<Ice::Identity> getExpiredSessions();
private:
    ZQ::common::Mutex lockExpiredSessions;
    std::vector<Ice::Identity> expiredSessions;
protected:
	bool openDB(const char* databasePath = NULL,const char* dbRuntimePath=NULL);
	void closeDB(void);
	void updateIceProperty(Ice::PropertiesPtr iceProperty , const std::string& key ,	const std::string& value );
};

#define DBFILENAME_C2TransferSession   "C2Locator"
#define INDEXFILENAME(_IDX)     #_IDX"Idx"

#define envlog if (_pLog) (*_pLog)

#define ReqLOGFMT(_MOD, _X) CLOGFMT(_MOD, "[req-%s] " _X), reqId.c_str()




}// namespace CDN
}// namespace ZQTianShan
#endif //__ZQTianShan_C2Env_H__
