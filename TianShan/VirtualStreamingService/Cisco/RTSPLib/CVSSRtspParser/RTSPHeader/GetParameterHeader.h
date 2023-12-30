#ifndef __GETPARAMETERHEADER_H__
#define __GETPARAMETERHEADER_H__

#include "../Common.h"

//the following header field should be include in GETPARAMETER request message
static const ::std::string g_strPosition = "position:";

typedef struct GetParameterReqHeader 
{
	::std::string	_strSDPField;	//the field should list in SDP(empty field means heartbeat message)
}GetParameterReqHeader;

typedef struct GetParameterResHeader 
{
	::std::string	_strRange;	//"position: npt=_strRange", represent the session current play posision)
}GetParameterResHeader;

#endif __GETPARAMETERHEADER_H__