#ifndef __ZQ_Vrep_Messages_H__ 
#define __ZQ_Vrep_Messages_H__ 
#include "VrepMessage.h"
#include "VrepOpenMessage.h"
#include "VrepNotificationMessage.h"
#include "VrepUpdateMessage.h"

namespace ZQ {
namespace Vrep {

class TransportHelper
{
public:
    TransportHelper(Context& ctx)
        :ctx_(ctx) {
    }
    template <class MessageT>
    bool sendMessage(const MessageT& msg, byte type, bool hexDump = false) {
        byte buf[VREP_MsgSize_Max];
        size_t len = sizeof buf;

        int msgLen = msg.build(buf + VREP_MsgSize_Header, len - VREP_MsgSize_Header);
        if(msgLen >= 0) {
            // update the header field
            VREPHeader hdr;
            hdr.length = VREP_MsgSize_Header + msgLen;
            hdr.type = type;
            buildVREPHeader(buf, len, hdr); // can't fail
            bool sendOk = ctx_.transportChannel->sendMessage(buf, hdr.length);
            if(hexDump)
                ctx_.trace->hexDump(ZQ::common::Log::L_DEBUG, buf, hdr.length, sendOk ? "Sent:" : "SendFailed:");
            return sendOk;
        } else {
            (*ctx_.trace)(ZQ::common::Log::L_ERROR, CLOGFMT(TranportHelper, "Failed to build message. type:%u"), (dword)type);
            return false;
        }
    }

    bool sendOPEN(const OpenMessage& msg) {
        std::string txt;
        msg.textDump(txt);
        (*ctx_.trace)(ZQ::common::Log::L_INFO, CLOGFMT(TranportHelper, "Send:%s"), txt.c_str());

        return sendMessage(msg, VREP_OPEN);
    }
    bool sendNOTIFICATION(const NotificationMessage& msg) {
        std::string txt;
        msg.textDump(txt);
        (*ctx_.trace)(ZQ::common::Log::L_INFO, CLOGFMT(TranportHelper, "Send:%s"), txt.c_str());

        return sendMessage(msg, VREP_NOTIFICATION);
    }

    class KeepAliveMessage {
    public:
        int build(byte*, size_t len) const {
            return 0;
        }
    };
    bool sendKEEPALIVE() {
        KeepAliveMessage msg;
        return sendMessage(msg, VREP_KEEPALIVE);
    }
    bool sendUPDATE(const UpdateMessage& msg) {
        std::string txt;
        msg.textDump(txt);
        (*ctx_.trace)(ZQ::common::Log::L_INFO, CLOGFMT(TranportHelper, "Send:%s"), txt.c_str());

        return sendMessage(msg, VREP_UPDATE);
    }
private:
    Context& ctx_;
};
}} // namespace ZQ::Vrep

#endif
