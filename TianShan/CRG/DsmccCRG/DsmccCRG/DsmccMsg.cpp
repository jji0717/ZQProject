#include "DsmccMsg.h"
#include "TianShanDefines.h"
namespace ZQ {
	namespace DSMCC{

//////////////////////////////////////////////////////////////////////
///////////////  class DsmccMsg       ////////////////////////////////
/////////////// //////////////////////////////////////////////////////
bool reversalBytes(uint8 buf[], uint16 len)
{
	if(sizeof(buf) < 1 || len < 1)
		return false;
	for(uint16 i = 0, j = len - 1; i < len /2; i++,j--)
	{
		uint8 temp;
		temp = buf[i];
		buf[i] =  buf[j];
		buf[j] = temp; 
	}
	return true;
}
bool HexToString(std::string& outputString , const uint8*buf, uint16 len)
{
	if(NULL == buf || len < 1)
		return false;
	char temp[4096] = "";
	for(uint16 i = 0 ; i < len; i++)
	{
		//	itoa(*(buf + i), temp + i * 2, 16);
		sprintf(temp + i * 2, "%02x",*(buf + i));
	}	
	outputString = (temp);
	return true;
}
bool HexToString(std::string& outputString , std::vector<uint8>buf)
{
	if(buf.size() < 1)
		return false;
	char temp[4096] = "";
	for(uint16 i = 0 ; i < buf.size(); i++)
	{
		//itoa(*(buf + i), temp + i * 2, 16);
		sprintf(temp + i * 2, "%02x", buf[i]);
	}	
	outputString = (temp);
	return true;
}
bool StringToHex(const std::string& outputString , uint8*buf, uint16 len)
{
	if(NULL == buf || outputString.size() /2  > len )
		return false;

	for(uint16 i = 0 ; i < outputString.size()/2; i++)
	{
		char bufTemp[3]="";
		bufTemp[0] = outputString[i*2];
		bufTemp[1] = outputString[i*2 +1];
		uint32 temp = 0;
		sscanf(bufTemp, "%02x",&temp);
		*buf = temp;
		buf++;
	}	
	return true;
}
bool StringToHex(const std::string& outputString, std::vector<uint8>&buf)
{	
	uint32 tempHex = 0;
	for(uint16 i = 0 ; i < outputString.size()/2; i++)
	{
		char bufTemp[3]="";
		bufTemp[0] = outputString[i*2];
		bufTemp[1] = outputString[i*2 +1];
		sscanf(bufTemp, "%02x",&tempHex);
		buf.push_back(tempHex);
	}	
	return true;
}
uint64 ntohll(uint64 netlonglong)
{
	uint32 lowData = (uint32)(netlonglong & 0xffffffff);
	uint32 highData = (uint32)(netlonglong >> 32);
	lowData = ntohl(lowData);
	highData = ntohl(highData);

	uint64 retLongLong = lowData;
	retLongLong = retLongLong << 32;
	retLongLong = retLongLong | highData;
	return retLongLong;
}
uint64 htonll(uint64 hostlonglong)
{
	uint32 lowData = (uint32)(hostlonglong & 0xffffffff);
	uint32 highData = (uint32)(hostlonglong >> 32);
	lowData = htonl(lowData);
	highData = htonl(highData);

	uint64 retLongLong = lowData;
	retLongLong = retLongLong << 32;
	retLongLong = retLongLong | highData;
	return retLongLong;
}
DsmccMsg::DsmccMsg(HardHeader& hardHeader)
 : _hardHeader(hardHeader),_adpationType(0),_messageLength(0)
{
	_debugLog.setVerbosity(ZQ::common::Log::L_DEBUG);
}
DsmccMsg::~DsmccMsg()
{

}
int DsmccMsg::toMessage(uint8* buf, size_t maxLen)
{
	    dumpLog();
        size_t processedLength = 0;
		uint8* p = buf;
		uint8 adaptionLength = (uint8)_adpationBytes.size();
		uint16 messageLength = getMessageLength();
		if (NULL ==p || maxLen < sizeof(_hardHeader) + 3 + adaptionLength + messageLength)
			return 0;

		HardHeader tmpHardHeader = _hardHeader;
		tmpHardHeader.messageId = htons(_hardHeader.messageId);
		tmpHardHeader.transactionId = htonl(_hardHeader.transactionId);
		// the hard header
		memcpy(p, &tmpHardHeader, sizeof(tmpHardHeader)); p += sizeof(tmpHardHeader);
		processedLength += sizeof(tmpHardHeader);

		// the adaptionLength
		if (adaptionLength >0)
			adaptionLength++;

		memcpy(p, &adaptionLength, sizeof(adaptionLength)); p += sizeof(adaptionLength);
        processedLength += sizeof(adaptionLength);

		// the messageLength
		uint16 tmpMessageLength = htons(messageLength);
		memcpy(p, &tmpMessageLength, sizeof(tmpMessageLength)); p += sizeof(tmpMessageLength);
        processedLength += sizeof(tmpMessageLength);

		// the _adpationBytes
		if (adaptionLength >0)
		{
			memcpy(p, &_adpationType, sizeof(_adpationType)); p += sizeof(_adpationType);
			for (uint i =0; i < (uint8)(adaptionLength-1); i++)
			{
				memcpy(p, &_adpationBytes[i], sizeof(_adpationBytes[i]));
				p += sizeof(_adpationBytes[i]);
				processedLength += sizeof(_adpationBytes[i]);
			}
		}
		uint16 messageSize = 0; 
		if(messageLength > 0)
			messageSize = formatMessageBody(p, maxLen - processedLength);
		int length = messageLength + (p - buf);
		return length;
}
	
uint16 DsmccMsg::getMessageId() 
{ 
	return _hardHeader.messageId;
}
	
bool DsmccMsg::parseMessageBody(const uint8* p, size_t messageLength)
{
	return true;
}
bool DsmccMsg::readAdpationBytes(const uint8* buf, uint8 adaptionLength)
{
	const uint8* p = buf;
	if(adaptionLength > 0)
	{
		_adpationType = *((uint8*) p); p += sizeof(uint8);
		for(uint8 i = 0; i < adaptionLength -1; i++)
		{
			_adpationBytes.push_back(*p); p += sizeof(uint8);
		}
	}
	return true;
}
DsmccMsg::Ptr DsmccMsg::parseMessage(const uint8* buf, size_t maxLen, size_t& bytesProcessed)
{
	Ptr pMsg = NULL;
	const uint8* p = buf;
	bytesProcessed =0;
	HardHeader tmpHeader;
	if (NULL ==p || maxLen < sizeof(tmpHeader) + 3)
		return pMsg;

	memcpy(&tmpHeader, p, sizeof(tmpHeader)); p += sizeof(tmpHeader);

	tmpHeader.messageId = ntohs(tmpHeader.messageId);
	tmpHeader.transactionId = ntohl(tmpHeader.transactionId);

	uint8 adaptionLength = *((uint8*) p); p += sizeof(uint8);
	uint16 messageLength = *((uint16*) p); p += sizeof(uint16);
	messageLength = ntohs(messageLength);

	if (maxLen < (sizeof(tmpHeader)) + 3 + adaptionLength + messageLength)
		return pMsg;

	switch(tmpHeader.messageId)
	{
	case MsgID_SetupRequest:
		pMsg = new ClientSessionSetupRequest(tmpHeader);
		break;
	case MsgID_SetupConfirm:
		pMsg = new ClientSessionSetupConfirm(tmpHeader);
		break;
	case MsgID_ReleaseRequest:                //ClientSessionReleaseRequest
		pMsg = new ClientSessionReleaseRequest(tmpHeader);
		break;
	case MsgID_ReleaseConfirm:    
		pMsg = new ClientSessionReleaseConfirm(tmpHeader);
		break;
	case MsgID_ReleaseIndication:    
		pMsg = new ClientSessionReleaseIndication(tmpHeader);
		break;
	case MsgID_ReleaseResponse:   
		pMsg = new ClientSessionReleaseResponse(tmpHeader);
		break;
	case MsgID_ProceedingIndication: 
		pMsg = new ClientSessionProceedingIndication(tmpHeader);
		break;
	case MsgID_InProgressRequest: 
		pMsg = new ClientSessionInProgressRequest(tmpHeader);
		break;
	case 0x0000:
	default:
		pMsg = new DsmccMsg(tmpHeader);
	}

	if (pMsg)
	{
		///readAdpationBytes
		pMsg->readAdpationBytes(p, adaptionLength); p +=adaptionLength;

		///parse messageBody
		if (!pMsg->parseMessageBody(p, messageLength))
			return NULL;
		p += messageLength;
	}

	pMsg->dumpLog();
	bytesProcessed = p - buf;
	return pMsg;
}
bool DsmccMsg::parseUUData(const uint8* buf, size_t maxLen, size_t& bytesProcessed)
{
	const uint8* p = buf;
	if (NULL == p)
		return false;

	bytesProcessed = 0;

	if(maxLen < sizeof(uint16))
		return false;

	uint16 uuDataLength =*((uint16*)p); p += sizeof(uint16); bytesProcessed += sizeof(uint16);
	uuDataLength = ntohs(uuDataLength);

	if(uuDataLength > 0)
	{
		if(maxLen < bytesProcessed + uuDataLength)
			return false;

		for(uint16 i = 0; i < uuDataLength; i++)
		{
			_uuDataBytes.push_back(*p);
			p += sizeof(uint8);
		}
	}
	bytesProcessed += uuDataLength;
	return true;
}
bool DsmccMsg::parseUserPrivateData(const uint8* buf, size_t maxLen, size_t& bytesProcessed)
{
	const uint8* p = buf;
	if (NULL == p)
		return false;

	bytesProcessed = 0;

	if(maxLen < sizeof(uint16))
		return false;

	uint16 privateDataLength =*((uint16*)p); p += sizeof(uint16); bytesProcessed += sizeof(uint16);
	privateDataLength = ntohs(privateDataLength);

	if(privateDataLength > 0)
	{
		if(maxLen <  bytesProcessed + privateDataLength)
			return false;
		for(uint16 i = 0; i < privateDataLength; i++)
		{
			_privateDataBytes.push_back(*p);
			p += sizeof(uint8);
		}
	}

	bytesProcessed += privateDataLength;
	return true;
}
bool DsmccMsg::parseResouce(const uint8* buf, size_t maxLen, size_t& bytesProcessed)
{
	const uint8* p = buf;
	if (NULL == p)
		return false;

	bytesProcessed = 0;

	if(maxLen < sizeof(uint16))
		return false;

	uint16 resDescriptorCount =*((uint16*)p); p += sizeof(uint16); bytesProcessed += sizeof(uint16);
	resDescriptorCount = ntohs(resDescriptorCount);

	_resources.clear();
	for(uint16 i  = 0 ;i < resDescriptorCount; i++)
	{
		Resource resource;

		//parse reource common header
		if(maxLen < bytesProcessed + sizeof(resource.resCommonHeader) - sizeof(uint32) *2)
			return false;
		memcpy(&resource.resCommonHeader, p, sizeof(resource.resCommonHeader) - sizeof(uint32) *2); p+= (sizeof(resource.resCommonHeader) - sizeof(uint32) *2);
        bytesProcessed += (sizeof(resource.resCommonHeader) - sizeof(uint32) *2);

		resource.resCommonHeader.resRequestId =  ntohs(resource.resCommonHeader.resRequestId);
		resource.resCommonHeader.resDescriptorType=  ntohs(resource.resCommonHeader.resDescriptorType);
		resource.resCommonHeader.resNum=  ntohs(resource.resCommonHeader.resNum);
		resource.resCommonHeader.associationTag=  ntohs(resource.resCommonHeader.associationTag);
		resource.resCommonHeader.resLength=  ntohs(resource.resCommonHeader.resLength);
		resource.resCommonHeader.resDataFieldCount=  ntohs(resource.resCommonHeader.resDataFieldCount);

		if(resource.resCommonHeader.resDescriptorType == 0xffff)
		{
			uint8 buf[4];
			memset(buf, 0, sizeof(buf));
			memcpy(buf + 1, p, 3); p += 3; bytesProcessed += 3;
			memcpy(&resource.resCommonHeader.typeOwnerId, buf, sizeof(resource.resCommonHeader.typeOwnerId));
			resource.resCommonHeader.typeOwnerId = ntohl(resource.resCommonHeader.typeOwnerId);

			memset(buf, 0, sizeof(buf));
			memcpy(buf + 1, p, 3); p += 3; bytesProcessed += 3;
			memcpy(&resource.resCommonHeader.typeOwnerValue, buf, sizeof(resource.resCommonHeader.typeOwnerValue));
			resource.resCommonHeader.typeOwnerValue = ntohl(resource.resCommonHeader.typeOwnerValue);
		}
		ResourceDataField resDataField;
		if(maxLen < bytesProcessed + resource.resCommonHeader.resLength)
			return false;
		for(uint16 k = 0 ; k < resource.resCommonHeader.resLength; k++)
		{
			resDataField.resDataValues.push_back(*p); 
			p += sizeof(resDataField.resDataValues[k]); bytesProcessed+= sizeof(resDataField.resDataValues[k]);
		}

		/*		for(uint16 j = 0; j < resource.resCommonHeader.resDataFieldCount; j++)
		{
		ResourceDataField resDataField;
		///Variable is a yes or no field which defines if a data field uses the dsmccResourceDescriptorValue format of the field
		///value(yes) or if the data field uses a simple string of bytes(no) In the case of variable being set to no,
		///the encoding specified for the data field has no meaning.
		std::string Variable = "No";
		if(Variable == "Yes")
		{
		if(maxLen < bytesProcessed + 2)
		return false;

		memcpy(&resDataField.resDescriptorValue.resValueType, p, sizeof(resDataField.resDescriptorValue.resValueType));
		p += sizeof(resDataField.resDescriptorValue.resValueType); 
		bytesProcessed += sizeof(resDataField.resDescriptorValue.resValueType);
		resDataField.resDescriptorValue.resValueType = ntohs(resDataField.resDescriptorValue.resValueType);

		if(resDataField.resDescriptorValue.resValueType == 0x0001) //singlevalue
		{
		// where is resourceValue() define
		//   resourceValue();
		}
		else if(resDataField.resDescriptorValue.resValueType == 0x0002)//listValue
		{
		if(maxLen < bytesProcessed + 2)
		return false;
		memcpy(&resDataField.resDescriptorValue.resListcount, p, sizeof(resDataField.resDescriptorValue.resListcount)); 
		p += sizeof(resDataField.resDescriptorValue.resListcount);
		bytesProcessed += sizeof(resDataField.resDescriptorValue.resListcount);
		resDataField.resDescriptorValue.resListcount = ntohs(resDataField.resDescriptorValue.resListcount);

		if(maxLen < bytesProcessed + resDataField.resDescriptorValue.resListcount)
		return false;

		for(uint16 m = 0; m < resDataField.resDescriptorValue.resListcount; m++)
		{
		// where is resourceValue() define
		//   resourceValue();
		}

		}
		else if(resDataField.resDescriptorValue.resValueType == 0x0003)//rangeValue
		{
		// where is mostDesiredRangeValue() and leastDesiredRangeValue  define
		//  mostDesiredRangeValue();
		//	leastDesiredRangeValue();
		}

		}
		else
		{
		if(maxLen < bytesProcessed + resource.resCommonHeader.resLength)
		return false;

		for(uint16 k = 0 ; k < resource.resCommonHeader.resLength; k++)
		{
		resDataField.resDataValues.push_back(*p); p++; bytesProcessed++;
		}
		}
		*/ 
		resource.resDataFields.push_back(resDataField);
		_resources.push_back(resource);
	}
	return true;
}

bool DsmccMsg::formatUUData(const uint8* buf, size_t maxLen, size_t& processedlength)
{
	const uint8* p = buf;
	if (NULL == p)
		return false;

	processedlength = 0;

	uint16 uuDataLength = (uint16)_uuDataBytes.size();
	if(maxLen < sizeof(uint16) + uuDataLength)
		return false;

	uint16 tmpUUDataLenth = htons(uuDataLength);
	memcpy((void*)p, &tmpUUDataLenth, sizeof(tmpUUDataLenth)); p += sizeof(tmpUUDataLenth); 
	processedlength += sizeof(tmpUUDataLenth);
	for(uint16 i = 0; i < uuDataLength; i++)
	{
		memcpy((void*)p, &_uuDataBytes[i], sizeof(_uuDataBytes[i]));
		p += sizeof(_uuDataBytes[i]);
		processedlength += sizeof(_uuDataBytes[i]);
	}
	return true;
}
bool DsmccMsg::formatUserPrivateData(const uint8* buf, size_t maxLen, size_t& processedlength)
{
	const uint8* p = buf;
	if (NULL == p)
		return false;

	processedlength = 0;

	uint16 privateDataLength = (uint16)_privateDataBytes.size();

	if(maxLen < sizeof(uint16) + privateDataLength)
		return false;

	uint16 tmpPrivateDataLength = htons(privateDataLength);
	memcpy((void*)p, &tmpPrivateDataLength, sizeof(tmpPrivateDataLength)); p += sizeof(tmpPrivateDataLength);
	processedlength += sizeof(tmpPrivateDataLength);

	for(uint16 i = 0; i < privateDataLength; i++)
	{
		memcpy((void*)p, &_privateDataBytes[i], sizeof(_privateDataBytes[i]));
		p += sizeof(_privateDataBytes[i]);
		processedlength += sizeof(_privateDataBytes[i]);
	}
	return true;
}
bool DsmccMsg::formatResource(const uint8* buf, size_t maxLen, size_t& processedlength)
{
	const uint8* p = buf;
	if (NULL == p)
		return false;

	processedlength = 0;
    uint16 resourceCount = (uint16)_resources.size();
	uint16 tmpResourceCount = htons(resourceCount);
	memcpy((void*)p, &tmpResourceCount, sizeof(tmpResourceCount)); p += sizeof(tmpResourceCount);
	processedlength += sizeof(tmpResourceCount);

	for(uint16 i = 0; i < resourceCount; i++)
	{
      ResourceCommonHeader tmpRsCommonHeader = _resources[i].resCommonHeader;
	  tmpRsCommonHeader.resRequestId =  htons(tmpRsCommonHeader.resRequestId);
	  tmpRsCommonHeader.resDescriptorType=  htons(tmpRsCommonHeader.resDescriptorType);
	  tmpRsCommonHeader.resNum=  htons(tmpRsCommonHeader.resNum);
	  tmpRsCommonHeader.associationTag=  htons(tmpRsCommonHeader.associationTag);
	  tmpRsCommonHeader.resLength=  htons(tmpRsCommonHeader.resLength);
	  tmpRsCommonHeader.resDataFieldCount=  htons(tmpRsCommonHeader.resDataFieldCount);

	  memcpy((void*)p, &tmpRsCommonHeader, sizeof(tmpRsCommonHeader) - sizeof(uint32) *2);
	  p+= (sizeof(tmpRsCommonHeader) - sizeof(uint32) *2);
	  processedlength += (sizeof(tmpRsCommonHeader) - sizeof(uint32) *2);

	  if(_resources[i].resCommonHeader.resDescriptorType == 0xffff)
	  {
		  uint8 buf[4];
		  memset(buf, 0, sizeof(buf));
		  tmpRsCommonHeader.typeOwnerId = htonl(tmpRsCommonHeader.typeOwnerId);
		  memcpy(buf, &tmpRsCommonHeader.typeOwnerId , sizeof(tmpRsCommonHeader.typeOwnerId));
		  memcpy((void*)p, buf + 1 , 3); p += 3; processedlength += 3;

		  memset(buf, 0, sizeof(buf)); 
		  tmpRsCommonHeader.typeOwnerValue = htonl(tmpRsCommonHeader.typeOwnerValue);
		  memcpy(buf, &tmpRsCommonHeader.typeOwnerValue , sizeof(tmpRsCommonHeader.typeOwnerValue));
		  memcpy((void*)p, buf + 1 , 3); p += 3; processedlength += 3;	  
	  }

	  for(uint16 j = 0; j < _resources[i].resDataFields.size(); j++)
	  {
		  Bytes& resDataValue = _resources[i].resDataFields[j].resDataValues;

		  for(uint16 k = 0 ; k < _resources[i].resCommonHeader.resLength; k++)
		  {
			  memcpy((void*)p, &resDataValue[k], sizeof(resDataValue[k]));
			  p += sizeof(resDataValue[k]); 
			  processedlength += sizeof(resDataValue[k]);
		  }
	  }
	}
  return true;
}
uint32 DsmccMsg::toDsmccHeader(StringMap& metadata)
{
	char strTemp[33] = "";

	memset(strTemp, 0, sizeof(strTemp));
    itoa(_hardHeader.protocolDiscriminator, strTemp, 10);
	std::string protocolDiscriminator = strTemp;

	memset(strTemp, 0, sizeof(strTemp));
	itoa(_hardHeader.dsmccType, strTemp, 10);
	std::string dsmccType = strTemp;

	memset(strTemp, 0, sizeof(strTemp));
	itoa(_hardHeader.messageId, strTemp, 10);
	std::string messageId = strTemp;

	memset(strTemp, 0, sizeof(strTemp));
	sprintf(strTemp, "%u", _hardHeader.transactionId);
//	itoa(_hardHeader.transactionId, strTemp, 10);
	std::string transactionId = strTemp;

	memset(strTemp, 0, sizeof(strTemp));
	itoa(_hardHeader.reserved1, strTemp, 10);
	std::string  reserved1 = strTemp;
    
	memset(strTemp, 0, sizeof(strTemp));
	itoa(_adpationType, strTemp, 10);
	std::string  adpationType = strTemp;

	MAPSET(StringMap, metadata, CRMetaData_protocolDiscriminator, protocolDiscriminator);
	MAPSET(StringMap, metadata, CRMetaData_dsmccType, dsmccType);
	MAPSET(StringMap, metadata, CRMetaData_messageId, messageId);
	MAPSET(StringMap, metadata, CRMetaData_transactionId, transactionId);
	MAPSET(StringMap, metadata, CRMetaData_reserved1, reserved1);
	MAPSET(StringMap, metadata, CRMetaData_adpationType, adpationType);
    
	if(_adpationBytes.size() > 0)
	{
		std::string adpationBytes(_adpationBytes.begin(), _adpationBytes.end());
		MAPSET(StringMap, metadata, CRMetaData_adpationBytes, adpationBytes);
	}
   return (uint32)metadata.size();
}
uint32 DsmccMsg::toMetaData(StringMap& metadata)
{
   return toDsmccHeader(metadata);
}
bool DsmccMsg::readMetaData(const StringMap& metadata)
{
	if(!readDmssHeader(metadata))
		return false;
	return true;
}
bool DsmccMsg::readDmssHeader(const StringMap& metadata)
{
	StringMap::const_iterator itorMd;
	itorMd = metadata.find(CRMetaData_protocolDiscriminator);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		_hardHeader.protocolDiscriminator = 0x11;
	}
	else
      _hardHeader.protocolDiscriminator = (uint8)atoi(itorMd->second.c_str());

