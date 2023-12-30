#ifndef __TianShanIce_C2LocatorImpl_H__
#define __TianShanIce_C2LocatorImpl_H__

#include "ZQ_common_conf.h"
#include "TransferSessionImpl.h"
#include "ClientManager.h"
#include "TransferPortManager.h"
// #include "C2Snmp.h"

//TransferSession Event Name
#define EVENTCategory "TransferSession"
#define TransferSessionUpdate "TransferSessionUpdate"
#define IngressCapcityUpdate  "IngressCapcityUpdate"

//TransferSession Event Key
#define Event_Client           "Client"
#define Event_TransferId       "TransferId"
#define Event_State            "State"
#define Event_IngressCapacity  "IngressCapacity"

//TransferSession Exception Code
#define Ice_SocketException    (-1)
#define Ice_TimeoutException   (-2)
#define Unknown_Exception      (-3)

namespace TianShanIce{
namespace SCS{

class StreamEventHandler;
typedef IceInternal::Handle<StreamEventHandler> StreamEventHandlerPtr;


typedef std::map<std::string, Ice::Identity> TransferIdMap;

class C2LocatorImpl : public C2Locator, //public ::IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>,
                                          public ICEAbstractMutexRLock, public ::ZQ::common::NativeThread
{
public:
	C2LocatorImpl(::ZQTianShan::CDN::C2Env &env); //, PortSnmpManager& portSnmpMgr);
	~C2LocatorImpl();
    typedef IceInternal::Handle<C2LocatorImpl> Ptr;

	//impl of local function
    bool commit(const std::string& reqId, ::TianShanIce::SCS::TransferSessionPrx transferSession, int& err);
	void destroy(::Ice::Identity &sessIdent);

	void InitEventSink();

	//impl of C2Locator

	virtual TransferSessionPrx openSessionByTransferId(const ::std::string& transferId, const ::Ice::Current& c);

    virtual ClientTransfers listClients(const Ice::Current& c = Ice::Current());

    virtual TransferPorts listTransferPorts(const Ice::Current& c = Ice::Current());

    virtual TransferSessions listSessionsByClient(const std::string& client, const Ice::Current& c = Ice::Current());

    virtual TransferSessions listSessionsByPort(const std::string& port, const Ice::Current& c = Ice::Current());

    virtual void updatePortsAvailability(const TianShanIce::StrValues& ports, bool enabled, const Ice::Current& c = Ice::Current());

	 virtual void OnRestore(const ::Ice::Current& c);
	
	//impl of BaseService
	virtual ::std::string getAdminUri(const ::Ice::Current& c);

	virtual ::TianShanIce::State getState(const ::Ice::Current& c);

	//impl of ReplicaSubscriber
	virtual void updateReplica_async(const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr& amd, const ::TianShanIce::Replicas& replicas, const ::Ice::Current& c);

	//impl of TimeoutObj
	//virtual void OnTimer(const ::Ice::Current& c);

	//impl of ZQ NativeThread
	virtual int run(void);

    ZQTianShan::CDN::ClientManager& getClientManager();
    ZQTianShan::CDN::TransferPortManager& getTransferPortManager();

	//impl of GenericEventSink
	virtual void ping(::Ice::Long timestamp, const ::Ice::Current& c){}

	virtual void post(const ::std::string& category, ::Ice::Int eventId, const ::std::string& eventName, const ::std::string& stampUTC, const ::std::string& sourceNetId, const ::TianShanIce::Properties& params, const ::Ice::Current& c);

    bool querySessionByStream(const std::string& streamId, Ice::Identity& sess) const;
private:
    void addStreamIndex(const std::string& streamId, const Ice::Identity& sess);
    void removeStreamIndexBySession(const Ice::Identity&);

    // remove the transfer session info from the in-memory index
	void removeTransferSession(const Ice::Identity &);

    bool checkRemoteHealth(const std::string& hostName);
private:
	::ZQTianShan::CDN::C2Env& _env;
    
    ZQTianShan::CDN::ClientManager _clientMgr;
    ZQTianShan::CDN::TransferPortManager _portMgr;
    bool _quit;

    ZQ::common::Mutex _indexLock; // for the transfer id map and stream index
	TransferIdMap             _transferIdMap;
    typedef std::map<std::string, Ice::Identity> StreamToSessionMap;
    StreamToSessionMap streamToSessionIndex_;

    StreamEventHandlerPtr streamEventHandler_;

//    PortSnmpManager& portSnmpMgr_;

    class RemoteHealthRecord {
    public:
        struct RHStatus {
            std::string hostName;
            int status;
            int64 stamp;
        };
        void set(const RHStatus& status) {
            ZQ::common::MutexGuard guard(_lock);
            _statusMap[status.hostName] = status;
        }
        bool get(const std::string& hostName, RHStatus& status) {
            ZQ::common::MutexGuard guard(_lock);
            if(_statusMap.find(hostName) != _statusMap.end()) {
                status = _statusMap[hostName];
                return true;
            } else {
                return false;
            }
        }
    private:
        ZQ::common::Mutex _lock;
        std::map<std::string, RHStatus> _statusMap;
    } _rhRecord;
};

class StreamEventHandler: public ::TianShanIce::Streamer::StreamEventSink
{
public:
    explicit StreamEventHandler(ZQTianShan::CDN::C2Env& env);
    virtual ~StreamEventHandler();
    void bindLocator(C2LocatorImpl* loc);
    void releaseLocator();
public:
    virtual void ping(::Ice::Long timestamp, const ::Ice::Current&);
    virtual void OnEndOfStream(const std::string &proxy, const std::string &id, const TianShanIce::Properties &, const ::Ice::Current&) const;
    virtual void OnBeginningOfStream(const std::string &proxy, const std::string &id, const TianShanIce::Properties &, const ::Ice::Current&) const;
    virtual void OnStateChanged(const std::string &proxy, const std::string &id, TianShanIce::Streamer::StreamState prevState, TianShanIce::Streamer::StreamState currentState, const TianShanIce::Properties &, const ::Ice::Current&) const;
    virtual void OnSpeedChanged(const std::string &proxy, const std::string &id, Ice::Float prevSpeed, Ice::Float currentSpeed, const TianShanIce::Properties &, const ::Ice::Current&) const;
    virtual void OnExit(const std::string &proxy, const std::string &id, Ice::Int exitCode, const std::string &reason, const ::Ice::Current&) const;
    virtual void OnExit2(const std::string &proxy, const std::string &id, Ice::Int exitCode, const std::string &reason ,const ::TianShanIce::Properties& props, const ::Ice::Current&) const;
private:
    ZQTianShan::CDN::C2Env& _env;
    ZQ::common::Mutex lockLocator_;
    C2LocatorImpl* locator_;
};
#define DEFAULT_C2_CHECKTIME (600*1000)
}//namespace SCS
}// namespace TianShanIce

#endif
