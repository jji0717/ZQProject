#ifndef _RTSPSTARTPOINTHEADER_H_
#define _RTSPSTARTPOINTHEADER_H_

#include "ZQ_common_conf.h"
#include "../Common.h"

typedef struct RTSPStarPointHeader
{
	int32	iSlot;	//<slot>
	int32	iNpt;	//<npt>
}RTSPStarPointHeader;

#endif