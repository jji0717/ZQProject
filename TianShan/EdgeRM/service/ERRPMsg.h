#ifndef __ERRPMSG_HEADER_FILE_H__
#define __ERRPMSG_HEADER_FILE_H__

#include <Pointer.h>
#include <vector>
#include <string>
#include <map>
#include "ERRPMsgDefine.h"
#include "Log.h"
#include "TianShanDefines.h"

#pragma pack(1)
namespace ZQ {
	namespace ERRP {

		/// class ERRPMsg
		class ERRPMsg : public ZQ::common::SharedObject
		{
		public:
			typedef ZQ::common::Pointer < ERRPMsg > Ptr;
			typedef std::vector < uint8 > Bytes;

			typedef struct _HardHeader
			{
				uint16 Length;
				uint8  msgType;
			} HardHeader;

		public:
			virtual uint16 getMessageLength() { return _messageLength;}
			virtual uint16 formatMessageBody(uint8* buf, size_t maxLen) { return 0; }
			virtual bool parseMessageBody(const uint8* buf, size_t messageLength);

			int toMessage(uint8* buf, size_t maxLen);
			uint8 getMessageType();

			virtual uint32 toMetaData(StringMap& metadata);
			virtual bool readMetaData(const StringMap& metadata);

			static Ptr parseMessage(const uint8* buf, size_t maxLen, size_t& bytesProcessed);

		protected:
			uint32  toERRPHeader(StringMap& metadata);//get header metadata
			bool    readERRPHeader(const StringMap& metadata);
		public:
			virtual void dumpLog();
			virtual void dumpBody(){};

		protected:
			ERRPMsg(HardHeader& hardHeader);
			virtual ~ERRPMsg();
			HardHeader _hardHeader;
		protected:
			uint16 _messageLength;//except header
		public:
			ZQ::common::DebugMsg _log;
		};

		/// class OpenRequest
		class OpenRequest : public ERRPMsg // msgType =0x1
		{

		public:
			OpenRequest(HardHeader& hardHeader):ERRPMsg(hardHeader){
				memset(&_minBody, 0 , sizeof(_minBody));
			};
			virtual ~OpenRequest(){};

		public:
			virtual uint16 formatMessageBody(uint8* buf, size_t maxLen);
			virtual bool parseMessageBody(const uint8* buf, size_t messageLength);

			virtual uint32 toMetaData(StringMap& metadata);
			virtual bool readMetaData(const StringMap& metadata);

		private:
			uint16 formatCapabilityInfo(uint16 parameterType,uint8* parameterBuf);
			bool   parseCapabilityInfo(uint16 parameterType,const uint8* buf,size_t& byteProcessed);
		public:
			void dumpBody();
		public:
			typedef struct _MinBody
			{
				uint8   version;               //maps to metadata OpenRequest.version
				uint8   Reserved;
				uint16  holdTime;              //maps to metadata OpenRequest.holdTime
				uint32  adddressDomain;
				uint32  errpIdentifier;
				uint16  parametersLength;
			} MinBody;

			MinBody _minBody;

			//optional parameters,if parametersLength != 0
			std::string streamingZone; ///parameter type = 2
			uint16 streamingZoneLength;
			std::string componentName;///parameter type = 3
			uint16 componentNameLength;
			std::string vendorSpecificString;///parameter type = 4
			uint16 vendorSpecificeStringLength;

			//capability information
			//uint16 capabilityCode;
			typedef std::vector< uint16 > CapabilityCodes;//change uint16 to vector
			CapabilityCodes capabilityCodes;
            
			//if capabilityCode = 1
			uint16  addressFamiliy;
			uint16  applicationProtocol;

			//if capabilityCode = 2
            //uint16  sendReceivedMode;
			uint32 sendReceivedMode;//change uint16 to uint32

			//if capabilityCode = 32768
			uint32  errpVersion;
			
		};

		/// class UpdataRequest
		class UpdataRequest : public ERRPMsg // msgType = 0x2
		{

		public:
			UpdataRequest(HardHeader& hardHeader):ERRPMsg(hardHeader){

			};
			virtual ~UpdataRequest(){};

		public:
			virtual uint16 formatMessageBody(uint8* buf, size_t maxLen);
			virtual bool parseMessageBody(const uint8* buf, size_t messageLength);

			virtual uint32 toMetaData(StringMap& metadata);
			virtual bool readMetaData(const StringMap& metadata);
		public:
			void dumpBody();

		public:
			typedef struct _Route
			{
              uint16 addressFamily;
			  uint16 applicationProtocol;
			  uint16 length;
			  Bytes  address;
			} Route;
			typedef std::vector<_Route>Routes;
            
			typedef struct _NextHopServer
			{
			  uint32 nextHopAddressDomain;
			  uint16 componentAddrLength;
			  Bytes  componentAddr;
			  uint16 streamingZoneNameLength;
			  Bytes  streamingZoneName;
			}NextHopServer;

