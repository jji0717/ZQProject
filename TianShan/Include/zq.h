//
// Copyright (c) 1997-2004 by
// SeaChange International, Inc., Maynard, Mass.
// All Rights Reserved.  Unpublished rights  reserved  under  the  copyright
// laws of the United States.
//
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of SeaChange International Inc. Possession, use,
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
//
// title:       zq.h
//
// facility:    none (generic SeaChange definitions)
//
// abstract:
//      This header file contains very basic types and definitions for SeaChange
//      products.  NOTHING product specific should go in here.  Only very generic
//      stuff.  Very generic.  Product generic stuff should go in a product-wide
//      header file.
//
// Revision History:
// ------------------------
//
//   Rev       Date           Who               Description
//  ------ ---------------- -------  -----------------------------------
//                                    Initial Rev.
//                                    Added SCTP multiple instance ports
//                                    Added SRM management ports.
//                                    Added hammer management ports.
//                                    Added SmsSync Service management ports.
//                                    Included SeaVersion.h
//                                    created an port block for Client Engineering i.e DVA
//                                    added the SC_ASSERT definitions, and SeaError.h include
//                                    added SeaDAC MCS port, Mod-Lite & VODtiny ports.
//                                    added SmsES service management ports
//                                    added Music Choice Nexus managment ports
//                                    added additional ports for MC Nexus and Nexus Profile Mgr
//                                    added service port for MC Nexus
//                                    added mgmt ports for VASmon
//                                    added management ports for Device Manager Service (DM)
//                                    added service port for Music Choice Scheduler service
//		                              added mgt port for a second instance of MCAServer

//
#ifndef _SEACHANGE_H_
#define _SEACHANGE_H_

// *** This file should never include anything like windows.h or afx.h ***

#include "DebugCriticalSection.h"
#include "ZQVersion.h"
#include "ZQError.h"
#include "ZQCommon.h"

//-----------------------------------------------------------------------------
// 64-bit Data Quantities
//
#ifdef _MSC_VER
typedef unsigned _int64 QWORD, UINT64,  // Unsigned 64-bit integer
                      *PQWORD;          // pointer to QWORD

typedef _int64          LONGLONG,       // Signed 64-bit integer
                      *PLONGLONG;       // pointer to signed 64 bit integer
#else
typedef unsigned __int64 QWORD, UINT64, // Unsigned 64-bit integer
                       *PQWORD;         // pointer to QWORD

typedef __int64         LONGLONG,       // Signed 64-bit integer
                      *PLONGLONG;       // pointer to signed 64 bit integer
#endif

#define LODWORD( q )            ((DWORD)((QWORD)( q )))
#define HIDWORD( q )            ((DWORD)((QWORD)( q ) >> 32))
#define MAKEQWORD( lo, hi )     ((QWORD)( lo ) | ((QWORD)( hi ) << 32))

//-----------------------------------------------------------------------------
// A version structure for specifying the build version of a product, exe, dll,
// lib, etc.
//
typedef union _SEAVERSION
{
    WORD        wVersion;   // The whole version, can compare.
    struct 
    {
        BYTE    byteMinor;  // Minor version, 0-99
        BYTE    byteMajor;  // Major version, 0-255
    } VersionComponents;
} SEAVERSION, *PSEAVERSION;

//-----------------------------------------------------------------------------
//  TYPEINST
//
//  TYPEINST Is a unique identifier for the caller (CM or AS).  The identifier
//  is composed of an application type and and application instance number.
//
typedef union _TYPEINST
{
    QWORD       qwTypeInst;
    struct
    {
        DWORD   dwInst;
        DWORD   dwType;
    }s;
} TYPEINST, *PTYPEINST, *LPTYPEINST;

//-----------------------------------------------------------------------------
//  SAFE_DELETE (Idiom)
//
//  SAFE_DELETE is a convenience macro for conditionally deleting a pointer
//  based on whether or not it is NULL.  Also, it sets the pointer to NULL for
//  you after it's been deleted.
//
#define SAFE_DELETE(x) { if (x) { delete (x); (x) = NULL; } }

//-----------------------------------------------------------------------------
//  TCHARSOF(array of characters)
//
//  TCHARSOF counts the number of characters in a fixed size array of
//  TCHAR characters.  The types of strings that can be passed into this macro
//  are strings that have been defined with a fixed size either on the stack or
//  in global scope.
//
//  Example:
//
//  TCHAR szComputerName[256];
//  
//  TCHARSOF(szComputerName) returns 256 in either a UNICODE or ASCII
//  environment.  Note that in a UNICODE environment, simply using
//  sizeof(szComputerName) would return 512 (a common mistake.)
//-----------------------------------------------------------------------------
#define TCHARSOF(x) (sizeof(x)/sizeof(TCHAR))

//-----------------------------------------------------------------------------
// SC_ASSERT macros - use these instead of assert, _ASSERT, etc if you need
// the assert to show up as an exception in a Release Build of VC++
// assert()	-> SC_CASSERT
// ASSERT()	-> SC_ASSERT
// _ASSERT()	-> SC_UASSERT
// _ASSERTE()	-> SC_UASSERTE
// ToDo: add _LINE_ to the argument list of RaiseException so DrWatson dumps will show it in the registers??

