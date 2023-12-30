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
// title:        dsmccuser.h
//
// facility:    all DSMCC users
//
// abstract:
//
//        This is the data definitions for all SeaChange User data extentions to DSMCC.
//
//
// Revision History: 
// ------------------------
//
//   Rev       Date           Who               Description
//  ------ ---------------- -------  -----------------------------------
//  T0_0_0 12/23/98            JMB         Created from cm.h
// 
//  V1.3     08/22/01          NJW         Added PackageId for SVOD support.
//           10/11/01          NJW         Added TargetIP for TP streaming support.
//
/////////////////////////////////////////////////////////////////////////////


#ifndef __DSMCCUSER_H__
#define __DSMCCUSER_H__

#ifndef _ITV_H_
#include "itv.h"
#endif

//-----------------------------------------------------------------------------
// Codes used in the user data for SessionSetup Function
//

const BYTE CODE_NOT_SPECIFIED    = 0;     // Not Specified

const BYTE SESFUN_BIND_AS        = 1;     // Request for AppServer IP Address
const BYTE SESFUN_PLAY_RESTART   = 2;     // Request to play or restart an asset
const BYTE SESFUN_ATTACH         = 3;     // Request to attach to a forced copy asset

const BYTE SESSUBFUN_EMBEDDED    = 1;     // Request to play asset immediately
const BYTE SESSUBFUN_DELAYED     = 2;     // Request not to play immediately

const BYTE SESSUBFUN_LOOPED      = 128;   // (OR with *EMBEDDED or *DELAYED to indicate LOOP VIDEO)

const BYTE LSC_UDP_WITHSCPROT    = 64;    // Flag bit to OR with function on SC setup V1/V2 format to indicate the use of LSC UDP for SOPs
const BYTE LSC_TCP_WITHSCPROT    = 128;   // Flag bit to OR with function on SC setup V1/V2 format to indicate the use of LSC TCP fro SOPs

#define SCSOP_PROTOCOL           0
#define LSC_PROTOCOL_UDP         1
#define LSC_PROTOCOL_TCP         2
#define LSC_PROTOCOL_RTP         3

// Define the Descriptor Types of the SSP 
#define SSP_ASSET                       0x01
#define SSP_NODEGROUP                   0x02 
#define SSP_IP                          0x03
#define SSP_STREAMID                    0x04
#define SSP_APPREQDATA                  0x05
#define SSP_APPRESPDATA                 0x06
// Pending descriptor types for inclusion into SSP
#define SSP_TSID_LIST                   0x81
                                        
#define SSP_SC_BILLINGID                0x03
#define SSP_SC_PURCHASETIME             0x04
#define SSP_SC_TIMEREMAINING            0x05
#define SSP_SC_ERRORCODE                0x06
#define SSP_SC_HOMEID                   0x07
#define SSP_SC_PURCHASEID               0x08
#define SSP_SC_SMARTCARDID              0x09
#define SSP_SC_KEEPALIVE                0x0a
#define SSP_SC_TYPEINST                 0x0b
#define SSP_SC_SIGANALOGCOPYSTATE       0x0c
#define SSP_SC_SIGANALOGCOPYPURCHASE    0x0d
#define SSP_SC_SUPERCASID               0x0e
#define SSP_SC_PACKAGEID                0x0f
#define SSP_SC_IPTARGET                 0x10
#define SSP_SC_ASSETINFORMATION         0x11
#define SSP_SC_ON2_IP_TARGET            0x12
#define SSP_SC_CONTEXTID                0x13
#define SSP_SC_HIERARCHYID              0x14
#define SSP_SC_IPV6_TARGET              0x15
#define SSP_SC_CLIENT_REQUIREMENT       0x16
                                        
// Vendor Specific                      
#define SSP_SEACHANGE                   0x80
#define SSP_SC_SOPPROTOCOL              0x81
#define SSP_SC_APPSTATUS                0x82
#define SSP_SC_STREAMID                 0x83
#define SSP_SC_REQPRIORITY              0x84
#define SSP_SC_FORCETSID                0x85
#define SSP_SC_FORCEDNP                 0x86
#define SSP_SC_REQLOGENTRY              0x87
#define SSP_SC_PROVIDER_ID              0x88
#define SSP_SC_PROVIDER_ASSET_ID        0x89
#define SSP_SC_APPLICATION_NAME         0x8A
                                        
#define SSP_SC_FUNCTION                 0x01
#define SSP_SC_SUBFUNCTION              0x02

// following is here to get structures ready for on the wire transmission.
#ifdef _WIN32
#pragma pack( push, dsmcc_user_h, 1 )
#endif


//------------------------------------------------------------------------------
// DSM-CC User Data Protocol Versions supported
//
#define SSP_PROTOCOL_VERSION_BIT    0x80
//
const BYTE DSMCCUserProtocolVersion    = 2; // support this or earlier protocols
const BYTE DSMCCUserSSPProtocolVersion = (1 | SSP_PROTOCOL_VERSION_BIT); // support this or earlier protocols
 

const BYTE SSPProtocolId         = 1;
const BYTE SSPProtocolVersion    = 1;   // defined in lsc.h: LSC_PROTOCOL_VERSION

const BYTE SCVProtocolId         = 1;   // Vendor specific (SeaChange) protocol type
const BYTE SCVProtocolVersion    = 1;   // Vendor specific (SeaChange) protocol version


typedef struct _STREAMHANDLE
{
    DWORD      dwStreamIdNumber; // unique stream number for a CM.
    TYPEINST   TypeInst;         // the TypeInst of the CM.
} STREAMHANDLE, *PSTREAMHANDLE, *LPSTREAMHANDLE;


