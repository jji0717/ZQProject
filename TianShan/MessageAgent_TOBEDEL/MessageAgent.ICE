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
// Ident : $Id: Message.ICE $
// Branch: $Name:  $
// Author: Ken Qian
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/MessageAgent/MessageAgent.ICE $
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// ===========================================================================
// build steps:
//$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/TianShan/Ice $(InputPath)  
//$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/TianShan/Ice --dict Messages,string,TianShanIce::MessageAgent::Message MessageData $(InputPath) 

#ifndef __ZQ_TianShanIce_Message_ICE__
#define __ZQ_TianShanIce_Message_ICE__

#include "TianShanIce.ICE"
#include <Ice/Identity.ice>

module TianShanIce
{
module MessageAgent
{

struct Message
{
	string content;
};



};
};


#endif