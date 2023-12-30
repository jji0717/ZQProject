// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: CdmiClientBase.h Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : common CDMI file operations
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/CdmiFuse/CdmiClientBase.cpp $
// 
// 9     9/10/15 4:41p Hongquan.zhang
// 
// 8     6/04/14 12:17p Ketao.zhang
// 
// 7     6/04/14 11:41a Ketao.zhang
// check in for BufferList
// 
// 6     12/12/13 9:59a Hui.shao
// 
// 5     12/11/13 5:25p Hui.shao
// 
// 4     8/01/13 10:24a Li.huang
// 
// 3     7/31/13 6:24p Ketao.zhang
// 
// 2     7/19/13 6:51p Ketao.zhang
// 
// 1     7/11/13 4:58p Li.huang

// ===========================================================================

#include "CdmiClientBase.h"
#include "TimeUtil.h"
#include "urlstr.h"
#include "SystemUtils.h"
#include "Hmac_sha1.h"
#include "base_64.h"
#include <algorithm>

#define CDMI_DATAOBJECT_TYPE   "application/cdmi-object"
#define CDMI_CONTAINER_TYPE    "application/cdmi-container"
#define CDMI_URER_TYPE         "application/cdmi-user"
#define CDMI_CONFIG_TYPE       "application/cdmi-config"
#define CDMI_Version   "1.0"


#define SET_OPTIONAL_STR_PARAM(_JSONROOT, _KEY, _VAL) if (!_VAL.empty()) _JSONROOT[_KEY] = _VAL

// -----------------------------
// class CdmiClientBase
// -----------------------------
CdmiClientBase::CdmiClientBase(ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, const std::string& rootUrl, const std::string& homeContainer, const uint32 flags)
: _log(log), _rootUrl(rootUrl), _homeContainer(homeContainer), _thrdPool(thrdPool), _connectTimeout(5000), _flags(flags), _timeout(10000)
{
	_secretKeyId = "";
	_accessKeyId = "";

	if(!_rootUrl.empty() && _rootUrl[_rootUrl.length() -1] != '/')
		_rootUrl += "/";

	ZQ::common::URLStr strUrl(_rootUrl.c_str());
	_password = strUrl.getPwd();
	_username = strUrl.getUserName();

	// cut off the leading '/'
	size_t startpos = _homeContainer.find_first_not_of("/");
	if (std::string::npos != startpos)
		_homeContainer = _homeContainer.substr(startpos);

	// make sure there is an ending '/'
	if(!_homeContainer.empty() && _homeContainer[_homeContainer.length() -1] != '/')
		_homeContainer += "/";
}

CdmiClientBase::~CdmiClientBase()
{
}
void CdmiClientBase::setAccount(const std::string& serverLogin, const std::string& password, const std::string& localRunAs)
{
	_username = serverLogin;
	_password = password;
	_localRunAs = localRunAs;
}
void  CdmiClientBase::setTimeout(uint connectTimeout, uint timeout)
{	
	_connectTimeout = connectTimeout;
	_timeout = timeout;
}

std::string CdmiClientBase::pathToUri(const std::string& pathname)
{
	// cut off the leading slashes
	std::string uri;
	size_t startpos = pathname.find_first_not_of(FNSEPS LOGIC_FNSEPS);
	if (std::string::npos != startpos)
		uri += pathname.substr(startpos);

	if (FNSEPC != LOGIC_FNSEPC && !uri.empty())
		std::replace(uri.begin(), uri.end(), FNSEPC, LOGIC_FNSEPC);

	// see if it is necessary to perform URL encoding for some parameters/fields
	std::string olduri = uri; uri="";
	char buf[400];
	for (size_t pos = olduri.find_first_of(LOGIC_FNSEPS "?=&;"); 
		std::string::npos != pos; 
		olduri = olduri.substr(pos+1), pos = olduri.find_first_of(LOGIC_FNSEPS "?=&;"))
	{
		std::string token = olduri.substr(0, pos);
		ZQ::common::URLStr::encode(token.c_str(), buf, sizeof(buf)-2);
		uri +=buf; uri += olduri[pos];
	}

	if (!olduri.empty())
	{
		ZQ::common::URLStr::encode(olduri.c_str(), buf, sizeof(buf)-2);
		uri +=buf;
	}

	uri = _homeContainer + uri;
	return uri;
}
std::string CdmiClientBase::assembleURL(const std::string& serverIp, const std::string& uri, bool bContainer, bool includeUsername)
{
	//	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "assembleURL()uri[%s]"),uri.c_str());

	std::string newuri = _rootURI;
	if (!newuri.empty() && newuri[0]!= '/')
		newuri = std::string("/") + newuri;

	if(!newuri.empty() && newuri[newuri.length() -1] != '/')
		newuri += "/";
	newuri += ((!uri.empty() && uri[0] == '/') ? uri.substr(1): uri);

	ZQ::common::URLStr urlst((std::string("http://") + serverIp + newuri).c_str());
	urlst.setPort(_portHTTP);
	std::string path = urlst.getPath();
	if (bContainer && !path.empty() && path[path.length() -1] != '/')
	{
		path +="/";
		urlst.setPath((char*)path.c_str());
	}

	if (includeUsername)
	{
		urlst.setUserName(_username.c_str());
		urlst.setPwd(_password.c_str());
	}

	newuri = urlst.generate();
	if(!newuri.empty() && newuri[newuri.length() -1] == '=')
		newuri = newuri.substr(0, newuri.length() - 1);

	// fixup those "key=&" to "key&"
	size_t pos;
	while (std::string::npos != (pos = newuri.find("=&")))
		newuri.replace(pos, sizeof("=&")-1, "&");

	//	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "assembleURL()newuri[%s]"),newuri.c_str());

	return newuri;
}

std::string CdmiClientBase::getServerIp(bool next)
{
	static size_t idxServer =0;
	if(_portHTTP <= 0 || _portHTTPS <= 0 || _rootURI.empty())
	{

		if(!getSysConfig(_log, _thrdPool, _rootUrl, _flags & 0xffff))
			return "";

		// re-park the idxServer by searching the fetched IP list
		ZQ::common::URLStr urlst(_rootUrl.c_str());
		std::string currentServer = urlst.getHost();

		ZQ::common::MutexGuard g(_lkLogin);
		StrList ipBak = _serverIPs;
		_serverIPs.clear();
		_serverIPs.push_back(currentServer);

		for (idxServer =0; idxServer < ipBak.size(); idxServer++)
		{
			if (0 == currentServer.compare(ipBak[idxServer]))
				continue;
			_serverIPs.push_back(ipBak[idxServer]);
		}

		idxServer = 0;
	}

	ZQ::common::MutexGuard g(_lkLogin);
	if (next && _serverIPs.size() >1)
		idxServer++;

	idxServer %= _serverIPs.size();
	return _serverIPs[idxServer];
}
uint CdmiClientBase::getCurrentServerIdx()
{
	return _currentServerIdx;
}

static std::string methodToStr(const ZQ::common::CURLClient::HTTPMETHOD& method)
{
	switch(method)
	{
	case ZQ::common::CURLClient::HTTP_GET:
		return "GET";
	case ZQ::common::CURLClient:: HTTP_DEL:
		return "DELETE";
	case ZQ::common::CURLClient::HTTP_PUT:
		return "PUT";
	case ZQ::common::CURLClient::HTTP_POST:
		return "POST";
	default:
		return "CUSTOM";
	}
}
#define CDMIRC_CASE(_RC) case _RC: return #_RC
const char* CdmiClientBase::cdmiRetStr(int retCode)
{
	switch(retCode)
	{
		CDMIRC_CASE(cdmirc_OK);
		CDMIRC_CASE(cdmirc_Created);
		CDMIRC_CASE(cdmirc_Accepted);
		CDMIRC_CASE(cdmirc_NoContent);
		CDMIRC_CASE(cdmirc_PartialContent);

		CDMIRC_CASE(cdmirc_Found);

		CDMIRC_CASE(cdmirc_BadRequest);
		CDMIRC_CASE(cdmirc_Unauthorized);
		CDMIRC_CASE(cdmirc_Forbidden);
		CDMIRC_CASE(cdmirc_NotFound);
		CDMIRC_CASE(cdmirc_NotAcceptable);
		CDMIRC_CASE(cdmirc_Conflict);
		CDMIRC_CASE(cdmirc_ServerError);

		CDMIRC_CASE(cdmirc_RequestFailed);
		CDMIRC_CASE(cdmirc_RequestTimeout);
		CDMIRC_CASE(cdmirc_AquaLocation);
		CDMIRC_CASE(cdmirc_RetryFailed);

	default: return "Unknown";
	}
}