	itorMd = metadata.find(CRMetaData_dsmccType);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		_hardHeader.dsmccType = 0x02;
	}
	else
		_hardHeader.dsmccType = (uint8)atoi(itorMd->second.c_str());

	itorMd = metadata.find(CRMetaData_messageId);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	_hardHeader.messageId = (uint16)atoi(itorMd->second.c_str());

	itorMd = metadata.find(CRMetaData_transactionId);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		_hardHeader.transactionId = 0x00000020;
	}
	else
	{
		sscanf(itorMd->second.c_str(), "%u", &_hardHeader.transactionId);
//		_hardHeader.transactionId = (uint32)atoi(itorMd->second.c_str());
	}
	
	itorMd = metadata.find(CRMetaData_reserved1);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		_hardHeader.reserved1 = 0xff;
	}
	else
		_hardHeader.reserved1 = (uint8)atoi(itorMd->second.c_str());

	itorMd = metadata.find(CRMetaData_adpationType);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		_adpationType = 0x00;
	}
	else
		_adpationType = (uint8)atoi(itorMd->second.c_str());

	itorMd = metadata.find(CRMetaData_adpationBytes);
	if(itorMd == metadata.end())
	{
		_adpationBytes.clear();
	} 
	else
	{
		_adpationBytes.clear();
		_adpationBytes.assign(itorMd->second.begin(), itorMd->second.end());
		/*for(uint8 i = 0; i < itorMd->second.size(); i++)
		{
			_adpationBytes.push_back((uint8)(itorMd->second)[i]);
		}*/
	}

   return true;
}
void DsmccMsg::dumpLog()
{
	_debugLog(ZQ::common::Log::L_INFO, CLOGFMT(DsmccMsg, "DsmccHeader: protocolDiscriminator (0x%02x) dsmccType(0x%02x) messageId(0x%04x) transactionId(0x%08x) reserved1(0x%02x)"),
		_hardHeader.protocolDiscriminator, _hardHeader.dsmccType,_hardHeader.messageId,_hardHeader.transactionId,_hardHeader.reserved1);
	std::string strAdpationbytes;
	HexToString(strAdpationbytes, _adpationBytes);
	_debugLog(ZQ::common::Log::L_INFO, CLOGFMT(DsmccMsg, "DsmccHeader: adpationType(0x%02x)adpationBytes(%s)"), _adpationType, strAdpationbytes.c_str());

	dumpPrivateData();

	for(uint16 i = 0; i < _resources.size(); i++)
	{
		Resource& resource = _resources[i];
		_debugLog(ZQ::common::Log::L_INFO, CLOGFMT(DsmccMsg, "ResourceHeader: RequestId(0x%04x) DescriptorType(0x%04x)resNum(0x%04x)associationTag(0x%04x)Flags(0x%02x) Status(0x%04x) Length(0x%04x) DataFieldCount(0x%04x) typeOwnerId(0x%08x) typeOwnerValue(0x%08x)"), 
															 resource.resCommonHeader.resRequestId, resource.resCommonHeader.resDescriptorType,resource.resCommonHeader.resNum,
															 resource.resCommonHeader.associationTag,resource.resCommonHeader.resFlags,resource.resCommonHeader.resStatus,
															 resource.resCommonHeader.resLength,resource.resCommonHeader.resDataFieldCount,resource.resCommonHeader.typeOwnerId,resource.resCommonHeader.typeOwnerValue);
		for(uint16 j = 0; j < resource.resDataFields.size(); j++)
		{
			std::string strResData;
			HexToString(strResData, resource.resDataFields[j].resDataValues);
			_debugLog(ZQ::common::Log::L_INFO, CLOGFMT(DsmccMsg, "resData: (%u)(%s)"), resource.resDataFields[j].resDataValues.size(), strResData.c_str());
		}
	}

	std::string strUUbytes;
	HexToString(strUUbytes, _uuDataBytes);
	_debugLog(ZQ::common::Log::L_INFO, CLOGFMT(DsmccMsg, "UUData: (%u)(%s)"),_uuDataBytes.size(), strUUbytes.c_str());

	std::string strPrivateBytes;
	HexToString(strPrivateBytes, _privateDataBytes);
	_debugLog(ZQ::common::Log::L_INFO, CLOGFMT(DsmccMsg, "PrivateBytes:(%u)(%s)"),_privateDataBytes.size(), strPrivateBytes.c_str());
}
//////////////////////////////////////////////////////////////////////
///////////////  class ClientSessionSetupRequest  ////////////////////
/////////////// //////////////////////////////////////////////////////

