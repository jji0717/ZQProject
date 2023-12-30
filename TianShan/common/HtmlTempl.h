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
// Ident : $Id: HtmlTempl.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/common/HtmlTempl.h $
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 5     08-02-28 16:59 Xiaohui.chai
// added page attribute charset
// 
// 4     08-02-19 17:34 Xiaohui.chai
// 
// 3     08-02-19 14:42 Xiaohui.chai
// 
// 2     07-06-04 14:46 Hui.shao
// add log to Adapter
// ===========================================================================
#ifndef __ZQTianShan_HtmlTempl_H__
#define __ZQTianShan_HtmlTempl_H__

#include "TianShanDefines.h"
#include <iostream>

namespace ZQTianShan {
namespace HtmlTempl {

typedef struct _Tab
{
	std::string name;
	std::string href;
} Tab;

void HtmlHeader_MainPage(std::ostream& out, const char* subTitle, const Tab tabs[], const int tabCount, const int current =0, const char* charset=NULL);
void HtmlFooter_MainPage(std::ostream& out, const char* hostname, const char* url=NULL);
void HtmlFooter_MainPage_trivial(std::ostream& out);
}} // namespace

#endif // __ZQTianShan_HtmlTempl_H__
