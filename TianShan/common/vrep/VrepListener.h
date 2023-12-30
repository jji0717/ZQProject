#ifndef __ZQ_VREP_Listener_H__
#define __ZQ_VREP_Listener_H__

#include "VrepStates.h"
#include "VrepMessages.h"
#include <NativeThreadPool.h>

extern "C" {
#ifdef ZQ_OS_LINUX
#include <arpa/inet.h>
#endif
}

namespace ZQ {
namespace Vrep {

class TransportListener: public TransportChannel
{
public:
    TransportListener();
    virtual ~TransportListener();
    void setup(ZQ::common::NativeThreadPool& pool, SOCKET s, const char* peerAddr, u_short peerPort);
    bool connected() const;
    virtual bool openConnection(TransportNotifier* notifier);
    virtual void reset();
    virtual bool sendMessage(const byte* data, size_t length);
private:
    void startReceiving();
    void stopReceiving();
    void releaseConnection();
private:
    ZQ::common::NativeThreadPool* pool_;
    SOCKET s_;
    // peer info
    std::string peer_;
    u_short port_;
    TransportNotifier* notifier_;
};

class MonitorFactory {
public:
    virtual ~MonitorFactory() {}
    virtual StateMachine::Monitor* create() = 0;
    virtual void destroy(StateMachine::Monitor* m) = 0;
};
class Server;
class Listener: public StateMachine::Monitor
{
public:
    Listener(Server& svr, ZQ::common::Log& log, ZQ::common::NativeThreadPool& pool, Watchdog& watchdog);
    virtual ~Listener();
    bool start(const Configuration& conf, SOCKET s, const char* peer, u_short port, StateMachine::Monitor* moni);
    void stop();
public:
    virtual void onStateChanged(StateDescriptor from, StateDescriptor to);
private:
    Server& svr_;
    ZQ::common::Log& log_;
    ZQ::common::NativeThreadPool& pool_;
    Watchdog& watchdog_;
    StateMachine fsm_;
    TransportListener transportChannel_;
    Configuration conf_;
    Context context_;

    SOCKET sock_;
};

class Server: public ZQ::common::NativeThread {
    friend class RemoveClientCommand;
public:
    Server(ZQ::common::Log& log, ZQ::common::NativeThreadPool& pool);
    ~Server();
    void setBindAddress(const char* addr, u_short port);
    void setMonitorFactory(MonitorFactory& fac);
    void configure(const Configuration& conf);
    void issueRemoveClientCommand(SOCKET s);
    void stop();
public:
    virtual int run();
private:
    void addNewClient(SOCKET s, const char* addr, u_short port);
    void removeClient(SOCKET s);
private:
    ZQ::common::Log& log_;
    ZQ::common::NativeThreadPool& pool_;
    Watchdog watchdog_;
    std::string bindAddr_;
    u_short bindPort_;
    SOCKET sock_;
    MonitorFactory* moniFac_;
    Configuration conf_;
    struct ClientRecord {
        SOCKET sock;
        std::string addr;
        u_short port;
        Listener* listener;
        StateMachine::Monitor* monitor;
    };
    ZQ::common::Mutex lockClients_;
    std::vector<ClientRecord> clients_;
};

}}
#endif
