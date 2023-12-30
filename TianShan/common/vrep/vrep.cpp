#include "vrep.h"

namespace ZQ {
namespace Vrep {
const char* showState(StateDescriptor st)
{
    static const char* description[VREP_StatesCount] = {
        "Idle",
        "Connect",
        "Active",
        "OpenSent",
        "OpenConfirm",
        "Established"
    };
    return description[st];
}

const char* showEvent(Event e)
{
    static const char* description[VREP_EventsCount] = {
        "Start",
        "Stop",
        "ConnectionOpen",
        "ConnectionClosed",
        "ConnectionOpenFailed",
        "TransportFatalError",
        "ConnectRetryTimerExpired",
        "HoldTimerExpired",
        "KeepAliveTimerExpired",
        "ReceiveOpenMessage",
        "ReceiveKeepAliveMessage",
        "ReceiveUpdateMessage",
        "ReceiveNotificationMessage"
    };

    return description[e];
}

// borrow from Ice/Network.cpp
const char* showSocketError(int sockErr) {
#ifdef ZQ_OS_MSWIN
    static struct {
        int code;
        const char* desc;
    } errmsgs[] = {
        { WSAEINTR, "WSAEINTR" },
        { WSAEBADF, "WSAEBADF" },
        { WSAEACCES, "WSAEACCES" },
        { WSAEFAULT, "WSAEFAULT" },
        { WSAEINVAL, "WSAEINVAL" },
        { WSAEMFILE, "WSAEMFILE" },
        { WSAEWOULDBLOCK, "WSAEWOULDBLOCK" },
        { WSAEINPROGRESS, "WSAEINPROGRESS" },
        { WSAEALREADY, "WSAEALREADY" },
        { WSAENOTSOCK, "WSAENOTSOCK" },
        { WSAEDESTADDRREQ, "WSAEDESTADDRREQ" },
        { WSAEMSGSIZE, "WSAEMSGSIZE" },
        { WSAEPROTOTYPE, "WSAEPROTOTYPE" },
        { WSAENOPROTOOPT, "WSAENOPROTOOPT" },
        { WSAEPROTONOSUPPORT, "WSAEPROTONOSUPPORT" },
        { WSAESOCKTNOSUPPORT, "WSAESOCKTNOSUPPORT" },
        { WSAEOPNOTSUPP, "WSAEOPNOTSUPP" },
        { WSAEPFNOSUPPORT, "WSAEPFNOSUPPORT" },
        { WSAEAFNOSUPPORT, "WSAEAFNOSUPPORT" },
        { WSAEADDRINUSE, "WSAEADDRINUSE" },
        { WSAEADDRNOTAVAIL, "WSAEADDRNOTAVAIL" },
        { WSAENETDOWN, "WSAENETDOWN" },
        { WSAENETUNREACH, "WSAENETUNREACH" },
        { WSAENETRESET, "WSAENETRESET" },
        { WSAECONNABORTED, "WSAECONNABORTED" },
        { WSAECONNRESET, "WSAECONNRESET" },
        { WSAENOBUFS, "WSAENOBUFS" },
        { WSAEISCONN, "WSAEISCONN" },
        { WSAENOTCONN, "WSAENOTCONN" },
        { WSAESHUTDOWN, "WSAESHUTDOWN" },
        { WSAETOOMANYREFS, "WSAETOOMANYREFS" },
        { WSAETIMEDOUT, "WSAETIMEDOUT" },
        { WSAECONNREFUSED, "WSAECONNREFUSED" },
        { WSAELOOP, "WSAELOOP" },
        { WSAENAMETOOLONG, "WSAENAMETOOLONG" },
        { WSAEHOSTDOWN, "WSAEHOSTDOWN" },
        { WSAEHOSTUNREACH, "WSAEHOSTUNREACH" },
        { WSAENOTEMPTY, "WSAENOTEMPTY" },
        { WSAEPROCLIM, "WSAEPROCLIM" },
        { WSAEUSERS, "WSAEUSERS" },
        { WSAEDQUOT, "WSAEDQUOT" },
        { WSAESTALE, "WSAESTALE" },
        { WSAEREMOTE, "WSAEREMOTE" },
        { WSAEDISCON, "WSAEDISCON" },
        { WSASYSNOTREADY, "WSASYSNOTREADY" },
        { WSAVERNOTSUPPORTED, "WSAVERNOTSUPPORTED" },
        { WSANOTINITIALISED, "WSANOTINITIALISED" },
        { WSAHOST_NOT_FOUND, "WSAHOST_NOT_FOUND" },
        { WSATRY_AGAIN, "WSATRY_AGAIN" },
        { WSANO_RECOVERY, "WSANO_RECOVERY" },
        { WSANO_DATA, "WSANO_DATA" }
    };

    static const char* unknownErrMsg = "Unknown";
    for(size_t i = 0; i < sizeof errmsgs / sizeof errmsgs[0]; ++i)
        if(sockErr == errmsgs[i].code)
            return errmsgs[i].desc;
    return unknownErrMsg;
#else
	static char buf[1024];
	memset(buf, '\0', sizeof buf);
	return strerror_r(sockErr, buf, sizeof buf);
#endif
}

}} // namespace ZQ::Vrep