#ifdef _DEBUG
#define SC_CASSERT(arg) assert(arg)
#define SC_ASSERT(arg) ASSERT(arg)
#define SC_UASSERT(arg) _ASSERT(arg)
#define SC_UASSERTE(arg) _ASSERTE(arg)
#else /* _DEBUG */
/*
#define SC_CASSERT(arg) ((arg) ? 1: RaiseException(APPSHELL_ASSERT_EXCEPTION, EXCEPTION_NONCONTINUABLE, 0, 0))
#define SC_ASSERT(arg) ((arg) ? 1: RaiseException(APPSHELL_ASSERT_EXCEPTION, EXCEPTION_NONCONTINUABLE, 0, 0))
#define SC_UASSERT(arg) ((arg) ? 1: RaiseException(APPSHELL_ASSERT_EXCEPTION, EXCEPTION_NONCONTINUABLE, 0, 0))
#define SC_UASSERTE(arg) ((arg) ? 1: RaiseException(APPSHELL_ASSERT_EXCEPTION, EXCEPTION_NONCONTINUABLE, 0, 0))
*/
#define SC_CASSERT(arg)     S_ASSERT(arg)
#define SC_ASSERT(arg)      S_ASSERT(arg)
#define SC_UASSERT(arg)     S_ASSERT(arg)
#define SC_UASSERTE(arg)    S_ASSERT(arg)
#endif /* _DEBUG */

//-----------------------------------------------------------------------------
// DEVELOPER BUILD MACROS
//
// These macros allow reminders to be placed in the code such that they show up
// in the output window of the build window.  They are useful for putting notes
// in the code to remind you to implement a function, fix an issue, etc.
//
// Example:
//
// int MyUnimplementedFunction()
// {
//     #pragma message(__TODO__ "Remember to implement this function one day")
//     return 0;
// }
//-----------------------------------------------------------------------------
#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __TODO__ __FILE__ "("__STR1__(__LINE__)") : *** TODO ***: "
#define __INFO__ __FILE__ "("__STR1__(__LINE__)") : *** INFO ***: "

//-----------------------------------------------------------------------------
// Default Management ports
// These are global, so it only makes sense to put 'em here
//
// These are cut and pasted from CDCI so I'll leave 'em as #defines
// all new ports are const WORD values.
// 10/16/95 CJH - use the correct TC port value as the default
#define DEF_TC_PORTNUM                  2000    // Default Task Controller TCP/IP Port Number
#define DEF_SDVL_PORTNUM                2889    // Default Scheduler DVL TCP/IP Port Number
#define DEF_PBI_PORTNUM                 2890    // Default Playback Interface TCP/IP Port Number
#define DEF_TB_PORTNUM                  41862   // Default T&B Service TCP/IP Port Number
#define DEF_SDVLC_PORTNUM               2890    // Default SDVL Client port number
#define DEF_SDVLS_PORTNUM               2891    // Default SDVL Server port number
#define DEF_SCSETUP_PORTNUM             2892    // Default SCSetup port number
#define DEF_NVODPLAYER_PORTNUM          2893    // Default NVOD Player port for Scheduler commands
#define DEF_MANTUTIL_PORTNUM            2894    // Default Manutil port number for machine builder
#define DEF_VODPLAYER_PORTNUM           2895    // Default VODPlayer port for IP Commands (Diamondbacks)
#define DEF_ARCHIVE_PORTNUM             2896    // Default Archive Service TCP/IP Port Number

// v3.0 08-Dec-95 PHB - Default management socket ports
#define DEF_DVLMGR_MGMT_PORT            4000
#define DEF_DVLSRV_MGMT_PORT            4001
#define DEF_DVLSRV_SHELL_MGMT_PORT      4002
#define DEF_SDVLC_MGMT_PORT             4003
#define DEF_SDVLC_SHELL_MGMT_PORT       4004
#define DEF_SDVLS_MGMT_PORT             4005
#define DEF_SDVLS_SHELL_MGMT_PORT       4006
#define DEF_DBM_MGMT_PORT               4007
#define DEF_DBM_SHELL_MGMT_PORT         4008
#define DEF_TB_MGMT_PORT                4009
#define DEF_TB_SHELL_MGMT_PORT          4010
#define DEF_SS32_MGMT_PORT              4011
// TC 4012 --> 4028 (16)
#define DEF_TC_MGMT_PORT                4012
// TC_SHELL 4028 --> 4044 (16)
#define DEF_TC_SHELL_MGMT_PORT          4028
#define DEF_VSMSERV_MGMT_PORT           4044
#define DEF_VSMSERV_SHELL_MGMT_PORT     4045
#define DEF_ALARMCOL_MGMT_PORT          4046
#define DEF_ALARMCOL_SHELL_MGMT_PORT    4047
#define DEF_ALARMANA_MGMT_PORT          4048
#define DEF_ALARMANA_SHELL_MGMT_PORT    4049
#define DEF_REP_MGMT_PORT               4050
#define DEF_REP_SHELL_MGMT_PORT         4051
#define DEF_SDVL_MGMT_PORT              4052
#define DEF_SDVL_SHELL_MGMT_PORT        4053
#define DEF_DVLRUL_MGMT_PORT            4054
#define DEF_DVLRUL_SHELL_MGMT_PORT      4055
#define DEF_PAGER_SHELL_MGMT_PORT       4056
#define DEF_PAGER_MGMT_PORT             4057
#define DEF_TM_SHELL_PORT               4058
#define DEF_TRAFFIC_MANAGER_PORT        4059
#define DEF_CNXSERVER_SHELL_PORT        4060
#define DEF_CNXSERVER_MGMT_PORT         4061
#define DEF_AMT_SHELL_PORT              4062
#define DEF_AMT_MGMT_PORT               4063
#define DEF_RVM_MGMT_PORT               4064
#define DEF_RVM_SHELL_MGMT_PORT         4065
#define DEF_RCM_MGMT_PORT               4066
#define DEF_RCM_SHELL_MGMT_PORT         4067
#define DEF_SVR_MGMT_PORT               4068
#define DEF_SVR_SHELL_MGMT_PORT         4069
#define DEF_NVP_MGMT_PORT               4070
#define DEF_NVP_SHELL_MGMT_PORT         4071
#define DEF_SDS_SHELL_PORT              4072
#define DEF_SDS_MGMT_PORT               4073
#define DEF_SSSrvr_MGMT_PORT            4074
#define DEF_SSSrvr_SHELL_PORT           4075
#define DEF_SSRcvr_MGMT_PORT            4076
#define DEF_SSRcvr_SHELL_PORT           4077
#define DEF_MPA_MGMT_PORT               4078
#define DEF_MPA_SHELL_PORT              4079
#define DEF_VP_MGMT_PORT                4080
#define DEF_VP_SHELL_MGMT_PORT          4081
#define DEF_ARCHIVE_MGMT_PORT           4082
#define DEF_ARCHIVE_SHELL_MGMT_PORT     4083

