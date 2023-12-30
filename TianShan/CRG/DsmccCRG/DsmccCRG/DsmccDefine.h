#ifndef __H_DSMCCDEFINE_HEADER_26D8OP_	
#define __H_DSMCCDEFINE_HEADER_26D8OP_
#include <map>
#include <TsSRM.h>

#pragma pack(1)

namespace ZQ {
	namespace DSMCC{
typedef std::map<std::string, std::string> StringMap;
typedef struct _dmsccResource
{
  StringMap resCommonHeader;
  TianShanIce::SRM::Resource resource;
}DsmccResource;
typedef std::vector<DsmccResource>DsmccResources;
enum DsmccMessageId
{
	MsgID_SetupRequest            = 0x4010,
	MsgID_SetupConfirm            = 0x4011,
	MsgID_ReleaseRequest          = 0x4020,
	MsgID_ReleaseConfirm          = 0x4021,
	MsgID_ReleaseIndication       = 0x4022,
	MsgID_ReleaseResponse         = 0x4023,
	MsgID_ProceedingIndication    = 0x4082,
	MsgID_InProgressRequest       = 0x40b0
};
enum ResponseCode
{
   RsnOK           = 0x0000,
   MotoReserved    = 0x0001,
   RspNeNoCalls    = 0x0002,
   RspNeNoSession  = 0x0005,
   RspSeNOCalls    = 0x0006,
   RspSeNoService  = 0x0008,
   RspSeNoSession  = 0x0010,
   RspNeNoResource = 0x0019,
   RspSeNoResource = 0x0020,
   RspNeProcError  = 0x0023,
   RspSeProcError  = 0x0024,
   RspSeNoResponse = 0x000B,
/// From 0x8001 - 8fff defined by 3rd servers
   RspUserAccountNotExist = 0x8001,
   RspMovieNotExist       = 0x8002,
   RspNoEnoughMoney       = 0x8003,
   RspExceptionError      = 0x800F,
   RspProgramInfoError    = 0x8050,
   RspProgramError        = 0x8051,
   RspProgramUpdateError  = 0x805F,
   RspSystemCongestion    = 0x8FFF,
   RspBillingUnavailable  = 0x9003,
   RspAssetNotFound       = 0x9100
};

enum ReasonCode
{
	RsnNormal           = 0x0001,
	RsnClProcError      = 0x0002,
	RsnNeProcError      = 0x0003,
	RsnSeProcError      = 0x0004,
	RsnSeSessionRelease = 0x001A,
	RsnNeSessionRelease = 0x001B,
	RspMoto151A         = 0x151A,
	RsnMoto9051         = 0x9051,
	RsnMoto9052         = 0x9052
};
enum ResourceType
{
	RsrcType_CONTINUOUS_FEED_SESSION    = 0x0001,
	RsrcType_ATM_CONNECTION             = 0x0002,
	RsrcType_MPEG_PROGRAM               = 0x0003,
	RsrcType_PHYSICAL_CHANNEL           = 0x0004,
	RsrcType_TS_UPSTREAM_BW             = 0x0005,
	RsrcType_TS_DOWNSTREAM_BW           = 0x0006,
	RsrcType_ATM_SVC_CONNECTION         = 0x0007,
	RsrcType_CONNECTION_NOTIFY          = 0x0008,
	RsrcType_IP                         = 0x0009,
	RsrcType_HEAD_END_LIST              = 0x000F,
	RsrcType_ATM_VC_CONNECTION          = 0x0010,
	RsrcType_SA_SERVER_CAS_2            = 0x8000,
	RsrcType_SA_CLIENT_CAS_2            = 0x8001,
	RsrcType_SA_ATSC_MODULATION_MODE    = 0xF001,
	// 0xF002 reserved
	RsrcType_SA_HEAD_END_ID             = 0xF003,
	RsrcType_SA_SERVER_CAS              = 0xF004,
	RsrcType_SA_CLIENT_CAS              = 0xF005,
	RsrcType_SA_ETHERNET_INTERFACE      = 0xF006,
	RsrcType_SEACHANGE_SERVER_CAS       = 0xF100,
	RsrcType_SEACHANGE_VIRTUAL_CHANNEL_NUMBER
	= 0xF101,
	RsrcType_SEACHANGE_SERVER_CAS_2     = 0xF102,
	RsrcType_SEACHANGE_PRE_ENCRYPTED    = 0xF103,
	RsrcType_SEACHANGE_DIGITAL_COPY_PROTECTION
	= 0xF104,
	RsrcType_SEACHANGE_IP_TRANSPORT     = 0xF105,
	/* CtM 8/11/04 these were added for the MTS project  */
	RsrcType_SEACHANGE_CPE_ID           = 0xF106,
	RsrcType_SEACHANGE_BP_ID            = 0xF107,
	RsrcType_SEACHANGE_IP_STREAM_SOURCE = 0xF108,

	/* mrk; ISA wants streamop conn desc in addresource */
	RsrcType_SEACHANGE_STREAMOP_DESC    = 0xf109,

