#include "VrepSpeaker.h"
#include <TimeUtil.h>

extern "C" {
#include <fcntl.h>
#ifdef ZQ_OS_LINUX
#include <arpa/inet.h>
#endif
}

namespace ZQ {
namespace Vrep {
TransportSpeaker::TransportSpeaker()
    :pool_(NULL), s_(INVALID_SOCKET), port_(0), notifier_(NULL)
{
}

void TransportSpeaker::setThreadPool(ZQ::common::NativeThreadPool& pool) {
    pool_ = &pool;
}
void TransportSpeaker::setPeer(const char* peer, u_short port) {
    peer_ = peer;
    port_ = port;
}
bool TransportSpeaker::connected() const {
    // with 1 msec timeout
    return (ZQ::Vrep::connected(s_, 1) > 0);
}

bool TransportSpeaker::openConnection(TransportNotifier* notifier) {
    notifier_ = notifier;
    s_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(s_ == INVALID_SOCKET) {
        std::string error = showSocketError(socket_errno);
        return false;
    }
#ifdef ZQ_OS_MSWIN
    u_long nonblocking = 1;
    ioctlsocket(s_, FIONBIO, &nonblocking);
#else
	int flags = fcntl(s_, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(s_, F_SETFL, flags);
#endif
    sockaddr_in addr;
    addr.sin_family = AF_INET; // host byte order
    addr.sin_port = htons(port_); // short, network byte order
    addr.sin_addr.s_addr = inet_addr(peer_.c_str());
    memset(addr.sin_zero, '\0', sizeof addr.sin_zero);

    int ret = connect(s_, (struct sockaddr*)&addr, sizeof addr);
#ifdef ZQ_OS_MSWIN
    if(ret == 0 || (ret == SOCKET_ERROR && socket_errno == WSAEWOULDBLOCK)) {
#else
    if(ret == 0 || (ret == SOCKET_ERROR && socket_errno == EWOULDBLOCK)) {
#endif
        startReceiving();
        return true;
    } else {
        std::string error = showSocketError(socket_errno);
        return false;
    }
}
void TransportSpeaker::reset() {
    stopReceiving();
    releaseConnection();
}
bool TransportSpeaker::sendMessage(const byte* data, size_t length) {
    if(NULL == data || length <= 0)
        return false;

    size_t nTotalSent = 0;
    while(nTotalSent < length) {
        int nSent = send(s_, (const char*)(data + nTotalSent), length - nTotalSent, 0);
        if(nSent > 0){
            nTotalSent += nSent;
        } else {
            std::string error = showSocketError(socket_errno);
            return false;
        }
    }
    return true;
}


void TransportSpeaker::startReceiving() {
    if(pool_ != NULL) {
        SpeakerReceivingThread* recvThrd = new SpeakerReceivingThread(*pool_, s_, notifier_, peer_.c_str(), port_, 2000);
        recvThrd->start();
    }
}
void TransportSpeaker::stopReceiving() {
    closeConnection(s_);
}
void TransportSpeaker::releaseConnection() {
    releaseSocket(s_);
    s_ = INVALID_SOCKET;
    notifier_ = NULL;
}


// aux class
class TimerTrigger: public TimeoutObject
{
public:
    TimerTrigger(Event e, StateMachine* fsm)
        :event_(e), fsm_(fsm) {
    }
	virtual ~TimerTrigger(){}
    virtual void onTimer() {
        if(fsm_)
            fsm_->issueEvent(event_);
    }
private:
    Event event_;
    StateMachine* fsm_;
};

// speaker
Speaker::Speaker(ZQ::common::Log& log, ZQ::common::NativeThreadPool& pool)
    :log_(log), pool_(pool), watchdog_(log, pool) {
    autoRestartTimer_ = NULL;
    restartInterval_ = 0;
    connEstablished_ = false;
}
Speaker::~Speaker() {
    stop();
#define ClearField(F) if(F) { delete F; F = NULL; }
    ClearField(context_.connectRetryTimer);
    ClearField(context_.holdTimer);
    ClearField(context_.keepAliveTimer);
    ClearField(autoRestartTimer_);
}

// auto issue e_Start in the Idle state
void Speaker::enableAutoRestart(int intervalSec) {
    restartInterval_ = intervalSec * 1000;
    if(restartInterval_ < 0)
        restartInterval_ = 0;

    if(!autoRestartTimer_) {
        autoRestartTimer_ = new Timer(watchdog_, TimeoutObjectPtr(new TimerTrigger(e_Start, &fsm_)));
    }
}

void Speaker::setPeer(const char* ip, u_short port) {
    transportChannel_.setPeer(ip, port);
}

bool Speaker::start(const Configuration& conf) {
    conf_ = conf;
    // adjust some fields as need
    // TODO: check the conf fields
    conf_.sendReceiveMode = VREP_SendOnlyMode;

    // build context
    context_.trace = &log_;
    context_.fsm = &fsm_;
    context_.transportChannel = &transportChannel_;
    context_.conf = &conf_;
    context_.connectRetryTimer = new Timer(watchdog_, TimeoutObjectPtr(new TimerTrigger(e_ConnectRetryTimerExpired, &fsm_)));
    context_.holdTimer = new Timer(watchdog_, TimeoutObjectPtr(new TimerTrigger(e_HoldTimerExpired, &fsm_)));
    context_.keepAliveTimer = new Timer(watchdog_, TimeoutObjectPtr(new TimerTrigger(e_KeepAliveTimerExpired, &fsm_)));
    watchdog_.start();

    // states
    transportChannel_.setThreadPool(pool_);
    fsm_.setup(context_);
    fsm_.addMonitor(this);
    fsm_.start();
    fsm_.issueEvent(e_Start);
    return true;
}
void Speaker::stop() {
    fsm_.removeMonitor(this);
    fsm_.issueEvent(e_Stop);
    fsm_.stop();
	watchdog_.stop();
}

void Speaker::sendUpdate(const UpdateMessage& msg, size_t timeout) {
    if(connEstablished_) {
        TransportHelper(context_).sendUPDATE(msg);
    } else {
        ZQ::common::MutexGuard guard(lockMsgs_);
        delayedMsgs_.push_back(DelayedMessage(msg, ZQ::common::now() + timeout));
    }
}

void Speaker::onStateChanged(StateDescriptor from, StateDescriptor to) {
    // AutoRestart
    if(autoRestartTimer_) {
        if(to == st_Idle) {
            log_(ZQ::common::Log::L_DEBUG, CLOGFMT(Speaker, "*Auto restart state machine in %d msec"), restartInterval_);
            autoRestartTimer_->start(restartInterval_);
        } else if(from == st_Idle) {
            autoRestartTimer_->clear();
        }
    }

    // delayed messages
    if(to == st_Established) {
        connEstablished_ = true;
        ZQ::common::MutexGuard guard(lockMsgs_);
        int64 currentStamp = ZQ::common::now();
        for(DelayedMessages::const_iterator it = delayedMsgs_.begin(); it != delayedMsgs_.end(); ++it) {
            if(currentStamp <= it->second) // not expired yet
                TransportHelper(context_).sendUPDATE(it->first);
        }
        delayedMsgs_.clear();
    } else {
        connEstablished_ = false;
    }
}
}} // namespace ZQ::Vrep
