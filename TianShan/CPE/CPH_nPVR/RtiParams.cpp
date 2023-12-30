// ===========================================================================
// Copyright (c) 2008 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : 
// Branch: 
// Author: 
// Desc  : 
//
// Revision History: 
// ===========================================================================

#include "RtiParams.h"

namespace ZQTianShan 
{
namespace ContentProvision
{

RtiParams::RtiParams()
{
	_nPort = 0;
	_ctType = MPEG2;	
	_nBandwidth = 0;
}

void RtiParams::setFilename(const std::string& strFilename)
{
	_strFilename = strFilename;
}
	
void RtiParams::setMulticast(const std::string& Ip, int port)
{
	_strMulticastIp = Ip;
	_nPort = port;
}

void RtiParams::setContentType(ContentType nType)
{
	_ctType	= nType;
}
	
void RtiParams::setBandwidth(int nBandwidth)
{
	_nBandwidth	= nBandwidth;
}

void RtiParams::copy(const RtiParams* pParam)
{
	*this = *pParam;
}

void RtiParams::setFileSuffix(std::vector<std::string>& suffix)
{
	_suffixVec = suffix;
}

SessionGroupId RtiParams::getSessionGroup()
{
	return _sessionGroup;
}

void RtiParams::setSessionGroup(const SessionGroupId& sgId)
{
	_sessionGroup = sgId;
}

void RtiParams::setSessionGroup(const std::string& strFileName)
{
	_sessionGroup.assign(strFileName);
}


}
}

