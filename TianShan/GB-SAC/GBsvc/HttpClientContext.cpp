#include <string>
#include <fstream>
#include <sstream>

#include "HttpClientContext.h"
#include "XMLPreferenceEx.h"
#include "IOCPThreadPool.h"
#include "NGBCmdResponse.h"
#include "GBsvcConfig.h"

namespace ZQTianShan {	  
namespace GBServerNS { 

#define CRLF "\r\n"
#define ONE_SPACE " "

char * XMLFileInf::_parserStatus[XMLFileInf::STATUS_COUNT + 1] = 
{
	"SUCCEED",
	"INPUT_XML_EMPTY",
	"READ_FAILED",	
	"GET_ROOTPREFERENCE_FAILED",
	"GET_HEADER_FAILED",
	"GET_BODY_FAILED",
	"OPCODE_FAILED",
	"STATUS_COUNT"
};

HttpContext::HttpStatusContent  HttpContext::_httpStatusTbl[HttpContext::_httpStatusTblSize] = 
{
	{HTTP_CONTINUE             , "100", "Continue"},
	{HTTP_OK                   , "200", "OK"},
	{HTTP_BAD_REQUEST          , "400", "Bad Request"},
	{HTTP_FORBIDDEN            , "403", "Forbidden"},
	{HTTP_NOT_FOUND            , "404", "Not Found"},
	{HTTP_METHOD_NOT_ALLOWED   , "405", "Method not Allowed"},
	{HTTP_INTERNAL_SERVER_ERROR, "500", "Internal Server Error"},
	{HTTP_NOT_IMPLEMENTED      , "501", "Not Implemented"},
	{HTTP_STATUS_END           , "end", "Http status end"}
};

int  XMLFileInf::parseXmlBuf(void)
{
	int nRev = false;
	if (_xmlBuf.empty())
	{
		_xmlParseResult["parseHttpResponse"] = XMLFileInf::_parserStatus[XMLFileInf::INPUT_XML_EMPTY];
		return nRev;
	}

	try
	{
		_xmlParseResult["parseHttpResponse"] = XMLFileInf::_parserStatus[XMLFileInf::SUCCEED];
		ZQ::common::XMLPreferenceDocumentEx xmlDoc;
		if(!xmlDoc.read((void*)_xmlBuf.c_str(), (int)_xmlBuf.length(), 1)	)
		{
			_xmlParseResult["parseHttpResponse"] = XMLFileInf::_parserStatus[XMLFileInf::READ_FAILED];
			return nRev;
		}

		ZQ::common::XMLPreferenceEx* pRoot = xmlDoc.getRootPreference();
		if (!pRoot)	 
			_xmlParseResult["parseHttpResponse"] = XMLFileInf::_parserStatus[XMLFileInf::GET_ROOTPREFERENCE_FAILED];
		else  
		{
			ZQ::common::XMLPreferenceEx* header = pRoot->findChild("Header");
			if (!header)
				_xmlParseResult["parseHttpResponse"] = XMLFileInf::_parserStatus[XMLFileInf::GET_HEADER_FAILED];
			else		
			{
				for(ZQ::common::XMLPreferenceEx* pNext = header->firstChild(); pNext != NULL; pNext = header->nextChild())
				{
					const int preferenceSize = 128;
					char preferenceName[preferenceSize] = {0};
					char preferenceText[preferenceSize] = {0};
					pNext->getPreferenceName(preferenceName, true, preferenceSize);
					pNext->getPreferenceText(preferenceText, preferenceSize);
					_xmlParseResult[preferenceName] = preferenceText;
					pNext->free(); pNext = NULL;
				}

				header->free(); 
				header = NULL;
			}

			ZQ::common::XMLPreferenceEx* body = pRoot->findChild("Body");
			if (!body)
				_xmlParseResult["parseHttpResponse"] = XMLFileInf::_parserStatus[XMLFileInf::GET_BODY_FAILED];
			else		
			{
				ZQ::common::XMLPreferenceEx* pStoreLast = body;
				ZQ::common::XMLPreferenceEx* pFree = NULL;
				for(ZQ::common::XMLPreferenceEx* pNext = pStoreLast->firstChild(); pNext != NULL; )
				{
					const int preferenceSize = 128;
					char preferenceName[preferenceSize] = {0};
					char preferenceText[preferenceSize] = {0};
					std::map<std::string, std::string> storeProperties =  pNext->getProperties();

					pNext->getPreferenceName(preferenceName, true, preferenceSize);
					pNext->getPreferenceText(preferenceText, preferenceSize);
					_xmlParseResult[preferenceName] = preferenceText;

					if(!storeProperties.empty())
						_xmlParseResult.insert(storeProperties.begin(), storeProperties.end());

					pFree = pNext;
					pNext = pStoreLast->nextChild();
					if (NULL == pNext)
					{
						pStoreLast = body->findChild(preferenceName);
						if(NULL != pStoreLast)
							pNext = pStoreLast->firstChild();
					}
					
					pFree->free();
					pFree = NULL;
				}

				body->free(); 
				body = NULL;
				nRev = true;
			}

			pRoot->free();
		}

		xmlDoc.clear();
	}
	catch (...)
	{
		_xmlParseResult["parseHttpResponse"] = XMLFileInf::_parserStatus[XMLFileInf::READ_FAILED];
	   nRev = false;
	}

	return nRev;
}

HttpContext::HttpContext(ZQ::common::NativeThreadPool&  releasePool, ZQ::common::Log& log)
: _releasePool(releasePool), _log(log)
{
	_httpRequst = HTTP_REQUEST_END;
	_httpStatus = HTTP_OK;
}

int  HttpContext::obtainHttpContent(char * buf, int bufSize)
{
	int nRev = false;
	if (NULL != strstr(buf, "POST"))
		_httpRequst = HTTP_POST_FROM_CLIENT;
	else if (NULL != strstr(buf, "HTTP/1.1"))
		_httpRequst = HTTP_POST_TO_CLIENT;
	else 
	{
	    _httpStatus = HTTP_NOT_IMPLEMENTED;
		return nRev;
	}
	
	char * posLength = ::strstr(buf, "Content-Length");
	char * swap = posLength;
	if (NULL == posLength)
	{
		_httpStatus = HTTP_METHOD_NOT_ALLOWED;
		return nRev;
	}

	if(NULL == (posLength = ::strchr(posLength, ':')) ||
		NULL == (swap = ::strchr(swap, '\r')) )
	{
		_httpStatus = HTTP_BAD_REQUEST;
		return nRev;
    }

	++posLength;
	unsigned int sizeXML = (swap - posLength);
	if(24 < sizeXML)
	{
		_httpStatus = HTTP_BAD_REQUEST;
		return nRev;
	}

	char bufXMLsize[24] = {0};
	memcpy(bufXMLsize, posLength, sizeXML);
	_contentInf._xmlBufSize = ::atoi(bufXMLsize);
	if(NULL == (swap = ::strstr(swap, "\r\n\r\n")))
	{
		_httpStatus = HTTP_BAD_REQUEST;
		return nRev;
	}

	swap += 4;
	if ((buf - swap) < _contentInf._xmlBufSize )
	{
		_httpStatus = HTTP_BAD_REQUEST;
		return nRev;
	}

	_contentInf._xmlBuf.clear();
	_contentInf._xmlBuf.append(swap, _contentInf._xmlBufSize);
	return true;
}

int  HttpContext::parseHttpContent(void)
{
	int nRev = false;
	if (HTTP_OK != _httpStatus)
		return nRev;

	ZQ::common::MutexGuard g(_lockXmlParse);
	nRev = _contentInf.parseXmlBuf();
	if (!nRev)
	{
		_httpStatus = HTTP_BAD_REQUEST;
		return nRev;
	}

	std::map<std::string, std::string>::iterator itOpCode = _contentInf._xmlParseResult.find("OpCode");
	if(itOpCode == _contentInf._xmlParseResult.end())
		_httpStatus = HTTP_BAD_REQUEST;
	else 
	{
		_contentInf._opCodeInXml = itOpCode->second;
		nRev =true;
	}

    return nRev;
}

int  HttpContext::reponseHttpClient(ClientSocketContext* dataKey)
{
	std::string reqHttpReponseBuf(preHttpResponse());

	int preSendSize = reqHttpReponseBuf.size();
	dataKey->_comPortType = WREATE;
	memset(&(dataKey->_wsaBuf), 0, sizeof(dataKey->_wsaBuf));
	dataKey->_wsaBuf.buf = const_cast<char*>(reqHttpReponseBuf.c_str());
	dataKey->_wsaBuf.len = static_cast<unsigned long>(preSendSize);
	DWORD dwSent = 0;
	DWORD dwFlag = 0;

	memset(&dataKey->_overlapped, 0, sizeof(dataKey->_overlapped) );
	int iRet = WSASend(dataKey->_clientSocket, &dataKey->_wsaBuf, 1, &dwSent, dwFlag, &dataKey->_overlapped, NULL);
	if(SOCKET_ERROR == iRet)
	{
		(new ReleaseResource(_releasePool, dataKey))->start();
	}

	return 0;
}

std::string  HttpContext::preHttpStatusLine(void)
{
	if (HTTP_POST_TO_CLIENT == _httpRequst)
		return std::string("POST HTTP/1.1 \r\n");

	std::string statusLine("HTTP/1.1 ");
	statusLine += _httpStatusTbl[_httpStatus].statusCode;
	statusLine += ONE_SPACE;
	statusLine += _httpStatusTbl[_httpStatus].reasonPhrase;
	statusLine += CRLF;

	return statusLine;
}

std::string  HttpContext::preHttpGeneralHeader(void)
{
	std::string generalHeader;
	generalHeader += "Cache-Control: no-cache";
	generalHeader += CRLF;

	generalHeader += "Date: ";
	generalHeader += HttpContextUtil::currentDateString();
	generalHeader += CRLF;

	generalHeader += "Connection: close";
	generalHeader += CRLF;

	return generalHeader;
}

std::string  HttpContext::preHttpResponseHeader(void)
{
	std::string responseHeader;
	responseHeader += "Server: GBSVC";
	responseHeader += CRLF;

	return  responseHeader;
}

std::string  HttpContext::preHttpEntityHeader(unsigned int messageBodySize)
{
	std::string entityHeader;
	entityHeader += "Content-Type: text/xml";
	entityHeader += CRLF;

	entityHeader += "Content-Length: ";
	entityHeader += HttpContextUtil::int2str(messageBodySize);
	entityHeader += CRLF;

	return entityHeader;
}

std::string  HttpContext::preHttpMessageBody(void)
{
	if (HTTP_OK != _httpStatus)
		return std::string();

	NGBCmdCode ngbCmdCode = A5_cmd_end;
	int ngbCmdTblNum = sizeof(ngbCmdTbl)/sizeof(ngbCmdTbl[0]);
	for (int nLoop = 0; nLoop < ngbCmdTblNum; ++nLoop)
	{
		if (0 == _contentInf._opCodeInXml.compare(ngbCmdTbl[nLoop]._ngbCmdStr))
		{
			ngbCmdCode = ngbCmdTbl[nLoop]._ngbCmdCode;
			break;
		}
	}

	std::string messageBody;
	switch(ngbCmdCode)
	{
	case AO_CDN_A4_FILE_PROPAGATION_REQ:
	case AO_CDN_A4_FILE_BAT_PROPAGATION_REQ:
	case AO_CDN_A4_FILE_PROPAGATION_CANCEL: 
	case AO_CDN_A4_FILE_DELETE:
		{
			A4FileStateNotifyResp combineMessageBody(_contentInf._xmlParseResult, _contentInf._opCodeInXml);
			messageBody = combineMessageBody.makeXmlContent();
			(new FeedBack(_releasePool, _contentInf, CDN_AO_A4_FILE_STATE_NOTIFY, _log))->start();
		}
		break;

	case AO_CDN_A5_STREAM_INGEST_REQ:
	case AO_CDN_A5_STREAM_BAT_INGEST_REQ:
	case AO_CDN_A5_STREAM_INGEST_CANCEL:
		{
			IGBA5Resp combineMessageBody(_contentInf._xmlParseResult, _contentInf._opCodeInXml);
			messageBody = combineMessageBody.makeXmlContent();
			(new FeedBack(_releasePool, _contentInf, CDN_AO_A5_STREAM_STATE_NOTIFY, _log))->start();
		}
		break;

	case AO_CDN_A4_FILE_STATE_REQ:
		{
			A4FileStateResp combineMessageBody(_contentInf._xmlParseResult, _contentInf._opCodeInXml);
			messageBody = combineMessageBody.makeXmlContent();
		}
		break;

	case AO_CDN_A5_STREAM_STATE_REQ:
		{
			A5StreamStateResp combineMessageBody(_contentInf._xmlParseResult, _contentInf._opCodeInXml);
			messageBody = combineMessageBody.makeXmlContent();
		}
		break;

	case A5_cmd_end:
	case A4_cmd_end:
	default:
		   break;
	}

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpContext,"preHttpMessageBody ngbCmdCode[%s], httpStatus[%s]"), ngbCmdTbl[ngbCmdCode]._ngbCmdStr, _httpStatusTbl[_httpStatus].statusCode);
	return messageBody;
}

