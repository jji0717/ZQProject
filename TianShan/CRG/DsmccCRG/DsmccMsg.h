#ifndef __H_DSMCCMESSAGE_HEADER_255DFQOP_	
#define __H_DSMCCMESSAGE_HEADER_255DFQOP_

#include "Pointer.h"
#include <vector>
#include <string>
#include "DsmccDefine.h"
#include "Log.h"

#pragma pack(1)
namespace ZQ {
	namespace DSMCC{

/// class DsmccMsg
class DsmccMsg : public ZQ::common::SharedObject
{
public:
	typedef ZQ::common::Pointer < DsmccMsg > Ptr;
	typedef std::vector < uint8 > Bytes;
	typedef struct _HardHeader
	{
		uint8 protocolDiscriminator;
		uint8 dsmccType;
		uint16 messageId;
		uint32 transactionId;
		uint8  reserved1;
	} HardHeader;

	typedef struct _ResourceCommonHeader
	{
		uint16 resRequestId;
		uint16 resDescriptorType;
		uint16 resNum;
		uint16 associationTag;
		uint8  resFlags;
		uint8  resStatus;
		uint16 resLength;
		uint16 resDataFieldCount;
		uint32 typeOwnerId;
		uint32 typeOwnerValue;
	}ResourceCommonHeader;

	/// struct confused
	typedef struct _ResourceDescriptorValue
	{
		int32 resValueType;
		int32 resListcount;
		Bytes resDescriptorValues; 
	}ResourceDescriptorValue;

	typedef struct _ResourceDataField
	{
		Bytes       resDataValues;
		ResourceDescriptorValue  resDescriptorValue;
	}ResourceDataField;

	typedef struct _Resource
	{
		ResourceCommonHeader resCommonHeader;
		std::vector<ResourceDataField> resDataFields;
	}Resource;

public:
	virtual uint16 getMessageLength() { return _messageLength; }
	virtual uint16 formatMessageBody(uint8* buf, size_t maxLen) { return 0; }
	virtual bool parseMessageBody(const uint8* buf, size_t messageLength);

	int toMessage(uint8* buf, size_t maxLen);
	uint16 getMessageId();

	virtual uint32 toMetaData(StringMap& metadata);
	virtual bool readMetaData(const StringMap& metadata);
	virtual bool readResource(const DsmccResources dsmssResources){return true;};

	virtual void dumpLog();
	virtual void dumpPrivateData(){};

	static Ptr parseMessage(const uint8* buf, size_t maxLen, size_t& bytesProcessed, ProtocolType protocolType = Protocol_MOTO);
protected:
	bool parseUUData(const uint8* buf, size_t maxLen, size_t& bytesProcessed);
	bool parseUserPrivateData(const uint8* buf, size_t maxLen, size_t& bytesProcessed);
	bool parseResouce(const uint8* buf, size_t maxLen, size_t& bytesProcessed);

	bool formatUUData(const uint8* buf, size_t maxLen, size_t& processedlength);
	bool formatUserPrivateData(const uint8* buf, size_t maxLen, size_t& processedlength);
	bool formatResource(const uint8* buf, size_t maxLen, size_t& processedlength);
	
	bool readAdpationBytes(const uint8* buf, uint8 adaptionLength);

	uint32  toDsmccHeader(StringMap& metadata);
	bool readDmssHeader(const StringMap& metadata);

protected:
	DsmccMsg(HardHeader& hardHeader, ProtocolType protocolType);
    virtual ~DsmccMsg();
	HardHeader _hardHeader;
	uint8      _adpationType;
	Bytes      _adpationBytes;
	Bytes      _uuDataBytes;
	Bytes      _privateDataBytes;
	std::vector<Resource> _resources;

protected:
	uint16 _messageLength;
	ProtocolType _protocolType;
public:
	ZQ::common::DebugMsg _debugLog;
};

/// class ClientSessionSetupRequest
class ClientSessionSetupRequest : public DsmccMsg // 0x4010
{

public:
	ClientSessionSetupRequest(HardHeader& hardHeader, ProtocolType protocolType = Protocol_MOTO):DsmccMsg(hardHeader, protocolType){
		_assetId = "";  // private tag 0x01
		_nodeGroupId = 0;   // private tag 0x02,  6byte
		_billingId = 0;     // private tag 0x03
		_purchaseTime = 0;  // private tag 0x04
		_remainingPlayTime = 0;  // private tag 0x05
		_errorCode = 0;  // private tag 0x06
		_homeId = 0;  // private tag 0x07
		_purchaseId = 0;  // private tag 0x08
		_smartCardId = 0;  // private tag 0x09
		_analogCopyPurchase = 0;  // private tag 0x0d
		_packageId = 0;  // private tag 0x0f

		memset(&_minBody, 0 , sizeof(_minBody));
	};
	virtual ~ClientSessionSetupRequest(){};

public:
	virtual uint16 formatMessageBody(uint8* buf, size_t maxLen);
	virtual bool parseMessageBody(const uint8* buf, size_t messageLength);

