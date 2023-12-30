#include "GBCSReq.h"
#include "Guid.h"


extern int gbcsContentInterfaceTestEnable;
extern std::string gbcsContentInterfaceTestFolder;

namespace ZQTianShan {
namespace ContentStore {

ZQ::common::Mutex  GBCSReq::_lockXmlParse;

ZQ::common::Log    g_nullLog;  
const std::string  XML_HEADER("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n");


char * GBCSCmdUtil::_parserStatus[GBCSCmdUtil::STATUS_COUNT + 1] = {
	"SUCCEED",
	"INPUT_XML_EMPTY",
	"READ_FAILED",	
	"GET_ROOTPREFERENCE_FAILED",
	"GET_HEADER_FAILED",
	"GET_BODY_FAILED",
	"OPCODE_FAILED",
	"STATUS_COUNT"
};


IGBCSCmd::IGBCSCmd()
{
	_uuid.clear();
	ZQ::common::Guid guid;
	guid.create();
	char buf[80];
	guid.toCompactIdstr(buf, sizeof(buf) -2);
	_uuid = buf;
}

std::string IGBCSCmd::getUUID(void)
{
	return _uuid;
}

std::string  IGBCSCmd::makeHttpContent(void)
{
	std::string httpContent(XML_HEADER);
	std::string httpContentHead;
	std::string httpContentBody;

	httpContentHead =  makeContentHeader();
	httpContentBody =  makeContentBody();

	httpContent += "<Message>\n";
	httpContent += httpContentHead;
	httpContent += httpContentBody;
	httpContent += "</Message>\r\n";

	return httpContent;
}


std::map<std::string, std::string >  IGBCSCmd::parseHttpResponse(std::string & httpResponse)
{
	std::map<std::string, std::string> httpResponseMap;
	if (httpResponse.empty())
	{
		httpResponseMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::INPUT_XML_EMPTY];
		return httpResponseMap;
	}

	httpResponseMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::SUCCEED];
	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	if(!xmlDoc.read((void*)httpResponse.c_str(), (int)httpResponse.length(), 1)	)
	{
		httpResponseMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::READ_FAILED];
		return httpResponseMap;
	}

	ZQ::common::XMLPreferenceEx* pRoot = xmlDoc.getRootPreference();
	if (!pRoot)	 
		httpResponseMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_ROOTPREFERENCE_FAILED];
	else  
	{
		ZQ::common::XMLPreferenceEx* header = pRoot->findChild("Header");
		if (!header)
			httpResponseMap["parseHttpResponse"] = GBCSCmdUtil::_parserStatus[GBCSCmdUtil::GET_HEADER_FAILED];
		else		
		{
			for(ZQ::common::XMLPreferenceEx* pNext = header->firstChild(); pNext != NULL; pNext = header->nextChild())
			{
				const int preferenceSize = 128;
				char preferenceName[preferenceSize] = {0};
				char preferenceText[preferenceSize] = {0};
				pNext->getPreferenceName(preferenceName, true, preferenceSize);
				pNext->getPreferenceText(preferenceText, preferenceSize);
				httpResponseMap[preferenceName] = preferenceText;
				pNext->free(); pNext = NULL;
			}

			header->free(); header = NULL;
		}

		pRoot->free();
	}

	xmlDoc.clear();
	return httpResponseMap;
}

std::string GBCSCmdUtil::setAttrStr(const char* key, const char* value)
{
	if(key == NULL || *key == '\0' || value == NULL || *value == '\0')
		return std::string();

	std::string strCont;
	strCont = key;
	strCont += "=\"";
	strCont += value;
	strCont += "\" ";

	return  strCont;
}

std::string  GBCSCmdUtil::setAttrStr(const char* key, const std::string & value)
{
	if(key == NULL || *key == '\0' || value.empty())
		return std::string();

	std::string strCont;
	strCont = key;
	strCont += "=\"";
	strCont += value;
	strCont += "\" ";

	return  strCont;
}

std::string  GBCSCmdUtil::setAttrStr(const std::string & key, const std::string & value)
{
	if (key.empty())
		return std::string();
	
	if (value.empty())
		return key + "=\"  \" ";

	std::string strCont;
	strCont = key;
	strCont += "=\"";
	strCont += value;
	strCont += "\" ";

	return  strCont;
}

std::string  GBCSCmdUtil::setElementStr(const std::string & tag, const std::string & attrStr)//<tag  attrStr />
{
	std::string element("<");

	element += tag;
	element += "  ";
	element += attrStr;
	element += " />";
    return element;
}

std::string  GBCSCmdUtil::setElementStr(const std::string & tag, const std::string & attrStr,  int endTagErase)// <tag  attrStr> </tag>
{  //  endTagErase for type system checking
	std::string element("<");

	element += tag;
	element += " ";
	element += attrStr;
	element += "  ></";
	element += tag;
	element += ">";
	return element;
}

std::string  GBCSCmdUtil::setElementStr(const std::string & tagStart, const std::string & entity, const std::string & tagEnd)// <tag> entity </tag>    tagStart like <tag>,  tagEnd like </tag>
{
	std::string element(tagStart);

	element += entity;
	element += tagEnd;
	return element;
}

std::string  GBCSCmdUtil::setElementStr(const std::string & tagStart, const std::string attrStr, const std::string & entity, const std::string & tagEnd)// <tag attrStr> entity </tag>   tagStart like <tag ,  tagEnd like </tag>
{
	std::string element(tagStart);

	element += attrStr;
	element += " >";
	element += entity;
	element += tagEnd;
	return element;
}

