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
// Ident : $Id: TermService.h,v 1.2 2004/06/17 03:40:44 jshen Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/PT_FtpServer/TermService.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 1     08-02-15 12:38 Jie.zhang
// 
// 2     07-12-19 15:33 Fei.huang
// 
// 1     07-08-16 10:12 Jie.zhang
// 
// 2     07-03-08 11:20 Ken.qian
// 
// 1     04-12-03 13:56 Jie.zhang
// 
// 2     04-11-19 16:57 Jie.zhang
// Revision 1.2  2004/06/17 03:40:44  jshen
// ftp module 1.0
//
// Revision 1.1  2004/06/07 09:17:14  shao
// copied to production tree
//
// 03/20/2004	    0		Hui Shao	Original Program (Created)
// ===========================================================================

#ifndef __TERMSRV_H__
#define __TERMSRV_H__


#include <string>
#include "FtpSock.h"
#include "Socket.h"
using ZQ::common::Socket;

const int MAX_CMDLINE_LENGTH = 2048;
///////////////////////////////////////////////////////////////////////////////
// Supplemental class used to send and receive Terminal commands as a server
///////////////////////////////////////////////////////////////////////////////

class TermService
{
public:
	TermService(SOCKET sd = SOCK_INVALID);
	virtual ~TermService();

	//Used to set the socket descriptor
	void SetSocketDesc(SOCKET sd);

	bool getUseSSL();

	//Functions used for sending and receiving Terminal commands 
	bool sendResponse(const char *response, const char *code, int i = 0);
	bool recvCommand(std::string& command);
	bool recvCommand(char *command, int maxcmdsize);

private:

	SOCKET _sd;            //socket desc for the control connection

	int m_flagssl;          //0 = use clear text, !0 = use SSL
};

#endif //__TERMSRV_H__