	virtual uint32 toMetaData(StringMap& metadata);
	virtual bool readMetaData(const StringMap& metadata);
    
	virtual void dumpPrivateData();
protected:
	bool parsePrivateData();
	bool readPrivateAppData(uint32 element);

private:
    bool parseMotoPrivateData();
	bool parseTangbergPrivateData();
public:
	typedef struct _MinBody
	{
		uint8 sessionId[10];
		uint8 reserved[2];
		uint8 clientId[20];
		uint8 serverId[20];
	} MinBody;

	MinBody _minBody;
	std::string _assetId;  // private tag 0x01
	uint64 _nodeGroupId;   // private tag 0x02
	uint32 _billingId;     // private tag 0x03
	uint32 _purchaseTime;  // private tag 0x04
	uint32 _remainingPlayTime;  // private tag 0x05
	uint32 _errorCode;  // private tag 0x06
	uint32 _homeId;  // private tag 0x07
	uint32 _purchaseId;  // private tag 0x08
	uint32 _smartCardId;  // private tag 0x09
	uint32 _analogCopyPurchase;  // private tag 0x0d
	uint32 _packageId;  // private tag 0x0f

	//for Tangberg
	std::string _serviceGateway;
	std::string _service;
	uint16 _serviceGroup;
	uint32 _assetIdPayLoad;
	uint8  _ProtocolId;
	uint8  _ProtocolVersion;
};

/// class ClientSessionSetupConfirm
class ClientSessionSetupConfirm : public DsmccMsg // 0x4011
{

public:
	ClientSessionSetupConfirm(HardHeader& hardHeader, ProtocolType protocolType = Protocol_MOTO);
	virtual ~ClientSessionSetupConfirm(){};

public:
	virtual uint16 formatMessageBody(uint8* buf, size_t maxLen);
	virtual bool parseMessageBody(const uint8* buf, size_t messageLength);
	virtual uint32 toMetaData(StringMap& metadata);
	virtual bool readMetaData(const StringMap& metadata);
	virtual bool readResource(const DsmccResources dsmssResources);
	virtual void dumpPrivateData();
protected:
	bool parsePrivateData();
	bool readPrivateAppData(uint32 element);

private:
	bool parseMotoPrivateData();
	bool parseTangbergPrivateData();
public:
	typedef struct _MinBody
	{
		uint8  sessionId[10];
		uint16 response;
		uint8  serverId[20];
	} MinBody;

	MinBody _minBody;

	uint16 _IpPort;     // private tag 0x03
    uint32 _IpAddress;
	uint32 _streamHandel; //private tag 0x04 

//	Function_Descriptor 
//    descriptor_tag    1byte 0x80
//	  descriptor_length 1byte
//	  protocolID        1byte
//	  version           1byte
//   FunctionDescriptorCount 1byte;

	uint32 _billingIdF;     // private tag 0x03
	uint32 _purchaseTimeF;  // private tag 0x04
	uint32 _remainingPlayTimeF;  // private tag 0x05
	uint32 _errorCodeF;  // private tag 0x06
	uint32 _homeIdF;  // private tag 0x07
	uint32 _purchaseIdF;  // private tag 0x08
	uint32 _smartCardIdF;  // private tag 0x09
	uint32 _analogCopyPurchaseF;  // private tag 0x0d
	uint32 _packageIdF;  // private tag 0x0f

//	AppResponseData_Descriptor 
//    descriptor_tag    1byte 0x06
//	  descriptor_length 1byte
//	  protocolID        1byte
//	  version           1byte
 //   AppResDescriptorCount 1byte;
	uint32 _billingId;     // private tag 0x03
	uint32 _purchaseTime;  // private tag 0x04
	uint32 _remainingPlayTime;  // private tag 0x05
	uint32 _errorCode;  // private tag 0x06
	uint32 _homeId;  // private tag 0x07
	uint32 _purchaseId;  // private tag 0x08
	uint32 _smartCardId;  // private tag 0x09
	uint32 _analogCopyPurchase;  // private tag 0x0d
	uint32 _packageId;  // private tag 0x0f
    uint32 _heartbeat;  // private tag 0x80
	uint32 _lscpIpProtocol;//private tag 0x81

	std::string _serviceGateway; //16byte
	std::string _service;        //16byte
	uint32      _offeringId;
	uint8       _responseType; // 0x01, 0x02, 0x05;
	uint8  _ProtocolId;
	uint8  _ProtocolVersion;
};


/// class ClientSessionRelease: base class client session release
class ClientSessionRelease : public DsmccMsg // 0x4020,0x4021,0x4022,0x4023
{

public:
	ClientSessionRelease(HardHeader& hardHeader, ProtocolType protocolType = Protocol_MOTO):DsmccMsg(hardHeader, protocolType){
		memset(&_minBody, 0 , sizeof(_minBody));
	};
	virtual ~ClientSessionRelease(){};

public:
	virtual uint16 formatMessageBody(uint8* buf, size_t maxLen);
	virtual bool parseMessageBody(const uint8* buf, size_t messageLength);