//------------------------------------------------------------------------------
// DNV1REQUSERDATAAPP structure
//
// Defines Version 2 of the user data format for session requests
//
#define SSP_SC_OPTION_PROVIDER_ID       0x00000001L
#define SSP_SC_OPTION_PROVIDER_ASSET_ID 0x00000002L
#define SSP_SC_OPTION_APPLICATION_NAME  0x00000004L
#define SSP_SC_OPTION_SMARTCARD_ID      0x00000008L
#define SSP_SC_OPTION_PURCHASE_ID       0x00000010L
#define SSP_SC_OPTION_PACKAGE_ID        0x00000020L
#define SSP_SC_OPTION_SUPERCAS_ID       0x00000040L

typedef struct _DNV2REQUSERDATA {
    /* SSP_SEACHANGE DESCRIPTORS (mandatory) */
    BYTE        Function;           // Function
    BYTE        SubFunction;        // Subfunction
    BYTE        Version;            // User Protocol Version (applies to all signalling user data)
    /* MAIN SSP DESCRIPTORS */
    DWORD       AssetUID;           // Asset UID
    DWORD       NodeGroupID;        // Node Group ID
    /* SSP_SEACHANGE DESCRIPTORS (mandatory) */
    //DWORD       AppType;            // Application type field
    SHORT       RequestPriority;    // Priority of request (0 = NORMAL)
    /* SSP_SEACHANGE DESCRIPTORS (optional) */
    DWORD       OptionFlag;         // Defines what of the following are valid (have been specified)
    /* The following really need to be moved from cmsmdef.h EVTDNSETUPIND_V1a.
    DWORD       SmartCardId;
    DWORD       PackageId;
    DWORD       SuperCasId;
    */
    BYTE        Reserved;           // Used to square up the following variables on a boundary.
    CHAR        ProviderId[32];     // Cablelabs Provider ID specification
    CHAR        ProviderAssetId[32];// Cablelabs Provider Asset ID specification
    CHAR        ApplicationName[256];// Cablelabs Application Name specification
    WORD        AppDataLen;         // Length of optional Application Data
   //BYTE        AppData[AppDataLen];
} DNV2REQUSERDATA, *PDNV2REQUSERDATA;


//------------------------------------------------------------------------------
// DNV1REQUSERDATA structure
//
// Defines Version 1 of the user data format for session requests
//
typedef struct _DNV1REQUSERDATA {
    BYTE        Function;           // Function
    BYTE        SubFunction;        // Subfunction
    BYTE        Version;            // User Protocol Version (applies to all signalling user data)
    DWORD       AssetUID;           // Asset UID
    DWORD       NodeGroupID;        // Node Group ID
    //DWORD      AppType;             // Application type field
} DNV1REQUSERDATA, *PDNV1REQUSERDATA;


//------------------------------------------------------------------------------
// DNSSPREQUSERDATA structure
//
// Defines the root elements of the SSP structure format
//
typedef struct _DNSSPREQUSERDATA {
    BYTE        ProtocolId;       // ID of the SSP protocol (value = 1)
    BYTE        Version;          // ID of protocol version (value = 1)
    BYTE        DescriptorCount;  // Number of descriptors in protocol block
} DNSSPREQUSERDATA, *PDNSSPREQUSERDATA;

// SSP Descriptor for ASSET
typedef struct _DNSSPASSETDSC {
    BYTE        descType;       // = SSP_ASSET
    BYTE        descLength;     // = 8
    DWORD       AppType;        // Application Type (UID)
    DWORD       AssetUID;       // Asset UID
} DNSSPASSETDSC, *PDNSSPASSETDSC;

// SSP Descriptor for IP
typedef struct _DNSSPIPPDSC {
    BYTE        descType;       // = SSP_IP
    BYTE        descLength;     // = 6
    WORD        ipPort;
    DWORD       ipAddress;
} DNSSPIPPDSC, *PDNSSPIPPDSC;

// SSP Descriptor for NODEGROUP
typedef struct _DNSSPNODEGRPPDSC {
    BYTE        descType;       // = SSP_NODEGROUP
    BYTE        descLength;     // = 6
    WORD        notused;        // MUST BE ZERO
    DWORD       nodeGrp;        // Integer value of NodeGroup ID (Service Group)
} DNSSPNODEGRPPDSC, *PDNSSPNODEGRPPDSC;

// SSP Descriptor Header
typedef struct _DNSSPDESCRIPTOR {
    BYTE        descType;
    BYTE        descLength;
} DNSSPDESCRIPTOR, *PDNSSPDESCRIPTOR;

// SSP Descriptor for TSID_LIST
typedef struct _DN_SSP_TSID_LIST_DESCRIPTOR {
    BYTE        descType;       // = SSP_TSID_LIST
    BYTE        descLength;     // = variable: 2 + (2 * tsidCount)
    WORD        tsidCount;
    /*
    WORD        tsidList[ tsidCount ];
    */
} DN_SSP_TSID_LIST, *PDN_SSP_TSID_LIST;

////////////////////////////////////////////////////////////////////////////////
//
// SSP SeaChange Descriptors
//
////////////////////////////////////////////////////////////////////////////////

// SSP Descriptor for SSP_SEACHANGE
typedef struct _DNSSPSCSPECIFICDSC {
    BYTE        descType;       // = SSP_SEACHANGE
    BYTE        descLength;     // = <minimum 3 -- depending on number of descriptors within>
    BYTE        ScProtType;     // Protocol type
    BYTE        ScProtVersion;  // Protocol version
    BYTE        descCount;      // number of SC specific descriptors within
} DNSSPSCSPECIFICDSC, *PDNSSPSCSPECIFICDSC;

// SSP Descriptor for SC_STREAMID
typedef struct _DNSSPSCSTREAMIDDSC {
    BYTE            descType;   // = SSP_SC_STREAMID
    BYTE            descLength; // = 12
    STREAMHANDLE    StrId;      // unsigned integer
} DNSSPSCSTREAMIDDSC, *PDNSSPSCSTREAMIDDSC;

