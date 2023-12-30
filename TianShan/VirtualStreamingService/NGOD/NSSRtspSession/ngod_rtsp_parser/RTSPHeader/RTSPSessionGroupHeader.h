#ifndef _RTSPSESSIONGROUPHEADER_H_
#define _RTSPSESSIONGROUPHEADER_H_

#include "ZQ_common_conf.h"
#include "../Common.h"

//definition of SessionGroupHeader
typedef struct RTSPSessionGroupHeader
{
	string		strToken;				//<token>
}RTSPSessionGroupHeader;

typedef struct RTSPSessionGroupsHeader
{
	vector<string> strSessionGroup;
}RTSPSessionGroupsHeader;

#endif