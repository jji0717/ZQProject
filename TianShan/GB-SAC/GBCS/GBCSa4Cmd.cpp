#include "GBCSa4Cmd.h"
#include "ContentImpl.h"
#include "GBCSportal.h"	
#include "GBCSConfig.h"

extern ZQTianShan::GBCS::GBCSBaseConfig::GBCSHolder	*pGBCSBaseConfig;

namespace ZQTianShan {
namespace ContentStore {  

//a4 begin
A4FilePropagationReq::A4FilePropagationReq(const std::string& sourceUrl, const std::string& contentName, const ContentImpl& content, GBCSStorePortal* pPortalCtx, int maxTransferBitrate)
  : _a4FileProReq("AO_CDN_A4_FILE_PROPAGATION_REQ"), _sourceUrl(sourceUrl), _contentName(contentName), _content(content)
{
	_maxTransferBitrate = maxTransferBitrate;
	_pPortalCtx = pPortalCtx;
}

std::string  A4FilePropagationReq::makeContentHeader(void)
{
	char tempBuf[64] = {0};
	std::string senderId("GBVSS netID");
	std::string receiverId(pGBCSBaseConfig->_videoServer.VideoServerId);
	std::string transactionId(IGBCSCmd::getUUID());
	std::string version(pGBCSBaseConfig->_videoServer.VideoOnDemandVersion);
	std::string time(ZQTianShan::TimeToUTC(ZQTianShan::now(), tempBuf, sizeof(tempBuf)-2));  
	std::string opCode(_a4FileProReq);
	std::string msgType("REQ");

	size_t posProviderID = _contentName.find("_");
	senderId = _contentName.substr(posProviderID + 1);

	std::string contentHeader = "<Header>\n";

	contentHeader += "<SenderID>";
	contentHeader += senderId;
	contentHeader += "</SenderID>\n";

	contentHeader += "<ReceiverID>";
	contentHeader += receiverId;
	contentHeader += "</ReceiverID>\n";

	contentHeader += "<TransactionID>";
	contentHeader += transactionId;
	contentHeader += "</TransactionID>\n";

	contentHeader += "<Version>";
	contentHeader += version;
	contentHeader += "</Version>\n";

	contentHeader += "<Time>";
	contentHeader += time;
	contentHeader += "</Time>\n";

	contentHeader += "<OpCode>";
	contentHeader += opCode;
	contentHeader += "</OpCode>\n";

	contentHeader += "<MsgType>";
	contentHeader += msgType;
	contentHeader += "</MsgType>\n";

	contentHeader += "</Header>\n";	 

	return contentHeader;
}

std::string  A4FilePropagationReq::makeContentBody(void)
{
	char  storeBuf[64] = {0};
	std::string contentID;	       //assetID
	std::string contentName;       
	std::string volumeName;	       //volumeName
	std::string transferBitRate;   //transferBitRate
	std::string responseURL;	   //responseURL

	std::string sourceURL;		   //sourceURL
	std::string userName;		   //userName
	std::string password;		   //password

	std::string inputInfo;
	size_t pos = _contentName.find("_");

	contentID   = GBCSCmdUtil::setAttrStr("contentID",   _contentName.substr(0, pos));	 
	contentName = GBCSCmdUtil::setAttrStr("contentName", _content._name());

	TianShanIce::Storage::VolumeExPrx  vol = _content._volume();
	std::string volName = vol->getName();
	std::string targetVolName = vol->getMountPath();
	size_t  len = targetVolName.length();
	if (FNSEPC == targetVolName[len -1] || LOGIC_FNSEPC == targetVolName[len -1])
		targetVolName = targetVolName.substr(0, len -1);

	volumeName = GBCSCmdUtil::setAttrStr("volumeName", targetVolName);
	sprintf(storeBuf, "%d", _maxTransferBitrate);
	transferBitRate = GBCSCmdUtil::setAttrStr("transferBitRate", storeBuf);

	memset(storeBuf, 0, sizeof(storeBuf));
	std::string responseAddr(_pPortalCtx->getResponseAddr());
	snprintf(storeBuf, sizeof(storeBuf) - 2, "http://%s/", responseAddr.c_str());
	responseURL  = GBCSCmdUtil::setAttrStr("responseURL", storeBuf);

	size_t index = _sourceUrl.find("@", 0);
	size_t sBI   = _sourceUrl.find("//");
	size_t sEI   = _sourceUrl.find(":", sBI);
	std::string proto;
	if (sBI != std::string::npos)
		proto = _sourceUrl.substr(0, sBI+2);

	if (index == std::string::npos) //not find
		sourceURL = GBCSCmdUtil::setAttrStr("sourceURL", _sourceUrl);
	else
	{
		sourceURL = GBCSCmdUtil::setAttrStr("sourceURL", proto + _sourceUrl.substr(index+1));
		userName  = GBCSCmdUtil::setAttrStr("userName",  _sourceUrl.substr(sBI+2, sEI-sBI-2));
		password  = GBCSCmdUtil::setAttrStr("password",  _sourceUrl.substr(sEI+1, index-sEI-1));
	}

	std::string contentInfo;
	std::string contentBody;

	inputInfo += "<InputURL  ";
	inputInfo += sourceURL;
	inputInfo += userName;
	inputInfo += password;
	inputInfo += " >\n";
	inputInfo += "</InputURL>";

	contentInfo += "<ContentInfo   ";
	contentInfo += contentID;
	contentInfo += contentName;
	contentInfo += volumeName;
	contentInfo += transferBitRate;
	contentInfo += responseURL;
	contentInfo += " >\n";
	contentInfo += inputInfo;
	contentInfo += "</ContentInfo>\n";

	contentBody = "<Body>\n";
	contentBody += contentInfo;
	contentBody += "</Body>";

	return contentBody;
}

std::map<std::string, std::string>  A4FilePropagationReq::parseHttpResponse(std::string & httpResponse)
{
	std::map<std::string, std::string> a4FilePropagationResp;

	a4FilePropagationResp["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::SUCCEED];
	if (httpResponse.empty())
	{   a4FilePropagationResp["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::INPUT_XML_EMPTY];
		return a4FilePropagationResp;
	}

	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	if(!xmlDoc.read((void*)httpResponse.c_str(), (int)httpResponse.length(), 1))
	{
		a4FilePropagationResp["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::READ_FAILED];
		return a4FilePropagationResp;
	}

	ZQ::common::XMLPreferenceEx* pRoot = xmlDoc.getRootPreference();
	if (!pRoot)	 
		a4FilePropagationResp["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_ROOTPREFERENCE_FAILED];
	else  
	{
		ZQ::common::XMLPreferenceEx* header = pRoot->findChild("Header");
		if (!header)
			a4FilePropagationResp["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_HEADER_FAILED];
		else
		{
			for(ZQ::common::XMLPreferenceEx* pNext = header->firstChild(); pNext != NULL; pNext = header->nextChild())
			{
				const int preferenceSize = 128;
				char preferenceName[preferenceSize] = {0};
				char preferenceText[preferenceSize] = {0};
				pNext->getPreferenceName(preferenceName, true, preferenceSize);
				pNext->getPreferenceText(preferenceText, preferenceSize);
				a4FilePropagationResp[preferenceName] = preferenceText;
				pNext->free(); pNext = NULL;
			}

			header->free();	header = NULL;
		}

		pRoot->free(); pRoot = NULL;
	}

	xmlDoc.clear();
	return a4FilePropagationResp;
}



A4BatFilePropagationReq::A4BatFilePropagationReq()
	:_a4CmdName("AO_CDN_A4_FILE_BAT_PROPAGATION_REQ")
{
	//TODO: not been acknowledge yet
}

std::string A4BatFilePropagationReq::makeContentHeader(void)
{
	//TODO: not been acknowledged yet in  a4
	return std::string();
}

std::string  A4BatFilePropagationReq::makeContentBody(void)
{
	//TODO: not been acknowledged yet in  a4
	return std::string();
}

std::map<std::string, std::string>  A4BatFilePropagationReq::parseHttpResponse(std::string & httpResponse)
{
	//TODO: not been acknowledged yet in  a4
	return std::map<std::string, std::string>();
}



A4FileStateReq::A4FileStateReq(std::string & contentName, ContentImpl& content)
	:_a4FileStateReq("AO_CDN_A4_FILE_STATE_REQ"), _contentName(contentName), _content(content)
{
	//TODO: GetTransferStatus in background thread, ignore in this stage
}

std::string  A4FileStateReq::makeContentHeader(void)
{
	char tempBuf[64] = {0};
	std::string senderId("GBVSS netID");
	std::string receiverId(pGBCSBaseConfig->_videoServer.VideoServerId);
	std::string transactionId(IGBCSCmd::getUUID());
	std::string version(pGBCSBaseConfig->_videoServer.VideoOnDemandVersion);
	std::string time(ZQTianShan::TimeToUTC(ZQTianShan::now(), tempBuf, sizeof(tempBuf)-2));  
	std::string opCode(_a4FileStateReq);
	std::string msgType("REQ");

	size_t posProviderID = _contentName.find("_");
	senderId = _contentName.substr(posProviderID + 1);

	std::string contentHeader = "<Header>\n";

	contentHeader += "<SenderID>";
	contentHeader += senderId;
	contentHeader += "</SenderID>\n";

	contentHeader += "<ReceiverID>";
	contentHeader += receiverId;
	contentHeader += "</ReceiverID>\n";

	contentHeader += "<TransactionID>";
	contentHeader += transactionId;
	contentHeader += "</TransactionID>\n";

	contentHeader += "<Version>";
	contentHeader += version;
	contentHeader += "</Version>\n";

	contentHeader += "<Time>";
	contentHeader += time;
	contentHeader += "</Time>\n";

	contentHeader += "<OpCode>";
	contentHeader += opCode;
	contentHeader += "</OpCode>\n";

	contentHeader += "<MsgType>";
	contentHeader += msgType;
	contentHeader += "</MsgType>\n";

	contentHeader += "</Header>\n";	 

	return contentHeader;
}

std::string  A4FileStateReq::makeContentBody(void)
{
	std::string contentID;
	std::string volumeName;

	try
	{
		TianShanIce::Storage::VolumeExPrx vol =  _content._volume();
		std::string  volume(vol->getMountPath());
		size_t pos = volume.length() - 1;
		if (volume[pos] == FNSEPC) {volume = volume.substr(0, pos);}

		volumeName = GBCSCmdUtil::setAttrStr("volumeName", volume);
	}
	catch (...)
	{
	}

	size_t  posContentID  = _contentName.find("_");
	contentID  = GBCSCmdUtil::setAttrStr("contentID", _contentName.substr(0, posContentID));

	std::string queryContentInfo = "<QueryContentInfo  ";
	queryContentInfo += contentID;
	queryContentInfo += volumeName;
	queryContentInfo += " >\n";
	queryContentInfo += "</QueryContentInfo>\n";

	std::string  fileStateReqBody = "<Body>\n";
	fileStateReqBody += queryContentInfo;
	fileStateReqBody += "</Body>";

	return fileStateReqBody;
}

std::map<std::string, std::string>    A4FileStateReq::parseHttpResponse(std::string & httpResponse)
{
	std::map<std::string, std::string> a4FileStateReqMap;

	a4FileStateReqMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::SUCCEED];
	if (httpResponse.empty())
	{
		a4FileStateReqMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::INPUT_XML_EMPTY];
		return a4FileStateReqMap;
	}

	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	if(!xmlDoc.read((void*)httpResponse.c_str(), (int)httpResponse.length(), 1))
	{
		xmlDoc.clear();
		a4FileStateReqMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::READ_FAILED];
		return a4FileStateReqMap;
	}

	ZQ::common::XMLPreferenceEx* pRoot = xmlDoc.getRootPreference();
	if (!pRoot)	 
		a4FileStateReqMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_ROOTPREFERENCE_FAILED];
	else  
	{
		ZQ::common::XMLPreferenceEx* header = pRoot->findChild("Header");
		if (!header)
			a4FileStateReqMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_HEADER_FAILED];
		else
		{
			for(ZQ::common::XMLPreferenceEx* pNext = header->firstChild(); pNext != NULL; pNext = header->nextChild())
			{
				const int preferenceSize = 128;
				char preferenceName[preferenceSize] = {0};
				char preferenceText[preferenceSize] = {0};
				pNext->getPreferenceName(preferenceName, true, preferenceSize);
				pNext->getPreferenceText(preferenceText, preferenceSize);
				a4FileStateReqMap[preferenceName] = preferenceText;
				pNext->free(); pNext = NULL;
			}

			header->free(); header = NULL;
			ZQ::common::XMLPreferenceEx* body = pRoot->findChild("Body");
			if (!body)
				a4FileStateReqMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_BODY_FAILED];
			else		
			{
				for(ZQ::common::XMLPreferenceEx* pBodyChild = body->firstChild(); pBodyChild != NULL; pBodyChild = body->nextChild())
				{
					std::map<std::string, std::string> nodeM = pBodyChild->getProperties();
					a4FileStateReqMap.insert(nodeM.begin(), nodeM.end());
					pBodyChild->free();	pBodyChild = NULL;
				}

				body->free(); body = NULL;
				std::map<std::string, std::string>::iterator itOpCode = a4FileStateReqMap.find("OpCode");
				if(itOpCode != a4FileStateReqMap.end())
				{
					std::string opCode = itOpCode->second;
					if(opCode != _a4FileStateReq)
						a4FileStateReqMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::OPCODE_FAILED];
				}
			} 
		}