static std::string nowToAquaData()
{
	static const int64 stamp1970= ZQ::common::TimeUtil::ISO8601ToTime("1970-01-01T00:00:00Z");
	char buf[64];
	snprintf(buf, sizeof(buf)-2, "%lld", ZQ::common::now() - stamp1970);
	return buf;
}
bool CdmiClientBase::generateSignature(std::string& signature, const std::string& uri, const std::string& contentType, ZQ::common::CURLClient::HTTPMETHOD method, const std::string& xAquaDate)
{
	std::string url="";
	// step 1. fetch secretKeyId
	if (_secretKeyId.empty())
	{
		Json::Value result;
		std::string serverIp = getServerIp();
		if(serverIp.empty())
			return false;
		std::string rootUrl = assembleURL(serverIp, "");
		int cdmiCode = loginIn(result, _log, _thrdPool, rootUrl, _username, _password, _flags & 0xffff,this);
		if(CdmiRet_FAIL(cdmiCode))
		{
			return false;
		}
		try
		{	
			if (!result.isMember("secretAccessKey") || !result.isMember("objectID"))
				return false;

			_secretKeyId = result["secretAccessKey"].asString();
			_accessKeyId = result["objectID"].asString();
		}
		catch(std::exception& ex)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "generateSignature[%s] failed to parse response caught exception[%s]"),url.c_str(), ex.what());
			_secretKeyId = "";
		}
		catch (...)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "generateSignature[%s] failed to parse response caught unknown exception[%d]"),url.c_str(), SYS::getLastErr());
			_secretKeyId = "";
		}
	}

	if (_secretKeyId.empty() || uri.empty() || xAquaDate.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "generateSignature() invalid input parameters secretKey[%s] or uri[%s] or xAquaDate[%s]"),
			 _secretKeyId.c_str(), uri.c_str() , xAquaDate.c_str());
		return false;
	}
	std::string CanonicalizedResource  = uri;
    if(!CanonicalizedResource.empty() && CanonicalizedResource[0] != '/')
		CanonicalizedResource = "/" + CanonicalizedResource;

	std::string strToSign = methodToStr(method) + "\n" + contentType + "\n" + xAquaDate + "\n" + CanonicalizedResource;
	{
		char buf[512]= "";
		snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiClientBase, "generateSignature() stringToSign:"));
		_log.hexDump(ZQ::common::Log::L_DEBUG, strToSign.c_str(), (int)strToSign.size(), buf, 0 == (flgHexDump&_flags));

	}

	unsigned char digestBuf[HMAC_SHA1_DIGEST_LENGTH+1]="";
	hmac_sha1((unsigned char*)strToSign.c_str(), strToSign.size(), (unsigned char*)_secretKeyId.c_str(),  _secretKeyId.size(), digestBuf);
   
	char* pOutBufPtr = NULL;
	size_t outBufLen = 0;
	bool bret = base64Encode((const char*)digestBuf, HMAC_SHA1_DIGEST_LENGTH, &pOutBufPtr, &outBufLen);
	if(!bret)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "generateSignature() failed to encode to base64 with secretKey[%s]"), _secretKeyId.c_str());
		_secretKeyId = "";
		return false;
	}

	signature.clear();
	signature.append(pOutBufPtr, outBufLen);

	try
	{
		if(pOutBufPtr)
			delete [] pOutBufPtr;
		pOutBufPtr = NULL;
	}	
	catch (...){
	}

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiClientBase, "generateSignature secretKey[%s]signature:"), _secretKeyId.c_str());
	_log.hexDump(ZQ::common::Log::L_DEBUG, signature.c_str(), (int)outBufLen, buf, 0 == (flgHexDump&_flags));

//	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiClientBase, "generateSignature secretKey[%s] signature[%s]"),_secretKeyId.c_str(), signature.c_str());

	return true;
}

std::string CdmiClientBase::_domainURI;
std::string CdmiClientBase::_rootURI;
CdmiClientBase::StrList     CdmiClientBase::_serverIPs;
int         CdmiClientBase::_portHTTP; 
int         CdmiClientBase::_portHTTPS;
ZQ::common::Mutex CdmiClientBase::_lkLogin;

int CdmiClientBase::loginIn(Json::Value& result, ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, const std::string& rootUrl, const std::string&userName, const std::string &password, uint16 curlflags, void* pCtx)
{
	ZQ::common::MutexGuard g(_lkLogin);

	ZQ::common::URLStr rooturl(rootUrl.c_str());
	const char* host = rooturl.getHost();
	ZQ::common::URLStr strUrl;
	std::string path = DEFAULT_LOGIN_PATH + userName;
	strUrl.setPath(path.c_str());
	strUrl.setUserName(userName.c_str());
	strUrl.setPwd(password.c_str());
	strUrl.setProtocol("https");
	strUrl.setHost(host);
	//strUrl.setPort(port);
	if(_portHTTPS > 0)
		strUrl.setPort(_portHTTPS);
	else
		strUrl.setPort(8443);

	std::string url = (char*)strUrl.generate();

	int64 lStartTime = ZQ::common::now();
	ZQ::CDMIClient::CDMIHttpClient::Ptr cdmiClient = new ZQ::CDMIClient::CDMIHttpClient((char*)url.c_str(), log, thrdPool, curlflags, ZQ::common::CURLClient::HTTP_GET); 
	if(!cdmiClient)
	{
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}

	if(!cdmiClient->init())
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "login[%s] failed to init libcurl"), url.c_str());
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}

	cdmiClient->setHeader("Accept", CDMI_URER_TYPE);
	cdmiClient->setHeader("X-CDMI-Specification-Version", CDMI_Version);
	if(_domainURI.empty())
		cdmiClient->setHeader("x-aqua-user-domain-uri", "/cdmi_domains/defaultdomainname/");
	else
		cdmiClient->setHeader("x-aqua-user-domain-uri", _domainURI.c_str());

	CURLcode retcode = CURLE_OK;
	bool bRet = cdmiClient->sendRequest(retcode, true);
	if(!bRet)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "login[%s] request failed took %dms"), url.c_str(), (int)(ZQ::common::now() - lStartTime));
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}

	std::string strStatus;
	int cdmiRetCode = cdmiClient->getStatusCode(strStatus);

	if (!CdmiRet_SUCC(cdmiRetCode))
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "login[%s] failed to login: %s(%d)<=[%s]: %s took %dms"),
			url.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), cdmiClient->getErrorMessage().c_str(), (int)(ZQ::common::now() - lStartTime));
		cdmiClient = NULL;
		return cdmiRetCode;
	}

	size_t dataSize = 0;
	//const char* pStrResponse = cdmiClient->getRespBuf(dataSize);
	ZQ::common::BufferList::Ptr resPtr = cdmiClient->getResponseBody();
	std::string strResponse;
	resPtr->readToString(strResponse);
	//strResponse.append(pStrResponse, dataSize);

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiClientBase, "login[%s]: %s(%d)<=[%s] took %dms"), 
		url.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	log.hexDump(ZQ::common::Log::L_INFO, /*pStrResponse*/strResponse.c_str(), (int)/*dataSize*/strResponse.length(), buf, true);

	cdmiClient = NULL;
	//1.1 parser cdmi response boy
	Json::Reader reader;
	try
	{	
		if(!strResponse.empty() && !reader.parse(strResponse, result))
		{
			log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "login[%s] failed to parse response took %dms"),
				url.c_str(), (int)(ZQ::common::now() - lStartTime));
			return cdmirc_RequestFailed;
		}
		return cdmirc_OK;
	}
	catch(std::exception& ex)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "login[%s] failed to parse response caught exception[%s]"),url.c_str(), ex.what());
	}
	catch (...)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "login[%s] failed to parse response caught unknown exception[%d]"),url.c_str(), SYS::getLastErr());
	}

  return cdmirc_RequestFailed;
}
bool CdmiClientBase::getSysConfig(ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool,const std::string& rootUrl, uint16 curlflags)
{
	ZQ::common::MutexGuard g(_lkLogin);

	log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "getSysConfig()populating the server configuration from %s"), rootUrl.c_str());
	// refer to http://192.168.87.16/mediawiki/index.php/Other_-_CDMI_-_High_Level_Design_-_Aqua#Get_configuration
	// to send GET _rootURL "/aqua/rest/cdmi/cdmi_config" to retrieve the configurations
	// into _serverIPs, _portHTTP, _portHTTPS and _domainURI

	_portHTTPS = 0;
	_portHTTP = 0;
	_domainURI.clear();
	_serverIPs.clear();

	std::string url =  rootUrl + "cdmi_config";
	int64 lStartTime = ZQ::common::now();
	ZQ::CDMIClient::CDMIHttpClient::Ptr cdmiClient = new ZQ::CDMIClient::CDMIHttpClient((char*)url.c_str(), log, thrdPool, curlflags, ZQ::common::CURLClient::HTTP_GET); 
	if(!cdmiClient)
	{
		cdmiClient = NULL;
		return false;
	}

	if(!cdmiClient->init())
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "getSysConfig[%s] failed to init libcurl"), url.c_str());
		cdmiClient = NULL;
		return false;
	}

	cdmiClient->setHeader("Accept", CDMI_CONFIG_TYPE);

	CURLcode retcode = CURLE_OK;
	bool bRet = cdmiClient->sendRequest(retcode, true);
	if(!bRet)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "getSysConfig[%s] request failed took %dms"), url.c_str(), (int)(ZQ::common::now() - lStartTime));
		cdmiClient = NULL;
		return false;
	}

	std::string strStatus;
	int cdmiRetCode = cdmiClient->getStatusCode(strStatus);
	if (!CdmiRet_SUCC(cdmiRetCode))
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "getSysConfig[%s] failed to get root URL: %s(%d)<=[%s]: %s took %dms"),
			url.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), cdmiClient->getErrorMessage().c_str(), (int)(ZQ::common::now() - lStartTime));
		cdmiClient = NULL;
		return false;
	}

	size_t dataSize = 0;
	//const char* pStrResponse = cdmiClient->getRespBuf(dataSize);
	ZQ::common::BufferList::Ptr resPtr = cdmiClient->getResponseBody();
	std::string strResponse;
	resPtr->readToString(strResponse);