// SSP Descriptor for SC_SOPPROTOCOL
typedef struct _DNSSPSOPPROTDSC {
    BYTE        descType;       // = SSP_SC_SOPPROTOCOL
    BYTE        descLength;     // = 1
    BYTE        SopProt;        // unsigned integer
} DNSSPSOPPROTDSC, *PDNSSPSOPPROTDSC;

// SSP Descriptor for SC_FUNCTION
typedef struct _DNSSPSCFUNCTIONDSC {
    BYTE        descType;       // = SSP_SC_FUNCTION
    BYTE        descLength;     // = 1
    BYTE        function;       //  PLAY = 0x02
} DNSSPSCFUNCTIONDSC, *PDNSSPSCFUNCTIONDSC;

// SSP Descriptor for SC_SUBFUNCTION
typedef struct _DNSSPSCSUBFUNCTIONDSC {
    BYTE        descType;       // = SSP_SC_SUBFUNCTION
    BYTE        descLength;     // = 1
    BYTE        subFunction;    // 0x1 start 0x2 don't start play
} DNSSPSCSUBFUNCTIONDSC, *PDNSSPSCSUBFUNCTIONDSC;

// SSP Descriptor for SSP_SC_REQPRIORITY (ABSENCE OF DESCRIPTOR IMPLIES NORMAL PRIORITY)
typedef struct _DNSSPSCREQPRIORITYDSC {
    BYTE        descType;       // = SSP_SC_REQPRIORITY
    BYTE        descLength;     // = 2
    WORD        priority;       // 0 = NORMAL, +n = ELEVATED PRIORITY
} DNSSPSCREQPRIORITYDSC, *PDNSSPSCREQPRIORITYDSC;

// SSP Descriptor for SSP_SC_FORCETSID (Force the CM to select a path using the specified TSID)
typedef struct _DNSSPSCFORCETSID {
    BYTE        descType;       // = SSP_SC_FORCETSID
    BYTE        descLength;     // = 4
    DWORD       TSID;           // 0 = ANY; nnn = USE THIS TSID
} DNSSPSCFORCETSID, *PDNSSPSCFORCETSID;

// SSP Descriptor for SSP_SC_PURCHASEID 
typedef struct _DNSSPSCPURCHASEID {
    BYTE        descType;       // = SSP_SC_PURCHASEID
    BYTE        descLength;     // = 4
    DWORD       purchaseId;     // 
} DNSSPSCPURCHASEID, *PDNSSPSCPURCHASEID;

// SSP Descriptor for SSP_SC_SMARTCARDID
typedef struct _DNSSPSCSMARTCARDID {
    BYTE        descType;       // = SSP_SC_SMARTCARDID
    BYTE        descLength;     // = 4
    DWORD       smartCardId;    // 
} DNSSPSCSMARTCARDID, *PDNSSPSCSMARTCARDID;

// SSP Descriptor for SSP_SC_KEEPALIVE
typedef struct _DNSSPSCKEEPALIVE {
    BYTE        descType;       // = SSP_SC_KEEPALIVE
    BYTE        descLength;     // = 4
    DWORD       keepAlive;      // 
} DNSSPSCKEEPALIVE, *PDNSSPSCKEEPALIVE;

// SSP Descriptor for SSP_SC_TYPEINST
typedef struct _DNSSPSCTYPEINST {
    BYTE        descType;       // = SSP_SC_TYPEINST
    BYTE        descLength;     // = 8
    TYPEINST    typeInst;       // 
} DNSSPSCTYPEINST, *PDNSSPSCTYPEINST;

// SSP Descriptor for SSP_SC_SIGANALOGCOPYSTATE
typedef struct _DNSSPSCSIGANALOGCOPYSTATE {
    BYTE        descType;           // = SSP_SC_SIGANALOGCOPYSTATE
    BYTE        descLength;         // = 4
    DWORD       sigAnalogCopyState; // 
} DNSSPSCSIGANALOGCOPYSTATE, *PDNSSPSCSIGANALOGCOPYSTATE;

// SSP Descriptor for SSP_SC_SIGANALOGCOPYPURCHASE
typedef struct _DNSSPSCSIGANALOGCOPYPURCHASE {
    BYTE        descType;               // = SSP_SC_SIGANALOGCOPYPURCHASE
    BYTE        descLength;             // = 4
    DWORD       sigAnalogCopyPurchase;  // 
} DNSSPSCSIGANALOGCOPYPURCHASE, *PDNSSPSCSIGANALOGCOPYPURCHASE;

// SSP Descriptor for SSP_SC_SUPERCASID
typedef struct _DNSSPSCSUPERCASID {
    BYTE        descType;       // = SSP_SC_SUPERCASID
    BYTE        descLength;     // = 4
    DWORD       superCasID;     // 
} DNSSPSCSUPERCASID, *PDNSSPSCSUPERCASID;

// SSP Descriptor for SSP_SC_PACKAGEID
typedef struct _DNSSPSCPACKAGEID {
    BYTE        descType;       // = SSP_SC_PACKAGEID
    BYTE        descLength;     // = 4
    DWORD       packageID;      // 
} DNSSPSCPACKAGEID, *PDNSSPSCPACKAGEID;

typedef struct _SCIPTARGET {
    DWORD       IP;
    USHORT      Port;
    BYTE        MAC[6];
} SCIPTARGET, *PSCIPTARGET;

// SSP Descriptor for SSP_SC_IPTARGET
typedef struct _DNSSPSCIPTARGET {
    BYTE        descType;       // = SSP_SC_IPTARGET
    BYTE        descLength;     // = 12
    SCIPTARGET  s;
} DNSSPSCIPTARGET, *PDNSSPSCIPTARGET;

