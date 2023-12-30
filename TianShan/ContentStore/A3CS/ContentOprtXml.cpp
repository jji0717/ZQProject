// ContentOprtXml.cpp: implementation of the ContentOprtXml class.
//
//////////////////////////////////////////////////////////////////////

#include "ContentOprtXml.h"
#include "XMLPreferenceEx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define XML_HEADER "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
using namespace std;
using namespace ZQ::common;

ZQ::common::Log nullLogger;

#define COXLOG (*_pContOprtLog)

ContentOprtXml::ContentOprtXml(ZQ::common::Log* pLog)
{
	_pContOprtLog = (pLog) ? pLog : &nullLogger;
}

ContentOprtXml::~ContentOprtXml()
{

}

std::string ContentOprtXml::setAttrStr(const char* key, const char* value)
{
	if(key == NULL || *key == '\0' || value == NULL || *value == '\0')
		return "";
	
	std::string strCont;
	strCont = key;
	strCont += "=\"";
	strCont += value;
	strCont += "\"";
	strCont += " ";
	return  strCont;
}

std::string ContentOprtXml::setAttrStr(const char* key, int value)
{
	if(key == NULL || key == "" || value == 0)
		return "";

	char chval[10] = {0};
	std::string strCont;
	strCont = key;
	strCont += "=\"";
	sprintf(chval,"%d",value);
	strCont += chval;
	strCont += "\"";
	strCont += " ";
	return strCont;

}

std::string ContentOprtXml::makeTransferContent(const TransferInfo& tranCont)
{
	std::string strCon = XML_HEADER;
	strCon += "<TransferContent \n";
	strCon += setAttrStr("providerID",tranCont.providerID.c_str());
	strCon += setAttrStr("assetID",tranCont.assetID.c_str());
	strCon += setAttrStr("captureStart",tranCont.captureStart.c_str());
	strCon += setAttrStr("captureEnd",tranCont.captureEnd.c_str());
	strCon += setAttrStr("transferBitRate",tranCont.transferBitRate);
	strCon += setAttrStr("sourceURL",tranCont.sourceURL.c_str());
	strCon += setAttrStr("sourceIP",tranCont.sourceIP.c_str());
	strCon += setAttrStr("sourceURL1",tranCont.sourceURL1.c_str());
	strCon += setAttrStr("sourceIP1",tranCont.sourceIP1.c_str());
	strCon += setAttrStr("userName",tranCont.userName.c_str());
	strCon += setAttrStr("password",tranCont.password.c_str());
	strCon += setAttrStr("volumeName",tranCont.volumeName.c_str());
	strCon += setAttrStr("responseURL",tranCont.responseURL.c_str());
	//strCon += setAttrStr("homeID", tranCont.homeId.c_str());
	strCon += setAttrStr("homeID", "0");

	strCon += ">\r\n";

	strCon += "<ContentAsset>\r\n";
	if (tranCont.element.empty())
		strCon += "<!-- metadata -->\r\n";
	else
	{
		for (TransferInfo::ContentAssets::const_iterator iter = tranCont.element.begin(); iter != tranCont.element.end(); iter++)
			strCon += "<metadata name=\"" + iter->first + "\" value=\"" + iter->second + "\"/>\r\n";
	}
	strCon += "</ContentAsset>\r\n";
	strCon += "</TransferContent>";

	COXLOG(Log::L_DEBUG, strCon.c_str());

	return strCon;
}

std::string ContentOprtXml::makeGetVolumeInfo(struct VolumeInfo& voluInfo)
{
	std::string strCon = XML_HEADER;
	strCon += "<GetVolumeInfo \n";
	strCon += setAttrStr("volumeName",voluInfo.volumeName.c_str());
	strCon += "/>";

	COXLOG(Log::L_DEBUG, strCon.c_str());

	return strCon;
}