uint16 ClientSessionSetupRequest::formatMessageBody(uint8* buf, size_t maxLen)
{
	uint8* p = buf;
	if (NULL == p)
		return 0;

	if(maxLen < _messageLength)
		return -1;

	memcpy(p, &_minBody, sizeof(_minBody)); p+= sizeof(_minBody);

	// append with uudata
	size_t processedlength = 0;
	if(!formatUUData(p, maxLen - (uint16)(p - buf), processedlength))
		return -1;
	p += processedlength;

	//append with privatedata
	processedlength = 0;
	if(!formatUserPrivateData(p, maxLen - (uint16)(p - buf), processedlength))
		return -1;
	p += processedlength;

    return (uint16)(p - buf);
}
	
bool ClientSessionSetupRequest::parseMessageBody(const uint8* buf, size_t messageLength)
{
	const uint8* p = buf;
	if (NULL == p)
		return false;

	size_t processedLength =  0;
	_messageLength = (uint16)messageLength;

	if(messageLength < sizeof(_minBody))
		return false;

   //the minBody
	memcpy(&_minBody, p, sizeof(_minBody)); p+= sizeof(_minBody); processedLength += sizeof(_minBody);

	//the UUData
	size_t byteprocessed = 0;
	if(!parseUUData(p, _messageLength - processedLength, byteprocessed))
		return false;
    p += byteprocessed; processedLength += byteprocessed;

	//the privateData
	byteprocessed = 0;
	if(!parseUserPrivateData(p, _messageLength - processedLength, byteprocessed))
		return false;
	p += byteprocessed; processedLength += byteprocessed; 

	if(processedLength != messageLength)
		return false;	

	if(!parsePrivateData())
		return false;

	return true;
}

bool ClientSessionSetupRequest::parsePrivateData()
{
  uint16 privateDataLength = (uint16)_privateDataBytes.size();
  uint16 processedLength = 0;

  if(privateDataLength < 3)
	  return false;
  //parse ProtocolId, ProtocolVersion, DescriptorCount
  uint8 ProtocolId = _privateDataBytes[processedLength]; processedLength++;
  uint8 ProtocolVersion = _privateDataBytes[processedLength]; processedLength++;
  uint8	DescriptorCount = _privateDataBytes[processedLength]; processedLength++;

  //parse AssetId,Nodegroup ...TLV 
  for(uint8 i = 0; i < DescriptorCount; i++)
  {
    //T L
    if(privateDataLength < processedLength + 2)
		return false;
    uint8 tag = _privateDataBytes[processedLength]; processedLength++;
	uint8 length = _privateDataBytes[processedLength]; processedLength++;

	if(privateDataLength < processedLength +length)
		return false;
	switch(tag)
	{
	case 0x01:
		_assetId= std::string(&_privateDataBytes[processedLength], &_privateDataBytes[processedLength] + length);
		processedLength += length;
		break;
		
	case 0x02:
		uint8 buf[8];
		memset(buf, 0, sizeof(buf));
		memcpy(buf + 2, &_privateDataBytes[processedLength], length);
		memcpy(&_nodeGroupId, buf, length + 2);
		_nodeGroupId = ntohll(_nodeGroupId);
		processedLength += length;
		break;
	case 0x05: // AppRequest Data 
		{
			if(privateDataLength < processedLength + 5)
				return false;

			uint8 AppRequestProtocolId =_privateDataBytes[processedLength]; processedLength++;
			uint8 AppRequestProtocolVersion = _privateDataBytes[processedLength]; processedLength++;
			uint8 AppRequestDescriptorCount =_privateDataBytes[processedLength]; processedLength++;

			//parse AppRequest Data  
			for(uint8 j = 0; j < AppRequestDescriptorCount; j++)
			{
				//T L
				if(privateDataLength < processedLength + 2)
					return false;
				uint8 appDataTag = _privateDataBytes[processedLength]; processedLength++;
				uint8 appDataLength = _privateDataBytes[processedLength]; processedLength++;

				if(privateDataLength < processedLength + appDataLength)
					return false;
				switch(appDataTag)
				{
				case 0x03:
					memcpy(&_billingId, &_privateDataBytes[processedLength], appDataLength);
					_billingId = ntohl(_billingId);
					break;
				case 0x04:
					memcpy(&_purchaseTime, &_privateDataBytes[processedLength], appDataLength);
					_purchaseTime = ntohl(_purchaseTime);
					break;
				case 0x05:
					memcpy(&_remainingPlayTime, &_privateDataBytes[processedLength], appDataLength);
					_remainingPlayTime = ntohl(_remainingPlayTime);
					break;
				case 0x06:
					memcpy(&_errorCode, &_privateDataBytes[processedLength], appDataLength);
					_errorCode = ntohl(_errorCode);
					break;
				case 0x07:
					memcpy(&_homeId, &_privateDataBytes[processedLength], appDataLength);
					_homeId = ntohl(_homeId);
					break;
				case 0x08:
					memcpy(&_purchaseId, &_privateDataBytes[processedLength], appDataLength);
					_purchaseId = ntohl(_purchaseId);
					break;
				case 0x09:
					memcpy(&_smartCardId, &_privateDataBytes[processedLength], appDataLength);
					_smartCardId = ntohl(_smartCardId);
					break;
				case 0x0d:
					memcpy(&_analogCopyPurchase, &_privateDataBytes[processedLength], appDataLength);
					_analogCopyPurchase = ntohl(_analogCopyPurchase);
					break;
				case 0x0f:
					memcpy(&_packageId, &_privateDataBytes[processedLength], appDataLength);
					_packageId = ntohl(_packageId);
					break;
				default:
					break;
				}
				processedLength += appDataLength;
			}
		}
		break;
	default:
		processedLength += length;
		break;
	}
  }
//  printf("	assetId(%s)       nodeGroupId(%lld)\n", _assetId.c_str(), _nodeGroupId);
//  printf("	billingId(%d)		purchaseTime(%d)	remainingPlayTime(%d)\n", _billingId, _purchaseTime,_remainingPlayTime);
//  printf("	errorCode(%d)		homeId(%d)		    purchaseId(%d)\n", _errorCode, _homeId, _purchaseId);
//  printf("	smartCardId(%d)	analogCopyPurchase(%d)		packageId(%d)\n", _smartCardId, _analogCopyPurchase, _packageId);
  return true;
}

