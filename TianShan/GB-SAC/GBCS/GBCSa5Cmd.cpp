#include "GBCSa5Cmd.h"
#include "GBCSConfig.h"

extern ZQTianShan::GBCS::GBCSBaseConfig::GBCSHolder	*pGBCSBaseConfig;

namespace ZQTianShan {
namespace ContentStore {

using namespace ZQTianShan::ContentStore;

A5StreamIngestReq::A5StreamIngestReq(const std::string & sourceUrl, const std::string & contentName, ContentImpl& content,
									 ContentStoreImpl & store, int maxTransferBitrate, const std::string& sourceType, 
									 const std::string& startTimeUTC, const std::string& stopTimeUTC, std::string & responseAddr)
	:_maxTransferBitrate(maxTransferBitrate), 
	 _a5StreamIngestReq("AO_CDN_A5_STREAM_INGEST_REQ"),
	 _sourceUrl(sourceUrl),	_contentName(contentName), _content(content),
	 _store(store), _sourceType(sourceType), _startTimeUTC(startTimeUTC), 
	 _stopTimeUTC(stopTimeUTC), _responseAddr(responseAddr)
{}

std::string  A5StreamIngestReq::makeContentHeader(void)
{
	char tempBuf[64] = {0};
	std::string senderId("GBVSS netID");
	std::string receiverId(pGBCSBaseConfig->_videoServer.VideoServerId);
	std::string transactionId(IGBCSCmd::getUUID());
	std::string version(pGBCSBaseConfig->_videoServer.VideoOnDemandVersion);
	std::string time(ZQTianShan::TimeToUTC(ZQTianShan::now(), tempBuf, sizeof(tempBuf)-2));  
	std::string opCode(_a5StreamIngestReq);
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

std::string  A5StreamIngestReq::makeContentBody(void)
{
	std::string inputURL;
	std::string list;
	std::string transferInfo;
	std::string body;
	std::string sourceURL;
	std::string sourceIP;//TODO: Get??

	size_t index = _sourceUrl.find("@", 0);
	size_t sBI   = _sourceUrl.find("//");
	size_t sEI   = _sourceUrl.find(":", sBI);
	std::string proto;
	if (sBI != std::string::npos)
		proto = _sourceUrl.substr(0, sBI+2);

	if (index == std::string::npos) //not find
		sourceURL = GBCSCmdUtil::setAttrStr("sourceURL", _sourceUrl);
	else
		sourceURL = GBCSCmdUtil::setAttrStr("sourceURL", proto + _sourceUrl.substr(index+1)); 

	sourceIP     = GBCSCmdUtil::setAttrStr("sourceIP", _store._netId);	//TODO: need to confirm
	inputURL     = GBCSCmdUtil::setElementStr(std::string("InputURL"),  sourceURL + sourceIP,  true);
	list         = GBCSCmdUtil::setElementStr(std::string("<List>"), inputURL, std::string("</List>"));
	transferInfo = GBCSCmdUtil::setElementStr(makeTransferInfoTagStart(), list, std::string("</TransferInfo>"));
	body         = GBCSCmdUtil::setElementStr(std::string("<Body>"), transferInfo, std::string("</Body>"));

	return body;
}

std::string A5StreamIngestReq::makeTransferInfoTagStart(void)
{
	char  storeBuf[64] = {0};
	size_t pos  = _contentName.find("_");
	std::string contentID("contentID");	 //
	std::string providerID("providerID");  //TODO: need  to confirm(contentName)
	std::string contentName("contentName"); //TODO: need  to confirm(providerID)
	std::string volumeName("volumeName");//
	std::string transferBitRate("transferBitRate"); //
	std::string recordType("recordType");   //
	std::string captureStart("captureStart");//
	std::string captureEnd("captureEnd");//
	std::string timeShiftDuration;	//TODO: GET??
	std::string responseURL("responseURL");	//
	std::string transferInfoTagStart("<TransferInfo ");

	transferInfoTagStart += GBCSCmdUtil::setAttrStr(contentID,   _contentName.substr(0, pos));
	transferInfoTagStart += GBCSCmdUtil::setAttrStr(providerID,  _contentName.substr(pos + 1));
	transferInfoTagStart += GBCSCmdUtil::setAttrStr(contentName, _contentName.substr(pos + 1));

	TianShanIce::Storage::VolumeExPrx  vol = _content._volume();
	std::string volName = vol->getName();
	std::string targetVolName = vol->getMountPath();
	size_t  len = targetVolName.length();
	if (FNSEPC == targetVolName[len -1] || LOGIC_FNSEPC == targetVolName[len -1])
		targetVolName = targetVolName.substr(0, len -1);

	sprintf(storeBuf, "%d", _maxTransferBitrate);
	transferInfoTagStart += GBCSCmdUtil::setAttrStr(volumeName, targetVolName);
	transferInfoTagStart += GBCSCmdUtil::setAttrStr(transferBitRate, storeBuf);
	if(!_responseAddr.empty())
	{
	    memset(storeBuf, 0, sizeof(storeBuf));
	    snprintf(storeBuf, sizeof(storeBuf) - 2, "http://%s/", _responseAddr.c_str());
	    transferInfoTagStart += GBCSCmdUtil::setAttrStr(responseURL, storeBuf);
	}

	if (19 == _startTimeUTC.length())
		transferInfoTagStart += GBCSCmdUtil::setAttrStr(captureStart, _startTimeUTC);

	if (19 == _stopTimeUTC.length())
		transferInfoTagStart += GBCSCmdUtil::setAttrStr(captureEnd, _stopTimeUTC);

    transferInfoTagStart += " >";
	return transferInfoTagStart;
}


A5StreamBatIngestReq::A5StreamBatIngestReq()
{ 
	// TODO: n/a
}  

std::map<std::string, std::string>  A5StreamBatIngestReq::parseHttpResponse(std::string & httpResponse)
{
	// TODO: n/a
	return std::map<std::string, std::string>();
}

std::string  A5StreamBatIngestReq::makeContentBody(void)
{
	// TODO: n/a
	return std::string();
}

std::string  A5StreamBatIngestReq::makeContentHeader(void)
{
	// TODO: n/a
	return std::string();
}


A5StreamStateNotify::A5StreamStateNotify() 
: _a5StreamStateNotify("CDN_AO_A5_STREAM_STATE_NOTIFY"), _returnCode("0200"), _errorMessage("OK")
{
	_transactionId = IGBCSCmd::getUUID();
}

std::string  A5StreamStateNotify::makeContentBody(void)
{
	std::string body("<Body>\n</Body>");
	return body;
}

std::string  A5StreamStateNotify::makeContentHeader(void)
{
	char tempBuf[64] = {0};
	std::string time(ZQTianShan::TimeToUTC(ZQTianShan::now(), tempBuf, sizeof(tempBuf)-2));  
	std::string opCode(_a5StreamStateNotify);
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

std::map<std::string, std::string>  A5StreamStateNotify::parseHttpResponse(std::string & httpResponse)
{
	std::map<std::string, std::string> a5StreamStateNotifyMap;

	a5StreamStateNotifyMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::SUCCEED];
	if (httpResponse.empty())
	{
		a5StreamStateNotifyMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::INPUT_XML_EMPTY];
		return a5StreamStateNotifyMap;
	}

	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	if(!xmlDoc.read((void*)httpResponse.c_str(), (int)httpResponse.length(), 1))
	{
		xmlDoc.clear();
		a5StreamStateNotifyMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::READ_FAILED];
		return a5StreamStateNotifyMap;
	}