// ITV Management ports
// BASE = 5000
//
#define ITV_BASE_MGMT_PORT              5000
#define ITV_CM_MGMT_PORT                ITV_BASE_MGMT_PORT + 0      // 0x1388
#define ITV_DS_MGMT_PORT                ITV_BASE_MGMT_PORT + 1      // 0x1389
#define ITV_DS_SHELL_MGMT_PORT          ITV_BASE_MGMT_PORT + 2      // 0x138a
#define ITV_PS_MGMT_PORT                ITV_BASE_MGMT_PORT + 3      // 0x138b
#define ITV_PS_SHELL_MGMT_PORT          ITV_BASE_MGMT_PORT + 4      // 0x138c
#define ITV_SS_MGMT_PORT                ITV_BASE_MGMT_PORT + 5      // 0x138d
#define ITV_SS_SHELL_MGMT_PORT          ITV_BASE_MGMT_PORT + 6      // 0x138e
#define ITV_MOD_MGMT_PORT               ITV_BASE_MGMT_PORT + 7      // 0x138f
#define ITV_MOD_SHELL_MGMT_PORT         ITV_BASE_MGMT_PORT + 8      // 0x1390
#define ITV_SCTP_MGMT_PORT              ITV_BASE_MGMT_PORT + 9      // 0x1391
#define ITV_SCTP_SHELL_MGMT_PORT        ITV_BASE_MGMT_PORT + 10     // 0x1392
#define ITV_AM_MGMT_PORT                ITV_BASE_MGMT_PORT + 11     // 0x1393
#define ITV_AM_SHELL_MGMT_PORT          ITV_BASE_MGMT_PORT + 12     // 0x1394
#define ITV_DSAPITESTER_MGMT_PORT       ITV_BASE_MGMT_PORT + 13     // 0x1395
#define ITV_DCM_MGMT_PORT               ITV_BASE_MGMT_PORT + 14     // 0x1396
#define ITV_DCM_SHELL_MGMT_PORT         ITV_BASE_MGMT_PORT + 15     // 0x1397
#define ITV_AD_MGMT_PORT                ITV_BASE_MGMT_PORT + 16     // 0x1398
#define ITV_AD_SHELL_MGMT_PORT          ITV_BASE_MGMT_PORT + 17     // 0x1399
#define ITV_ADAPITESTER_MGMT_PORT       ITV_BASE_MGMT_PORT + 18     // 0x139a
#define ITV_PAGER_MGMT_PORT             ITV_BASE_MGMT_PORT + 19     // 0x139b
#define ITV_ALARMCOL_MGMT_PORT          ITV_BASE_MGMT_PORT + 20     // 0x139c
#define ITV_ALARMANA_MGMT_PORT          ITV_BASE_MGMT_PORT + 21     // 0x139d

#define ITV_CM2_MGMT_PORT               ITV_BASE_MGMT_PORT + 23     // 0x139f
#define ITV_SS2_MGMT_PORT               ITV_BASE_MGMT_PORT + 24     // 0x13a0
#define ITV_SS2_SHELL_MGMT_PORT         ITV_BASE_MGMT_PORT + 25     // 0x13a1
#define ITV_MOD2_MGMT_PORT              ITV_BASE_MGMT_PORT + 26     // 0x13a2
#define ITV_MOD2_SHELL_MGMT_PORT        ITV_BASE_MGMT_PORT + 27     // 0x13a3
#define ITV_SCTP2_MGMT_PORT             ITV_BASE_MGMT_PORT + 28     // 0x13a4
#define ITV_SCTP2_SHELL_MGMT_PORT       ITV_BASE_MGMT_PORT + 29     // 0x13a5
#define ITV_DCM2_MGMT_PORT              ITV_BASE_MGMT_PORT + 30     // 0x13a6
#define ITV_MODB_MGMT_PORT              ITV_BASE_MGMT_PORT + 31     // 0x13a7
#define ITV_MODB_SHELL_MGMT_PORT        ITV_BASE_MGMT_PORT + 32     // 0x13a8
#define ITV_SNMPEXT_MGMT_PORT           ITV_BASE_MGMT_PORT + 33     // 0x13a9

#define ITV_SYSMON_MGMT_PORT            ITV_BASE_MGMT_PORT + 34     // 0x13aa
#define ITV_SYSMON_SHELL_MGMT_PORT      ITV_BASE_MGMT_PORT + 35     // 0x13ab
#define ITV_MCRELAY_MGMT_PORT           ITV_BASE_MGMT_PORT + 36     // 0x13ac
#define ITV_MCRELAY_SHELL_MGMT_PORT     ITV_BASE_MGMT_PORT + 37     // 0x13ad
#define ITV_MCRELAY2_MGMT_PORT          ITV_BASE_MGMT_PORT + 38     // 0x13ae
#define ITV_MCRELAY2_SHELL_MGMT_PORT    ITV_BASE_MGMT_PORT + 39     // 0x13af