uint32 ClientSessionSetupRequest::toMetaData(StringMap& metadata)
{
    toDsmccHeader(metadata);

/*	std::string sessionId(_minBody.sessionId, _minBody.sessionId + sizeof(_minBody.sessionId));
	std::string reserved(_minBody.reserved, _minBody.reserved + sizeof(_minBody.reserved));
	std::string clientId(_minBody.clientId, _minBody.clientId + sizeof(_minBody.clientId));
	std::string serverId(_minBody.serverId, _minBody.serverId + sizeof(_minBody.serverId));*/

	std::string sessionId;
	HexToString(sessionId, _minBody.sessionId, sizeof(_minBody.sessionId));

	std::string reserved;
	HexToString(reserved, _minBody.reserved, sizeof(_minBody.reserved));

	std::string clientId;
	HexToString(clientId, _minBody.clientId, sizeof(_minBody.clientId));

	std::string serverId;
	HexToString(serverId, _minBody.serverId, sizeof(_minBody.serverId));

	MAPSET(StringMap, metadata, CRMetaData_SessionId, sessionId);
	MAPSET(StringMap, metadata, CRMetaData_CSSRreserved, reserved);
	MAPSET(StringMap, metadata, CRMetaData_CSSRclientId, clientId );
  	MAPSET(StringMap, metadata, CRMetaData_CSSRserverId, serverId);

	//  privatedata
    MAPSET(StringMap, metadata, CRMetaData_assetId, _assetId);
	char strTemp[65] = "";
	memset(strTemp, 0, 65);
	sprintf(strTemp, "%llu",  _nodeGroupId);
	std::string nodegroupid = strTemp;
	MAPSET(StringMap, metadata, CRMetaData_nodeGroupId, nodegroupid);
   
	memset(strTemp, 0, 65);
	sprintf(strTemp, "%u",  _billingId);
	std::string billingId = strTemp;
	MAPSET(StringMap, metadata, CRMetaData_billingId, billingId);

	memset(strTemp, 0, 65);
	sprintf(strTemp, "%u",  _purchaseTime);
	std::string purchaseTime = strTemp;
	MAPSET(StringMap, metadata, CRMetaData_purchaseTime, purchaseTime);

	memset(strTemp, 0, 65);
	sprintf(strTemp, "%u",  _remainingPlayTime);
	std::string remainingPlayTime = strTemp;
	MAPSET(StringMap, metadata, CRMetaData_remainingPlayTime, remainingPlayTime);

	memset(strTemp, 0, 65);
	sprintf(strTemp, "%u",  _errorCode);
	std::string errorCode = strTemp;
	MAPSET(StringMap, metadata, CRMetaData_errorCode , errorCode);

	memset(strTemp, 0, 65);
	sprintf(strTemp, "%u",  _homeId);
	std::string homeId = strTemp;
	MAPSET(StringMap, metadata, CRMetaData_homeId, homeId);

	memset(strTemp, 0, 65);
	sprintf(strTemp, "%u",  _purchaseId);
	std::string purchaseId = strTemp;
	MAPSET(StringMap, metadata, CRMetaData_purchaseId, purchaseId);

	memset(strTemp, 0, 65);
	sprintf(strTemp, "%u",  _smartCardId);
	std::string smartCardId = strTemp;
	MAPSET(StringMap, metadata, CRMetaData_smartCardId, smartCardId);

	memset(strTemp, 0, 65);
	sprintf(strTemp, "%u",  _analogCopyPurchase);
	std::string analogCopyPurchase = strTemp;
	MAPSET(StringMap, metadata, CRMetaData_analogCopyPurchase, analogCopyPurchase);

/*	memset(strTemp, 0, 65);
	sprintf(strTemp, "%u",  _packageId);
	std::string packageId = strTemp;
	MAPSET(StringMap, metadata, CRMetaData_packageId, packageId);*/
	
	return (uint32)metadata.size();
}
bool ClientSessionSetupRequest::readMetaData(const StringMap& metadata)
{
	if(!readDmssHeader(metadata))
		return false;

	_messageLength = 0;
	// read miniBody
	StringMap::const_iterator itorMd;
	itorMd = metadata.find(CRMetaData_SessionId);
	if(itorMd == metadata.end() || itorMd->second.empty() || itorMd->second.size() != 20)
	{
		return false;
	}
	const std::string& strSessionId = itorMd->second;

	StringToHex(strSessionId, _minBody.sessionId, sizeof(_minBody.sessionId));

	itorMd = metadata.find(CRMetaData_CSSRreserved);
	if(itorMd == metadata.end() || itorMd->second.empty() || itorMd->second.size() != 4)
	{
		return false;
	}
	const std::string& strReserved = itorMd->second;
/*	for(uint16 i = 0 ; i < strReserved.size(); i++)
	{
		_minBody.reserved[i] =  (uint8)strReserved[i];
	}*/
	StringToHex(strReserved, _minBody.reserved, sizeof(_minBody.reserved));

	itorMd = metadata.find(CRMetaData_CSSRclientId);
	if(itorMd == metadata.end() || itorMd->second.empty()|| itorMd->second.size() != 40)
	{
		return false;
	}
	const std::string& strClientId = itorMd->second;
/*	for(uint16 i = 0 ; i < strClientId.size(); i++)
	{
		_minBody.clientId[i] = (uint8)strClientId[i];
	}*/
	StringToHex(strClientId, _minBody.clientId, sizeof(_minBody.clientId));

	itorMd = metadata.find(CRMetaData_CSSRserverId);
	if(itorMd == metadata.end() || itorMd->second.empty()|| itorMd->second.size() != 40)
	{
		return false;
	}
	const std::string& strServerId = itorMd->second;
/*	for(uint16 i = 0 ; i < strServerId.size(); i++)
	{
		_minBody.serverId[i] = (uint8)strServerId[i];
	}*/
	StringToHex(strServerId, _minBody.serverId, sizeof(_minBody.serverId));

	_messageLength += sizeof(_minBody);


	_privateDataBytes.clear();
	//read privateData 
    uint8 pdDescriptorCount = 0;
	uint8 pdOffsetAPPDataCount = 0;
	itorMd = metadata.find(CRMetaData_assetId);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	_assetId = itorMd->second;
	pdDescriptorCount++;

	itorMd = metadata.find(CRMetaData_nodeGroupId);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	_nodeGroupId =(uint64)_atoi64(itorMd->second.c_str());
    pdDescriptorCount++;

	//insert privatedata ProtocolId, ProtocolVersion DescriptorCount
	_privateDataBytes.push_back(pdProtocolId);
	_privateDataBytes.push_back(pdProtocolVersion);
	_privateDataBytes.push_back(pdDescriptorCount); //update it 
    //insert asset info 
	_privateDataBytes.push_back(0x01);
	_privateDataBytes.push_back((uint8)_assetId.size());
	_privateDataBytes.insert(_privateDataBytes.begin() + _privateDataBytes.size(), _assetId.begin(), _assetId.end());
	//insert node group 
	uint8 buf[8];
	memset(buf, 0, sizeof(buf));
    uint64 NodeGroupId = htonll(_nodeGroupId);
	memcpy(buf, &NodeGroupId, sizeof(buf));
	_privateDataBytes.push_back(0x02);
	_privateDataBytes.push_back(0x06);
    _privateDataBytes.insert(_privateDataBytes.begin() + _privateDataBytes.size(), buf +2, buf +8);
   
    uint8 appDataLenthOffset = 0;
	uint8 appDataDescriptorCount = 0;
	uint8 appDataDescriptorCountOffset = 0;
	if(metadata.find(CRMetaData_billingId) != metadata.end() ||
	   metadata.find(CRMetaData_purchaseTime) != metadata.end() ||
	   metadata.find(CRMetaData_remainingPlayTime) != metadata.end() ||
	   metadata.find(CRMetaData_errorCode) != metadata.end() ||
	   metadata.find(CRMetaData_homeId) != metadata.end() ||
	   metadata.find(CRMetaData_purchaseId) != metadata.end() ||
	   metadata.find(CRMetaData_smartCardId) != metadata.end() ||
	   metadata.find(CRMetaData_analogCopyPurchase) != metadata.end() ||
	   metadata.find(CRMetaData_packageId) != metadata.end())
	{
		_privateDataBytes.push_back(0x05);
		appDataLenthOffset = (uint8) _privateDataBytes.size();
		_privateDataBytes.push_back(0x00); // appDataLenth
		_privateDataBytes.push_back(pdAppRequestProtocolId);
		_privateDataBytes.push_back(pdAppRequestProtocolVersion);
		appDataDescriptorCountOffset = (uint8)_privateDataBytes.size();
		_privateDataBytes.push_back(0x00); // AppRequestDescriptorCount
		pdDescriptorCount++;

		itorMd = metadata.find(CRMetaData_billingId);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x03);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_billingId);
			readPrivateAppData(_billingId);
			appDataDescriptorCount++;
		}

		itorMd = metadata.find(CRMetaData_purchaseTime);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x04);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_purchaseTime);
			readPrivateAppData(_purchaseTime);
			appDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_remainingPlayTime);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x05);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_remainingPlayTime);
			readPrivateAppData(_remainingPlayTime);
			appDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_errorCode);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x06);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_errorCode);
			readPrivateAppData(_errorCode);
			appDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_homeId);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x07);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_homeId);
			readPrivateAppData(_homeId);
			appDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_purchaseId);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x08);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_purchaseId);
			readPrivateAppData(_purchaseId);
			appDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_smartCardId);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x09);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_smartCardId);
			readPrivateAppData(_smartCardId);
			appDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_analogCopyPurchase);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x0d);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_analogCopyPurchase);
			readPrivateAppData(_analogCopyPurchase);
			appDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_packageId);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x03);
			_privateDataBytes.push_back(0x0f);
			sscanf(itorMd->second.c_str(), "%u", &_packageId);
			readPrivateAppData(_packageId);
			appDataDescriptorCount++;
		}
		_privateDataBytes[appDataLenthOffset] = appDataDescriptorCount * 6 + 3;
		_privateDataBytes[appDataDescriptorCountOffset] = appDataDescriptorCount;
	}

	_privateDataBytes[2] = pdDescriptorCount;
	_messageLength += sizeof(uint16) + (uint16)_uuDataBytes.size() + sizeof(uint16) + (uint16)_privateDataBytes.size();
	return true;
}
bool ClientSessionSetupRequest::readPrivateAppData(uint32 element)
{
	uint8 buf[4];
	memset(buf, 0, sizeof(buf));
	uint32 uTemp = htonl(element);
	memcpy(buf, &uTemp, sizeof(uTemp));
	_privateDataBytes.insert(_privateDataBytes.begin() + _privateDataBytes.size(), buf, buf + 4);
	return true;
}
void ClientSessionSetupRequest::dumpPrivateData()
{
	//dump miniBody
	std::string sessionId;
	HexToString(sessionId, _minBody.sessionId, sizeof(_minBody.sessionId));

	std::string reserved;
	HexToString(reserved, _minBody.reserved, sizeof(_minBody.reserved));

	std::string clientId;
	HexToString(clientId, _minBody.clientId, sizeof(_minBody.clientId));

	std::string serverId;
	HexToString(serverId, _minBody.serverId, sizeof(_minBody.serverId));

	_debugLog(ZQ::common::Log::L_INFO, CLOGFMT(ClientSessionSetupRequest, "sessionId(%s) reserved(%s) clientId(%s) serverId(%s)"), sessionId.c_str(), reserved.c_str(), clientId.c_str(), serverId.c_str());
	_debugLog(ZQ::common::Log::L_INFO, CLOGFMT(ClientSessionSetupRequest, "AppRequestData: assetId(%s) nodeGroupId(%lld) billingId(%u) purchaseTime(%u)remainingPlayTime(%u) errorCode(%u) homeId(%u) purchaseId(0x%08x) smartCardId(0x%08x) analogCopyPurchase(%u) packageId(%u)"),
		_assetId.c_str(), _nodeGroupId, _billingId, _purchaseTime, _remainingPlayTime, _errorCode, _homeId, _purchaseId, _smartCardId, _analogCopyPurchase, _packageId);
}
//////////////////////////////////////////////////////////////////////
///////////////  class ClientSessionSetupRequest  ////////////////////
/////////////// //////////////////////////////////////////////////////
ClientSessionSetupConfirm::ClientSessionSetupConfirm(HardHeader& hardHeader):DsmccMsg(hardHeader)
{
	memset(&_minBody, 0 , sizeof(_minBody));

	Resource resource;

	resource.resCommonHeader.resRequestId = 0x00;
	resource.resCommonHeader.resDescriptorType = 0xf003;
	resource.resCommonHeader.resNum = 0xc005;
	resource.resCommonHeader.associationTag = 0xffff;
	resource.resCommonHeader.resFlags = 0x83;
	resource.resCommonHeader.resStatus = 0x01;
	resource.resCommonHeader.resLength = 26;
	resource.resCommonHeader.resDataFieldCount = 3;
    
	ResourceDataField resDataField;
    SaHeadEndIdRsrc_t   saheadendid;
	saheadendid.headEndFlag = htons(0x0001);
	saheadendid.headEndId.assign(0, 20);
	saheadendid.tsid = 0x00000000;
	char temp[26]; memset(temp, 0 , 26);
	memcpy(temp, &saheadendid, sizeof(saheadendid));
	resDataField.resDataValues.assign(temp, temp + sizeof(temp));

	resource.resDataFields.push_back(resDataField);
	_resources.push_back(resource);


	_IpAddress = 0;  
	_IpPort = 0; 
	_streamHandel = 0;
	_billingId = 0;  
	_purchaseTime = 0;  
	_remainingPlayTime = 0;  
	_errorCode = 0;  
	_homeId = 0; 
	_purchaseId = 0;  
	_smartCardId = 0; 
	_analogCopyPurchase = 0;  
	_packageId = 0;  
	_heartbeat = 0; 
	_lscpIpProtocol = 0;

	_billingIdF = 0;    
	_purchaseTimeF = 0;  
	_remainingPlayTimeF = 0; 
	_errorCodeF = 0;  
	_homeIdF = 0;  
	_purchaseIdF = 0;  
	_smartCardIdF = 0;  
	_analogCopyPurchaseF = 0;  
	_packageIdF = 0;  
};
uint16 ClientSessionSetupConfirm::formatMessageBody(uint8* buf, size_t maxLen)
{
	uint8* p = buf;
	if (NULL == p)
		return 0;

	if(maxLen < _messageLength)
		return -1;

	MinBody tmpMinBody = _minBody;
	tmpMinBody.response = htons(_minBody.response);
	memcpy(p, &tmpMinBody, sizeof(tmpMinBody)); p+= sizeof(tmpMinBody);
  
	size_t processedlength = 0;

	// append with resource
	if(!formatResource(p, maxLen - sizeof(_minBody), processedlength))
		return -1;
	p += processedlength;

	// append with uudata
	processedlength = 0;
	if(!formatUUData(p, maxLen - (uint16)(p - buf), processedlength))
		return -1;
	p += processedlength;

	//append with privatedata
	processedlength = 0;
	if(!formatUserPrivateData(p,  maxLen - (uint16)(p - buf), processedlength))
		return -1;
	p += processedlength;

	if(_minBody.sessionId[0] == 0xFF)
	{
		uint32 streamhandel = htonl(_streamHandel);
		memcpy(p, &streamhandel, sizeof(streamhandel));
		p += sizeof(streamhandel);
	}

	return (uint16)(p - buf);
}
bool ClientSessionSetupConfirm::parseMessageBody(const uint8* buf, size_t messageLength)
{
	const uint8* p = buf;
	if (NULL == p)
		return false;

	size_t processedLength =  0;
	_messageLength = (uint16)messageLength;

	if(messageLength < sizeof(_minBody))
		return false;

	//the minBody
	memcpy(&_minBody, p, sizeof(_minBody)); p+= sizeof(_minBody); processedLength += sizeof(_minBody);
    _minBody.response = ntohs(_minBody.response);

	size_t byteprocessed = 0;
	if(!parseResouce(p, _messageLength - processedLength, byteprocessed))
		return false;
	 p += byteprocessed; processedLength += byteprocessed;

	//the UUData
	byteprocessed = 0;
	if(!parseUUData(p, _messageLength - processedLength, byteprocessed))
		return false;
	p += byteprocessed; processedLength += byteprocessed; 

	//the privateData
	byteprocessed = 0;
	if(!parseUserPrivateData(p, _messageLength - processedLength, byteprocessed))
		return false;
	p += byteprocessed; processedLength += byteprocessed; 

	if(processedLength != messageLength)
		return false;	

	if(!parsePrivateData())
		return false;
  return true;
}
bool ClientSessionSetupConfirm::parsePrivateData()
{
	uint16 privateDataLength = (uint16)_privateDataBytes.size();
	uint16 processedLength = 0;

	if(privateDataLength < 3)
		return false;
	//parse ProtocolId, ProtocolVersion, DescriptorCount
	uint8 ProtocolId = _privateDataBytes[processedLength]; processedLength++;
	uint8 ProtocolVersion = _privateDataBytes[processedLength]; processedLength++;
	uint8 DescriptorCount = _privateDataBytes[processedLength]; processedLength++;

	//parse AssetId,Nodegroup ...TLV 
	for(uint8 i = 0; i < DescriptorCount; i++)
	{
		//T L
		if(privateDataLength < processedLength + 2)
			return false;
		uint8 tag = _privateDataBytes[processedLength]; processedLength++;
		uint8 length = _privateDataBytes[processedLength]; processedLength++;

		if(privateDataLength < processedLength +length)
			return false;
		switch(tag)
		{
		case 0x03:
            memcpy(&_IpPort, &_privateDataBytes[processedLength], 2);processedLength += 2;
			memcpy(&_IpAddress, &_privateDataBytes[processedLength], 4);processedLength += 4;
			_IpPort = ntohs(_IpPort);
			_IpAddress = ntohl(_IpAddress);
			break;
		case 0x04:
			memcpy(&_streamHandel, &_privateDataBytes[processedLength], length);
			_streamHandel = ntohl(_streamHandel);
			processedLength += length;
			break;
		case 0x06: // AppRequest Data 
			{
				if(privateDataLength < processedLength + 5)
					return false;

				uint8 AppResponseProtocolId =_privateDataBytes[processedLength]; processedLength++;
				uint8 AppRResponseProtocolVersion = _privateDataBytes[processedLength]; processedLength++;
				uint8 AppResponseDescriptorCount =_privateDataBytes[processedLength]; processedLength++;

				//parse AppRequest Data  
				for(uint8 j = 0; j < AppResponseDescriptorCount; j++)
				{
					//T L
					if(privateDataLength < processedLength + 2)
						return false;
					uint8 appDataTag = _privateDataBytes[processedLength]; processedLength++;
					uint8 appDataLength = _privateDataBytes[processedLength]; processedLength++;

					if(privateDataLength < processedLength + appDataLength)
						return false;
					switch(appDataTag)
					{
					case 0x03:
						memcpy(&_billingId, &_privateDataBytes[processedLength], appDataLength);
						_billingId = ntohl(_billingId);
						break;
					case 0x04:
						memcpy(&_purchaseTime, &_privateDataBytes[processedLength], appDataLength);
						_purchaseTime = ntohl(_purchaseTime);
						break;
					case 0x05:
						memcpy(&_remainingPlayTime, &_privateDataBytes[processedLength], appDataLength);
						_remainingPlayTime = ntohl(_remainingPlayTime);
						break;
					case 0x06:
						memcpy(&_errorCode, &_privateDataBytes[processedLength], appDataLength);
						_errorCode = ntohl(_errorCode);
						break;
					case 0x07:
						memcpy(&_homeId, &_privateDataBytes[processedLength], appDataLength);
						_homeId = ntohl(_homeId);
						break;
					case 0x08:
						memcpy(&_purchaseId, &_privateDataBytes[processedLength], appDataLength);
						_purchaseId = ntohl(_purchaseId);
						break;
					case 0x09:
						memcpy(&_smartCardId, &_privateDataBytes[processedLength], appDataLength);
						_smartCardId = ntohl(_smartCardId);
						break;
					case 0x0d:
						memcpy(&_analogCopyPurchase, &_privateDataBytes[processedLength], appDataLength);
						_analogCopyPurchase = ntohl(_analogCopyPurchase);
						break;
					case 0x0f:
						memcpy(&_packageId, &_privateDataBytes[processedLength], appDataLength);
						_packageId = ntohl(_packageId);
						break;
					case 0x80:
						memcpy(&_heartbeat, &_privateDataBytes[processedLength], appDataLength);
						_heartbeat = ntohl(_heartbeat);
						break;
					case 0x81:
						memcpy(&_lscpIpProtocol, &_privateDataBytes[processedLength], appDataLength);
						_lscpIpProtocol = ntohl(_lscpIpProtocol);
						break;
					default:
						break;
					}
					processedLength += appDataLength;
				}
			}
			break;
		case 0x80:
			{
				if(privateDataLength < processedLength + 5)
					return false;

				uint8 FunctionProtocolId =_privateDataBytes[processedLength]; processedLength++;
				uint8 FunctionProtocolVersion = _privateDataBytes[processedLength]; processedLength++;
				uint8 FunctionDescriptorCount =_privateDataBytes[processedLength]; processedLength++;

				//parse AppRequest Data  
				for(uint8 j = 0; j < FunctionDescriptorCount; j++)
				{
					//T L
					if(privateDataLength < processedLength + 2)
						return false;
					uint8 FunctionDataTag = _privateDataBytes[processedLength]; processedLength++;
					uint8 FunctionDataLength = _privateDataBytes[processedLength]; processedLength++;

					if(privateDataLength < processedLength + FunctionDataLength)
						return false;
					switch(FunctionDataTag)
					{
					case 0x03:
						memcpy(&_billingIdF, &_privateDataBytes[processedLength], FunctionDataLength);
						_billingIdF = ntohl(_billingIdF);
						break;
					case 0x04:
						memcpy(&_purchaseTimeF, &_privateDataBytes[processedLength], FunctionDataLength);
						_purchaseTimeF = ntohl(_purchaseTimeF);
						break;
					case 0x05:
						memcpy(&_remainingPlayTimeF, &_privateDataBytes[processedLength], FunctionDataLength);
						_remainingPlayTimeF = ntohl(_remainingPlayTimeF);
						break;
					case 0x06:
						memcpy(&_errorCodeF, &_privateDataBytes[processedLength], FunctionDataLength);
						_errorCodeF = ntohl(_errorCodeF);
						break;
					case 0x07:
						memcpy(&_homeId, &_privateDataBytes[processedLength], FunctionDataLength);
						_homeIdF = ntohl(_homeIdF);
						break;
					case 0x08:
						memcpy(&_purchaseIdF, &_privateDataBytes[processedLength], FunctionDataLength);
						_purchaseIdF = ntohl(_purchaseIdF);
						break;
					case 0x09:
						memcpy(&_smartCardIdF, &_privateDataBytes[processedLength], FunctionDataLength);
						_smartCardIdF = ntohl(_smartCardIdF);
						break;
					case 0x0d:
						memcpy(&_analogCopyPurchaseF, &_privateDataBytes[processedLength], FunctionDataLength);
						_analogCopyPurchaseF = ntohl(_analogCopyPurchaseF);
						break;
					case 0x0f:
						memcpy(&_packageIdF, &_privateDataBytes[processedLength], FunctionDataLength);
						_packageIdF = ntohl(_packageIdF);
						break;
					default:
						break;
					}
					processedLength += FunctionDataLength;
				}
			}
			break;
		default:
			processedLength += length;
			break;
		}
	}
	return true;
}

