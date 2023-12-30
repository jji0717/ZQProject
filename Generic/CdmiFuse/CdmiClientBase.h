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
// $Log: /ZQProjs/Generic/CdmiFuse/CdmiClientBase.h $
// 
// 3     8/01/13 10:38a Li.huang
// 
// 2     8/01/13 10:24a Li.huang
// 
// 1     7/11/13 4:58p Li.huang
// ===========================================================================
#ifndef __CdmiClientBase_H__
#define __CdmiClientBase_H__

#include <string>
#include <vector>
#include <map>

#include "Log.h"
#include "NativeThreadPool.h"
#include "CDMIHttpClient.h"
#include "LRUMap.h"

#include <json/json.h>


extern "C" {
#include <sys/stat.h>
}

#define out
#define in

#define DEFAULT_CONTENT_MIMETYPE		"application/octet-stream"
#define DEFAULT_LOGIN_PATH              "aqua/rest/cdmi/cdmi_users/"

#define MIN_BULKET_SIZE (1024*1024)
#define MIN_BYTE_SIZE (MIN_BULKET_SIZE *16)

class CdmiClientBase
{
public:
	CdmiClientBase(ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, const std::string& rootUrl, const std::string& homeContainer, const uint32 flags = 0x0f);
	~CdmiClientBase(void);
public:

	enum {
		// the low 16bits were reserved for curl client
		flgDumpMsgBody  =FLAG(16),
		flgHexDump      =FLAG(17),

	};
	typedef std::map <std::string, std::string> Properties;
	typedef std::vector <std::string> StrList;
	typedef enum _CdmiErrors
	{
		cdmirc_OK = 200,
		cdmirc_Created = 201,
		cdmirc_Accepted = 202, 
		cdmirc_NoContent = 204,
		cdmirc_PartialContent = 206,
		cdmirc_Found = 302, 
		cdmirc_BadRequest = 400,
		cdmirc_Unauthorized = 401,
		cdmirc_Forbidden = 403,
		cdmirc_NotFound = 404,
		cdmirc_NotAcceptable = 406,
		cdmirc_Conflict = 409,
		cdmirc_InvalidRange = 416,
		cdmirc_ServerError = 500,

		cdmirc_ExtErr = 700,
		cdmirc_RequestFailed,
		cdmirc_RequestTimeout,
		cdmirc_AquaLocation = 732,
		cdmirc_RetryFailed,
		cdmirc_MAX
	} CdmiErrors;

	typedef int CdmiRetCode;

#define CdmiRet_SUCC(_cdmiRetCode) (_cdmiRetCode >=200 && _cdmiRetCode< 300)
#define CdmiRet_FAIL(_cdmiRetCode) !CdmiRet_SUCC(_cdmiRetCode)
#define CdmiRet_ClientErr(_cdmiRetCode) (_cdmiRetCode >=400 && _cdmiRetCode< 500)
#define CdmiRet_ServerErr(_cdmiRetCode) (_cdmiRetCode >=500 && _cdmiRetCode< 700)
#define CdmiRet_CommunicationErr(_cdmiRetCode) (cdmirc_RequestFailed == _cdmiRetCode || cdmirc_RequestTimeout == _cdmiRetCode)


	typedef struct _ResourceLocation
	{
		std::string path, locationIp, paramsAppended;
		int64  stampInfoKnown;  // timestamp
	} ResourceLocation;

	typedef ZQ::common::LRUMap <std::string, ResourceLocation> LocationCache; // cache of uri-path to location

protected:
	uint  _connectTimeout, _timeout;
	ZQ::common::Log& _log;
	std::string      _rootUrl, _homeContainer;
	ZQ::common::NativeThreadPool& _thrdPool;

	std::string _username, _password, _localRunAs;
	std::string _accessKeyId, _secretKeyId;

	std::string _domainURIOfMP;

	uint32  _flags; // the lower 16bits is curl flags 

	uint              _currentServerIdx;

private:
	LocationCache     _locationCache;
	ZQ::common::Mutex _lkLocationCache;

public:
	virtual void setAccount(const std::string& serverLogin, const std::string& password="", const std::string& localRunAs="");
	void setTimeout(uint connectTimeout, uint timeout);

	virtual void fixupPathname(std::string& pathname) {}
	//	virtual std::string generateURL(const std::string& uri, bool bContainer = false, bool includeUsername =false);
	virtual std::string pathToUri(const std::string& pathname);

	std::string assembleURL(const std::string& serverIp, const std::string& uri, bool bContainer =false, bool includeUsername =false);
	std::string getServerIp(bool next=false);

	uint getCurrentServerIdx();
	bool generateSignature(std::string& signature, const std::string& uri, const std::string& contentType, ZQ::common::CURLClient::HTTPMETHOD method, const std::string& xAquaDate);

protected:

