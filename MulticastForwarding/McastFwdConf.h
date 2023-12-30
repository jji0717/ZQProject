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
// Ident : $Id: McastFwdConf.h,v 1.4 2004/07/22 09:06:12 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : Define common McastFwd definitions
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MulticastForwarding/McastFwdConf.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 8     05-03-29 19:12 Kaliven.lee
// 
// 6     04-11-08 18:03 Hui.shao
// switched listener to sniffer
// 
// 5     04-11-02 12:03 Kaliven.lee
// 
// 4     04-09-15 16:32 Kaliven.lee
// add flag for packet lost test
// 
// 3     04-08-26 18:02 Kaliven.lee
// 
// 2     04-08-26 11:56 Kaliven.lee
// Revision 1.4  2004/07/22 09:06:12  shao
// checked in
//
// ===========================================================================

#ifndef __MCASTFWD_CONF_H__
#define __MCASTFWD_CONF_H__

#include "Log.h"
#include "InetAddr.h"
#include "ITVServiceTypes.h"

#define TYPEINST_TYPE           ITV_TYPE_PRIMARY_ZQ_MCASTFORWARD
#define TYPEINST_INST           1

#ifdef TESTCLIENT
#undef TYPEINST_TYPE           
#undef TYPEINST_INST           
#define TYPEINST_TYPE           100
#define TYPEINST_INST           1
#endif TESTCLIENT

#define TUNNEL_VERSION          1

#define MCASTFWD_VERSION		"1.02.004"

#define DEFAULT_TUNNEL_LISTERN_PORT		1000
#define DEFAULT_TIMEOUT					10000 // 10 sec
#define DEFAULT_REMCAST_PORT			1000


#define UDP_MAX_BUFFER_SIZE				64*1024

#define TUNNEL_SCANER_TIME_OUT			2000

#define MCASTFWD_PEER_STAMP				'%'

#ifndef _DEBUG
#undef  ALLOW_TUNNEL_SELF
#endif // !_DEBUG

#define Socket_Pref_Name "Socket"
#define Address_Pref_Name "Address"

#define  ALLOW_TUNNEL_SELF

#define DEFAULT_QUEUE_SIZE             100

extern ZQ::common::Log* pProglog;
extern ZQ::common::InetHostAddress gLocalAddrs;

#define log      (*pProglog)

#endif // __MCASTFWD_CONF_H__