uint32 ClientSessionSetupConfirm::toMetaData(StringMap& metadata)
{
	toDsmccHeader(metadata);

	std::string sessionId;
	HexToString(sessionId, _minBody.sessionId, sizeof(_minBody.sessionId));

	char strTemp[33];
	memset(strTemp, 0, sizeof(33));
	itoa(_minBody.response, strTemp, 10);
	std::string response = strTemp;

	std::string serverId;
	HexToString(serverId, _minBody.serverId, sizeof(_minBody.serverId));

	MAPSET(StringMap, metadata, CRMetaData_SessionId, sessionId);
	MAPSET(StringMap, metadata, CRMetaData_CSSCresponse, response);
	MAPSET(StringMap, metadata, CRMetaData_CSSCserverId, serverId);

	return (uint32)metadata.size();
}
bool ClientSessionSetupConfirm::readMetaData(const StringMap& metadata)
{
	if(!readDmssHeader(metadata))
		return false;

	_messageLength = 0;

	StringMap::const_iterator itorMd;
	itorMd = metadata.find(CRMetaData_SessionId);
	if(itorMd == metadata.end() || itorMd->second.empty()|| itorMd->second.size() != 20)
	{
		return false;
	}
	const std::string& strSessId = itorMd->second ;  

	StringToHex(strSessId, _minBody.sessionId, sizeof(_minBody.sessionId));

	itorMd = metadata.find(CRMetaData_CSSCresponse);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	_minBody.response =(uint16) atoi(itorMd->second.c_str());

	itorMd = metadata.find(CRMetaData_CSSCserverId);
	if(itorMd == metadata.end() || itorMd->second.empty()|| itorMd->second.size() != 40)
	{
		return false;
	}
	const std::string& strServerId = itorMd->second ;  
/*	for(uint16 i = 0; i < strServerId.size(); i++)
		_minBody.serverId[i] = (uint8)strServerId[i];*/
	StringToHex(strServerId, _minBody.serverId, sizeof(_minBody.serverId));

	_messageLength += sizeof(_minBody);

	_privateDataBytes.clear();
	//read privateData 
	uint8 pdDescriptorCount = 0;
	uint8 pdOffsetAPPDataCount = 0;
	itorMd = metadata.find(CRMetaData_Port);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	_IpPort = (uint16)atoi(itorMd->second.c_str());

	itorMd = metadata.find(CRMetaData_IPaddress);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	_IpAddress = (uint32)inet_addr(itorMd->second.c_str());

	pdDescriptorCount++;

	itorMd = metadata.find(CRMetaData_StreamHandelId);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	sscanf(itorMd->second.c_str(),"%u", &_streamHandel);
	pdDescriptorCount++;

	//insert privatedata ProtocolId, ProtocolVersion DescriptorCount
	_privateDataBytes.push_back(pdProtocolId);
	_privateDataBytes.push_back(pdProtocolVersion);
	_privateDataBytes.push_back(pdDescriptorCount); //update it 
	//insert ip port 
	uint8 buf[6];
	memset(buf, 0, sizeof(buf));
	uint16 port = htons(_IpPort);
	memcpy(buf, &port, sizeof(port));
	memcpy(buf + sizeof(port), &_IpAddress, sizeof(_IpAddress));
	_privateDataBytes.push_back(0x03);
	_privateDataBytes.push_back(0x06);
	
	_privateDataBytes.insert(_privateDataBytes.begin() + _privateDataBytes.size(), buf, buf +6);

	//insert streamhandel
	memset(buf, 0, sizeof(buf));
	uint32 streamhandel = htonl(_streamHandel);
	memcpy(buf, &streamhandel, sizeof(streamhandel));
	_privateDataBytes.push_back(0x04);
	_privateDataBytes.push_back(0x04);
	_privateDataBytes.insert(_privateDataBytes.begin() + _privateDataBytes.size(), buf, buf +4);

	//insert FunctionData
	if(metadata.find(CRMetaData_billingIdF) != metadata.end() ||
		metadata.find(CRMetaData_purchaseTimeF) != metadata.end() ||
		metadata.find(CRMetaData_remainingPlayTimeF) != metadata.end() ||
		metadata.find(CRMetaData_errorCodeF) != metadata.end() ||
		metadata.find(CRMetaData_homeIdF) != metadata.end() ||
		metadata.find(CRMetaData_purchaseIdF) != metadata.end() ||
		metadata.find(CRMetaData_smartCardIdF) != metadata.end() ||
		metadata.find(CRMetaData_analogCopyPurchaseF) != metadata.end() ||
		metadata.find(CRMetaData_packageIdF) != metadata.end())
	{
		uint8 functionDataLenthOffset = 0;
		uint8 functionDataDescriptorCount = 0;
		uint8 functionDataDescriptorCountOffset = 0;

		_privateDataBytes.push_back(0x80);
		functionDataLenthOffset = (uint8) _privateDataBytes.size();
		_privateDataBytes.push_back(0x00); // functionDataLenth
		_privateDataBytes.push_back(pdFunctionProtocolId);
		_privateDataBytes.push_back(pdFunctionVersion);
		functionDataDescriptorCountOffset = (uint8)_privateDataBytes.size();
		_privateDataBytes.push_back(0x00); // functionDescriptorCount
		pdDescriptorCount++;

		itorMd = metadata.find(CRMetaData_billingIdF);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x03);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_billingIdF);
			readPrivateAppData(_billingIdF);
			functionDataDescriptorCount++;
		}

		itorMd = metadata.find(CRMetaData_purchaseTimeF);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x04);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_purchaseTimeF);
			readPrivateAppData(_purchaseTimeF);
			functionDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_remainingPlayTimeF);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x05);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_remainingPlayTimeF);
			readPrivateAppData(_remainingPlayTimeF);
			functionDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_errorCodeF);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x06);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_errorCodeF);
			readPrivateAppData(_errorCodeF);
			functionDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_homeIdF);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x07);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_homeIdF);
			readPrivateAppData(_homeIdF);
			functionDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_purchaseIdF);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x08);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_purchaseIdF);
			readPrivateAppData(_purchaseIdF);
			functionDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_smartCardIdF);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x09);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_smartCardIdF);
			readPrivateAppData(_smartCardIdF);
			functionDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_analogCopyPurchaseF);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x0d);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_analogCopyPurchaseF);
			readPrivateAppData(_analogCopyPurchaseF);
			functionDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_packageIdF);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x0f);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_packageIdF);
			readPrivateAppData(_packageIdF);
			functionDataDescriptorCount++;
		}
		_privateDataBytes[functionDataLenthOffset] = functionDataDescriptorCount * 6 + 3; //functiondata length(tag 1 length 1 data 4, ProtocolId 1, ProtocolVersion 1, DescriptorCount 1)
		_privateDataBytes[functionDataDescriptorCountOffset] = functionDataDescriptorCount;
	}

	//insert AppResponseData
	if(metadata.find(CRMetaData_billingId) != metadata.end() ||
		metadata.find(CRMetaData_purchaseTime) != metadata.end() ||
		metadata.find(CRMetaData_remainingPlayTime) != metadata.end() ||
		metadata.find(CRMetaData_errorCode) != metadata.end() ||
		metadata.find(CRMetaData_homeId) != metadata.end() ||
		metadata.find(CRMetaData_purchaseId) != metadata.end() ||
		metadata.find(CRMetaData_smartCardId) != metadata.end() ||
		metadata.find(CRMetaData_analogCopyPurchase) != metadata.end() ||
		metadata.find(CRMetaData_packageId) != metadata.end()||
		metadata.find(CRMetaData_heartbeat) != metadata.end() ||
		metadata.find(CRMetaData_lscpIpProtocol) != metadata.end())
	{
		uint8 appDataLenthOffset = 0;
		uint8 appDataDescriptorCount = 0;
		uint8 appDataDescriptorCountOffset = 0;

		_privateDataBytes.push_back(0x06);
		appDataLenthOffset = (uint8) _privateDataBytes.size();
		_privateDataBytes.push_back(0x00); // appDataLenth
		_privateDataBytes.push_back(pdAppResponseProtocolId);
		_privateDataBytes.push_back(pdAppResponseProtocolId);
		appDataDescriptorCountOffset = (uint8)_privateDataBytes.size();
		_privateDataBytes.push_back(0x00); // AppResponseDescriptorCount
		pdDescriptorCount++;

		itorMd = metadata.find(CRMetaData_billingId);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x03);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_billingId);
			readPrivateAppData(_billingId);
			appDataDescriptorCount++;
		}

		itorMd = metadata.find(CRMetaData_purchaseTime);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x04);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_purchaseTime);
			readPrivateAppData(_purchaseTime);
			appDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_remainingPlayTime);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x05);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_remainingPlayTime);
			readPrivateAppData(_remainingPlayTime);
			appDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_errorCode);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x06);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_errorCode);
			readPrivateAppData(_errorCode);
			appDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_homeId);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x07);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_homeId);
			readPrivateAppData(_homeId);
			appDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_purchaseId);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x08);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_purchaseId);
			readPrivateAppData(_purchaseId);
			appDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_smartCardId);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x09);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_smartCardId);
			readPrivateAppData(_smartCardId);
			appDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_analogCopyPurchase);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x0d);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_analogCopyPurchase);
			readPrivateAppData(_analogCopyPurchase);
			appDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_packageId);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x0f);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_packageId);
			readPrivateAppData(_packageId);
			appDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_heartbeat);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x80);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_heartbeat);
			readPrivateAppData(_heartbeat);
			appDataDescriptorCount++;
		}
		itorMd = metadata.find(CRMetaData_lscpIpProtocol);
		if(itorMd != metadata.end() && !itorMd->second.empty())
		{
			_privateDataBytes.push_back(0x81);
			_privateDataBytes.push_back(0x04);
			sscanf(itorMd->second.c_str(), "%u", &_lscpIpProtocol);
			readPrivateAppData(_lscpIpProtocol);
			appDataDescriptorCount++;
		}
		_privateDataBytes[appDataLenthOffset] = appDataDescriptorCount * 6 + 3; //appdata length(tag 1 length 1 data 4, ProtocolId 1, ProtocolVersion 1, DescriptorCount 1)
		_privateDataBytes[appDataDescriptorCountOffset] = appDataDescriptorCount;
	}

	_privateDataBytes[2] = pdDescriptorCount;

	_messageLength += sizeof(uint16) + (uint16)_uuDataBytes.size() + sizeof(uint16) + (uint16)_privateDataBytes.size();


	if(_minBody.sessionId[0] == 0xFF)
	{
		_messageLength += sizeof(_streamHandel);

	}
	return true;
}
bool ClientSessionSetupConfirm::readPrivateAppData(uint32 element)
{
	uint8 buf[4];
	memset(buf, 0, sizeof(buf));
	uint32 uTemp = htonl(element);
	memcpy(buf, &uTemp, sizeof(uTemp));
	_privateDataBytes.insert(_privateDataBytes.begin() + _privateDataBytes.size(), buf, buf + 4);
	return true;
}
bool ClientSessionSetupConfirm::readResource(const DsmccResources dsmssResources)
{
   uint16 resCount = (uint16)dsmssResources.size();
   _messageLength += sizeof(uint16);

   if(dsmssResources.size() < 1)
   {
	   _resources.clear();
	   return true;
   }
   if(_resources.size() > 0)
   {
	   Resource& resource = _resources[0];
	   if(resource.resCommonHeader.resDescriptorType == 0xf003)
		   _messageLength += sizeof(resource.resCommonHeader) - sizeof(uint32) *2 + 26;
   }

   for(uint16 i = 0; i < resCount; i++)
   {
	  uint16  resHeadLength = 0;
	  const DsmccResource& dsmccRes = dsmssResources[i];
	  StringMap::const_iterator itorMd;
      Resource resource;
	  //default From 1 to....
	  itorMd = dsmccRes.resCommonHeader.find(CRMetaData_RESresRequestId);
	  if(itorMd == dsmccRes.resCommonHeader.end() || itorMd->second.empty())
	  {
		   resource.resCommonHeader.resRequestId = (uint16)(i+1);
	  }
	  else
	  {
		  resource.resCommonHeader.resRequestId = (uint16)atoi(itorMd->second.c_str());
	  }

      // if no have resDescriptorType, continue;
	  itorMd = dsmccRes.resCommonHeader.find(CRMetaData_RESresDescriptorType);
	  if(itorMd == dsmccRes.resCommonHeader.end() || itorMd->second.empty())
	  {
		  continue;
	  }
	  resource.resCommonHeader.resDescriptorType = (uint16)atoi(itorMd->second.c_str());


	  itorMd = dsmccRes.resCommonHeader.find(CRMetaData_RESresNum);
	  if(itorMd != dsmccRes.resCommonHeader.end() && !itorMd->second.empty())
	  {
		  resource.resCommonHeader.resNum = (uint16)atoi(itorMd->second.c_str());
	  }

	  itorMd = dsmccRes.resCommonHeader.find(CRMetaData_RESassociationTag);
	  if(itorMd != dsmccRes.resCommonHeader.end() && !itorMd->second.empty())
	  {
		  resource.resCommonHeader.associationTag = (uint16)atoi(itorMd->second.c_str());
	  }
	  else
	  {
		  resource.resCommonHeader.associationTag =  0xbfff;
	  }

	  //default value 0x43
	  itorMd = dsmccRes.resCommonHeader.find(CRMetaData_RESresFlags);
	  if(itorMd == dsmccRes.resCommonHeader.end() || itorMd->second.empty())
	  {
		  resource.resCommonHeader.resFlags = (uint8)0x43;
	  }		
	  else
		  resource.resCommonHeader.resFlags = (uint8)atoi(itorMd->second.c_str());


	  //default value 0x04
	  itorMd = dsmccRes.resCommonHeader.find(CRMetaData_RESresStatus);
	  if(itorMd == dsmccRes.resCommonHeader.end() || itorMd->second.empty())
	  {
		   resource.resCommonHeader.resStatus = (uint8)0x04;
	  }
	  else
	     resource.resCommonHeader.resStatus = (uint8)atoi(itorMd->second.c_str());

	  //´ÕÊý default value
	  itorMd = dsmccRes.resCommonHeader.find(CRMetaData_RESresLength);
	  if(itorMd != dsmccRes.resCommonHeader.end() && !itorMd->second.empty())
	  {
		  resource.resCommonHeader.resLength = (uint16)atoi(itorMd->second.c_str());
	  }
      //´ÕÊý default value
	  itorMd = dsmccRes.resCommonHeader.find(CRMetaData_RESresDataFieldCount);
	  if(itorMd != dsmccRes.resCommonHeader.end() && !itorMd->second.empty())
	  {
		  resource.resCommonHeader.resDataFieldCount = (uint16)atoi(itorMd->second.c_str());
	  }

	  resHeadLength = sizeof(resource.resCommonHeader) - sizeof(uint32) *2;

	  if(resource.resCommonHeader.resDescriptorType == 0xffff)
	  {
		  itorMd = dsmccRes.resCommonHeader.find(CRMetaData_REStypeOwnerId);
		  if(itorMd == dsmccRes.resCommonHeader.end() || itorMd->second.empty())
		  {
			  return false;
		  }
		  sscanf(itorMd->second.c_str(), "%u", resource.resCommonHeader.typeOwnerId);

		  itorMd = dsmccRes.resCommonHeader.find(CRMetaData_REStypeOwnerValue);
		  if(itorMd == dsmccRes.resCommonHeader.end() || itorMd->second.empty())
		  {
			  return false;
		  }
		  sscanf(itorMd->second.c_str(), "%u", resource.resCommonHeader.typeOwnerValue);

		  resHeadLength += 6;
	  }

	  ResourceDataField resDataField;
	  const TianShanIce::ValueMap&  resData = dsmccRes.resource.resourceData;	 
	  TianShanIce::ValueMap::const_iterator itorVM;

	  //read resDataFields
	  switch(resource.resCommonHeader.resDescriptorType)
	  {
	  case RsrcType_MPEG_PROGRAM:
		  {
			  resource.resCommonHeader.resDataFieldCount = 5;
	          resource.resCommonHeader.resLength = 16;
			  resource.resCommonHeader.resNum = 0xc001;

			  MpegProgramRsrc_t  mpegPR;
			  itorVM = resData.find("Id");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtInts || itorVM->second.bRange != false || itorVM->second.ints.empty())
				  return false;
			  mpegPR.pnValue = htons((uint16)itorVM->second.ints[0]);
			  mpegPR.pnType = htons(0x0001);

			  itorVM = resData.find("PmtPid");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtInts || itorVM->second.bRange != false || itorVM->second.ints.empty())
			  {
				  mpegPR.pmtPidValue = htons((uint16)0xFFFF);
			  }
			  else
				  mpegPR.pmtPidValue = htons((uint16)itorVM->second.ints[0]);
			  mpegPR.pmtPidType = htons(0x0001);

			  itorVM = resData.find("CaPid");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtInts || itorVM->second.bRange != false || itorVM->second.ints.empty())
			  {
				  mpegPR.caPidValue = htons((uint16)0xFFFF);
			  }
			  else
				  mpegPR.caPidValue = htons((uint16)itorVM->second.ints[0]);

			  mpegPR.elemStreamCountValue = htons((uint16)0);

			  mpegPR.pcrType = htons((uint16)0x0001);
			  mpegPR.pcrValue = htons((uint16)0xffff); //??????????????

			  char temp[16]; memset(temp, 0 , 16);
			  memcpy(temp, &mpegPR, sizeof(mpegPR));
			  resDataField.resDataValues.assign(temp, temp + sizeof(temp));
			 
			  break;
		  }
	  case RsrcType_PHYSICAL_CHANNEL:
		  {
			  resource.resCommonHeader.resDataFieldCount = 2;
			  resource.resCommonHeader.resLength = 8;
			  resource.resCommonHeader.resNum = 0xc002;

			  PhysicalChannelRsrc_t phyChannel;
			  itorVM = resData.find("channelId");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtInts || itorVM->second.bRange != false || itorVM->second.ints.empty())
				  return false;
			  phyChannel.channelValue = htonl((uint32)itorVM->second.ints[0]);
			  phyChannel.channelType = htons(0x0001);
			  phyChannel.directionValue = htons(0x0000); ///?????????????????

			  char tempphyChannel[8]; memset(tempphyChannel, 0 , 8);
			  memcpy(tempphyChannel, &phyChannel, sizeof(phyChannel));
			  resDataField.resDataValues.assign(tempphyChannel, tempphyChannel + sizeof(tempphyChannel)); 
		  } 
		  break;
  case RsrcType_TS_DOWNSTREAM_BW:
		  {
			  resource.resCommonHeader.resDataFieldCount = 2;
			  resource.resCommonHeader.resLength = 12;
			  resource.resCommonHeader.resNum = 0x8003;

			  TransportStreamDownstreamBwRsrc_t tsDownStreamBw;
			  itorVM = resData.find("bandwidth");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtLongs || itorVM->second.bRange != false || itorVM->second.lints.empty())
				  return false;
			  tsDownStreamBw.bwValue = htonl((uint32)itorVM->second.lints[0]);
			  tsDownStreamBw.bwType = htons((uint16)0x0001);

			  itorVM = resData.find("tsid");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtLongs || itorVM->second.bRange != false || itorVM->second.lints.empty())
			  {
				  tsDownStreamBw.idValue = htonl(0x00000000);
			  }
			  else
				  tsDownStreamBw.idValue = htonl((uint32)itorVM->second.lints[0]);
			  tsDownStreamBw.idType = htons(0x0001);

			  char tempTs[12]; memset(tempTs, 0 , 12);
			  memcpy(tempTs, &tsDownStreamBw, sizeof(tsDownStreamBw));
			  resDataField.resDataValues.assign(tempTs, tempTs + sizeof(tempTs));
		  }
		  break;
	  case RsrcType_SA_ATSC_MODULATION_MODE:
		  {
			  resource.resCommonHeader.resDataFieldCount = 9;
			  resource.resCommonHeader.resLength = 12;
			  resource.resCommonHeader.resNum = 0xc004;

			  SaAtscModulationModeRsrc_t saModulationModa;

			  saModulationModa.innerCodingMode = 0x0F;
			  saModulationModa.splitBitStreamMode = 0x00;

			  itorVM = resData.find("symbolRate");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtInts || itorVM->second.bRange != false || itorVM->second.ints.empty())
			  {
				  saModulationModa.symbolRate = htonl(0x004c4b40);
			  }
			  else
				  saModulationModa.symbolRate = htonl(itorVM->second.ints[0]);

			  itorVM = resData.find("modulationFormat");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtInts || itorVM->second.bRange != false || itorVM->second.ints.empty())
				  return false;
			  saModulationModa.modulationFormat = (uint8)(itorVM->second.ints[0]);

