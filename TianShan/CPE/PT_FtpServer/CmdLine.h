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
// Ident : $Id: CmdLine.h,v 1.4 2004/07/06 07:20:43 jshen Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : handle the ftp command 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/PT_FtpServer/CmdLine.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 1     08-02-15 12:38 Jie.zhang
// 
// 2     07-12-19 15:30 Fei.huang
// 
// 1     07-08-16 10:12 Jie.zhang
// 
// 2     04-12-08 15:50 Jie.zhang
// 
// 2     04-11-19 17:05 Jie.zhang
// Revision 1.4  2004/07/06 07:20:43  jshen
// add skeleton for SeaChange AppShell Service
//
// Revision 1.3  2004/07/05 02:18:50  jshen
// add comments
//
// Revision 1.2  2004/06/17 03:40:44  jshen
// ftp module 1.0
//
// Revision 1.1  2004/06/07 09:19:43  shao
// copied to production tree
//
// 03/20/2004	    0		Hui Shao	Original Program (Created)
// ===========================================================================

#ifndef __CMDLINE_H__
#define __CMDLINE_H__

#include <string>

const int MAX_CMD_LINE = 2048;
const int MAX_CMD_ARG = 20;

class CmdLine  
{
public:
	CmdLine();
	virtual ~CmdLine();

	void set(const char *cmdline);
	void get(char *outbuff, int maxbuffsize);
	void resize(int cmdlinesize);
	int size();

	char **parse(int *argc, char delimiter = ' ', int usepadding = 0);

	static bool combineArgs(std::string& outline, int argc, const char **argv, char optionchar = 0, int startarg = 1);

	static bool combineArgs(char* outline, int argc, const char **argv, char optionchar = 0, int startarg = 1);
public:
	std::string _cmdline;    //pointer to the command line buffer

private:
	char _cmdbuffer[MAX_CMD_LINE];  //stores the contents of the command line
	char *_argptr[MAX_CMD_ARG];    //points to the individual parameters in _cmdbuffer
};

#endif //__CMDLINE_H__