// 	std::string strResponse;
// 	strResponse.append(pStrResponse, dataSize);

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiClientBase, "getSysConfig[%s]: %s(%d)<=[%s] took %dms"), 
		url.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	log.hexDump(ZQ::common::Log::L_INFO, /*pStrResponse*/strResponse.c_str(), (int)/*dataSize*/strResponse.length(), buf, true);

	cdmiClient = NULL;
	//1.1 parser cdmi response boy
	Json::Reader reader;
	Json::Value result;
	try
	{	
		if(!strResponse.empty() && !reader.parse(strResponse, result))
		{
			log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "getSysConfig[%s] failed to parse response took %dms"),
				url.c_str(), (int)(ZQ::common::now() - lStartTime));
			return false;
		}
		if (!result.isMember("httpport") || !result.isMember("httpsport") || !result.isMember("rootURI") || !result.isMember("defaultDomainURI") || !result.isMember("frontendIPs"))
		{
			log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "getSysConfig[%s]Auqa server missed some parameter"), url.c_str());
			return false;
		}

		_portHTTPS = result["httpsport"].asInt();
		_portHTTP = result["httpport"].asInt();
		_rootURI = result["rootURI"].asString();
		_domainURI = result["defaultDomainURI"].asString();

		std::string frontendIps;
		Json::Value& children = result["frontendIPs"];

		for (Json::Value::iterator itF = children.begin(); itF != children.end(); itF++)
		{
			_serverIPs.push_back((*itF).asString());
			frontendIps += (*itF).asString()+ ";";
		}			
		log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiClientBase, "getSysConfig[%s]rootURI[%s]httpport[%d]httpsPort[%d]defaultDomainURI[%s]frontendIPs[%s] "),
			url.c_str(),_rootURI.c_str(), _portHTTP, _portHTTPS, _domainURI.c_str(), frontendIps.c_str());

		return true;
	}
	catch(std::exception& ex)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "getSysConfig[%s] failed to parse response caught exception[%s]"),url.c_str(), ex.what());
	}
	catch (...)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "getSysConfig[%s] failed to parse response caught unknown exception[%d]"),url.c_str(), SYS::getLastErr());
	}
	return false;
}


CdmiClientBase::CdmiRetCode CdmiClientBase::cdmi_CreateDataObject(Json::Value& result, const std::string& uri, const std::string& mimetype, const Properties& metadata, const std::string& value,
															const StrList& valuetransferencoding,
															const std::string& domainURI, const std::string& deserialize, const std::string& serialize,
															const std::string& copy, const std::string& move, const std::string& reference, 
															const std::string& deserializevalue)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "cdmi_CreateDataObject[%s]"), uri.c_str());

	Json::Value requestParams;
	Json::FastWriter writer;
	Json::Value v;

	// the mimetype
	requestParams["mimetype"] = mimetype.empty() ? DEFAULT_CONTENT_MIMETYPE : mimetype;

	// the metadata
	for (Properties::const_iterator it= metadata.begin(); it!=metadata.end(); it++)
		v[it->first] = it->second;
	if (v.size() >0)
		requestParams["metadata"] = v;
	v.clear();

	SET_OPTIONAL_STR_PARAM(requestParams, "value", value);

	// valuetransferencoding 
	for (StrList::const_iterator itS= valuetransferencoding.begin(); itS <valuetransferencoding.end(); itS++)
		v.append(*itS);
	if (v.size() >0)
		requestParams["valuetransferencoding"] = v;
	v.clear();

	// the URIs	
	SET_OPTIONAL_STR_PARAM(requestParams, "domainURI",   domainURI);
	SET_OPTIONAL_STR_PARAM(requestParams, "deserialize", deserialize);
	SET_OPTIONAL_STR_PARAM(requestParams, "serialize",   serialize);
	SET_OPTIONAL_STR_PARAM(requestParams, "copy",        copy);
	SET_OPTIONAL_STR_PARAM(requestParams, "move",        move);
	SET_OPTIONAL_STR_PARAM(requestParams, "reference",   reference);

	//1.1 sending the request to the server
	result.clear();
	std::string requestBody = writer.write(requestParams);

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "cdmi_CreateDataObject[%s] requestbody [%s]"),uri.c_str(), requestBody.c_str());

	int64 lStartTime = ZQ::common::now();
	std::string finalURL;
	std::string strStatus;
	bool bContainer = false;
	bool includeUsername = false;

	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Accept", CDMI_DATAOBJECT_TYPE);
	MAPSET(Properties, requestHeaders, "Content-Type", CDMI_DATAOBJECT_TYPE);
	MAPSET(Properties, requestHeaders, "X-CDMI-Specification-Version", CDMI_Version);
	MAPSET(Properties, requestHeaders, "X-CDMI-Partial", "false");

	Properties resHeaders;
	std::string strResponse;

	uint buflen = 0;

	int cdmiRetCode = callRemote("cdmi_CreateDataObject", uri, finalURL, strStatus, false, false, requestHeaders, requestBody, NULL, 0,
		resHeaders, strResponse, buflen, NULL,
		ZQ::common::CURLClient::HTTP_PUT,  (_flags & 0xffff) | ZQ::common::CURLClient::sfEnableOutgoingDataCB);

	if(CdmiRet_FAIL(cdmiRetCode))
		return cdmiRetCode;

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiClientBase, "cdmi_CreateDataObject[%s]: %s(%d)<=[%s]took %dms"),
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	_log.hexDump((flgHexDump&_flags) ? ZQ::common::Log::L_DEBUG : ZQ::common::Log::L_INFO, strResponse.c_str(), (int)strResponse.size(), buf, 0 == (flgHexDump&_flags));

   //1.2 parser cdmi response boy
	Json::Reader reader;
	try
	{	
		if(!strResponse.empty() && !reader.parse(strResponse, result))
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "cdmi_CreateDataObject[%s] failed to parse response"),uri.c_str());
			cdmiRetCode =  cdmirc_RequestFailed;
		}
	}
	catch(std::exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "cdmi_CreateDataObject[%s] failed to parse response caught exception[%s]"),uri.c_str(), ex.what());
		cdmiRetCode = cdmirc_RequestFailed;
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "cdmi_CreateDataObject[%s] failed to parse response caught unknown exception[%d]"),uri.c_str(), SYS::getLastErr());
		cdmiRetCode = cdmirc_RequestFailed;
	}

	return cdmiRetCode;
}

