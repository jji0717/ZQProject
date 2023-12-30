#include "./RequestHandler.h"
#include "./Environment.h"

namespace ZQTianShan
{
namespace Plugin
{
namespace SsmRichURL
{

FixupHandle::FixupHandle(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse) : 
	_env(env), _pSite(pSite), _pRequest(pReq), _pResponse(pResponse), _logFromEnv(env.getLog())
{
	_pRequestWriter = dynamic_cast<IClientRequestWriter*>(_pRequest);
}

FixupHandle::~FixupHandle()
{
}

std::string FixupHandle::getUrl()
{
	_szBuf[sizeof(_szBuf) - 1] = '\0';
	const char* pUrl = _pRequest->getUri(_szBuf, sizeof(_szBuf) - 1);
	std::string strUrl = "rtsp://";
	strUrl += NULL != pUrl ? pUrl : "";
	return strUrl;
}

std::string FixupHandle::getRequestHeader(const char* pHeaderStr)
{
	uint16 szBufLen = sizeof(_szBuf) - 1;
	const char* pRetStr = NULL;
	pRetStr = _pRequest->getHeader(pHeaderStr, _szBuf, &szBufLen);
	std::string retHeader;
	retHeader = (NULL != pRetStr) ? pRetStr : "";
	return retHeader;
}

std::string FixupHandle::getResponseHeader(const char* pHeaderStr)
{
	uint16 szBufLen = sizeof(_szBuf) - 1;
	const char* pRetStr = NULL;
	memset(_szBuf, 0, sizeof(_szBuf));
	pRetStr = _pResponse->getHeader(pHeaderStr, _szBuf, &szBufLen);
	std::string retHeader;
	retHeader = (NULL != pRetStr) ? pRetStr : "";
	return retHeader;
}

void FixupHandle::setResponseHeader(const char* pHeaderStr, const char* strValue)
{
	_pResponse->setHeader(pHeaderStr, strValue);
}

bool FixupHandle::needProcess()
{
	std::string userAgent = getRequestHeader(HeaderUserAgent);
	if (!strstr(userAgent.c_str(), _env._config._userAgent._keyword.c_str()))
		return false;

	return true;
}

FixupSetup::FixupSetup(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse) : FixupHandle(env, pSite, pReq, pResponse)
{
}

FixupSetup::~FixupSetup()
{
}

static std::vector<std::string> tokenize(const std::string& str,const std::string& delimiters)
{
	std::vector<std::string> tokens;

	// skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);

	// find first "non-delimiter".
	std::string::size_type pos = str.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		// found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));

		// skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);

		// find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}

	return tokens;
}

static void trim(std::string& str, const char* space_chars=" \t")
{
	std::string::size_type pos1 = str.find_first_not_of(space_chars);
	std::string::size_type pos2 = str.find_last_not_of(space_chars);

	str = str.substr(pos1 == std::string::npos ? 0 : pos1, pos2 == std::string::npos ? str.length() - 1 : pos2 - pos1 + 1);
}

bool FixupSetup::process()
{
	std::string urlStr = getUrl();

	_logFromEnv(DebugLevel, "FixupSetup::process() request url[%s], entry", urlStr.c_str());
	if (!needProcess())
	{
		_logFromEnv(InfoLevel, "FixupSetup::process() [needProcess()] failed but return true, request url[%s]", urlStr.c_str());
		return true; // but here I return true to indicate no error occurs
	}

	_logFromEnv(InfoLevel, "request url is: %s", urlStr.c_str());
	ZQ::common::URLStr urlPsr(urlStr.c_str(), true);

	std::string appData;
	std::string sTransType, sDest, sCltPort, sCltMac, sBandWidth;

	{
		// read the orginal Transport header, overwrite later if necessary
		std::string trspStr;
		char buf[512];
		uint16 len = sizeof(buf)-2;
		if (NULL != _pRequestWriter->getHeader(HeaderTransport, buf, &len))
			trspStr = buf;

//		String::removeChar(trspStr, ' ');
//		STRINGVECTOR trspStrs;
//		String::splitStr(trspStr, ";", trspStrs);

		trim(trspStr);
		std::vector<std::string> trspStrs = tokenize(trspStr, ";");

		for (std::vector<std::string>::const_iterator trspStrsItor = trspStrs.begin(); trspStrsItor != trspStrs.end(); trspStrsItor ++)
		{
			std::vector<std::string> tmpstrs = tokenize(*trspStrsItor, "=");
//			if (trspStrsItor == trspStrs.begin() && tmpstrs.size() < 2)
//				sTransType = *trspStrsItor + ";";

			if (tmpstrs.size() <2)
			{
				if (tmpstrs.size() ==1)
					sTransType += *trspStrsItor + ";";
					
				continue;
			}

			if (0 == tmpstrs[0].compare(Destination))
				sDest = *trspStrsItor + ";";
			else if (0 == tmpstrs[0].compare(ClientPort))
				sCltPort = *trspStrsItor + ";";
			else if (0 == tmpstrs[0].compare(ClientMac))
				sCltMac = *trspStrsItor + ";";
			else if (0 == tmpstrs[0].compare(BandWidth))
				sBandWidth = *trspStrsItor + ";";
		}

		_logFromEnv(DebugLevel, "original Transport: %s%s%s%s", sTransType.c_str(), sDest.c_str(), sCltPort.c_str(), sBandWidth.c_str());
	}

	// enum parameters in url and add them to private data
	std::map<std::string, std::string> urlVars = urlPsr.getEnumVars();
	for (std::map<std::string, std::string>::iterator urlVars_itor = urlVars.begin(); urlVars_itor != urlVars.end(); urlVars_itor ++)
	{
		// set transport type
		if (stricmp(urlVars_itor->first.c_str(), "transport") == 0)
		{
			if (urlVars_itor->second.empty())
				continue;
			
			sTransType = urlVars_itor->second + ";";
			if (0 == urlVars_itor->second.compare("dvbc"))
				sTransType = "MP2T/DVBC/QAM;";
			else if (0 == urlVars_itor->second.compare("udp"))
				sTransType = "MP2T/AVP/UDP;";
			
			continue;
		}

		// 对bandwidth, client_port, destination, client_mac处理，并将他们加入到TransPort字段中
		if (stricmp(urlVars_itor->first.c_str(), BandWidth) == 0)
		{
			if (urlVars_itor->second.empty())
				continue;
			if (!sBandWidth.empty())
			{
				_logFromEnv(WarningLevel,"Bind width [%s] is already special in transport", sBandWidth.c_str());
				continue;
			}

			sBandWidth = std::string(BandWidth) + "=" + urlVars_itor->second + ";";
			continue;
		}

		if (stricmp(urlVars_itor->first.c_str(), Destination) == 0)
		{
			// TODO: 
			if (urlVars_itor->second.empty())
				continue;
			if (!sDest.empty())
			{
				_logFromEnv(WarningLevel,"Client IP [%s] is already special in transport", sDest.c_str());
				continue;
			}

			sDest = std::string(Destination) + "=" + urlVars_itor->second + ";";
			continue;
		}

		if (stricmp(urlVars_itor->first.c_str(), ClientPort) == 0)
		{
			// TODO:
			if (urlVars_itor->second.empty())
				continue;
			if (!sCltPort.empty())
			{
				_logFromEnv(WarningLevel,"Client port [%s] is already special in transport", sCltPort.c_str());
				continue;
			}

			sCltPort = std::string(ClientPort) + "=" + urlVars_itor->second + ";";
			continue;
		}

		if (stricmp(urlVars_itor->first.c_str(), ClientMac) == 0)
		{
			if (urlVars_itor->second.empty())
				continue;
			if (!sCltMac.empty())
			{
				_logFromEnv(WarningLevel,"Client MAC address [%s] is already special in transport", sCltMac.c_str());
				continue;
			}

			sCltMac = std::string(ClientMac) + "=" + urlVars_itor->second + ";";
			continue;
		}

		// set TianShan-ServiceGroup
		if (stricmp(urlVars_itor->first.c_str(), "ServiceGroup") == 0)
		{
//			if (urlVars_itor->second.empty())
//				continue;

			_pRequestWriter->setHeader(HeaderTianShanServiceGroup, (char*) urlVars_itor->second.c_str());
			_logFromEnv(DebugLevel, "%s: %s", HeaderTianShanServiceGroup, urlVars_itor->second.c_str());
			continue;
		}

		appData += urlVars_itor->first + "=" + urlVars_itor->second + ";";
	}

	std::string sTransport = sTransType + sDest + sCltPort + sCltMac + sBandWidth;
	_pRequestWriter->setHeader(HeaderTransport, (char*) sTransport.c_str());
	_logFromEnv(DebugLevel, "%s: %s", HeaderTransport, sTransport.c_str());

	std::string preAppData = getRequestHeader(HeaderTianShanAppData);
	if (!preAppData.empty())
		preAppData += ";";
	preAppData += appData;
	_pRequestWriter->setHeader(HeaderTianShanAppData, (char*) preAppData.c_str());
	_logFromEnv(DebugLevel, "%s: %s", HeaderTianShanAppData, preAppData.c_str());

	_logFromEnv(DebugLevel, "FixupSetup::process() request url[%s], succeed", urlStr.c_str());
	return true;
}

FixupDescribe::FixupDescribe(Environment& env, IStreamSmithSite* pSite, IClientRequestWriter* pReq, IServerResponse* pResponse) : FixupHandle(env, pSite, pReq, pResponse)
{
}

FixupDescribe::~FixupDescribe()
{
}

bool FixupDescribe::process()
{
	std::string urlStr = getUrl();

	_logFromEnv(DebugLevel, "FixupDescribe::process() request url[%s], entry", urlStr.c_str());
	if (!needProcess())
	{
		_logFromEnv(InfoLevel, "FixupDescribe::process()  [needProcess()] failed but return true, request url[%s]", urlStr.c_str());
		return true; // but here I return true to indicate no error occurs
	}

	// if transport Header has no 'MP2T/DVBC/QAM' or 'MP2T/AVP/UDP', get the transport type from url
	std::string trspStr = getRequestHeader(HeaderTransport);

	// if (NULL == strstr(trspStr.c_str(), "MP2T/DVBC/QAM") &&	NULL == strstr(trspStr.c_str(), "MP2T/AVP/UDP"))
	if (NULL == strstr(trspStr.c_str(), "/DVBC/") && NULL == strstr(trspStr.c_str(), "/QAM") && NULL == strstr(trspStr.c_str(), "/UDP"))
	{
		_logFromEnv(InfoLevel, CLOGFMT(FixupDescribe, "request url is: %s"), urlStr.c_str());
		ZQ::common::URLStr urlPsr(urlStr.c_str(), true);		
		std::string resultTrans;
		std::map<std::string, std::string> urlVars = urlPsr.getEnumVars();
		for (std::map<std::string, std::string>::iterator urlVars_itor = urlVars.begin(); urlVars_itor != urlVars.end(); urlVars_itor ++)
		{
			// set transport type
			if (stricmp(urlVars_itor->first.c_str(), "transport") == 0)
			{
				resultTrans = urlVars_itor->second + ";";
				if (0 == urlVars_itor->second.compare("dvbc"))
					resultTrans = "MP2T/DVBC/QAM;";
				else if (0 == urlVars_itor->second.compare("udp"))
					resultTrans = "MP2T/AVP/UDP;";
					
				break;
			}
		}
		if (!resultTrans.empty())
		{
			resultTrans += ";" + trspStr;
			_pRequestWriter->setHeader(HeaderTransport, (char*) resultTrans.c_str());
		}
	}

	_logFromEnv(DebugLevel, "FixupDescribe::process() request url[%s], succeed", urlStr.c_str());
	return true;
}

}
}
}

