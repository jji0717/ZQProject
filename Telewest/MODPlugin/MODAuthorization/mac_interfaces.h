/*
** Copyright (c) 1998 by
** SeaChange Technology Inc., Maynard, Mass.
** All Rights Reserved.  Unpublished rights  reserved  under  the  copyright
** laws of the United States.
** 
** The software contained  on  this media is proprietary to and embodies the
** confidential technology of SeaChange  Technology  Inc.   Possession, use,
** duplication or dissemination of the software and media is authorized only
** pursuant to a valid written license from SeaChange Technology Inc.
** 
** This software is furnished under a  license  and  may  be used and copied
** only in accordance with the terms of  such license and with the inclusion
** of the above copyright notice.  This software or any other copies thereof
** may not be provided or otherwise made available to  any other person.  No
** title to and ownership of the software is hereby transferred.
** 
** The information in this software is subject to change without  notice and
** should not be construed as a commitment by SeaChange Technology Inc.
** 
** SeaChange  assumes  no  responsibility  for the use or reliability of its
** software on equipment which is not supplied by SeaChange.
** 
** RESTRICTED RIGHTS  LEGEND  Use,  duplication,  or  disclosure by the U.S.
** Government is subject  to  restrictions  as  set  forth  in  Subparagraph
** (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
*/

/*
** title:       mac_interfaces.h
**
** version:     V1.0
**
** facility:    Interactive Television - Movies-on-Demand subsystem
**              MOD Authorization Check Plug-In DLL
**
** abstract:    This module is the include file containing definitions 
**              of the macdll.cpp
**
** Revision History:
**
** History:
**   Revision      Date     Modified By     Reviewed By
**  ----------  ----------  -----------     -----------
**   V1.0       11-21-2000     PCS
**      Initial version
**/

#ifndef _MAC_INTERFACES_H_
#define _MAC_INTERFACES_H_

#include "ManPkg.h"

// This header file defines the interface between MOD and the DLLs that it loads

// Version Number for Structures
#define MAC_MAJOR_VERSION      2
#define MAC_MINOR_VERSION      0

#define MAC_VERSION_2_0        0x0200

// Define the structures that are passed

// Structure that contains information about DS or AD data that the DLL can request
typedef struct _MACINFO {
    BOOL   bRequested;      // Does the DLL want this data when MAC_PROCESS_RTN or MAC_REPORT_RTN is invoked?
    WCHAR  wszName[255];    // Name
    WORD   wTag;            // See request tags below
} MACINFO;

// This structure defines the handles that are passed from MOD to the DLL so
// the DLL can identify itself on the callbacks

typedef struct _MACHANDLES {
    ITVVERSION  Version;        // Version of structure
    HANDLE      hMgmt;          // Management handle for DLL to provide mgmt interface
    WORD        wID;            // Instance Id of MOD
    WORD        wDsInfo;        // The number of DS values in ppDsInfo
    MACINFO     **ppDsInfo;     // DS data requested when invoking MAC_PROCESS_RTN
    WORD        wAdInfo;        // The number of AD values in ppDsInfo
    MACINFO     **ppAdInfo;     // AD data requested when invoking MAC_REPORT_RTN
} MACHANDLES;

// This structure defines the callbacks that a DLL can make into MOD.
// The unload callback is provided so a DLL can signal MOD to unload itself.
// A DLL might unload itself when it becomes inoperative.
 
typedef BOOL      (__cdecl *MAC_UNLOAD_RTN) (void);
typedef BYTE *    (__cdecl *MAC_ALLOC_RTN)  (DWORD dwLength);
typedef ITVSTATUS (__cdecl *MAC_UPDATE_RTN) (STREAMID Sid, BYTE *pRequest, BOOL *pbResponse);

typedef struct _MACCALLBACKS {
    ITVVERSION      Version;        // Version of structure
    MAC_UNLOAD_RTN  pfnUnloadRtn;   // Causes DLL to be unloaded
    MAC_ALLOC_RTN   pfnAllocRtn;    // Used by DLL to allocate TLV used by MAC_UPDATE_RTN
    MAC_UPDATE_RTN  pfnUpdateRtn;   // Used by DLL to update Session data
} MACCALLBACKS;

// Define request tags
//  Tags: session setup
#define MAC_TAG_MACADDRESS  1
#define MAC_TAG_BILLINGID   2
#define MAC_TAG_HOMEID      3
#define MAC_TAG_SMARTCARDID 4
#define MAC_TAG_STREAMID    5

//  Tags: DS info, using CSession *
#define MAC_TAG_DS_ASSET_LENGTH                 21
#define MAC_TAG_DS_ASSET_PRICE                  22
#define MAC_TAG_DS_ASSET_ANALOG_COPY_CHARGE     23
#define MAC_TAG_DS_ASSET_ANALOG_COPY_ALLOWED    24
#define MAC_TAG_DS_ASSET_VCR_ALLOWED            25
#define MAC_TAG_DS_ASSET_VCR_CHARGE             26
#define MAC_TAG_DS_ASSET_COMPUTED_PRICE         27
#define MAC_TAG_DS_ASSET_RENTAL_TIME            28
#define MAC_TAG_DS_ASSET_PREVIEW_TIME           29
#define MAC_TAG_DS_ASSET_CONTENT_PROVIDER       30
#define MAC_TAG_DS_ASSET_GENRE                  31
#define MAC_TAG_DS_ASSET_SHORT_TITLE            32
#define MAC_TAG_DS_ASSET_RENTAL_TYPE            33
#define MAC_TAG_DS_ASSET_VIEWING_TIME           34
#define MAC_TAG_DS_ASSET_VIEWING_TYPE           35
#define MAC_TAG_DS_ASSET_SCRAMBLED              36
#define MAC_TAG_DS_ASSET_SCRAMBLED_TIME         37
#define MAC_TAG_DS_ASSET_SCRAMBLED_TYPE         38
#define MAC_TAG_DS_ASSET_EVENT_ID               39
#define MAC_TAG_DS_ASSET_DEACTIVATE_TIME        40
#define MAC_TAG_DS_ASSET_SUSPENDABLE            41
#define MAC_TAG_DS_ASSET_PROVIDER               42
#define MAC_TAG_DS_ASSET_RATING                 43
#define MAC_TAG_DS_ASSET_ASSET_TITLE            44
#define MAC_TAG_DS_ASSET_ASSET_BRIEF_TITLE      45
#define MAC_TAG_DS_ASSET_PROVIDER_ID            46
#define MAC_TAG_DS_ASSET_PROVIDER_ASSET_ID      47

