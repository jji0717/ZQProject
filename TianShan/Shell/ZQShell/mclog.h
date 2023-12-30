/*
** Copyright (c) 1995 by
** ZQ Technology Inc., West Concord, Mass.
** All Rights Reserved.  Unpublished rights  reserved  under  the  copyright
** laws of the United States.
**
** The software contained  on  this media is proprietary to and embodies the
** confidential technology of zq  Technology  Inc.   Possession, use,
** duplication or dissemination of the software and media is authorized only
** pursuant to a valid written license from zq Technology Inc.
**
** This software is furnished under a  license  and  may  be used and copied
** only in accordance with the terms of  such license and with the inclusion
** of the above copyright notice.  This software or any other copies thereof
** may not be provided or otherwise made available to  any other person.  No
** title to and ownership of the software is hereby transferred.7
**
** The information in this software is subject to change without  notice and
** should not be construed as a commitment by zq Technology Inc.
**
** zq  assumes  no  responsibility  for the use or reliability of its
** software on equipment which is not supplied by zq.
**
** RESTRICTED RIGHTS  LEGEND  Use,  duplication,  or  disclosure by the U.S.
** Government is subject  to  restrictions  as  set  forth  in  Subparagraph
** (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
*/
/*
**
** title:		mclog.h  -- multiple circular log file writer
**
** version:     V1.1-0
**
** facility:    Digital Ad Insertion
**
** abstract:
**          This file contains definitions needed to update multiple
**			circular log files.
**			MCLOG is extended from CLOG.
**
**
** Revision History: 
** ------------------
**
**   Rev      Date       Who               Description
**  ------  ---------  --------  -----------------------------------
**    Created from chrish's tc clog code
** 	  Created from the favorite DavidR clog code
**/

#define MAXLINE 512 // Max log line length (sans date/time)


typedef struct MCB_s {
	HANDLE      hClogMutex;
	FILE        *fLogfp;
	int         iLogTraceLevel;
	int         iLogFileSize; 
} MCB_t;
	
//extern 
MCB_t* MClogInit(TCHAR *logfile, int iTraceLevel, int iFileSize);
//extern 
BOOL MClogTerm(MCB_t *hHandle );
//extern 
BOOL MClogSetTraceLevel(MCB_t *hHandle, int iTraceLevel);
//extern 
int  MClog(MCB_t *hHandle, int iTraceLevel, TCHAR* szMessage, ...);