bool ContentOprtXml::parseGetVolumeInfo(struct VolumeInfo& voluInfo, const char *buf, size_t bufLen)
{
	XMLPreferenceDocumentEx xmlDoc;
	
	try
	{
		if(xmlDoc.read((void*)buf, (int)bufLen, 1))//successful
			COXLOG(Log::L_DEBUG,"ContentOprtXml::parseGetVolumeInfo() read xml content successful");
		
		else//failed
		{
			COXLOG(Log::L_ERROR,"ContentOprtXml::parseGetVolumeInfo() read xml content failed");
			return false;
		}
	}
	catch (ZQ::common::XMLException& xmlEx) 
	{
		COXLOG(Log::L_ERROR,"ContentOprtXml::parseGetVolumeInfo() read xml content catch a exception: %s",xmlEx.getString());
		return false;
	}
	catch(...)
	{
		COXLOG(Log::L_ERROR,"ContentOprtXml::parseGetVolumeInfo() read xml content catch a exception");
		return false;
	}
	
	XMLPreferenceEx* pRoot = xmlDoc.getRootPreference();
	if(pRoot == NULL)
	{
		COXLOG(Log::L_ERROR,"ContentOprtXml::parseGetVolumeInfo() getRootPreference error");
		xmlDoc.clear();
		return false;
	}
	
	std::map<std::string, std::string> nodeM = pRoot->getProperties();
	voluInfo.volumeName =  nodeM["volumeName"];
	voluInfo.state = VolumeState(atoi(nodeM["state"].c_str()));
	voluInfo.volumeSize = atoi(nodeM["volumeSize"].c_str());
	voluInfo.freeSize = atoi(nodeM["freeSize"].c_str());

	pRoot->free();
	xmlDoc.clear();

	return true;
}

std::string ContentOprtXml::makeCancelTransfer(const DeleteCancelContent& cancelTrans)
{
	std::string strCon = XML_HEADER;

	strCon += "<CancelTransfer \n";
	strCon += setAttrStr("providerID",cancelTrans.providerID.c_str());
	strCon += setAttrStr("assetID",cancelTrans.assetID.c_str());
	strCon += setAttrStr("volumeName",cancelTrans.volumeName.c_str());
	strCon += setAttrStr("reasonCode", cancelTrans.reasonCode);

	strCon += "/>";
	
	COXLOG(Log::L_DEBUG, strCon.c_str());

	return strCon;
}

std::string ContentOprtXml::makeDeleteContent(const DeleteCancelContent& delContent)
{
	std::string strCon = XML_HEADER;

	strCon += "<DeleteContent \n";
	strCon += setAttrStr("providerID",delContent.providerID.c_str());
	strCon += setAttrStr("assetID",delContent.assetID.c_str());
	strCon += setAttrStr("volumeName",delContent.volumeName.c_str());
	strCon += setAttrStr("reasonCode", delContent.reasonCode);

	strCon += "/>";

	COXLOG(Log::L_DEBUG, strCon.c_str());

	return strCon;
}

std::string ContentOprtXml::makeGetContentInfo(const ContentInfo& contInfo)
{
	std::string strCon = XML_HEADER;

	strCon += "<GetContentInfo \n";
	strCon += setAttrStr("providerID",contInfo.providerID.c_str());
	strCon += setAttrStr("assetID",contInfo.assetID.c_str());
	strCon += setAttrStr("volumeName",contInfo.volumeName.c_str());

	strCon += "/>";
	
	COXLOG(Log::L_DEBUG, strCon.c_str());

	return strCon;
}

bool ContentOprtXml::parseGetContentInfo(std::vector<struct ContentInfo>& contInfoVector, const char *buf, size_t bufLen)
{
	XMLPreferenceDocumentEx xmlDoc;
	
	try
	{
		if(xmlDoc.read((void*)buf, (int)bufLen, 1))//successful
			COXLOG(Log::L_DEBUG,"ContentOprtXml::parseGetContentInfo() read xml content successful");
		
		else//failed
		{
			COXLOG(Log::L_ERROR,"ContentOprtXml::parseGetContentInfo() read xml content failed");
			return false;
		}
	}
	catch (ZQ::common::XMLException& xmlEx) 
	{
		COXLOG(Log::L_ERROR,"ContentOprtXml::parseGetContentInfo() read xml content catch a exception: %s",xmlEx.getString());
		return false;
	}
	catch(...)
	{
		COXLOG(Log::L_ERROR,"ContentOprtXml::parseGetContentInfo() read xml content catch a exception");
		return false;
	}

	XMLPreferenceEx* pRoot = xmlDoc.getRootPreference();
	if(pRoot == NULL)
	{
		COXLOG(Log::L_ERROR,"ContentOprtXml::parseGetContentInfo() getRootPreference error");
		xmlDoc.clear();
		return false;
	}
		
	XMLPreferenceEx* pNode = pRoot->firstChild("ContentInfo");
	if(pNode == NULL)
	{
		COXLOG(Log::L_ERROR,"ContentOprtXml::parseGetContentInfo() get firstChild ContentInfo false");
		pRoot->free();
		xmlDoc.clear();
		return false;
	}
	contInfoVector.clear();
	//add every contentInfo to vector
	for(; pNode; pNode = pRoot->nextChild())
	{
		struct ContentInfo contInfo;
		std::map<std::string, std::string> nodeM = pNode->getProperties();
		contInfo.providerID = nodeM["providerID"];
		contInfo.assetID = nodeM["assetID"];
		contInfo.volumeName = nodeM["volumeName"];
		contInfo.contentSize = _atoi64(nodeM["contentSize"].c_str());
		contInfo.supportFileSize = _atoi64(nodeM["supportFileSize"].c_str());
		contInfo.contentState = nodeM["contentState"].c_str();
		contInfo.createDate = nodeM["createDate"].c_str();
		contInfo.md5Checksum = nodeM["md5Checksum"].c_str();
		contInfo.md5DateTime = nodeM["md5DateTime"].c_str();

		contInfoVector.push_back(contInfo);

		pNode->free();
		pNode = NULL;
	}
	pRoot->free();
	xmlDoc.clear();
	
	return true;
}

