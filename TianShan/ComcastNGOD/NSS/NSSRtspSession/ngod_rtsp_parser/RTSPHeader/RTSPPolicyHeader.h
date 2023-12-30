#ifndef _RTSPPOLICYHEADER_H_
#define _RTSPPOLICYHEADER_H_

#include "ZQ_common_conf.h"
#include "../Common.h"
//definition of PolicyHeader
typedef struct RTSPPolicyHeader
{
	string		strPolicy_name;			//<policy-name>
	string		strPolicy_value;		//<policy-value>
}RTSPPolicyHeader;

typedef vector<RTSPPolicyHeader> RTSPPolicyHeaderVec;

#endif