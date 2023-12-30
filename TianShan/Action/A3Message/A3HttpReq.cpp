#include "A3HttpReq.h"

#define XML_HEADER "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"

#define A3REQLOG if(_pLog) (*_pLog) 
#define COXLOG (*_pContOprtLog)
ZQ::common::Log nullLog;


/////////////////////////////////////////////////////
///// class ContentOprtXml 
/////////////////////////////////////////////////////
ContentOprtXml::ContentOprtXml(ZQ::common::Log* pLog)
{
	_pContOprtLog = (pLog) ? pLog : &nullLog;
}

ContentOprtXml::~ContentOprtXml()
{

}

void ContentOprtXml::setLog(ZQ::common::Log* pLog)
{
	_pContOprtLog = (pLog) ? pLog : &nullLog;
}

std::string ContentOprtXml::setAttrStr(const char* key, const char* value)
{
	if(key == NULL || *key == '\0' || value == NULL || *value == '\0')
		return "";
	
	std::string strCont;
	strCont = key;
	strCont += "=\"";
	strCont += value;
	strCont += "\" ";

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
	strCont += "\" ";

	return strCont;

}

std::string ContentOprtXml::makeTransferContent(A3Request::MessageCtx& msgCtx)
{
	std::string strCon = XML_HEADER;
	strCon += "<TransferContent \n";
	strCon += setAttrStr("providerID", msgCtx.params["providerID"].c_str());
	strCon += setAttrStr("assetID", msgCtx.params["assetID"].c_str());
	strCon += setAttrStr("captureStart", msgCtx.params["captureStart"].c_str());
	strCon += setAttrStr("captureEnd", msgCtx.params["captureEnd"].c_str());
	strCon += setAttrStr("transferBitRate", msgCtx.params["transferBitRate"].c_str());
	strCon += setAttrStr("sourceURL", msgCtx.params["sourceURL"].c_str());
	strCon += setAttrStr("sourceIP", msgCtx.params["sourceIP"].c_str());
	strCon += setAttrStr("sourceURL1", msgCtx.params["sourceURL1"].c_str());
	strCon += setAttrStr("sourceIP1", msgCtx.params["sourceIP1"].c_str());
	strCon += setAttrStr("userName", msgCtx.params["userName"].c_str());
	strCon += setAttrStr("password", msgCtx.params["password"].c_str());
	strCon += setAttrStr("volumeName", msgCtx.params["volumeName"].c_str());
	strCon += setAttrStr("responseURL", msgCtx.params["responseURL"].c_str());  
//	strCon += setAttrStr("homeID", msgCtx.params["homeID"].c_str());


	strCon += ">\r\n";

	strCon += "<ContentAsset>\r\n";
	if (msgCtx.table.size() == 0)
		strCon += "<!-- metadata -->\r\n";
	else
	{
		::std::vector< ::TianShanIce::Properties >::iterator itv = msgCtx.table.begin();
		for (TianShanIce::Properties::iterator iter = itv->begin(); iter != itv->end(); iter++)
			strCon += "<metadata name=\"" + iter->first + "\" value=\"" + iter->second + "\"/>\r\n";
	}
	strCon += "</ContentAsset>\r\n";
	strCon += "</TransferContent>";

	COXLOG(ZQ::common::Log::L_DEBUG,"TransferContent() string [%s]" ,strCon.c_str());

	return strCon;
}

std::string ContentOprtXml::makeGetTransferStatus(A3Request::MessageCtx& msgCtx)
{
	std::string strCon = XML_HEADER;

	strCon += "<GetTransferStatus \n";
	strCon += setAttrStr("providerID",msgCtx.params["providerID"].c_str());
	strCon += setAttrStr("assetID",msgCtx.params["assetID"].c_str());
	strCon += setAttrStr("volumeName",msgCtx.params["volumeName"].c_str());

	strCon += "/>";

	COXLOG(ZQ::common::Log::L_DEBUG, "GetTransferStatus() string [%s]",strCon.c_str());

	return strCon;	
}

