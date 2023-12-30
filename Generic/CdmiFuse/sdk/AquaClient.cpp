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
// Ident : $Id: JndiClient.java, hui.shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/CdmiFuse/sdk/AquaClient.cpp $
// 
// 13    12/04/14 2:20p Hui.shao
// 
// 12    11/19/14 8:05p Hui.shao
// userDomain
// 
// 11    6/05/14 6:00p Hui.shao
// temprarily rollback for linux build
// 
// 9     3/28/14 2:57p Li.huang
// 
// 8     2/26/14 1:40p Hongquan.zhang
// 
// 7     2/24/14 1:49p Hui.shao
// to call AquaClient in a DLL
// 
// 6     2/21/14 2:20p Hui.shao
// 
// 5     2/21/14 1:02p Ketao.zhang
// 
// 4     2/20/14 10:03a Hui.shao
// nonCdmi_CreateObject
// 
// 3     2/19/14 7:13p Hui.shao
// draft C++ API
// 
// 2     1/02/14 5:47p Hui.shao
// 
// 1     11/19/13 3:15p Hui.shao
// AquaClient draft
// ===========================================================================
#define _CUSTOM_TYPE_DEFINED_
#include "AquaClientImpl.h"
#include "../CdmiFuseOps.h"

#include <json/json.h>
#include <vector>

#ifdef ZQ_OS_MSWIN
HANDLE	_gCurrentModule=NULL;
#endif


