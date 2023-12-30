#ifndef _RTSPREQUIREHEADER_H_
#define _RTSPREQUIREHEADER_H_

#include "ZQ_common_conf.h"
#include "../Common.h"

//definition of RequireHeader
typedef struct RTSPRequireHeader
{
	string		strComPath;			//example: comd.comcast.ngod
	string		strInterface_id;	//<interface-id>
}RTSPRequireHeader;

#endif