#define MAC_TAG_DS_FOLDER_PRICINGOPTIONS        48
#define MAC_TAG_DS_FOLDER_PRICINGVALUE          49
#define MAC_TAG_DS_FOLDER_SUSPENDLISTOPTIONS    50
#define MAC_TAG_DS_FOLDER_SUSPENDLISTLIFETIME   51
#define MAC_TAG_DS_FOLDER_PACKAGEID             52
#define MAC_TAG_DS_FOLDER_BILLINGTAG            53

#define MAC_DS_NUM                              MAC_TAG_DS_FOLDER_BILLINGTAG - MAC_TAG_DS_ASSET_LENGTH + 1

//  Tags: AD info, using MODREQUEST *
#define MAC_TAG_AD_TYPE                         101
#define MAC_TAG_AD_MAC_ADDRESS                  102
#define MAC_TAG_AD_BILLING_ID                   103
#define MAC_TAG_AD_ASSET_ID                     104
#define MAC_TAG_AD_TITLE                        105
#define MAC_TAG_AD_SHORT_TITLE                  106
#define MAC_TAG_AD_BILLABLE                     107
#define MAC_TAG_AD_ASSET_PROVIDER               108
#define MAC_TAG_AD_ANALOG_COPY_ALLOWED          109
#define MAC_TAG_AD_ASSET_GENRE                  110
#define MAC_TAG_AD_FAST_FORWARD_COUNT           111
#define MAC_TAG_AD_REWIND_COUNT                 112
#define MAC_TAG_AD_PAUSE_COUNT                  113
#define MAC_TAG_AD_PLAY_TIME                    114
#define MAC_TAG_AD_PRICE                        115
#define MAC_TAG_AD_PURCHASE_TIME                116
#define MAC_TAG_AD_RENTAL_TIME                  117
#define MAC_TAG_AD_HOME_ID                      118
#define MAC_TAG_AD_SMART_CARD_ID                119
#define MAC_TAG_AD_PURCHASE_ID                  120
#define MAC_TAG_AD_EVENT_ID                     121
#define MAC_TAG_AD_PACKAGE_ID                   122
#define MAC_TAG_AD_PROVIDER                     123
#define MAC_TAG_AD_RATING                       124
#define MAC_TAG_AD_VCR_CHARGE                   125
#define MAC_TAG_AD_COPY_CHARGE                  126
#define MAC_TAG_AD_CONTEXT_ID                   127
#define MAC_TAG_AD_PROVIDER_ID                  128
#define MAC_TAG_AD_PROVIDER_ASSET_ID            129
#define MAC_TAG_AD_NEW_PURCHASE                 130
#define MAC_TAG_AD_RELEASE_CODE                 131

#define MAC_AD_NUM                              MAC_TAG_AD_RELEASE_CODE - MAC_TAG_AD_TYPE + 1

// Define the routines themselves
typedef ITVSTATUS (__cdecl *MAC_INIT_RTN)    (MACHANDLES *pHandles, MACCALLBACKS *pCallbacks);
typedef ITVSTATUS (__cdecl *MAC_UNINIT_RTN)  (void);
typedef ITVSTATUS (__cdecl *MAC_PROCESS_RTN) (BYTE *pRequest, BOOL *pbResponse);
typedef ITVSTATUS (__cdecl *MAC_REPORT_RTN)  (BYTE *pRequest, BOOL *pbResponse);

/*
 * If these definitions are included by the DLL defining the routines
 * then declare them as exports.
 * The users of the DLL will get the simple declarations
 */
#if defined(MACAPIDLL_DEF)
#define MACRTNDEF ITVSTATUS __declspec(dllexport)
#else
#define MACRTNDEF ITVSTATUS
#endif

#if defined(__cplusplus)
extern "C" {                                                // C++ to C linkage
#endif

// Declaration of MAC APIs
extern MACRTNDEF MacInitialize   (IN MACHANDLES *pHandles, IN MACCALLBACKS *pCallbacks);
extern MACRTNDEF MacUninitialize (void);
extern MACRTNDEF MacProcess      (IN BYTE *pRequest, IN OUT BOOL *pbResponse);
extern MACRTNDEF MacReport       (IN BYTE *pRequest, IN OUT BOOL *pbResponse);

// Internal Function Definitions
extern MANSTATUS MacManUtilCB (IN WCHAR *pwszCmd, 
                               IN WCHAR **ppwszResponse, 
                               IN DWORD *pwLength);

#if defined(__cplusplus)
} // End extern "C" bracket
#endif

#endif /* _MAC_INTERFACES_H_ */
