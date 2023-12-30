#include "A3HttpReq.h"
#include "NGODStorePortal.h"

#define DEFAULT_TIMEOUT_LONG      (60*30) // half an hour
#define DEFAULT_TIMEOUT_SHORT     (10)    // 10sec
#define DEFAULT_TIMEOUT_CONNECT   DEFAULT_TIMEOUT_SHORT
#define DEFAULT_TIMEOUT_SEND      DEFAULT_TIMEOUT_SHORT

// #define XML_NEWLINE "\n"
#define XML_NEWLINE
#define XML_HEADER "<?xml version=\"1.0\" encoding=\"utf-8\" ?>" XML_NEWLINE

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
	strCon += "<TransferContent " XML_NEWLINE;
	strCon += setAttrStr("providerID", msgCtx.params["providerID"].c_str());
	strCon += setAttrStr("assetID", msgCtx.params["assetID"].c_str());

	strCon += setAttrStr("transferBitRate", msgCtx.params["transferBitRate"].c_str());
	strCon += setAttrStr("sourceURL",       msgCtx.params["sourceURL"].c_str());
	strCon += setAttrStr("sourceIP",        msgCtx.params["sourceIP"].c_str());
	strCon += setAttrStr("sourceURL1",      msgCtx.params["sourceURL1"].c_str());
	strCon += setAttrStr("sourceIP1",       msgCtx.params["sourceIP1"].c_str());
	strCon += setAttrStr("userName",        msgCtx.params["userName"].c_str());
	strCon += setAttrStr("password",        msgCtx.params["password"].c_str());
	strCon += setAttrStr("volumeName",      msgCtx.params["volumeName"].c_str());
	strCon += setAttrStr("responseURL",     msgCtx.params["responseURL"].c_str());  

	if (msgCtx.params.end() != msgCtx.params.find("homeID"))
		strCon += setAttrStr("homeID",      msgCtx.params["homeID"].c_str());

	if (msgCtx.params.end() != msgCtx.params.find("captureStart"))
		strCon += setAttrStr("captureStart", msgCtx.params["captureStart"].c_str());
	if (msgCtx.params.end() != msgCtx.params.find("captureEnd"))
		strCon += setAttrStr("captureEnd",   msgCtx.params["captureEnd"].c_str());

	strCon += ">" XML_NEWLINE;

	strCon += "<ContentAsset>" XML_NEWLINE;
	if (msgCtx.table.size() == 0)
		strCon += "<!-- metadata -->" XML_NEWLINE;
	else
	{
		::std::vector< ::TianShanIce::Properties >::iterator itv = msgCtx.table.begin();
		for (TianShanIce::Properties::iterator iter = itv->begin(); iter != itv->end(); iter++)
			strCon += "<metadata name=\"" + iter->first + "\" value=\"" + iter->second + "\"/>" XML_NEWLINE;
	}
	strCon += "</ContentAsset>" XML_NEWLINE;
	strCon += "</TransferContent>";

	COXLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(A3HttpReq, "TransferContent() req: %s"), strCon.c_str());

	return strCon;
}

std::string ContentOprtXml::makeGetTransferStatus(A3Request::MessageCtx& msgCtx)
{
	std::string strCon = XML_HEADER;

	strCon += "<GetTransferStatus " XML_NEWLINE;
	strCon += setAttrStr("providerID",msgCtx.params["providerID"].c_str());
	strCon += setAttrStr("assetID",msgCtx.params["assetID"].c_str());
	strCon += setAttrStr("volumeName",msgCtx.params["volumeName"].c_str());

	strCon += "/>";

	COXLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(A3HttpReq, "GetTransferStatus() req: %s"), strCon.c_str());

	return strCon;	
}