std::string  HttpContext::preHttpResponse(void)
{
	extern ZQ::common::Config::Loader<ZQTianShan::GBServerNS::GBServerConfig > gConfig;

	std::string response;

	if(gConfig._gbSvcBase._enableFileDebug)
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HttpContext,"preHttpMessageBody ngbCmdCode[%s], httpStatus[%s]"), _contentInf._opCodeInXml.c_str(), _httpStatusTbl[_httpStatus].statusCode);
		std::string filePath(gConfig._gbSvcBase._filesDirectory);
		if(filePath.compare(filePath.length(), 1, "\\", 1)) { filePath +="\\"; };		
		filePath += _contentInf._opCodeInXml; 
		filePath +=".txt"; 
		std::fstream fileStr(filePath.c_str());
		std::ostringstream tmp;
		tmp << fileStr.rdbuf();
		return (response = tmp.str());
	}
	
	response += preHttpStatusLine();
	response += preHttpGeneralHeader();
	response += preHttpResponseHeader();

	std::string messageBody = preHttpMessageBody();
	if (!messageBody.empty())
	{
		response += preHttpEntityHeader(messageBody.size());
		response += CRLF;
		response += messageBody;
	}

	response += CRLF;
	return response;
}


HttpFeedBackContext::HttpFeedBackContext(XMLFileInf& contentInf, unsigned int ngbCmdCode)
:_contentInf(contentInf), _ngbCmdCode(A5_cmd_end)
{
	int ngbCmdTblNum = sizeof(ngbCmdTbl)/sizeof(ngbCmdTbl[0]);
	if(ngbCmdTblNum > ngbCmdCode)
		_ngbCmdCode = ngbCmdCode;
};

