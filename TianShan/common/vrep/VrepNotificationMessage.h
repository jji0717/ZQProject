#ifndef __ZQ_Vrep_NotifationMessage_H__
#define __ZQ_Vrep_NotifationMessage_H__
#include "VrepMessage.h"

namespace ZQ {
namespace Vrep {

#define VREP_Code_MessageHeaderError 1
#define VREP_Code_OPENMessageError 2
#define VREP_Code_UPDATEMessageError 3
#define VREP_Code_HoldTimerExpired 4
#define VREP_Code_FiniteStateMachineError 5
#define VREP_Code_Cease 6

// Unspecific
#define VREP_Subcode_Default 0

// message header error
#define VREP_Subcode_BadMessageLength 1
#define VREP_SubCode_BadMessageType 2

// OPEN message error
#define VREP_Subcode_UnsupportedVersion 1
#define VERP_Subcode_BadPeerAddressDomain 2
#define VREP_Subcode_BadIdentifier 3
#define VREP_Subcode_UnsuportedParameter 4
#define VREP_Subcode_UnaccecptableHoldTime 5
#define VREP_Subcode_UnsuportedCapacity 6
#define VREP_Subcode_CapacityMismatch 7

// UPDATE message error
#define VREP_Subcode_MalformedAttributes 1
#define VREP_Subcode_UnrecognizedWellknownAttribute 2
#define VREP_Subcode_MissingAttribute 3
#define VREP_Subcode_AttributeFlagsError 4
#define VREP_Subcode_AttributeLengthError 5
#define VREP_Subcode_InvalidAttribute 6

class NotificationMessage
{
public:
    NotificationMessage();
    void setCode(byte code);
    void getCode(byte& code) const;

    void setSubcode(byte subcode);
    void getSubcode(byte& subcode) const;

    void setData(byte bData);
    void setData(word wData);
    void setData(dword dwData);
    void setData(const bytes& data);
    void getData(bytes& data) const;

    bool validate() const;
    int parse(const byte* buf, size_t len);
    int build(byte* buf, size_t len) const;
    void clear();

    void textDump(std::string& data) const;
private:
    byte code_;
    byte subcode_;
    bytes data_;
};

}} // namespace ZQ::Vrep

#endif