#define ITV_ALARMANA_SHELL_MGMT_PORT    ITV_BASE_MGMT_PORT + 40     // 0x13b0
#define ITV_ALARMCOL_SHELL_MGMT_PORT    ITV_BASE_MGMT_PORT + 41     // 0x13b1
#define ITV_PAGER_SHELL_MGMT_PORT       ITV_BASE_MGMT_PORT + 42     // 0x13b2

#define ITV_SRM_MGMT_PORT               ITV_BASE_MGMT_PORT + 43     // 0x13b3
#define ITV_SRM2_MGMT_PORT              ITV_BASE_MGMT_PORT + 44     // 0x13b4
#define ITV_SRM_SHELL_MGMT_PORT         ITV_BASE_MGMT_PORT + 45     // 0x13b5
#define ITV_SRM2_SHELL_MGMT_PORT        ITV_BASE_MGMT_PORT + 46     // 0x13b6

#define ITV_AMS_MGMT_PORT               ITV_BASE_MGMT_PORT + 47     // 0x13b7
#define ITV_AMS_SHELL_MGMT_PORT         ITV_BASE_MGMT_PORT + 48     // 0x13b8
#define ITV_ACCESSUP_MGMT_PORT          ITV_BASE_MGMT_PORT + 49     // 0x13b9
#define ITV_ACCESSUP_SHELL_MGMT_PORT    ITV_BASE_MGMT_PORT + 50     // 0x13ba

#define ITV_ISACONTENT_MGMT_PORT        ITV_BASE_MGMT_PORT + 51     // 0x13bb
#define ITV_ISACONTENT_SHELL_MGMT_PORT  ITV_BASE_MGMT_PORT + 52     // 0x13bc
#define ITV_ISASTREAM_MGMT_PORT         ITV_BASE_MGMT_PORT + 53     // 0x13bd
#define ITV_ISASTREAM_SHELL_MGMT_PORT   ITV_BASE_MGMT_PORT + 54     // 0x13be

#define ITV_ICS_MGMT_PORT               ITV_BASE_MGMT_PORT + 55     // 0x13bf
#define ITV_ICS_SHELL_MGMT_PORT         ITV_BASE_MGMT_PORT + 56     // 0x13c0
#define ITV_XML_MGMT_PORT               ITV_BASE_MGMT_PORT + 57     // 0x13c1
#define ITV_ICC_MGMT_PORT               ITV_BASE_MGMT_PORT + 58     // 0x13c2
#define ITV_ICC_SHELL_MGMT_PORT         ITV_BASE_MGMT_PORT + 59     // 0x13c3

#define ITV_HAMMER_MGMT_PORT            (ITV_BASE_MGMT_PORT + 60)   // 0x13c4
#define ITV_HAMMER_SHELL_MGMT_PORT      (ITV_BASE_MGMT_PORT + 61)   // 0x13c5

#define ITV_SMS_SYNC_MGMT_PORT          ITV_BASE_MGMT_PORT + 62     // 0x13c6
#define ITV_SMS_SYNC_SHELL_MGMT_PORT    ITV_BASE_MGMT_PORT + 63     // 0x13c7

#define ITV_ISADOWNLOAD_MGMT_PORT       ITV_BASE_MGMT_PORT + 64     // 0x13c8
#define ITV_ISADOWNLOAD_SHELL_MGMT_PORT ITV_BASE_MGMT_PORT + 65     // 0x13c9

#define ITV_ETF_MGMT_PORT               ITV_BASE_MGMT_PORT + 66     // 0x13ca
#define ITV_ETF_SHELL_MGMT_PORT         ITV_BASE_MGMT_PORT + 67     // 0x13cb

#define ITV_CM_SHELL_MGMT_PORT          (ITV_BASE_MGMT_PORT + 68)   // 0x13cc
#define ITV_CM2_SHELL_MGMT_PORT         (ITV_BASE_MGMT_PORT + 69)   // 0x13cd

#define ITV_DMN_MGMT_PORT               (ITV_BASE_MGMT_PORT + 70)   // 0x13ce
#define ITV_DMN_SHELL_MGMT_PORT         (ITV_BASE_MGMT_PORT + 71)   // 0x13cf

#define ITV_CSGDATA_MGMT_PORT           (ITV_BASE_MGMT_PORT + 72)   // 0x13d0

#define ITV_DVA_APP1_MGMT_PORT			(ITV_BASE_MGMT_PORT + 73)   // 0x13d1
#define ITV_DVA_APP1_SHELL_MGMT_PORT	(ITV_BASE_MGMT_PORT + 74)	// 0x13d2
#define ITV_DVA_APP12_MGMT_PORT			(ITV_BASE_MGMT_PORT + 75)   // 0x13d3
#define ITV_DVA_APP12_SHELL_MGMT_PORT	(ITV_BASE_MGMT_PORT + 76)	// 0x13d4

#define ITV_WMSOPS_MGMT_PORT            (ITV_BASE_MGMT_PORT + 77)   // 0x13d5

#define ITV_PFS_MGMT_PORT               (ITV_BASE_MGMT_PORT + 78)   // 0x13d6
#define ITV_PFS_SHELL_MGMT_PORT         (ITV_BASE_MGMT_PORT + 79)   // 0x13d7

#define ITV_GSRM_MGMT_PORT               (ITV_BASE_MGMT_PORT + 80)   // 0x13d8
#define ITV_GSRM_SHELL_MGMT_PORT         (ITV_BASE_MGMT_PORT + 81)   // 0x13d9
#define ITV_GSRM2_MGMT_PORT              (ITV_BASE_MGMT_PORT + 82)   // 0x13da
#define ITV_GSRM2_SHELL_MGMT_PORT        (ITV_BASE_MGMT_PORT + 83)   // 0x13db

