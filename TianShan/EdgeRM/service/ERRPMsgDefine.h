#ifndef __H_ERRPMSGDEFINE_HEADER_26D41Y_	
#define __H_ERRPMSGDEFINE_HEADER_26D41Y_
#include <map>
#pragma pack(1)

namespace ZQ {
	namespace ERRP{

		typedef std::map<std::string, std::string> StringMap;

		enum ERRPMSGTYPE
		{
			MsgType_OPEN            = 0x1,
			MsgType_UPDATE          = 0x2,
			MsgType_NOTIFICATION    = 0x3,
			MsgType_KEEPALIVE       = 0x4
		};

		enum ParameterType
		{
			CapabilityInfomation	= 0x1,
			StreamingZone			= 0x2,
			ComponentName			= 0x3,
			VendorSpecificString	= 0x4
		};

		enum CapabilityCode
		{
			RouteTypeSupported = 1,
			SendReceive        = 2,
			ERRPVersion        = 32768
		};

		enum ApplicationProtocol
		{
			StaticPortMapping          = 32766,
			SessionParameter           = 32768,
			SessionParameterConfig     = 32770
		};

		enum QAMModulationMode
		{
			QAM64   = 3,
			QAM256  = 4,
			QAM128  = 5,
			QAM512  = 6,
			QAM1024 = 7
		};

		enum RoutAttriTypeCode
		{
			Code_WithDrawRouts			 = 1,
			Code_ReachableRoutes		 = 2,
			Code_NextHopServer           = 3,
			Code_QamNames		         = 232,
			Code_CasCapability		     = 233,
			Code_TotalBW		         = 234,
			Code_AvailableBW		     = 235,
			Code_Cost		             = 236,
			Code_EdgeInputs		         = 237,
			Code_QamChConfig		     = 238,
			Code_UdpMap				     = 239,
			Code_ServiceStatus		     = 241,
			Code_MaxMPEGFlows		     = 242,
			Code_NextHopServerAlternate	 = 243,
			Code_PortID					 = 244,
			Code_FiberNodeNames		     = 245,
			Code_QamCapability		     = 247,
			Code_InputMaps				 = 249
		};

#define ERRPMetaData(_MetaData) "$ERRP." #_MetaData

///////////////////head key/////////////////////////
#define ERRPMD_MsgType		          ERRPMetaData(MsgType) 

////////////////// OpenRequest key/////////////////
#define ERRPMD_OpenVersion				     ERRPMetaData(OpenVersion)       //uint8  1byte 
#define ERRPMD_OpenReserved					 ERRPMetaData(OpenReserved)      //uint8  1byte 
#define ERRPMD_OpenHoldTime					 ERRPMetaData(OpenHoldTime)      //uint16 2byte 
#define ERRPMD_OpenAdddressDomain			 ERRPMetaData(OpenAdddressDomain)//uint32 4byte 
#define ERRPMD_OpenErrpIdentifier			 ERRPMetaData(OpenErrpIdentifier)//uint32 4byte 

#define ERRPMD_OpenStreamingZone			 ERRPMetaData(OpenStreamingZone) //bytes
#define ERRPMD_OpencomponetName				 ERRPMetaData(OpencomponetName)  //bytes 
#define ERRPMD_OpenVendorSString			 ERRPMetaData(OpenVendorSString )//bytes
#define ERRPMD_OpenCapabilityCode			 ERRPMetaData(OpenCapabilityCode)//uint16 2byte 
#define ERRPMD_OpenAddressFamiliy			 ERRPMetaData(OpenAddressFamiliy)//uint16 2byte 
#define ERRPMD_OpenAppProtocol				 ERRPMetaData(OpenAppProtocol)   //uint16 2byte 
#define ERRPMD_OpenSendReceivedMode			 ERRPMetaData(OpenSendReceivedMode)  //uint16 2byte
#define ERRPMD_OpenErrpVersion				 ERRPMetaData(OpenErrpVersion)   //uint32 4byte

/// Notification key
#define ERRPMD_NoErrorCode					 ERRPMetaData(NoErrorCode)       //uint8  1byte 
#define ERRPMD_NoErrorSubCode				 ERRPMetaData(NoErrorSubCode)    //uint8  1byte 
#define ERRPMD_NoErrorData					 ERRPMetaData(NoErrorData)       //bytes   

	}// end namespace ERRP
}// end namespace ZQ
#pragma pack()
#endif //enddef __H_ERRPMSGDEFINE_HEADER_26D41Y_
