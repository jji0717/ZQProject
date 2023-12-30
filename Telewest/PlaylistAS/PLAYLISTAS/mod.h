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
// Name  : mod.h
// Author : Ken
// Date  : 2005-1-19
// Desc  : This file is from MOD application source, and be modified to cooperate 
//         STVPlayback project
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/PlaylistAS/PLAYLISTAS/mod.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 1     05-02-16 11:56 Bernie.zhao
// 
// 2     05-01-19 16:57 Bernie.zhao
// ===========================================================================

#ifndef _MOD_H_
#define _MOD_H_

#ifndef _ITV_H_
#include "Itv.h"
#endif

#include "IssApi.h"
#include "dsmccuser.h"

// Stream Terminate Reason Codes
//      Must be negative numbers that never use bit 30.
#define MOD_TERM_NOT_AUTHORIZED             0xbfffffff
#define MOD_TERM_MOVIE_DONE                 0xbffffffe
#define MOD_TERM_DURATION_TIMEOUT           0xbffffffd
#define MOD_TERM_SUSPENDED                  0xbffffffc
#define MOD_TERM_CLIENT_CANCELED            0xbffffffb
#define MOD_TERM_SYSTEM_CANCELED            0xbffffffa
#define MOD_TERM_INTERNAL_SERVICE_ERROR     0xbffffff9
#define MOD_TERM_BAD_ASSET                  0xbffffff8
#define MOD_TERM_BLOCKED_ASSET              0xbffffff7
#define MOD_TERM_PACKAGE_INVALID_ASSET      0xbffffff6
#define MOD_TERM_PACKAGE_UNAVAILABLE        0xbffffff5
#define MOD_TERM_VIEW_LIMIT_REACHED         0xbffffff4


// Obsolete values; for backwards compatibility
#define MOD_TERM_BAD_CREDIT                 MOD_TERM_NOT_AUTHORIZED

// Application User Data definition
//      The data passed on the NewStream callback
//      interface is a TLV (i.e. type-length-value)

#define MOD_APP_DATA_TYPE       0x1
#define MOD_APP_DATA_LENGTH     0x5

//      The value portion of the TLV contains the following information
//      The length is set to the size of this structure

#pragma pack( push, mod_app_data, 1 )

typedef struct _MODAPPDATA_V_1
        {
         DWORD  dwBillingId;
         time_t tPurchaseTime;
        } MODAPPDATA_V_1;

typedef struct _MODAPPDATA_V_2
        {
         BYTE   byteVersion;
         BYTE   byteDescriptorCnt;
         // descriptors follow in TLV foramt
        } MODAPPDATA_V_2;

typedef struct _MODAPPDATA
        {
         BYTE   byteVersion;    // See acceptable values below
         union {
            MODAPPDATA_V_1 v1;
            MODAPPDATA_V_2 v2;
         };
        } MODAPPDATA;

#pragma pack( pop, mod_app_data )

#define MOD_APP_DATA_VERSION_1          0x01
#define MOD_APP_DATA_VERSION_2          0x80

#endif // _MOD_H_