#define ITV_ISAAS_MGMT_PORT             (ITV_BASE_MGMT_PORT + 84)   // 0x13dc
#define ITV_ISAAS_SHELL_MGMT_PORT       (ITV_BASE_MGMT_PORT + 85)   // 0x13dd
#define ITV_ISAAS2_MGMT_PORT            (ITV_BASE_MGMT_PORT + 86)   // 0x13de
#define ITV_ISAAS2_SHELL_MGMT_PORT      (ITV_BASE_MGMT_PORT + 87)   // 0x13df

#define ITV_ARS_MGMT_PORT				(ITV_BASE_MGMT_PORT + 88)   // 0x13e0
#define ITV_ARS_SHELL_MGMT_PORT			(ITV_BASE_MGMT_PORT + 89)   // 0x13e1

#define ITV_ARS2_MGMT_PORT				(ITV_BASE_MGMT_PORT + 90)   // 0x13e2
#define ITV_ARS2_SHELL_MGMT_PORT		(ITV_BASE_MGMT_PORT + 91)   // 0x13e3

#define ITV_VODOMETER_MGMT_PORT         (ITV_BASE_MGMT_PORT + 92)   // 0x13e4
#define ITV_VODOMETER_SHELL_MGMT_PORT   (ITV_BASE_MGMT_PORT + 93)   // 0x13e5

#define ITV_SMSES_MGMT_PORT             (ITV_BASE_MGMT_PORT + 94)   // 0x13e6
#define ITV_SMSES_SHELL_MGMT_PORT       (ITV_BASE_MGMT_PORT + 95)   // 0x13e7

#define ITV_CLIENT_SUP_MGMT_PORT        (ITV_BASE_MGMT_PORT + 96)   // 0x13e8
#define ITV_CLIENT_SUP_SHELL_MGMT_PORT  (ITV_BASE_MGMT_PORT + 97)   // 0x13e9

#define ITV_SRMP_MGMT_PORT              (ITV_BASE_MGMT_PORT + 98)   // 0x13ea
#define ITV_SRMP_SHELL_MGMT_PORT        (ITV_BASE_MGMT_PORT + 99)   // 0x13eb

#define ITV_ISAUPLOAD_MGMT_PORT         (ITV_BASE_MGMT_PORT + 100)  // 0x13ec
#define ITV_ISAUPLOAD_SHELL_MGMT_PORT   (ITV_BASE_MGMT_PORT + 101)  // 0x13ed

#define ITV_DM_MGMT_PORT                (ITV_BASE_MGMT_PORT + 102)  // 0x13ee
#define ITV_DM_SHELL_MGMT_PORT          (ITV_BASE_MGMT_PORT + 103)  // 0x13ef
#define ITV_DM2_MGMT_PORT               (ITV_BASE_MGMT_PORT + 104)  // 0x13f0
#define ITV_DM2_SHELL_MGMT_PORT         (ITV_BASE_MGMT_PORT + 105)  // 0x13f1

#define ITV_SRMP2_MGMT_PORT             (ITV_BASE_MGMT_PORT + 106)  // 0x13f2
#define ITV_SRMP2_SHELL_MGMT_PORT       (ITV_BASE_MGMT_PORT + 107)  // 0x13f3

#define ITV_RTSPGATEWAY_MGMT_PORT       (ITV_BASE_MGMT_PORT + 108)  // 0x13f4
#define ITV_RTSPGATEWAY_SHELL_MGMT_PORT (ITV_BASE_MGMT_PORT + 109)  // 0x13f5 
#define ITV_RTSPGATEWAY2_MGMT_PORT      (ITV_BASE_MGMT_PORT + 110)  // 0x13f6  
#define ITV_RTSPGATEWAY2_SHELL_MGMT_PORT (ITV_BASE_MGMT_PORT + 111) // 0x13f7

#define ITV_VODOMETER1_MGMT_PORT         (ITV_BASE_MGMT_PORT + 112)   // 0x13f8
#define ITV_VODOMETER1_SHELL_MGMT_PORT   (ITV_BASE_MGMT_PORT + 113)   // 0x13f9

#define ITV_VODOMETER2_MGMT_PORT         (ITV_BASE_MGMT_PORT + 114)   // 0x13fa
#define ITV_VODOMETER2_SHELL_MGMT_PORT   (ITV_BASE_MGMT_PORT + 115)   // 0x13fb

#define ITV_VODOMETER3_MGMT_PORT         (ITV_BASE_MGMT_PORT + 116)   // 0x13fc
#define ITV_VODOMETER3_SHELL_MGMT_PORT   (ITV_BASE_MGMT_PORT + 117)   // 0x13fd

#define ITV_VODOMETER4_MGMT_PORT         (ITV_BASE_MGMT_PORT + 118)   // 0x13fe
#define ITV_VODOMETER4_SHELL_MGMT_PORT   (ITV_BASE_MGMT_PORT + 119)   // 0x13ff

#define ITV_VODOMETER5_MGMT_PORT         (ITV_BASE_MGMT_PORT + 120)   // 0x1400
#define ITV_VODOMETER5_SHELL_MGMT_PORT   (ITV_BASE_MGMT_PORT + 121)   // 0x1401

#define ITV_VODOMETER6_MGMT_PORT         (ITV_BASE_MGMT_PORT + 122)   // 0x1402
#define ITV_VODOMETER6_SHELL_MGMT_PORT   (ITV_BASE_MGMT_PORT + 123)   // 0x1403

#define ITV_VODOMETER7_MGMT_PORT         (ITV_BASE_MGMT_PORT + 124)   // 0x1404
#define ITV_VODOMETER7_SHELL_MGMT_PORT   (ITV_BASE_MGMT_PORT + 125)   // 0x1405

#define ITV_VODOMETER8_MGMT_PORT         (ITV_BASE_MGMT_PORT + 126)   // 0x1406
#define ITV_VODOMETER8_SHELL_MGMT_PORT   (ITV_BASE_MGMT_PORT + 127)   // 0x1407