GBCSReq::GBCSReq(ZQ::common::Log*  log, std::string reqHost)
	:_log(log), _reqGBCmd(NULL), _reqHost(reqHost)
{	
	_http.init(); 
	_http.setLog(_log);
	if (NULL == _log)
		_log = &g_nullLog;
}

int  GBCSReq::setHttpHeader(char* key/* = NULL*/, char* val/* = NULL*/)
{
	 int nRev = false;
	 if (NULL == key)
	 {
		  (*_log)(ZQ::common::Log::L_ERROR,"setHttpHeader() failed, key[NULL]");
		  return nRev;
	 }

	 if(NULL == val)
	 {
		 (*_log)(ZQ::common::Log::L_ERROR,"setHttpHeader() failed, key[%s], val[NULL]", key);
		 return nRev;
	 }

	 nRev = true;
	 _http.setHeader(key, val);
	 
	 return nRev;
}

int  GBCSReq::sendRequest(IGBCSCmd*  reqGBCmd, int timeout)	
{
   _reqGBCmd = reqGBCmd;
   return sendRequest(timeout);
}

int  GBCSReq::sendRequest(int timeout)
{
	int nRev = false;
	if (NULL == _reqGBCmd)
	{
		(*_log)(ZQ::common::Log::L_ERROR,"sendRequest() failed, cmd[Unknown], endpoint[%s]", _reqHost.c_str());
		return nRev;
	}

	if(!gbcsContentInterfaceTestEnable)
	{
		std::string reqGBCmdStr = _reqGBCmd->getCmdStr();
		if (0 >= _reqHost.length())
		{
			(*_log)(ZQ::common::Log::L_ERROR,"sendRequest() failed, cmd[%s], endpoint[NULL]", reqGBCmdStr.c_str());
			return nRev;
		}

		_reqHttpSendBuf = _reqGBCmd->makeHttpContent();
		if (_reqHttpSendBuf.empty())
		{
			(*_log)(ZQ::common::Log::L_ERROR,"sendRequest() failed, send buffer[empty],cmd[%s], endpoint[%s]", reqGBCmdStr.c_str(), _reqHost.c_str());
			return nRev;
		}

		int httpTimeOut = timeout;
		if (timeout < 0)
			httpTimeOut = 0;
		else if(timeout / 1000  > 0)
			httpTimeOut = timeout / 1000;
		else
			httpTimeOut = -(timeout * 1000);

		_http.setConnectTimeout(httpTimeOut);
		_http.setSendTimeout(httpTimeOut);
		_http.setRecvTimeout(httpTimeOut);

		_http.setHeader("Content-Type", "text/xml; charset=utf-8");
		_http.setHeader("User-Agent",   "TianShan GBCS/1.15"); // TODO: should reset for  ngb

		return httpSendRequest();  
	} 
	else
	{ 
		std::string filePath(gbcsContentInterfaceTestFolder);
		filePath += _reqGBCmd->getCmdStr();
		filePath += ".txt";

		std::fstream fileStr(filePath.c_str());
		std::ostringstream tmp;
		tmp << fileStr.rdbuf();
		_reqHttpReponseBuf = tmp.str();	
	}

	return 200;
}


int  GBCSReq::httpSendRequest(void)
{
	int nRev = false;
	std::string reqGBCmdStr = _reqGBCmd->getCmdStr();
	if(_http.httpConnect(_reqHost.c_str())) 
	{
		(*_log)(ZQ::common::Log::L_ERROR,"httpSendRequest() connect failed,cmd[%s], endpoint[%s]", reqGBCmdStr.c_str(), _reqHost.c_str());
		_http.uninit();
		return nRev;
	}

	if(_http.httpSendContent(_reqHttpSendBuf.data(), _reqHttpSendBuf.length()) || _http.httpEndSend()) 
	{
		(*_log)(ZQ::common::Log::L_ERROR,"httpSendRequest() send failed, cmd[%s], errorcode[%d]", reqGBCmdStr.c_str(), _http.getErrorcode());
		_http.uninit();
		return nRev;
	}

	if(_http.httpBeginRecv()) 
	{
		(*_log)(ZQ::common::Log::L_ERROR,"httpSendRequest() receive failed, cmd[%s], error[%s]", reqGBCmdStr.c_str(), _http.getErrorstr());
		_http.uninit();
		return nRev;
	}

	while(!_http.isEOF()) 
	{
		if(_http.httpContinueRecv()) 
		{
			(*_log)(ZQ::common::Log::L_ERROR,"httpSendRequest() continue receive failed, cmd[%s], error[%s]", reqGBCmdStr.c_str(), _http.getErrorstr());
			_http.uninit();	
			return nRev;
		}
	}

	_http.getContent(_reqHttpReponseBuf);
	_http.uninit();
	(*_log)(ZQ::common::Log::L_DEBUG, "sendRequest() cmd[%s] receive succeed", reqGBCmdStr.c_str());
	_log->hexDump(ZQ::common::Log::L_DEBUG, _reqHttpReponseBuf.c_str(), _reqHttpReponseBuf.length(), NULL);

	return _http.getStatusCode();
}


std::map<std::string, std::string>  GBCSReq::getStatusMsg(void)
{
	if (NULL == _reqGBCmd)
	{
		(*_log)(ZQ::common::Log::L_ERROR, "getStatusMsg() failed, cmd[Unknown]");
		return std::map<std::string, std::string>();
	}

	ZQ::common::MutexGuard sync(ZQTianShan::ContentStore::GBCSReq::_lockXmlParse);
	return _reqGBCmd->parseHttpResponse(_reqHttpReponseBuf);
}

}//namespace  ContentStore
}//	namespace ZQTianShan