	ZQ::common::XMLPreferenceEx* pRoot = xmlDoc.getRootPreference();
	if (!pRoot)	 
		a5StreamStateNotifyMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_ROOTPREFERENCE_FAILED];
	else  
	{
		ZQ::common::XMLPreferenceEx* header = pRoot->findChild("Header");
		if (!header)
			a5StreamStateNotifyMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_HEADER_FAILED];
		else
		{
			for(ZQ::common::XMLPreferenceEx* pNext = header->firstChild(); pNext != NULL; pNext = header->nextChild())
			{
				const int preferenceSize = 128;
				char preferenceName[preferenceSize] = {0};
				char preferenceText[preferenceSize] = {0};
				pNext->getPreferenceName(preferenceName, true, preferenceSize);
				pNext->getPreferenceText(preferenceText, preferenceSize);
				a5StreamStateNotifyMap[preferenceName] = preferenceText;
				pNext->free(); pNext = NULL;
			}

			header->free();	header = NULL;
			ZQ::common::XMLPreferenceEx* body = pRoot->findChild("Body");
			if (!body)
				a5StreamStateNotifyMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_BODY_FAILED];
			else
			{
				for(ZQ::common::XMLPreferenceEx* pBodyChild = body->firstChild(); pBodyChild != NULL; pBodyChild = body->nextChild())
				{
					std::map<std::string, std::string> nodeM = pBodyChild->getProperties();
					a5StreamStateNotifyMap.insert(nodeM.begin(), nodeM.end());
					pBodyChild->free();	pBodyChild = NULL;
				}

				body->free(); body = NULL;
				std::map<std::string, std::string>::iterator itOpCode = a5StreamStateNotifyMap.find("OpCode");
				if(itOpCode != a5StreamStateNotifyMap.end())
				{
					std::string opCode = itOpCode->second;
					if(opCode != _a5StreamStateNotify)
						a5StreamStateNotifyMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::OPCODE_FAILED];
				}
			}
		}

		pRoot->free(); pRoot = NULL;
	}

	std::map<std::string, std::string>::iterator itTransId = a5StreamStateNotifyMap.find("TransactionID");
	if(itTransId != a5StreamStateNotifyMap.end())
		_transactionId = itTransId->second;

	xmlDoc.clear();
	return a5StreamStateNotifyMap;
}

