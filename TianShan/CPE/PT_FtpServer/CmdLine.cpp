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
// Ident : $Id: CmdLine.cpp,v 1.5 2004/07/23 09:04:36 jshen Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : handle the ftp command 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/PT_FtpServer/CmdLine.cpp $
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
// 1     06-08-30 12:32 Jie.zhang
// 
// 1     05-09-06 13:57 Jie.zhang
// 
// 2     04-12-08 15:50 Jie.zhang
// 
// 2     04-11-19 17:05 Jie.zhang
// Revision 1.5  2004/07/23 09:04:36  jshen
// For QA test
//
// Revision 1.4  2004/07/06 07:20:43  jshen
// add skeleton for SeaChange AppShell Service
//
// Revision 1.3  2004/07/05 02:18:54  jshen
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CmdLine.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Constructor for the CmdLine class.
//
// [in] cmdlinesize : Size of the buffer to use for the command line.
//
CmdLine::CmdLine()
{
}

CmdLine::~CmdLine()
{
}

//////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Load the command line buffer.
//
// [in] cmdline : String to load the command line with.
//
// Return : VOID
//
void CmdLine::set(const char *cmdline)
{
	if (cmdline != NULL)
		_cmdline = cmdline;
}

//////////////////////////////////////////////////////////////////////
// Get the current command line buffer.
//
// [out] outbuff    : String to load the command line into.
// [in] maxbuffsize : Max size of the "outbuff" string.
//
// Return : VOID
//
void CmdLine::get(char *outbuff, int maxbuffsize)
{

	if (_cmdline.empty() || outbuff == NULL)
		return;

	strncpy(outbuff, _cmdline.c_str(), maxbuffsize-1);
	outbuff[maxbuffsize-1] = '\0';
}

//////////////////////////////////////////////////////////////////////
// Set the size of the command line buffer.
//
// [in] cmdlinesize : Size the command line buffer should be set to.
//
// Return : VOID
//
// NOTE: Setting the cmd line size may change the m_cmdline pointer.
//
void CmdLine::resize(int cmdlinesize)
{
	if (cmdlinesize >=0 && ((int) _cmdline.length()) > cmdlinesize)
		_cmdline = _cmdline.substr(0, cmdlinesize);
}

//////////////////////////////////////////////////////////////////////
// Get the size of the current command line buffer.
//
// Return : Size of the current command line.
//
int CmdLine::size()
{
	return (int) _cmdline.length();
}

//////////////////////////////////////////////////////////////////////
// Parses the command line that is currently stored in the internal
// command line buffer.  This must be called to process a new command
// line.
//
// [out] argc      : Number of command line arguments.
// [in] delimiter  : Delimiter to use when parsing the command line.
// [in] usepadding : If == 0, multiple delimiters count as 1 delimiter
//
// Return : On successs, an array of pointers for the command line
//          arguments is returned.  On failure, NULL is returned.
//
char **CmdLine::parse(int *argc, char delimiter /*=' '*/, int usepadding /*=0*/)
{
	char *ptr, delimiter2;
	int numargs, delimiterflag;

	if (argc == NULL)
		return NULL;

	*argc = 0;  //initialize the number of arguments to 0

	//check if the current command line is valid
	if (_cmdline.empty())
		return NULL;


	strcpy(_cmdbuffer,_cmdline.c_str());

	//if the delimiter is a ' ' also make '\t' a delimiter
	delimiter2 = (delimiter == ' ') ? '\t' : '\0';

	//get the number of parameters in the command line
	numargs = 0;
	ptr = _cmdbuffer;
	delimiterflag = 1;
	if (usepadding != 0) numargs++; //the first argument
	while (*ptr != '\0')
	{
		if (*ptr == delimiter || *ptr == delimiter2)
		{
			if (usepadding != 0) numargs++; //a new argument was encountered            
			delimiterflag = 1;  //a "delimiter" was encountered
		}
		else if (*ptr != delimiter && *ptr != delimiter2 && delimiterflag != 0)
		{
			if (usepadding == 0) numargs++; //a new argument was encountered            
			delimiterflag = 0;  //current character is not a "delimiter"
		}
		ptr++;
	}

	if (numargs == 0)
	{
		return NULL;
	}


	//set all the delimiters to '\0'
	numargs = 0;
	ptr = _cmdbuffer;
	delimiterflag = 1;
	if (usepadding != 0)
		_argptr[numargs++] = ptr; //add the argument to the list
	while (*ptr != '\0')
	{
		if (*ptr == delimiter || *ptr == delimiter2)
		{
			*ptr = '\0';    //set the "delimiter" to '\0'
			if (usepadding != 0) _argptr[numargs++] = ptr + 1; //add the argument to the list
			delimiterflag = 1;  //a "delimiter" was encountered
		}
		else if (*ptr != delimiter && *ptr != delimiter2 && delimiterflag != 0)
		{
			if (usepadding == 0) _argptr[numargs++] = ptr;     //add the argument to the list
			delimiterflag = 0;  //current character is not a "delimiter"
		}
		ptr++;
	}
	_argptr[numargs] = NULL;   //terminate the list with a NULL entry

	*argc = numargs;
	return(_argptr);
}

//////////////////////////////////////////////////////////////////////
// Returns a buffer containing the combined arguments in argv starting
// with argv[startarg].  Any arguments starting with "optionchar" will
// be ignored.  The buffer returned has one extra byte in case a
// trailing slash (or other char) needs to be added.
//
// [in] argc       : Number of command line arguments.
// [in] argv       : Array of command line arguments.
// [in] optionchar : Character used to indicate an option on the
//                   command line.  If == 0, none is used.
// [in] startarg   : Argument to start will (first arg in output)
//
// Return : On successs, a sting containing the combined arguments is
//          returned.  On failure, NULL is returned.
//
bool CmdLine::combineArgs(std::string& outline, int argc, const char **argv, char optionchar /*=0*/, int startarg /*=1*/)
{
	outline ="";
	int flagspace = 0;

	//combine all the arguments
	for (int i = startarg; i < argc; i++)
	{
		if (*argv[i] != optionchar)
		{
			if (flagspace != 0)
				outline += " ";
			outline += argv[i];
			flagspace = 1;
		}
	}

	return true;
}

bool CmdLine::combineArgs(char* outline, int argc, const char **argv, char optionchar /*=0*/, int startarg /*=1*/)
{
	int flagspace = 0;

	int nLen = 0;

	//combine all the arguments
	for (int i = startarg; i < argc; i++)
	{
		if (*argv[i] != optionchar)
		{
			if (flagspace != 0)
				outline[nLen++] = ' ';
						
			nLen += sprintf(outline+nLen, "%s", argv[i]);			
			flagspace = 1;
		}
	}

	outline[nLen++] = '\0';

	return true;
}
