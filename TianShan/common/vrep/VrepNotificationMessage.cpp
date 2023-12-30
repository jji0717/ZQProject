#include "VrepNotificationMessage.h"
#include <sstream>

namespace ZQ {
namespace Vrep {
NotificationMessage::NotificationMessage() {
    clear();
}
void NotificationMessage::setCode(byte code) {
    code_ = code;
}
void NotificationMessage::getCode(byte& code) const {
    code = code_;
}

void NotificationMessage::setSubcode(byte subcode) {
    subcode_ = subcode;
}
void NotificationMessage::getSubcode(byte& subcode) const {
    subcode = subcode_;
}

void NotificationMessage::setData(byte bData) {
    data_.resize(1);
    data_[0] = bData;
}
void NotificationMessage::setData(word wData) {
    data_.resize(2);
    MessageHelper helper(&data_[0], 2);
    helper.putWord(wData, 0);
}
void NotificationMessage::setData(dword dwData) {
    data_.resize(4);
    MessageHelper helper(&data_[0], 2);
    helper.putDword(dwData, 0);
}
void NotificationMessage::setData(const bytes& data) {
    data_ = data;
}
void NotificationMessage::getData(bytes& data) const {
    data = data_;
}

bool NotificationMessage::validate() const {
    switch(code_) {
    case VREP_Code_MessageHeaderError:
        if(subcode_ != VREP_Subcode_BadMessageLength
           && subcode_ != VREP_SubCode_BadMessageType
           ) {
            return false;
        } else {
            return true;
        }
    case VREP_Code_OPENMessageError:
        if(subcode_ != VREP_Subcode_UnsupportedVersion
           && subcode_ != VERP_Subcode_BadPeerAddressDomain
           && subcode_ != VREP_Subcode_BadIdentifier
           && subcode_ != VREP_Subcode_UnsuportedParameter
           && subcode_ != VREP_Subcode_UnaccecptableHoldTime
           && subcode_ != VREP_Subcode_UnsuportedCapacity
           && subcode_ != VREP_Subcode_CapacityMismatch
           ) {
            return false;
        } else {
            return true;
        }
    case VREP_Code_UPDATEMessageError:
        if(subcode_ != VREP_Subcode_MalformedAttributes
           && subcode_ != VREP_Subcode_UnrecognizedWellknownAttribute
           && subcode_ != VREP_Subcode_MissingAttribute
           && subcode_ != VREP_Subcode_AttributeFlagsError
           && subcode_ != VREP_Subcode_AttributeLengthError
           && subcode_ != VREP_Subcode_InvalidAttribute
           ) {
            return false;
        } else {
            return true;
        }
    default:
        return true;
    }
}
int NotificationMessage::parse(const byte* buf, size_t len) {
    NOTIFICATIONHeader hdr;
    int hdrLen = parseNOTIFICATIONHeader(hdr, buf, len);
    if(hdrLen > 0) {
        code_ = hdr.code;
        subcode_ = hdr.subcode;
        data_.assign(buf + hdrLen, buf + len);
        return len;
    } else {
        return -1;
    }
}
int NotificationMessage::build(byte* buf, size_t len) const {
    if(!validate())
        return -1;

    size_t offset = 0;
    NOTIFICATIONHeader hdr;
    hdr.code = code_;
    hdr.subcode = subcode_;
    int hdrLen = buildNOTIFICATIONHeader(buf, len, hdr);
    if(hdrLen > 0) {
        offset += hdrLen;
        if(hdrLen + data_.size() <= len) {
            MessageHelper helper(buf, len);
            offset += helper.put(data_, offset);
            return offset;
        } else { // buffer too small
            return -1;
        }
    } else {
        return -1;
    }
}

void NotificationMessage::clear() {
    code_ = VREP_Code_Cease;
    subcode_ = VREP_Subcode_Default;
    data_.clear();
}

void NotificationMessage::textDump(std::string& data) const {
    std::ostringstream buf;
    buf << "VREP NOTIFICATION: Code(" << ((int)code_) << "), "
        << "Subcode(" << ((int)subcode_) << "), "
        << "Data(";
    static char hexTable[] = {
        '0', '1', '2', '3',
        '4', '5', '6', '7',
        '8', '9', 'A', 'B',
        'C', 'D', 'E', 'F',
    };
    for(size_t i = 0; i < data_.size(); ++i) {
        byte c = data_[i];
        buf << hexTable[(c & 0xF0) >> 4]
            << hexTable[c & 0x0F]
            << " ";
    }

    buf << ").";
    buf.str().swap(data);
}

}} // namespace ZQ::Vrep
