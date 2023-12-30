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


#include "LeadSessCol.h"
#include "LeadSessI.h"
#include "VirtualSessI.h"
#include "RtiParams.h"
#include "VirtualSessFac.h"
#include "Log.h"

namespace ZQTianShan 
{
namespace ContentProvision
{

void createSubjectCol(ZQ::common::NativeThreadPool* pool, IMemAlloc* pAlloc)
{
	LeadSessColI* pSubjectCol = new LeadSessCol(pool, pAlloc);
	LeadSessColI::setInstance(pSubjectCol);
}

VirtualSessI* preload(const std::string& filename, const std::string& multicastIp, int nPort, ZQ::common::Log* pLog)
{
	LeadSessColI* pSubjectCol = LeadSessColI::instance();

	SessionGroupId sgId(filename);
	//sgId
	
	bool bCreated = false;
	LeadSessI* pLeadSess = pSubjectCol->get(sgId, bCreated);
	
	if (bCreated)
	{
		pLeadSess->setFilename(filename);
		pLeadSess->setMulticast(multicastIp, nPort);
		pLeadSess->setContentType(RtiParams::MPEG2);

		pLeadSess->SetLog(pLog);
		pLeadSess->Init();
	}

	VirtualSessFac vsFac;
	VirtualSessI* pVirSess = vsFac.create();

	pVirSess->setSubject(pLeadSess);
	
	pVirSess->Init();

	return pVirSess;
}

void execute(VirtualSessI* pVirSess)
{
	//pVirSess->
}


}
}