		pRoot->free();   pRoot = NULL;
	}

	xmlDoc.clear();
	return a4FileStateReqMap;
}



A4FileStateNotify::A4FileStateNotify()
	:_a4FileStateNotify("CDN_AO_A4_FILE_STATE_NOTIFY"), _errorMessage("OK"), _returnCode("0200")
{
	_transactionId = IGBCSCmd::getUUID();
}

std::string A4FileStateNotify::makeContentHeader(void)
{
	char tempBuf[64] = {0};
	std::string time(ZQTianShan::TimeToUTC(ZQTianShan::now(), tempBuf, sizeof(tempBuf)-2));  
	std::string opCode(_a4FileStateNotify);
	std::string msgType("RESP");

	std::string contentHeader = "<Header>\n";

	contentHeader += "<TransactionID>";
	contentHeader += _transactionId;
	contentHeader += "</TransactionID>\n";

	contentHeader += "<Time>";
	contentHeader += time;
	contentHeader += "</Time>\n";

	contentHeader += "<OpCode>";
	contentHeader += opCode;
	contentHeader += "</OpCode>\n";

	contentHeader += "<MsgType>";
	contentHeader += msgType;
	contentHeader += "</MsgType>\n";

	contentHeader += "<ReturnCode>";
	contentHeader += _returnCode;
	contentHeader += "</ReturnCode>\n";

	contentHeader += "<ErrorMessage>";
	contentHeader += _errorMessage;
	contentHeader += "</ErrorMessage>";

	contentHeader += "</Header>\n";	 

	return contentHeader;
}

