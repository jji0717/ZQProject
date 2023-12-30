// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: TsClient.h $
// Branch: $Name:  $
// Author: Jie Zhang
// Desc  : 
//
// Revision History: 
// ===========================================================================

#ifndef _TIANSHAN_TSCLIENT_H_
#define _TIANSHAN_TSCLIENT_H_

#include "TsLayout.h"
#include "ZQ_common_conf.h"

using namespace ZQTianShan::Layout;


#ifdef TSCLIENT_EXPORTS
#define TSCLIENT_API __EXPORT
#else
#define TSCLIENT_API __DLLRTL
#endif

extern "C" {

TSCLIENT_API int TCInitialize(const char* szLogFile);


TSCLIENT_API void TCUninitialize();


TSCLIENT_API int TCFillGrid(PLayoutCtx ctx);


TSCLIENT_API int TCPopulateSnmpVariables(PLayoutCtx ctx, const char* baseOid, bool bPost=false, const char* snmpServer = NULL, const char *community = NULL);

}



#endif