#define ITV_VODOMETER9_MGMT_PORT         (ITV_BASE_MGMT_PORT + 128)   // 0x1408
#define ITV_VODOMETER9_SHELL_MGMT_PORT   (ITV_BASE_MGMT_PORT + 129)   // 0x1409

#define ITV_VODOMETER10_MGMT_PORT         (ITV_BASE_MGMT_PORT + 130)   // 0x140a
#define ITV_VODOMETER10_SHELL_MGMT_PORT   (ITV_BASE_MGMT_PORT + 131)   // 0x140b

// update the port below EVERY time you add a mgmt port!
#define ITV_MGMT_PORT_NEXT              (ITV_BASE_MGMT_PORT + 132)  // 0x140c

// ^--add more mgmt ports here--^
#define ITV_MGMT_PORT_LAST              5999  // 0x176F do not exceed this value!!

// MCA Ports (New Hampshire stuff)
#define MCA_BASE_PORT                   6000
#define MCA_SERVER_PORT                 (MCA_BASE_PORT + 0)         // 0x1770
#define MCA_CLIENT_MCS_PORT             (MCA_BASE_PORT + 1)         // 0x1771
#define MCA_SERVER_MCS_PORT             (MCA_BASE_PORT + 2)         // 0x1772
#define MCA_SRVTOSRV_PORT               (MCA_BASE_PORT + 3)         // 0x1773
#define MCA_SERVER_MGMT_PORT            (MCA_BASE_PORT + 4)         // 0x1774
#define MCA_SHELL_MGMT_PORT             (MCA_BASE_PORT + 5)         // 0x1775
#define MCA_BURNIN_MGMT_PORT            (MCA_BASE_PORT + 6)         // 0x1776
#define MCA_TEST_MGMT_PORT              (MCA_BASE_PORT + 7)         // 0x1777
#define MCA_WMS_KEEPALIVE_PORT          (MCA_BASE_PORT + 8)         // 0x1778
#define MCA_SERVER01_MGMT_PORT          (MCA_BASE_PORT + 9)         // 0x1779 
#define MCA_SERVER02_MGMT_PORT          (MCA_BASE_PORT + 10)        // 0x177A 
#define MCA_SERVER03_MGMT_PORT          (MCA_BASE_PORT + 11)        // 0x177B 
#define MCA_SERVER04_MGMT_PORT          (MCA_BASE_PORT + 12)        // 0x177C 
#define MCA_SERVER05_MGMT_PORT          (MCA_BASE_PORT + 13)        // 0x177D 
#define MCA_SERVER06_MGMT_PORT          (MCA_BASE_PORT + 14)        // 0x177E 
#define MCA_SERVER07_MGMT_PORT          (MCA_BASE_PORT + 15)        // 0x177F 
#define MCA_SERVER08_MGMT_PORT          (MCA_BASE_PORT + 16)        // 0x1780 
#define MCA_SERVER09_MGMT_PORT          (MCA_BASE_PORT + 17)        // 0x1781 
#define MCA_SERVER10_MGMT_PORT          (MCA_BASE_PORT + 18)        // 0x1782 
// reserve previous 10 ports for additional instances of the server
// update the port below EVERY time you add a mgmt port!
#define MCA_PORT_NEXT                   (MCA_BASE_PORT + 20)
// ^--add more MCA ports here--^
#define MCA_LAST                        6499 // do not exceed this value!

// MCP Ports (New Hampshire stuff)
#define MCP_BASE_PORT                   6500
#define MCP_SERVER_MGMT_PORT            (MCP_BASE_PORT + 1)         // 0x1965
#define MCP_SHELL_MGMT_PORT             (MCP_BASE_PORT + 2)         // 0x1966
// ^--add more MCP ports here--^
#define MCP_LAST                        6699 // do not exceed this value!

// SFMS Ports (New Hampshire stuff)
#define	SFMS_BASE_PORT					6700
#define	SFMS_SERVER_MGMT_PORT			(SFMS_BASE_PORT + 1)		// 0x1a2d
#define	SFMS_SHELL_MGMT_PORT			(SFMS_BASE_PORT + 2)		// 0x1a2e
// ^--add more SFMS ports here--^
#define	SFMS_LAST						6799 // do not exceed this value!

// ITV DLL Server ports
// These are the ports on which the server listens for clients
#define ITV_BASE_PORT                   7000
#define ITV_DS_SERVER_PORT              ITV_BASE_PORT + 0           // 0x1b58
#define ITV_PS_SERVER_PORT              ITV_BASE_PORT + 1           // 0x1b59
#define ITV_SS_SERVER_PORT              ITV_BASE_PORT + 2           // 0x1b5a
#define ITV_AM_SERVER_PORT              ITV_BASE_PORT + 3           // 0x1b5b
#define ITV_SCTP_SERVER_PORT            ITV_BASE_PORT + 4           // 0x1b5c
#define ITV_MOD_SERVER_PORT             ITV_BASE_PORT + 5           // 0x1b5d