std::string A4FileStateNotify::makeContentBody(void)
{
	std::string body("<Body>\n</Body>");
	return body;
}

std::map<std::string, std::string>  A4FileStateNotify::parseHttpResponse(const std::string & httpResponse)
{
	std::string httpResponseBuf(httpResponse);
	return parseHttpResponse(httpResponseBuf);
}

std::map<std::string, std::string>  A4FileStateNotify::parseHttpResponse(std::string & httpResponse)
{
	std::map<std::string, std::string> a4FileStateNotifyMap;

	a4FileStateNotifyMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::SUCCEED];
	if (httpResponse.empty())
	{
		a4FileStateNotifyMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::INPUT_XML_EMPTY];
		return a4FileStateNotifyMap;
	}

	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	if(!xmlDoc.read((void*)httpResponse.c_str(), (int)httpResponse.length(), 1))
	{
		xmlDoc.clear();
		a4FileStateNotifyMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::READ_FAILED];
		return a4FileStateNotifyMap;
	}

	ZQ::common::XMLPreferenceEx* pRoot = xmlDoc.getRootPreference();
	if (!pRoot)	 
		a4FileStateNotifyMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_ROOTPREFERENCE_FAILED];
	else  
	{
		ZQ::common::XMLPreferenceEx* header = pRoot->findChild("Header");
		if (!header)
			a4FileStateNotifyMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_HEADER_FAILED];
		else		
		{
			for(ZQ::common::XMLPreferenceEx* pNext = header->firstChild(); pNext != NULL; pNext = header->nextChild())
			{
				const int preferenceSize = 128;
				char preferenceName[preferenceSize] = {0};
				char preferenceText[preferenceSize] = {0};
				pNext->getPreferenceName(preferenceName, true, preferenceSize);
				pNext->getPreferenceText(preferenceText, preferenceSize);
				a4FileStateNotifyMap[preferenceName] = preferenceText;
				pNext->free(); pNext = NULL;
			}

			header->free(); header = NULL;
			ZQ::common::XMLPreferenceEx* body = pRoot->findChild("Body");
			if (!body)
				a4FileStateNotifyMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_BODY_FAILED];
			else			
			{
				body->free(); 
				std::map<std::string, std::string>::iterator itOpCode = a4FileStateNotifyMap.find("OpCode");
				if(itOpCode != a4FileStateNotifyMap.end())
				{
					std::string opCode = itOpCode->second;
					if(opCode != _a4FileStateNotify)
						a4FileStateNotifyMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::OPCODE_FAILED];
				}
			}
		}

		pRoot->free(); pRoot = NULL;
	}
	
	std::map<std::string, std::string>::iterator itTransId = a4FileStateNotifyMap.find("TransactionID");
	if(itTransId != a4FileStateNotifyMap.end())
		_transactionId = itTransId->second;

	xmlDoc.clear();
	return a4FileStateNotifyMap;
}