////// 2001/03/01: FORCE DNP
//
// Clients can send up the SSP_SC_FORCEDNP descriptor and request a specific DNP
// to be configured for them.
#define SC_FORCE_CLUSTER    0x01
#define SC_FORCE_SLICEGROUP 0x02
#define SC_FORCE_SLICE      0x04
#define SC_FORCE_PIPE       0x08
#define SC_FORCE_NODEGROUP  0x10

typedef struct _SCFORCEDNP {
    BYTE        byFlags;        // 000001 = 0x01 - cluster valid
                                // 000010 = 0x02 - slicegroup valid
                                // 000100 = 0x04 - slice valid
                                // 001000 = 0x08 - pipe valid
                                // 010000 = 0x10 - nodegroup valid
    BYTE        byReserved;     // = 0
    DWORD       dwClusterID;
    DWORD       dwSliceGroupID;
    DWORD       dwSliceID;
    DWORD       dwPipeID;
    DWORD       dwNodeGroupID;
} SCFORCEDNP, *PSCFORCEDNP;

// SSP Descriptro for SSP_SC_FORCEDNP
typedef struct _DNSSPSCFORCEDNP {
    BYTE        descType;       // = SSP_SC_FORCEDNP
    BYTE        descLength;     // = 22 = sizeof(SCFORCEDNP)
    SCFORCEDNP  s;
} DNSSPSCFORCEDNP, *PDNSSPSCFORCEDNP;
//
////// FORCE DNP

// SSP Descriptor for SSP_SC_HOMEID
typedef struct _DNSSPSCHOMEID {
    BYTE        descType;       // = SSP_SC_HOMEID
    BYTE        descLength;     // = 4
    DWORD       homeId;         // 
} DNSSPSCHOMEID, *PDNSSPSCHOMEID;

// SSP Descriptor for SSP_SC_BILLINGID
typedef struct _DNSSPSCBILLINGID {
    BYTE        descType;       // = SSP_SC_BILLINGID
    BYTE        descLength;     // = 4
    DWORD       billingId;      // 
} DNSSPSCBILLINGID, *PDNSSPSCBILLINGID;

// SSP Descriptor for SSP_SC_CONTEXTID
typedef struct _DNSSPSCCONTEXTID {
    BYTE        descType;       // = SSP_SC_CONTEXTID
    BYTE        descLength;     // = 4
    DWORD       contextId;      // 
} DNSSPSCCONTEXTID, *PDNSSPSCCONTEXTID;

// SSP Descriptor for SSP_SC_HIERARCHYID
typedef struct _DN_SSP_SC_HIERARCHY_ID {
    BYTE        descType;       // = SSP_SC_HIERARCHYID
    BYTE        descLength;     // = 4
    DWORD       hierarchyId;    //
} DN_SSP_SC_HIERARCHY_ID, *PDN_SSP_SC_HIERARCHY_ID;

// SSP Descriptor for SSP_SC_ASSETINFORMATION
typedef struct _DNSSPSCASSETINFORMATION {
    BYTE        descType;       // = SSP_SC_ASSETINFORMATION
    WORD        descLength;     // = variable
    //PVOID     dataBlock
} DNSSPSCASSETINFORMATION, *PDNSSPSCASSETINFORMATION;

// SSP Descriptor for SSP_SC_REQLOGENTRY
typedef struct _SSP_SC_REQLOGDATA { // Size (bytes) Total (bytes)
    MACADDR     Settop;             // 6            6
    DNSID       Session;            // 10           16
    DWORD       Sid;                // 4            20
    DWORD       BillingId;          // 4            24
    DWORD       Asset;              // 4            28
    DWORD       AppType;            // 4            32
    TYPEINST    CmType;             // 8            40
    DWORD       PeerGrp;            // 4            44
    DWORD       NodeGrp;            // 4            48
    time_t      ReqTime;            // 4            52
    time_t      ComplTime;          // 4            56
    DWORD       Bitrate;            // 4            60
    DWORD       Cluster;            // 4            64
    DWORD       SliceGroup;         // 4            68
    DWORD       Slice;              // 4            72
    DWORD       Pipe;               // 4            76
    DWORD       Status;             // 4            80
    DWORD       ExtendedStatus;     // 4            84
    DWORD       SetupTime;          // 4            88
    DWORD       AEListTime;         // 4            92
    DWORD       ConfigureTime;      // 4            96
    DWORD       ResourceTime;       // 4            100
    DWORD       ReadyTime;          // 4            104
    BYTE        SOPType;            // 1            105
    BYTE        Function;           // 1            106
    BYTE        SubFunction;        // 1            107
    BYTE        Reserved;           // 1            108
    DWORD       MpegPn;             // 4            112
    CHAR        ProviderId[20];     // 20           132
    CHAR        ProviderAssetId[20];// 20           152
    CHAR        AppName[64];        // 64           216
    time_t      PurchaseTime;       // 4            220
    BYTE        SettopIp[16];       // 16           236
} SSP_SC_REQLOGDATA, *PSSP_SC_REQLOGDATA;
typedef const SSP_SC_REQLOGDATA *PCSSP_SC_REQLOGDATA;

typedef struct _DN_SSP_SC_REQLOGENTRY {
    BYTE        descType;       // = SSP_SC_REQLOGENTRY
    BYTE        descLength;     // = sizeof(SSP_SC_REQLOGDATA)
    SSP_SC_REQLOGDATA
                s;
} DN_SSP_SC_REQLOGENTRY, *PDN_SSP_SC_REQLOGENTRY;

typedef struct _SC_ON2_IP_TARGET {
    DWORD       IP;
    USHORT      VideoPort;
    USHORT      AudioPort1;
    USHORT      AudioPort2;
    BYTE        MAC[6];
    DWORD       VideoId;
    DWORD       AudioId1;
    DWORD       AudioId2;
} SC_ON2_IP_TARGET, *PSC_ON2_IP_TARGET;

