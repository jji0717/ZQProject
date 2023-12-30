#ifndef _SETUPHEADER_H__
#define _SETUPHEADER_H__

#include "../Common.h"

//the header must be included in SETUP request
static const ::std::string g_strNotify = "x-mayNotify:";

typedef struct SetupReqHeader 
{
	//"Transport:" header field params
	::std::string	_strDestination;	//destination=_strDestination
	int32			_iClientPort;		//client_port=_iClientPort
}SetupReqHeader;

typedef struct SetupResHeader 
{
	int32			_iTimeout;			//timeout= field value
	::std::string	_strClient;			//client= field value
	::std::string	_strControlAddress;	//control_address= field value
	::std::string	_strDestinationIp;	//destination= field value
	int32			_strDestinationPort;//destination= field value
	int32			_iBandWidth;		//bandwidth= field value
}SetupResHeader;
#endif _SETUPHEADER_H__