bool ContentOprtXml::getXmlRootItem(const char* buf, size_t bufLen, ZQ::common::XMLPreferenceDocumentEx& xmlDoc, ZQ::common::XMLPreferenceEx** pRoot)
{
	try
	{
		*pRoot = NULL;
		if(xmlDoc.read((void*)buf, (int)bufLen, 1))//successful
			COXLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(A3HttpReq, "getXmlRootItem() root element found"));
		
		else//failed
		{
			COXLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "getXmlRootItem() failed to read root element"));
			return false;
		}
	
		*pRoot = xmlDoc.getRootPreference();
	}
	catch (ZQ::common::XMLException& xmlEx) 
	{
		COXLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "getXmlRootItem() read xml content caught exception[%s]"), xmlEx.getString());
		return false;
	}
	catch(...)
	{
		COXLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "getXmlRootItem() read xml content caught exception"));
		return false;
	}
	
	if(*pRoot == NULL)
	{
		COXLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "getXmlRootItem() getRootPreference error"));
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
		COXLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "parseGetTransferStatus() failed to read root element"));
		return false;
	}
	
	char chName[256] = {0};
	pRoot->name(chName, sizeof(chName));
	if(strcmp(chName,"TransferStatus") != 0)
	{
		//COXLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "parseGetTransferStatus() the root name is not 'TransferStatus', it is '%s' "),chName);
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

	strCon += "<CancelTransfer " XML_NEWLINE;
	strCon += setAttrStr("providerID", msgCtx.params["providerID"].c_str());
	strCon += setAttrStr("assetID", msgCtx.params["assetID"].c_str());
	strCon += setAttrStr("volumeName", msgCtx.params["volumeName"].c_str());
	strCon += setAttrStr("reasonCode", msgCtx.params["reasonCode"].c_str());

	strCon += "/>";
	
	COXLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(A3HttpReq, "CancelTransfer() req: %s"), strCon.c_str());

	return strCon;
}

std::string ContentOprtXml::makeDeleteContent(A3Request::MessageCtx& msgCtx)
{
	std::string strCon = XML_HEADER;

	strCon += "<DeleteContent " XML_NEWLINE;
	strCon += setAttrStr("providerID", msgCtx.params["providerID"].c_str());
	strCon += setAttrStr("assetID", msgCtx.params["assetID"].c_str());
	strCon += setAttrStr("volumeName", msgCtx.params["volumeName"].c_str());
	strCon += setAttrStr("reasonCode", msgCtx.params["reasonCode"].c_str());

	strCon += "/>";

	COXLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(A3HttpReq, "DeleteContent() req: %s"), strCon.c_str());

	return strCon;
}

std::string ContentOprtXml::makeExposeContent(A3Request::MessageCtx& msgCtx)
{
	std::string strCon = XML_HEADER;

	strCon += "<ExposeContent " XML_NEWLINE;
	strCon += setAttrStr("providerID", msgCtx.params["providerID"].c_str());
	strCon += setAttrStr("assetID", msgCtx.params["assetID"].c_str());
	strCon += setAttrStr("volumeName", msgCtx.params["volumeName"].c_str());
	strCon += setAttrStr("protocol", msgCtx.params["protocol"].c_str());
	strCon += setAttrStr("transferBitRate", msgCtx.params["transferBitRate"].c_str());

	strCon += "/>";

	COXLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(A3HttpReq, "ExposeContent() req: %s"), strCon.c_str());

	return strCon;
}

bool ContentOprtXml::parseExposeContent(const char* buf, size_t bufLen, A3Request::MessageCtx& msgCtx)
{
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	ZQ::common::XMLPreferenceEx* pRoot = NULL;

	if( !getXmlRootItem(buf, bufLen, xmlDoc, &pRoot))
	{
		COXLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "parseExposeContent() failed to read root element"));
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

	strCon += "<GetContentChecksum " XML_NEWLINE;
	strCon += setAttrStr("providerID", msgCtx.params["providerID"].c_str());
	strCon += setAttrStr("assetID", msgCtx.params["assetID"].c_str());
	strCon += setAttrStr("volumeName", msgCtx.params["volumeName"].c_str());
	strCon += setAttrStr("responseURL", msgCtx.params["responseURL"].c_str());

	strCon += "/>";

	COXLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(A3HttpReq, "GetContentChecksum() req: %s"), strCon.c_str());

	return strCon;
}

std::string ContentOprtXml::makeGetContentInfo(A3Request::MessageCtx& msgCtx)
{
	std::string strCon = XML_HEADER;

	strCon += "<GetContentInfo " XML_NEWLINE;
	strCon += setAttrStr("providerID", msgCtx.params["providerID"].c_str());
	strCon += setAttrStr("assetID", msgCtx.params["assetID"].c_str());
	strCon += setAttrStr("volumeName", msgCtx.params["volumeName"].c_str());

	strCon += "/>";
	
	COXLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(A3HttpReq, "GetContentInfo() req: %s"), strCon.c_str());

	return strCon;
}