//			  saModulationModa.symbolRate =htonl((uint32) 0x004C4B40);

			  itorVM = resData.find("transmissionSystem");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtBin || itorVM->second.bRange != false || itorVM->second.bin.empty())
			  {
				  saModulationModa.transmissionSystem = 0x02;
			  }
			  else
				  saModulationModa.transmissionSystem = itorVM->second.bin[0];

			  saModulationModa.reserved = 0;

			  itorVM = resData.find("interleaveDepth");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtBin || itorVM->second.bRange != false || itorVM->second.bin.empty())
			  {  
				  saModulationModa.interleaveDepth = 0xFF;
			  }
			  else
				  saModulationModa.interleaveDepth = itorVM->second.bin[0];

			  itorVM = resData.find("modulationMode");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtBin || itorVM->second.bRange != false || itorVM->second.bin.empty())
			  {  
				  saModulationModa.modulationMode = 0x00;
			  }
			  else
				  saModulationModa.modulationMode = itorVM->second.bin[0];

			  itorVM = resData.find("FEC");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtBin || itorVM->second.bRange != false || itorVM->second.bin.empty())
			  {  
				  saModulationModa.forwardErrorCorrection = 0x00;
			  }
			  else
				  saModulationModa.forwardErrorCorrection = itorVM->second.bin[0];

			  char temp[12]; memset(temp, 0 , 12);
			  memcpy(temp, &saModulationModa, sizeof(saModulationModa));
			  resDataField.resDataValues.assign(temp, temp + sizeof(temp));  
		  }
		  break;
	  case RsrcType_SA_HEAD_END_ID:
		  {
			  resource.resCommonHeader.resDataFieldCount = 3;
			  resource.resCommonHeader.resLength = 26;
			  resource.resCommonHeader.resNum = 0xc005;

			  SaHeadEndIdRsrc_t   saheadendid;
			  itorVM = resData.find("id");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtBin || itorVM->second.bRange != true || itorVM->second.bin.size() != 20)
			  {  
				  saheadendid.headEndId.assign(0, 20);
			  }
			  else
				  saheadendid.headEndId = itorVM->second.bin;

			  itorVM = resData.find("flag");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtLongs || itorVM->second.bRange != false || itorVM->second.lints.empty())
			  {  
				  saheadendid.headEndFlag = htons(0x0001);
			  }
			  else
				  saheadendid.headEndFlag = htons((uint16)itorVM->second.lints[0]);

			  itorVM = resData.find("tsid");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtInts || itorVM->second.bRange != false || itorVM->second.ints.empty())
			  {
				  saheadendid.tsid = htonl(0x00000000);
			  }
			  else
				  saheadendid.tsid = htonl(itorVM->second.ints[0]);

			  char temp[26]; memset(temp, 0 , 26);
			  memcpy(temp, &saheadendid, sizeof(saheadendid));
			  resDataField.resDataValues.assign(temp, temp + sizeof(temp)); 
		  }
		  break;
	  case RsrcType_SA_ETHERNET_INTERFACE:
		  {
			  resource.resCommonHeader.resDataFieldCount = 6;
			  resource.resCommonHeader.resLength = 48;
			  resource.resCommonHeader.resNum = 0xc006;

			  SaEthernetInterfaceRsrc_NoSource_t ethernetInterface;
			  ethernetInterface.sourceUdpPortType =  htons(0x0002);          // ResourceValueType_LIST_VALUE
			  ethernetInterface.sourceUdpPortCount = htons(0x0001);         // = 1
			  itorVM = resData.find("srcPort");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtInts|| itorVM->second.ints.empty())
			  {
				  ethernetInterface.sourceUdpPort = htons(0x0000);
			  }
			  else
				  ethernetInterface.sourceUdpPort = htons(itorVM->second.ints[0]);

			  ethernetInterface.sourceIpAddressType  = htons(0x0002);        // ResourceValueType_LIST_VALUE
			  ethernetInterface.sourceIpAddressCount = htons(0x0001);       // = 1
			  itorVM = resData.find("srcIP");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtStrings || itorVM->second.strs.empty())
			  {
				  ethernetInterface.sourceIpAddress = htonl(0x00000000);
			  }
			  else
			  {
				  ethernetInterface.sourceIpAddress = inet_addr(itorVM->second.strs[0].c_str());              // = 1
			  }

			  ethernetInterface.sourceMacAddressType = htons(0x0002);       // ResourceValueType_LIST_VALUE
			  ethernetInterface.sourceMacAddressCount = htons(0x0001);      // = 1
			  itorVM = resData.find("srcMac");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtStrings || itorVM->second.strs.empty())
			  {
				  memset( ethernetInterface.sourceMacAddress, 0, sizeof(ethernetInterface.sourceMacAddress));
			  }
			  else
			  {
				  uint8 srcMacTemp[6]; 
				  memset(srcMacTemp, 0, 6);
				  StringToHex(itorVM->second.strs[0],srcMacTemp, 6);
				  memcpy(ethernetInterface.sourceMacAddress, srcMacTemp, 6);
			  }

			  ethernetInterface.destinationUdpPortType = htons(0x0002);     // ResourceValueType_LIST_VALUE
			  ethernetInterface.destinationUdpPortCount = htons(0x0001);    // = 1
			  itorVM = resData.find("destPort");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtInts || itorVM->second.ints.empty())
			  {
				  ethernetInterface.destinationUdpPort = 0x00;
			  }
			  else
				  ethernetInterface.destinationUdpPort = htons(itorVM->second.ints[0]);         // Pipe calculated port number

			  ethernetInterface.destinationIpAddressType = htons(0x0002);   // ResourceValueType_LIST_VALUE
			  ethernetInterface.destinationIpAddressCount = htons(0x0001);  // = 1
			  itorVM = resData.find("destIP");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtStrings || itorVM->second.strs.empty())
			  {
				  ethernetInterface.destinationIpAddress = 0x00000000;
			  }
			  else
			  {
				 ethernetInterface.destinationIpAddress = inet_addr(itorVM->second.strs[0].c_str()); // Pipe::IpTrnIp
			  }

			  ethernetInterface.destinationMacAddressType = htons(0x0002);  // ResourceValueType_LIST_VALUE
			  ethernetInterface.destinationMacAddressCount = htons(0x0001); // = 1
			  itorVM = resData.find("destMac");
			  if(itorVM == resData.end() || itorVM->second.type != TianShanIce::vtStrings || itorVM->second.strs.empty())
				  return false; 
			  
			  uint8 desMacTemp[6]; 
			  memset(desMacTemp, 0, 6);
			  StringToHex(itorVM->second.strs[0],desMacTemp, 6);
			  memcpy(ethernetInterface.destinationMacAddress, desMacTemp, 6);

			  char temp[48]; memset(temp, 0 , 48);
			  memcpy(temp, &ethernetInterface, sizeof(ethernetInterface));
			  resDataField.resDataValues.assign(temp, temp + sizeof(temp));
		  }
		  break;
	  case 0x0000:
	  default:
		  resource.resCommonHeader.resLength = 0;
		  break;
	  }
	  if(resDataField.resDataValues.size() > 0)
	  {
		  resource.resDataFields.push_back(resDataField);
		  _resources.push_back(resource);
		  _messageLength += resource.resCommonHeader.resLength + resHeadLength;
	  }
   }
   return true;
}
void ClientSessionSetupConfirm::dumpPrivateData()
{
	//dump miniBody
	std::string sessionId;
	HexToString(sessionId, _minBody.sessionId, sizeof(_minBody.sessionId));

	std::string serverId;
	HexToString(serverId, _minBody.serverId, sizeof(_minBody.serverId));

	_debugLog(ZQ::common::Log::L_INFO, CLOGFMT(ClientSessionSetupConfirm, "sessionId(%s) response(%u) serverId(%s)"), sessionId.c_str(), _minBody.response, serverId.c_str());

	_debugLog(ZQ::common::Log::L_INFO, CLOGFMT(ClientSessionSetupConfirm, "AppResponsetData: IpAddress(0x%08x) port(0x%08x) streamHandel(0x%08d) billingId(%u) purchaseTime(%u)remainingPlayTime(%u) errorCode(%u) homeId(%u) purchaseId(0x%08x) smartCardId(0x%08x) analogCopyPurchase(%u) packageId(%u) heartbeat(%u) lscpIpProtocol(%u)"),
		_IpAddress, _IpPort, _streamHandel, _billingId, _purchaseTime, _remainingPlayTime, _errorCode, _homeId, _purchaseId, _smartCardId, _analogCopyPurchase, _packageId, _heartbeat,_lscpIpProtocol);

	_debugLog(ZQ::common::Log::L_INFO, CLOGFMT(ClientSessionSetupConfirm, "FunctionData: billingId(%u) purchaseTime(%u)remainingPlayTime(%u) errorCode(%u) homeId(%u) purchaseId(0x%08x) smartCardId(0x%08x) analogCopyPurchase(%u) packageId(%u)"),
		_billingId, _purchaseTime, _remainingPlayTime, _errorCode, _homeId, _purchaseId, _smartCardId, _analogCopyPurchase, _packageId);

}
//////////////////////////////////////////////////////////////////////
///////////////  class ClientSessionReleaseRequest  ////////////////////
/////////////// //////////////////////////////////////////////////////
uint16 ClientSessionRelease::formatMessageBody(uint8* buf, size_t maxLen)
{
	uint8* p = buf;
	if (NULL == p)
		return 0;
	if(maxLen < _messageLength)
		return -1;

	MinBody tmpMinBody = _minBody;
	tmpMinBody.reason = htons(_minBody.reason);
	memcpy(p, &tmpMinBody, sizeof(tmpMinBody)); p+= sizeof(tmpMinBody);

	// append with uudata
	size_t processedlength = 0;
	if(!formatUUData(p, maxLen - (uint16)(p - buf), processedlength))
		return -1;
	p += processedlength;

	//append with privatedata
	processedlength = 0;
	if(!formatUserPrivateData(p, maxLen - (uint16)(p - buf), processedlength))
		return -1;
	p += processedlength;

	return (uint16)(p - buf);
}
bool ClientSessionRelease::parseMessageBody(const uint8* buf, size_t messageLength)
{
	const uint8* p = buf;
	if (NULL == p)
		return false;

	size_t processedLength =  0;
	_messageLength = (uint16)messageLength;

	if(messageLength < sizeof(_minBody))
		return false;

	//the minBody
	memcpy(&_minBody, p, sizeof(_minBody)); p+= sizeof(_minBody); processedLength += sizeof(_minBody);
	_minBody.reason = ntohs(_minBody.reason);

	//the UUData
	size_t byteprocessed = 0;
	if(!parseUUData(p, _messageLength - processedLength, byteprocessed))
		return false;
	p += byteprocessed; processedLength += byteprocessed;

	//the privateData
	byteprocessed = 0;
	if(!parseUserPrivateData(p, _messageLength - processedLength, byteprocessed))
		return false;
	p += byteprocessed; processedLength += byteprocessed; 

	if(processedLength != messageLength)
		return false;	

	if(!parsePrivateData())
		return false;
	return true;
}
uint32 ClientSessionRelease::toMetaData(StringMap& metadata)
{
   toDsmccHeader(metadata);

   std::string sessionId;
   HexToString(sessionId, _minBody.sessionId, sizeof(_minBody.sessionId));

   char strTemp[33];
   memset(strTemp, 0, sizeof(32));
   itoa(_minBody.reason, strTemp, 10);
   std::string reason = strTemp;
   
   MAPSET(StringMap, metadata, CRMetaData_SessionId, sessionId);
   MAPSET(StringMap, metadata, CRMetaData_CSRreason, reason);

   return (uint32)metadata.size();
}
bool ClientSessionRelease::readMetaData(const StringMap& metadata)
{
	if(!readDmssHeader(metadata))
		return false;

    _messageLength =  0;

	StringMap::const_iterator itorMd;
	itorMd = metadata.find(CRMetaData_SessionId);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	const std::string& strSessId = itorMd->second;
	StringToHex(strSessId, _minBody.sessionId, sizeof(_minBody.sessionId));

	itorMd = metadata.find(CRMetaData_CSRreason);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	_minBody.reason =(uint16)atoi(itorMd->second.c_str());
     //messageLenth = 16;ReleaseRequest,ReleaseConfirm,ReleaseIndication, ReleaseResponse 
	_messageLength += sizeof(_minBody) + sizeof(uint16) + (uint16)_uuDataBytes.size() +  sizeof(uint16) + (uint16)_privateDataBytes.size();
	return true;
}
 void  ClientSessionRelease::dumpPrivateData()
{
	//dump mini body
	std::string sessionId;
	HexToString(sessionId, _minBody.sessionId,  sizeof(_minBody.sessionId));

	_debugLog(ZQ::common::Log::L_INFO, CLOGFMT(ClientSessionRelease, "sessionId(%s) reason(%u)"), sessionId.c_str(), _minBody.reason);
}
//////////////////////////////////////////////////////////////////////
///////////////  class ClientSessionReleaseRequest  ////////////////////
/////////////// //////////////////////////////////////////////////////
bool ClientSessionReleaseRequest::parsePrivateData()
{
	return true;
}
/*int ClientSessionReleaseRequest::toMetaData(StringMap& metadata)
{
	toDsmccHeader(metadata);
   return metadata.size();
}
bool ClientSessionReleaseRequest::readMetaData(const StringMap& metadata)
{
	if(!readDmssHeader(metadata))
		return false;
	return true;
}*/
//////////////////////////////////////////////////////////////////////
///////////////  class ClientSessionReleaseConfirm  ////////////////////
/////////////// //////////////////////////////////////////////////////
bool ClientSessionReleaseConfirm::parsePrivateData()
{
	return true;
}
/*int ClientSessionReleaseConfirm::toMetaData(StringMap& metadata)
{
	toDsmccHeader(metadata);
   return metadata.size();
}
bool ClientSessionReleaseConfirm::readMetaData(const StringMap& metadata)
{
	if(!readDmssHeader(metadata))
		return false;
	return true;
}*/
//////////////////////////////////////////////////////////////////////
///////////////  class ClientSessionReleaseIndication  ////////////////////
/////////////// //////////////////////////////////////////////////////
bool ClientSessionReleaseIndication::parsePrivateData()
{
	return true;
}
/*int ClientSessionReleaseIndication::toMetaData(StringMap& metadata)
{
   toDsmccHeader(metadata);
   return metadata.size();
}
bool ClientSessionReleaseIndication::readMetaData(const StringMap& metadata)
{
	if(!readDmssHeader(metadata))
		return false;
	return true;
}*/
//////////////////////////////////////////////////////////////////////
///////////////  class ClientSessionReleaseResponse  ////////////////////
/////////////// //////////////////////////////////////////////////////
bool ClientSessionReleaseResponse::parsePrivateData()
{
	return true;
}
/*int ClientSessionReleaseResponse::toMetaData(StringMap& metadata)
{
  toDsmccHeader(metadata);
  return metadata.size();
}
bool  ClientSessionReleaseResponse::readMetaData(const StringMap& metadata)
{
	if(!readDmssHeader(metadata))
		return false;
	return true;
}*/
//////////////////////////////////////////////////////////////////////
///////////////  class ClientSessionProceedingIndication  ////////////////////
/////////////// //////////////////////////////////////////////////////
uint16 ClientSessionProceedingIndication::formatMessageBody(uint8* buf, size_t maxLen)
{
	uint8* p = buf;
	if (NULL == p)
		return 0;

	if(maxLen < _messageLength)
		return -1;
    MinBody tmpMinbody = _minBody;
	tmpMinbody.reason = htons(_minBody.reason);
	memcpy(p, &tmpMinbody, sizeof(tmpMinbody)); p+= sizeof(tmpMinbody);

	return (uint16)(p - buf);
}
bool ClientSessionProceedingIndication::parseMessageBody(const uint8* buf, size_t messageLength)
{
	const uint8* p = buf;
	if (NULL == p)
		return false;

	size_t processedLength =  0;
	_messageLength = (uint16)messageLength;

	if(messageLength < sizeof(_minBody))
		return false;

	//the minBody
	memcpy(&_minBody, p, sizeof(_minBody)); p+= sizeof(_minBody); processedLength += sizeof(_minBody);
    _minBody.reason = ntohs(_minBody.reason);

	if(processedLength != messageLength)
		return false;	

	return true;
}
uint32 ClientSessionProceedingIndication::toMetaData(StringMap& metadata)
{
	toDsmccHeader(metadata);

	std::string sessionId;
	HexToString(sessionId, _minBody.sessionId, sizeof(_minBody.sessionId));

	char strTemp[33];
	memset(strTemp, 0, sizeof(33));
	itoa(_minBody.reason, strTemp, 10);
	std::string reason = strTemp;

	MAPSET(StringMap, metadata, CRMetaData_SessionId, sessionId);
	MAPSET(StringMap, metadata, CRMetaData_CSPIreason, reason);

	return (uint32)metadata.size();
}
bool ClientSessionProceedingIndication::readMetaData(const StringMap& metadata)
{
	if(!readDmssHeader(metadata))
		return false;

	_messageLength = 0;
	StringMap::const_iterator itorMd;
	itorMd = metadata.find(CRMetaData_SessionId);
	if(itorMd == metadata.end() || itorMd->second.empty()|| itorMd->second.size() != 20)
	{
		return false;
	}
	const std::string& strSessId = itorMd->second ;  
	StringToHex(strSessId, _minBody.sessionId, sizeof(_minBody.sessionId));

	itorMd = metadata.find(CRMetaData_CSPIreason);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	_minBody.reason =(uint16) atoi(itorMd->second.c_str());

	//messageLenth = 12;
	_messageLength += sizeof(_minBody);
	return true;
}
void  ClientSessionProceedingIndication::dumpPrivateData()
{
	//dump mini body
	std::string sessionId;
	HexToString(sessionId, _minBody.sessionId,  sizeof(_minBody.sessionId));

	_debugLog(ZQ::common::Log::L_INFO, CLOGFMT(ClientSessionProceedingIndication, "sessionId(%s) reason(%u)"), sessionId.c_str(), _minBody.reason);
}
//////////////////////////////////////////////////////////////////////
///////////////  class ClientSessionInProgressRequest  ////////////////////
/////////////// //////////////////////////////////////////////////////
uint16 ClientSessionInProgressRequest::formatMessageBody(uint8* buf, size_t maxLen)
{
	uint8* p = buf;
	if (NULL == p)
		return 0;

	if(maxLen < _messageLength)
		return -1;
	uint16 sessionCount = htons((uint16)sessionIds.size());
	memcpy(p, &sessionCount, sizeof(sessionCount)); p += sizeof(sessionCount);

	for(uint16 i = 0; i < sessionIds.size(); i++)
	{
		for(uint16 j = 0; j < 10; j++)
		{
          memcpy(p, &sessionIds[i][j], sizeof(sessionIds[i][j]));p += sizeof(sessionIds[i][j]);
		}
	}

	return (uint16)(p - buf);
}
bool ClientSessionInProgressRequest::parseMessageBody(const uint8* buf, size_t messageLength)
{
	const uint8* p = buf;
	if (NULL == p)
		return false;

	size_t processedLength =  0;
	_messageLength = (uint16)messageLength;

	uint16 sessionCount;
	if(messageLength < sizeof(sessionCount))
		return false;

	//the sessionCount
	memcpy(&sessionCount, p, sizeof(sessionCount)); p+= sizeof(sessionCount); processedLength += sizeof(sessionCount);
    sessionCount =  ntohs(sessionCount);

	//the sessionIds
	if(sessionCount > 0)
	{
	  if(messageLength < processedLength + sessionCount*10)
		  return false;
      for(uint16 i = 0; i < sessionCount; i++)
	  {
		  Bytes sessionId;
		  sessionId.assign(p, p+10); p += 10;
		  sessionIds.push_back(sessionId);
		  processedLength += 10;
	  }
	}
/*	if(processedLength != messageLength)
		return false;	*/

	return true;
}
uint32 ClientSessionInProgressRequest::toMetaData(StringMap& metadata)
{
   toDsmccHeader(metadata);

   uint16 sessionCount = (uint16)sessionIds.size();

   char strTemp[33];
   memset(strTemp, 0, sizeof(33));
   itoa(sessionCount, strTemp, 10);
   std::string strSessionCount = strTemp;
   MAPSET(StringMap, metadata, CRMetaData_CSPRsessionCount, strSessionCount);

   for(uint i = 0; i < sessionCount; i++)
   {
	   Bytes& sessionId = sessionIds[i];
	   //	   std::string strSessId(sessionId.begin(), sessionId.end());
	   std::string strSessId;
	   HexToString(strSessId, sessionId);

	   char strTemp[32] = "";
	   memset(strTemp, 0, sizeof(strTemp));
	   sprintf(strTemp,"%d", i);
	   std::string strKey = CSPRSessionID_ +std::string(strTemp);
	   MAPSET(StringMap, metadata, strKey, strSessId);

	   if(i==0)
		   MAPSET(StringMap, metadata, CRMetaData_SessionId, strSessId);
   }

   return (uint32)metadata.size();
}
bool ClientSessionInProgressRequest::readMetaData(const StringMap& metadata)
{
	if(!readDmssHeader(metadata))
		return false;

    _messageLength = 0;

	StringMap::const_iterator itorMd;
	itorMd = metadata.find(CRMetaData_CSPRsessionCount);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
    
	uint16 sessionCount = (uint16)atoi(itorMd->second.c_str());
	//messageLenth = 12;
	_messageLength += sizeof(uint16);

	sessionIds.clear();
	for(uint16 i = 0; i < sessionCount; i++)
	{
	    char strTemp[32] = "";
		memset(strTemp, 0, sizeof(strTemp));
		sprintf(strTemp,"%d", i);
		std::string strKey = CSPRSessionID_ +std::string(strTemp);

		itorMd = metadata.find(strKey);
		if(itorMd == metadata.end() || itorMd->second.empty() || itorMd->second.size() != 20)
		{
			return false;
		}
		const std::string& strSessId = itorMd->second;
		Bytes  sessionID;
		StringToHex(strSessId, sessionID);
		sessionIds.push_back(sessionID);
		_messageLength += (uint16)strSessId.size();
	}

	return true;
}
void  ClientSessionInProgressRequest::dumpPrivateData()
{
	//dump mini body

	uint16 sessionCount = (uint16)sessionIds.size();

	for(uint i = 0; i < sessionCount; i++)
	{
		Bytes& sessionId = sessionIds[i];
		//	   std::string strSessId(sessionId.begin(), sessionId.end());
		std::string strSessId;
		HexToString(strSessId, sessionId);
		_debugLog(ZQ::common::Log::L_INFO, CLOGFMT(ClientSessionProceedingIndication, "sessionId(%s)"), strSessId.c_str());
	}
}

}// end namespace DSMCC
}// end namespace ZQ
#pragma pack()