std::string  HttpFeedBackContext::preHttpFeedBack(void)
{
	extern ZQ::common::Config::Loader<ZQTianShan::GBServerNS::GBServerConfig > gConfig;

	std::string request;

	if(gConfig._gbSvcBase._enableFileDebug)
	{
		std::string filePath(gConfig._gbSvcBase._filesDirectory);
		if(filePath.compare(filePath.length(), 1, "\\", 1)) { filePath +="\\"; };		
		filePath += ngbCmdTbl[_ngbCmdCode]._ngbCmdStr; 
		filePath +=".txt"; 
		std::fstream fileStr(filePath.c_str());
		std::ostringstream tmp;
		tmp << fileStr.rdbuf();
		return (request = tmp.str());
	}

	request += httpStatusLine();	
	std::string messageBody = httpMessageBody();
	if (!messageBody.empty())
	{
		request += httpEntityHeader(messageBody.size());
		request += CRLF;
		request += messageBody;
	}

	return request;
}

std::string  HttpFeedBackContext::httpEntityHeader(unsigned int messageBodySize)
{
	std::string entityHeader;
	entityHeader += "Content-Type: text/xml";
	entityHeader += CRLF;

	entityHeader += "Content-Length: ";
	entityHeader += HttpContextUtil::int2str(messageBodySize);
	entityHeader += CRLF;

	return entityHeader;
}