CdmiClientBase::CdmiRetCode CdmiClientBase::nonCdmi_CreateDataObject(const std::string& uri, const std::string& contentType, const char* value, uint32 size)
{	
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "nonCdmi_CreateDataObject[%s]"), uri.c_str());

	std::string strConType = contentType.empty() ? DEFAULT_CONTENT_MIMETYPE : contentType; // DEFAULT_MIMETYPE ";charset=utf-8" : contentType;

	int64 lStartTime = ZQ::common::now();
	std::string finalURL;
	std::string strStatus;
	bool bContainer = false;
	bool includeUsername = false;

	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Content-Type", strConType);

	std::string requestBody;
	Properties resHeaders;
	std::string strResponse;

	uint buflen = 0;

	int cdmiRetCode = callRemote("nonCdmi_CreateDataObject", uri, finalURL, strStatus, false, false, requestHeaders, requestBody, (char*)value, size,
		resHeaders, strResponse, buflen, NULL,
		ZQ::common::CURLClient::HTTP_PUT,  (_flags & 0xffff) | ZQ::common::CURLClient::sfEnableOutgoingDataCB);

	if(CdmiRet_FAIL(cdmiRetCode))
		return cdmiRetCode;

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiClientBase, "nonCdmi_CreateDataObject[%s]: %s(%d)<=[%s] took %dms"), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	_log.hexDump((flgHexDump&_flags) ? ZQ::common::Log::L_DEBUG : ZQ::common::Log::L_INFO, strResponse.c_str(), (int)strResponse.size(), buf, 0 == (flgHexDump&_flags));

	return cdmiRetCode;
}

CdmiClientBase::CdmiRetCode CdmiClientBase::cdmi_ReadDataObject(Json::Value& result, const std::string& uri, std::string& location)
{	
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "cdmi_ReadDataObject[%s]"), uri.c_str());
//////////////////////////////////
	std::string finalURL;
	Properties requestHeaders, respHeaders;
	MAPSET(Properties, requestHeaders, "Accept",       CDMI_DATAOBJECT_TYPE); ///Optional
	MAPSET(Properties, requestHeaders, "Content-Type", CDMI_DATAOBJECT_TYPE);///Mandatory

	std::string requestBody;
	std::string strResponse, respStatus;
	uint buflen=0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("cdmi_ReadDataObject", uri, finalURL, respStatus, false, false,
							 requestHeaders, requestBody, NULL, 0,
							 respHeaders, strResponse, buflen, NULL,
							 ZQ::common::CURLClient::HTTP_GET, _flags &0xffff);

	if (!CdmiRet_SUCC(cdmiRetCode))
		return (CdmiClientBase::CdmiRetCode) cdmiRetCode;

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiClientBase, "cdmi_ReadDataObject[%s]: %s(%d)<=[%s] took %dms"), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, respStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	_log.hexDump((flgHexDump&_flags) ? ZQ::common::Log::L_DEBUG : ZQ::common::Log::L_INFO, strResponse.c_str(), (int)strResponse.size(), buf, 0 == (flgHexDump&_flags));

	if(respHeaders.find("Location") != respHeaders.end())
		location = respHeaders["Location"];

	//1.2 parser cdmi response boy
	Json::Reader reader;
	try
	{	
		if(!reader.parse(strResponse, result))
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "cdmi_ReadDataObject[%s] failed to paser response"),finalURL.c_str());
			return cdmirc_RequestFailed;
		}
	}
	catch(std::exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "cdmi_ReadDataObject[%s] failed to parse response caught exception[%s]"), finalURL.c_str(), ex.what());
		cdmiRetCode = cdmirc_RequestFailed;
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "cdmi_ReadDataObject[%s] failed to parse response caught unknown exception[%d]"), finalURL.c_str(), SYS::getLastErr());
		cdmiRetCode = cdmirc_RequestFailed;
	}

	return cdmiRetCode;
}

CdmiClientBase::CdmiRetCode CdmiClientBase::nonCdmi_ReadDataObject(const std::string& uri, std::string& contentType,
													   std::string& location,
													   uint64 startOffset, in out uint& len, char* recvbuff)
{	
	return nonCdmi_ReadDataObject_direct(uri,contentType, location,startOffset,len,recvbuff);
}

CdmiClientBase::CdmiRetCode CdmiClientBase::nonCdmi_ReadDataObject_direct(const std::string& uri, std::string& contentType, std::string& location, uint64 startOffset, in out uint& len, char* recvbuff)
{  
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "nonCdmi_ReadDataObject[%s]"), uri.c_str());

	if (NULL == recvbuff || len <=0)
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiClientBase, "nonCdmi_ReadDataObject[%s], illegal recvbuf w/ len[%u]"), uri.c_str(), len);
		return cdmirc_RequestFailed;
	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiClientBase, "nonCdmi_ReadDataObject[%s], offset[%lld] len[%u]"), uri.c_str(), startOffset, len);

	uint recBufSize = len;

	int64 lStartTime = ZQ::common::now();
	std::string finalURL;
	std::string strStatus;
	bool bContainer = false;
	bool includeUsername = false;

	Properties requestHeaders;
	char strRang[100] ="", *p = strRang;
	snprintf(p, sizeof(strRang) -1, "bytes=%llu-", startOffset); p += strlen(p);
	if (len >0)
		snprintf(p, strRang + sizeof(strRang) -p -1, "%llu", startOffset+len-1);
	MAPSET(Properties, requestHeaders, "Range", strRang);

	std::string requestBody;

	Properties resHeaders;
	std::string strResponse;

	uint buflen = 0;

	int cdmiRetCode = callRemote("nonCdmi_ReadDataObject", uri, finalURL, strStatus, false, false, requestHeaders, requestBody, NULL, 0,
		resHeaders, strResponse, buflen, NULL,
		ZQ::common::CURLClient::HTTP_GET, _flags &0xffff);

	if(CdmiRet_FAIL(cdmiRetCode))
		return cdmiRetCode;

	int elapsedTime = (int)(ZQ::common::now() - lStartTime);

	if (flgDumpMsgBody & _flags)
	{
		char buf[512]= "";
		snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiClientBase, "nonCdmi_ReadDataObject[%s]: "), finalURL.c_str());
		_log.hexDump(ZQ::common::Log::L_DEBUG, strResponse.c_str(), (int)strResponse.size(), buf, 0 == (flgHexDump&_flags));
	}

	if (resHeaders.find("Location") != resHeaders.end())
		location = resHeaders["Location"];
	if (resHeaders.find("Content-Type") != resHeaders.end())
		contentType = resHeaders["Content-Type"];

	uint contentLen = (uint)strResponse.size();

	if( len > strResponse.size())
		len = (uint)strResponse.size();

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiClientBase, "nonCdmi_ReadDataObject[%s] took %dms: %s(%d)<=[%s] len(%d/%d/%d) requestRange(%s)"),
		finalURL.c_str(), elapsedTime, cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), len, contentLen, recBufSize,strRang);

    memcpy(recvbuff, (char*)strResponse.c_str(), len);

	return cdmiRetCode;
}

CdmiClientBase::CdmiRetCode CdmiClientBase::cdmi_UpdateDataObject(out std::string& location, const std::string& uri, const Properties& metadata,
		uint64 startOffset, const std::string& value, bool partial, const StrList& valuetransferencoding,
		const std::string& domainURI, const std::string& deserialize, const std::string& copy, const std::string& deserializevalue)
{
	Json::Value jMetadata;
	for (Properties::const_iterator it= metadata.begin(); it!=metadata.end(); it++)
		jMetadata[it->first] = it->second;

	return CdmiClientBase::cdmi_UpdateDataObjectEx(location, uri, jMetadata,
		startOffset, value, -1, partial, valuetransferencoding,
		domainURI, deserialize, copy, deserializevalue);
}