	virtual int  callRemote(const char* purposeStr, std::string uri, std::string& finalURL, std::string& respStatusLine, bool bContainer, bool includeUsername,
		const Properties& requestHeaders, const std::string& requestBody, const char* reqBodyBuf, int reqBodyLen,
		Properties& responseHeaders, std::string& responseTxtBody, in out uint& len, char* recvbuff,
		ZQ::common::CURLClient::HTTPMETHOD method, int clientFlags);

	bool readLocationFromCache(const std::string path, ResourceLocation& resLoc);
	void cacheLocation(const ResourceLocation& resLoc);
	void uncacheLocation(const std::string path);

	std::string getRelocatedURL(std::string& uri, const ResourceLocation& loc, bool bContainer, bool includeUsername);

public:
	/// Create a Data Object Using CDMI Content Type
	///@param[out] result   the JSON value returned from the server
	///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>
	///@param[in] mimetype  MIME type of the data contained within the value field of the data object
	///@param[in] metadata  Metadata for the data object
	///@param[in] value     The data object value
	///@param[in] valuetransferencoding  the value transfer encoding used for the data object value. Two value transfer encodings are defined.
	///@param[in] domainURI  URI of the owning domain
	///@param[in] deserialize  URI of a serialized CDMI data object that shall be deserialized to create the new data object
	///@param[in] serialize  URI of a CDMI object that shall be serialized into the new data object
	///@param[in] copy       URI of a CDMI data object or queue that shall be copied into the new data object
	///@param[in] move       URI of an existing local or remote CDMI data object (source URI) that shall be relocated to the URI specified in the PUT
	///@param[in] reference  URI of a CDMI data object that shall be redirected to by a reference.
	///@param[in] deserializevalue a data object serialized as specified in Clause 15 and encoded using base 64 encoding rules described in RFC 4648.
	virtual CdmiRetCode cdmi_CreateDataObject(Json::Value& result, const std::string& uri, const std::string& mimetype, const Properties& metadata,
		const std::string& value="", const StrList& valuetransferencoding = StrList(),
		const std::string& domainURI="", const std::string& deserialize="", const std::string& serialize="",
		const std::string& copy="", const std::string& move="", const std::string& reference="", 
		const std::string& deserializevalue="");

	/// Create a Data Object using a Non-CDMI Content Type
	///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>
	///@param[in] contentType the content type of the data to be stored as a data object.
	///@param[in] value     the data object value
	///@param[in] size      size in bytes of value
	virtual CdmiRetCode nonCdmi_CreateDataObject(const std::string& uri, const std::string& contentType, const char* value="", uint32 size=0);

	/// Read a Data Object using CDMI Content Type
	///@param[out] result   the JSON value returned from the server
	///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>
	///@param[in] location  to indicate the server to respond with the URI that the reference redirects to if the object is a reference.
	virtual CdmiRetCode cdmi_ReadDataObject(Json::Value& result, const std::string& uri, std::string& location);

	/// Read a Data Object using Non-CDMI Content Type
	///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>
	///@param[out] contentType the content type of the data to be stored as a data object.
	///@param[out] location  The server shall respond with the URI that the reference redirects to if the object is a reference
	///@param[in] startOffset the offset to start read
	///@param[in,out] len   to indicate the max bytes can receive as the input parameter; to confirm the actual bytes have been read as the output parameter
	virtual CdmiRetCode nonCdmi_ReadDataObject(const std::string& uri, std::string& contentType, std::string& location, uint64 startOffset, in out uint32& len, char* recvbuff);
	virtual CdmiRetCode nonCdmi_ReadDataObject_direct(const std::string& uri, std::string& contentType, std::string& location, uint64 startOffset, in out uint32& len, char* recvbuff);

	/// Update a Data Object using a Non-CDMI Content Type
	///@param[out] location  The server shall respond with the URI that the reference redirects to if the object is a reference
	///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>
	///@param[in] partial   true to ndicates that the object is in the process of being updated, and has not yet been fully updated. Maps to HTTP header X-CDMI-Partial
	///@param[in] value     The data object value
	///@param[in] valuetransferencoding  the value transfer encoding used for the data object value. Two value transfer encodings are defined.
	///@param[in] domainURI  URI of the owning domain
	///@param[in] deserialize  URI of a serialized CDMI data object that shall be deserialized to create the new data object
	///@param[in] copy       URI of a CDMI data object or queue that shall be copied into the new data object
	///@param[in] deserializevalue a data object serialized as specified in Clause 15 and encoded using base 64 encoding rules described in RFC 4648.
	virtual CdmiRetCode cdmi_UpdateDataObjectEx(out std::string& location, const std::string& uri, const Json::Value& metadata,
		uint64 startOffset, const std::string& value="", int base_version=-1, bool partial=false, const StrList& valuetransferencoding = StrList(),
		const std::string& domainURI="", const std::string& deserialize="", const std::string& copy="",
		const std::string& deserializevalue="");