	/* --------------------------------------------------*/
};

struct MpegProgramRsrc_t                // T=RsrcType_MPEG_PROGRAM, L=16 bytes
{
	uint16        pnType;
	uint16        pnValue;
	uint16        pmtPidType;
	uint16        pmtPidValue;
	uint16        caPidValue;
	uint16        elemStreamCountValue;
	uint16        pcrType;
	uint16        pcrValue;
};
struct PhysicalChannelRsrc_t            // T=RsrcType_PHYSICAL_CHANNEL, L=8 bytes
{
	uint16        channelType;
	uint32        channelValue;
	uint16        directionValue;
};
struct TransportStreamDownstreamBwRsrc_t// T=RsrcType_TS_DOWNSTREAM_BW, L=12 bytes
{
	uint16        bwType;
	uint32        bwValue;
	uint16        idType;
	uint32        idValue;
};

struct SaAtscModulationModeRsrc_t       // T=RsrcType_SA_ATSC_MODULATION_MODE, L=12 bytes
{
	uint8         transmissionSystem;
	uint8         innerCodingMode; // fixed to 0x0F
	uint8         splitBitStreamMode; // fixed to 0x00
	uint8         modulationFormat;
	uint32        symbolRate; // fixed to 0x004C4B40
	uint8         reserved;
	uint8         interleaveDepth; // 0x00-0x0F
	uint8         modulationMode;
	uint8         forwardErrorCorrection;
};
struct SaHeadEndIdRsrc_t                // T=RsrcType_SA_HEAD_END_ID, L=26 bytes
{
	uint16        headEndFlag;
	std::vector<uint8>          headEndId;
	uint32        tsid;
};

struct SaEthernetInterfaceRsrc_NoSource_t
{
	uint16        sourceUdpPortType;          // ResourceValueType_LIST_VALUE
	uint16        sourceUdpPortCount;         // = 1
	uint16		  sourceUdpPort;              // = 0

	uint16        sourceIpAddressType;        // ResourceValueType_LIST_VALUE
	uint16        sourceIpAddressCount;       // = 1
	uint32        sourceIpAddress;              // = 1

	uint16        sourceMacAddressType;       // ResourceValueType_LIST_VALUE
	uint16        sourceMacAddressCount;      // = 1
	uint8         sourceMacAddress[6];        // = 0

	uint16        destinationUdpPortType;     // ResourceValueType_LIST_VALUE
	uint16        destinationUdpPortCount;    // = 1
	uint16        destinationUdpPort;         // Pipe calculated port number

	uint16        destinationIpAddressType;   // ResourceValueType_LIST_VALUE
	uint16        destinationIpAddressCount;  // = 1
	uint32        destinationIpAddress;       // Pipe::IpTrnIp