CdmiClientBase::CdmiRetCode CdmiClientBase::cdmi_UpdateDataObjectEx(out std::string& location, const std::string& uri, const Json::Value& metadata,
		uint64 startOffset, const std::string& value, int base_version, bool partial, const StrList& valuetransferencoding,
		const std::string& domainURI, const std::string& deserialize, const std::string& copy, const std::string& deserializevalue)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "cdmi_UpdateDataObjectEx[%s]"), uri.c_str());

	Json::Value requestParams;
	Json::FastWriter writer;
	Json::Value v;

	// the mimetype
	requestParams["mimetype"] = DEFAULT_CONTENT_MIMETYPE ;

	// the metadata
	/*
	for (Properties::const_iterator it= metadata.begin(); it!=metadata.end(); it++)
		v[it->first] = it->second;
	if (v.size() >0)
		requestParams["metadata"] = v;
	v.clear();
	*/
	if (metadata.size() >0)
		requestParams["metadata"] = metadata;

	SET_OPTIONAL_STR_PARAM(requestParams, "value", value);

	// valuetransferencoding 
	for (StrList::const_iterator itS= valuetransferencoding.begin(); itS <valuetransferencoding.end(); itS++)
		v.append(*itS);
	if (v.size() >0)
		requestParams["valuetransferencoding"] = v;
	v.clear();

	// the URIs
	SET_OPTIONAL_STR_PARAM(requestParams, "domainURI",   domainURI);
	SET_OPTIONAL_STR_PARAM(requestParams, "deserialize",      deserialize);
	SET_OPTIONAL_STR_PARAM(requestParams, "copy",             copy);
	SET_OPTIONAL_STR_PARAM(requestParams, "deserializevalue", deserializevalue);

	//1.1 sending the request to the server
	std::string requestBody = writer.write(requestParams);
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "cdmi_UpdateDataObjectEx[%s] requestbody [%s]"),uri.c_str(), requestBody.c_str());

	std::string url;
	Properties requestHeaders, respHeaders;
	MAPSET(Properties, requestHeaders, "Content-Type",   CDMI_DATAOBJECT_TYPE);///Mandatory
	MAPSET(Properties, requestHeaders, "X-CDMI-Partial", partial == true? "true":"false");///Optional

	if (base_version >=0)
	{
		// http://192.168.87.16/mediawiki/index.php/Specify_object/container_version_-_CDMI_-_Function_Design_-_Aqua
		char verstr[10];
		snprintf(verstr, sizeof(verstr), "%d", base_version);
		MAPSET(Properties, requestHeaders, "x-aqua-verify-version", verstr);///Optional
	}

	std::string strResponse, strStatus;
	uint buflen=0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("cdmi_UpdateDataObjectEx", uri, url, strStatus, false, false,
							 requestHeaders, requestBody, NULL, 0,
							 respHeaders, strResponse, buflen, NULL,
							 ZQ::common::CURLClient::HTTP_PUT, (_flags & 0xffff) | ZQ::common::CURLClient::sfEnableOutgoingDataCB);

	if (!CdmiRet_SUCC(cdmiRetCode))
		return (CdmiClientBase::CdmiRetCode) cdmiRetCode;

	if(respHeaders.find("Location") != respHeaders.end())
		location = respHeaders["Location"];

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiClientBase, "cdmi_UpdateDataObjectEx[%s]: %s(%d)<=[%s] took %dms, loc[%s]"), 
		url.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime), location.c_str());

	// cdmiClient = NULL;
	return cdmiRetCode;
}

CdmiClientBase::CdmiRetCode CdmiClientBase::nonCdmi_UpdateDataObject(const std::string& uri, out std::string& location, const std::string& contentType, uint64 startOffset, uint len, const char* buff, int64 objectSize, bool partial)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "nonCdmi_UpdateDataObject[%s]: %ubytes at offset[%llu]"), uri.c_str(), len, startOffset);
    
	if (!buff && len>0)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "nonCdmi_UpdateDataObject[%s]: value is null "), uri.c_str());
		return cdmirc_RequestFailed;
	}

	int64 lStartTime = ZQ::common::now();
	std::string finalURL;
	std::string strStatus;
	bool bContainer = false;
	bool includeUsername = false;

	Properties requestHeaders;

	if (len <=0)
	{
		if (objectSize >=0)
		{
			char sizestr[30];
			snprintf(sizestr, sizeof(sizestr)-2, "%llu", objectSize);
			MAPSET(Properties, requestHeaders, "x-aqua-file-truncate", sizestr);

		}
	}
	else
	{
		char strRang[256]="", *p = strRang;
		snprintf(p, sizeof(strRang) -1," bytes="FMT64 "-" FMT64, startOffset, (uint64)((len > 0) ? (startOffset + len-1) : startOffset)); p += strlen(p);
		if (objectSize >=0)
			snprintf(p, strRang + sizeof(strRang) -p -1, "/" FMT64, objectSize);
		//	else
		//		snprintf(p, strRang + sizeof(strRang) -p -1, "/?");

		MAPSET(Properties, requestHeaders, "Content-Range", strRang);
	}
	MAPSET(Properties, requestHeaders, "Content-Type", contentType);
	MAPSET(Properties, requestHeaders, "X-CDM-Partial", partial == true ? "true":"false");

	std::string requestBody;
	Properties respHeaders;
	std::string strResponse;

	uint buflen = 0;

	int cdmiRetCode = callRemote("nonCdmi_UpdateDataObject", uri, finalURL, strStatus, false, false, requestHeaders, requestBody, (char*)buff, len,
		respHeaders, strResponse, buflen, NULL,
		ZQ::common::CURLClient::HTTP_PUT,  (_flags & 0xffff) | ZQ::common::CURLClient::sfEnableOutgoingDataCB);

	if(CdmiRet_FAIL(cdmiRetCode))
		return cdmiRetCode;

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiClientBase, "nonCdmi_UpdateDataObject[%s]: %s(%d)<=[%s] took %dms"), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));

	if(respHeaders.find("Location") != respHeaders.end())
		location = respHeaders["Location"];

	return cdmiRetCode;
}

CdmiClientBase::CdmiRetCode CdmiClientBase::cdmi_DeleteDataObject(Json::Value& result, const std::string& uri)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "cdmi_DeleteDataObject[%s]"), uri.c_str());

	int64 lStartTime = ZQ::common::now();
	std::string finalURL;
	std::string strStatus;
	bool bContainer = false;
	bool includeUsername = false;

	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Content-Type", CDMI_DATAOBJECT_TYPE);

	std::string requestBody;
	Properties respHeaders;
	std::string strResponse;

	uint buflen = 0;

	int cdmiRetCode = callRemote("cdmi_DeleteDataObject", uri, finalURL, strStatus, false, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, strResponse, buflen, NULL,
		ZQ::common::CURLClient::HTTP_DEL,  (_flags & 0xffff));

	if(CdmiRet_FAIL(cdmiRetCode))
		return cdmiRetCode;

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiClientBase, "cdmi_DeleteDataObject[%s]: %s(%d)<=[%s] took %dms"), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));

	return cdmiRetCode;
}

CdmiClientBase::CdmiRetCode CdmiClientBase::nonCdmi_DeleteDataObject(const std::string& uri)
{	
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "nonCdmi_DeleteDataObject[%s]"), uri.c_str());

	int64 lStartTime = ZQ::common::now();
	std::string finalURL;
	std::string strStatus;
	bool bContainer = false;
	bool includeUsername = false;

	Properties requestHeaders;
	std::string requestBody;
	Properties respHeaders;
	std::string strResponse;

	uint buflen = 0;

	int cdmiRetCode = callRemote("nonCdmi_DeleteDataObject", uri, finalURL, strStatus, false, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, strResponse, buflen, NULL,
		ZQ::common::CURLClient::HTTP_DEL, (_flags & 0xffff));

	if(CdmiRet_FAIL(cdmiRetCode))
		return cdmiRetCode;

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiClientBase, "nonCdmi_DeleteDataObject[%s]: %s(%d)<=[%s] took %dms"), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));

	return cdmiRetCode;
}

