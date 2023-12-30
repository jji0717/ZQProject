#include "ERRPMsg.h"
#include <map>

#ifdef ZQ_OS_LINUX
#include<netinet/in.h>
#endif

namespace ZQ {
	namespace ERRP{

//////////////////////////////////////////////////////////////////////
///////////////  class ERRPMsg       ////////////////////////////////
/////////////// //////////////////////////////////////////////////////
ERRPMsg::ERRPMsg(HardHeader& hardHeader)
 : _hardHeader(hardHeader),_messageLength(0)
{
	_log.setVerbosity(ZQ::common::Log::L_DEBUG);
}
ERRPMsg::~ERRPMsg()
{

}
int ERRPMsg::toMessage(uint8* buf, size_t maxLen)
{
	//dumpLog();
	uint8* p = buf;
	
	uint16 messageLength = getMessageLength();//  读取 Message 的长度。
	if (NULL ==p || maxLen < sizeof(_hardHeader) + messageLength)
		return 0;

	HardHeader tmpHardHeader = _hardHeader;
	tmpHardHeader.msgType = _hardHeader.msgType;
	tmpHardHeader.Length = htons(sizeof(_hardHeader) + messageLength);
	memcpy(p, &tmpHardHeader, sizeof(tmpHardHeader)); 
	p += sizeof(tmpHardHeader);

	if(_hardHeader.msgType == MsgType_KEEPALIVE)
		return sizeof(_hardHeader);
	uint16 messageSize = 0; 
	if(messageLength > 0)
		messageSize = formatMessageBody(p, messageLength);
	if(0 == messageSize)
	{
#ifdef _DEBUG
		printf("formatMessageBody failed\n");
#endif
		return 0;
	}

	return (int)(sizeof(_hardHeader) + messageSize);
}
	
uint8 ERRPMsg::getMessageType() 
{ 
	return _hardHeader.msgType;
}
	
bool ERRPMsg::parseMessageBody(const uint8* p, size_t messageLength)
{
	return true;
}

ERRPMsg::Ptr ERRPMsg::parseMessage(const uint8* buf, size_t maxLen, size_t& bytesProcessed)
{
	Ptr pMsg = NULL;
	const uint8* p = buf;
	bytesProcessed =0;
	HardHeader tmpHeader;
	if (NULL ==p || maxLen < sizeof(tmpHeader) )
		return pMsg;

	memcpy(&tmpHeader, p, sizeof(tmpHeader)); p += sizeof(tmpHeader);

	tmpHeader.Length = ntohs(tmpHeader.Length);

	uint16 messageLength =  tmpHeader.Length - sizeof(tmpHeader);
	//_messageLength = tmpHeader.Length - sizeof(tmpHeader);

	if (maxLen < tmpHeader.Length)
		return pMsg;

	switch(tmpHeader.msgType)
	{
	case MsgType_OPEN:
		pMsg = new OpenRequest(tmpHeader);
		break;
	case MsgType_UPDATE:
		pMsg = new UpdataRequest(tmpHeader);
		break;
	case MsgType_NOTIFICATION:                //ClientSessionReleaseRequest
		pMsg = new Notification(tmpHeader);
		break;
	case MsgType_KEEPALIVE:    
		pMsg = new KeepAliveRequest(tmpHeader);
		break;
	case 0x0000:
	default:
		pMsg = new ERRPMsg(tmpHeader);
	}

	if (pMsg)
	{
		///parse messageBody
		if (!pMsg->parseMessageBody(p, messageLength))
		{
#ifdef _DEBUG
			printf("parseMessageBodu failed\n");
#endif
			return NULL;
		}
		p += messageLength;
	}

	bytesProcessed = p - buf;
	if(bytesProcessed != tmpHeader.Length)
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(ERRPMsg::parseMessage,"wrong message length"));
		return NULL;
	}
	return pMsg;
}
uint32 ERRPMsg::toERRPHeader(StringMap& metadata)//set metadata
{
	if(_hardHeader.Length < 3 || _hardHeader.Length > 4096)
		return 0;

	//_messageLength = _hardHeader.Length - sizeof(_hardHeader);

	char strTemp[33] = "";
	memset(strTemp,0,sizeof(strTemp));
	itoa(_hardHeader.msgType,strTemp,10);
	std::string msgType = strTemp;

	MAPSET(ZQ::ERRP::StringMap,metadata,ERRPMD_MsgType,msgType);

	return (uint32)metadata.size();
}
uint32 ERRPMsg::toMetaData(StringMap& metadata)
{
	return toERRPHeader(metadata);
}
bool ERRPMsg::readMetaData(const StringMap& metadata)
{
	if(!readERRPHeader(metadata))
		return false;
	return true;
}
bool ERRPMsg::readERRPHeader(const StringMap& metadata)//从 MetaData 中读取 Header 中MessageType来填充msgType字段。
{	
	StringMap::const_iterator itorMD;
	itorMD = metadata.find(ERRPMD_MsgType);

	if(itorMD == metadata.end() || itorMD->second.empty())
		return false;
	_hardHeader.msgType = (uint8)atoi(itorMD->second.c_str());

	return true;
}

void ERRPMsg::dumpLog()
{
	//_debugLog(ZQ::common::Log::L_INFO,CLOGFMT(ERRPMsg,"MsgType: [%d], MsgLength: [%d]"),_hardHeader.msgType,_hardHeader.Length);
	printf("      MsgLength : [%d]\n",_hardHeader.Length);
	ZQ::ERRP::StringMap metaData;
	toMetaData(metaData);
	ZQ::ERRP::StringMap::iterator iter = metaData.begin();
	for(iter;iter != metaData.end();iter++)
	{
		std::cout<<iter->first<<" : ["<<iter->second<<"]"<<std::endl;
	}
}

void OpenRequest::dumpBody()
{

}

void UpdataRequest::dumpBody()
{
	
}

void Notification::dumpBody()
{
}

void KeepAliveRequest::dumpBody()
{
}

static uint16 bytesToString(ZQ::ERRP::ERRPMsg::Bytes bytes,char* buf)
{
	if(bytes.size() == 0)
		return false;

	uint16 msgLength = 0;
	ZQ::ERRP::ERRPMsg::Bytes::iterator iter = bytes.begin();
	for(iter;iter!=bytes.end();iter++)
	{
		uint8 type = *iter;
		memcpy(buf,&type,sizeof(uint8)); buf += sizeof(uint8);
		msgLength++;
	}

	return msgLength;
}