int A5StreamStateNotify::setTransactionId(std::string transactionId)
{
	if(transactionId.empty())
		return false;

	_transactionId = transactionId;
	return true;
}

int A5StreamStateNotify::setErrorMessage(std::string errorMessage)
{
	if(errorMessage.empty())
		return false;

	_errorMessage = errorMessage;
	return true;
}

int A5StreamStateNotify::setReturnCode(std::string returnCode)
{
	if(returnCode.empty())
		return false;

	_returnCode = returnCode;
	return true;
}

A5StreamStateReq::A5StreamStateReq(std::string & contentName, ContentImpl& content)
: _a5StreamStateReq("AO_CDN_A5_STREAM_STATE_REQ"), _contentName(contentName), _content(content)
{
}

std::string  A5StreamStateReq::makeContentHeader(void)
{
	char tempBuf[64] = {0};
	std::string senderId("GBVSS netID");
	std::string receiverId(pGBCSBaseConfig->_videoServer.VideoServerId);
	std::string transactionId(IGBCSCmd::getUUID());
	std::string version(pGBCSBaseConfig->_videoServer.VideoOnDemandVersion);
	std::string time(ZQTianShan::TimeToUTC(ZQTianShan::now(), tempBuf, sizeof(tempBuf)-2));  
	std::string opCode(_a5StreamStateReq);
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

std::string  A5StreamStateReq::makeContentBody(void)
{
	std::string contentID;
	std::string volumeName;

	TianShanIce::Storage::VolumeExPrx vol =  _content._volume();
	std::string  volume(vol->getMountPath());
	size_t pos = volume.length() - 1;
	if (volume[pos] == FNSEPC)
		volume = volume.substr(0, pos);

	size_t  posContentID  = _contentName.find("_");
	contentID  = GBCSCmdUtil::setAttrStr("contentID", _contentName.substr(0, posContentID));
	volumeName = GBCSCmdUtil::setAttrStr("volumeName", volume);

	std::string queryContentInf = "<QueryContentInfo  ";
	queryContentInf += contentID;
	queryContentInf += volumeName;
	queryContentInf += " >\n";
	queryContentInf += "</QueryContentInfo>\n";

	std::string  streamStateReqBody = "<Body>\n";
	streamStateReqBody += queryContentInf;
	streamStateReqBody += "</Body>";

	return streamStateReqBody;
}

std::map<std::string, std::string>  A5StreamStateReq::parseHttpResponse(std::string & httpResponse)
{
	std::map<std::string, std::string> a5StreamStateReqMap;

	a5StreamStateReqMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::SUCCEED];;
	if (httpResponse.empty())
	{
		a5StreamStateReqMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::INPUT_XML_EMPTY];;
		return a5StreamStateReqMap;
	}

	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	if(!xmlDoc.read((void*)httpResponse.c_str(), (int)httpResponse.length(), 1))
	{
		xmlDoc.clear();
		a5StreamStateReqMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::READ_FAILED];;
		return a5StreamStateReqMap;
	}

	ZQ::common::XMLPreferenceEx* pRoot = xmlDoc.getRootPreference();
	if (!pRoot)	 
		a5StreamStateReqMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_ROOTPREFERENCE_FAILED];
	else  
	{
		ZQ::common::XMLPreferenceEx* header =  pRoot->findChild("Header");
		if (!header)
			a5StreamStateReqMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_HEADER_FAILED];	
		else
		{
			for(ZQ::common::XMLPreferenceEx* pNext = header->firstChild(); pNext != NULL; pNext = header->nextChild())
			{
				const int preferenceSize = 128;
				char preferenceName[preferenceSize] = {0};
				char preferenceText[preferenceSize] = {0};
				pNext->getPreferenceName(preferenceName, true, preferenceSize);
				pNext->getPreferenceText(preferenceText, preferenceSize);
				a5StreamStateReqMap[preferenceName] = preferenceText;
				pNext->free();
				pNext = NULL;
			}

			header->free();	header = NULL;
			ZQ::common::XMLPreferenceEx* body = pRoot->findChild("Body");
			if (!body)
				a5StreamStateReqMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_BODY_FAILED];
			else
			{
				for(ZQ::common::XMLPreferenceEx* pBodyChild = body->firstChild(); pBodyChild != NULL; pBodyChild = body->nextChild())
				{
					std::map<std::string, std::string> nodeM = pBodyChild->getProperties();
					a5StreamStateReqMap.insert(nodeM.begin(), nodeM.end());
					pBodyChild->free();	pBodyChild = NULL;
				}

				body->free(); body = NULL;
				std::map<std::string, std::string>::iterator itOpCode = a5StreamStateReqMap.find("OpCode");
				if(itOpCode != a5StreamStateReqMap.end())
				{
					std::string opCode = itOpCode->second;
					if(opCode != _a5StreamStateReq)
						a5StreamStateReqMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::OPCODE_FAILED];
				}
			}
		}

		pRoot->free(); pRoot = NULL;
	}

	xmlDoc.clear();
	return a5StreamStateReqMap;
}