///////////////////////////////////////////////////////////////////////////////////////////////
CdmiClientBase::CdmiRetCode CdmiClientBase::cdmi_CreateContainer(Json::Value& result, const std::string& uri, const Properties& metadata,
														   const Json::Value& exports,
														   const std::string& domainURI, const std::string& deserialize, 
														   const std::string& copy, const std::string& move, const std::string& reference, 
														   const std::string& deserializevalue)
{
	//	std::string url = generateURL(uri, true);

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "cdmi_CreateContainer[%s]"), uri.c_str());
	Json::Value requestParams;
	Json::FastWriter writer;
	Json::Value v;

	// the metadata
	for (Properties::const_iterator it= metadata.begin(); it!=metadata.end(); it++)
		v[it->first] = it->second;
	if (v.size() >0)
		requestParams["metadata"] = v;
	v.clear();

	// exprots
	if (exports.size() >0)
		requestParams["exports"] = exports;

	// the URIs	
	SET_OPTIONAL_STR_PARAM(requestParams, "domainURI",   domainURI);
	SET_OPTIONAL_STR_PARAM(requestParams, "deserialize", deserialize);
	SET_OPTIONAL_STR_PARAM(requestParams, "copy",        copy);
	SET_OPTIONAL_STR_PARAM(requestParams, "move",        move);
	SET_OPTIONAL_STR_PARAM(requestParams, "reference",   reference);
	SET_OPTIONAL_STR_PARAM(requestParams, "deserializevalue",   deserializevalue);

	//1.1 sending the request to the server
	result.clear();
	std::string requestBody = writer.write(requestParams);
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "cdmi_CreateContainer[%s] requestbody [%s]"),uri.c_str(), requestBody.c_str());

	std::string finalURL;
	std::string strStatus;

	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Accept", CDMI_CONTAINER_TYPE);
	MAPSET(Properties, requestHeaders, "Content-Type", CDMI_CONTAINER_TYPE);

	Properties respHeaders;
	std::string strResponse;
	uint buflen = 0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("cdmi_CreateContainer", uri, finalURL, strStatus, true, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, strResponse, buflen,NULL,
		ZQ::common::CURLClient::HTTP_PUT, (_flags & 0xffff)| ZQ::common::CURLClient::sfEnableOutgoingDataCB);

	if(CdmiRet_FAIL(cdmiRetCode))
		return  (CdmiClientBase::CdmiRetCode)cdmiRetCode;

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiClientBase, "cdmi_CreateContainer[%s]: %s(%d)<=[%s] took %dms"),
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	_log.hexDump((flgHexDump&_flags) ? ZQ::common::Log::L_DEBUG : ZQ::common::Log::L_INFO, strResponse.c_str(), (int)strResponse.size(), buf, 0 == (flgHexDump&_flags));

	//1.2 parser cdmi response boy
	Json::Reader reader;

	try
	{	
		if(!strResponse.empty() && !reader.parse(strResponse, result))
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "cdmi_CreateContainer[%s] failed to parse response"),finalURL.c_str());
			return cdmirc_RequestFailed;
		}

	}
	catch(std::exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "cdmi_CreateContainer[%s] failed to parse response caught exception[%s]"),finalURL.c_str(), ex.what());
		cdmiRetCode = cdmirc_RequestFailed;
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "cdmi_CreateContainer[%s] failed to parse response caught unknown exception[%d]"),finalURL.c_str(), SYS::getLastErr());
		cdmiRetCode = cdmirc_RequestFailed;
	}
	return cdmiRetCode;
}

CdmiClientBase::CdmiRetCode CdmiClientBase::nonCdmi_CreateContainer(const std::string& uri)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "nonCdmi_CreateContainer[%s]"), uri.c_str());

	std::string finalURL;
	std::string strStatus;

	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Expect", "");
	MAPSET(Properties, requestHeaders, "Transfer-Encoding", "");

	std::string requestBody;

	Properties respHeaders;
	std::string responseTxtBody;
	uint buflen = 0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("nonCdmi_CreateContainer", uri, finalURL, strStatus, true, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, responseTxtBody, buflen,NULL,
		ZQ::common::CURLClient::HTTP_PUT, (_flags & 0xffff) | ZQ::common::CURLClient::sfEnableOutgoingDataCB);

	if(CdmiRet_FAIL(cdmiRetCode))
		return  (CdmiClientBase::CdmiRetCode)cdmiRetCode;

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiClientBase, "nonCdmi_CreateContainer[%s]: %s(%d)<=[%s] took %dms"), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));

	return (CdmiClientBase::CdmiRetCode) cdmiRetCode;
}

CdmiClientBase::CdmiRetCode CdmiClientBase::cdmi_ReadContainer(Json::Value& result, const std::string& uri)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "cdmi_ReadContainer[%s]"), uri.c_str());

	std::string finalURL;
	std::string strStatus;

	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Accept", CDMI_CONTAINER_TYPE);
	MAPSET(Properties, requestHeaders, "Content-Type", CDMI_CONTAINER_TYPE);

	std::string requestBody;
	Properties respHeaders;
	std::string strResponse;
	uint buflen = 0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("cdmi_ReadContainer", uri, finalURL, strStatus, true, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, strResponse, buflen,NULL,
		ZQ::common::CURLClient::HTTP_GET, (_flags & 0xffff));

	if(CdmiRet_FAIL(cdmiRetCode))
		return  (CdmiClientBase::CdmiRetCode)cdmiRetCode;

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiClientBase, "cdmi_ReadContainer[%s] %s(%d)<=[%s] took %dms: "), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	_log.hexDump((flgHexDump&_flags) ? ZQ::common::Log::L_DEBUG : ZQ::common::Log::L_INFO, strResponse.c_str(), (int)strResponse.size(), buf, 0 == (flgHexDump&_flags));

	//	cdmiClient = NULL;
	//1.2 parser cdmi response boy
	Json::Reader reader;
	try
	{	
		if(!strResponse.empty() && !reader.parse(strResponse, result))
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "cdmi_ReadContainer[%s] failed to parse response"),finalURL.c_str());
			return cdmirc_RequestFailed;
		}

	}
	catch(std::exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "cdmi_ReadContainer[%s] failed to parse response caught exception[%s]"),finalURL.c_str(), ex.what());
		cdmiRetCode = cdmirc_RequestFailed;
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "cdmi_ReadContainer[%s] failed to parse response caught unknown exception[%d]"),finalURL.c_str(), SYS::getLastErr());
		cdmiRetCode = cdmirc_RequestFailed;
	}

	{
		ZQ::common::MutexGuard g(_lkLocationCache); // just borrow a lock
		if (_domainURIOfMP.empty() && result.isMember("domainURI"))
			_domainURIOfMP = result["domainURI"].asString();
	}

	if (strResponse.size() > 600 && result.isMember("children"))
	{
		std::string childrenlist;
		Json::Value& children = result["children"];
		for (Json::Value::iterator itF = children.begin(); itF != children.end(); itF++)
			childrenlist += (*itF).asString() +",";
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "cdmi_ReadContainer() children: %s"), childrenlist.c_str());
	}

	return cdmiRetCode;
}

CdmiClientBase::CdmiRetCode CdmiClientBase::cdmi_UpdateContainerEx(Json::Value& result, out std::string& location,
														   const std::string& uri, const Json::Value& metadata,	const Json::Value& exports,
																 const std::string& domainURI, const std::string& deserialize, const std::string& copy,
																 const std::string& snapshot, const std::string& deserializevalue)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "cdmi_UpdateContainer[%s]"), uri.c_str());
	Json::Value requestParams;
	Json::FastWriter writer;

/*	Json::Value v;

	// the metadata
	for (Properties::const_iterator it= metadata.begin(); it!=metadata.end(); it++)
		v[it->first] = it->second;	

		if (v.size() >0)
		requestParams["metadata"] = v;

		v.clear();
*/

	if (metadata.size() >0)
		requestParams["metadata"] = metadata;

	if (exports.size() > 0)
		requestParams["exports"] = exports;
	// the URIs
	SET_OPTIONAL_STR_PARAM(requestParams, "domainURI",		  domainURI);
	SET_OPTIONAL_STR_PARAM(requestParams, "deserialize",      deserialize);
	SET_OPTIONAL_STR_PARAM(requestParams, "copy",             copy);
	SET_OPTIONAL_STR_PARAM(requestParams, "snapshot",         snapshot);
	SET_OPTIONAL_STR_PARAM(requestParams, "deserializevalue", deserializevalue);

	//1.1 sending the request to the server
	result.clear();
	std::string requestBody = writer.write(requestParams);
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "cdmi_UpdateContainer[%s] requestbody [%s]"),uri.c_str(), requestBody.c_str());

	std::string finalURL;
	std::string strStatus;

	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Accept", CDMI_CONTAINER_TYPE);
	MAPSET(Properties, requestHeaders, "Content-Type", CDMI_CONTAINER_TYPE);

	Properties respHeaders;
	std::string strResponse;
	uint buflen = 0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("cdmi_UpdateContainer", uri, finalURL, strStatus, true, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, strResponse, buflen,NULL,
		ZQ::common::CURLClient::HTTP_PUT, (_flags & 0xffff) | ZQ::common::CURLClient::sfEnableOutgoingDataCB);

	if(CdmiRet_FAIL(cdmiRetCode))
		return  (CdmiClientBase::CdmiRetCode)cdmiRetCode;

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiClientBase, "cdmi_UpdateContainer[%s]: %s(%d)<=[%s] took %dms"), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	_log.hexDump((flgHexDump&_flags) ? ZQ::common::Log::L_DEBUG : ZQ::common::Log::L_INFO, strResponse.c_str(), (int)strResponse.size(), buf, 0 == (flgHexDump&_flags));

	if(respHeaders.find("Location") != respHeaders.end())
		location = respHeaders["Location"];

	//1.2 parser cdmi response boy
	Json::Reader reader;
	try
	{	
		if(!strResponse.empty() && !reader.parse(strResponse, result))
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "cdmi_UpdateContainer[%s] failed to parse response took %dms"),
				finalURL.c_str(), (int)(ZQ::common::now() - lStartTime));
			return cdmirc_RequestFailed;
		}
	}
	catch(std::exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "cdmi_UpdateContainer[%s] failed to parse response caught exception[%s]"),finalURL.c_str(), ex.what());
		cdmiRetCode = cdmirc_RequestFailed;
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "cdmi_UpdateContainer[%s] failed to parse response caught unknown exception[%d]"),finalURL.c_str(), SYS::getLastErr());
		cdmiRetCode = cdmirc_RequestFailed;
	}
	return cdmiRetCode;
}