int A4FileStateNotify::setTransactionId(std::string transactionId)
{
	if(transactionId.empty())
		return false;

	_transactionId = transactionId;
	return true;
}

int A4FileStateNotify::setErrorMessage(std::string errorMessage)
{
	if(errorMessage.empty())
		return false;

	_errorMessage = errorMessage;
	return true;
}

int A4FileStateNotify::setReturnCode(std::string returnCode)
{
	if(returnCode.empty())
		return false;

	_returnCode = returnCode;
	return true;
}



A4FilePropagationCancel::A4FilePropagationCancel(std::string & contentName, ContentImpl& content, int reasonCode /*= CANCEL_OPR_INIT*/)
	:_a4FileProCancel("AO_CDN_A4_FILE_PROPAGATION_CANCEL"), _contentName(contentName), _content(content), _reasonCode(reasonCode)
{

}

std::string  A4FilePropagationCancel::makeContentHeader(void)
{
	char tempBuf[64] = {0};
	std::string senderId("GBVSS netID");
	std::string receiverId(pGBCSBaseConfig->_videoServer.VideoServerId);
	std::string transactionId(IGBCSCmd::getUUID());
	std::string version(pGBCSBaseConfig->_videoServer.VideoOnDemandVersion);
	std::string time(ZQTianShan::TimeToUTC(ZQTianShan::now(), tempBuf, sizeof(tempBuf)-2));  
	std::string opCode(_a4FileProCancel);
	std::string msgType("REQ");

	size_t posProviderID = _contentName.find("_");
	senderId = _contentName.substr(posProviderID + 1);

	std::string contentHeader = "<Header>\n";

	contentHeader += "<SenderID>";
	contentHeader += senderId;
	contentHeader += "</SenderID>\n";

	contentHeader += "<ReceiverID>";
	contentHeader += receiverId;
	contentHeader += "</ReceiverID>\n";

	contentHeader += "<TransactionID>";
	contentHeader += transactionId;
	contentHeader += "</TransactionID>\n";

	contentHeader += "<Version>";
	contentHeader += version;
	contentHeader += "</Version>\n";

	contentHeader += "<Time>";
	contentHeader += time;
	contentHeader += "</Time>\n";

	contentHeader += "<OpCode>";
	contentHeader += opCode;
	contentHeader += "</OpCode>\n";

	contentHeader += "<MsgType>";
	contentHeader += msgType;
	contentHeader += "</MsgType>\n";

	contentHeader += "</Header>\n";	 

	return contentHeader;
}