#define ITV_DS_SBS_PORT                 ITV_BASE_PORT + 6           // 0x1b5e
#define ITV_PS_SBS_PORT                 ITV_BASE_PORT + 7           // 0x1b5f
#define ITV_SS_SBS_PORT                 ITV_BASE_PORT + 8           // 0x1b60
#define ITV_AM_SBS_PORT                 ITV_BASE_PORT + 9           // 0x1b61
#define ITV_SCTP_SBS_PORT               ITV_BASE_PORT + 10          // 0x1b62
#define ITV_MOD_SBS_PORT                ITV_BASE_PORT + 11          // 0x1b63
#define ITV_CM_SBS_PORT                 ITV_BASE_PORT + 12          // 0x1b64
#define ITV_SET_TOP_PORT                ITV_BASE_PORT + 13          // 0x1b65
#define ITV_AD_SERVER_PORT              ITV_BASE_PORT + 14          // 0x1b66
#define ITV_LA_SBS_PORT                 ITV_BASE_PORT + 15          // 0x1b67
#define ITV_CM_GROUP_SBS_PORT           ITV_BASE_PORT + 16          // 0x1b68
#define ITV_CM_ADD_SBS_PORT             ITV_BASE_PORT + 17          // 0x1b69
#define ITV_CM_IGU_SBS_PORT             ITV_BASE_PORT + 18          // 0x1b6a

// Add System Monitor ports
#define ITV_SYSMON_SBS_PORT             ITV_BASE_PORT + 19          // 0x1b6b
#define ITV_SYSMON_PINGPORT             ITV_BASE_PORT + 20          // 0x1b6c

#define ITV_SCTP2_SERVER_PORT           ITV_BASE_PORT + 21          // 0x1b6d

// Multicast port where ITV services listening for multicast messages
#define ITV_MCS_PORT_BASE               ITV_BASE_PORT + 27          // 0x1b73

// ICS Server TCP/IP server port/listener
#define ITV_ICS_SERVER_PORT             ITV_BASE_PORT + 28          // 0x1b74
// ICS Server Multicast config request listener port
#define ITV_ICS_SBS_LSTPORT             ITV_BASE_PORT + 29          // 0x1b75
// ICS Server Multicast config responder port
#define ITV_ICS_SBS_SNDPORT             ITV_BASE_PORT + 30          // 0x1b76
// ICC Service Listener port
#define ITV_ICC_SERVER_PORT             ITV_BASE_PORT + 31          // 0x1b77

// SMS-Sync plug-in for CSG listener port
#define ITV_SMSCSG_PORT                 ITV_BASE_PORT + 32          // 0x1b78
#define ITV_STB_REPORT_PORT             ITV_BASE_PORT + 33          // 0x1b79

// APS multicast port
#define ARS_IA_SBS_PORT					ITV_BASE_PORT + 34          // 0x1b7a

#define ITV_VODOMETER_SBS_PORT          ITV_BASE_PORT + 35          // 0x1b7b
#define ITV_VODOMETER_PINGPORT          ITV_BASE_PORT + 36          // 0x1b7c

// SrmP(SRM Proxy)'s default port for HTTP request
#define ITV_SRMP_HTTP_PORT              ITV_BASE_PORT + 37          // 0x1b7d

// SyncApi port
#define ITV_SYN_PORT                    ITV_BASE_PORT + 38          // 0x1b7e

// update the port below EVERY time you add a port!
#define ITV_PORT_NEXT                   ITV_BASE_PORT + 39          // 0x1b7f

// Port numbers 7100 through 7120 not available, used for PS_SBS_PORTS
// in the Maynard ITV lab.

// ICM Ports beginning at 8000
#define ITV_CM_BASE_PORT                (ITV_BASE_PORT + 1000)
#define ITV_CM_STREAMOP_PORT_RANGE      10                          // range of port ids reserved for specific streamop protocol
#define ITV_CM_SCSOP_PORT_BASE          ITV_CM_BASE_PORT            // range 8000 - 8009 BASE + ProcessInstance for SeaChange Streamop client listener ports
#define ITV_CM_LSCUDP_PORT_BASE         (ITV_CM_BASE_PORT + ITV_CM_STREAMOP_PORT_RANGE)
                                                                    // range 8010 - 8019 BASE + ProcessInstance for LSC UDP Streamop client listener ports
#define ITV_CM_LSCTCP_PORT_BASE         (ITV_CM_BASE_PORT + ITV_CM_STREAMOP_PORT_RANGE * 2)
                                                                    // range 8020 - 8029 BASE + ProcessInstance for LSC TCP Streamop client listener ports
#define ITV_CM_LSCRTP_PORT_BASE         (ITV_CM_BASE_PORT + ITV_CM_STREAMOP_PORT_RANGE * 3)
                                                                    // range 8030 - 8039 BASE + ProcessInstance for LSC RTP Streamop client listener ports
// update the port below EVERY time you add a CM port!
#define ITV_CM_PORT_NEXT                (ITV_CM_BASE_PORT + ITV_CM_STREAMOP_PORT_RANGE * 4)

//------- TCP Ports (49201 to 49699 )to be used by Client Engineering i.e. DVA  --------//
#define ITV_CLIENT_BASE                                 49200 // Base for ITV Client services

// Mod-lite uses the UDP side of SCTP's TCP/IP port. Maintain the open slot for
// future use.
#define ITV_CLIENT_MOD_LITE_SVC_PORT		          (ITV_SCTP_SERVER_PORT)
//#define ITV_CLIENT_MOD_LITE_SVC_PORT		          (ITV_CLIENT_BASE + 1)
#define ITV_CLIENT_VODTINY_SVC_PORT                     (ITV_CLIENT_BASE + 2)
#define ITV_CLIENT_SEADAC_MCS_PORT                      (ITV_CLIENT_BASE + 3)
#define ITV_CLIENT_MUSICCHOICENEXUS_SVC_PORT            (ITV_CLIENT_BASE + 4)
#define ITV_CLIENT_LDC_NOTIFICATION_PORT                (ITV_CLIENT_BASE + 5)
#define ITV_CLIENT_SMSNTL_SVC_PORT                      (ITV_CLIENT_BASE + 6)
#define ITV_CLIENT_SGSPROXY_SVC_PORT                    (ITV_CLIENT_BASE + 7)
#define ITV_CLIENT_MODLITE_MCS_PORT                     (ITV_CLIENT_BASE + 8)
#define ITV_CLIENT_MUSICCHOICESCHEDULER_SVC_PORT        (ITV_CLIENT_BASE + 9)
#define ITV_CLIENT_SGSPROXY_SSL_SERVER_PORT             (ITV_CLIENT_BASE + 10)
#define ITV_CLIENT_SGSPROXY_SSL_CLIENT_PORT             (ITV_CLIENT_BASE + 11)

