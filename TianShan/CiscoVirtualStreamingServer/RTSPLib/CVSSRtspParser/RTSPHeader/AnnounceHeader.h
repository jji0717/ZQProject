#ifndef __ANNOUNCEHEADER_H__
#define __ANNOUNCEHEADER_H__

#include "../Common.h"

static const ::std::string strEndOfStream = "2101";
static const ::std::string strStartOfStream = "2104";
static const ::std::string strErrReadContent = "4400";
static const ::std::string strClientTerminate = "5402";

//the following header field should be parsed in ANNOUNCE request
static const ::std::string g_strXNoticeHeader = "x-notice:";
static const ::std::string g_strEventDate = "Event-Date=";

typedef struct AnnounceReqHeader 
{
	::std::string		_strEventCode;			//<Notice-code>
	::std::string		_strEventPhrase;		//<text description>
	::std::string		_strEventDate;			//<date time>
}AnnounceReqHeader;

typedef struct AnnounceResHeader 
{
	int32			_iCSeq;			//RTSP server response sequence number 
}AnnounceResHeader;
#endif __ANNOUNCEHEADER_H__