CdmiClientBase::CdmiRetCode CdmiClientBase::cdmi_UpdateContainer(Json::Value& result, out std::string& location,
														   const std::string& uri, const Properties& metadata,	const Json::Value& exports,
														   const std::string& domainURI, const std::string& deserialize, const std::string& copy,
														   const std::string& snapshot, const std::string& deserializevalue)
{
	Json::Value jMetadata;
	for (Properties::const_iterator it= metadata.begin(); it!=metadata.end(); it++)
		jMetadata[it->first] = it->second;

	return CdmiClientBase::cdmi_UpdateContainerEx(result, location, uri, jMetadata,
		exports, domainURI, deserialize, copy, snapshot, deserializevalue);
}

CdmiClientBase::CdmiRetCode CdmiClientBase::cdmi_DeleteContainer(Json::Value& result, const std::string& uri)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "cdmi_DeleteContainer[%s]"), uri.c_str());

	std::string finalURL;
	std::string strStatus;

	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Content-Type", CDMI_CONTAINER_TYPE);

	std::string requestBody;
	Properties respHeaders;
	std::string strResponse;
	uint buflen = 0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("cdmi_DeleteContainer", uri, finalURL, strStatus, true, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, strResponse, buflen,NULL,
		ZQ::common::CURLClient::HTTP_DEL, (_flags & 0xffff));

	if(CdmiRet_FAIL(cdmiRetCode))
		return  (CdmiClientBase::CdmiRetCode)cdmiRetCode;

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiClientBase, "cdmi_DeleteContainer[%s]: %s(%d)<=[%s] took %dms"), finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	return cdmiRetCode;
}

CdmiClientBase::CdmiRetCode CdmiClientBase::nonCdmi_DeleteContainer(const std::string& uri)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "nonCdmi_DeleteContainer[%s]"), uri.c_str());

	std::string finalURL;
	std::string strStatus;
	Properties requestHeaders;
	std::string requestBody;
	Properties respHeaders;
	std::string responseTxtBody;
	uint buflen = 0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("nonCdmi_DeleteContainer", uri, finalURL, strStatus, true, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, responseTxtBody, buflen,NULL,
		ZQ::common::CURLClient::HTTP_DEL, (_flags & 0xffff));
	if(CdmiRet_FAIL(cdmiRetCode))
		return  (CdmiClientBase::CdmiRetCode)cdmiRetCode;

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiClientBase, "nonCdmi_DeleteContainer[%s]: %s(%d)<=[%s] took %dms"), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));

	return (CdmiClientBase::CdmiRetCode) cdmiRetCode;
}

typedef enum _TryStage
{
	TryStage_PreCached,
	TryStage_CurrentRootURL,
	TryStage_NextServerIP,
	TryStage_Redirected,
	TryStage_GivenUp,
	TryStage_MAX
} TryStage;

static const char* RestryStageName(int tryStage)
{
#define CASE_STAGE(_STAGE_CODE) case TryStage_##_STAGE_CODE: return #_STAGE_CODE
	switch(tryStage)
	{
		CASE_STAGE(PreCached);
		CASE_STAGE(CurrentRootURL);
		CASE_STAGE(NextServerIP);
		CASE_STAGE(Redirected);
	default:
		CASE_STAGE(GivenUp);
	}
}
bool CdmiClientBase::readLocationFromCache(const std::string path, ResourceLocation& resLoc)
{
	std::string key = path;
	if (!key.empty() && key[key.length() -1] == '/')
		key = key.substr(0, key.length() -1);

	ZQ::common::MutexGuard g(_lkLocationCache);
	LocationCache::iterator it = _locationCache.find(key);
	if (_locationCache.end() ==it)
		return false;

	resLoc = it->second;
	return true;
}

void CdmiClientBase::cacheLocation(const ResourceLocation& resLoc)
{
	std::string key = resLoc.path;
	if (!key.empty() && key[key.length() -1] == '/')
		key = key.substr(0, key.length() -1);

	ZQ::common::MutexGuard g(_lkLocationCache);
	_locationCache[key] = resLoc;
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "cacheLocation()[%s]=>[%s]:[%s]"), key.c_str(), resLoc.locationIp.c_str(), resLoc.paramsAppended.c_str());
}

