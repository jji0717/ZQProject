#ifndef _RTSPNoticeHeader_H_
#define _RTSPNoticeHeader_H_

#include "ZQ_common_conf.h"
#include "../Common.h"
static const string strInProgress = "5700";
static const string strEndOfStream = "2101";
static const string strStartOfStream = "2104";

//definition of Notice header
typedef struct RTSPNoticeHeader
{
	string		strNotice_code;			//<Notice-code>
	string		strText_description;	//<text description>
	string		strDatetime;			//<datetime>
	string		strNpt_value;			//<<npt-value>
}RTSPNoticeHeader;

typedef vector<RTSPNoticeHeader> RTSPNoticeHeaderVec;

#endif