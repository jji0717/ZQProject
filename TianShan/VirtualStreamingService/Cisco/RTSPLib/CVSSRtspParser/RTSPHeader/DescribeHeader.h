#ifndef __DESCRIBEHEADER_H__
#define __DESCRIBEHEADER_H__

#include "../Common.h"

//the following header field should be parsed in every RTSP interaction
static const ::std::string g_strARange = "a=range:";

typedef struct DescribeResHeader 
{
	//RTSP response "a=range:npt=_strRangeStart-_strRangeEnd" 
	::std::string	_strRangeStart;
	::std::string	_strRangeEnd;
}DescribeResHeader;

#endif __DESCRIBEHEADER_H__