bool ContentOprtXml::getXmlRootItem(const char* buf, size_t bufLen, ZQ::common::XMLPreferenceDocumentEx& xmlDoc, ZQ::common::XMLPreferenceEx** pRoot)
{
	try
	{
		if(xmlDoc.read((void*)buf, (int)bufLen, 1))//successful
			COXLOG(ZQ::common::Log::L_DEBUG,"ContentOprtXml::getXmlRootItem() read xml content successful");
		
		else//failed
		{
			COXLOG(ZQ::common::Log::L_ERROR,"ContentOprtXml::getXmlRootItem() read xml content failed");
			return false;
		}
	}
	catch (ZQ::common::XMLException& xmlEx) 
	{
		COXLOG(ZQ::common::Log::L_ERROR,"ContentOprtXml::getXmlRootItem() read xml content catch a exception: %s",xmlEx.getString());
		return false;
	}
	catch(...)
	{
		COXLOG(ZQ::common::Log::L_ERROR,"ContentOprtXml::getXmlRootItem() read xml content catch a exception");
		return false;
	}
	
	*pRoot = xmlDoc.getRootPreference();
	if(*pRoot == NULL)
	{
		COXLOG(ZQ::common::Log::L_ERROR,"ContentOprtXml::getXmlRootItem() getRootPreference error");
		xmlDoc.clear();
		return false;
	}

	return true;
}

bool ContentOprtXml::parseGetTransferStatus(const char* buf, size_t bufLen, A3Request::MessageCtx& msgCtx)
{
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	ZQ::common::XMLPreferenceEx* pRoot = NULL;

	if( !getXmlRootItem(buf, bufLen, xmlDoc, &pRoot))
	{
		COXLOG(ZQ::common::Log::L_ERROR,"ContentOprtXml::parseGetTransferStatus() get root item error");
		return false;
	}
	
	char chName[256] = {0};
	pRoot->name(chName, sizeof(chName));
	if(strcmp(chName,"TransferStatus") != 0)
	{
		//COXLOG(ZQ::common::Log::L_ERROR,"ContentOprtXml::parseGetTransferStatus() the root name is not 'TransferStatus',it is '%s' ",chName);
		pRoot->free();
		xmlDoc.clear();
		return false;
	}

	//get properties
	msgCtx.params.clear();
	msgCtx.params = pRoot->getProperties();

	pRoot->free();
	xmlDoc.clear();

	return true;
}

std::string ContentOprtXml::makeCancelTransfer(A3Request::MessageCtx& msgCtx)
{
	std::string strCon = XML_HEADER;

	strCon += "<CancelTransfer \n";
	strCon += setAttrStr("providerID", msgCtx.params["providerID"].c_str());
	strCon += setAttrStr("assetID", msgCtx.params["assetID"].c_str());
	strCon += setAttrStr("volumeName", msgCtx.params["volumeName"].c_str());
	strCon += setAttrStr("reasonCode", msgCtx.params["reasonCode"].c_str());

	strCon += "/>";
	
	COXLOG(ZQ::common::Log::L_DEBUG, "CancelTransfer() string [%s]",strCon.c_str());

	return strCon;
}

std::string ContentOprtXml::makeDeleteContent(A3Request::MessageCtx& msgCtx)
{
	std::string strCon = XML_HEADER;

	strCon += "<DeleteContent \n";
	strCon += setAttrStr("providerID", msgCtx.params["providerID"].c_str());
	strCon += setAttrStr("assetID", msgCtx.params["assetID"].c_str());
	strCon += setAttrStr("volumeName", msgCtx.params["volumeName"].c_str());
	strCon += setAttrStr("reasonCode", msgCtx.params["reasonCode"].c_str());

	strCon += "/>";

	COXLOG(ZQ::common::Log::L_DEBUG, "DeleteContent() string [%s]", strCon.c_str());

	return strCon;
}

std::string ContentOprtXml::makeExposeContent(A3Request::MessageCtx& msgCtx)
{
	std::string strCon = XML_HEADER;

	strCon += "<ExposeContent \n";
	strCon += setAttrStr("providerID", msgCtx.params["providerID"].c_str());
	strCon += setAttrStr("assetID", msgCtx.params["assetID"].c_str());
	strCon += setAttrStr("volumeName", msgCtx.params["volumeName"].c_str());
	strCon += setAttrStr("protocol", msgCtx.params["protocol"].c_str());
	strCon += setAttrStr("transferBitRate", msgCtx.params["transferBitRate"].c_str());

	strCon += "/>";

	COXLOG(ZQ::common::Log::L_DEBUG, "ExposeContent() string [%s]", strCon.c_str());

	return strCon;
}