std::string  HttpFeedBackContext::httpStatusLine()
{
	extern ZQ::common::Config::Loader<ZQTianShan::GBServerNS::GBServerConfig > gConfig;
	std::string statusLine("POST http://");
	statusLine += gConfig._gbSvcBase._feedBackIP;
	statusLine += ":";
	statusLine += HttpContextUtil::int2str(gConfig._gbSvcBase._feedBackPort);
	statusLine += " ";
	statusLine += "HTTP/1.1 \r\n";

	return statusLine;
}

std::string  HttpFeedBackContext::httpMessageBody()
{
	using namespace ZQTianShan::GBServerNS;
	NGBCmdCode ngbCmdCode = (NGBCmdCode)_ngbCmdCode;
	switch(ngbCmdCode)
	{
	case CDN_AO_A4_FILE_STATE_NOTIFY:
		{
			A4FileStateNotifyResp combineMessageBody(_contentInf._xmlParseResult, ngbCmdTbl[_ngbCmdCode]._ngbCmdStr);
			return combineMessageBody.makeXmlContent();
		}

	case CDN_AO_A5_STREAM_STATE_NOTIFY:
		{
			A5StreamStateNotifyResp combineMessageBody(_contentInf._xmlParseResult, ngbCmdTbl[_ngbCmdCode]._ngbCmdStr);
			return combineMessageBody.makeXmlContent();
		}

	default:
		break;
	}

	return std::string();
}

std::string HttpContextUtil::currentDateString(void)
{ // format: <Fri, 01 Oct 1999 21:11:54 GMT>
	static char* monthText[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	static char* dayText[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

	char buf[32];
	time_t tmt;
	struct tm* ptm;
	tmt = time(0);
	ptm = gmtime(&tmt);
	if(!ptm)
		return "";
	sprintf(buf, "%3s, %02d %3s %4d %02d:%02d:%02d GMT",
		dayText[ptm->tm_wday],
		ptm->tm_mday,
		monthText[ptm->tm_mon],
		ptm->tm_year+1900,
		ptm->tm_hour,
		ptm->tm_min,
		ptm->tm_sec);

	return buf;
}

std::string HttpContextUtil::int2str(int size)
{
	char buf[12] = {0};
	sprintf(buf,"%d", size);
	return buf;
}

}//GBServerNS
}//	ZQTianShan