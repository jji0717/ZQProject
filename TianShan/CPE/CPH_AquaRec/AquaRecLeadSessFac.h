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

#ifndef ZQTS_CPE_AQUARECLEADSESSFAC_H
#define ZQTS_CPE_AQUARECLEADSESSFAC_H

class CdmiClientBase;

namespace ZQ
{
	namespace common{
		class NativeThreadPool;
	}
}

namespace ZQTianShan 
{
namespace ContentProvision
{

class AquaRecLeadSessI;

class AquaRecLeadSessFac
{
public:
	AquaRecLeadSessFac(ZQ::common::NativeThreadPool* pool, CdmiClientBase* pCdmiClient): _pCdmiClient(pCdmiClient)
	{
		_threadPool=pool;
	}

	virtual AquaRecLeadSessI* create();

protected:
	ZQ::common::NativeThreadPool*		_threadPool;
	CdmiClientBase*                    _pCdmiClient;
};

}
}

#endif
