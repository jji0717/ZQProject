#ifndef __COMMONHEADER_H__
#define __COMMONHEADER_H__

#include "../Common.h"

typedef struct CommonReqHeader 
{
	::std::string	_strServerIp;	//RTSP server ip address
	int32			_iServerPort;	//RTSP server listening port
	::std::string	_strContentId;	//RTSP request URI content identifier
	int32			_iCSeq;			//RTSP request client sequence number
	::std::string	_strSessionId;	//RTSP request sessionId(not in DESCRIBE)
	::std::string	_strScale;		//optional, RTSP request "Scale: " header field
	::std::string	_strRang;		//optional, RTSP request "Range: npt=_strRange-" 

	const char *getURI()
	{
		::std::stringstream ss;
		ss << _iServerPort;
		::std::string strURI = ::std::string("RTSP://") + _strServerIp + "/" + ss.str() + "/" + _strContentId;
		return strURI.c_str();
	}
}CommonReqHeader;

//the following header field should be parsed in every RTSP interaction
static const ::std::string g_strCSeq = "CSeq:";
static const ::std::string g_strSession = "Session:";
static const ::std::string g_strScale = "Scale:";
static const ::std::string g_strRange = "Range:";
static const ::std::string g_strNPT = "npt=";
static const ::std::string g_strHyphen = "-";
static const ::std::string g_strColon = ":";

typedef struct CommonResHeader 
{
	int32			_iCSeq;			//RTSP server response sequence number
	::std::string	_strSessionId;	//RTSP server response sessionId(not in DESCRIBE)
	int32			_iTimeOut;		//RTSP session timeout, in SETUP response
	::std::string	_strScale;		//RTSP response "Scale: " header field
	::std::string	_strRangeStart;	//RTSP response "Range: npt=_strRangeStart-_strRangeEnd" 
	::std::string	_strRangeEnd;
}CommonResHeader;

#endif __COMMONHEADER_H__