bool ContentOprtXml::parseExposeContent(const char* buf, size_t bufLen, A3Request::MessageCtx& msgCtx)
{
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	ZQ::common::XMLPreferenceEx* pRoot = NULL;

	if( !getXmlRootItem(buf, bufLen, xmlDoc, &pRoot))
	{
		COXLOG(ZQ::common::Log::L_ERROR,"ContentOprtXml::parseExposeContent() get root item error");
		return false;
	}

	//get expose response property
	msgCtx.params.clear();
	msgCtx.params = pRoot->getProperties();

	pRoot->free();
	xmlDoc.clear();

	return true;
}

std::string ContentOprtXml::makeGetContentChecksum(A3Request::MessageCtx& msgCtx)
{
	std::string strCon = XML_HEADER;

	strCon += "<GetContentChecksum \n";
	strCon += setAttrStr("providerID", msgCtx.params["providerID"].c_str());
	strCon += setAttrStr("assetID", msgCtx.params["assetID"].c_str());
	strCon += setAttrStr("volumeName", msgCtx.params["volumeName"].c_str());
	strCon += setAttrStr("responseURL", msgCtx.params["responseURL"].c_str());

	strCon += "/>";

	COXLOG(ZQ::common::Log::L_DEBUG, "GetContentChecksum() string [%s]", strCon.c_str());

	return strCon;
}

std::string ContentOprtXml::makeGetContentInfo(A3Request::MessageCtx& msgCtx)
{
	std::string strCon = XML_HEADER;

	strCon += "<GetContentInfo \n";
	strCon += setAttrStr("providerID", msgCtx.params["providerID"].c_str());
	strCon += setAttrStr("assetID", msgCtx.params["assetID"].c_str());
	strCon += setAttrStr("volumeName", msgCtx.params["volumeName"].c_str());

	strCon += "/>";
	
	COXLOG(ZQ::common::Log::L_DEBUG, "GetContentInfo() string [%s]", strCon.c_str());

	return strCon;
}

bool ContentOprtXml::parseGetContentInfo(const char *buf, size_t bufLen, A3Request::MessageCtx& msgCtx)
{
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	ZQ::common::XMLPreferenceEx* pRoot = NULL;

	if( !getXmlRootItem(buf, bufLen, xmlDoc, &pRoot))
	{
		COXLOG(ZQ::common::Log::L_ERROR,"ContentOprtXml::parseGetContentInfo() get root item error");
		return false;
	}
		
	ZQ::common::XMLPreferenceEx* pNode = pRoot->firstChild("ContentInfo");
	if(pNode == NULL)
	{
		COXLOG(ZQ::common::Log::L_ERROR,"ContentOprtXml::parseGetContentInfo() get firstChild ContentInfo false");
		pRoot->free();
		xmlDoc.clear();
		return false;
	}
	msgCtx.table.clear();
	//add every contentInfo to vector
	for(; pNode != NULL; pNode = pRoot->nextChild())
	{
		std::map<std::string, std::string> nodeM = pNode->getProperties();
		msgCtx.table.push_back(nodeM);

		pNode->free();
		pNode = NULL;
	}
	pRoot->free();
	xmlDoc.clear();
	
	return true;
}

std::string ContentOprtXml::makeGetVolumeInfo(A3Request::MessageCtx& msgCtx)
{
	std::string strCon = XML_HEADER;
	strCon += "<GetVolumeInfo \n";
	strCon += setAttrStr("volumeName", msgCtx.params["volumeName"].c_str());
	strCon += "/>";

	COXLOG(ZQ::common::Log::L_DEBUG, "GetVolumeInfo() string [%s]", strCon.c_str());

	return strCon;
}

bool ContentOprtXml::parseGetVolumeInfo(const char *buf, size_t bufLen, A3Request::MessageCtx& msgCtx)
{
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	ZQ::common::XMLPreferenceEx* pRoot = NULL;

	if( !getXmlRootItem(buf, bufLen, xmlDoc, &pRoot))
	{
		COXLOG(ZQ::common::Log::L_ERROR,"ContentOprtXml::parseGetVolumeInfo() get root item error");
		return false;
	}
	
	msgCtx.params.clear();
	msgCtx.params = pRoot->getProperties();
	
	pRoot->free();
	xmlDoc.clear();

	return true;
}

