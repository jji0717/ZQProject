/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1998 by
// SeaChange International Inc., Maynard, MA
// All Rights Reserved.  Unpublished rights  reserved  under  the  copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of SeaChange  International  Inc.   Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from SeaChange International Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without  notice and
// should not be construed as a commitment by SeaChange International Inc.
// 
// SeaChange  assumes  no  responsibility  for the use or reliability of its
// software on equipment which is not supplied by SeaChange.
// 
// RESTRICTED RIGHTS  LEGEND  Use,  duplication,  or  disclosure by the U.S.
// Government is subject  to  restrictions  as  set  forth  in  Subparagraph
// (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//
// TITLE:
//      MOD.h
//
// VERSION:
//      V1.0
//
// FACILITY
//      ITV -- Movies on Demand Application
//
// ABSTRACT:
//      This module contains the Movies on Demand Common Definitions
//      In other words, the items shared between the MOD Server and Client
//
// REVISION HISTORY: (Most recent at top)
//
//   Revision      Date     Modified By     Reviewed By
//  ----------  ---------   -----------     -----------
//  V1.0        22-Jul-1998     SMD
//      Initial version
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _MOD_H_
#define _MOD_H_

#ifndef _MOD_STDAFX_H_
#include "stdafx.h"  // standard MFC includes
#endif

#ifndef _ITV_H_
#include "Itv.h"
#endif

#include "IssApi.h"
//#include "dsmccuser.h"

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
