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
// Name  : ZqMessages.mc
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-2-18
// Desc  : This is the message compiler source file for all ZQ facility
//         messages. (Currently only used by Server Engineer 1)
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/ZQMessages/ZqMessages.h $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 1     05-02-21 11:44 Bernie.zhao
// ===========================================================================
//*******************************************************************************
//*
//*    Header Section
//*
//*  There will be no typedefs for ZQ messages.  The severity names will be the
//*    the default ones: SeverityNames=(Success= 0x0,Informational= 0x1,Warning= 0x2,
//*  Error= 0x3). Language names will likewise default to:
//*    LanguageNames=(English= 1:MSG00001)
//*******************************************************************************
//*******************************************************************************
//*
//*     Generic ZQ messages
//*******************************************************************************
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: ZQ_GENERIC_SUCCESS
//
// MessageText:
//
//  ZQ Service General Success:%1.
//
#define ZQ_GENERIC_SUCCESS               0x00001000L

//
// MessageId: ZQ_GENERIC_INFO
//
// MessageText:
//
//  ZQ Service General Information:%1.
//
#define ZQ_GENERIC_INFO                  0x40001001L

//
// MessageId: ZQ_GENERIC_WARNING
//
// MessageText:
//
//  ZQ Service General Warning:%1.
//
#define ZQ_GENERIC_WARNING               0x80001002L

//
// MessageId: ZQ_GENERIC_ERROR
//
// MessageText:
//
//  ZQ Service General Error:%1.
//
#define ZQ_GENERIC_ERROR                 0xC0001003L