void CdmiClientBase::uncacheLocation(const std::string path)
{
	ZQ::common::MutexGuard g(_lkLocationCache);
	_locationCache.erase(path);
}
std::string CdmiClientBase::getRelocatedURL(std::string& uri, const ResourceLocation& loc, bool bContainer, bool includeUsername)
{
	if (std::string::npos == uri.find(loc.paramsAppended))
	{
		char* leadingDelimitor = "&";
		if (std::string::npos == uri.find_last_of("?"))
			leadingDelimitor = "?";

		uri += std::string(leadingDelimitor) + loc.paramsAppended;
	}

	return assembleURL(loc.locationIp, uri, bContainer, includeUsername);
}
int CdmiClientBase::callRemote(const char* purposeStr, in std::string uri, out std::string& finalURL, out std::string& statusLine, bool bContainer, bool includeUsername,
							 const Properties& requestHeaders, const std::string& requestBody, const char* reqBodyBuf, int reqBodyLen,
							 Properties& responseHeaders, std::string& responseTxtBody, in out uint& len, char* recvbuff,
							 ZQ::common::CURLClient::HTTPMETHOD method, int clientFlags)
{
	std::string path_original = uri, param_original, path;
	size_t pos = uri.find_first_of("?#");
	if (std::string::npos != pos)
	{
		path_original = uri.substr(0, pos);
		param_original = uri.substr(pos);
	}

	if (bContainer && !path_original.empty() && path_original[path_original.length() -1] != '/')
		path_original += "/";

	std::string startServerIp, serverIp;
	int cdmiRetCode = cdmirc_RetryFailed;

	ResourceLocation loc;
	int nextTryStage = TryStage_PreCached;
    ZQ::CDMIClient::CDMIHttpClient::Ptr cdmiClient;
	int64 lTxnStartTime = ZQ::common::now(), lStartTime=lTxnStartTime;
	const char* stageName = RestryStageName(TryStage_GivenUp);

	for (int retry=1; TryStage_GivenUp != nextTryStage && retry < TryStage_MAX; retry++) // maximally try 3 times
	{
		stageName = RestryStageName(nextTryStage);
		lStartTime = ZQ::common::now();
		if (lStartTime > lTxnStartTime + _timeout)
		{
			// quit the loop if the retries in sum had exceeded the timeout
			char stampBuf[64];
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "path[%s] retries in sum had exceeded timeout: currentTry[%d@%s], txnStart[%s]+to[%d]msec"), path.c_str(),
				retry, stageName, ZQ::common::TimeUtil::TimeToUTC(lTxnStartTime, stampBuf, sizeof(stampBuf)-2, true), _timeout);
			cdmiRetCode = cdmirc_RetryFailed;
			break; 
		}

		// step 1. determin the various uri and url per the different try stages
		switch(nextTryStage)
		{
		case TryStage_PreCached: // try the location cache
			nextTryStage = TryStage_CurrentRootURL;
			if (readLocationFromCache(path_original, loc))
			{
				// adjust the uri and url with cached information
				uri = path_original + param_original;
				finalURL = getRelocatedURL(uri, loc, bContainer, includeUsername);
				break; // of switch
			}
			// NOTE: no "break;" if no cache found here

		case TryStage_CurrentRootURL:
			stageName = RestryStageName(TryStage_CurrentRootURL); // because could be continued from TryStage_PreCached

			nextTryStage = TryStage_NextServerIP;

			// previous cached location must became invalid when reach here, get rid of it
			uncacheLocation(path_original);

			// remember where we start with the server ip
			startServerIp = serverIp = getServerIp();

			if(serverIp.empty())
				return cdmirc_RequestFailed;
			// compose the new uri and url with the current url
			uri = path_original + param_original;
			finalURL = assembleURL(serverIp, uri, bContainer, includeUsername);
			break;

		case TryStage_NextServerIP:
			serverIp = getServerIp(true);
			if(serverIp.empty())
			   return cdmirc_RequestFailed;
			// if all the server IPs has been tried, give up the retry
			if (serverIp == startServerIp)
			{
				nextTryStage = TryStage_GivenUp;
				continue;
			}

			uri = path_original + param_original;
			finalURL = assembleURL(serverIp, uri, bContainer, includeUsername);

			break;

		case TryStage_Redirected:

			nextTryStage = TryStage_GivenUp;

			if (!readLocationFromCache(path_original, loc) || loc.locationIp.empty())
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "path[%s]can't find locationIp from Cache()"), path.c_str());
				continue; // should be recently cached, if no, move the next stage = TryStage_GivenUp
			}

			// compose the new uri and url up to the location
			uri = path_original + param_original;
			finalURL = getRelocatedURL(uri, loc, bContainer, includeUsername);

			break;

		case TryStage_GivenUp:
		default:
			//TODO: log

			// no thing to do but get out the retry loop
			nextTryStage = TryStage_GivenUp;
			cdmiRetCode = cdmirc_RetryFailed;
			continue; 
		}

		// step 2.0 generate the signature
		std::string contentType="";
		Properties::const_iterator itHd = requestHeaders.find("Content-Type");
		if (requestHeaders.end() != itHd)
			contentType = itHd->second;
		std::string signature, xAquaDate = nowToAquaData();

		ZQ::common::URLStr tempURL((char*)finalURL.c_str());
		std::string path = tempURL.getPath();

		if(!generateSignature(signature, path, contentType, method, xAquaDate))
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "cdmi_CreateDataObject[%s] failed to generate Signature"), finalURL.c_str());
			return cdmirc_RequestFailed;
		}

		// step 2. create the client and compose the outgoing request
		cdmiClient = new ZQ::CDMIClient::CDMIHttpClient((char*)finalURL.c_str(), _log, _thrdPool, clientFlags, method); 
		if (!cdmiClient)
		{
			cdmiClient = NULL;
			return cdmirc_RequestFailed;
		}

		cdmiClient->setTimeout(_connectTimeout, _timeout);

		if (!cdmiClient->init())
		{		
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "%s[%s] failed to init libcurl"), purposeStr, finalURL.c_str());
			cdmiClient = NULL;
			return cdmirc_RequestFailed;
		}

		// 2.1 apply the request headers
		for (Properties::const_iterator it= requestHeaders.begin(); it != requestHeaders.end(); it++)
		{
			if (it->first.empty())
				continue;
			cdmiClient->setHeader(it->first.c_str(), it->second.c_str());
		}

		// 2.2 apply the signature
		cdmiClient->setHeader("Authorization" , std::string("AQUA ") + _accessKeyId + ":" + signature);
		cdmiClient->setHeader("x-aqua-date", xAquaDate);

		// 2.3 other common headers
		cdmiClient->setHeader("X-CDMI-Specification-Version",CDMI_Version);

		// 2.4 apply the request body
		if (NULL == reqBodyBuf || reqBodyLen <=0)
		{
			reqBodyBuf = requestBody.c_str();
			reqBodyLen = requestBody.size();
		}
		ZQ::common::BufferList::Ptr  reqBufPtr = new ZQ::common::BufferList ();;
		char* bodyBuffer = const_cast<char*> (reqBodyBuf);
		size_t bufSize = reqBufPtr->join((uint8*)bodyBuffer, reqBodyLen);

		//if (reqBodyLen >0 && !cdmiClient->setReqData(reqBodyBuf, reqBodyLen))
		if (bufSize >0 && !cdmiClient->setRequestBody(reqBufPtr))
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "%s[%s]@%s failed to set request boby"), purposeStr, finalURL.c_str(), stageName);
			cdmiClient = NULL;
			continue; // continue to the next stage
		}

		// step 3. send the request out
		lStartTime = ZQ::common::now();
		CURLcode retcode = CURLE_OK;
		if (!cdmiClient->sendRequest(retcode, true))
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "%s[%s]@%s request failed took %dms"), purposeStr, finalURL.c_str(), stageName, (int)(ZQ::common::now() - lStartTime));
			cdmiClient = NULL;
			continue; // continue to the next stage
		}

		// step 4. read the response

		// step 4.1 pre-dispatch the known error cases
		cdmiRetCode = cdmiClient->getStatusCode(statusLine);
		
		// case 4.1.1 stupid Aqua redirect, cache the location and try TryStage_Redirected
		if (cdmirc_AquaLocation == cdmiRetCode)
		{
			loc.locationIp = loc.paramsAppended = "";
			loc.stampInfoKnown = ZQ::common::now();
			loc.path = path_original;

			responseHeaders = cdmiClient->getResponseHeaders();
			itHd = responseHeaders.find("x-aqua-redirect-ip");
			if (responseHeaders.end()!=itHd)
				loc.locationIp = itHd->second;

			itHd = responseHeaders.find("x-aqua-redirect-tag");
			if (responseHeaders.end()!=itHd)
				loc.paramsAppended = itHd->second;

			if (!loc.path.empty() && !loc.locationIp.empty())
				cacheLocation(loc);

			_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiClientBase, "%s[%s]@%s server indicates redirect: %s(%d)<=[%s]: %s took %dms, x-aqua-redirect-ip[%s], x-aqua-redirect-tag[%s]"), purposeStr, finalURL.c_str(), stageName, 
				cdmiRetStr(cdmiRetCode), cdmiRetCode, statusLine.c_str(), cdmiClient->getErrorMessage().c_str(), (int)(ZQ::common::now() - lStartTime), loc.locationIp.c_str(), loc.paramsAppended.c_str());

			// continue to the next stage = STAGE_TRY_REDIRECTED
			cdmiClient = NULL;
			nextTryStage  = TryStage_Redirected;
			cdmiRetCode = cdmirc_RetryFailed;

			continue;  
		}

		// case 4.1.2 stupid Aqua authentication, no auto renewable expiration, to start over again if retry count allows
		if (cdmirc_Unauthorized == cdmiRetCode)
		{
			_secretKeyId = "";

			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "%s[%s]@%s failed: %s(%d)<=[%s]: %s took %dms"), purposeStr, finalURL.c_str(), stageName, 
				cdmiRetStr(cdmiRetCode), cdmiRetCode, statusLine.c_str(), cdmiClient->getErrorMessage().c_str(), (int)(ZQ::common::now() - lStartTime));

			// start over again from next stage=TryStage_PreCached, if retry count allows
			cdmiClient  = NULL;
			nextTryStage   = TryStage_PreCached;
			cdmiRetCode = cdmirc_RetryFailed;
			continue; 
		}

		// when reach here, there must be a explict return from the CDMI server, break the retry loop
		break;
	}

	// step Ex4.2 either explict response from the CDMI reserver or max-retry reached

	if (!CdmiRet_SUCC(cdmiRetCode))
	{
		std::string errorMessage = "";
		if(cdmiClient)
			errorMessage = cdmiClient->getErrorMessage();
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiClientBase, "%s[%s]@%s failed: %s(%d)<=[%s]: %s took %dms"), purposeStr, finalURL.c_str(), stageName,
			cdmiRetStr(cdmiRetCode), cdmiRetCode, statusLine.c_str(), errorMessage.c_str(), (int)(ZQ::common::now() - lStartTime));

		cdmiClient = NULL;
		return (CdmiClientBase::CdmiRetCode) cdmiRetCode;
	}

	// read the response body
	size_t dataSize = 0;
	//const char* pStrResponse = cdmiClient->getRespBuf(dataSize);
	ZQ::common::BufferList::Ptr resPtr = cdmiClient->getResponseBody();
	std::string strResponse;
	resPtr->readToString(strResponse);
	//if (NULL == pStrResponse)
	if(strResponse.empty())
	{
		cdmiClient = NULL;
		return cdmiRetCode;
	}

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiClientBase, "%s[%s]@%s: %s(%d)<=[%s]: %s took %dms; content-len[%d]"), purposeStr, finalURL.c_str(), stageName,
		cdmiRetStr(cdmiRetCode), cdmiRetCode, statusLine.c_str(), cdmiClient->getErrorMessage().c_str(), (int)(ZQ::common::now() - lStartTime), dataSize);

	if (NULL == recvbuff || len <=0)
	{
		responseTxtBody = strResponse;//std::string(pStrResponse, dataSize);
		cdmiClient = NULL;
		return cdmiRetCode;
	}
	else responseTxtBody ="";

	uint contentLen = (uint)dataSize;

	if (len > dataSize)
		len = (uint)dataSize;

    memcpy(recvbuff, /*(char*)pStrResponse*/strResponse.c_str(), len);

	cdmiClient = NULL;
	return cdmiRetCode;
}
