#include "VrepListener.h"
namespace ZQ {
namespace Vrep {
// aux class

#ifdef ZQ_OS_MSWIN
typedef int socklen_t;
#endif

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
TransportListener::TransportListener()
:s_(INVALID_SOCKET), port_(0), notifier_(NULL)
{
}
TransportListener::~TransportListener()
{
}
void TransportListener::setup(ZQ::common::NativeThreadPool& pool, SOCKET s, const char* peerAddr, u_short peerPort) {
    pool_ = &pool;
    s_ = s;
    peer_ = peerAddr;
    port_ = peerPort;
}
bool TransportListener::connected() const {
    // with 1 msec timeout
    return (ZQ::Vrep::connected(s_, 1) > 0);
}

bool TransportListener::openConnection(TransportNotifier* notifier){
    if(s_ != INVALID_SOCKET) {
        notifier_ = notifier;
        startReceiving();
        return true;
    } else {
        return false;
    }
}
void TransportListener::reset() {
    stopReceiving();
    releaseConnection();
}
bool TransportListener::sendMessage(const byte* data, size_t length){
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
void TransportListener::startReceiving() {
    if(pool_ != NULL) {
        SpeakerReceivingThread* recvThrd = new SpeakerReceivingThread(*pool_, s_, notifier_, peer_.c_str(), port_, 2000);
        recvThrd->start();
    }
}
void TransportListener::stopReceiving() {
    closeConnection(s_);
}
void TransportListener::releaseConnection() {
    releaseSocket(s_);
    s_ = INVALID_SOCKET;
    notifier_ = NULL;
}


class RemoveClientCommand: public ZQ::common::ThreadRequest {
public:
    RemoveClientCommand(ZQ::common::NativeThreadPool& pool, Server& svr, SOCKET sock)
        :ThreadRequest(pool), svr_(svr), sock_(sock) {
    }
    virtual bool init(void)	{ return true; };
    virtual int run(void) {
        svr_.removeClient(sock_);
        return 0;
    }
    virtual void final(int retcode =0, bool bCancelled =false) { delete this; }
private:
    Server& svr_;
    SOCKET sock_;
};
Listener::Listener(Server& svr, ZQ::common::Log& log, ZQ::common::NativeThreadPool& pool, Watchdog& watchdog)
:svr_(svr), log_(log), pool_(pool), watchdog_(watchdog) {
}
Listener::~Listener() {
    stop();
#define ClearField(F) if(F) { delete F; F = NULL; }
    ClearField(context_.connectRetryTimer);
    ClearField(context_.holdTimer);
    ClearField(context_.keepAliveTimer);
#undef ClearField
}

bool Listener::start(const Configuration& conf, SOCKET s, const char* peer, u_short port, StateMachine::Monitor* moni){
    conf_ = conf;
    sock_ = s;
    // adjust some fields as need
    // TODO: check the conf fields
    conf_.sendReceiveMode = VREP_ReceiveOnlyMode;

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
    transportChannel_.setup(pool_, s, peer, port);
    fsm_.setup(context_);
    fsm_.addMonitor(this);
    fsm_.addMonitor(moni);
    fsm_.start();
    fsm_.issueEvent(e_Start);
    return true;
}
void Listener::stop() {
    fsm_.removeMonitor(this);
    fsm_.issueEvent(e_Stop);
    fsm_.stop();
    watchdog_.stop();
}
void Listener::onStateChanged(StateDescriptor from, StateDescriptor to) {
    if(to == st_Idle) {
        log_(ZQ::common::Log::L_INFO, CLOGFMT(Listener, "onStateChanged() [%s->%s] stop the listener, socket=%d"), showState(from), showState(to), sock_);
        // notify to remove the client
        (new RemoveClientCommand(pool_, svr_, sock_))->start();
    }
}
Server::Server(ZQ::common::Log& log, ZQ::common::NativeThreadPool& pool)
:log_(log), pool_(pool), watchdog_(log, pool), sock_(INVALID_SOCKET) {
}
Server::~Server() {
    stop();
    if(isRunning()) {
        log_(ZQ::common::Log::L_INFO, CLOGFMT(Vrep::Server, "Request to stop the vrep server."));
        waitHandle(-1);
        log_(ZQ::common::Log::L_INFO, CLOGFMT(Vrep::Server, "Vrep server is stop."));
    }
}
void Server::setBindAddress(const char* addr, u_short port) {
    bindAddr_ = addr;
    bindPort_ = port;
}
void Server::setMonitorFactory(MonitorFactory& fac) {
    moniFac_ = &fac;
}
void Server::configure(const Configuration& conf) {
    conf_ = conf;
}
void Server::addNewClient(SOCKET s, const char* addr, u_short port) {
    log_(ZQ::common::Log::L_INFO, CLOGFMT(Vrep::Server, "New client detected: socket=%d address=%s port=%u"), (int)s, addr, port);
    ZQ::common::MutexGuard guard(lockClients_);
    if(moniFac_) {
        StateMachine::Monitor* monitor = moniFac_->create();
        if(monitor) {
            Listener *listener = new Listener(*this, log_, pool_, watchdog_);
            if(listener) {
                if(listener->start(conf_, s, addr, port, monitor)) {
                    log_(ZQ::common::Log::L_INFO, CLOGFMT(Vrep::Server, "Listener is prepared for client: socket=%d address=%s port=%u"), (int)s, addr, port);
                    ClientRecord c;
                    c.sock = s;
                    c.addr = addr;
                    c.port = port;
                    c.listener = listener;
                    c.monitor = monitor;
                    clients_.push_back(c);
                    return;
                } else {
                    log_(ZQ::common::Log::L_WARNING, CLOGFMT(Vrep::Server, "Failed to start listener for client: socket=%d address=%s port=%u"), (int)s, addr, port);
                }
                delete listener;
            } else {
                log_(ZQ::common::Log::L_WARNING, CLOGFMT(Vrep::Server, "Failed to create listener for client: socket=%d address=%s port=%u"), (int)s, addr, port);
            }
            moniFac_->destroy(monitor);
        } else {
            log_(ZQ::common::Log::L_WARNING, CLOGFMT(Vrep::Server, "Failed to create monitor for client: socket=%d address=%s port=%u"), (int)s, addr, port);
        }
    } else {
        log_(ZQ::common::Log::L_ERROR, CLOGFMT(Vrep::Server, "No monitor factory configured. Reject the client: socket=%d address=%s port=%u"), (int)s, addr, port);
    }
    log_(ZQ::common::Log::L_WARNING, CLOGFMT(Vrep::Server, "Failed to serve the client: socket=%d address=%s port=%u."), (int)s, addr, port);
    closeConnection(s);
    releaseSocket(s);
    return;
}
void Server::removeClient(SOCKET s) {
    log_(ZQ::common::Log::L_DEBUG, CLOGFMT(Vrep::Server, "Requested to remove client: socket=%d"), (int)s);
    ZQ::common::MutexGuard guard(lockClients_);
    for(std::vector<ClientRecord>::iterator it = clients_.begin(); it != clients_.end(); ++it) {
        ClientRecord& c = (*it);
        if(s == c.sock) {
            c.listener->stop();
            delete c.listener;
            moniFac_->destroy(c.monitor);
            closeConnection(s);
            releaseSocket(s);
            clients_.erase(it);
            log_(ZQ::common::Log::L_INFO, CLOGFMT(Vrep::Server, "Remove client successfully: socket=%d"), (int)s);
            return;
        }
    }
    log_(ZQ::common::Log::L_WARNING, CLOGFMT(Vrep::Server, "No client found with socket=%d"), (int)s);
}
int Server::run() {
    log_(ZQ::common::Log::L_INFO, CLOGFMT(Vrep::Server, "run() Listen thread enter with bind address=%s:%u"), bindAddr_.c_str(), bindPort_);
    sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock_ == INVALID_SOCKET) {
        std::string error = showSocketError(socket_errno);
        log_(ZQ::common::Log::L_ERROR, CLOGFMT(Vrep::Server, "run() Failed to initialize socket. error=%s"), error.c_str());
        return -1;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET; // host byte order
    addr.sin_port = htons(bindPort_); // short, network byte order
    addr.sin_addr.s_addr = inet_addr(bindAddr_.c_str());
    memset(addr.sin_zero, '\0', sizeof addr.sin_zero);
    if(0 != bind(sock_, (struct sockaddr*)&addr, sizeof addr) ) {
        std::string error = showSocketError(socket_errno);
        log_(ZQ::common::Log::L_ERROR, CLOGFMT(Vrep::Server, "run() Failed to bind address=%s:%u. error=%s"), bindAddr_.c_str(), bindPort_, error.c_str());
        releaseSocket(sock_);
        return -1;
    }
    if(0 != listen(sock_, SOMAXCONN)) {
        std::string error = showSocketError(socket_errno);
        log_(ZQ::common::Log::L_ERROR, CLOGFMT(Vrep::Server, "run() Failed to listen address=%s:%u. error=%s"), bindAddr_.c_str(), bindPort_, error.c_str());
        return -1;
    }

    do {
        sockaddr_in peerAddr;
        socklen_t peerLen = sizeof peerAddr;
        SOCKET peerSock = accept(sock_, (struct sockaddr*)&peerAddr, &peerLen);
        if(peerSock != INVALID_SOCKET) {
            addNewClient(peerSock, inet_ntoa(peerAddr.sin_addr), ntohs(peerAddr.sin_port));
        } else {
            int sockErr = socket_errno;
            std::string error = showSocketError(sockErr);
            // TODO: should check the recoverable error
            log_(ZQ::common::Log::L_WARNING, CLOGFMT(Vrep::Server, "run() get %s from accept()"), error.c_str());
            break;
        }
    } while (true);
    releaseSocket(sock_);
    log_(ZQ::common::Log::L_INFO, CLOGFMT(Vrep::Server, "run() Listen thread leave."));
    return 0;
}
void Server::stop() {
    if(sock_ != INVALID_SOCKET) {
        closeConnection(sock_);
        releaseSocket(sock_);
        log_(ZQ::common::Log::L_INFO, CLOGFMT(Vrep::Server, "Stop socket server."));
    }
}
}}