// SSP Descriptor for SSP_SC_ON2_IP_TARGET
typedef struct _DN_SSP_SC_ON2_IP_TARGET {
    BYTE        descType;   // = SSP_SC_ON2_IP_TARGET
    BYTE        descLength; // = 28 bytes
    SC_ON2_IP_TARGET
                s;
} DN_SSP_SC_ON2_IP_TARGET, *PDN_SSP_SC_ON2_IP_TARGET;

// SSP Descriptor for SSP_SC_PROVIDER_ID
typedef struct _DN_SSP_SC_PROVIDER_ID
{
    BYTE        descType;       // = SSP_SC_PROVIDER_ID (0x88)
    BYTE        descLength;     // = 20
    BYTE        ProviderId[20];
} DN_SSP_SC_PROVIDER_ID, *PDN_SSP_SC_PROVIDER_ID;

// SSP Descriptor for SSP_SC_PROVIDER_ASSET_ID
typedef struct _DN_SSP_SC_PROVIDER_ASSET_ID
{
    BYTE        descType;       // = SSP_SC_PROVIDER_ASSET_ID (0x89)
    BYTE        descLength;     // = 20
    BYTE        ProviderAssetId[20];
} DN_SSP_SC_PROVIDER_ASSET_ID, *PDN_SSP_SC_PROVIDER_ASSET_ID;

// SSP Descriptor for SSP_SC_APPLICATION_NAME
typedef struct _DN_SSP_SC_APPLICATION_NAME
{
    BYTE        descType;       // = SSP_SC_APPLICATION_NAME (0x8A)
    BYTE        descLength;     // =
    //BYTE        ApplicationName[ descLength ]; // Ascii character set
} DN_SSP_SC_APPLICATION_NAME, *PDN_SSP_SC_APPLICATION_NAME;

typedef struct _SC_IPV6_TARGET {
    BYTE        IP[16];
    USHORT      Port;
    MACADDR     MAC;
} SC_IPV6_TARGET, *PSC_IPV6_TARGET;

// SSP Descriptor for SSP_SC_IPV6_TARGET
typedef struct _DN_SSP_SC_IPV6_TARGET
{
    BYTE        descType;       // = SSP_SC_IPV6_TARGET (0x15)
    BYTE        descLength;     // = 16
    SC_IPV6_TARGET
                s;
} DN_SSP_SC_IPV6_TARGET, *PDN_SSP_SC_IPV6_TARGET;

#define SSP_SC_CLIENT_REQ_QAM_SPECIFIER             0x0001
#define SSP_SC_CLIENT_REQ_SPIGOT_FAMILY_SPECIFIER   0x0002

// SSP Descriptor for SSP_SC_CLIENT_REQUIREMENT
typedef struct _DN_SSP_SC_CLIENT_REQUIREMENT
{
    BYTE        descType;       // = SSP_SC_CLIENT_REQUIREMENT (0x16)
    BYTE        descLength;     // = 6
    WORD        requirementType;
    DWORD       requirementValue;
} DN_SSP_SC_CLIENT_REQUIREMENT, *PDN_SSP_SC_CLIENT_REQUIREMENT;

////////////////////////////////////////////////////////////////////////////////
//
// SSP Setup Request Convenience Structures
//
////////////////////////////////////////////////////////////////////////////////


// Standard request for SSP with out app data
typedef struct _DNSSPSETUPREQWOAPP {
    DNSSPREQUSERDATA       sspHdr;
    DNSSPASSETDSC          sspAsset;
    DNSSPNODEGRPPDSC       sspNG;
    DNSSPSCSPECIFICDSC     sspSCHdr;
    DNSSPSOPPROTDSC        sspSCSop;
    DNSSPSCFUNCTIONDSC     sspSCFun;
    DNSSPSCSUBFUNCTIONDSC  sspSCSubFun;
} DNSSPSETUPREQWOAPP, *PDNSSPSETUPREQWOAPP;


// Request for SSP with out app data and SmartCardId
typedef struct _DNSSPSETUPREQWOAPPSMARTCARD {
    DNSSPREQUSERDATA       sspHdr;
    DNSSPASSETDSC          sspAsset;
    DNSSPNODEGRPPDSC       sspNG;
    DNSSPSCSPECIFICDSC     sspSCHdr;
    DNSSPSOPPROTDSC        sspSCSop;
    DNSSPSCFUNCTIONDSC     sspSCFun;
    DNSSPSCSUBFUNCTIONDSC  sspSCSubFun;
    DNSSPSCSMARTCARDID     sspSCSmartCard;
} DNSSPSETUPREQWOAPPSMARTCARD, *PDNSSPSETUPREQWOAPPSMARTCARD;

/*
XXX: Get rid of this
// Request for SSP with out app data and SmartCardId
typedef struct _DNSSPSETUPREQWOAPPSMARTCARDPURID {
    DNSSPREQUSERDATA       sspHdr;
    DNSSPASSETDSC          sspAsset;
    DNSSPNODEGRPPDSC       sspNG;
    DNSSPSCSPECIFICDSC     sspSCHdr;
    DNSSPSOPPROTDSC        sspSCSop;
    DNSSPSCFUNCTIONDSC     sspSCFun;
    DNSSPSCSUBFUNCTIONDSC  sspSCSubFun;
    DNSSPSCSMARTCARDID     sspSCSmartCard;
    DNSSPSCPURCHASEID      sspSCPurchaseId;
} DNSSPSETUPREQWOAPPSMARTCARDPURID, *PDNSSPSETUPREQWOAPPSMARTCARDPURID ;
*/

// There are four responses cut on 1 vs 2 IP address and StreamID (streamop) vs StreamHandle (LSC)

