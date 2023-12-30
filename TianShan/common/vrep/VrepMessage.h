#ifndef __ZQ_VREP_Message_H__
#define __ZQ_VREP_Message_H__

#include "vrep.h"
#include "VrepUtils.h"

namespace ZQ {
namespace Vrep {

#define VREP_MsgSize_Min 3
#define VREP_MsgSize_Max 4096

/*
  0                   1                   2
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
  +--------------+----------------+---------------+
  |          Length               |      Type     |
  +--------------+----------------+---------------+
*/
#define VREP_MsgSize_Header 3

struct VREPHeader
{
    word length;
    byte type;
};
int parseVREPHeader(VREPHeader&, const byte*, size_t);
int buildVREPHeader(byte*, size_t, const VREPHeader&);

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
#define VREP_MsgSize_OPENHeader 14
#define VREP_MsgSize_OPEN_Min (VREP_MsgSize_Header + VREP_MsgSize_OPENHeader)
struct OPENHeader
{
    byte version;
    byte reserved1;
    word holdTime;
    dword reserved2;
    dword identifier;
    word parametersLength;
};
int parseOPENHeader(OPENHeader&, const byte*, size_t);
int buildOPENHeader(byte*, size_t, const OPENHeader&);

/*
  0                   1                   2
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |       Parameter Type          |       Parameter Length        |
  +---------------+---------------+--------------+----------------+
  |                  Parameter Value (variable)...
  +---------------+---------------+--------------+----------------+
*/
#define VREP_MsgSize_OPENParameterHeader 4
struct OPENParameterHeader
{
    word type;
    word length;
};
int parseOPENParameterHeader(OPENParameterHeader&, const byte*, size_t);
int buildOPENParameterHeader(byte*, size_t, const OPENParameterHeader&);

/*
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |  Error Code   | Error Subcode |       Data... (variable)
  +---------------+---------------+--------------+----------------+
*/
#define VREP_MsgSize_NOTIFICATIONHeader 2
#define VREP_MsgSize_NOTIFICATION_Min (VREP_MsgSize_Header + VREP_MsgSize_NOTIFICATIONHeader)
struct NOTIFICATIONHeader
{
    byte code;
    byte subcode;
};
int parseNOTIFICATIONHeader(NOTIFICATIONHeader&, const byte*, size_t);
int buildNOTIFICATIONHeader(byte*, size_t, const NOTIFICATIONHeader&);

// no KEEPALIVE and UPDATE header parsing&building needed
#define VREP_MsgSize_UPDATE_Min VREP_MsgSize_Header
/*
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |  Attr. Flags  |Attr. Type Code|         Attr. Length          |
  +---------------+---------------+--------------+----------------+
  |                   Attribute Value (variable)                  |
  +---------------+---------------+--------------+----------------+
*/
#define VREP_MsgSize_UPDATEAttributeHeader 4
struct UPDATEAttributeHeader
{
    byte flags;
    byte type;
    word length;
};
int parseUPDATEAttributeHeader(UPDATEAttributeHeader&, const byte*, size_t);
int buildUPDATEAttributeHeader(byte*, size_t, const UPDATEAttributeHeader&);


/*
  0                   1                   2
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |       Capability Code         |       Capability Length       |
  +---------------+---------------+--------------+----------------+
  |       Capability Value (variable)...
  +---------------+---------------+--------------+----------------+
*/
#define VREP_MsgSize_OPENCapacityHeader 4
struct OPENCapacityHeader
{
    word code;
    word length;
};
int parseOPENCapacityHeader(OPENCapacityHeader&, const byte*, size_t);
int buildOPENCapacityHeader(byte*, size_t, const OPENCapacityHeader&);

/*
  RouteType
  0                   1                   2
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |        Address Family         |     Application Protocol      |
  +---------------+---------------+--------------+----------------+
*/
#define VREP_MsgSize_RouteType 4
struct RouteType
{
    word family;
    word protocol;
};
int parseRouteType(RouteType&, const byte*, size_t);
int buildRouteType(byte*, size_t, const RouteType&);

/*
  SendReceiveMode
  0                   1                   2
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |                   Send Receive Capacity                       |
  +---------------+---------------+--------------+----------------+
*/
#define VREP_MsgSize_SendReceiveCapacity 4

// generic route format
/*
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +---------------+---------------+--------------+----------------+
  |       Address Family          |      Application Protocol     |
  +---------------+---------------+--------------+----------------+
  |            Length             |       Address (variable)     ...
  +---------------+---------------+--------------+----------------+
*/
#define VREP_MsgSize_RouteHeader 6
struct RouteHeader
{
    word family;
    word protocol;
    word length;
};

int parseRouteHeader(RouteHeader&, const byte*, size_t);
int buildRouteHeader(byte*, size_t, const RouteHeader&);

}} // namespace ZQ::Vrep

#endif
