#ifndef __ZQ_VREP_Transport_H__
#define __ZQ_VREP_Transport_H__
#include <NativeThreadPool.h>
#include "vrep.h"
#include <string>

namespace ZQ {
namespace Vrep {

class TransportNotifier
{
public:
	virtual ~TransportNotifier(){}
    virtual void onConnected(const char* localIp, u_short localPort, const char* remoteIp, u_short remotePort);
    virtual void onConnectTimeout(size_t timeout);
    virtual void onMessage(const byte* data, size_t length);
    virtual void onRemoteClosed();
    virtual void onFatalError(const std::string& error);
};

class TransportChannel
{
public:
	virtual ~TransportChannel(){}
    virtual bool openConnection(TransportNotifier*);
    virtual void reset();
    virtual bool sendMessage(const byte* data, size_t length);
};

class SpeakerReceivingThread: public ZQ::common::ThreadRequest
{
public:
    SpeakerReceivingThread(ZQ::common::NativeThreadPool& pool, SOCKET s, TransportNotifier* notifier, const char* peerAddr, u_short peerPort, size_t connectTimeout);
protected:
    virtual bool init();
    virtual int run();
    virtual void final(int retcode =0, bool bCancelled =false) { delete this; }
private:
    SOCKET s_;
    TransportNotifier* notifier_;
    std::string peer_;
    u_short port_;
    size_t connTimeout_;
};

unsigned long timevalToMsec(struct timeval tv);
struct timeval msecToTimeval(unsigned long msec);

// check the socket's connective status
// return positive number for success
//        0 for timeout
//        negative number for failure
int connected(SOCKET s, int timeout);
int messageArrived(SOCKET s, int timeout);

// close the connection
// return 0 for success. otherwise check the socket_errno for reason
int closeConnection(SOCKET s);

// release the socket resource
// return 0 for success. otherwise check the socket_errno for reason
int releaseSocket(SOCKET s);
}} // namespace ZQ::Vrep

#endif