// Standard response for SSP
typedef struct _DNSSPRESPUSERDATA {
    BYTE            ProtocolId;         // ID of the SSP protocol (value = 1)
    BYTE            Version;            // ID of protocol version (value = 1)
    BYTE            DescriptorCount;    // Number of descriptors in protocol block = 3
    BYTE            ipDescType;         // IP Descriptor Type byte
    BYTE            ipDescLength;       // IP Descriptor Length byte
    WORD            ipPort;             // IP Port
    DWORD           ipAddress;          // IP Address
    BYTE            shDescType;         // Stream Handle Descriptor Type byte
    BYTE            shDescLength;       // Stream Handle Descriptor Length byte
    DWORD           shSid;              // Stream Handle DWORD (network order)
    BYTE            reqLogDescType;     // = SSP_SC_REQLOGENTRY
    BYTE            reqLogDescLength;   // = sizeof(SSP_SC_REQLOGDATA)
    SSP_SC_REQLOGDATA
                    reqLogEntry;
} DNSSPRESPUSERDATA, *PDNSSPRESPUSERDATA;

// Response for SSP with SC_STREAMID descriptor
typedef struct _DNSSPRESPUSERDATA_1 {
    BYTE            ProtocolId;         // ID of the SSP protocol (value = 1)
    BYTE            Version;            // ID of protocol version (value = 1)
    BYTE            DescriptorCount;    // Number of descriptors in protocol block = 3
    BYTE            ipDescType;         // IP Descriptor Type byte
    BYTE            ipDescLength;       // IP Descriptor Length byte
    WORD            ipPort;             // IP Port
    DWORD           ipAddress;          // IP Address
    BYTE            descType;           // = SSP_SEACHANGE
    BYTE            descLength;         // = 17
    BYTE            ScProtType;         // Protocol type
    BYTE            ScProtVersion;      // Protocol version
    BYTE            descCount;          // 1
    BYTE            sidType;            // = SSP_SC_STREAMID
    BYTE            sidLength;          // = 12
    STREAMHANDLE    StrId;              // no comment
    BYTE            reqLogDescType;     // = SSP_SC_REQLOGENTRY
    BYTE            reqLogDescLength;   // = sizeof(SSP_SC_REQLOGDATA)
    SSP_SC_REQLOGDATA
                    reqLogEntry;
} DNSSPRESPUSERDATA_1, *PDNSSPRESPUSERDATA_1;

// Response for SSP with Alternate IP 
typedef struct _DNSSPRESPUSERDATA_2 {
    BYTE            ProtocolId;         // ID of the SSP protocol (value = 1)
    BYTE            Version;            // ID of protocol version (value = 1)
    BYTE            DescriptorCount;    // Number of descriptors in protocol block = 3
    BYTE            ipDescType;         // IP Descriptor Type byte
    BYTE            ipDescLength;       // IP Descriptor Length byte
    WORD            ipPort;             // IP Port
    DWORD           ipAddress;          // IP Address
    BYTE            shDescType;         // Stream Handle Descriptor Type byte
    BYTE            shDescLength;       // Stream Handle Descriptor Length byte
    DWORD           shSid;              // Stream Handle DWORD (network order)
    BYTE            descType;           // = SSP_SEACHANGE
    BYTE            descLength;         // = 11
    BYTE            ScProtType;         // Protocol type
    BYTE            ScProtVersion;      // Protocol version
    BYTE            descCount;          // 1
    BYTE            aipDescType;        // IP Descriptor Type byte
    BYTE            aipDescLength;      // IP Descriptor Length byte
    WORD            aipPort;            // IP Port
    DWORD           aipAddress;         // IP Address
    BYTE            reqLogDescType;     // = SSP_SC_REQLOGENTRY
    BYTE            reqLogDescLength;   // = sizeof(SSP_SC_REQLOGDATA)
    SSP_SC_REQLOGDATA
                    reqLogEntry;
} DNSSPRESPUSERDATA_2, *PDNSSPRESPUSERDATA_2;


// Response for SSP with Alternate IP AND SC_STREAMID descriptors
typedef struct _DNSSPRESPUSERDATA_3 {
    BYTE            ProtocolId;         // ID of the SSP protocol (value = 1)
    BYTE            Version;            // ID of protocol version (value = 1)
    BYTE            DescriptorCount;    // Number of descriptors in protocol block = 4
    BYTE            ipDescType;         // IP Descriptor Type byte
    BYTE            ipDescLength;       // IP Descriptor Length byte
    WORD            ipPort;             // IP Port
    DWORD           ipAddress;          // IP Address
    BYTE            descType;           // = SSP_SEACHANGE
    BYTE            descLength;         // = 25
    BYTE            ScProtType;         // Protocol type
    BYTE            ScProtVersion;      // Protocol version
    BYTE            descCount;          // 2
    BYTE            aipDescType;        // Alternate IP Descriptor Type byte
    BYTE            aipDescLength;      // Alternate IP Descriptor Length byte
    WORD            aipPort;            // Alternate IP Port
    DWORD           aipAddress;         // Alternate IP Address
    BYTE            sidType;            // = SSP_SC_STREAMID
    BYTE            sidLength;          // = 12
    STREAMHANDLE    StrId;              // no comment
    BYTE            reqLogDescType;     // = SSP_SC_REQLOGENTRY
    BYTE            reqLogDescLength;   // = sizeof(SSP_SC_REQLOGDATA)
    SSP_SC_REQLOGDATA
                    reqLogEntry;
} DNSSPRESPUSERDATA_3, *PDNSSPRESPUSERDATA_3;


// There are four responses cut on 1 vs 2 IP address and StreamID (streamop) vs StreamHandle (LSC)
//  Each of the following is for DNCSType 2 and  has purchase ID