std::string ContentOprtXml::makeGetTransferStatus(const TransferStatus& transStatus)
{
	std::string strCon = XML_HEADER;

	strCon += "<GetTransferStatus \n";
	strCon += setAttrStr("providerID",transStatus.providerID.c_str());
	strCon += setAttrStr("assetID",transStatus.assetID.c_str());
	strCon += setAttrStr("volumeName",transStatus.volumeName.c_str());

	strCon += "/>";

	COXLOG(Log::L_DEBUG, strCon.c_str());

	return strCon;	
}

bool ContentOprtXml::parseGetTransferStatus(TransferStatus& transStatus, const char* buf, size_t bufLen)
{
	XMLPreferenceDocumentEx xmlDoc;
	
	try
	{
		if(xmlDoc.read((void*)buf, (int)bufLen, 1))//successful
			COXLOG(Log::L_DEBUG,"ContentOprtXml::parseGetTransferStatus() read xml content successful");
		
		else//failed
		{
			COXLOG(Log::L_ERROR,"ContentOprtXml::parseGetTransferStatus() read xml content failed");
			return false;
		}
	}
	catch (ZQ::common::XMLException& xmlEx) 
	{
		COXLOG(Log::L_ERROR,"ContentOprtXml::parseGetTransferStatus() read xml content catch a exception: %s",xmlEx.getString());
		return false;
	}
	catch(...)
	{
		COXLOG(Log::L_ERROR,"ContentOprtXml::parseGetTransferStatus() read xml content catch a exception");
		return false;
	}
	
	XMLPreferenceEx* pRoot = xmlDoc.getRootPreference();
	if(pRoot == NULL)
	{
		COXLOG(Log::L_ERROR,"ContentOprtXml::parseGetTransferStatus() getRootPreference error");
		xmlDoc.clear();
		return false;
	}

	char chName[256] = {0};
	pRoot->name(chName, sizeof(chName));
	if(strcmp(chName,"TransferStatus") != 0)
	{
		//		COXLOG(Log::L_ERROR,"ContentOprtXml::parseGetTransferStatus() the root name is not 'TransferStatus',it is '%s' ",chName);
		pRoot->free();
		xmlDoc.clear();
		return false;
	}

	//get properties
	std::map<std::string, std::string> nodeM = pRoot->getProperties();
	transStatus.providerID = nodeM["providerID"];
	transStatus.assetID = nodeM["assetID"];
	transStatus.volumeName = nodeM["volumeName"];
	transStatus.state = nodeM["state"];
	transStatus.percentComplete = atoi(nodeM["percentComplete"].c_str());
	transStatus.contentSize = atoi(nodeM["contentSize"].c_str());
	transStatus.supportFileSize = atoi(nodeM["supportFileSize"].c_str());
	transStatus.md5Checksum = nodeM["md5Checksum"].c_str();
	transStatus.md5DateTime = nodeM["md5DateTime"].c_str();
	transStatus.bitrate = atoi(nodeM["bitrate"].c_str());

	pRoot->free();
	xmlDoc.clear();

	return true;
}

std::string ContentOprtXml::makeExposeContent(const ExposeContentInfo& exposecont)
{
	std::string strCon = XML_HEADER;

	strCon += "<ExposeContent \n";
	strCon += setAttrStr("providerID",exposecont.providerID.c_str());
	strCon += setAttrStr("assetID",exposecont.assetID.c_str());
	strCon += setAttrStr("volumeName",exposecont.volumeName.c_str());
	strCon += setAttrStr("protocol", exposecont.protocol.c_str());
	strCon += setAttrStr("transferBitRate", exposecont.transferBitRate.c_str());

	strCon += "/>";

	COXLOG(Log::L_DEBUG, strCon.c_str());

	return strCon;
}