A5StreamIngestCancel::A5StreamIngestCancel(std::string & contentName, ContentImpl& content, int reasonCode)
:_a5StreamIngestCancel("AO_CDN_A5_STREAM_INGEST_CANCEL"),
  _contentName(contentName), _content(content), _reasonCode(reasonCode)
{																										 
}

std::string  A5StreamIngestCancel::makeContentBody(void)
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

	std::string streamInfo = "<StreamInfo  ";   // TODO: can not know if it CancelInfo or StreamInfo
	streamInfo += contentID;
	streamInfo += volumeName;
	streamInfo += reasonCode;
	streamInfo += " >\n";
	streamInfo += "</StreamInfo>\n";

	std::string  cancelContentBody = "<Body>\n";
	cancelContentBody += streamInfo;
	cancelContentBody += "</Body>";

	return cancelContentBody;
}

std::string  A5StreamIngestCancel::makeContentHeader(void)
{
	char tempBuf[64] = {0};
	std::string senderId("GBVSS netID");
	std::string receiverId(pGBCSBaseConfig->_videoServer.VideoServerId);
	std::string transactionId(IGBCSCmd::getUUID());
	std::string version(pGBCSBaseConfig->_videoServer.VideoOnDemandVersion);
	std::string time(ZQTianShan::TimeToUTC(ZQTianShan::now(), tempBuf, sizeof(tempBuf)-2));  
	std::string opCode(_a5StreamIngestCancel);
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

}//namespace  ContentStore
}//	namespace ZQTianShan 