	virtual uint32 toMetaData(StringMap& metadata);
	virtual bool   readMetaData(const StringMap& metadata);
	virtual void dumpPrivateData();

	virtual bool parsePrivateData(){ return true;};
public:
	typedef struct _MinBody
	{
		uint8 sessionId[10];
		uint16 reason;
	} MinBody;

	MinBody _minBody;
};

/// class ClientSessionReleaseRequest
class ClientSessionReleaseRequest : public ClientSessionRelease //0x4020
{
public:
	ClientSessionReleaseRequest(HardHeader& hardHeader, ProtocolType protocolType = Protocol_MOTO):ClientSessionRelease(hardHeader, protocolType){};
	virtual ~ClientSessionReleaseRequest(){};

//	virtual uint32 toMetaData(StringMap& metadata);
//	virtual bool readMetaData(const StringMap& metadata);
public:
	virtual bool parsePrivateData();
public:
   // private data define
};

/// class ClientSessionReleaseConfirm
class ClientSessionReleaseConfirm : public ClientSessionRelease //0x4021
{
public:
	ClientSessionReleaseConfirm(HardHeader& hardHeader, ProtocolType protocolType = Protocol_MOTO):ClientSessionRelease(hardHeader, protocolType){};
	virtual ~ClientSessionReleaseConfirm(){};

//	virtual uint32 toMetaData(StringMap& metadata);
//	virtual bool readMetaData(const StringMap& metadata);
public:
	virtual bool parsePrivateData();
public:

};

/// class ClientSessionReleaseIndication
class ClientSessionReleaseIndication : public ClientSessionRelease// 0x4022
{
public:
	ClientSessionReleaseIndication(HardHeader& hardHeader, ProtocolType protocolType = Protocol_MOTO):ClientSessionRelease(hardHeader, protocolType){};
	virtual ~ClientSessionReleaseIndication(){};

//	virtual uint32 toMetaData(StringMap& metadata);
//	virtual bool readMetaData(const StringMap& metadata);
public:
	virtual bool parsePrivateData();
public:

};

/// class ClientSessionReleaseResponse
class ClientSessionReleaseResponse : public ClientSessionRelease //0x4023
{
public:
	ClientSessionReleaseResponse(HardHeader& hardHeader, ProtocolType protocolType = Protocol_MOTO):ClientSessionRelease(hardHeader, protocolType){};
	virtual ~ClientSessionReleaseResponse(){};

//	virtual uint32 toMetaData(StringMap& metadata);
//	virtual bool readMetaData(const StringMap& metadata);
public:
	virtual bool parsePrivateData();
public:

};

/// class ClientSessionProceedingIndication
class ClientSessionProceedingIndication : public DsmccMsg // 0x4082
{

public:
	ClientSessionProceedingIndication(HardHeader& hardHeader, ProtocolType protocolType = Protocol_MOTO):DsmccMsg(hardHeader, protocolType){
	memset(&_minBody, 0 , sizeof(_minBody));
	};
	virtual ~ClientSessionProceedingIndication(){};

	virtual uint32 toMetaData(StringMap& metadata);
	virtual bool readMetaData(const StringMap& metadata);
	virtual void  dumpPrivateData();
public:
	virtual uint16 formatMessageBody(uint8* buf, size_t maxLen);
	virtual bool parseMessageBody(const uint8* buf, size_t messageLength);
public:
	typedef struct _MinBody
	{
		uint8 sessionId[10];
		uint16 reason;
	} MinBody;

	MinBody _minBody;
};

/// class ClientSessionInProgressRequest
class ClientSessionInProgressRequest : public DsmccMsg // 0x40b0
{

public:
	ClientSessionInProgressRequest(HardHeader& hardHeader, ProtocolType protocolType = Protocol_MOTO):DsmccMsg(hardHeader, protocolType){
		sessionIds.clear();
	};
	virtual ~ClientSessionInProgressRequest(){};

	virtual uint32 toMetaData(StringMap& metadata);
	virtual bool readMetaData(const StringMap& metadata);
	virtual void  dumpPrivateData();
public:
	virtual uint16 formatMessageBody(uint8* buf, size_t maxLen);
	virtual bool parseMessageBody(const uint8* buf, size_t messageLength);
public:
	std::vector<Bytes> sessionIds;	
};

}// end namespace DSMCC
}// end namespace ZQ
#pragma pack()
#endif //enddef __H_DSMCCMESSAGE_HEADER_255DFQOP_