			typedef struct _QAMCapability
			{
              uint16 channelBandwidth;
			  uint16 J83;
			  uint32 interLeaver;
			  uint32 capabilities;
			  uint16 modulation;
			}QAMCapability;

			typedef struct _QAMChannelConfig
			{
              uint32 frequency;//HZ
			  uint8  modMode;
			  uint8  interleaver;
			  uint16 TSID;
			  uint8  annex;
			  uint8  channelWidth;
			  uint16 reserved;
			}QAMChannelConfig;

			typedef struct _CASCapability
			{
              uint8  encType;
			  uint8  encScheme;
			  uint16 keyLength;
			  Bytes  casIdentifier;
			}CASCapability;

			typedef struct _EdgeInput
			{
              uint32 subMask;
			  uint16 hostLength;
			  Bytes  host;
			  uint32 interfaceID;
			  uint32 maxBandwidth;
			  uint16 groupNameLength;
              Bytes  groupName;
			}EdgeInput;
			typedef std::vector < EdgeInput > EdgeInputs;
 
			typedef struct _StaticPortRange
			{
				uint16 startPort;
				uint16 startMpegProgram;
				uint32 count;
			}StaticPortRange;
			typedef std::vector < StaticPortRange > StaticPortRanges;
			typedef struct _UDPMap
			{
			   std::map < uint16, uint16 >staticPorts;
			   StaticPortRanges staticPortRanges;
			   std::map < uint16, uint16 >dynamicPorts;
			}UDPMap;

			typedef struct _typeCode
			{
				uint8				attributeTypeFlags; //default value = 0;
				uint8				attributeTypeCode;
				uint16				attributeLength;
			}TypeCode;

			typedef std::vector<TypeCode>TypeCodes;

			typedef struct _AttrbuteValue
			{
                TypeCodes           typecodes;
				Routes				withdrawRoutes;				/// attributeTypeCode = 1
				Routes				reachableRoutes;			/// attributeTypeCode = 2 
				NextHopServer		nextHopServer;				/// attributeTypeCode = 3	
				Bytes				qamNames;					/// attributeTypeCode = 232
				CASCapability		casCapability;				/// attributeTypeCode = 233
				uint32				totalBandwidth;//Kbps		/// attributeTypeCode = 234
				uint32				availableBandwidth;//Kbps	/// attributeTypeCode = 235
				uint8				cost;						/// attributeTypeCode = 236
				EdgeInputs			edgeInputs;					/// attributeTypeCode = 237
				QAMChannelConfig	qamChConfig;				/// attributeTypeCode = 238
				UDPMap              udpMap;						/// attributeTypeCode = 239
				uint32				serviceStatus;				/// attributeTypeCode = 241
				uint32              maxMPEGFlows;				/// attributeTypeCode = 242
				std::vector < Bytes >nextHopServerAlternate;     /// attributeTypeCode = 243
				uint32				portID;						/// attributeTypeCode = 244
				std::vector < Bytes >fiberNodeNames;				/// attributeTypeCode = 245
				QAMCapability		qamCapability;			    /// attributeTypeCode = 247	
				std::vector < Bytes >inputMaps;				    /// attributeTypeCode = 249
			}AttrbuteValue;
		protected:
	        AttrbuteValue _attributeValue;
			
		public:
		public:
			void setRoute(uint8 typeflags, RoutAttriTypeCode typeCode, AttrbuteValue& attr);
			void setRoutes(AttrbuteValue& attr);
			
			AttrbuteValue& getRouteAttributes(){ return _attributeValue; }
		};

		/// class KeepAliveRequest
		class KeepAliveRequest : public ERRPMsg //  msgType = 0x4
		{

		public:
			KeepAliveRequest(HardHeader& hardHeader):ERRPMsg(hardHeader){};

			virtual ~KeepAliveRequest(){};

		public:
			virtual uint16 formatMessageBody(uint8* buf, size_t maxLen);
			virtual bool parseMessageBody(const uint8* buf, size_t messageLength);

			virtual uint32 toMetaData(StringMap& metadata);
			virtual bool readMetaData(const StringMap& metadata);
		public:
			void dumpBody();
		};

		/// class Notification
		class Notification : public ERRPMsg //  msgType = 0x3
		{

		public:
			Notification(HardHeader& hardHeader):ERRPMsg(hardHeader)
			{	
				memset(&_minBody, 0 , sizeof(_minBody));
			};

			virtual ~Notification() {};

		public:
			virtual uint16 formatMessageBody(uint8* buf, size_t maxLen);
			virtual bool parseMessageBody(const uint8* buf, size_t messageLength);

			virtual uint32 toMetaData(StringMap& metadata);
			virtual bool readMetaData(const StringMap& metadata);
		public:
			void dumpBody();

		public:
			typedef struct _MinBody
			{
				uint8   errorCode;
				uint8   errorSubCode;
			} MinBody;

			MinBody _minBody;
			Bytes   _errorData;
		};

}/// end namespace ERRP
}/// end namespace ZQ
#pragma pack()
#endif // __ERRPMSG_HEADER_FILE_H__