	virtual CdmiRetCode cdmi_UpdateDataObject(out std::string& location, const std::string& uri, const Properties& metadata,
		uint64 startOffset, const std::string& value="", bool partial=false, const StrList& valuetransferencoding = StrList(),
		const std::string& domainURI="", const std::string& deserialize="", const std::string& copy="",
		const std::string& deserializevalue="");

	/// Update a Data Object using a Non-CDMI Content Type
	///@param[in] uri  the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>
	///@param[out] location  The server shall respond with the URI that the reference redirects to if the object is a reference
	///@param[in] contentType  the content type of the data to be stored as a data object.
	///@param[in] startOffset the offset to start write
	///@param[in] len   the length in bytes to write
	///@param[in] buff  the data to write
	///@param[in] objectSize to indicate the total size of the data object for the purpose of truncate, negative means not specified
	///@param[in] partial   true to ndicates that the object is in the process of being updated, and has not yet been fully updated.  Maps to HTTP header X-CDMI-Partial
	virtual CdmiRetCode nonCdmi_UpdateDataObject(const std::string& uri, out std::string& location, const std::string& contentType, uint64 startOffset, uint len, const char* buff, int64 objectSize=-1, bool partial=false);

	// Delete a Data Object using CDMI Content Type
	///@param[out] result   the JSON value returned from the server
	///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>
	virtual CdmiRetCode cdmi_DeleteDataObject(Json::Value& result, const std::string& uri);

	/// Delete a Data Object using a Non-CDMI Content Type
	///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>
	virtual CdmiRetCode nonCdmi_DeleteDataObject(const std::string& uri);

	virtual CdmiRetCode cdmi_CreateContainer(Json::Value& result, const std::string& uri, const Properties& metadata,
		const Json::Value& exports=Json::Value(),
		const std::string& domainURI="", const std::string& deserialize="", 
		const std::string& copy="", const std::string& move="", const std::string& reference="", 
		const std::string& deserializevalue="");

	/// Create a Container Object using a Non-CDMI Content Type
	///@param[in] uri  the container name appends to the rootUri, will be appended with a '/' if not available
	virtual CdmiRetCode nonCdmi_CreateContainer(const std::string& uri);

	/// Read a Container Object using CDMI Content Type
	///@param[out] result   the JSON value returned from the server
	///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>/
	virtual CdmiRetCode cdmi_ReadContainer(Json::Value& result, const std::string& uri);

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
	virtual CdmiRetCode cdmi_UpdateContainer(Json::Value& result, out std::string& location,
		const std::string& uri, const Properties& metadata,	const Json::Value& exports="",
		const std::string& domainURI="", const std::string& deserialize="", const std::string& copy="",
		const std::string& snapshot="", const std::string& deserializevalue="");

	virtual CdmiRetCode  cdmi_UpdateContainerEx(Json::Value& result, out std::string& location,
		const std::string& uri, const Json::Value& metadata,	const Json::Value& exports="",
		const std::string& domainURI="", const std::string& deserialize="", const std::string& copy="",
		const std::string& snapshot="", const std::string& deserializevalue="");
	/// Delete a Container Object using CDMI Content Type
	///@param[out] result   the JSON value returned from the server
	///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>/
	virtual CdmiRetCode cdmi_DeleteContainer(Json::Value& result, const std::string& uri);

	/// Delete a Container Object using a Non-CDMI Content Type
	///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>/
	virtual CdmiRetCode nonCdmi_DeleteContainer(const std::string& uri);

public:

	static std::string _domainURI, _rootURI;
	static StrList     _serverIPs;
	static int         _portHTTP, _portHTTPS;
	static ZQ::common::Mutex _lkLogin;

	static const char* cdmiRetStr(int retCode);  
	static bool getSysConfig(ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool,const std::string& rootURL, uint16 curlflags = 0x0f);
	static int loginIn(Json::Value& result,ZQ::common::Log& _log, ZQ::common::NativeThreadPool& thrdPool, const std::string& rootUrl, const std::string&userName, const std::string &password, uint16 curlflags = 0x0f, void* pCtx = NULL);

};

#ifndef MAPSET
#  define MAPSET(_MAPTYPE, _MAP, _KEY, _VAL) if (_MAP.end() ==_MAP.find(_KEY)) _MAP.insert(_MAPTYPE::value_type(_KEY, _VAL)); else _MAP[_KEY] = _VAL
#endif // MAPSET

#endif //__CdmiClientBase_H__
