#include "VrepOpenMessage.h"
#include <sstream>
#include <assert.h>

namespace ZQ {
namespace Vrep {
OpenCapacities::OpenCapacities() {
    clear();
}

void OpenCapacities::setSendReceiveCapacity(dword mode) {
    sendReceiveCapacity_ = mode;
}
bool OpenCapacities::getSendReceiveCapacity(dword& mode) const {
    if(sendReceiveCapacity_ != VREP_Invalid_SendReceiveCapacity) {
        mode = sendReceiveCapacity_;
        return true;
    } else {
        return false;
    }
}

void OpenCapacities::setSupportedRouteTypes(const RouteTypes& rts) {
    supportedRouteTypes_ = rts;
}
bool OpenCapacities::getSupportedRouteTypes(RouteTypes& rts) const {
    if(!supportedRouteTypes_.empty()) {
        rts = supportedRouteTypes_;
        return true;
    } else {
        return false;
    }
}

void OpenCapacities::setGenericCapacities(const GenericCapacities& gcs) {
    genericCapacities_ = gcs;
}
bool OpenCapacities::getGenericCapacities(GenericCapacities& gcs) const {
    if(!genericCapacities_.empty()) {
        gcs = genericCapacities_;
        return true;
    } else {
        return false;
    }
}

bool OpenCapacities::validate() const {
    // send receive capacity
    if (sendReceiveCapacity_ != VREP_SendReceiveMode
        && sendReceiveCapacity_ != VREP_SendOnlyMode
        && sendReceiveCapacity_ != VREP_ReceiveOnlyMode
        )
        return false;

    // supported route types
    for(RouteTypes::const_iterator it = supportedRouteTypes_.begin(); it != supportedRouteTypes_.end(); ++it) {
        if(it->family != VREP_AddressFamily_NGOD)
            return false;

        if(it->protocol != VREP_AppProtocol_Preprovisioned
           && it->protocol != VREP_AppProtocol_R6_SessionParameterOnly
           && it->protocol != VREP_AppProtocol_R6_WithProvisioning
           && it->protocol != VREP_AppProtocol_R4
           && it->protocol != VREP_AppProtocol_A3
           && it->protocol != VREP_AppProtocol_R2
           && it->protocol != VREP_AppProtocol_S6_RTSP
           && it->protocol != VREP_AppProtocol_S6_TBD
           && it->protocol != VREP_AppProtocol_S4
           && it->protocol != VREP_AppProtocol_S3
           )
            return false;
    }
    return true;
}
int OpenCapacities::parse(const byte* buf, size_t len) {
    clear();
    // chop the capacities
    size_t offset = 0;
    while(offset < len) {
        OPENCapacityHeader hdr;
        int hdrLen = parseOPENCapacityHeader(hdr, buf + offset, len - offset);
        if(hdrLen > 0) { // successful
            if(offset + hdrLen + hdr.length <= len) { // valid message
                // parse the well-known capacities
                const byte* capVal = buf + offset + hdrLen;
                switch(hdr.code) {
                case VREP_Capacity_RouteTypesSupported:
                    {
                        int nRouteTypesCount = hdr.length / VREP_MsgSize_RouteType;
                        for(int i = 0; i < nRouteTypesCount; ++i) {
                            RouteType rt;
                            parseRouteType(rt, capVal + VREP_MsgSize_RouteType * i, VREP_MsgSize_RouteType);
                            supportedRouteTypes_.push_back(rt);
                        }
                    }
                    break;
                case VREP_Capacity_SendReceiveCapacity:
                    {
                        MessageHelper helper((byte*)buf, len);
                        helper.getDword(sendReceiveCapacity_, offset + hdrLen);
                    }
                    break;
                default:
                    {
                        GenericCapacity cap;
                        cap.code = hdr.code;
                        cap.data.assign(capVal, capVal + hdr.length);
                        genericCapacities_.push_back(cap);
                    }
                    break;
                }
                offset += hdrLen + hdr.length;
            } else { // bad message
                return -1;
            }
        } else { // failed: bad capacity header
            return -1;
        }
    }
    assert(offset == len);
    return len;
}
int OpenCapacities::build(byte* buf, size_t len) const {
    if(!validate())
        return -1;

    MessageHelper helper(buf, len);
    size_t offset = 0;
    // route types supported
    if(!supportedRouteTypes_.empty()) {
        OPENCapacityHeader capHdr;
        capHdr.code = VREP_Capacity_RouteTypesSupported;
        capHdr.length = VREP_MsgSize_RouteType * supportedRouteTypes_.size();
        if(offset + VREP_MsgSize_OPENCapacityHeader + capHdr.length <= len) {
            offset += buildOPENCapacityHeader(helper.buffer(offset), helper.length(offset), capHdr);
            for(RouteTypes::const_iterator it = supportedRouteTypes_.begin(); it != supportedRouteTypes_.end(); ++it) {
                offset += buildRouteType(helper.buffer(offset), helper.length(offset), *it);
            }
        } else { // buffer too small
            return -1;
        }
    }

    // send receive capacity
    if(sendReceiveCapacity_ != VREP_Invalid_SendReceiveCapacity) {
        OPENCapacityHeader capHdr;
        capHdr.code = VREP_Capacity_SendReceiveCapacity;
        capHdr.length = VREP_MsgSize_SendReceiveCapacity;
        if(offset + VREP_MsgSize_OPENCapacityHeader + capHdr.length <= len) {
            offset += buildOPENCapacityHeader(helper.buffer(offset), helper.length(offset), capHdr);
            offset += helper.putDword(sendReceiveCapacity_, offset);
        } else { // buffer too small
            return -1;
        }
    }

    // generic capacities
    for(GenericCapacities::const_iterator it = genericCapacities_.begin(); it != genericCapacities_.end(); ++it) {
        OPENCapacityHeader capHdr;
        capHdr.code = it->code;
        capHdr.length = it->data.size();
        if(offset + VREP_MsgSize_OPENCapacityHeader + capHdr.length <= len) {
            offset += buildOPENCapacityHeader(helper.buffer(offset), helper.length(offset), capHdr);
            offset += helper.put(it->data, offset);
        } else { // buffer too small
            return -1;
        }
    }
    return offset;
}
void OpenCapacities::clear() {
    supportedRouteTypes_.clear();
    sendReceiveCapacity_ = VREP_Invalid_SendReceiveCapacity;
    genericCapacities_.clear();
}


OpenParameters::OpenParameters() {
    clear();
}
void OpenParameters::setStreamingZone(const std::string& sz) {
    streamingZone_ = sz;
}
bool OpenParameters::getStreamingZone(std::string& sz) const {
    if(!streamingZone_.empty()) {
        sz = streamingZone_;
        return true;
    } else {
        return false;
    }
}

void OpenParameters::setComponentName(const std::string& cn) {
    componentName_ = cn;
}
bool OpenParameters::getComponentName(std::string& cn) const {
    if(!componentName_.empty()) {
        cn = componentName_;
        return true;
    } else {
        return false;
    }
}

void OpenParameters::setVendorString(const std::string& vs) {
    vendorString_ = vs;
}
bool OpenParameters::getVendorString(std::string& vs) const {
    if(!vendorString_.empty()) {
        vs = vendorString_;
        return true;
    } else {
        return false;
    }
}

const OpenCapacities& OpenParameters::capacities() const{
    return capacities_;
}
OpenCapacities& OpenParameters::capacities() {
    return capacities_;
}

void OpenParameters::setGenericParameters(const GenericParameters& gps) {
    genericParameters_ = gps;
}
bool OpenParameters::getGenericParameters(GenericParameters& gps) const {
    if(!genericParameters_.empty()) {
        gps = genericParameters_;
        return true;
    } else {
        return false;
    }
}
 
bool OpenParameters::validate() const {
    // mandatory parameters
    if(streamingZone_.empty() || componentName_.empty())
        return false;

    return capacities_.validate();
}

int OpenParameters::parse(const byte* buf, size_t len) {
    clear();
    // chop the parameters
    size_t offset = 0;
    while(offset < len) {
        OPENParameterHeader hdr;
        int hdrLen = parseOPENParameterHeader(hdr, buf + offset, len - offset);
        if(hdrLen > 0) { // successful
            if(offset + hdrLen + hdr.length <= len) { // valid message header
                const byte* paramVal = buf + offset + hdrLen;
                // parse the well-known parameters
                switch(hdr.type) {
                case VREP_Parameter_Capacity:
                    { // parse the capacity
                        if(capacities_.parse(paramVal, hdr.length) < 0)
                            return -1;
                    }
                    break;
                case VREP_Parameter_StreamingZone:
                    {
                        streamingZone_.assign(paramVal, paramVal + hdr.length);
                    }
                    break;
                case VREP_Parameter_ComponentName:
                    {
                        componentName_.assign(paramVal, paramVal + hdr.length);
                    }
                    break;
                case VREP_Parameter_VendorString:
                    {
                        vendorString_.assign(paramVal, paramVal + hdr.length);
                    }
                    break;
                default:
                    {
                        GenericParameter param;
                        param.type = hdr.type;
                        param.data.assign(paramVal, paramVal + hdr.length);
                        genericParameters_.push_back(param);
                    }
                    break;
                }

                // update the offset
                offset += hdrLen + hdr.length;
            } else { // bad message
                return -1;
            }
        } else { // failed: bad parameter header
            return -1;
        }
    }
    assert(offset == len);
    return len;
}
int OpenParameters::build(byte* buf, size_t len) const {
    if(!validate())
        return -1;

    MessageHelper helper(buf, len);
    size_t offset = 0;
    byte localBuf[VREP_MsgSize_Max];
    // the capacity
    int capSize = capacities_.build(localBuf, sizeof localBuf);
    if(capSize > 0) {
        // build header
        OPENParameterHeader paramHdr;
        paramHdr.type = VREP_Parameter_Capacity;
        paramHdr.length = capSize;
        if(offset + VREP_MsgSize_OPENParameterHeader + paramHdr.length <= len) {
            offset += buildOPENParameterHeader(helper.buffer(offset), helper.length(offset), paramHdr);
            offset += helper.put(localBuf, capSize, offset);
        } else { // buffer too small
            return -1;
        }
    } else if(capSize == 0) { // no capacity info
        // just ignore this field
    } else {
        return -1;
    }

    // the streaming zone info
    {
        OPENParameterHeader paramHdr;
        paramHdr.type = VREP_Parameter_StreamingZone;
        paramHdr.length = streamingZone_.size();
        if(offset + VREP_MsgSize_OPENParameterHeader + paramHdr.length <= len) {
            offset += buildOPENParameterHeader(helper.buffer(offset), helper.length(offset), paramHdr);
            offset += helper.put((const byte*)streamingZone_.data(), streamingZone_.size(), offset);
        } else { // buffer too small
            return -1;
        }
    }
    // component name
    {
        OPENParameterHeader paramHdr;
        paramHdr.type = VREP_Parameter_ComponentName;
        paramHdr.length = componentName_.size();
        if(offset + VREP_MsgSize_OPENParameterHeader + paramHdr.length <= len) {
            offset += buildOPENParameterHeader(helper.buffer(offset), helper.length(offset), paramHdr);
            offset += helper.put((const byte*)componentName_.data(), componentName_.size(), offset);
        } else { // buffer too small
            return -1;
        }
    }

    // vendor string
    if(!vendorString_.empty()) {
        OPENParameterHeader paramHdr;
        paramHdr.type = VREP_Parameter_VendorString;
        paramHdr.length = vendorString_.size();
        if(offset + VREP_MsgSize_OPENParameterHeader + paramHdr.length <= len) {
            offset += buildOPENParameterHeader(helper.buffer(offset), helper.length(offset), paramHdr);
            offset += helper.put((const byte*)vendorString_.data(), vendorString_.size(), offset);
        } else { // buffer too small
            return -1;
        }
    }

    // generic parameters
    for(GenericParameters::const_iterator it = genericParameters_.begin(); it != genericParameters_.end(); ++it) {
        OPENParameterHeader paramHdr;
        paramHdr.type = it->type;
        paramHdr.length = it->data.size();
        if(offset + VREP_MsgSize_OPENParameterHeader + paramHdr.length <= len) {
            offset += buildOPENParameterHeader(helper.buffer(offset), helper.length(offset), paramHdr);
            offset += helper.put(it->data, offset);
        } else { // buffer too small
            return -1;
        }
    }

    return offset;
}

void OpenParameters::clear() {
    streamingZone_.clear();
    componentName_.clear();
    vendorString_.clear();
    capacities_.clear();
    genericParameters_.clear();
}

OpenMessage::OpenMessage() {
    clear();
}
void OpenMessage::setVersion(byte ver) {
    version_ = ver;
}

void OpenMessage::getVersion(byte& ver) const {
    ver = version_;
}

void OpenMessage::setHoldTime(word ht) {
    if(ht != 0 && ht < VREP_HoldTime_Default)
        ht = VREP_HoldTime_Default;
 
    holdTime_ = ht;
}
void OpenMessage::getHoldTime(word& ht) const {
    ht = holdTime_;
}
void OpenMessage::setIdentifier(dword id) {
    identifier_ = id;
}
void OpenMessage::getIdentifier(dword& id) const {
    id = identifier_;
}

OpenParameters& OpenMessage::parameters() {
    return parameters_;
}
const OpenParameters& OpenMessage::parameters() const {
    return parameters_;
}

bool OpenMessage::validate() const {
    if(holdTime_ != 0 && holdTime_ < VREP_HoldTime_Default)
        return false;

    if(identifier_ == VREP_Invalid_Identifier)
        return false;

    return parameters_.validate();
}
// parse&build
int OpenMessage::parse(const byte* buf, size_t len) {
    clear();

    size_t offset = 0;
    OPENHeader hdr;
    int hdrLen = parseOPENHeader(hdr, buf, len);
    if(hdrLen > 0) {
        offset += hdrLen;
        int paramsLen = parameters_.parse(buf + offset, len - offset);
        if(paramsLen > 0) {
            offset += paramsLen;

            // get header info
            version_ = hdr.version;
            holdTime_ = hdr.holdTime;
            identifier_ = hdr.identifier;
        } else if(paramsLen == 0) { // no parameters
            // just ignore
        } else {
            clear();
            return -1;
        }
    } else {
        clear();
        return -1;
    }

    return offset;
}
int OpenMessage::build(byte* buf, size_t len) const {
    if(!validate())
        return -1;

    // get the header
    OPENHeader hdr;
    hdr.version = version_;
    hdr.reserved1 = 0;
    hdr.holdTime = holdTime_;
    hdr.reserved2 = 0;
    hdr.identifier = identifier_;
    hdr.parametersLength = 0;

    MessageHelper helper(buf, len);
    size_t offset = 0;
    int hdrLen = buildOPENHeader(buf, len, hdr);
    if(hdrLen > 0) {
        offset += hdrLen;

        int paramsLen = parameters_.build(helper.buffer(offset), helper.length(offset));
        if(paramsLen > 0) {
            // update the header
            hdr.parametersLength = paramsLen;
            buildOPENHeader(buf, len, hdr); // can't fail
            offset += paramsLen;
        } else {
            return -1;
        }
    } else {
        return -1;
    }

    return offset;
}
void OpenMessage::clear() {
    version_ = VREP_Version_Current;
    holdTime_ = VREP_HoldTime_Default;
    identifier_ = VREP_Invalid_Identifier;
    parameters_.clear();
}

void OpenMessage::textDump(std::string& data) const {
    std::ostringstream buf;
    buf << "VREP OPEN: Version(" << ((unsigned int)version_) << "), "
        << "HoldTime(" << holdTime_ << "), "
        << "Identifier(" << std::hex << std::showbase << identifier_ << ")";

    std::string v;
    if(parameters_.getStreamingZone(v)) {
        buf << ", StreamingZone(" << v << ")";
    }

    v.clear();
    if(parameters_.getComponentName(v)) {
        buf << ", ComponentName(" << v << ")";
    }

    v.clear();
    if(parameters_.getVendorString(v)) {
        buf << ", VendorString(" << v << ")";
    }

    dword sendRecvMode = 0;
    if(parameters_.capacities().getSendReceiveCapacity(sendRecvMode)) {
        buf << ", SendReceiveMode(" << std::hex << std::showbase << sendRecvMode << ")";
    }
    buf << ".";
    buf.str().swap(data);
}

}} // namespace ZQ::Vrep