bool ContentOprtXml::parseGetContentInfo(const char *buf, size_t bufLen, A3Request::MessageCtx& msgCtx)
{
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	ZQ::common::XMLPreferenceEx* pRoot = NULL;

	if( !getXmlRootItem(buf, bufLen, xmlDoc, &pRoot))
	{
		COXLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "parseGetContentInfo() failed to read root element"));
		return false;
	}
		
	ZQ::common::XMLPreferenceEx* pNode = pRoot->firstChild("ContentInfo");
	if(pNode == NULL)
	{
		COXLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "parseGetContentInfo() failed to find child elem[ContentInfo]"));
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
	strCon += "<GetVolumeInfo " XML_NEWLINE;
	strCon += setAttrStr("volumeName", msgCtx.params["volumeName"].c_str());
	strCon += "/>";

	COXLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(A3HttpReq, "GetVolumeInfo() request: %s"), strCon.c_str());

	return strCon;
}

bool ContentOprtXml::parseGetVolumeInfo(const char *buf, size_t bufLen, A3Request::MessageCtx& msgCtx)
{
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	ZQ::common::XMLPreferenceEx* pRoot = NULL;

	if (!getXmlRootItem(buf, bufLen, xmlDoc, &pRoot) || !pRoot)
	{
		COXLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "parseGetVolumeInfo() failed to read root element"));
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
		COXLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "parseContentChecksum() failed to read root element"));
		return false;
	}

	char chName[256] = {0};
	pRoot->name(chName, sizeof(chName));
	if(strcmp(chName,"ContentChecksum") != 0)
	{
		//		COXLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "parseContentChecksum() the root name is not 'ContentChecksum',it is '%s' "), chName);
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

A3Request::A3Request(ZQTianShan::NGOD_CS::NGODStorePortal& csp) // const std::string& strHost, ZQ::common::Log* pLog, const std::string& str2ndHost)
: _pLog(&csp._store._log)
{
	setHost(csp._defaultA3Url, csp._2ndA3Url);

	_http.init();

	_http.setConnectTimeout(DEFAULT_TIMEOUT_CONNECT);
	_http.setSendTimeout(DEFAULT_TIMEOUT_SEND);
	_http.setRecvTimeout(DEFAULT_TIMEOUT_SHORT);

	_http.setLog(&csp._store._log);
}

void A3Request::setHost(const std::string& strHost, const std::string& str2ndHost)
{
	_strHost = strHost;
	_str2ndHost = str2ndHost;
	if(!_strHost.empty() && _strHost[_strHost.length() -1] != '/')
		_strHost += '/';

	if(!_str2ndHost.empty() && _str2ndHost[_str2ndHost.length() -1] != '/')
		_str2ndHost += '/';
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

int A3Request::sendRequest(const char* uri, std::string& buffer, bool longRequest)
{
	std::string url = _strHost + (uri ? uri :""); 
	
	_http.setRecvTimeout(longRequest ? DEFAULT_TIMEOUT_LONG : DEFAULT_TIMEOUT_SHORT);

	if (0 != _http.httpConnect(url.c_str()))
	{
		A3REQLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "sendRequest() connect failed, endpoint: %s"),url.c_str());
		_http.uninit();

		if (_str2ndHost.length() <3)
			return (-1);

		// enh#17598 - For CacheServer's ContentEdge/ secondary ContentEdge, NSS to enable dual interface to A3 server
		// retry at the secondary A3 interface
		url = _str2ndHost + (uri ? uri :""); 
		_http.init();
		if (0 != _http.httpConnect(url.c_str())) 
		{
			A3REQLOG(ZQ::common::Log::L_WARNING, "sendRequest() failed at connecting secondary endpoint[%s]", url.c_str());
			_http.uninit();
			return (-1);
		}
	}

	if(_http.httpSendContent(buffer.data(), buffer.length()) || _http.httpEndSend()) 
	{
		A3REQLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "sendRequest() send errorcode: %d"),_http.getErrorcode());
		_http.uninit();
		return (-1);
	}

	if(_http.httpBeginRecv()) 
	{
		A3REQLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "sendRequest() receive error: %s"), _http.getErrorstr());
		_http.uninit();
		return (-1);
	}

	while(!_http.isEOF()) 
	{
		if(_http.httpContinueRecv()) 
		{
			A3REQLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "sendRequest() continue receive error: %s"), _http.getErrorstr());
			_http.uninit();	
			return (-1);
		}
	}

	_http.getContent(buffer);

	if (_pLog)
		_pLog->hexDump(ZQ::common::Log::L_DEBUG, buffer.c_str(), (int)buffer.length(), "Content-Body received: ", true);

	_http.uninit();
	
	return _http.getStatusCode();
}

