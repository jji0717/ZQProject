#ifndef __ZQ_Vrep_States_H__
#define __ZQ_Vrep_States_H__
#include "vrep.h"
#include "VrepUtils.h"
#include "VrepTransport.h"
#include "VrepMessages.h"
#include <NativeThread.h>
namespace ZQ {
namespace Vrep {
class State
{
public:
	virtual ~State(){}
    // get the descriptor of the state
    virtual StateDescriptor descriptor() const = 0;
    // return the next state
    virtual StateDescriptor onEvent(Event event, Context& ctx) = 0;
};

class IdleState: public State
{
public:
	virtual ~IdleState(){}
    // get the descriptor of the state
    virtual StateDescriptor descriptor() const;
    // return the next state
    virtual StateDescriptor onEvent(Event event, Context& ctx);
};

class ConnectState: public State
{
public:
    virtual ~ConnectState(){}
    // get the descriptor of the state
    virtual StateDescriptor descriptor() const;
    // return the next state
    virtual StateDescriptor onEvent(Event event, Context& ctx);
};

class ActiveState: public State
{
public:
	virtual ~ActiveState(){}
    // get the descriptor of the state
    virtual StateDescriptor descriptor() const;
    // return the next state
    virtual StateDescriptor onEvent(Event event, Context& ctx);
};

class OpenSentState: public State
{
public:
	virtual ~OpenSentState(){}
    // get the descriptor of the state
    virtual StateDescriptor descriptor() const;
    // return the next state
    virtual StateDescriptor onEvent(Event event, Context& ctx);
};

class OpenConfirmState: public State
{
public:
	virtual ~OpenConfirmState(){}
    // get the descriptor of the state
    virtual StateDescriptor descriptor() const;
    // return the next state
    virtual StateDescriptor onEvent(Event event, Context& ctx);
};

class EstablishedState: public State
{
public:
	virtual ~EstablishedState(){}
    // get the descriptor of the state
    virtual StateDescriptor descriptor() const;
    // return the next state
    virtual StateDescriptor onEvent(Event event, Context& ctx);
};


class StateMachine: public virtual ZQ::common::NativeThread, public virtual TransportNotifier
{
public:
    class Monitor { // event monitor
    public:
		virtual ~Monitor(){} 
        virtual void onStateChanged(StateDescriptor from, StateDescriptor to) {}
        virtual void onEvent(Event e) {}
        virtual void onOpenMessage(const OpenMessage& msg) {}
        virtual void onUpdateMessage(const UpdateMessage& msg) {}
        virtual void onNotificationMessage(const NotificationMessage& msg) {}
    };

public:
    StateMachine();
    void setup(Context& ctx);
    void teardown();
    void stop();

    void releaseResource();
    void issueEvent(Event event);

    void addMonitor(Monitor* m);
    void removeMonitor(Monitor* m);
public:
    // the transport notifier
    virtual void onConnected(const char* localIp, u_short localPort, const char* remoteIp, u_short remotePort);
    virtual void onConnectTimeout(size_t timeout);
    virtual void onMessage(const byte* data, size_t length);
    virtual void onRemoteClosed();
    virtual void onFatalError(const std::string& error);

public:
    // for State objects
    bool getReceivedOpen(OpenMessage&, bool peekOnly = false);
    bool getReceivedUpdate(UpdateMessage&, bool peekOnly = false);
    bool getReceivedNotification(NotificationMessage&, bool peekOnly = false);
private:
    void saveReceivedOpen(const OpenMessage&);
    void saveReceivedUpdate(const UpdateMessage&);
    void saveReceivedNotification(const NotificationMessage&);

    // the native thread
    virtual int run();
    void forceIdle();
private:
    bool quit_;
    ZQ::common::Mutex stateLock_; // the protection lock of state pool and current state

    // current state
    StateDescriptor current_;

    // the states pool. use the state descriptor as the index
    State* states_[VREP_StatesCount];
    // the states implementation
    IdleState stIdle_;
    ConnectState stConnect_;
    ActiveState stActive_;
    OpenSentState stOpenSent_;
    OpenConfirmState stOpenConfirm_;
    EstablishedState stEstablished_;

    // event queue
    EventQueue<Event> eventQ_;
    Context* context_;

    // receiving buffer
    byte msgBuf_[VREP_MsgSize_Max];
    size_t nReceived_;
    size_t nExpected_;

    class Counter: public Monitor {
    public:
		virtual ~Counter(){}
        void addMonitor(Monitor* m);
        void removeMonitor(Monitor* m);
        void clear();
        virtual void onStateChanged(StateDescriptor from, StateDescriptor to);
        virtual void onEvent(Event e);
        virtual void onOpenMessage(const OpenMessage& msg);
        virtual void onUpdateMessage(const UpdateMessage& msg);
        virtual void onNotificationMessage(const NotificationMessage& msg);
    private:
        ZQ::common::Mutex moniLock_;
        std::vector<Monitor*> monitors_;
    } counter_;

    ZQ::common::Mutex msgLock_; // the protection lock of received message
    std::queue<OpenMessage> openQ_;
    std::queue<NotificationMessage> notiQ_;
    std::queue<UpdateMessage> updateQ_;
};

}} // namespace ZQ::Vrep
#endif