bool ContentOprtXml::parseContentChecksum(const char* buf, size_t bufLen, A3Request::MessageCtx& msgCtx)
{
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	ZQ::common::XMLPreferenceEx* pRoot = NULL;

	if( !getXmlRootItem(buf, bufLen, xmlDoc, &pRoot))
	{
		COXLOG(ZQ::common::Log::L_ERROR,"ContentOprtXml::parseContentChecksum() get root item error");
		return false;
	}

	char chName[256] = {0};
	pRoot->name(chName, sizeof(chName));
	if(strcmp(chName,"ContentChecksum") != 0)
	{
		//		COXLOG(ZQ::common::Log::L_ERROR,"ContentOprtXml::parseContentChecksum() the root name is not 'ContentChecksum',it is '%s' ",chName);
		pRoot->free();
		xmlDoc.clear();
		return false;
	}
	//get contentchecksun
	msgCtx.params.clear();
	msgCtx.params = pRoot->getProperties();

	pRoot->free();
	xmlDoc.clear();

	return true;
}


/////////////////////////////////////////////////////////
/////////// class A3Request
/////////////////////////////////////////////////////////

A3Request::A3Request()
:_pLog(NULL)
{
	_http.init();
//	_http.setRecvTimeout(20);
}

A3Request::A3Request(std::string& strHost, ZQ::common::Log* pLog)
:_strHost(strHost),_pLog(pLog)
{
	size_t ns = _strHost.length();
	if(_strHost[ns-1] != '/')
		_strHost += '/';

	_http.init();
//	_http.setRecvTimeout(20);
	_http.setLog(pLog);
}

void A3Request::setHost(std::string& strHost)
{
	_strHost = strHost;
	size_t ns = _strHost.length();
	if(_strHost[ns-1] != '/')
		_strHost += '/';
}

void A3Request::setLog(ZQ::common::Log* pLog)
{
	_pLog = pLog;
	_http.setLog(pLog);
}

A3Request::~A3Request(void)
{
}

std::string A3Request::getStatusMessage() 
{
	return _http.getMsg();
}

int A3Request::sendRequest(const std::string& url, std::string& buffer) 
{
	if(_http.httpConnect(url.c_str())) 
	{
		A3REQLOG(ZQ::common::Log::L_ERROR,"sendRequest() connect failed,endpoint: %s",url.c_str());
		_http.uninit();
		return (-1);
	}

	if(_http.httpSendContent(buffer.data(), buffer.length()) || _http.httpEndSend()) 
	{
		A3REQLOG(ZQ::common::Log::L_ERROR,"sendRequest() send errorcode: %d",_http.getErrorcode());
		_http.uninit();
		return (-1);
	}

	if(_http.httpBeginRecv()) 
	{
		A3REQLOG(ZQ::common::Log::L_ERROR,"sendRequest() receive errorstring:%s",_http.getErrorstr());
		_http.uninit();
		return (-1);
	}

	while(!_http.isEOF()) 
	{
		if(_http.httpContinueRecv()) 
		{
			A3REQLOG(ZQ::common::Log::L_ERROR,"sendRequest() continue receive errorstring:%s",_http.getErrorstr());
			_http.uninit();	
			return (-1);
		}
	}

	_http.getContent(buffer);

	if (_pLog)
		_pLog->hexDump(ZQ::common::Log::L_DEBUG, buffer.c_str(), buffer.length(), NULL);

	_http.uninit();
	
	return _http.getStatusCode();
}

