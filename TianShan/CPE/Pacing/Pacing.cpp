// ===========================================================================
// Copyright (c) 2010 by
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


#ifdef ZQ_OS_MSWIN
#include "stdafx.h"
#endif
#include "Pacing.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}
#endif

#ifdef _MANAGED
#pragma managed(pop)
#endif


#include "PacedIndex.h"

extern "C"
{
	__EXPORT bool CreatePacedIndexFactory(PacedIndexFactory** pFactroyPointer)
{
	if (!pFactroyPointer)
		return false;

	*pFactroyPointer = new PacedIndexVvcFactory();
	return true;
}

__EXPORT void DestroyPacedIndexFactory(PacedIndexFactory* pFactory)
{
	if (pFactory)
	{
		delete pFactory;
	}
}
}

