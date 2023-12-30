#include "VrepMessage.h"
#include <assert.h>

namespace ZQ {
namespace Vrep {
/*
  0                   1                   2
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
  +--------------+----------------+---------------+
  |          Length               |      Type     |
  +--------------+----------------+---------------+
*/
int parseVREPHeader(VREPHeader& hdr, const byte* data, size_t len) {
    if(len < VREP_MsgSize_Header)
        return -1;

    // the message helper call shouldn't fail here
    MessageHelper helper((byte*)data, len);
    size_t offset = 0;
    offset += helper.getWord(hdr.length, offset);
    offset += helper.getByte(hdr.type, offset);

    assert(offset == VREP_MsgSize_Header);
    return offset;
}

int buildVREPHeader(byte* data, size_t len, const VREPHeader& hdr) {
    if(len < VREP_MsgSize_Header)
        return -1;

    // the message helper call shouldn't fail here
    MessageHelper helper(data, len);
    size_t offset = 0;
    offset += helper.putWord(hdr.length, offset);
    offset += helper.putByte(hdr.type, offset);

    assert(offset == VREP_MsgSize_Header);
    return offset;
}

/*
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |    Version    |    Reserved   |          Hold Time            |
  +---------------+---------------+--------------+----------------+
  |                            reserved                           |
  +---------------+---------------+--------------+----------------+
  |                        VREP Identifier                        |
  +---------------+---------------+--------------+----------------+
  |    Parameters Len             | Parameters (variable)...      |
  +---------------+---------------+--------------+----------------+
*/
int parseOPENHeader(OPENHeader& hdr, const byte* data, size_t len) {
    if(len < VREP_MsgSize_OPENHeader)
        return -1;

    // the message helper call shouldn't fail here
    MessageHelper helper((byte*)data, len);
    size_t offset = 0;
    offset += helper.getByte(hdr.version, offset);
    offset += helper.getByte(hdr.reserved1, offset);
    offset += helper.getWord(hdr.holdTime, offset);
    offset += helper.getDword(hdr.reserved2, offset);
    offset += helper.getDword(hdr.identifier, offset);
    offset += helper.getWord(hdr.parametersLength, offset);

    assert(offset == VREP_MsgSize_OPENHeader);
    return offset;
}
int buildOPENHeader(byte* data, size_t len, const OPENHeader& hdr) {
    if(len < VREP_MsgSize_OPENHeader)
        return -1;

    // the message helper call shouldn't fail here
    MessageHelper helper(data, len);
    size_t offset = 0;
    offset += helper.putByte(hdr.version, offset);
    offset += helper.putByte(hdr.reserved1, offset);
    offset += helper.putWord(hdr.holdTime, offset);
    offset += helper.putDword(hdr.reserved2, offset);
    offset += helper.putDword(hdr.identifier, offset);
    offset += helper.putWord(hdr.parametersLength, offset);

    assert(offset == VREP_MsgSize_OPENHeader);
    return offset;
}

/*
  0                   1                   2
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |       Parameter Type          |       Parameter Length        |
  +---------------+---------------+--------------+----------------+
  |                  Parameter Value (variable)...
  +---------------+---------------+--------------+----------------+
*/
int parseOPENParameterHeader(OPENParameterHeader& hdr, const byte* data, size_t len) {
    if(len < VREP_MsgSize_OPENParameterHeader)
        return -1;

    // the message helper call shouldn't fail here
    MessageHelper helper((byte*)data, len);
    size_t offset = 0;
    offset += helper.getWord(hdr.type, offset);
    offset += helper.getWord(hdr.length, offset);

    assert(offset == VREP_MsgSize_OPENParameterHeader);
    return offset;
}

int buildOPENParameterHeader(byte* data, size_t len, const OPENParameterHeader& hdr) {
    if(len < VREP_MsgSize_OPENParameterHeader)
        return -1;

    // the message helper call shouldn't fail here
    MessageHelper helper(data, len);
    size_t offset = 0;
    offset += helper.putWord(hdr.type, offset);
    offset += helper.putWord(hdr.length, offset);

    assert(offset == VREP_MsgSize_OPENParameterHeader);
    return offset;
}

/*
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |  Error Code   | Error Subcode |       Data... (variable)
  +---------------+---------------+--------------+----------------+
*/
int parseNOTIFICATIONHeader(NOTIFICATIONHeader& hdr, const byte* data, size_t len) {
    if(len < VREP_MsgSize_NOTIFICATIONHeader)
        return -1;

    // the message helper call shouldn't fail here
    MessageHelper helper((byte*)data, len);
    size_t offset = 0;
    offset += helper.getByte(hdr.code, offset);
    offset += helper.getByte(hdr.subcode, offset);

    assert(offset == VREP_MsgSize_NOTIFICATIONHeader);
    return offset;
}
int buildNOTIFICATIONHeader(byte* data, size_t len, const NOTIFICATIONHeader& hdr) {
    if(len < VREP_MsgSize_NOTIFICATIONHeader)
        return -1;

    // the message helper call shouldn't fail here
    MessageHelper helper(data, len);
    size_t offset = 0;
    offset += helper.putByte(hdr.code, offset);
    offset += helper.putByte(hdr.subcode, offset);

    assert(offset == VREP_MsgSize_NOTIFICATIONHeader);
    return offset;
}

/*
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |  Attr. Flags  |Attr. Type Code|         Attr. Length          |
  +---------------+---------------+--------------+----------------+
  |                   Attribute Value (variable)                  |
  +---------------+---------------+--------------+----------------+
*/
int parseUPDATEAttributeHeader(UPDATEAttributeHeader& hdr, const byte* data, size_t len) {
    if(len < VREP_MsgSize_UPDATEAttributeHeader)
        return -1;

    // the message helper call shouldn't fail here
    MessageHelper helper((byte*)data, len);
    size_t offset = 0;
    offset += helper.getByte(hdr.flags, offset);
    offset += helper.getByte(hdr.type, offset);
    offset += helper.getWord(hdr.length, offset);

    assert(offset == VREP_MsgSize_UPDATEAttributeHeader);
    return offset;
}
int buildUPDATEAttributeHeader(byte* data, size_t len, const UPDATEAttributeHeader& hdr) {
    if(len < VREP_MsgSize_UPDATEAttributeHeader)
        return -1;

    // the message helper call shouldn't fail here
    MessageHelper helper(data, len);
    size_t offset = 0;
    offset += helper.putByte(hdr.flags, offset);
    offset += helper.putByte(hdr.type, offset);
    offset += helper.putWord(hdr.length, offset);

    assert(offset == VREP_MsgSize_UPDATEAttributeHeader);
    return offset;
}

/*
  0                   1                   2
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |       Capability Code         |       Capability Length       |
  +---------------+---------------+--------------+----------------+
  |       Capability Value (variable)...
  +---------------+---------------+--------------+----------------+
*/
int parseOPENCapacityHeader(OPENCapacityHeader& hdr, const byte* data, size_t len) {
    if(len < VREP_MsgSize_OPENCapacityHeader)
        return -1;

    // the message helper call shouldn't fail here
    MessageHelper helper((byte*)data, len);
    size_t offset = 0;
    offset += helper.getWord(hdr.code, offset);
    offset += helper.getWord(hdr.length, offset);

    assert(offset == VREP_MsgSize_OPENCapacityHeader);
    return offset;
}
int buildOPENCapacityHeader(byte* data, size_t len, const OPENCapacityHeader& hdr) {
    if(len < VREP_MsgSize_OPENCapacityHeader)
        return -1;

    // the message helper call shouldn't fail here
    MessageHelper helper(data, len);
    size_t offset = 0;
    offset += helper.putWord(hdr.code, offset);
    offset += helper.putWord(hdr.length, offset);

    assert(offset == VREP_MsgSize_OPENCapacityHeader);
    return offset;
}

/*
  RouteType
  0                   1                   2
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |        Address Family         |     Application Protocol      |
  +---------------+---------------+--------------+----------------+
*/
int parseRouteType(RouteType& rt, const byte* data, size_t len) {
    if(len < VREP_MsgSize_RouteType)
        return -1;

    // the message helper call shouldn't fail here
    MessageHelper helper((byte*)data, len);
    size_t offset = 0;
    offset += helper.getWord(rt.family, offset);
    offset += helper.getWord(rt.protocol, offset);

    assert(offset == VREP_MsgSize_RouteType);
    return offset;
}
int buildRouteType(byte* data, size_t len, const RouteType& rt) {
    if(len < VREP_MsgSize_RouteType)
        return -1;

    // the message helper call shouldn't fail here
    MessageHelper helper(data, len);
    size_t offset = 0;
    offset += helper.putWord(rt.family, offset);
    offset += helper.putWord(rt.protocol, offset);

    assert(offset == VREP_MsgSize_RouteType);
    return offset;
}

}} // namespace ZQ::Vrep