std::string A4FilePropagationCancel::makeContentBody(void)
{
	std::string contentID;
	std::string volumeName;
	std::string reasonCode;

	TianShanIce::Storage::VolumeExPrx vol =  _content._volume();
	std::string  volume(vol->getMountPath());
	size_t pos = volume.length() - 1;
	if (volume[pos] == FNSEPC)
		volume = volume.substr(0, pos);

	char  reasonCodeBuf[64] = {0};
	sprintf(reasonCodeBuf, "%d", _reasonCode);
	size_t  posContentID  = _contentName.find("_");
	contentID  = GBCSCmdUtil::setAttrStr("contentID", _contentName.substr(0, posContentID));
	volumeName = GBCSCmdUtil::setAttrStr("volumeName", volume);
	reasonCode = GBCSCmdUtil::setAttrStr("reasonCode", reasonCodeBuf);

	std::string cancelInfo = "<CancelInfo  ";
	cancelInfo += contentID;
	cancelInfo += volumeName;
	cancelInfo += reasonCode;
	cancelInfo += " >\n";
	cancelInfo += "</CancelInfo>\n";

	std::string  cancelContentBody = "<Body>\n";
	cancelContentBody += cancelInfo;
	cancelContentBody += "</Body>";

	return cancelContentBody;
}