int A3Request::request(const A3MsgType type, MessageCtx& msgCtx, int timeout)
{
	if(_strHost.length() == 0)
	{
		A3REQLOG(ZQ::common::Log::L_ERROR,"A3Request::request() not set URL");
		return -1;
	}

	if(msgCtx.params.size() == 0)
	{
		A3REQLOG(ZQ::common::Log::L_ERROR,"A3Request::request() parameter MessageCtx is NULL");
		return -1;

	}
	if(timeout > 0)
	{
		if(timeout/1000 > 0)
			timeout = timeout/1000;
		else
			timeout = -(timeout*1000);

		_http.setConnectTimeout(timeout);
		_http.setSendTimeout(timeout);
		_http.setRecvTimeout(timeout);		
	}
	else
	{
		_http.setConnectTimeout(0);
		_http.setSendTimeout(0);
		_http.setRecvTimeout(0);
	}


	int nstate = -1;
	switch(type)
	{
	case A3_TransferContent:
		nstate = transferContent(msgCtx);
		break;
	case A3_GetTransferStatus:
		nstate = getTransferStatus(msgCtx);
		break;
	case A3_CancelTransfer:
		nstate = cancelTransfer(msgCtx);
		break;
	case A3_ExposeContent:
		nstate = exposeContent(msgCtx);
		break;
	case A3_GetContentChecksum:
		nstate = getContentChecksum(msgCtx);
		break;
	case A3_GetContentInfo:
		nstate = getContentInfo(msgCtx);
		break;
	case A3_DeleteContent:
		nstate = deleteContent(msgCtx);
		break;
	case A3_GetVolumeInfo:
		nstate = getVolumeInfo(msgCtx);
		break;
	default:
		A3REQLOG(ZQ::common::Log::L_ERROR,"A3Request::request() unknown the type %d",type);
		break;
	};

	msgCtx.errorCode = nstate;

	return nstate;
}

int A3Request::transferContent(MessageCtx& msgCtx) 
{
	ContentOprtXml xmlOp(_pLog);
	std::string str = xmlOp.makeTransferContent(msgCtx);
	std::string url = _strHost + "TransferContent";

	return sendRequest(url, str);
}

int A3Request::getTransferStatus(MessageCtx& msgCtx) 
{
	ContentOprtXml xmlOp(_pLog);
	std::string str = xmlOp.makeGetTransferStatus(msgCtx);
	std::string url = _strHost + "GetTransferStatus";
	
	int code = sendRequest(url, str);
	if(code == 200) 
	{
		if(!xmlOp.parseGetTransferStatus(str.c_str(), str.length(),msgCtx)) 	
			return (-1);
	
	}
	
	return code;
}

int A3Request::cancelTransfer(MessageCtx& msgCtx) 
{
	ContentOprtXml xmlOp(_pLog);
	std::string str = xmlOp.makeCancelTransfer(msgCtx);
	std::string url = _strHost + "CancelTransfer";

	return sendRequest(url, str);
}

int A3Request::exposeContent(MessageCtx& msgCtx) 
{
	ContentOprtXml xmlOp(_pLog);
	std::string str = xmlOp.makeExposeContent(msgCtx);
	std::string url = _strHost + "ExposeContent";
	
	int code = sendRequest(url, str);
	if(code == 200) 
	{
		if(!xmlOp.parseExposeContent(str.c_str(), str.length(), msgCtx)) 
			return (-1);
	}
	return code;
}

int A3Request::getContentChecksum(MessageCtx& msgCtx) 
{
	ContentOprtXml xmlOp(_pLog);
	std::string str = xmlOp.makeGetContentChecksum(msgCtx);
	std::string url = _strHost + "GetContentChecksum";

	return sendRequest(url, str);
}

int A3Request::getContentInfo(MessageCtx& msgCtx) 
{
	ContentOprtXml xmlOp(_pLog);
	std::string str = xmlOp.makeGetContentInfo(msgCtx);
	std::string url = _strHost + "GetContentInfo";
	
	int code = sendRequest(url, str);
	if(code == 200) 
	{
		if(!xmlOp.parseGetContentInfo(str.c_str(), str.length(), msgCtx)) 
			return (-1);
	}
	return code;
}

int A3Request::deleteContent(MessageCtx& msgCtx) 
{
	ContentOprtXml xmlOp(_pLog);
	std::string str = xmlOp.makeDeleteContent(msgCtx);
	std::string url = _strHost + "DeleteContent";

	return sendRequest(url, str);
}

int A3Request::getVolumeInfo(MessageCtx& msgCtx) 
{
	ContentOprtXml xmlOp(_pLog);
	std::string str = xmlOp.makeGetVolumeInfo(msgCtx);
	std::string url = _strHost + "GetVolumeInfo";

	int code = sendRequest(url, str);
	if(code == 200) 
	{
		if(!xmlOp.parseGetVolumeInfo(str.c_str(), str.length(), msgCtx)) 
			return (-1);	

	}
	return code;
}
