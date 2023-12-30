#ifndef __ZQ_VREP_H__
#define __ZQ_VREP_H__

#include <ZQ_common_conf.h>
#include "VrepUtils.h"
#ifdef ZQ_OS_MSWIN
#include <winsock2.h>
#endif
#include <vector>

namespace ZQ {
    namespace common {
        class Log;
    }
}
namespace ZQ {
namespace Vrep {

#ifdef ZQ_OS_MSWIN
#define socket_errno WSAGetLastError()
#else
#define socket_errno errno
#define SOCKET int
#define INVALID_SOCKET (-1)
#endif

#define VREP_OPEN 1
#define VREP_UPDATE 2
#define VREP_NOTIFICATION 3
#define VREP_KEEPALIVE 4

enum StateDescriptor
    {
        st_Idle = 0,
        st_Connect = 1,
        st_Active = 2,
        st_OpenSent = 3,
        st_OpenConfirm = 4,
        st_Established = 5
    };
#define VREP_StatesCount 6

enum Event
    {
        // system events
        e_Start = 0,
        e_Stop = 1,

        // communication events
        e_ConnectionOpen = 2,
        e_ConnectionClosed = 3,
        e_ConnectionOpenFailed = 4,
        e_TransportError = 5,

        // protocol events
        e_ConnectRetryTimerExpired = 6,
        e_HoldTimerExpired = 7,
        e_KeepAliveTimerExpired = 8,
        e_ReceiveOPEN = 9,
        e_ReceiveKEEPALIVE = 10,
        e_ReceiveUPDATE = 11,
        e_ReceiveNOTIFICATION = 12
    };
#define VREP_EventsCount 13

#define VREP_MsgSize_Max 4096

const char* showState(StateDescriptor st);
const char* showEvent(Event e);
const char* showSocketError(int sockErr);

struct Configuration
{
    dword identifier;
    std::string streamingZone;
    std::string componentName;
    std::string vendorString;

    int defaultHoldTimeSec;
    int connectRetryTimeSec;
    int connectTimeoutMsec;
    int keepAliveTimeSec;
    dword sendReceiveMode;
    Configuration() {
        identifier = 0;
        defaultHoldTimeSec = 0;
        connectRetryTimeSec = 0;
        connectTimeoutMsec = 0;
        keepAliveTimeSec = 0;
        sendReceiveMode = 0;
    }
};
class StateMachine;
class TransportChannel;
struct Context
{
    ZQ::common::Log* trace;
    StateMachine* fsm;
    TransportChannel* transportChannel;
    Configuration* conf;
    Timer* connectRetryTimer;
    Timer* holdTimer;
    Timer* keepAliveTimer;
    Context() {
        trace = NULL;
        fsm = NULL;
        transportChannel = NULL;
        conf = NULL;
        connectRetryTimer = NULL;
        holdTimer = NULL;
        keepAliveTimer = NULL;
    }
};

}} // namespace ZQ::Vrep

#endif