// Standard response for SSP for DNCSType == 2
typedef struct _DNT2SSPRESPUSERDATA {
    BYTE            ProtocolId;         // ID of the SSP protocol (value = 1)
    BYTE            Version;            // ID of protocol version (value = 1)
    BYTE            DescriptorCount;    // Number of descriptors in protocol block = 4
    BYTE            ipDescType;         // IP Descriptor Type byte
    BYTE            ipDescLength;       // IP Descriptor Length byte
    WORD            ipPort;             // IP Port
    DWORD           ipAddress;          // IP Address
    BYTE            shDescType;         // Stream Handle Descriptor Type byte
    BYTE            shDescLength;       // Stream Handle Descriptor Length byte
    DWORD           shSid;              // Stream Handle DWORD (network order)
    BYTE            descType;           // = SSP_SEACHANGE
    BYTE            descLength;         // = 9
    BYTE            ScProtType;         // Protocol type
    BYTE            ScProtVersion;      // Protocol version
    BYTE            descCount;          // 1
    BYTE            piDescType;         // Purchase ID descriptor type
    BYTE            piDescLength;       // Purchase ID descriptor length
    DWORD           PurchaseId;         // Purchase ID
    BYTE            reqLogDescType;     // = SSP_SC_REQLOGENTRY
    BYTE            reqLogDescLength;   // = sizeof(SSP_SC_REQLOGDATA)
    SSP_SC_REQLOGDATA
                    reqLogEntry;
} DNT2SSPRESPUSERDATA, *PDNT2SSPRESPUSERDATA;


// Response for SSP with SC_STREAMID descriptor for DNCSType == 2
typedef struct _DNT2SSPRESPUSERDATA_1 {
    BYTE            ProtocolId;         // ID of the SSP protocol (value = 1)
    BYTE            Version;            // ID of protocol version (value = 1)
    BYTE            DescriptorCount;    // Number of descriptors in protocol block = 3
    BYTE            ipDescType;         // IP Descriptor Type byte
    BYTE            ipDescLength;       // IP Descriptor Length byte
    WORD            ipPort;             // IP Port
    DWORD           ipAddress;          // IP Address
    BYTE            descType;           // = SSP_SEACHANGE
    BYTE            descLength;         // = 23
    BYTE            ScProtType;         // Protocol type
    BYTE            ScProtVersion;      // Protocol version
    BYTE            descCount;          // 2
    BYTE            sidType;            // = SSP_SC_STREAMID
    BYTE            sidLength;          // = 12
    STREAMHANDLE    StrId;              // no comment
    BYTE            piDescType;         // Purchase ID descriptor type
    BYTE            piDescLength;       // Purchase ID descriptor length
    DWORD           PurchaseId;         // Purchase ID
    BYTE            reqLogDescType;     // = SSP_SC_REQLOGENTRY
    BYTE            reqLogDescLength;   // = sizeof(SSP_SC_REQLOGDATA)
    SSP_SC_REQLOGDATA
                    reqLogEntry;
} DNT2SSPRESPUSERDATA_1, *PDNT2SSPRESPUSERDATA_1;


// Response for SSP with Alternate IP for DNCSType == 2
typedef struct _DNT2SSPRESPUSERDATA_2 {
    BYTE            ProtocolId;         // ID of the SSP protocol (value = 1)
    BYTE            Version;            // ID of protocol version (value = 1)
    BYTE            DescriptorCount;    // Number of descriptors in protocol block = 4
    BYTE            ipDescType;         // IP Descriptor Type byte
    BYTE            ipDescLength;       // IP Descriptor Length byte
    WORD            ipPort;             // IP Port
    DWORD           ipAddress;          // IP Address
    BYTE            shDescType;         // Stream Handle Descriptor Type byte
    BYTE            shDescLength;       // Stream Handle Descriptor Length byte
    DWORD           shSid;              // Stream Handle DWORD (network order)
    BYTE            descType;           // = SSP_SEACHANGE
    BYTE            descLength;         // = 17
    BYTE            ScProtType;         // Protocol type
    BYTE            ScProtVersion;      // Protocol version
    BYTE            descCount;          // 2
    BYTE            piDescType;         // Purchase ID descriptor type
    BYTE            piDescLength;       // Purchase ID descriptor length
    DWORD           PurchaseId;         // Purchase ID
    BYTE            aipDescType;        // IP Descriptor Type byte
    BYTE            aipDescLength;      // IP Descriptor Length byte
    WORD            aipPort;            // IP Port
    DWORD           aipAddress;         // IP Address
    BYTE            reqLogDescType;     // = SSP_SC_REQLOGENTRY
    BYTE            reqLogDescLength;   // = sizeof(SSP_SC_REQLOGDATA)
    SSP_SC_REQLOGDATA
                    reqLogEntry;
} DNT2SSPRESPUSERDATA_2, *PDNT2SSPRESPUSERDATA_2;


// Response for SSP with Alternate IP AND SC_STREAMID descriptors for DNCSType == 2
typedef struct _DNT2SSPRESPUSERDATA_3 {
    BYTE            ProtocolId;         // ID of the SSP protocol (value = 1)
    BYTE            Version;            // ID of protocol version (value = 1)
    BYTE            DescriptorCount;    // Number of descriptors in protocol block = 3
    BYTE            ipDescType;         // IP Descriptor Type byte
    BYTE            ipDescLength;       // IP Descriptor Length byte
    WORD            ipPort;             // IP Port
    DWORD           ipAddress;          // IP Address
    BYTE            descType;           // = SSP_SEACHANGE
    BYTE            descLength;         // = 31
    BYTE            ScProtType;         // Protocol type
    BYTE            ScProtVersion;      // Protocol version
    BYTE            descCount;          // 3
    BYTE            aipDescType;        // Alternate IP Descriptor Type byte
    BYTE            aipDescLength;      // Alternate IP Descriptor Length byte
    WORD            aipPort;            // Alternate IP Port
    DWORD           aipAddress;         // Alternate IP Address
    BYTE            sidType;            // = SSP_SC_STREAMID
    BYTE            sidLength;          // = 12
    STREAMHANDLE    StrId;              // no comment
    BYTE            piDescType;         // Purchase ID descriptor type
    BYTE            piDescLength;       // Purchase ID descriptor length
    DWORD           PurchaseId;         // Purchase ID
    BYTE            reqLogDescType;     // = SSP_SC_REQLOGENTRY
    BYTE            reqLogDescLength;   // = sizeof(SSP_SC_REQLOGDATA)
    SSP_SC_REQLOGDATA
                    reqLogEntry;
} DNT2SSPRESPUSERDATA_3, *PDNT2SSPRESPUSERDATA_3;