int A3Request::request(const A3MsgType type, MessageCtx& msgCtx, int timeout)
{
	if(_strHost.length() == 0)
	{
		A3REQLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "request() no URL"));
		return -1;
	}

	if(msgCtx.params.size() == 0)
	{
		A3REQLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3HttpReq, "request() NULL MessageCtx given"));
		return -1;

	}

	if (timeout > 500) // timeout minimal 1sec
		timeout = (timeout +500) /1000;
	else timeout = DEFAULT_TIMEOUT_SHORT;

	_http.setConnectTimeout(timeout);
	_http.setSendTimeout(timeout);
	_http.setRecvTimeout(timeout);		

	_http.setHeader("Content-Type", "text/xml; charset=utf-8");
	_http.setHeader("User-Agent",   "TianShan NSS/1.15"); // TODO: should read version from the ZQResource.h

	int nstate = -1;
	const char* reqName = "UNKNOWN";
	switch(type)
	{
	case A3_TransferContent:
		reqName = "TransferContent";
		nstate = transferContent(msgCtx);
		break;

	case A3_GetTransferStatus:
		reqName = "GetTransferStatus";
		nstate = getTransferStatus(msgCtx);
		break;

	case A3_CancelTransfer:
		reqName = "CancelTransfer";
		nstate = cancelTransfer(msgCtx);
		break;

	case A3_ExposeContent:
		reqName = "ExposeContent";
		nstate = exposeContent(msgCtx);
		break;

	case A3_GetContentChecksum:
		reqName = "GetContentChecksum";
		nstate = getContentChecksum(msgCtx);
		break;

	case A3_GetContentInfo:
		reqName = "GetContentInfo";
		nstate = getContentInfo(msgCtx);
		break;

	case A3_DeleteContent:
		reqName = "DeleteContent";
		nstate = deleteContent(msgCtx);
		break;

	case A3_GetVolumeInfo:
		reqName = "GetVolumeInfo";
		nstate = getVolumeInfo(msgCtx);
		break;

	default:
		A3REQLOG(ZQ::common::Log::L_ERROR, CLOGFMT(A3Request, "request() unknown req type %d"), type);
		break;
	};

	msgCtx.errorCode = nstate;
	A3REQLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(A3Request, "request() received resp: %s <= %d"), reqName, nstate);

	return nstate;
}

int A3Request::transferContent(MessageCtx& msgCtx) 
{
	ContentOprtXml xmlOp(_pLog);
	std::string str = xmlOp.makeTransferContent(msgCtx);

	return sendRequest("TransferContent", str);
}

int A3Request::getTransferStatus(MessageCtx& msgCtx) 
{
	ContentOprtXml xmlOp(_pLog);
	std::string str = xmlOp.makeGetTransferStatus(msgCtx);
	
	int code = sendRequest("GetTransferStatus", str);
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

	return sendRequest("CancelTransfer", str);
}

int A3Request::exposeContent(MessageCtx& msgCtx) 
{
	ContentOprtXml xmlOp(_pLog);
	std::string str = xmlOp.makeExposeContent(msgCtx);
	
	int code = sendRequest("ExposeContent", str);
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

	return sendRequest("GetContentChecksum", str);
}

int A3Request::getContentInfo(MessageCtx& msgCtx) 
{
	ContentOprtXml xmlOp(_pLog);
	std::string str = xmlOp.makeGetContentInfo(msgCtx);
	
	int code = sendRequest("GetContentInfo", str, true);
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

	return sendRequest("DeleteContent", str);
}

int A3Request::getVolumeInfo(MessageCtx& msgCtx) 
{
	ContentOprtXml xmlOp(_pLog);
	std::string str = xmlOp.makeGetVolumeInfo(msgCtx);

	int code = sendRequest("GetVolumeInfo", str);
	if(code == 200) 
	{
		if(!xmlOp.parseGetVolumeInfo(str.c_str(), str.length(), msgCtx)) 
			return (-1);	

	}
	return code;
}
