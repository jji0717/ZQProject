      // ===========================================================================
// Copyright (c) 1997, 1998 by
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
// Ident : $Id: STVMainHeaders.h$
// Branch: $Name:  $
// Author: Bernie(Tianbin) Zhao
// Desc  : some common STV macros
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ScheduledTV_new/STVMainHeaders.h $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:34 Admin
// Created.
// 
// 6     10/18/04 2:34p Jie.zhang
// 
// 
// 5     10/13/04 5:22p Jie.zhang
// 
// 4     04-10-13 16:05 Bernie.zhao
// 
// 3     10/12/04 6:48p Jie.zhang
// 
// 2     04-10-11 19:41 Bernie.zhao
// ===========================================================================
#pragma once

#define LIST_GLOBAL				0x10
#define	LISTTYPE_UNKNOWN		0x0
#define LISTTYPE_NORMAL			0x1		// normal STV list	
#define LISTTYPE_BARKER 		0x2		// barker list
#define LISTTYPE_FILLER			0x3		// filler list
#define LISTTYPE_GFILLER		LISTTYPE_FILLER | LIST_GLOBAL	// global filler list
#define LISTTYPE_GBARKER		LISTTYPE_BARKER | LIST_GLOBAL	// global barker list
#define BASICTYPE(__x)	(__x & 0xF)		// get basic type, ignoring global attribute
#define ISGLOBALTYPE(__x)	(__x & LIST_GLOBAL)		// check if it is global attribute

///@name ErrorCode
// Error code for ITV system
//@{
#define STVSUCCESS		0
#define	STVERROR		100

// bernie
#define	STVINVALIDPL	(STVERROR+1)

#define STVERRFORMAT	(STVERROR+2)

#define	STVPLDBFAIL		(STVERROR+3)

#define STVIDSERROR		(STVERROR+4)

#define STVNOEFFECTSET	(STVERROR+5)

#define STVRTSPFAIL		(STVERROR+6)

#define STVINVALIDCHNL	(STVERROR+7)

#define STVNOSCHNO		(STVERROR+8)

#define STVBADFEEDBACK	(STVERROR+9)

#define STVBUFFTOOSMALL (STVERROR+10)

#define STVCHNLBUSY		(STVERROR+11)

// jie zhang
#define STVXMLPARSEERR		(STVERROR + 100)



struct ERROR_DESC{
	int	nCode;
	const char* sString;
};

// fill error code and desc for the error
const ERROR_DESC g_errInfo[] = {
	{STVERROR,			"General error"},
	{STVINVALIDPL,		"Can not create new playlist"},
	{STVERRFORMAT,		"XML format error"},
	{STVPLDBFAIL,		"Playlist database mirror failed"},
	{STVIDSERROR,		"Some sort of error on IDS"},
	{STVNOEFFECTSET,	"No effective fill/barker set"},
	{STVRTSPFAIL,		"Fail to start Rtsp Stream"},
	{STVINVALIDCHNL,	"Specified channel not found"},
	{STVNOSCHNO,		"No list with specified schedule NO"},
	{STVBADFEEDBACK,	"The feedback from ISS is invalid"},
	{STVBUFFTOOSMALL,	"Buffer to small"},
	{STVCHNLBUSY,		"Channel is busy"},
	{STVXMLPARSEERR,	"Parse xml error"}
};


inline const char* GetSTVErrorDesc(int nCode)
{
	for(int i=0;i<sizeof(g_errInfo)/sizeof(ERROR_DESC);i++)
		if(g_errInfo[i].nCode == nCode)
			return g_errInfo[i].sString;

	return 0;
}
//@}




const int GEN_ID_DOWNBOUND		= 1;		///< generated id,  low bound
const int GEN_ID_UPBOUND		= 100000;		//



const int MC_SCHEDULE_LIST				= 1001;
const int MC_PLAY_SPECIFIC_ASSET		= 1011;
const int MC_SKIPTO_SPECIFIC_ASSET		= 1012;
const int MC_HOLD_NEXT_ASSET			= 1013;
const int MC_ENQUIRY_SCHDULE			= 1014;
const int MC_PLAY_FILLER_IMMEDIATE		= 1015;
const int MC_STATUS_FEEDBACK			= 1021;
const int MC_ENQUIRY_STATUS				= 1022;
const int MC_PLAYLIST_BARKER			= 1031;
const int MC_FILLERLIST					= 1041;
const int MC_QUERY_FILLER				= 1042;
const int MC_CONFIGURATION				= 1051;
const int MC_QUERY_CONFIGURATION		= 1052;
const int MC_STARTUP_CHANNEL			= 1058;
const int MC_SHUTDOWN_CHANNEL			= 1059;
const int MC_RESPONSE					= 4002;
const int MC_HANDSHAKE					= 1000;


const int TIME_MINI = 500;		///< element time block, used for sleep(), ispending()


const int TIME_RF_WAIT = 3000;	///< connnect error errResourceFailure waiting time to retry


const int TIME_CB_WAIT = 3000;  ///< connect error errConnectBusy waiting time to retry


const int TIME_CR_WAIT = 4000;	///< connect error errConnectRefused waiting time to retry


const int TIME_DEFAULT_WAIT = 3000;	///< connect error default error waiting time to retry


const int MAX_SEND_XML_BUF = 8192;	///<  max send xml package length


const int MAX_XML_BUF = 500*1024;	///<  max xml package length


const char MSG_END_FLAG = 127;		///< translate protocol, this is the end flag of a package







//@{
#define	MAIN_IDLE		0
#define MAIN_RUNNING	1
//@}


#define MAX_URL_LEN		1024