bool ContentOprtXml::parseExposeContent(struct ExposeResponse& exposeResp, const char* buf, size_t bufLen)
{
	XMLPreferenceDocumentEx xmlDoc;
	
	try
	{
		if(xmlDoc.read((void*)buf, (int)bufLen, 1))//successful
			COXLOG(Log::L_DEBUG,"ContentOprtXml::parseExposeContent() read xml content successful");
		
		else//failed
		{
			COXLOG(Log::L_ERROR,"ContentOprtXml::parseExposeContent() read xml content failed");
			return false;
		}
	}
	catch (ZQ::common::XMLException& xmlEx) 
	{
		COXLOG(Log::L_ERROR,"ContentOprtXml::parseExposeContent() read xml content catch a exception: %s",xmlEx.getString());
		return false;
	}
	catch(...)
	{
		COXLOG(Log::L_ERROR,"ContentOprtXml::parseExposeContent() read xml content catch a exception");
		return false;
	}
	
	XMLPreferenceEx* pRoot = xmlDoc.getRootPreference();
	if(pRoot == NULL)
	{
		COXLOG(Log::L_ERROR,"ContentOprtXml::parseExposeContent() getRootPreference error");
		xmlDoc.clear();
		return false;
	}

	//get expose response property
	std::map<std::string, std::string> nodeM = pRoot->getProperties();
	exposeResp.providerID = nodeM["providerID"];
	exposeResp.assetID = nodeM["assetID"];
	exposeResp.URL = nodeM["URL"];
	exposeResp.userName = nodeM["userName"];
	exposeResp.password = nodeM["password"];
	exposeResp.ttl = atoi(nodeM["ttl"].c_str());
	exposeResp.transferBitRate = atoi(nodeM["transferBitRate"].c_str());

	pRoot->free();
	xmlDoc.clear();

	return true;
}

std::string ContentOprtXml::makeGetContentChecksum(const ContentChecksumInfo& getcontCheck)
{
	std::string strCon = XML_HEADER;

	strCon += "<GetContentChecksum \n";
	strCon += setAttrStr("providerID",getcontCheck.providerID.c_str());
	strCon += setAttrStr("assetID",getcontCheck.assetID.c_str());
	strCon += setAttrStr("volumeName",getcontCheck.volumeName.c_str());
	strCon += setAttrStr("responseURL",getcontCheck.responseURL.c_str());

	strCon += "/>";

	COXLOG(Log::L_DEBUG, strCon.c_str());

	return strCon;
}

bool ContentOprtXml::parseContentChecksum(struct ContentChecksum& contCheck, const char* buf, size_t bufLen)
{
	XMLPreferenceDocumentEx xmlDoc;

	try
	{
		if(xmlDoc.read((void*)buf, (int)bufLen, 1))//successful
		{
			COXLOG(Log::L_DEBUG,"ContentOprtXml::parseContentChecksum() read xml content successful");
		}
		else//failed
		{
			COXLOG(Log::L_ERROR,"ContentOprtXml::parseContentChecksum() read xml content failed");
			return false;
		}
	}
	catch (ZQ::common::XMLException& xmlEx) 
	{
		COXLOG(Log::L_ERROR,"ContentOprtXml::parseContentChecksum() read xml content catch a exception: %s",xmlEx.getString());
		return false;
	}
	catch(...)
	{
		COXLOG(Log::L_ERROR,"ContentOprtXml::parseContentChecksum() read xml content catch a exception");
		return false;
	}

	XMLPreferenceEx* pRoot = xmlDoc.getRootPreference();
	if(pRoot == NULL)
	{
		COXLOG(Log::L_ERROR,"ContentOprtXml::parseContentChecksum() getRootPreference error");
		xmlDoc.clear();
		return false;
	}
	char chName[256] = {0};
	pRoot->name(chName, sizeof(chName));
	if(strcmp(chName,"ContentChecksum") != 0)
	{
		//		COXLOG(Log::L_ERROR,"ContentOprtXml::parseContentChecksum() the root name is not 'ContentChecksum',it is '%s' ",chName);
		pRoot->free();
		xmlDoc.clear();
		return false;
	}
	//get contentchecksun
	std::map<std::string, std::string> nodeM = pRoot->getProperties();

	contCheck.providerID = nodeM["providerID"];
	contCheck.assetID = nodeM["assetID"];
	contCheck.volumeName = nodeM["volumeName"];
	contCheck.md5Checksum = nodeM["md5Checksum"];
	contCheck.md5DateTime = nodeM["md5DateTime"];
	contCheck.resultCode = atoi(nodeM["resultCode"].c_str());	

	pRoot->free();
	xmlDoc.clear();

	return true;
}