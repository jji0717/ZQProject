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

#ifndef ZQTS_CPE_RTIPARAMS_H
#define ZQTS_CPE_RTIPARAMS_H

#include <string>
#include <vector>
#include "SessionGroupId.h"

namespace ZQTianShan 
{
namespace ContentProvision
{

class RtiParams
{
public:
	enum ContentType{
		MPEG2,
		H264
	};

	RtiParams();

	virtual void setFilename(const std::string& strFilename);
	virtual void setMulticast(const std::string& Ip, int port);
	virtual void setContentType(ContentType nType);
	virtual void setBandwidth(int nBandwidth);
	virtual void setFileSuffix(std::vector<std::string>& suffix);
	virtual void setSessionGroup(const SessionGroupId& sgId);
	virtual void setSessionGroup(const std::string& strFileName);

	virtual SessionGroupId getSessionGroup();
	
	virtual void copy(const RtiParams* pParam);
	
protected:
	std::string						_strFilename;	
	std::string						_strMulticastIp;
	int 							_nPort;
	int 							_nBandwidth;
	std::vector<std::string>		_suffixVec;
	ContentType						_ctType;
	SessionGroupId					_sessionGroup;
};


}
}

#endif