namespace XOR_Media {
namespace AquaClient {

ZQ::common::FileLog* pLogger = NULL;
ZQ::common::NativeThreadPool* pThrdPool = NULL;
#define log       (*pLogger)

bool AquaClient::initSDK(const std::string& jsnstrConfig)
{
	try {
		Json::Value jsonConfig;
		if (!jsnstrConfig.empty() && !Json::Reader().parse(jsnstrConfig, jsonConfig))
			return false;

		// initialize the logger
		std::string logdir = ".\\";
		int loglevel = 5, logcount=2;
		long logsize = 10*1024*1024;
		int threads = 10;
		CdmiFuseOps::readEnvConfig(jsonConfig, logdir, loglevel, logsize, logcount, threads);

		if (NULL == pLogger)
			pLogger = new ZQ::common::FileLog((logdir+"AquaClient.log").c_str(), loglevel, logcount, logsize);

		log(ZQ::common::Log::L_INFO, CLOGFMT(AquaClient, "===== env initialize ===="));
		log(ZQ::common::Log::L_DEBUG, CLOGFMT(AquaClient, "   configuration: %s"), jsnstrConfig.c_str());

		// initialize the thread pool
		if (NULL == pThrdPool)
			pThrdPool = new ZQ::common::NativeThreadPool(threads);

		CdmiFuseOps::startCURLenv();

		return (NULL != pLogger && NULL !=pThrdPool);
	}
	catch(std::exception&)
	{
	}
	catch (...)
	{
	}

	return false;
}

void AquaClient::uninitSDK(void)
{
	CdmiFuseOps::stopCURLenv();

	if (NULL != pThrdPool)
		delete pThrdPool;
	pThrdPool = NULL;

	if (NULL != pLogger)
		delete pLogger;
	pLogger = NULL;
}

AquaClientImpl::AquaClientImpl(const std::string& rootUrl, const std::string& userDomain, const std::string& homeContainer, const uint32 flags, const FuseOpsConf& conf)
:_ctx(NULL)
{
	_ctx = new CdmiFuseOps(*pLogger, *pThrdPool, rootUrl, userDomain, homeContainer, flags, conf);
}

AquaClientImpl::~AquaClientImpl()
{
	if (NULL != _ctx)
		delete _ctx;
	_ctx = NULL;
}


AquaClient* AquaClient::newClient(const std::string& rootUrl, const std::string& userDomain, const std::string& homeContainer, const std::string& strProps)
{
	FuseOpsConf foConf;

	Json::Value jsonProps;
	if (!strProps.empty() && Json::Reader().parse(strProps, jsonProps))
		CdmiFuseOps::readClientConfig(jsonProps, foConf); 

	AquaClientImpl* pClient = new AquaClientImpl(rootUrl, userDomain, homeContainer, foConf.fuseFlag, foConf);
	if (NULL == pClient)
		return NULL;

	if (NULL == pClient->_ctx)
	{
		delete pClient;
		return NULL;
	}
	
	log(ZQ::common::Log::L_INFO, CLOGFMT(AquaClient, "client[%p] created with rootURL[%s] homeContainer[%s] flags[%x]"), pClient, rootUrl.c_str(), homeContainer.c_str(), foConf.fuseFlag);
	return pClient;
}

#define JCLIENTFMT(_X) CLOGFMT(NestedClient, "client[%p] " _X), this

#define NTAG(_T)  ("NTAG_" #_T)

#define READ_JSON_ARG(_VAR, _TAGNAME, _TYPE) if (jsonArgs.isMember(_TAGNAME)) { _VAR = jsonArgs[_TAGNAME].as##_TYPE(); }
#define READ_JSON_ARG_STR(_VAR, _TAGNAME) std::string _VAR; READ_JSON_ARG(_VAR, _TAGNAME, String)
#define READ_JSON_ARG_LONG(_VAR, _TAGNAME, _DEFVAL) int64 _VAR=_DEFVAL; READ_JSON_ARG(_VAR, _TAGNAME, Int64)
#define READ_JSON_ARG_BOOL(_VAR, _TAGNAME) bool _VAR=false; READ_JSON_ARG(_VAR, _TAGNAME, Bool)

#define READ_JSON_ARG_PROPS(_VAR, _TAGNAME) CdmiFuseOps::Properties _VAR; \
	if (jsonArgs.isMember(_TAGNAME) && jsonArgs[_TAGNAME].isObject()) { \
	Json::Value& jsonValue = jsonArgs[_TAGNAME]; Json::Value::Members members(jsonValue.getMemberNames()); \
	for (Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it ) \
	MAPSET(CdmiFuseOps::Properties, metadata, *it, jsonValue[*it].asString());	}

#define READ_JSON_ARG_STRLIST(_VAR, _TAGNAME) CdmiFuseOps::StrList _VAR; \
	if (jsonArgs.isMember(_TAGNAME) && jsonArgs[_TAGNAME].isArray()) { \
	Json::Value& jsonValue = jsonArgs[_TAGNAME]; \
	for (size_t i =0; i < jsonValue.size(); ++i ) \
	_VAR.push_back(jsonValue[i].asString()); }

/*
* Class:     NestedClient
* Method:    _destroy
* Signature: (LNestedClient;)V
*/
void AquaClientImpl::release(void)
{
	if (NULL != _ctx)
		delete _ctx;

	_ctx = NULL;
	delete this;
}

int AquaClientImpl::exec_Cdmi(const std::string& cmd, const std::string& uri, Json::Value& jsonArgs, Json::Value& jsonResult)
{
	log(ZQ::common::Log::L_DEBUG, JCLIENTFMT("dispatching cdmi cmd[%s] uri[%s]"), cmd.c_str(), uri.c_str());

	CdmiFuseOps::CdmiRetCode ret = CdmiFuseOps::cdmirc_SDK_Unsupported;
	do {
		if (0 == cmd.compare("CreateDataObject"))
		{
			READ_JSON_ARG_PROPS(metadata, NTAG(METADATA));
			READ_JSON_ARG_STRLIST(valuetransferencoding, NTAG(TRANSFER_ENCODING));

			READ_JSON_ARG_STR(mimetype,     NTAG(MIME_TYPE));
			READ_JSON_ARG_STR(value,        NTAG(VALUE));
			// READ_ARG(strContentType,  NTAG(CONTENT_TYPE));
			READ_JSON_ARG_STR(domainURI,    NTAG(DOMAIN_URI));
			READ_JSON_ARG_STR(deserialize,  NTAG(DESERIALIZE));
			READ_JSON_ARG_STR(serialize,    NTAG(SERIALIZE));
			READ_JSON_ARG_STR(copy,         NTAG(COPY));
			READ_JSON_ARG_STR(move,         NTAG(MOVE));
			READ_JSON_ARG_STR(reference,    NTAG(REFERENCE));
			READ_JSON_ARG_STR(deserializevalue,   NTAG(DESER_VALUE));

			ret = ((CdmiFuseOps*)_ctx)->cdmi_CreateDataObject(jsonResult, uri, mimetype, metadata,
				value, valuetransferencoding, domainURI, deserialize, serialize,
				copy, move, reference, deserializevalue);

			break;
		}

		if (0 == cmd.compare("ReadDataObject"))
		{
			Json::Value resp;
			std::string location;

			ret = ((CdmiFuseOps*)_ctx)->cdmi_ReadDataObject(resp, uri, location);

			jsonResult[NTAG(RESPBODY)] = "";
			if (resp.size() >0)
				jsonResult[NTAG(RESPBODY)] = resp;

			if (!location.empty())
				jsonResult[NTAG(LOCATION)] = location;
			break;
		}

		if (0 == cmd.compare("UpdateDataObject"))
		{
			Json::Value metadata;
			if (jsonArgs.isMember(NTAG(METADATA)) && jsonArgs[NTAG(METADATA)].isObject())
				metadata = jsonArgs[NTAG(METADATA)];

			READ_JSON_ARG_STRLIST(valuetransferencoding, NTAG(TRANSFER_ENCODING));

			READ_JSON_ARG_LONG(base_version,   NTAG(BASE_VERSION), -1);
			READ_JSON_ARG_BOOL(partial,        NTAG(PARTIAL));
			READ_JSON_ARG_LONG(startOffset,    NTAG(START_OFFSET), 0);
			READ_JSON_ARG_STR(value,    NTAG(VALUE));
			READ_JSON_ARG_STR(domainURI,    NTAG(DOMAIN_URI));
			READ_JSON_ARG_STR(deserialize,  NTAG(DESERIALIZE));
			READ_JSON_ARG_STR(copy,         NTAG(COPY));
			READ_JSON_ARG_STR(deserializevalue,   NTAG(DESER_VALUE));

			///@param[out] location  The server shall respond with the URI that the reference redirects to if the object is a reference
			///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>
			///@param[in] partial   true to ndicates that the object is in the process of being updated, and has not yet been fully updated. Maps to HTTP header X-CDMI-Partial
			///@param[in] value     The data object value
			///@param[in] valuetransferencoding  the value transfer encoding used for the data object value. Two value transfer encodings are defined.
			///@param[in] domainURI  URI of the owning domain
			///@param[in] deserialize  URI of a serialized CDMI data object that shall be deserialized to create the new data object
			///@param[in] copy       URI of a CDMI data object or queue that shall be copied into the new data object
			///@param[in] deserializevalue a data object serialized as specified in Clause 15 and encoded using base 64 encoding rules described in RFC 4648.
			// virtual CdmiRetCode cdmi_UpdateDataObjectEx(out std::string& location, const std::string& uri, const Json::Value& metadata,
			//	uint64 startOffset, const std::string& value="", int base_version=-1, bool partial=false, const StrList& valuetransferencoding = StrList(),
			//	const std::string& domainURI="", const std::string& deserialize="", const std::string& copy="",
			//	const std::string& deserializevalue="");

			Json::Value resp;
			std::string location;

			ret = ((CdmiFuseOps*)_ctx)->cdmi_UpdateDataObjectEx(location, uri, metadata,
				startOffset, value, (int)base_version, partial, valuetransferencoding,
				domainURI, deserialize, copy, deserializevalue);

			jsonResult[NTAG(RESPBODY)] = "";
			if (resp.size() >0)
				jsonResult[NTAG(RESPBODY)] = resp;

			if (!location.empty())
				jsonResult[NTAG(LOCATION)] = location;
			break;
		}

		if (0 == cmd.compare("DeleteDataObject"))
		{
			Json::Value resp;
			// Delete a Data Object using CDMI Content Type
			///@param[out] result   the JSON value returned from the server
			///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>
			// virtual CdmiRetCode cdmi_DeleteDataObject(Json::Value& result, const std::string& uri);

			ret = ((CdmiFuseOps*)_ctx)->cdmi_DeleteDataObject(resp, uri);

			jsonResult[NTAG(RESPBODY)] = "";
			if (resp.size() >0)
				jsonResult[NTAG(RESPBODY)] = resp;

			break;
		}

		if (0 == cmd.compare("ReadContainer"))
		{
			Json::Value resp;
			/// Read a Container Object using CDMI Content Type
			///@param[out] result   the JSON value returned from the server
			///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>/
			ret = ((CdmiFuseOps*)_ctx)->cdmi_ReadContainer(resp, uri);
			jsonResult[NTAG(RESPBODY)] = "";
			if (resp.size() >0)
				jsonResult[NTAG(RESPBODY)] = resp;

			break;
		}

		if (0 == cmd.compare("UpdateContainer"))
		{
			READ_JSON_ARG_PROPS(metadata, NTAG(METADATA));
			READ_JSON_ARG_STR(domainURI,    NTAG(DOMAIN_URI));
			READ_JSON_ARG_STR(deserialize,   NTAG(DESERIALIZE));
			READ_JSON_ARG_STR(copy,         NTAG(COPY));
			READ_JSON_ARG_STR(snapshot,     NTAG(SNAPSHOT));
			READ_JSON_ARG_STR(deserializevalue,   NTAG(DESER_VALUE));

			Json::Value resp, exports;
			/// Update a Container Object using CDMI Content Type
			///@param[out] result   the JSON value returned from the server
			///@param[out] location The server shall respond with the URI that the reference redirects to if the object is a reference.
			///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>/
			///@param[in] metadata  metadata for the container object.
			///@param[in] exports   A structure for each protocol that is enabled for this container object (see Clause 13).
			///@param[in] domainURI  URI of the owning domain
			///@param[in] deserialize  URI of a serialized CDMI container object that shall be deserialized to create the new container object
			///@param[in] copy       URI of a CDMI container object or queue that shall be copied into the new container object
			///@param[in] snapshot   Name of the snapshot to be taken
			///@param[in] deserializevalue  a container object serialized as specified in Clause 15 and encoded using base 64 encoding rules described in RFC 4648.
			// virtual CdmiRetCode cdmi_UpdateContainer(Json::Value& result, out std::string& location,
			//	const std::string& uri, const Properties& metadata,	const Json::Value& exports="",
			//	const std::string& domainURI="", const std::string& deserialize="", const std::string& copy="",
			//	const std::string& snapshot="", const std::string& deserializevalue="");

			std::string location;
			ret = ((CdmiFuseOps*)_ctx)->cdmi_UpdateContainer(resp, location, uri, metadata, exports,
				domainURI, deserialize, copy, snapshot, deserializevalue);

			jsonResult[NTAG(RESPBODY)] = "";
			if (resp.size() >0)
				jsonResult[NTAG(RESPBODY)] = resp;
			if (exports.size() >0)
				jsonResult[NTAG(EXPORTS)] = exports;

			break;
		}

		if (0 == cmd.compare("DeleteContainer"))
		{
			Json::Value resp;
			/// Delete a Container Object using CDMI Content Type
			///@param[out] result   the JSON value returned from the server
			///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>/
			// virtual CdmiRetCode cdmi_DeleteContainer(Json::Value& result, const std::string& uri);
			ret = ((CdmiFuseOps*)_ctx)->cdmi_DeleteContainer(resp, uri);

			jsonResult[NTAG(RESPBODY)] = "";
			if (resp.size() >0)
				jsonResult[NTAG(RESPBODY)] = resp;

			break;
		}

		if (0 == cmd.compare("ReadAquaDomain"))
		{
			Json::Value resp;
			READ_JSON_ARG_STR(domainURI,    NTAG(DOMAIN_URI));
			/// read the Aqua domain extersion
			///@param[out] result      the json value parsed from the response body
			ret = ((CdmiFuseOps*)_ctx)->cdmi_ReadAquaDomain(resp, domainURI);

			jsonResult[NTAG(RESPBODY)] = "";
			if (resp.size() >0)
				jsonResult[NTAG(RESPBODY)] = resp;

			break;
		}

		log(ZQ::common::Log::L_ERROR, JCLIENTFMT("unknown cdmi cmd[%s] uri[%s]"), cmd.c_str(), uri.c_str());

	} while(0);

	// strResult = Json::FastWriter().write(jResult);
	return ret;
}

int AquaClientImpl::exec_nonCdmi(const std::string& cmd, const std::string& uri, Json::Value& jsonArgs, Json::Value& result, char* buf, uint32& len)
{
	log(ZQ::common::Log::L_DEBUG, JCLIENTFMT("dispatching non-cdmi cmd[%s] uri[%s] buf[%p] len[%d]"), cmd.c_str(), uri.c_str(), buf, len);

	CdmiFuseOps::CdmiRetCode ret = CdmiFuseOps::cdmirc_SDK_Unsupported;
	do {
		if (0 == cmd.compare("CreateDataObject"))
		{
			READ_JSON_ARG_STR(contentType,     NTAG(CONTENT_TYPE));

			ret = _ctx->nonCdmi_CreateDataObject(uri, contentType, (const char*)buf, (uint32)len);
			break;
		}

		if (0 == cmd.compare("ReadDataObject"))
		{
			/// Read a Data Object using Non-CDMI Content Type
			///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>
			///@param[out] contentType the content type of the data to be stored as a data object.
			///@param[out] location  The server shall respond with the URI that the reference redirects to if the object is a reference
			///@param[in] startOffset the offset to start read
			///@param[in,out] len   to indicate the max bytes can receive as the input parameter; to confirm the actual bytes have been read as the output parameter
			// virtual CdmiRetCode nonCdmi_ReadDataObject(const std::string& uri, std::string& contentType, std::string& location, uint64 startOffset, in out uint32& len, char* recvbuff, bool disableCache = false);
			// READ_JSON_ARG_STR(contentType,     NTAG(CONTENT_TYPE));
			READ_JSON_ARG_LONG(startOffset,    NTAG(START_OFFSET), 0);
			READ_JSON_ARG_BOOL(directio,       NTAG(DIRECTIO));

			//!!!!! len = (uint32)jcap;
			std::string location, contentType;
			ret = _ctx->nonCdmi_ReadDataObject(uri, contentType, location, startOffset, len, (char*)buf, directio);
			if (!location.empty())
				result[NTAG(LOCATION)] = location;

			if (!contentType.empty())
				result[NTAG(CONTENT_TYPE)] = contentType;

			//!!!! limitJBuffer(jenv, jbuffer, len);
			result[NTAG(NBYTE_EXECUTED)] = len;
			break;
		}

		if (0 == cmd.compare("UpdateDataObject"))
		{
			READ_JSON_ARG_STR(contentType,    NTAG(CONTENT_TYPE));
			READ_JSON_ARG_LONG(startOffset,    NTAG(START_OFFSET), 0);
			READ_JSON_ARG_BOOL(directio,       NTAG(DIRECTIO));
			READ_JSON_ARG_BOOL(partial,        NTAG(PARTIAL));
			READ_JSON_ARG_LONG(objectSize,     NTAG(OBJECT_SIZE), -1);

			std::string location;
			ret = _ctx->nonCdmi_UpdateDataObject(uri, location, contentType, startOffset, len, (char*)buf, objectSize, partial, directio);
			if (!location.empty())
				result[NTAG(LOCATION)] = location;

			result[NTAG(NBYTE_EXECUTED)] = len;
			break;
		}

		if (0 == cmd.compare("DeleteDataObject"))
		{
			/// Delete a Data Object using a Non-CDMI Content Type
			///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>
			// virtual CdmiRetCode nonCdmi_DeleteDataObject(const std::string& uri);
			ret = _ctx->nonCdmi_DeleteDataObject(uri);
			break;
		}

		if (0 == cmd.compare("CreateContainer"))
		{
			/// Create a Container Object using a Non-CDMI Content Type
			///@param[in] uri  the container name appends to the rootUri, will be appended with a '/' if not available
			// virtual CdmiRetCode nonCdmi_CreateContainer(const std::string& uri);
			ret = _ctx->nonCdmi_CreateContainer(uri);
			break;
		}

		if (0 == cmd.compare("DeleteContainer"))
		{
			/// Delete a Container Object using a Non-CDMI Content Type
			///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>/
			// virtual CdmiRetCode nonCdmi_DeleteContainer(const std::string& uri);
			ret = _ctx->nonCdmi_DeleteContainer(uri);
			break;
		}

		if (0 == cmd.compare("FlushDataObject"))
		{
			// virtual CdmiRetCode flushdata( const std::string& uri );
			ret = _ctx->flushdata(uri);
			break;
		}

		log(ZQ::common::Log::L_ERROR, JCLIENTFMT("unknown non-cdmi cmd[%s] uri[%s]"), cmd.c_str(), uri.c_str());

	} while(0);

	// adjust pos and limit of the jbuffer
	// rangeBuffer(jenv, jbuffer, len);

	//	result[NTAG(RET)] = (int) ret;
	// strResult = Json::FastWriter().write(result);
	// fillStringBuffer(jenv, jresult, strResult);

	return ret;
}

int AquaClient::cdmi_CreateDataObject(std::string& strResult, const std::string& uri,
								 const std::string& mimetype, const std::string& jsonMetadata, const std::string& value,
								 const StrList& valuetransferencoding, const std::string& domainURI,
								 const std::string& deserialize, const std::string& serialize, const std::string& copy, const std::string& move,
								 const std::string& reference, const std::string& deserializeValue)
{
	// step 1. envelop the input parameters into jsonParams
	Json::Value jsonParams, jsonResult;

	jsonParams[NTAG(MIME_TYPE)] = mimetype;
	jsonParams[NTAG(VALUE)] = value;
	jsonParams[NTAG(METADATA)] = jsonMetadata;

	if (valuetransferencoding.size() >0)
	{
		Json::Value jo;
		for (size_t i = 0; i < valuetransferencoding.size(); i++)
			jo[i] = valuetransferencoding[i];
// 		for (StrList::const_iterator it = valuetransferencoding.begin(); it< valuetransferencoding.end(); it++)
// 			jo.append(*it);

		jsonParams[NTAG(TRANSFER_ENCODING)] = jo;
	}

	jsonParams[NTAG(DOMAIN_URI)] = domainURI;
	jsonParams[NTAG(DESERIALIZE)] = deserialize;
	jsonParams[NTAG(SERIALIZE)] = serialize;

	jsonParams[NTAG(COPY)] = copy;
	jsonParams[NTAG(MOVE)] = move;
	jsonParams[NTAG(REFERENCE)] = reference;

	jsonParams[NTAG(DESER_VALUE)] = deserializeValue;

	// step 2. call _exec_Cdmi()
	int ret = ((AquaClientImpl*)this)->exec_Cdmi("CreateDataObject", uri, jsonParams, jsonResult);

	// step 3. parse the jsonResult and dispatch the output parameters
	try {
		if (jsonResult.isMember(NTAG(RESPBODY)))
			strResult = Json::FastWriter().write(jsonResult[NTAG(RESPBODY)]);
	}
	catch(...) {}

	return ret;
}

int AquaClient::nonCdmi_CreateDataObject(const std::string& uri, const std::string& contentType,
										 const char value[], uint32 len) 
{
	// step 1. envelop the input parameters into jsonParams
	Json::Value jsonParams, jsonResult;
	if (!contentType.empty())
		jsonParams[NTAG(CONTENT_TYPE)] = contentType;

	// step 2. call _exec_nonCdmi()
	int ret = ((AquaClientImpl*)this)->exec_nonCdmi("CreateDataObject", uri, jsonParams, jsonResult, (char*)value, len);

	// step 3. parse the jsonResult and dispatch the output parameters
	return ret;
}

int AquaClient::cdmi_ReadDataObject(std::string& jsonResult, const std::string& uri, std::string& location)
{
	// step 1. envelop the input parameters into jsonParams
	Json::Value privateParams, privateResult;

	// step 2. call _exec_nonCdmi()
	int ret = ((AquaClientImpl*)this)->exec_Cdmi("ReadDataObject", uri, privateParams, privateResult);

	// step 3. parse the jsonResult and dispatch the output parameters
	if (jsonResult.empty())
		return ret;

	if (privateResult.isMember(NTAG(NTAG_RESPBODY)))
		jsonResult = Json::FastWriter().write(privateResult[NTAG(NTAG_RESPBODY)]);
	else
		jsonResult = Json::FastWriter().write(privateResult);

	if (!location.empty() && privateResult.isMember(NTAG(NTNTAG_LOCATION)))
		location = Json::FastWriter().write(privateResult[NTAG(NTAG_LOCATION)]);

	return ret;
}

int AquaClient::nonCdmi_ReadDataObject(const std::string& uri, std::string& contentType,
				   std::string& location, long startOffset, long& byteRead,
				   char buffer[], uint32 len, bool direct)
{
	// step 1. envelop the input parameters into jsonParams
	Json::Value jsonParams, privateResult;

	jsonParams[NTAG(NTAG_START_OFFSET)] = (long long)startOffset;
	jsonParams[NTAG(NTAG_DIRECTIO)] = direct;
	
	// step 2. call _exec_nonCdmi()
	byteRead = 0;
	int ret =((AquaClientImpl*)this)->exec_nonCdmi("ReadDataObject", uri, jsonParams, privateResult, (char*)buffer, len);

	// step 3. parse the jsonResult and dispatch the output parameters
	if (privateResult.isMember(NTAG(NNTAG_LOCATION)))
		location = Json::FastWriter().write(privateResult[NTAG(NTAG_LOCATION)]);

	if (privateResult.isMember(NTAG(NTAG_CONTENT_TYPE)))
		contentType = Json::FastWriter().write(privateResult[NTAG(NTAG_CONTENT_TYPE)]);

	if (privateResult.isMember(NTAG(NBYTE_EXECUTED)))
	{
		int len = (int) privateResult[NTAG(NBYTE_EXECUTED)].asInt();
		byteRead = len;
	}

	return ret;
}

int AquaClient::cdmi_UpdateDataObject(const std::string& uri, std::string& location, const std::string& jsonMetadata,
				  long startOffset, const std::string& value, int base_version, bool partial, const StrList& valuetransferencoding,
				  const std::string& domainURI, const std::string& deserialize, const std::string& copy,
				  const std::string& deserializevalue)
{
	// step 1. envelop the input parameters into jsonParams
	Json::Value jsonParams, privateResult;

	jsonParams[NTAG(NTAG_METADATA)] = jsonMetadata;
	jsonParams[NTAG(NTAG_BASE_VERSION)] = base_version;
	jsonParams[NTAG(NTAG_PARTIAL)] = partial;
	jsonParams[NTAG(NTAG_START_OFFSET)] = (long long)startOffset;
	jsonParams[NTAG(NTAG_VALUE)] = value;
	jsonParams[NTAG(NTAG_DOMAIN_URI)] = domainURI;
	jsonParams[NTAG(NTAG_DESERIALIZE)] = deserialize;
	jsonParams[NTAG(NTAG_COPY)] = copy;
	jsonParams[NTAG(NTAG_DESER_VALUE)] = deserializevalue;
	
	if (valuetransferencoding.size() >0)
	{
		Json::Value jo;
		for (size_t i = 0; i < valuetransferencoding.size(); i++)
			jo[i] = valuetransferencoding[i];
		jsonParams[NTAG(TRANSFER_ENCODING)] = jo;
	}

	// step 2. call _exec_Cdmi()
	int ret = ((AquaClientImpl*)this)->exec_Cdmi("UpdateDataObject", uri, jsonParams, privateResult);

	// step 3. parse the jsonResult and dispatch the output parameters
	//		if (null == jsonResult)
	//			return ret;

	//JSONObject jo = JSONObject.fromObject(privateResult.toString());
	//		if (jo.has(NTAG_RESPBODY))
	//			jsonResult.append(jo.getJSONObject(NTAG_RESPBODY).toString());
	//		else
	//			jsonResult.append(privateResult);

	if (privateResult.isMember(NTAG(NTAG_LOCATION)))
		location = Json::FastWriter().write(privateResult[NTAG(NTAG_LOCATION)]);

	return ret;
}

int AquaClient::nonCdmi_UpdateDataObject(const std::string& uri, std::string& location, const std::string& contentType, long startOffset, long objectSize, const char buffer[], uint32 len, bool partial, bool direct)
{
	// step 1. envelop the input parameters into jsonParams
	Json::Value jsonParams, privateResult;

	if (!contentType.empty())
		jsonParams[NTAG(NTAG_CONTENT_TYPE)] = contentType;
	jsonParams[NTAG(NTAG_START_OFFSET)] = (long long)startOffset;
	jsonParams[NTAG(NTAG_PARTIAL)] = partial;
	jsonParams[NTAG(NTAG_OBJECT_SIZE)] = (long long)objectSize;
	jsonParams[NTAG(NTAG_DIRECTIO)] = direct;

	// step 2. call _exec_nonCdmi()
	int ret = ((AquaClientImpl*)this)->exec_nonCdmi("UpdateDataObject", uri, jsonParams, privateResult, (char *)buffer, len);

	// step 3. parse the jsonResult and dispatch the output parameters
	//JSONObject jo = JSONObject.fromObject(privateResult.toString());
	if (privateResult.isMember(NTAG(NTAG_LOCATION)))
		location = Json::FastWriter().write(privateResult[NTAG(NTAG_LOCATION)]);

	//buffer.flip();
// 	if (jo.has(NTAG_NBYTE_EXECUTED)) {
// 		int len = (int) jo.getLong(NTAG_NBYTE_EXECUTED);
// 		buffer.limit(len);
//	}
	return ret;
}

int AquaClient::cdmi_DeleteDataObject(std::string& jsonResult, const std::string& uri)
{
	// step 1. envelop the input parameters into jsonParams
	Json::Value jsonParams, privateResult;

	// step 2. call _exec_nonCdmi()
	int ret = ((AquaClientImpl*)this)->exec_Cdmi("DeleteDataObject", uri, jsonParams, privateResult);
	// System.out.println("presult=" + privateResult.toString());

	// step 3. parse the jsonResult and dispatch the output parameters
	if (jsonResult.empty())
		return ret;

	//JSONObject jo = JSONObject.fromObject(privateResult.toString());
	// jo.remove(NTAG_RET);
	if (privateResult.isMember(NTAG(NTAG_RESPBODY)))
		jsonResult = Json::FastWriter().write(privateResult[NTAG(NTAG_RESPBODY)]);
	else
		jsonResult = Json::FastWriter().write(privateResult);

	return ret;
}

int AquaClient::nonCdmi_DeleteDataObject(const std::string& uri)
{
	// step 1. envelop the input parameters into jsonParams
	Json::Value jsonParams,privateResult;

	// step 2. call _exec_nonCdmi()
	uint32 len = 0;
	int ret = ((AquaClientImpl*)this)->exec_nonCdmi("DeleteDataObject", uri, jsonParams, privateResult, NULL, len);

	// step 3. parse the jsonResult and dispatch the output parameters
	return ret;
}

int AquaClient::cdmi_CreateContainer(std::string& jsonResult, const std::string& uri, const std::string& jsonMetadata,
				 const std::string& jsonExports, const std::string& domainURI, const std::string& deserialize, 
				 const std::string& copy, const std::string& move, const std::string& reference, 
				 const std::string& deserializeValue)
{
	// step 1. envelop the input parameters into jsonParams
	Json::Value jsonParams ;
	jsonParams[NTAG(NTAG_METADATA)] = jsonMetadata;
	jsonParams[NTAG(NTAG_EXPORTS)] = jsonExports;
	jsonParams[NTAG(NTAG_DOMAIN_URI)] = domainURI;
	jsonParams[NTAG(NTAG_DESERIALIZE)] = deserialize;
	jsonParams[NTAG(NTAG_COPY)] = copy;
	jsonParams[NTAG(NTAG_MOVE)] = move;
	jsonParams[NTAG(NTAG_REFERENCE)] = reference;
	jsonParams[NTAG(NTAG_DESER_VALUE)] = deserializeValue;

	// step 2. call _exec_Cdmi()
	Json::Value privateResult ;
	int ret =((AquaClientImpl*)this)->exec_Cdmi("CreateContainer", uri, jsonParams, privateResult);

	// step 3. parse the jsonResult and dispatch the output parameters
	if (jsonResult.empty())
		return ret;

	// JSONObject jo = JSONObject.fromObject(privateResult.toString());
	// if (jo.has(NTAG_RESPBODY))
	// 	jsonResult.append(jo.getJSONObject(NTAG_RESPBODY).toString());
	// else
	//	jsonResult.append(privateResult);
	return ret;
}


int AquaClient::nonCdmi_CreateContainer(const std::string& uri)
{
	// step 1. envelop the input parameters into jsonParams
	Json::Value privateResult , jsonParams;

	// step 2. call _exec_nonCdmi()
	uint32 len = 0;
	int ret = ((AquaClientImpl*)this)->exec_nonCdmi("CreateContainer", uri, jsonParams,
		privateResult, NULL, len);

	// step 3. parse the jsonResult and dispatch the output parameters
	return ret;
}

int AquaClient::cdmi_ReadContainer(std::string& jsonResult, const std::string& uri)
{
	// step 1. envelop the input parameters into jsonParams
	Json::Value privateResult , jsonParams;

	// step 2. call _exec_Cdmi()
	int ret = ((AquaClientImpl*)this)->exec_Cdmi("ReadContainer", uri, jsonParams, privateResult);

	// step 3. parse the jsonResult and dispatch the output parameters
	if (jsonResult.empty())
		return ret;

	//JSONObject jo = JSONObject.fromObject(privateResult.toString());
	if (privateResult.isMember(NTAG(NTAG_RESPBODY)))
		jsonResult = Json::FastWriter().write(privateResult[NTAG(NTAG_RESPBODY)]);
	else
		jsonResult = Json::FastWriter().write(privateResult);
	return ret;
}

int AquaClient::cdmi_UpdateContainer(std::string& jsonResult, const std::string& uri, std::string& location,
					  const std::string& jsonMetadata, const std::string& jsonExports,
					  const std::string& domainURI, const std::string& deserialize, const std::string& copy,
					  const std::string& snapshot, const std::string& deserializeValue)
{
	// step 1. envelop the input parameters into jsonParams
	Json::Value jsonParams ,privateResult;
	jsonParams[NTAG(NTAG_METADATA)] = jsonMetadata;
	jsonParams[NTAG(NTAG_EXPORTS)] = jsonExports;
	jsonParams[NTAG(NTAG_DOMAIN_URI)] = domainURI;
	jsonParams[NTAG(NTAG_DESERIALIZE)] = deserialize;
	jsonParams[NTAG(NTAG_COPY)] = copy;
	jsonParams[NTAG(NTAG_SNAPSHOT)] = snapshot;
	jsonParams[NTAG(NTAG_DESER_VALUE)] = deserializeValue;

	// step 2. call _exec_Cdmi()
	int ret = ((AquaClientImpl*)this)->exec_Cdmi("UpdateContainer", uri, jsonParams, privateResult);

	// step 3. parse the jsonResult and dispatch the output parameters
	if (jsonResult.empty())
		return ret;

	// JSONObject jo = JSONObject.fromObject(privateResult.toString());
	// if (jo.has(NTAG_RESPBODY))
	// 	jsonResult.append(jo.getJSONObject(NTAG_RESPBODY).toString());
	// else
	//	jsonResult.append(privateResult);
	return ret;
}

int AquaClient::cdmi_DeleteContainer(std::string& jsonResult, const std::string& uri)
 {
	 // step 1. envelop the input parameters into jsonParams
	 Json::Value privateResult, jsonParams;

	 // step 2. call _exec_Cdmi()
	 int ret = ((AquaClientImpl*)this)->exec_Cdmi("DeleteContainer", uri, jsonParams, privateResult);

	 // step 3. parse the jsonResult and dispatch the output parameters
	 if (jsonResult.empty())
		 return ret;

	 // JSONObject jo = JSONObject.fromObject(privateResult.toString());
	 // if (jo.has(NTAG_RESPBODY))
	 // 	jsonResult.append(jo.getJSONObject(NTAG_RESPBODY).toString());
	 // else
	 //	jsonResult.append(privateResult);
	 return ret;
 }

int AquaClient::nonCdmi_DeleteContainer(const std::string& uri)
{
	// step 1. envelop the input parameters into jsonParams
	Json::Value privateResult, jsonParams;

	// step 2. call _exec_nonCdmi()
	uint32 len = 0; 
	int ret = ((AquaClientImpl*)this)->exec_nonCdmi("DeleteContainer", uri, jsonParams,privateResult, NULL,len);

	// step 3. parse the jsonResult and dispatch the output parameters
	return ret;
}

int AquaClient::cdmi_ReadAquaDomain(std::string& jsonResult, const std::string& domainURI)
{
	// step 1. envelop the input parameters into jsonParams
	Json::Value jsonParams , privateResult;
	jsonParams[NTAG(NTAG_DOMAIN_URI)] = domainURI;

	// step 2. call _exec_Cdmi()
	int ret = ((AquaClientImpl*)this)->exec_Cdmi("ReadAquaDomain", "", jsonParams, privateResult);

	// step 3. parse the jsonResult and dispatch the output parameters
	if (jsonResult.empty())
		return ret;

	if (privateResult.isMember(NTAG(NTAG_RESPBODY)))
		jsonResult = Json::FastWriter().write(privateResult[NTAG(NTAG_RESPBODY)]);
	else
		jsonResult = Json::FastWriter().write(privateResult);

	return ret;
}

int AquaClient::flushDataObject( const std::string& uri )
{
	// step 1. envelop the input parameters into jsonParams
	Json::Value jsonParams , privateResult;

	// step 2. call _exec_nonCdmi()
	uint32  len = 0;
	int ret = ((AquaClientImpl*)this)->exec_nonCdmi("FlushDataObject", uri, jsonParams, privateResult, NULL, len);

	// step 3. parse the jsonResult and dispatch the output parameters
	return ret;
}


} } // namespaces
#ifdef ZQ_OS_MSWIN
// -----------------------------
// DLL Entries
// -----------------------------
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	_gCurrentModule=(HMODULE)hModule;
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

#endif