std::map<std::string, std::string> A4FilePropagationCancel::parseHttpResponse(std::string & httpResponse)
{
	std::map<std::string, std::string> a4FileProCancel;

	a4FileProCancel["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::SUCCEED];
	if (httpResponse.empty())
	{
		a4FileProCancel["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::INPUT_XML_EMPTY];
		return a4FileProCancel;
	}

	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	if(!xmlDoc.read((void*)httpResponse.c_str(), (int)httpResponse.length(), 1))
	{
		xmlDoc.clear();
		a4FileProCancel["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::READ_FAILED];
		return a4FileProCancel;
	}

	ZQ::common::XMLPreferenceEx* pRoot = xmlDoc.getRootPreference();
	if (!pRoot)	 
		a4FileProCancel["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_ROOTPREFERENCE_FAILED];
	else  
	{
		ZQ::common::XMLPreferenceEx* header = pRoot->findChild("Header");
		if (!header)
			a4FileProCancel["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_HEADER_FAILED];
		else		
		{
			for(ZQ::common::XMLPreferenceEx* pNext = header->firstChild(); pNext != NULL; pNext = header->nextChild())
			{
				const int preferenceSize = 128;
				char preferenceName[preferenceSize] = {0};
				char preferenceText[preferenceSize] = {0};
				pNext->getPreferenceName(preferenceName, true, preferenceSize);
				pNext->getPreferenceText(preferenceText, preferenceSize);
				a4FileProCancel[preferenceName] = preferenceText;
				pNext->free(); pNext = NULL;
			}

			header->free(); header = NULL;
		}

		pRoot->free(); pRoot = NULL;
	}

	xmlDoc.clear();
	return a4FileProCancel;
}



A4FileDelete::A4FileDelete(std::string & contentName, const ContentImpl& content, int reasonCode /*= DELETE_OPR_INIT*/)
	:_a4FileDel("AO_CDN_A4_FILE_DELETE"), _contentName(contentName), _content(content), _reasonCode(reasonCode)
{
}

std::string A4FileDelete::makeContentHeader(void)
{
	char tempBuf[64] = {0};
	std::string senderId("GBVSS netID");
	std::string receiverId(pGBCSBaseConfig->_videoServer.VideoServerId);
	std::string transactionId(IGBCSCmd::getUUID());
	std::string version(pGBCSBaseConfig->_videoServer.VideoOnDemandVersion);
	std::string time(ZQTianShan::TimeToUTC(ZQTianShan::now(), tempBuf, sizeof(tempBuf)-2));  
	std::string opCode(_a4FileDel);
	std::string msgType("REQ");

	size_t posProviderID = _contentName.find("_");
	senderId = _contentName.substr(posProviderID + 1);

	std::string contentHeader = "<Header>\n";

	contentHeader += "<SenderID>";
	contentHeader += senderId;
	contentHeader += "</SenderID>\n";

	contentHeader += "<ReceiverID>";
	contentHeader += receiverId;
	contentHeader += "</ReceiverID>\n";

	contentHeader += "<TransactionID>";
	contentHeader += transactionId;
	contentHeader += "</TransactionID>\n";

	contentHeader += "<Version>";
	contentHeader += version;
	contentHeader += "</Version>\n";

	contentHeader += "<Time>";
	contentHeader += time;
	contentHeader += "</Time>\n";

	contentHeader += "<OpCode>";
	contentHeader += opCode;
	contentHeader += "</OpCode>\n";

	contentHeader += "<MsgType>";
	contentHeader += msgType;
	contentHeader += "</MsgType>\n";

	contentHeader += "</Header>\n";	 

	return contentHeader;
}

std::string  A4FileDelete::makeContentBody(void)
{
	std::string contentID;
	std::string volumeName;
	std::string reasonCode;

	TianShanIce::Storage::VolumeExPrx vol =  _content._volume();
	std::string  volume(vol->getMountPath());
	size_t pos = volume.length() - 1;
	if (volume[pos] == FNSEPC)
		volume = volume.substr(0, pos);

	char  reasonCodeBuf[64] = {0};
	sprintf(reasonCodeBuf, "%d", _reasonCode);
	size_t  posContentID  = _contentName.find("_");
	contentID  = GBCSCmdUtil::setAttrStr("contentID", _contentName.substr(0, posContentID));
	volumeName = GBCSCmdUtil::setAttrStr("volumeName", volume);
	reasonCode = GBCSCmdUtil::setAttrStr("reasonCode", reasonCodeBuf);

	std::string deleteInfo = "<DeleteInfo  ";
	deleteInfo += contentID;
	deleteInfo += volumeName;
	deleteInfo += reasonCode;
	deleteInfo += " />\n";

	std::string  cancelContentBody = "<Body>\n";
	cancelContentBody += deleteInfo;
	cancelContentBody += "</Body>";

	return cancelContentBody;
}

std::map<std::string, std::string>  A4FileDelete::parseHttpResponse(std::string & httpResponse)
{
	std::map<std::string, std::string> a4FileDelMap;

	a4FileDelMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::SUCCEED];
	if (httpResponse.empty())
	{
		a4FileDelMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::INPUT_XML_EMPTY];
		return a4FileDelMap;
	}
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	if(!xmlDoc.read((void*)httpResponse.c_str(), (int)httpResponse.length(), 1))
	{
		xmlDoc.clear();
		a4FileDelMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::READ_FAILED];
		return a4FileDelMap;
	}

	ZQ::common::XMLPreferenceEx* pRoot = xmlDoc.getRootPreference();
	if (!pRoot)	 
		a4FileDelMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_ROOTPREFERENCE_FAILED];
	else  
	{
		ZQ::common::XMLPreferenceEx* header = pRoot->findChild("Header");
		if (!header)
			a4FileDelMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_HEADER_FAILED];
		else																							   		
		{
			for(ZQ::common::XMLPreferenceEx* pNext = header->firstChild(); pNext != NULL; pNext = header->nextChild())
			{
				const int preferenceSize = 128;
				char preferenceName[preferenceSize] = {0};
				char preferenceText[preferenceSize] = {0};
				pNext->getPreferenceName(preferenceName, true, preferenceSize);
				pNext->getPreferenceText(preferenceText, preferenceSize);
				a4FileDelMap[preferenceName] = preferenceText;
				pNext->free(); pNext = NULL;
			}

			header->free(); header = NULL;
		}

		pRoot->free(); pRoot =NULL;
	}

	xmlDoc.clear();
	return a4FileDelMap;
};

//a4 end

}//namespace  ContentStore
}//	namespace ZQTianShan 