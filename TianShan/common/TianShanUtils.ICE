// ===========================================================================
// Copyright (c) 2004 by
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
// Ident : $Id: ZqSentryIce.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/common/TianShanUtils.ICE $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 1     08-10-16 19:43 Hui.shao
// 
// 4     08-06-06 12:06 Xiaohui.chai
// added getWebView() to SentryService
// 
// 3     07-05-22 17:30 Hui.shao
// added exporting logger information
// 
// 2     07-05-21 11:52 Hui.shao
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice  

#ifndef __ZQ_Node_ICE__
#define __ZQ_Node_ICE__

#include "TianShanIce.ICE"

#include <Ice/Identity.ice>

module TianShanUtils
{

interface TimeoutObj
{
	void OnTimer();
};

};

#endif // __ZQ_Node_ICE__
