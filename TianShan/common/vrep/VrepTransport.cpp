#include "VrepTransport.h"

extern "C" {
#ifdef ZQ_OS_LINUX
#include <arpa/inet.h>
#endif
}

namespace ZQ {
namespace Vrep {
// connection
unsigned long timevalToMsec(struct timeval tv)
{
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

struct timeval msecToTimeval(unsigned long msec)
{
    timeval tv;
    tv.tv_sec = msec / 1000;
    tv.tv_usec = (msec % 1000) * 1000;
    return tv;
}

// check the socket's connective status
// return positive number for success
//        0 for timeout
//        negative number for failure
int connected(SOCKET s, int timeout)
{
    timeval tv = msecToTimeval(timeout);
    fd_set fs;
    FD_ZERO(&fs);
    FD_SET(s, &fs);

    return select((int)s + 1, NULL, &fs, NULL, &tv);
}

int messageArrived(SOCKET s, int timeout)
{
    timeval tv = msecToTimeval(timeout);
    fd_set fs;
    FD_ZERO(&fs);
    FD_SET(s, &fs);
    return select((int)s + 1, &fs, NULL, NULL, &tv);
}

// close the connection
// return 0 for success. otherwise check the socket_errno for reason
int closeConnection(SOCKET s) {
#ifdef ZQ_OS_MSWIN
    return shutdown(s, SD_BOTH);
#else
    return shutdown(s, SHUT_RDWR);
#endif
}

// release the socket resource
// return 0 for success. otherwise check the socket_errno for reason
int releaseSocket(SOCKET s) {
#ifdef ZQ_OS_MSWIN
    return closesocket(s);
#else
    return close(s);
#endif
}

void receiving(SOCKET s, TransportNotifier* notifier) {
    // connected
    size_t checkInterval = 2000;
    byte buf[VREP_MsgSize_Max];
    while(true) {
        int ret = messageArrived(s, checkInterval);
        if(ret > 0) {
            int nRead = recv(s, (char*)&buf, sizeof buf, 0);
            if(nRead > 0) {
                notifier->onMessage(buf, nRead);
            } else if (nRead == 0) {
                notifier->onRemoteClosed();
                break;
            } else {
                int sockErr = socket_errno;
#ifdef ZQ_OS_MSWIN
                if(sockErr == WSAESHUTDOWN) { // closed by local
#else
                if(sockErr == ESHUTDOWN) { // closed by local
#endif
                    // just quit silently
                    // log here
                } else {
                    std::string error = showSocketError(sockErr);
                    // log here
                    notifier->onFatalError(error);
                }
                break;
            }
        } else if (ret == 0) { // no data in 2 seconds
            continue;
        } else {
            int sockErr = socket_errno;
#ifdef ZQ_OS_MSWIN
            if(sockErr == WSAENOTSOCK) { // the socket may be released
#else
            if(sockErr == ENOTSOCK) { // the socket may be released
#endif
                // quit silently
                // log here
            } else {
                std::string error = showSocketError(sockErr);
                // log here
                notifier->onFatalError(error);
            }
            break;
        }
    } // while(true)
}

void TransportNotifier::onConnected(const char* localIp, u_short localPort, const char* remoteIp, u_short remotePort) {
}

void TransportNotifier::onConnectTimeout(size_t timeout) {
}
void TransportNotifier::onMessage(const byte* data, size_t length) {
}
void TransportNotifier::onRemoteClosed() {
}
void TransportNotifier::onFatalError(const std::string& error) {
}

bool TransportChannel::openConnection(TransportNotifier*) {
    return false;
}
void TransportChannel::reset() {
}
bool TransportChannel::sendMessage(const byte* data, size_t length) {
    return false;
}

SpeakerReceivingThread::SpeakerReceivingThread(ZQ::common::NativeThreadPool& pool, SOCKET s, TransportNotifier* notifier, const char* peerAddr, u_short peerPort, size_t connectTimeout)
    :ZQ::common::ThreadRequest(pool), s_(s), notifier_(notifier), peer_(peerAddr), port_(peerPort), connTimeout_(connectTimeout) {
}
bool SpeakerReceivingThread::init() {
    if(notifier_ == NULL) {
        return false;
    }
    return true;
}

int SpeakerReceivingThread::run() {
    int ret = connected(s_, connTimeout_);
    if(ret > 0)
    {
        // connected
        sockaddr_in localAddr;
#ifdef ZQ_OS_MSWIN
	int localLen;
#else
        socklen_t localLen;
#endif
        getsockname(s_, (struct sockaddr*)&localAddr, &localLen);
        std::string localIp = inet_ntoa(localAddr.sin_addr);

        notifier_->onConnected(localIp.c_str(), localAddr.sin_port, peer_.c_str(), port_);
        receiving(s_, notifier_);
        return 0;
    } else if( ret == 0) {
        notifier_->onConnectTimeout(connTimeout_);
        return -1;
    } else {
        notifier_->onFatalError(showSocketError(socket_errno));
        return -1;
    }
}


}} // namespace ZQ::Vrep
