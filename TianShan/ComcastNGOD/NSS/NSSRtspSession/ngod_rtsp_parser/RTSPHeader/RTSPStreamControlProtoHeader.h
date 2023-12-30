#ifndef _RTSPSTREAMCONTROLPROTOHEADER_H_
#define _RTSPSTREAMCONTROLPROTOHEADER_H_

#include "ZQ_common_conf.h"
#include "../Common.h"

typedef struct RTSPStreamControlProto
{
	string	strType;	//<type>
}RTSPStreamControlProto;

typedef vector<RTSPStopPointHeader> RTSPStreamControlProtoVec;

#endif