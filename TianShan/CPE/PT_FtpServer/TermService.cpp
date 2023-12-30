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
// Ident : $Id: TermService.cpp,v 1.3 2004/06/18 04:11:46 jshen Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/PT_FtpServer/TermService.cpp $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 3     08-12-19 16:28 Yixin.tian
// 
// 2     08-02-18 18:29 Jie.zhang
// changes check in
// 
// 1     08-02-15 12:38 Jie.zhang
// 
// 3     07-12-21 16:52 Fei.huang
// minor fixes, remove warning of size mismatch in copy expression while
// compiling
// 
// 2     07-12-19 15:33 Fei.huang
// 
// 1     07-08-16 10:12 Jie.zhang
// 
// 1     06-08-30 12:33 Jie.zhang
// 
// 1     05-09-06 13:58 Jie.zhang
// 
// 3     05-08-08 12:23 Jie.zhang
// 
// 3     05-06-11 18:04 Jie.zhang
// 
// 2     05-02-06 17:03 Jie.zhang
// 
// 1     04-12-03 13:56 Jie.zhang
// 
// 2     04-11-19 16:57 Jie.zhang
// Revision 1.3  2004/06/18 04:11:46  jshen
// no message
//
// Revision 1.2  2004/06/17 03:40:44  jshen
// ftp module 1.0
//
// Revision 1.1  2004/06/07 09:17:14  shao
// copied to production tree
//
// 03/20/2004	    0		Hui Shao	Original Program (Created)
// ===========================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TermService.h"
#include "utils.h"
#include "CECommon.h"

#define _OUTPUT_LOG_


#define TermSr			"TermSr"
#define MOLOG					glog

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Class for sending and receiving Terminal commands as a server
// Ex. Commands for FTP, SMTP, etc.
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Constructor for the TermService class
//
// [in] sd : Socket descriptor that will be used for sending and
//           receiving terminal commands.
//
TermService::TermService(SOCKET sd /*=SOCK_INVALID*/)
{
	_sd = sd;  //set the socket desc for the ctrl connection
	m_flagssl = 0;  //SSL is not used by default
}

TermService::~TermService()
{
}

//////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Set the socket desc for the ctrl connection
//
// [in] sd : Socket descriptor that will be used for sending and
//           receiving terminal commands.
//
// Return : VOID
//
void TermService::SetSocketDesc(SOCKET sd)
{
	_sd = sd;
}

//////////////////////////////////////////////////////////////////////
// Get the connection type (SSL or Clear).
//
// Return : 0 is returned if clear text is being used and 1 is
//          returned if SSL is being used.
//
bool TermService::getUseSSL()
{
	return(m_flagssl!=0);
}

#ifdef _OUTPUT_LOG_
#include "Log.h"
using namespace ZQ::common;
#endif
//////////////////////////////////////////////////////////////////////
// Send a line of text to the client.
//
// [in] response : Response to send back to the client.
// [in] code     : The 3 digit prefix code for the command.
// [in] i        : Flag used to indicate how to send the response.
//                 i = 0: the line is a terminating line
//                 i = 1: the line is an intermediate line (with
//                        prefix code)
//                 i = 2: the line is an intermediate line (without
//                        prefix code)
//
// Return : On success true is returned.
//
bool TermService::sendResponse(const char *response, const char *code, int i /*=0*/)
{
	char line[MAX_CMDLINE_LENGTH];

	if ((m_flagssl == 0 && _sd == SOCK_INVALID))
		return false;

	if (i != 2)
	{
		if (strlen(code) != 3)  //the code must be 3 characters
			return false;
	}

	if (i == 1)
		sprintf(line,"%s-%s",code,response);
	else if (i == 2)
		sprintf(line,"    %s",response);
	else
		sprintf(line,"%s %s",code,response);


	size_t n = strlen(line);
	char *ptr;
	if ((ptr = strchr(line,'\r')) != NULL)
	{
		//if '\r' is found set it to '\0'
		*ptr = '\0';
		n = strlen(line);
	}

	if ((ptr = strchr(line,'\n')) != NULL)
	{
		//if '\n' is found set it to '\0'
		*ptr = '\0';
		n = strlen(line);
	}

	//check if the buffer is big enough
	if (n+3 > strlen(response)+7)
		n = 0;

	line[n++] = '\r';
	line[n++] = '\n';
	line[n] = '\0';

	size_t nbyteSent =0;
	if (m_flagssl == 0)
		nbyteSent = FtpSock::SendN(_sd, line,n);

#ifdef _OUTPUT_LOG_
	//delete the last \r\n from the string
	{
		char* pPtr = line;
		while(*pPtr)pPtr++;

		pPtr--;

		while(pPtr >line)
		{
			if (*pPtr == '\n' ||*pPtr == '\r')
				pPtr--;
			else
				break;
		}

		*(pPtr+1) = '\0';
	}

	MOLOG(Log::L_DEBUG, CLOGFMT(TermSr, "send:   (%s)"), line);
#endif

	return (nbyteSent == n);
}

//////////////////////////////////////////////////////////////////////
// Receives the command from the client
//
// [out] command   : Command that was sent by the client.
// [in] maxcmdsize : Max size of the command.
//
// Return : On success 1 is returned.  On failure 0 is returned.
//
bool TermService::recvCommand(char *command, int maxcmdsize)
{
	char line[MAX_CMDLINE_LENGTH];
	int retval;

	if ((m_flagssl == 0 && _sd == SOCK_INVALID))
		return false;

	if (m_flagssl == 0)
		retval = FtpSock::RecvLn(_sd, line, MAX_CMDLINE_LENGTH);
	if (retval <= 0)
	{	
		return false;
	}

	char *ptr;

	if ((ptr = strchr(line,'\r')) != NULL)    //if '\r' is found set it to '\0'
		*ptr = '\0';
	if ((ptr = strchr(line,'\n')) != NULL)    //if '\n' is found set it to '\0'
		*ptr = '\0';

	if (command != NULL)
	{
		strncpy(command,line,maxcmdsize-1);
		command[maxcmdsize-1] = '\0';
	}

#ifdef _OUTPUT_LOG_
	MOLOG(Log::L_DEBUG, CLOGFMT(TermSr, "receive:   (%s)"), command);
#endif

	return true;
}

bool TermService::recvCommand(std::string& command)
{
	char cmdbuf[MAX_CMDLINE_LENGTH];

	if(!recvCommand(cmdbuf, sizeof(cmdbuf)/sizeof(cmdbuf[0])))
		return false;
	command = cmdbuf;
	return true;
}