#define ITV_CLIENT_MGMT_BASE                            (ITV_CLIENT_BASE + 200)
#define ITV_CLIENT_LDC_MGMT_PORT                        (ITV_CLIENT_MGMT_BASE + 1)
#define ITV_CLIENT_LDC_SHELL_MGMT_PORT                  (ITV_CLIENT_MGMT_BASE + 2)
#define ITV_CLIENT_SEAC_BROWSR_MGMT_PORT                (ITV_CLIENT_MGMT_BASE + 3)
#define ITV_CLIENT_MOD_LITE_MGMT_PORT                   (ITV_CLIENT_MGMT_BASE + 4)
#define ITV_CLIENT_MOD_LITE_SHELL_MGMT_PORT             (ITV_CLIENT_MGMT_BASE + 5)
#define ITV_CLIENT_VODTINY_MGMT_PORT                    (ITV_CLIENT_MGMT_BASE + 6)
#define ITV_CLIENT_VODTINY_SHELL_MGMT_PORT              (ITV_CLIENT_MGMT_BASE + 7)
#define ITV_CLIENT_MUSICCHOICENEXUS_MGMT_PORT           (ITV_CLIENT_MGMT_BASE + 8)
#define ITV_CLIENT_MUSICCHOICENEXUS_SHELL_MGMT_PORT     (ITV_CLIENT_MGMT_BASE + 9)
#define ITV_CLIENT_MUSICCHOICENEXUS2_MGMT_PORT          (ITV_CLIENT_MGMT_BASE + 10)
#define ITV_CLIENT_MUSICCHOICENEXUS2_SHELL_MGMT_PORT    (ITV_CLIENT_MGMT_BASE + 11)
#define ITV_CLIENT_MCNEXUSPROFILEMGR_MGMT_PORT          (ITV_CLIENT_MGMT_BASE + 12)
#define ITV_CLIENT_MCNEXUSPROFILEMGR_SHELL_MGMT_PORT    (ITV_CLIENT_MGMT_BASE + 13)
#define ITV_CLIENT_MCNEXUSPROFILEMGR2_MGMT_PORT         (ITV_CLIENT_MGMT_BASE + 14)
#define ITV_CLIENT_MCNEXUSPROFILEMGR2_SHELL_MGMT_PORT   (ITV_CLIENT_MGMT_BASE + 15)
#define ITV_CLIENT_MCNEXUSVASMON_MGMT_PORT              (ITV_CLIENT_MGMT_BASE + 16)
#define ITV_CLIENT_MCNEXUSVASMON_SHELL_MGMT_PORT        (ITV_CLIENT_MGMT_BASE + 17)
#define ITV_CLIENT_SMSNTL_MGMT_PORT                     (ITV_CLIENT_MGMT_BASE + 18)
#define ITV_CLIENT_SGS_MGMT_PORT                        (ITV_CLIENT_MGMT_BASE + 19)
#define ITV_CLIENT_SGS_SHELL_MGMT_PORT                  (ITV_CLIENT_MGMT_BASE + 20)

// ^--Add ports here as necessary--^


// ITV Multicast addresses
//
// ITV Services by default will use the ITV_MCS_ADDR defined multicast address.
// for all MCS messages.  Services should be written so they first check the
// registry for their MCSAddress key.  If that key does not exist, use the ITV_MCS_ADDR
// definition.  BFS_MCS_ADDR will be the defined multicast address for reaching the
// BFS and thus the set tops.
//#define ITV_MCS_ADDR "225.25.25.25"
#define BFS_MCS_ADDR                    "225.25.25.35"
#define IPADDR_SIZE                     16 //  Buffer size needed to hold ip address as a string

//#define SEG_MULTICAST //test the defaults.
#ifdef SEG_MULTICAST
//Multicast grouping test Addresses
#define ITV_MCS_LA_ADDR                 "239.0.1.25" // used by sctp
#define ITV_MCS_SSAPI_ADDR              "239.0.2.25" // used bu SSAPI

#define ITV_MCS_CM_ADD_ADDR             "239.0.3.25"  // CM advertisement 
#define ITV_MCS_CM_GROUP_ADDR           "239.0.4.25"  // CM peer group
#define ITV_MCS_CM_IGU_ADDR             "239.0.5.25"  // CM inter group usage exchange

#define ITV_MCS_SYSMON_PING_ADDR        "239.0.8.25"
#define ITV_MCS_SYSMON_SBS_ADDR         "239.0.8.25"

#define ITV_MCS_CONFIG_SBS_ADDR         "239.0.9.25"

#else

#define ITV_MCS_ADDR                    "225.25.25.25"
#define ITV_MCS_LA_ADDR                 ITV_MCS_ADDR
#define ITV_MCS_SSAPI_ADDR              ITV_MCS_ADDR

#define ITV_MCS_CM_ADD_ADDR             ITV_MCS_ADDR
#define ITV_MCS_CM_GROUP_ADDR           ITV_MCS_ADDR
#define ITV_MCS_CM_IGU_ADDR             ITV_MCS_ADDR

#define ITV_MCS_SYSMON_PING_ADDR        ITV_MCS_ADDR
#define ITV_MCS_SYSMON_SBS_ADDR         ITV_MCS_ADDR

#define ITV_MCS_CONFIG_SBS_ADDR         ITV_MCS_ADDR

#endif


#endif // _SEACHANGE_H_
