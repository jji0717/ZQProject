#ifndef _RTSPREASONHEADER_H_
#define _RTSPREASONHEADER_H_

#include "ZQ_common_conf.h"
#include "../Common.h"

//definition of ReasonHeader
typedef struct RTSPReasonHeader
{
	string	strReason_code;				//<reason-code>
	string	strReason_text;				//<reason-text>
}RTSPReasonHeader;

#endif