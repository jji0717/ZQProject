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



#include "NPVRTargetFactory.h"
#include "VstrmNPVRTarget.h"

namespace ZQTianShan 
{
namespace ContentProvision
{

NPVRTargetFactory::NPVRTargetFactory(FileIoFactory* pFactory)
	:_pFileIoFac(pFactory)
{
}

BaseTarget* NPVRTargetFactory::create(const char* szName)
{
	if (!stricmp(szName, TARGET_TYPE_VSTRMNPVR))
		return new VstrmNPVRTarget(_pFileIoFac);

	return NULL;
}

}}