	uint16        destinationMacAddressType;  // ResourceValueType_LIST_VALUE
	uint16        destinationMacAddressCount; // = 1
	uint8         destinationMacAddress[6];   // Pipe::IpTrnMac
};

#define pdProtocolId                 0xf1
#define pdProtocolVersion            0x01

#define pdAppRequestProtocolId       0x00
#define pdAppRequestProtocolVersion  0x00

#define pdAppResponseProtocolId      0x00
#define pdAppResponseProtocolVersion 0x00

#define pdFunctionProtocolId         0x01
#define pdFunctionVersion            0x01
#define LscpUDPMode                  "1"

#define ClientRequestMetaData(_MetaData) "$CR." #_MetaData
// uudata
#define CRMetaData_UUdatas               ClientRequestMetaData(UUdatas)         // bytes

/// DsmccMsg Header key
#define CRMetaData_protocolDiscriminator   ClientRequestMetaData(protocolDiscriminator)//uint8 1byte (option)
#define CRMetaData_dsmccType               ClientRequestMetaData(dsmccType) //uint8  1byte(option)
#define CRMetaData_messageId               ClientRequestMetaData(messageId) //uint16  2byte (must)
#define CRMetaData_transactionId           ClientRequestMetaData(transactionId)// uint32 4byte(option)
#define CRMetaData_reserved1               ClientRequestMetaData(reserved1)//uint8 1byte(option)
#define CRMetaData_adpationType            ClientRequestMetaData(adpationType) //uint8 1byte(option)
#define CRMetaData_adpationBytes           ClientRequestMetaData(adpationBytes) //bytes (option)

///define sessionID key
#define CRMetaData_SessionId            ClientRequestMetaData(SessionId) // 10byte

///privatedata  key
//   key (sessionsetuprequest)
#define CRMetaData_assetId              ClientRequestMetaData(assetId) //std::string 
#define CRMetaData_nodeGroupId          ClientRequestMetaData(nodeGroupId) //uint64 6byte
//   key (sessionsetupconfirm)
#define CRMetaData_Port                 ClientRequestMetaData(Port) //uint16 (must)
#define CRMetaData_IPaddress            ClientRequestMetaData(IPaddress) //uint32 2byte(must)
#define CRMetaData_StreamHandelId       ClientRequestMetaData(StreamHandelId) //uint32 2byte(must)
//   key (sessionsetuprequest(AppRequestData), sessionsetupconfirm(AppResponseData))
#define CRMetaData_billingId            ClientRequestMetaData(billingId)   //uint32 4byte
#define CRMetaData_purchaseTime         ClientRequestMetaData(purchaseTime)//uint32 4byte (must)
#define CRMetaData_remainingPlayTime    ClientRequestMetaData(remainingPlayTime)//uint32 4byte
#define CRMetaData_errorCode            ClientRequestMetaData(errorCode)//uint32 4byte
#define CRMetaData_homeId               ClientRequestMetaData(homeId)//uint32 4byte
#define CRMetaData_purchaseId           ClientRequestMetaData(purchaseId)//uint32 4byte
#define CRMetaData_smartCardId          ClientRequestMetaData(smartCardId)//uint32 4byte
#define CRMetaData_analogCopyPurchase   ClientRequestMetaData(analogCopyPurchase)//uint32 4byte
#define CRMetaData_packageId            ClientRequestMetaData(packageId)//uint32 4byte
//   key (sessionsetupconfirm(AppResponseData))
#define CRMetaData_heartbeat            ClientRequestMetaData(heartbeat)//uint32 4byte (must)
#define CRMetaData_lscpIpProtocol       ClientRequestMetaData(lscpIpProtocol)//uint32 4byte(must)
//   key (sessionsetupconfirm, Function)
#define CRMetaData_billingIdF           ClientRequestMetaData(billingIdF)   //uint32 4byte
#define CRMetaData_purchaseTimeF        ClientRequestMetaData(purchaseTimeF)//uint32 4byte (must)
#define CRMetaData_remainingPlayTimeF   ClientRequestMetaData(remainingPlayTimeF)//uint32 4byte
#define CRMetaData_errorCodeF           ClientRequestMetaData(errorCodeF)//uint32 4byte
#define CRMetaData_homeIdF              ClientRequestMetaData(homeIdF)//uint32 4byte
#define CRMetaData_purchaseIdF          ClientRequestMetaData(purchaseIdF)//uint32 4byte
#define CRMetaData_smartCardIdF         ClientRequestMetaData(smartCardIdF)//uint32 4byte
#define CRMetaData_analogCopyPurchaseF  ClientRequestMetaData(analogCopyPurchaseF)//uint32 4byte
#define CRMetaData_packageIdF           ClientRequestMetaData(packageIdF)//uint32 4byte

// resourceHeader sessionsetupconfirm
#define CRMetaData_RESresRequestId          ClientRequestMetaData(RESresRequestId)//uint16 2byte  (must)
#define CRMetaData_RESresDescriptorType     ClientRequestMetaData(RESresDescriptorType)//uint16 2byte(must)
#define CRMetaData_RESresNum                ClientRequestMetaData(RESresNum)//uint16 2byte(must)(must)
#define CRMetaData_RESassociationTag        ClientRequestMetaData(RESassociationTag)//uint16 2byte(must)
#define CRMetaData_RESresFlags              ClientRequestMetaData(RESresFlags)  //uint8 1byte (must)
#define CRMetaData_RESresStatus             ClientRequestMetaData(RESresStatus) //uint8 1byte (must)
#define CRMetaData_RESresLength             ClientRequestMetaData(RESresLength) //uint16 2byte(option)
#define CRMetaData_RESresDataFieldCount     ClientRequestMetaData(RESresDataFieldCount)//uint16 2byte(option)
#define CRMetaData_REStypeOwnerId           ClientRequestMetaData(REStypeOwnerId)//uint32 3byte(option)
#define CRMetaData_REStypeOwnerValue        ClientRequestMetaData(REStypeOwnerValue)//uint32 3byte(option)


///ClientSessionSetupRequest  key
//  miniBody
#define CRMetaData_CSSRreserved             ClientRequestMetaData(CSSRreserved)   // 2byte
#define CRMetaData_CSSRclientId             ClientRequestMetaData(CSSRclientId)   // 20byte
#define CRMetaData_CSSRserverId             ClientRequestMetaData(CSSRserverId)   // 20byte

//ClientSessionSetupConfirm key
//  miniBody
#define CRMetaData_CSSCresponse             ClientRequestMetaData(CSSCresponse)   // 2byte
#define CRMetaData_CSSCserverId             ClientRequestMetaData(CSSCserverId)   // 20byte

//ClientSessionRelease
//  miniBody
#define CRMetaData_CSRreason               ClientRequestMetaData(CSRreason)   // uint16

/// class ClientSessionProceedingIndication
//  miniBody
#define CRMetaData_CSPIreason             ClientRequestMetaData(CSPIreason)  // uint16

/// class ClientSessionInProgressRequest
#define CRMetaData_CSPRsessionCount         ClientRequestMetaData(CSPRsessionCount) // 2byte
#define CSPRSessionID_             "$SessionId_"

	}// end namespace DSMCC
}// end namespace ZQ
#pragma pack()
#endif //enddef __H_DSMCCDEFINE_HEADER_26D8OP_