// Release Response User data for SSP protocol containing a status value
typedef struct _DNSSPRELEASEUSERDATA {
    BYTE            ProtocolId;         // ID of the SSP protocol (value = 1)
    BYTE            Version;            // ID of protocol version (value = 1)
    BYTE            DescriptorCount;    // Number of descriptors in protocol block = 3
    BYTE            arDscType;
    BYTE            arDscLength;
    DWORD           arResponse;
    BYTE            reqLogDescType;     // = SSP_SC_REQLOGENTRY
    BYTE            reqLogDescLength;   // = sizeof(SSP_SC_REQLOGDATA)
    SSP_SC_REQLOGDATA
                    reqLogEntry;
} DNSSPRELEASEUSERDATA, *PDNSSPRELEASEUSERDATA;


//------------------------------------------------------------------------------
// DNV1ATTACHUSERDATA structure
//
// Defines the Version 1 user data format for attaching to a previously created session
// used in case a replica was unavailable and was copied on request
//
typedef struct _DNV1ATTACHUSERDATA {
    BYTE            Function;            // Function
    BYTE            SubFunction;         // Subfunction
    BYTE            Version;             // User Protocol Version (applies to all signalling user data)
    //DWORD           AppType;             // Application type field
    STREAMHANDLE    StrID;               // Stream to attach to
} DNV1ATTACHUSERDATA, *PDNV1ATTACHUSERDATA;


//------------------------------------------------------------------------------
// DNV1BINDREQUSERDATA structure
//
// Defines the user data format for attaching to a previously created session
// used in case a replica was unavailable and was copied on request
//
typedef struct _DNV1BINDREQUSERDATA {
    BYTE            Function;            // Function
    BYTE            SubFunction;         // Not Used -- Set to 0
    BYTE            Version;             // User Protocol Version (applies to all signalling user data)
    //DWORD           AppType;             // Application type field
} DNV1BINDREQUSERDATA, *PDNV1BINDREQUSERDATA;


//------------------------------------------------------------------------------
// DNRELEASEUSERDATA structure
//
// Defines the user data format for releasing streaming sessions
//
typedef struct _DNRELEASEUSERDATA {
    DWORD           Reason;              // Reason for release (WILL BE TREATED as TYPE "APPSTATUS") !!!!
    SSP_SC_REQLOGDATA
                    ReqLogEntry;
} DNRELEASEUSERDATA, *PDNRELEASEUSERDATA;


//------------------------------------------------------------------------------
// DNRESPUSERDATA structure
//
// Defines the user data format for session setup respose messages
//
typedef struct _DNRESPUSERDATA {
    IPADDRESS       IPAddress;           // CM Instance IP Address
    SHORT           Port;                // Used to connect to CM for stream operations
    STREAMHANDLE    StrID;               // Used to request stream operations
} DNRESPUSERDATA, *PDNRESPUSERDATA;

typedef struct _DNRESPUSERDATA_V1 {
    IPADDRESS       IPAddress;
    SHORT           Port;
    STREAMHANDLE    StrID;
    SSP_SC_REQLOGDATA
                    ReqLogEntry;
} DNRESPUSERDATA_V1, *PDNRESPUSERDATA_V1;


//------------------------------------------------------------------------------
// DNBINDRESPUSERDATA structure
//
// Defines the user data format for bind AS respose messages
//
typedef struct _DNBINDRESPUSERDATA {
    TYPEINST        ServerInstance;      // Requested Server Instance ID
    IPADDRESS       IPAddress;           // CM Instance IP Address
    SHORT           Port;                // Used to connect to CM for stream operations
} DNBINDRESPUSERDATA, *PDNBINDRESPUSERDATA;


//-------------------------------- BL3 Legacy definitions --------------------------
//
// ?? Delete these after all clients have implemented.
// ?? Double check cm.h for a union including these.

//------------------------------------------------------------------------------
// DNREQUSERDATA structure
//
// Defines the legacy version of the user data format for session requests
//
typedef struct _DNREQUSERDATA {
    BYTE            Function;            // Function
    BYTE            SubFunction;         // Subfunction
    //WORD            AppID;               // Application ID field
    DWORD           AssetUID;            // Asset UID
    DWORD           NodeGroupID;         // Node Group ID
} DNREQUSERDATA, *PDNREQUSERDATA;


//------------------------------------------------------------------------------
// DNATTACHUSERDATA structure
//
// Defines the Legacy version of user data format for attaching to a previously created session
// used in case a replica was unavailable and was copied on request
//
typedef struct _DNATTACHUSERDATA {
    BYTE            Function;            // Function
    BYTE            SubFunction;         // Subfunction
    //WORD            AppID;               // Application ID field
    STREAMHANDLE    StrID;               // Stream to attach to
} DNATTACHUSERDATA, *PDNATTACHUSERDATA;

//------------------------------------------------------------------------------
// DNBINDREQUSERDATA structure
//
// Defines the user data format for attaching to a previously created session
// used in case a replica was unavailable and was copied on request
//
typedef struct _DNBINDREQUSERDATA {
    BYTE            Function;            // Function
    //WORD            AppID;               // Application type field
} DNBINDREQUSERDATA, *PDNBINDREQUSERDATA;


#ifdef _WIN32
#pragma pack( pop,  dsmcc_user_h)
#endif


#endif
