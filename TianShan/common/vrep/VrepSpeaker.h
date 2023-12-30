#ifndef __ZQ_VREP_Speaker_H__
#define __ZQ_VREP_Speaker_H__

#include "VrepStates.h"
#include "VrepMessages.h"
#include <NativeThreadPool.h>

#ifdef ZQ_OS_LINUX
#define SOCKET int
#define SOCKET_ERROR (-1)
#endif

namespace ZQ {
namespace Vrep {

class TransportSpeaker: public TransportChannel
{
public:
    TransportSpeaker();
	virtual ~TransportSpeaker(){}
    void setThreadPool(ZQ::common::NativeThreadPool& pool);
    void setPeer(const char* ip, u_short port);
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

class Speaker: public StateMachine::Monitor
{
public:
    Speaker(ZQ::common::Log& log, ZQ::common::NativeThreadPool& pool);
    virtual ~Speaker();

    // auto issue e_Start in the Idle state
    void enableAutoRestart(int intervalSec);
    void setPeer(const char* ip, u_short port);

    bool start(const Configuration& conf);
    void stop();

    void sendUpdate(const UpdateMessage& msg, size_t timeout);
public:
    virtual void onStateChanged(StateDescriptor from, StateDescriptor to);
    virtual void onEvent(Event e) {}
private:
    ZQ::common::Log& log_;
    ZQ::common::NativeThreadPool& pool_;
    Watchdog watchdog_;
    StateMachine fsm_;
    TransportSpeaker transportChannel_;
    Configuration conf_;
    Context context_;

    Timer* autoRestartTimer_;
    int restartInterval_;

    // first: msg; second: expiration
    typedef std::pair<UpdateMessage, int64> DelayedMessage;
    typedef std::vector<DelayedMessage> DelayedMessages;
    DelayedMessages delayedMsgs_;
    ZQ::common::Mutex lockMsgs_;

    bool connEstablished_;
};
}} // namespace ZQ::Vrep
#endif