static bool stringToBytes(ZQ::ERRP::ERRPMsg::Bytes& bytes,const char* buf,size_t length)
{
	if(!buf)
		return false;

	const char* p = buf;
	for(size_t i = 0;i < length;i++)
	{
		uint8 byte = 0;
		memcpy(&byte,p,sizeof(uint8));p += sizeof(uint8);
		bytes.push_back(byte);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
///////////////  class OpenRequest  ////////////////////
//////////////////////////////////////////////////////////////////////
uint16 OpenRequest::formatCapabilityInfo(uint16 type,uint8* buf)
{
	uint8* parameterBuf = buf;
	uint16 parameterType = 0;
	uint16 parameterLength = 0;

	switch(type)
	{
	case CapabilityInfomation:
		{
			glog(ZQ::common::Log::L_INFO,CLOGFMT(OpenRequest,"format capability infomation"));
			parameterType = CapabilityInfomation;
			parameterType = htons(parameterType);
			memcpy(parameterBuf,&parameterType,sizeof(parameterType)); parameterBuf += sizeof(parameterType);
			parameterLength = (uint16)capabilityCodes.size()*8;
			parameterLength = htons((u_short)parameterLength);
			memcpy(parameterBuf,&parameterLength,sizeof(parameterLength));parameterBuf += sizeof(parameterLength);

			CapabilityCodes::iterator iter = capabilityCodes.begin();

			for(iter;iter!=capabilityCodes.end();iter++)
			{
				uint16 capabilityLength = 0;
				uint16 capabilityCode = *iter;
				if(capabilityLength > _messageLength)
					return 0;

				switch(capabilityCode)
				{
				case RouteTypeSupported:
					{
						if(addressFamiliy == 0 || applicationProtocol == 0)
							return 0;
						capabilityLength = sizeof(addressFamiliy) + sizeof(applicationProtocol);

						capabilityCode = htons(capabilityCode);
						capabilityLength = htons(capabilityLength);
						addressFamiliy = htons(addressFamiliy);
						applicationProtocol = htons(applicationProtocol);

						memcpy(parameterBuf,&capabilityCode,sizeof(capabilityCode)); parameterBuf += sizeof(capabilityCode);
						memcpy(parameterBuf,&capabilityLength,sizeof(capabilityLength)); parameterBuf += sizeof(capabilityLength);
						memcpy(parameterBuf,&addressFamiliy,sizeof(addressFamiliy)); parameterBuf += sizeof(addressFamiliy);
						memcpy(parameterBuf,&applicationProtocol,sizeof(applicationProtocol)); parameterBuf += sizeof(applicationProtocol);
					}
					break;
				case SendReceive:
					{
						capabilityLength = sizeof(sendReceivedMode);

						capabilityCode = htons(capabilityCode);
						capabilityLength = htons(capabilityLength);
						sendReceivedMode = htonl(sendReceivedMode);

						memcpy(parameterBuf,&capabilityCode,sizeof(capabilityCode)); parameterBuf += sizeof(capabilityCode);
						memcpy(parameterBuf,&capabilityLength,sizeof(capabilityLength)); parameterBuf += sizeof(capabilityLength);
						memcpy(parameterBuf,&sendReceivedMode,sizeof(sendReceivedMode)); parameterBuf += sizeof(sendReceivedMode);
					}
					break;
				case ERRPVersion:
					{
						if(errpVersion == 0)
							return 0;
						capabilityLength = sizeof(errpVersion);

						capabilityCode = htons(capabilityCode);
						capabilityLength = htons(capabilityLength);
						errpVersion = htonl(errpVersion);

						memcpy(parameterBuf,&capabilityCode,sizeof(capabilityCode)); parameterBuf += sizeof(capabilityCode);
						memcpy(parameterBuf,&capabilityLength,sizeof(capabilityLength)); parameterBuf += sizeof(capabilityLength);
						memcpy(parameterBuf,&errpVersion,sizeof(errpVersion)); parameterBuf += sizeof(errpVersion);
					}
					break;
				default:
					return 0;
				}
			}

			return (uint16)(parameterBuf - buf);
		}
	case StreamingZone:
		{
			glog(ZQ::common::Log::L_INFO,CLOGFMT(OpenRequest,"format streaming zone"));
			parameterType = StreamingZone;
			parameterLength = (uint16)streamingZone.length();

			parameterType = htons(parameterType);
			parameterLength = htons((u_short)parameterLength);
			
			memcpy(parameterBuf,&parameterType,sizeof(parameterType)); parameterBuf += sizeof(parameterType);
			memcpy(parameterBuf,&parameterLength,sizeof(parameterLength)); parameterBuf += sizeof(parameterLength);
			memcpy(parameterBuf,streamingZone.c_str(),streamingZone.length()); parameterBuf += streamingZone.length();
			uint16 nsize=(uint16)(parameterBuf - buf);
			return (uint16)(parameterBuf - buf);
		}
	case ComponentName:
		{
			glog(ZQ::common::Log::L_INFO,CLOGFMT(OpenRequest,"format component name"));
			parameterType = ComponentName;
			parameterLength = (uint16)componentName.length();

			parameterType = htons(parameterType);
			parameterLength = htons((u_short)parameterLength);

			memcpy(parameterBuf,&parameterType,sizeof(parameterType)); parameterBuf += sizeof(parameterType);
			memcpy(parameterBuf,&parameterLength,sizeof(parameterLength)); parameterBuf += sizeof(parameterLength);
			memcpy(parameterBuf,componentName.c_str(),componentName.length()); parameterBuf += componentName.length();

			return (uint16)(parameterBuf - buf);
		}
	case VendorSpecificString:
		{
			glog(ZQ::common::Log::L_INFO,CLOGFMT(OpenRequest,"format vendor specific string"));
			parameterType = VendorSpecificString;
			parameterLength = (uint16)vendorSpecificString.length();

			parameterType = htons(parameterType);
			parameterLength = htons((u_short)parameterLength);

			memcpy(parameterBuf,&parameterType,sizeof(parameterType)); parameterBuf += sizeof(parameterType);
			memcpy(parameterBuf,&parameterLength,sizeof(parameterLength)); parameterBuf += sizeof(parameterLength);
			memcpy(parameterBuf,vendorSpecificString.c_str(),vendorSpecificString.length()); parameterBuf += vendorSpecificString.length();

			return (uint16)(parameterBuf - buf);
		}
	default:
		return 0;
	}

	return (uint16)(parameterBuf - buf);
}

uint16 OpenRequest::formatMessageBody(uint8* buf, size_t maxLen)
{
	uint8* p = buf;
	if (NULL == p)
		return 0;

	if(maxLen < _messageLength)
		return 0;

	_minBody.holdTime = htons(_minBody.holdTime);
	_minBody.adddressDomain = htonl(_minBody.adddressDomain);
	_minBody.errpIdentifier = htonl(_minBody.errpIdentifier);
	_minBody.parametersLength = htons(_minBody.parametersLength);

	memcpy(p, &_minBody, sizeof(_minBody)); p += sizeof(_minBody);
    
	////......... format other parameter;
	if(_minBody.parametersLength == 0)
		return (uint16)(p - buf);

	if(streamingZone.empty() || componentName.empty() )
		return 0;
	uint16 capabiltyProcessed = 0;

	if(0 != capabilityCodes.size())
	{
		capabiltyProcessed = formatCapabilityInfo(CapabilityInfomation,p);//capability information
		if(capabiltyProcessed == 0)
			return 0;
		p += capabiltyProcessed;
	}

	capabiltyProcessed = formatCapabilityInfo(StreamingZone,p);
	if(capabiltyProcessed == 0)
		return 0;
	p += capabiltyProcessed;

	capabiltyProcessed = formatCapabilityInfo(ComponentName,p);
	if(capabiltyProcessed == 0)
		return 0;
	p += capabiltyProcessed;

	if(!vendorSpecificString.empty())
	{
		capabiltyProcessed = formatCapabilityInfo(VendorSpecificString,p);
		if(capabiltyProcessed == 0)
			return 0;
		p += capabiltyProcessed;
	}

    return (uint16)(p - buf);
}

bool OpenRequest::parseCapabilityInfo(uint16 parameterType,const uint8* buf,size_t& byteProcessed)
{
	const uint8* p = buf;
	uint16 parameterLength = 0;
	memcpy(&parameterLength,p,sizeof(parameterLength));
	parameterLength = ntohs(parameterLength);
	if(parameterLength > _messageLength)
		return false;
	p += sizeof(parameterLength);byteProcessed += sizeof(parameterLength);

	char temp[100];
	memset(temp,0,100);

	switch(parameterType)
	{
	case CapabilityInfomation: 
		{
			glog(ZQ::common::Log::L_INFO,CLOGFMT(OpenRequest,"parse capability info"));
			uint16 capabilityProcessed = 0;

			while(parameterLength > capabilityProcessed)
			{
				uint16 capabilityCode;
				memcpy(&capabilityCode,p,sizeof(capabilityCode));
				capabilityCode = ntohs(capabilityCode);
				capabilityCodes.push_back(capabilityCode);
				p += sizeof(capabilityCode);capabilityProcessed += sizeof(capabilityCode);

				uint16 capabilityLength = 0;
				memcpy(&capabilityLength,p,sizeof(capabilityLength));
				capabilityLength = ntohs(capabilityLength);
				p += sizeof(capabilityLength);capabilityProcessed += sizeof(capabilityLength);
				switch(capabilityCode)
				{
				case RouteTypeSupported://route type supported
					{
						memcpy(&addressFamiliy,p,sizeof(addressFamiliy));
						addressFamiliy = ntohs(addressFamiliy);
						p += sizeof(addressFamiliy);capabilityProcessed+= sizeof(addressFamiliy);
						memcpy(&applicationProtocol,p,sizeof(applicationProtocol));
						applicationProtocol = ntohs(applicationProtocol);
						p += sizeof(applicationProtocol);capabilityProcessed+= sizeof(applicationProtocol);
						break;
					}
				case SendReceive://send receive
					{
						memcpy(&sendReceivedMode,p,sizeof(sendReceivedMode));
						sendReceivedMode = ntohl(sendReceivedMode);
						p += sizeof(sendReceivedMode); capabilityProcessed += sizeof(sendReceivedMode);
						break;
					}
				case ERRPVersion://errp version
					{
						memcpy(&errpVersion,p,sizeof(errpVersion));
						errpVersion = ntohl(errpVersion);
						p += sizeof(errpVersion); capabilityProcessed += sizeof(errpVersion);
						break;
					}
				default://unsupported
					return false;
				}
			}
			if(parameterLength != capabilityProcessed)
				return false;
			break;
		}
	case StreamingZone:
		{	
			glog(ZQ::common::Log::L_INFO,CLOGFMT(OpenRequest,"parse streaming zone"));
			memcpy(temp,p,parameterLength);
			streamingZone = temp;
			break;
		}
	case ComponentName:
		{
			glog(ZQ::common::Log::L_INFO,CLOGFMT(OpenRequest,"parse component name"));
			memcpy(temp,p,parameterLength);
			componentName = temp;
			break;
		}
	case VendorSpecificString:
		{
			glog(ZQ::common::Log::L_INFO,CLOGFMT(OpenRequest,"parse vendor specific string"));
			memcpy(temp,p,parameterLength);
			vendorSpecificString = temp;
			break;
		}
	default:
		return false;
	}
	byteProcessed += parameterLength;
	
	return true;
}

bool OpenRequest::parseMessageBody(const uint8* buf, size_t messageLength)
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

	////......... parse other parameter;
	
	_minBody.holdTime =  ntohs(_minBody.holdTime);
	_minBody.adddressDomain = ntohl(_minBody.adddressDomain);
	_minBody.errpIdentifier = ntohl(_minBody.errpIdentifier);
	_minBody.parametersLength = ntohs(_minBody.parametersLength);

	if(_minBody.parametersLength == 0)
		return true;
	
	uint16 parameterType = 0;
	size_t parameterProcessed = 0;

	while(_minBody.parametersLength > parameterProcessed)
	{
		memcpy(&parameterType,p,sizeof(parameterType)); 
		p += sizeof(parameterType);parameterProcessed += sizeof(parameterType);
		parameterType = ntohs(parameterType);

		size_t byteProcessed = 0;
		switch(parameterType)
		{
		case CapabilityInfomation:
			{
				if(!parseCapabilityInfo(CapabilityInfomation,p,byteProcessed))
					return false;
				break;
			}
		case StreamingZone:
			{
				if(!parseCapabilityInfo(StreamingZone,p,byteProcessed))
					return false;
				break;
			}
		case ComponentName:
			{
				if(!parseCapabilityInfo(ComponentName,p,byteProcessed))
					return false;
				break;
			}
		case VendorSpecificString:
			{
				if (!parseCapabilityInfo(VendorSpecificString,p,byteProcessed))
					return false;
				break;
			}
		default:
			return false;
		}
		p += byteProcessed;
		parameterProcessed += byteProcessed;
	}
	if(_minBody.parametersLength != parameterProcessed)
		return false;
	if(streamingZone.empty() || componentName.empty())
		return false;
	processedLength += parameterProcessed;
	_hardHeader.Length = (uint16)processedLength + sizeof(_hardHeader);

	dumpBody();
	return true;	
}

uint32 OpenRequest::toMetaData(StringMap& metadata)
{
    toERRPHeader(metadata);
    
	/// cope other metadata;
	char strTemp[33] = "";

	memset(strTemp,0,sizeof(strTemp));
	itoa(_minBody.version,strTemp,10);
	std::string msgVer = strTemp;

	memset(strTemp,0,sizeof(strTemp));
	itoa(_minBody.Reserved,strTemp,10);
	std::string msgReserved = strTemp;

	memset(strTemp,0,sizeof(strTemp));
	itoa(_minBody.holdTime,strTemp,10);
	std::string msgHoldTime = strTemp;

	memset(strTemp,0,sizeof(strTemp));
	itoa(_minBody.adddressDomain,strTemp,10);
	std::string msgAdd = strTemp;

	memset(strTemp,0,sizeof(strTemp));
	itoa(_minBody.errpIdentifier,strTemp,10);
	std::string msgERRPId = strTemp;

	MAPSET(StringMap,metadata,ERRPMD_OpenVersion,msgVer);
	MAPSET(StringMap,metadata,ERRPMD_OpenReserved,msgReserved);
	MAPSET(StringMap,metadata,ERRPMD_OpenHoldTime,msgHoldTime);
	MAPSET(StringMap,metadata,ERRPMD_OpenAdddressDomain,msgAdd);
	MAPSET(StringMap,metadata,ERRPMD_OpenErrpIdentifier,msgERRPId);

	//Optional parameters 
	if(_minBody.parametersLength == 0)
		return (uint32)metadata.size();

	if(streamingZone.empty()|| componentName.empty())
		return 0;

	MAPSET(StringMap,metadata,ERRPMD_OpenStreamingZone,streamingZone);
	MAPSET(StringMap,metadata,ERRPMD_OpencomponetName,componentName);

	if(!vendorSpecificString.empty())
		MAPSET(StringMap,metadata,ERRPMD_OpenVendorSString,vendorSpecificString);

	//have no capability information 
	if(capabilityCodes.size() == 0)
		return (uint32)metadata.size();

	CapabilityCodes::iterator iter=capabilityCodes.begin();
	std::string msgCapabilityCode;
	for(iter;iter!=capabilityCodes.end();iter++)
	{
		uint16 capabilityCode = *iter;
		memset(strTemp,0,sizeof(strTemp));
		itoa(capabilityCode,strTemp,10);
		msgCapabilityCode += strTemp;
		if(iter != (capabilityCodes.end()-1))
			msgCapabilityCode += ",";

		if(capabilityCode == RouteTypeSupported)
		{
			memset(strTemp,0,sizeof(strTemp));
			itoa(addressFamiliy,strTemp,10);
			std::string msgAddFamily = strTemp;

			memset(strTemp,0,sizeof(strTemp));
			itoa(applicationProtocol,strTemp,10);
			std::string msgAppProtocol = strTemp;

			MAPSET(StringMap,metadata,ERRPMD_OpenAddressFamiliy,msgAddFamily);
			MAPSET(StringMap,metadata,ERRPMD_OpenAppProtocol,msgAppProtocol);
		}

		else if(capabilityCode == SendReceive)
		{
			memset(strTemp,0,sizeof(strTemp));
			itoa(sendReceivedMode,strTemp,10);
			std::string msgSendRecieveMode = strTemp;

			MAPSET(StringMap,metadata,ERRPMD_OpenSendReceivedMode,msgSendRecieveMode);
		}

		else if(capabilityCode == ERRPVersion)
		{
			memset(strTemp,0,sizeof(strTemp));
			itoa(errpVersion,strTemp,10);
			std::string msgErrpVersion = strTemp;

			MAPSET(StringMap,metadata,ERRPMD_OpenErrpVersion,msgErrpVersion);
		}
	}
	MAPSET(StringMap,metadata,ERRPMD_OpenCapabilityCode,msgCapabilityCode);

	return (uint32)metadata.size();
}
bool OpenRequest::readMetaData(const StringMap& metadata)
{
	///init _messageLength
	uint16 bodyProcessed = 0;
	uint16 parametersProcessed = 0;
	readERRPHeader(metadata);

	StringMap::const_iterator itorMD;

	itorMD = metadata.find(ERRPMD_OpenVersion);
	if(itorMD == metadata.end() || itorMD->second.empty())
		return false;
	_minBody.version = (uint8)atoi(itorMD->second.c_str());
	bodyProcessed += sizeof(_minBody.version);

	itorMD = metadata.find(ERRPMD_OpenReserved);
	if(itorMD == metadata.end() || itorMD->second.empty())
		_minBody.Reserved = 0;
	_minBody.Reserved = (uint8)atoi(itorMD->second.c_str());
	bodyProcessed += sizeof(_minBody.Reserved);

	itorMD = metadata.find(ERRPMD_OpenHoldTime);
	if(itorMD == metadata.end() || itorMD->second.empty())
		return false;
	_minBody.holdTime = (uint16)atoi(itorMD->second.c_str());
	bodyProcessed += sizeof(_minBody.holdTime);

	itorMD = metadata.find(ERRPMD_OpenAdddressDomain);
	if(itorMD == metadata.end() || itorMD->second.empty())
		return false;
	_minBody.adddressDomain = (uint32)atoi(itorMD->second.c_str());
	bodyProcessed += sizeof(_minBody.adddressDomain);

	itorMD = metadata.find(ERRPMD_OpenErrpIdentifier);
	if(itorMD == metadata.end() || itorMD->second.empty())
		return false;
	_minBody.errpIdentifier = (uint32)atoi(itorMD->second.c_str());
	bodyProcessed += sizeof(_minBody.errpIdentifier);

	bodyProcessed += sizeof(_minBody.parametersLength);
	//whether there are optional parameters 
	itorMD = metadata.find(ERRPMD_OpenStreamingZone);
	if(itorMD == metadata.end() || itorMD->second.empty())
	{
		_minBody.parametersLength = 0;
		bodyProcessed += parametersProcessed;
		_messageLength = bodyProcessed;
		_minBody.parametersLength = parametersProcessed;
		return true;
	}
	streamingZone = itorMD->second;
	parametersProcessed += (uint16)streamingZone.length() + 2 + 2;

	itorMD = metadata.find(ERRPMD_OpencomponetName);
	if(itorMD == metadata.end() || itorMD->second.empty())
		return false;
	componentName = itorMD->second;
	parametersProcessed += (uint16)componentName.length() + 2 + 2;

	itorMD = metadata.find(ERRPMD_OpenVendorSString);
	if(itorMD != metadata.end() && !itorMD->second.empty())
	{
		vendorSpecificString = itorMD->second;
		parametersProcessed += (uint16)vendorSpecificString.length() + 2 + 2;
	}

	itorMD = metadata.find(ERRPMD_OpenCapabilityCode);
	if(itorMD == metadata.end() || itorMD->second.empty())
	{
		//there is no capability information
		return true;
	}
	parametersProcessed += 4;//add the length of parameterType and parameterLength

	uint16 capabilityCode;
	std::string strCapabilityCode = itorMD->second;
	size_t startPos = 0;
	size_t capabilityProcessed = 0;
	while(capabilityProcessed < strCapabilityCode.length())
	{
		size_t endPos = strCapabilityCode.find(',',startPos);
		if(endPos == std::string::npos)
			endPos = strCapabilityCode.length();

		capabilityProcessed += endPos - startPos;

		capabilityCode = (uint16)atoi(strCapabilityCode.substr(startPos,endPos).c_str());
		capabilityCodes.push_back(capabilityCode);
		//printf("capabilityCode:%d\n",capabilityCode);
		//capabilityCode = (uint16)atoi(itorMD->second.c_str());
		parametersProcessed += sizeof(capabilityCode) + 2;

		switch(capabilityCode)
		{
		case RouteTypeSupported:
		{
			itorMD = metadata.find(ERRPMD_OpenAddressFamiliy );
			if(itorMD == metadata.end() || itorMD->second.empty())
				return false;
			addressFamiliy = (uint16)atoi(itorMD->second.c_str());

			itorMD = metadata.find(ERRPMD_OpenAppProtocol);
			if(itorMD == metadata.end() || itorMD->second.empty())
				return false;
			applicationProtocol = (uint16)atoi(itorMD->second.c_str());

			parametersProcessed += sizeof(addressFamiliy);
			parametersProcessed += sizeof(applicationProtocol);
			break;
		}
		case SendReceive:
		{
			itorMD = metadata.find(ERRPMD_OpenSendReceivedMode);
			if(itorMD == metadata.end() || itorMD->second.empty())
				return false;
			sendReceivedMode = (uint32)atoi(itorMD->second.c_str());

			parametersProcessed+= sizeof(sendReceivedMode);
			break;
		}
		case ERRPVersion:
		{
			itorMD = metadata.find(ERRPMD_OpenErrpVersion);
			if (itorMD == metadata.end() || itorMD->second.empty())
				return false;
			errpVersion = (uint32)atoi(itorMD->second.c_str());

			parametersProcessed += sizeof(errpVersion);
			break;
		}
		}

		startPos = endPos + 1;
		endPos = strCapabilityCode.find(',',startPos);
	}
	
	//printf("parametersProcessed:%d\n",parametersProcessed);
	bodyProcessed += parametersProcessed;
	_messageLength = bodyProcessed;
	_minBody.parametersLength = parametersProcessed;

	return true;
}

///////////////////////////////////////////////////////////////
////////////////// class UpdataRequest ///////////////
//////////////////////////////////////////////////////////////
uint16 UpdataRequest::formatMessageBody(uint8* buf, size_t maxLen)
{
	uint8* p =buf;
	if(NULL == buf)
		return 0 ;
	if(maxLen < _messageLength)
		return 0;

	size_t processedLength = 0;
	size_t attributeLength;
	TypeCodes::iterator iter = _attributeValue.typecodes.begin();
	for(iter;iter != _attributeValue.typecodes.end();iter++)
	{
		TypeCode typeCode;
		typeCode = *iter;
		attributeLength = typeCode.attributeLength;
		typeCode.attributeLength = htons(typeCode.attributeLength);
		memcpy(p,&typeCode,sizeof(typeCode));p += sizeof(typeCode);processedLength += sizeof(typeCode);

		switch(typeCode.attributeTypeCode)
		{
		case Code_WithDrawRouts:
			{
				size_t withDrawProcessed = 0;
				size_t length = 0;
				Routes::iterator iter = _attributeValue.withdrawRoutes.begin();
				Route route;
				for(iter;iter != _attributeValue.withdrawRoutes.end();iter++)
				{
					if(withDrawProcessed > _messageLength)
						return 0;
					route = *iter;
					route.addressFamily= htons(route.addressFamily);
					memcpy(p,&(route.addressFamily),sizeof(route.addressFamily));
					p += sizeof(route.addressFamily);withDrawProcessed += sizeof(route.addressFamily);

					route.applicationProtocol = htons(route.applicationProtocol);
					memcpy(p,&(route.applicationProtocol),sizeof(route.applicationProtocol));
					p += sizeof(route.applicationProtocol);withDrawProcessed += sizeof(route.applicationProtocol);

					char temp[1024] = "";
					memset(temp,0,sizeof(temp));
					length = bytesToString(route.address,temp);
					if(length != route.length)
						return 0;
					if(0 == temp)
						return 0;
					route.length = htons(route.length);
					memcpy(p,&(route.length),sizeof(route.length)); p += sizeof(route.length);withDrawProcessed += sizeof(route.length);
					memcpy(p,temp,length); p += length;withDrawProcessed += length;
				}

				if(withDrawProcessed != attributeLength)
					return 0;
				processedLength += withDrawProcessed;
				
				break;
			}
		case Code_ReachableRoutes:
			{
				size_t routeProcessed = 0;
				uint16 length = 0;
				Route route;
				Routes::iterator iter = _attributeValue.reachableRoutes.begin();
				for(iter;iter != _attributeValue.reachableRoutes.end();iter++)
				{
					if(routeProcessed > _messageLength)
						return 0;
					route = *iter;
					route.addressFamily = htons(route.addressFamily);
					memcpy(p,&(route.addressFamily),sizeof(route.addressFamily)); 
					p += sizeof(route.addressFamily);routeProcessed += sizeof(route.addressFamily);

					route.applicationProtocol = htons(route.applicationProtocol);
					memcpy(p,&(route.applicationProtocol),sizeof(route.applicationProtocol));
					p += sizeof(route.applicationProtocol);routeProcessed += sizeof(route.applicationProtocol);

					char temp[1024] = "";
					memset(temp,0,sizeof(temp));
					length = bytesToString(route.address,temp);
					if(0 == length)
						return 0;
					if(length != route.length)
						return 0;
					route.length = htons(route.length);
					memcpy(p,&(route.length),sizeof(route.length));p += sizeof(route.length);routeProcessed += sizeof(route.length);
					memcpy(p,temp,length);p += length;routeProcessed += length;

				}
				if(routeProcessed != attributeLength)
					return 0;
				processedLength += routeProcessed;
				break;
			}

		case Code_NextHopServer:
			{
				uint16 hopProcessed = 0;
				NextHopServer nextHop = _attributeValue.nextHopServer;
				nextHop.nextHopAddressDomain = htonl(nextHop.nextHopAddressDomain);
				memcpy(p,&nextHop.nextHopAddressDomain,sizeof(nextHop.nextHopAddressDomain));
				p += sizeof(nextHop.nextHopAddressDomain);hopProcessed += sizeof(nextHop.nextHopAddressDomain);

				uint16 addrLength = nextHop.componentAddrLength;
				nextHop.componentAddrLength = htons(nextHop.componentAddrLength);
				memcpy(p,&nextHop.componentAddrLength,sizeof(nextHop.componentAddrLength));
				p += sizeof(nextHop.componentAddrLength);hopProcessed += sizeof(nextHop.componentAddrLength);

				char temp[1024] = "";
				memset(temp,0,sizeof(temp));
				if(addrLength != bytesToString(nextHop.componentAddr,temp))
					return 0;
				memcpy(p,temp,addrLength);
				p += addrLength;hopProcessed += addrLength;

				uint16 streamLength = nextHop.streamingZoneNameLength;
				nextHop.streamingZoneNameLength = htons(nextHop.streamingZoneNameLength);
				memcpy(p,&nextHop.streamingZoneNameLength,sizeof(nextHop.streamingZoneNameLength));
				p += sizeof(nextHop.streamingZoneNameLength);hopProcessed += sizeof(nextHop.streamingZoneNameLength);

				memset(temp,0,sizeof(temp));
				if(streamLength != bytesToString(nextHop.streamingZoneName,temp))
					return 0;
				memcpy(p,temp,streamLength);
				p += streamLength;hopProcessed += streamLength;

				processedLength += hopProcessed;
				break;
			}
		case Code_QamNames:
			{
				char temp[1024] = "";
				memset(temp,0,sizeof(temp));
				uint16 qamLength = bytesToString(_attributeValue.qamNames,temp);
				if(0 == qamLength)
					return 0;
				uint16 length = qamLength;
				qamLength = htons(qamLength);
				memcpy(p,&qamLength,sizeof(qamLength));p += sizeof(qamLength);processedLength += sizeof(qamLength);
				
				memcpy(p,temp,length);p += length;processedLength += length;
				break;
			}
		case Code_CasCapability:
			{
				memcpy(p,&_attributeValue.casCapability.encType,sizeof(_attributeValue.casCapability.encType));
				p += sizeof(_attributeValue.casCapability.encType);processedLength += sizeof(_attributeValue.casCapability.encType);

				memcpy(p,&_attributeValue.casCapability.encScheme,sizeof(_attributeValue.casCapability.encScheme));
				p += sizeof(_attributeValue.casCapability.encScheme);processedLength += sizeof(_attributeValue.casCapability.encScheme);

				uint16 length = _attributeValue.casCapability.keyLength;
				_attributeValue.casCapability.keyLength = htons(_attributeValue.casCapability.keyLength);
				memcpy(p,&_attributeValue.casCapability.keyLength,sizeof(_attributeValue.casCapability.keyLength));
				p += sizeof(_attributeValue.casCapability.keyLength);processedLength += sizeof(_attributeValue.casCapability.keyLength);

				char temp[1024] = "";
				memset(temp,0,sizeof(temp));
				bytesToString(_attributeValue.casCapability.casIdentifier,temp);
				memcpy(p,temp,length);
				p += length;processedLength += length;

				break;
			}
		case Code_TotalBW:
			{
				_attributeValue.totalBandwidth = htonl(_attributeValue.totalBandwidth);
				memcpy(p,&_attributeValue.totalBandwidth,sizeof(_attributeValue.totalBandwidth));
				p += sizeof(_attributeValue.totalBandwidth);processedLength += sizeof(_attributeValue.totalBandwidth);

				break;
			}
		case Code_AvailableBW:
			{
				_attributeValue.availableBandwidth = htonl(_attributeValue.availableBandwidth);
				memcpy(p,&_attributeValue.availableBandwidth,sizeof(_attributeValue.availableBandwidth));
				p += sizeof(_attributeValue.availableBandwidth);processedLength += sizeof(_attributeValue.availableBandwidth);

				break;
			}
		case Code_Cost:
			{
				memcpy(p,&_attributeValue.cost,sizeof(_attributeValue.cost));
				p += sizeof(_attributeValue.cost);processedLength += sizeof(_attributeValue.cost);

				break;
			}
		case Code_EdgeInputs:
			{
				size_t edgeProcessed = 0;
				EdgeInput edgeInput;
				EdgeInputs::iterator iter = _attributeValue.edgeInputs.begin();
				for(iter;iter != _attributeValue.edgeInputs.end();iter++)
				{
					if(edgeProcessed > _messageLength)
						return 0;
					edgeInput = *iter;
					edgeInput.subMask = htonl(edgeInput.subMask);
					memcpy(p,&edgeInput.subMask,sizeof(edgeInput.subMask));
					p += sizeof(edgeInput.subMask);edgeProcessed += sizeof(edgeInput.subMask);

					uint16 length = edgeInput.hostLength;
					edgeInput.hostLength = htons(edgeInput.hostLength);
					memcpy(p,&edgeInput.hostLength,sizeof(edgeInput.hostLength));
					p += sizeof(edgeInput.hostLength);edgeProcessed += sizeof(edgeInput.hostLength);

					char temp[1024] = "";
					memset(temp,0,sizeof(temp));
					if(length != bytesToString(edgeInput.host,temp))
						return 0;
					memcpy(p,temp,length);
					p += length;edgeProcessed += length;

					edgeInput.interfaceID = htonl(edgeInput.interfaceID);
					memcpy(p,&edgeInput.interfaceID,sizeof(edgeInput.interfaceID));
					p += sizeof(edgeInput.interfaceID);edgeProcessed += sizeof(edgeInput.interfaceID);

					edgeInput.maxBandwidth = htonl(edgeInput.maxBandwidth);
					memcpy(p,&edgeInput.maxBandwidth,sizeof(edgeInput.maxBandwidth));
					p += sizeof(edgeInput.maxBandwidth);edgeProcessed += sizeof(edgeInput.maxBandwidth);

					length = edgeInput.groupNameLength;
					edgeInput.groupNameLength = htons(edgeInput.groupNameLength);
					memcpy(p,&edgeInput.groupNameLength,sizeof(edgeInput.groupNameLength));
					p += sizeof(edgeInput.groupNameLength);edgeProcessed += sizeof(edgeInput.groupNameLength);

					memset(temp,0,sizeof(temp));
					if(length != bytesToString(edgeInput.groupName,temp))
						return 0;
					memcpy(p,temp,length);
					p += length;edgeProcessed += length;
				}
				if(edgeProcessed != attributeLength)
					return 0;

				processedLength += edgeProcessed;

				break;
			}
		case Code_QamChConfig:
			{
				_attributeValue.qamChConfig.frequency = htonl(_attributeValue.qamChConfig.frequency);
				_attributeValue.qamChConfig.TSID = htons(_attributeValue.qamChConfig.TSID);
				_attributeValue.qamChConfig.reserved = htons(_attributeValue.qamChConfig.reserved);

				memcpy(p,&_attributeValue.qamChConfig,sizeof(_attributeValue.qamChConfig));
				p += sizeof(_attributeValue.qamChConfig);processedLength += sizeof(_attributeValue.qamChConfig);

				break;
			}
		case Code_UdpMap:
			{
				size_t udpProcessed = 0;
				//static port
				uint32 numStatic = (uint32)_attributeValue.udpMap.staticPorts.size();
				numStatic = htonl(numStatic);
				memcpy(p,&numStatic,sizeof(numStatic));p += sizeof(numStatic);udpProcessed += sizeof(numStatic);
				std::map<uint16,uint16>::iterator iter = _attributeValue.udpMap.staticPorts.begin();
				for(iter;iter != _attributeValue.udpMap.staticPorts.end();iter++)
				{
					if(udpProcessed > _messageLength)
						return 0;
					uint16 port = htons(iter->first);
					uint16 MPEGProgram = htons(iter->second);
					memcpy(p,&port,sizeof(port));p += sizeof(port);udpProcessed += sizeof(port);
					memcpy(p,&MPEGProgram,sizeof(MPEGProgram)); p += sizeof(MPEGProgram);udpProcessed += sizeof(MPEGProgram);
				}

				//static port ranges
				uint32 numPortRanges = (uint32)_attributeValue.udpMap.staticPortRanges.size();
				numPortRanges = htonl(numPortRanges);
				memcpy(p,&numPortRanges,sizeof(numPortRanges));
				p += sizeof(numPortRanges);udpProcessed += sizeof(numPortRanges);
				std::vector<StaticPortRange>::iterator vIter = _attributeValue.udpMap.staticPortRanges.begin();
				StaticPortRange staticPortRange;
				for(vIter;vIter != _attributeValue.udpMap.staticPortRanges.end();vIter++)
				{
					if(udpProcessed > _messageLength)
						return 0;
					staticPortRange = *vIter;
					staticPortRange.startPort = htons(staticPortRange.startPort);
					staticPortRange.startMpegProgram = htons(staticPortRange.startMpegProgram);
					staticPortRange.count = htonl(staticPortRange.count);
					memcpy(p,&staticPortRange,sizeof(staticPortRange));
					p += sizeof(staticPortRange);udpProcessed += sizeof(staticPortRange);
				}

				//dynamic port ranges
				uint32 numDynamicPortRange = (uint32)_attributeValue.udpMap.dynamicPorts.size();
				numDynamicPortRange = htonl(numDynamicPortRange);
				memcpy(p,&numDynamicPortRange,sizeof(numDynamicPortRange));
				p += sizeof(numDynamicPortRange);udpProcessed += sizeof(numDynamicPortRange);
				std::map<uint16,uint16>::iterator mIter = _attributeValue.udpMap.dynamicPorts.begin();
				for(mIter;mIter != _attributeValue.udpMap.dynamicPorts.end();mIter++)
				{
					if(udpProcessed > _messageLength)
						return false;
					uint16 port = htons(mIter->first);
					uint16 count = htons(mIter->second);
					memcpy(p,&port,sizeof(port)); p += sizeof(port);udpProcessed += sizeof(port);
					memcpy(p,&count,sizeof(count)); p += sizeof(count);udpProcessed += sizeof(count);
				}

				processedLength += udpProcessed;

				break;
			}
		case Code_ServiceStatus:
			{
				_attributeValue.serviceStatus = htonl(_attributeValue.serviceStatus);
				memcpy(p,&_attributeValue.serviceStatus,sizeof(_attributeValue.serviceStatus));
				p += sizeof(_attributeValue.serviceStatus);processedLength += sizeof(_attributeValue.serviceStatus);

				break;
			}
		case Code_MaxMPEGFlows:
			{
				_attributeValue.maxMPEGFlows = htonl(_attributeValue.maxMPEGFlows);
				memcpy(p,&_attributeValue.maxMPEGFlows,sizeof(_attributeValue.maxMPEGFlows));
				p += sizeof(_attributeValue.maxMPEGFlows);processedLength += sizeof(_attributeValue.maxMPEGFlows);

				break;
			}
		case Code_NextHopServerAlternate:
			{
				size_t alterProcessed = 0;
				uint16 numAlternates = htons((u_short)_attributeValue.nextHopServerAlternate.size());
				memcpy(p,&numAlternates,sizeof(numAlternates));
				p += sizeof(numAlternates);alterProcessed += sizeof(numAlternates);
				std::vector<Bytes>::iterator iter = _attributeValue.nextHopServerAlternate.begin();
				for(iter;iter != _attributeValue.nextHopServerAlternate.end();iter++)
				{
					if(alterProcessed > _messageLength)
						return 0;
					Bytes bytes = *iter;
					char temp[1024] = "";
					memset(temp,0,sizeof(temp));
					uint16 length = bytesToString(bytes,temp);
					if(0 == length)
						return false;
					uint16 count = length;
					length = htons(length);
					memcpy(p,&length,sizeof(length));p += sizeof(length);alterProcessed += sizeof(length);
					memcpy(p,temp,count);p += count;alterProcessed += count;
				}

				processedLength += alterProcessed;
				break;
			}
		case Code_PortID:
			{
				_attributeValue.portID = htonl(_attributeValue.portID);
				memcpy(p,&_attributeValue.portID,sizeof(_attributeValue.portID));
				p += sizeof(_attributeValue.portID);processedLength += sizeof(_attributeValue.portID);

				break;
			}
		case Code_FiberNodeNames:
			{
				uint16 nodeProcessed = 0;
				std::vector<Bytes>::iterator iter = _attributeValue.fiberNodeNames.begin();
				for(iter;iter != _attributeValue.fiberNodeNames.end();iter++)
				{
					if(nodeProcessed > _messageLength)
						return 0;
					Bytes bytes = *iter;
					char temp[1024] = "";
					memset(temp,'0',sizeof(temp));
					uint16 length = bytesToString(bytes,temp);
					uint16 count = length;
					length = htons(length);
					memcpy(p,&length,sizeof(length)); p += sizeof(length);nodeProcessed += sizeof(length);
					memcpy(p,temp,count); p += count; nodeProcessed += count;
				}

				processedLength += nodeProcessed;
				break;
			}
		case Code_QamCapability:
			{
				QAMCapability qamCapa = _attributeValue.qamCapability;
				qamCapa.channelBandwidth = htons(qamCapa.channelBandwidth);
				qamCapa.J83 = htons(qamCapa.J83);
				qamCapa.interLeaver = htonl(qamCapa.interLeaver);
				qamCapa.capabilities = htonl(qamCapa.capabilities);
				qamCapa.modulation = htons(qamCapa.modulation);

				memcpy(p,&qamCapa,sizeof(qamCapa));
				p += sizeof(qamCapa);processedLength += sizeof(qamCapa);
				break;
			}
		case Code_InputMaps:
			{
				uint32 numInterface = (uint32)_attributeValue.inputMaps.size();
				numInterface = htonl(numInterface);
				memcpy(p,&numInterface,sizeof(numInterface));p += sizeof(numInterface);processedLength += sizeof(numInterface);
				std::vector<Bytes>::iterator iter = _attributeValue.inputMaps.begin();
				for(iter;iter != _attributeValue.inputMaps.end();iter++)
				{
					if((p - buf) > _messageLength)
						return 0;
					Bytes bytes = *iter;
					char temp[1024] = "";
					memset(temp,0,sizeof(temp));
					uint16 length = bytesToString(bytes,temp);
					uint16 count = length;
					length = htons(length);
					memcpy(p,&length,sizeof(length)); p += sizeof(length);processedLength += sizeof(length);
					memcpy(p,temp,count); p += count; processedLength += count;
				}
				break;
			}
		}
	}
	return (uint16)(p - buf);
}
bool UpdataRequest::parseMessageBody(const uint8* buf, size_t messageLength)
{
	if(!buf)
		return false;

	_messageLength = (uint16)messageLength;

	const uint8* p = buf;
	uint16 attributeType = 0;
	uint16 attributeLength = 0;
	uint16 processedLength = 0;

	while(processedLength < messageLength)
	{
		memcpy(&attributeType,p,sizeof(attributeType)); p += sizeof(attributeType); processedLength += sizeof(attributeType);
		attributeType = ntohs(attributeType);
		memcpy(&attributeLength,p,sizeof(attributeLength)); p += sizeof(attributeLength); processedLength += sizeof(attributeLength);
		attributeLength = ntohs(attributeLength);
		uint8 attibuteFlag = 0;
		memcpy(&attibuteFlag,&attributeType,sizeof(attibuteFlag));
		if(0 == attributeType)
			return false;

		TypeCode typeCode;
		typeCode.attributeTypeCode = (uint8)attributeType;
		typeCode.attributeTypeFlags = 0;
		typeCode.attributeLength = attributeLength;
		_attributeValue.typecodes.push_back(typeCode);

		switch(attributeType)
		{
		case Code_WithDrawRouts:
			{
				uint16 listProcessed = 0;
				while (listProcessed < attributeLength)
				{
					if(listProcessed > _messageLength)
						return false;
					Route route;
					memcpy(&route.addressFamily,p,sizeof(route.addressFamily));
					route.addressFamily = ntohs(route.addressFamily);
					p += sizeof(route.addressFamily); listProcessed += sizeof(route.addressFamily);

					memcpy(&route.applicationProtocol,p,sizeof(route.applicationProtocol));
					route.applicationProtocol = ntohs(route.applicationProtocol);
					p += sizeof(route.applicationProtocol);listProcessed += sizeof(route.applicationProtocol);

					memcpy(&route.length,p,sizeof(route.length));
					route.length = ntohs(route.length);
					p += sizeof(route.length);listProcessed += sizeof(route.length);

					char temp[1024] = "";
					memset(temp,0,sizeof(temp));
					memcpy(temp,p,route.length);
					p += route.length;listProcessed += route.length;

					if(!stringToBytes(route.address,temp,route.length))
						return false;

					_attributeValue.withdrawRoutes.push_back(route);
				}
				processedLength += listProcessed;
				break;
			}
		case Code_ReachableRoutes:
			{
				uint16 listProcessed = 0;
				while (listProcessed < attributeLength)
				{
					if(listProcessed > _messageLength)
						return false;
					Route route;
					memcpy(&route.addressFamily,p,sizeof(route.addressFamily));
					route.addressFamily = ntohs(route.addressFamily);
					p += sizeof(route.addressFamily); listProcessed += sizeof(route.addressFamily);

					memcpy(&route.applicationProtocol,p,sizeof(route.applicationProtocol));
					route.applicationProtocol = ntohs(route.applicationProtocol);
					p += sizeof(route.applicationProtocol);listProcessed += sizeof(route.applicationProtocol);

					memcpy(&route.length,p,sizeof(route.length));
					route.length = ntohs(route.length);
					p += sizeof(route.length);listProcessed += sizeof(route.length);

					char temp[1024] = "";
					memset(temp,0,sizeof(temp));
					memcpy(temp,p,route.length);
					p += route.length;listProcessed += route.length;

					if(!stringToBytes(route.address,temp,route.length))
						return false;

					_attributeValue.reachableRoutes.push_back(route);
				}
				processedLength += listProcessed;
				break;
			}
		case Code_NextHopServer:
			{
				uint16 nextHopServerProcessed = 0;
				NextHopServer nextHopServer;
				memcpy(&nextHopServer.nextHopAddressDomain,p,sizeof(nextHopServer.nextHopAddressDomain));
				nextHopServer.nextHopAddressDomain = ntohl(nextHopServer.nextHopAddressDomain);
				p += sizeof(nextHopServer.nextHopAddressDomain);nextHopServerProcessed += sizeof(nextHopServer.nextHopAddressDomain);

				memcpy(&nextHopServer.componentAddrLength,p,sizeof(nextHopServer.componentAddrLength));
				nextHopServer.componentAddrLength = ntohs(nextHopServer.componentAddrLength);
				p += sizeof(nextHopServer.componentAddrLength);nextHopServerProcessed += sizeof(nextHopServer.componentAddrLength);

				char temp[1024] = "";
				memset(temp,0,sizeof(temp));
				memcpy(temp,p,nextHopServer.componentAddrLength);
				p += nextHopServer.componentAddrLength;nextHopServerProcessed += nextHopServer.componentAddrLength;
				if(!stringToBytes(nextHopServer.componentAddr,temp,nextHopServer.componentAddrLength))
					return false;

				memcpy(&nextHopServer.streamingZoneNameLength,p,sizeof(nextHopServer.streamingZoneNameLength));
				nextHopServer.streamingZoneNameLength = ntohs(nextHopServer.streamingZoneNameLength);
				p += sizeof(nextHopServer.streamingZoneNameLength);nextHopServerProcessed += sizeof(nextHopServer.streamingZoneNameLength);

				memset(temp,0,sizeof(temp));
				memcpy(temp,p,nextHopServer.streamingZoneNameLength);
				p += nextHopServer.streamingZoneNameLength;nextHopServerProcessed += nextHopServer.streamingZoneNameLength;
				if(!stringToBytes(nextHopServer.streamingZoneName,temp,nextHopServer.streamingZoneNameLength))
					return false;
				
				_attributeValue.nextHopServer = nextHopServer;
				processedLength += nextHopServerProcessed;
				break;
			}
		case Code_QamNames:
			{
				uint16 qamNameProcessed = 0;
				uint16 qamNamelength = 0;
				memcpy(&qamNamelength,p,sizeof(qamNamelength));
				qamNamelength = ntohs(qamNamelength);
				p += sizeof(qamNamelength);qamNameProcessed += sizeof(qamNamelength);
				char temp[1024] = "";
				memset(temp,0,sizeof(temp));
				memcpy(temp,p,qamNamelength);
				p += qamNamelength;qamNameProcessed += qamNamelength;
				if(!stringToBytes(_attributeValue.qamNames,temp,qamNamelength))
					return false;

				processedLength += qamNameProcessed;
				break;
			}
		case Code_CasCapability:
			{
				uint16 casProcessed = 0;
				CASCapability casCapability;
				memcpy(&casCapability.encType,p,sizeof(casCapability.encType));
				p += sizeof(casCapability.encType);casProcessed += sizeof(casCapability.encType);

				memcpy(&casCapability.encScheme,p,sizeof(casCapability.encScheme));
				p += sizeof(casCapability.encScheme);casProcessed += sizeof(casCapability.encScheme);

				memcpy(&casCapability.keyLength,p,sizeof(casCapability.keyLength));
				casCapability.keyLength = ntohs(casCapability.keyLength);
				p += sizeof(casCapability.keyLength);casProcessed += sizeof(casCapability.keyLength);

				char temp[1024] = "";
				memset(temp,0,sizeof(temp));
				memcpy(temp,p,casCapability.keyLength);
				p += casCapability.keyLength;casProcessed += casCapability.keyLength;
				if(!stringToBytes(casCapability.casIdentifier,temp,casCapability.keyLength))
					return false;

				_attributeValue.casCapability = casCapability;
				processedLength += casProcessed;
				break;
			}
		case Code_TotalBW:
			{
				memcpy(&(_attributeValue.totalBandwidth),p,sizeof(_attributeValue.totalBandwidth));
				p += sizeof(_attributeValue.totalBandwidth);
				_attributeValue.totalBandwidth = ntohl(_attributeValue.totalBandwidth);

				processedLength += sizeof(_attributeValue.totalBandwidth);
				
				break;
			}
		case Code_AvailableBW:
			{
				memcpy(&(_attributeValue.availableBandwidth),p,sizeof(_attributeValue.availableBandwidth));
				p += sizeof(_attributeValue.availableBandwidth);
				_attributeValue.availableBandwidth = ntohl(_attributeValue.availableBandwidth);

				processedLength += sizeof(_attributeValue.availableBandwidth);
				break;
			}
		case Code_Cost:
			{
				memcpy(&(_attributeValue.cost),p,sizeof(_attributeValue.cost));
				p += sizeof(_attributeValue.cost);

				processedLength += sizeof(_attributeValue.cost);
				break;
			}
		case Code_EdgeInputs:
			{
				uint16 edgeInputProcessed = 0;
				while(edgeInputProcessed < attributeLength)
				{
					if(edgeInputProcessed > _messageLength)
						return false;
					EdgeInput edgeInput;
					memcpy(&(edgeInput.subMask),p,sizeof(edgeInput.subMask));
					p += sizeof(edgeInput.subMask);edgeInputProcessed += sizeof(edgeInput.subMask);
					edgeInput.subMask = ntohl(edgeInput.subMask);

					memcpy(&(edgeInput.hostLength),p,sizeof(edgeInput.hostLength));
					p += sizeof(edgeInput.hostLength);edgeInputProcessed += sizeof(edgeInput.hostLength);
					edgeInput.hostLength = ntohs(edgeInput.hostLength);

					char temp[1024] = "";
					memset(temp,0,sizeof(temp));
					memcpy(temp,p,edgeInput.hostLength);
					p += edgeInput.hostLength;edgeInputProcessed += edgeInput.hostLength;
					if(!stringToBytes(edgeInput.host,temp,edgeInput.hostLength))
						return false;

					memcpy(&(edgeInput.interfaceID),p,sizeof(edgeInput.interfaceID));
					p += sizeof(edgeInput.interfaceID);edgeInputProcessed += sizeof(edgeInput.interfaceID);
					edgeInput.interfaceID = ntohl(edgeInput.interfaceID);

					memcpy(&(edgeInput.maxBandwidth),p,sizeof(edgeInput.maxBandwidth));
					p += sizeof(edgeInput.maxBandwidth);edgeInputProcessed += sizeof(edgeInput.maxBandwidth);
					edgeInput.maxBandwidth = ntohl(edgeInput.maxBandwidth);

					memcpy(&(edgeInput.groupNameLength),p,sizeof(edgeInput.groupNameLength));
					p += sizeof(edgeInput.groupNameLength);edgeInputProcessed += sizeof(edgeInput.groupNameLength);
					edgeInput.groupNameLength = ntohs(edgeInput.groupNameLength);

					memset(temp,0,sizeof(temp));
					memcpy(temp,p,edgeInput.groupNameLength);
					p += edgeInput.groupNameLength;edgeInputProcessed += edgeInput.groupNameLength;
					if(!stringToBytes(edgeInput.groupName,temp,edgeInput.groupNameLength))
						return false;

					_attributeValue.edgeInputs.push_back(edgeInput);
				}
				
				processedLength += edgeInputProcessed;
				break;
			}
		case Code_QamChConfig:
			{
				uint16 qamChProcessed = 0;
				QAMChannelConfig qamChConfig;

				memcpy(&(qamChConfig.frequency),p,sizeof(qamChConfig.frequency));
				p += sizeof(qamChConfig.frequency); qamChProcessed += sizeof(qamChConfig.frequency);
				qamChConfig.frequency = ntohl(qamChConfig.frequency);

				memcpy(&(qamChConfig.modMode),p,sizeof(qamChConfig.modMode));
				p += sizeof(qamChConfig.modMode); qamChProcessed += sizeof(qamChConfig.modMode);
				
				memcpy(&(qamChConfig.interleaver),p,sizeof(qamChConfig.interleaver));
				p += sizeof(qamChConfig.interleaver); qamChProcessed += sizeof(qamChConfig.interleaver);

				memcpy(&(qamChConfig.TSID),p,sizeof(qamChConfig.TSID));
				p += sizeof(qamChConfig.TSID); qamChProcessed += sizeof(qamChConfig.TSID);
				qamChConfig.TSID = ntohs(qamChConfig.TSID);

				memcpy(&(qamChConfig.annex),p,sizeof(qamChConfig.annex));
				p += sizeof(qamChConfig.annex); qamChProcessed += sizeof(qamChConfig.annex);

				memcpy(&(qamChConfig.channelWidth),p,sizeof(qamChConfig.channelWidth));
				p += sizeof(qamChConfig.channelWidth); qamChProcessed += sizeof(qamChConfig.channelWidth);

				memcpy(&(qamChConfig.reserved),p,sizeof(qamChConfig.reserved));
				p += sizeof(qamChConfig.reserved); qamChProcessed += sizeof(qamChConfig.reserved);
				qamChConfig.reserved = ntohs(qamChConfig.reserved);

				_attributeValue.qamChConfig = qamChConfig;
				processedLength += qamChProcessed;
				break;
			}
		case Code_UdpMap:
			{
				uint16 udpMapProcessed = 0;
				UDPMap udpMap;

				///static port
				uint32 numStaticPorts = 0;
				memcpy(&numStaticPorts,p,sizeof(numStaticPorts));
				p += sizeof(numStaticPorts);udpMapProcessed += sizeof(numStaticPorts);
				numStaticPorts = ntohl(numStaticPorts);

				for(uint32 i = 0;i < numStaticPorts;i++)
				{
					if((p - buf) > _messageLength)
						return false;
					uint16 udpPort = 0;
					uint16 MPEGProgram;
					memcpy(&udpPort,p,sizeof(udpPort));p += sizeof(udpPort);udpMapProcessed += sizeof(udpPort);
					udpPort = ntohs(udpPort);
					memcpy(&MPEGProgram,p,sizeof(MPEGProgram));p += sizeof(MPEGProgram);udpMapProcessed += sizeof(MPEGProgram);
					MPEGProgram = ntohs(MPEGProgram);

					udpMap.staticPorts[udpPort] = MPEGProgram;
				}

				////static port ranges
				uint32 numRanges = 0;
				memcpy(&numRanges,p,sizeof(numRanges));p += sizeof(numRanges);udpMapProcessed += sizeof(numRanges);
				numRanges = ntohl(numRanges);

				for(uint32 i = 0;i < numRanges;i++)
				{
					if((p - buf) > _messageLength)
						return false;
					uint16 startingPort = 0;
					uint16 MPEGProgram = 0;
					uint32 count;
					memcpy(&startingPort,p,sizeof(startingPort));p += sizeof(startingPort);udpMapProcessed += sizeof(startingPort);
					startingPort = ntohs(startingPort);
					memcpy(&MPEGProgram,p,sizeof(MPEGProgram));p += sizeof(MPEGProgram);udpMapProcessed += sizeof(MPEGProgram);
					MPEGProgram = ntohs(MPEGProgram);
					memcpy(&count,p,sizeof(count));p += sizeof(count);udpMapProcessed += sizeof(count);
					count = ntohl(count);
					
					StaticPortRange portRange;
					portRange.count = count;
					portRange.startPort = startingPort;
					portRange.startMpegProgram = MPEGProgram;
					
					udpMap.staticPortRanges.push_back(portRange);
				}

				///dynamic port ranges
				uint32 numDynamicPort = 0;
				memcpy(&numDynamicPort,p,sizeof(numDynamicPort));p += sizeof(numDynamicPort);udpMapProcessed += sizeof(numDynamicPort);
				numDynamicPort = ntohl(numDynamicPort);

				for(uint32 i = 0;i < numDynamicPort;i++)
				{
					if((p - buf) > _messageLength)
						return false;
					uint16 startPort = 0;
					uint16 count = 0;
					memcpy(&startPort,p,sizeof(startPort));p += sizeof(startPort);udpMapProcessed += sizeof(startPort);
					startPort = ntohs(startPort);

					memcpy(&count,p,sizeof(count));p += sizeof(count);udpMapProcessed += sizeof(count);
					count = ntohs(count);

					udpMap.dynamicPorts[startPort] = count;
				}

				_attributeValue.udpMap = udpMap;
				processedLength += udpMapProcessed;
				break;
			}
		case Code_ServiceStatus:
			{
				memcpy(&_attributeValue.serviceStatus,p,sizeof(_attributeValue.serviceStatus));
				p += sizeof(_attributeValue.serviceStatus);processedLength += sizeof(_attributeValue.serviceStatus);
				_attributeValue.serviceStatus = ntohl(_attributeValue.serviceStatus);

				break;
			}
		case Code_MaxMPEGFlows:
			{
				memcpy(&_attributeValue.maxMPEGFlows,p,sizeof(_attributeValue.maxMPEGFlows));
				p += sizeof(_attributeValue.maxMPEGFlows);processedLength += sizeof(_attributeValue.maxMPEGFlows);
				_attributeValue.maxMPEGFlows = ntohl(_attributeValue.maxMPEGFlows);

				break;
			}
		case Code_NextHopServerAlternate:
			{
				uint16 processed = 0;
				uint16 num = 0;
				memcpy(&num,p,sizeof(num));
				num = ntohs(num);
				p += sizeof(num);processed += num;

				for(int i = 0;i < num;i++)
				{
					if(processed > _messageLength)
						return false;
					uint16 serverLength = 0;
					memcpy(&serverLength,p,sizeof(serverLength));
					serverLength = ntohs(serverLength);
					p += sizeof(serverLength);processed += sizeof(serverLength);

					char temp[1024] = "";
					memset(temp,0,sizeof(temp));
					memcpy(temp,p,serverLength);p += serverLength;processed += serverLength;
					Bytes bytes;
					if(!stringToBytes(bytes,temp,serverLength))
						return false;
					_attributeValue.nextHopServerAlternate.push_back(bytes);
				}

				processedLength += processed;
				break;
			}
		case Code_PortID:
			{
				memcpy(&(_attributeValue.portID),p,sizeof(_attributeValue.portID));
				p += sizeof(_attributeValue.portID);
				_attributeValue.portID = ntohl(_attributeValue.portID);
				processedLength += sizeof(_attributeValue.portID);
				break;
			}
		case Code_FiberNodeNames:
			{
				uint16 nodeLength = 0;
				uint16 nodeProcessed = 0;

				while(nodeProcessed < attributeLength)
				{
					if(nodeProcessed > _messageLength)
						return false;
					memcpy(&nodeLength,p,sizeof(nodeLength));
					p += sizeof(nodeLength);nodeProcessed += sizeof(nodeLength);
					nodeLength = ntohs(nodeLength);

					char temp[1024] = "";
					memset(temp,0,sizeof(temp));
					memcpy(temp,p,nodeLength);p += nodeLength;nodeProcessed += nodeLength;
					Bytes bytes;
					if(!stringToBytes(bytes,temp,nodeLength))
						return false;
					_attributeValue.fiberNodeNames.push_back(bytes);
				}

				processedLength += nodeProcessed;
				break;
			}
		case Code_QamCapability:
			{
				uint16 qamCapabilityProcessed = 0;
				QAMCapability qamCap;

				memcpy(&qamCap.channelBandwidth,p,sizeof(qamCap.channelBandwidth)); 
				p += sizeof(qamCap.channelBandwidth);qamCapabilityProcessed += sizeof(qamCap.channelBandwidth);
				qamCap.channelBandwidth = ntohs(qamCap.channelBandwidth);

				memcpy(&qamCap.J83,p,sizeof(qamCap.J83));
				p += sizeof(qamCap.J83);qamCapabilityProcessed += sizeof(qamCap.J83);
				qamCap.J83 = ntohs(qamCap.J83);

				memcpy(&qamCap.interLeaver,p,sizeof(qamCap.interLeaver));
				p += sizeof(qamCap.interLeaver);qamCapabilityProcessed += sizeof(qamCap.interLeaver);
				qamCap.interLeaver = ntohl(qamCap.interLeaver);

				memcpy(&qamCap.capabilities,p,sizeof(qamCap.capabilities));
				p += sizeof(qamCap.capabilities);qamCapabilityProcessed += sizeof(qamCap.capabilities);
				qamCap.capabilities = ntohl(qamCap.capabilities);

				memcpy(&qamCap.modulation,p,sizeof(qamCap.modulation));
				p += sizeof(qamCap.modulation);qamCapabilityProcessed += sizeof(qamCap.modulation);
				qamCap.modulation = ntohs(qamCap.modulation);

				_attributeValue.qamCapability = qamCap;
				processedLength += qamCapabilityProcessed;
				break;
			}
		case Code_InputMaps:
			{
				uint16 inputMapProcessed = 0;
				uint32 num = 0;
				memcpy(&num,p,sizeof(num));p += sizeof(num);processedLength += sizeof(num);
				num = ntohl(num);
				for(uint32 i = 0;i< num;i++)
				{
					if(inputMapProcessed > _messageLength)
						return false;
					uint16 length = 0;
					memcpy(&length,p,sizeof(length));p += sizeof(length);processedLength += sizeof(length);
					length = ntohs(length);

					char temp[1024] = "";
					memset(temp,0,sizeof(temp));
					memcpy(temp,p,length);p += length;processedLength += length;
					Bytes bytes;
					if(!stringToBytes(bytes,temp,length))
						return false;
					_attributeValue.inputMaps.push_back(bytes);
				}
				processedLength += inputMapProcessed;
				break;
			}
		}
	}
	if(processedLength != messageLength)
		return false;

	_hardHeader.Length = processedLength + sizeof(_hardHeader);

	return true;
}

uint32 UpdataRequest::toMetaData(StringMap& metadata)
{
	toERRPHeader(metadata);

	return (uint32)metadata.size();
}

bool UpdataRequest::readMetaData(const StringMap& metadata)
{
	readERRPHeader(metadata);
	return true;
}

void UpdataRequest::setRoutes(AttrbuteValue& attr)
{
	ZQ::ERRP::UpdataRequest::TypeCodes::iterator iter = attr.typecodes.begin();
	_messageLength += (uint16)attr.typecodes.size() * sizeof(TypeCode);

	for(iter;iter != attr.typecodes.end();iter++)
	{
		uint16 attrLength = 0;
		switch(iter->attributeTypeCode)
		{
		case Code_WithDrawRouts:
			{
				Routes::iterator withDrawIter = attr.withdrawRoutes.begin();
				for(withDrawIter;withDrawIter != attr.withdrawRoutes.end();withDrawIter++)
				{
					attrLength += sizeof(withDrawIter->addressFamily);
					attrLength += sizeof(withDrawIter->applicationProtocol);
					attrLength += sizeof(withDrawIter->length);
					attrLength += (uint16)withDrawIter->address.size();

					withDrawIter->length = (uint16)withDrawIter->address.size();
				}
				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		case Code_ReachableRoutes:
			{
				Routes::iterator reachableIter = attr.reachableRoutes.begin();
				for(;reachableIter != attr.reachableRoutes.end();reachableIter++)
				{
					attrLength += sizeof(reachableIter->addressFamily);
					attrLength += sizeof(reachableIter->applicationProtocol);
					attrLength += sizeof(reachableIter->length);
					attrLength += (uint16)reachableIter->address.size();

					reachableIter->length = (uint16)reachableIter->address.size();
				}
				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		case Code_NextHopServer:
			{
				attrLength += sizeof(attr.nextHopServer.nextHopAddressDomain);
				attrLength += sizeof(attr.nextHopServer.componentAddrLength);
				attrLength += (uint16)attr.nextHopServer.componentAddr.size();
				attrLength += sizeof(attr.nextHopServer.streamingZoneNameLength);
				attrLength += (uint16)attr.nextHopServer.streamingZoneName.size();

				attr.nextHopServer.componentAddrLength = (uint16)attr.nextHopServer.componentAddr.size();
				attr.nextHopServer.streamingZoneNameLength = (uint16)attr.nextHopServer.streamingZoneName.size();

				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		case Code_QamNames:
			{
				attrLength += (uint16)attr.qamNames.size() + 2;//qam name length(2 bytes)

				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		case Code_CasCapability:
			{
				attrLength += sizeof(attr.casCapability.encType);
				attrLength += sizeof(attr.casCapability.encScheme);
				attrLength += sizeof(attr.casCapability.keyLength);
				attrLength += (uint16)attr.casCapability.casIdentifier.size();
				attr.casCapability.keyLength = (uint16)attr.casCapability.casIdentifier.size();

				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		case Code_TotalBW:
			{
				attrLength += sizeof(attr.totalBandwidth);

				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		case Code_AvailableBW:
			{
				attrLength += sizeof(attr.availableBandwidth);

				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		case Code_Cost:
			{
				attrLength += sizeof(attr.cost);

				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		case Code_EdgeInputs:
			{
				EdgeInputs::iterator edgeIter = attr.edgeInputs.begin();
				for(;edgeIter != attr.edgeInputs.end();edgeIter++)
				{
					attrLength += sizeof(edgeIter->subMask);
					attrLength += sizeof(edgeIter->hostLength);
					edgeIter->hostLength = (uint16)edgeIter->host.size();
					attrLength += edgeIter->hostLength;
					attrLength += sizeof(edgeIter->interfaceID);
					attrLength += sizeof(edgeIter->maxBandwidth);
					attrLength += sizeof(edgeIter->groupNameLength);
					edgeIter->groupNameLength = (uint16)edgeIter->groupName.size();
					attrLength += edgeIter->groupNameLength;
				}
				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		case Code_QamChConfig:
			{
				attrLength += sizeof(attr.qamChConfig);
				
				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		case Code_UdpMap:
			{
				attrLength += (uint16)attr.udpMap.staticPorts.size() * 4;
				attrLength += (uint16)attr.udpMap.staticPortRanges.size() * sizeof(StaticPortRange);
				attrLength += (uint16)attr.udpMap.dynamicPorts.size() * 4;
				attrLength += 3 * 4;//nums of static port(4 bytes),static port ranges(4 bytes) and dynamic port ranges(4 bytes)

				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		case Code_ServiceStatus:
			{
				attrLength += sizeof(attr.serviceStatus);

				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		case Code_MaxMPEGFlows:
			{
				attrLength += sizeof(attr.maxMPEGFlows);

				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		case Code_NextHopServerAlternate:
			{
				attrLength += (uint16)attr.nextHopServerAlternate.size();//num of alternates
				std::vector<Bytes>::iterator vIter = attr.nextHopServerAlternate.begin();
				for(;vIter != attr.nextHopServerAlternate.end();vIter++)
				{
					attrLength += 2;//length of server(2 bytes)
					attrLength += (uint16)vIter->size();
				}

				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		case Code_PortID:
			{
				attrLength += sizeof(attr.portID);

				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		case Code_FiberNodeNames:
			{
				std::vector<Bytes>::iterator vIter = attr.fiberNodeNames.begin();
				for(vIter;vIter != attr.fiberNodeNames.end();vIter++)
				{
					attrLength += 2;//fiber node name length (2 bytes)
					attrLength += (uint16)vIter->size();
				}
				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		case Code_QamCapability:
			{
				attrLength += sizeof(attr.qamCapability);

				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		case Code_InputMaps:
			{
				std::vector<Bytes>::iterator vIter = attr.inputMaps.begin();
				attrLength += 4;//num of interfaces(4 bytes)
				for(vIter;vIter != attr.inputMaps.end();vIter++)
				{
					attrLength += 2;//length of host (2 bytes)
					attrLength += (uint16)vIter->size();
				}
				iter->attributeLength = attrLength;
				_messageLength += attrLength;
				break;
			}
		}
	}
	_attributeValue = attr;
}

void UpdataRequest::setRoute(uint8 typeflags, RoutAttriTypeCode typeCode, AttrbuteValue& attr)
{
	TypeCode type;
	type.attributeTypeFlags = typeflags;
	type.attributeTypeCode = typeCode;
	uint16 length = 0;
	switch(typeCode)
	{
	case Code_WithDrawRouts:
		_attributeValue.withdrawRoutes = attr.withdrawRoutes;
		length += sizeof(_attributeValue.withdrawRoutes);
		break;
	case Code_ReachableRoutes:
		_attributeValue.reachableRoutes = attr.reachableRoutes;
		length += sizeof(_attributeValue.reachableRoutes);
		break;
	case Code_NextHopServer:
		_attributeValue.nextHopServer = attr.nextHopServer;
		length += sizeof(_attributeValue.nextHopServer);
		break;
	case Code_QamNames:
		_attributeValue.qamNames = attr.qamNames;
		length += sizeof(_attributeValue.qamNames);
		break;
	case Code_CasCapability:
		_attributeValue.casCapability = attr.casCapability;
		length += sizeof(_attributeValue.casCapability);
		break;
	case Code_TotalBW:
		_attributeValue.totalBandwidth = attr.totalBandwidth;
		length += sizeof(_attributeValue.totalBandwidth);
		break;
	case Code_AvailableBW:
		_attributeValue.availableBandwidth = attr.availableBandwidth;
		length += sizeof(_attributeValue.availableBandwidth);
		break;
	case Code_Cost:
		_attributeValue.cost = attr.cost;
		length += sizeof(_attributeValue.cost);
		break;
	case Code_EdgeInputs:
		_attributeValue.edgeInputs = attr.edgeInputs;
		length += sizeof(_attributeValue.edgeInputs);
		break;
	case Code_QamChConfig:
		_attributeValue.qamChConfig = attr.qamChConfig;
		length += sizeof(_attributeValue.qamChConfig);
		break;
	case Code_UdpMap:
		_attributeValue.udpMap = attr.udpMap;
		length += sizeof(_attributeValue.udpMap);
		break;
	case Code_ServiceStatus:
		_attributeValue.serviceStatus = attr.serviceStatus;
		length += sizeof(_attributeValue.serviceStatus);
		break;
	case Code_MaxMPEGFlows:
		_attributeValue.maxMPEGFlows = attr.maxMPEGFlows;
		length += sizeof(_attributeValue.maxMPEGFlows);
		break;
	case Code_NextHopServerAlternate:
		_attributeValue.nextHopServerAlternate = attr.nextHopServerAlternate;
		length += sizeof(_attributeValue.nextHopServerAlternate);
		break;
	case Code_PortID:
		_attributeValue.portID = attr.portID;
		length += sizeof(_attributeValue.portID);
		break;
	case Code_FiberNodeNames:
		_attributeValue.fiberNodeNames = attr.fiberNodeNames;
		length += sizeof(_attributeValue.fiberNodeNames);
		break;
	case Code_QamCapability:
		_attributeValue.qamCapability = attr.qamCapability;
		length += sizeof(_attributeValue.qamCapability);
		break;
	case Code_InputMaps:
		_attributeValue.inputMaps = attr.inputMaps;
		length += sizeof(_attributeValue.inputMaps);
		break;
	default:
		return;
	}

	type.attributeLength = length;
	_attributeValue.typecodes.push_back(type);
	
}

/////////////////////////////////////////////////////////////////
/////////////// class KeepAliveRequest ///////////////
/////////////////////////////////////////////////////////////////
uint16 KeepAliveRequest::formatMessageBody(uint8* buf, size_t maxLen)
{
	return 0;
}

bool KeepAliveRequest::parseMessageBody(const uint8* buf, size_t messageLength)
{
	return true;
}

uint32 KeepAliveRequest::toMetaData(StringMap& metadata)
{
	toERRPHeader(metadata);
	return (uint32)metadata.size();
}

bool KeepAliveRequest::readMetaData(const StringMap& metadata)
{
	readERRPHeader(metadata);
	return true;
}

///////////////////////////////////////////////////////////////////
////////////////// class Notification ///////////////////
///////////////////////////////////////////////////////////////////
uint16 Notification::formatMessageBody(uint8* buf, size_t maxLen)
{
	uint8* p = buf;
	if (NULL == p)
		return 0;

	if(maxLen < _messageLength)
		return 0;

	memcpy(p,&_minBody,sizeof(_minBody));p += sizeof(_minBody);

	uint16 msgLength = bytesToString(_errorData,(char*)p);
	p += msgLength;

	return (uint16)(p - buf);
}

bool Notification::parseMessageBody(const uint8* buf, size_t messageLength)
{
	const uint8* p = buf;
	if (NULL == p)
		return false;

	_messageLength = (uint16)messageLength;

	if(messageLength < sizeof(_minBody))
		return false;

	//the minBody
	memcpy(&_minBody, p, sizeof(_minBody)); p+= sizeof(_minBody); 
	stringToBytes(_errorData,(char*)p,(_messageLength - sizeof(_minBody)));

	return true;
}

uint32 Notification::toMetaData(StringMap& metadata)
{
	toERRPHeader(metadata);
	char strTemp[33] = "";

	memset(strTemp,0,sizeof(strTemp));
	itoa(_minBody.errorCode,strTemp,10);
	std::string msgErrorCode = strTemp;

	memset(strTemp,0,sizeof(strTemp));
	itoa(_minBody.errorSubCode,strTemp,10);
	std::string msgErrorSubCode = strTemp;

	memset(strTemp,0,sizeof(strTemp));
	bytesToString(_errorData,strTemp);
	std::string msgErrorData = strTemp;

	MAPSET(StringMap,metadata,ERRPMD_NoErrorCode,msgErrorCode);
	MAPSET(StringMap,metadata,ERRPMD_NoErrorSubCode,msgErrorSubCode);
	MAPSET(StringMap,metadata,ERRPMD_NoErrorData,msgErrorData);

	return (uint32)metadata.size();
}

bool Notification::readMetaData(const StringMap& metadata)
{
	readERRPHeader(metadata);

	StringMap::const_iterator itorMD;

	itorMD = metadata.find(ERRPMD_NoErrorCode);
	if(itorMD->second.empty() || itorMD == metadata.end())
		return false;
	_minBody.errorCode = (uint8)atoi(itorMD->second.c_str());

	itorMD = metadata.find(ERRPMD_NoErrorSubCode);
	if(itorMD->second.empty() || itorMD == metadata.end())
		return false;
	_minBody.errorSubCode = (uint8)atoi(itorMD->second.c_str());

	itorMD = metadata.find(ERRPMD_NoErrorData);
	if(itorMD->second.empty() || itorMD == metadata.end())
		return false;
	stringToBytes(_errorData,itorMD->second.c_str(),itorMD->second.length());

	_messageLength = (uint16)sizeof(_minBody.errorCode) + (uint16)sizeof(_minBody.errorSubCode) + (uint16)_errorData.size();

	return true;
}

}// end namespace ERRP
}// end namespace ZQ

