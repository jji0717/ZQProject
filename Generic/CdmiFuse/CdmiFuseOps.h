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
// Ident : $Id: CdmiFuseOps.h Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : common CDMI file operations for FUSE purposes
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/CdmiFuse/CdmiFuseOps.h $
// 
// 133   4/18/17 3:47p Hui.shao
// opendir/readdir
// 
// 132   4/14/17 3:24p Hui.shao
// 
// 131   4/10/17 11:12a Hui.shao
// uncacheChild()
// 
// 130   4/07/17 10:01a Hui.shao
// 
// 129   4/06/17 5:52p Hui.shao
// to reduce multiple concurrent queries for a same file to Aqua
// 
// 128   3/30/17 6:01p Hui.shao
// added readDir()
// 
// 127   3/28/17 5:46p Hui.shao
// _lkLogin
// 
// 126   3/17/17 12:03p Hui.shao
// 
// 125   3/17/17 11:19a Hongquan.zhang
// 
// 124   10/17/16 2:37p Hui.shao
// lower the load to readDomain()
// 
// 123   9/30/16 1:14p Hongquan.zhang
// 
// 122   9/21/16 9:44a Li.huang
// 
// 121   7/18/16 3:52p Hui.shao
// 
// 120   4/18/16 2:02p Hongquan.zhang
// 
// 119   3/09/16 3:39p Hui.shao
// merged from V2.5
// 
// 118   3/02/16 6:33p Hui.shao
// 
// 117   2/04/16 10:15a Hongquan.zhang
// add attrCache_size
// 
// 116   2/03/16 5:19p Hongquan.zhang
// fix CDMI_ACE_READ_ALL flag in CdmiFuse
// 
// 117   2/03/16 5:10p Hongquan.zhang
// fix the CDMI_ACE_READ_ALL flag in CdmiFuse
// 
// 116   1/28/16 3:13p Hui.shao
// 
// 115   1/28/16 1:56p Hui.shao
// adjust ACL value by comparing cdmi_owner
// 
// 114   1/27/16 2:14p Hui.shao
// seperated threadpool for slow SearchByParent
// 
// 113   1/15/16 2:44p Hui.shao
// 
// 112   1/08/16 4:16p Hui.shao
// cache of dir children
// 
// 111   11/12/15 4:47p Hui.shao
// indicateClose must be about DataObject
// 
// 110   8/27/15 5:10p Li.huang
// fix bug 21650
//
// 109   8/20/15 3:04p Li.huang
// fix bug 21651
// 
// 108   8/13/15 2:58p Li.huang
// 
// 107   8/13/15 11:40a Li.huang
// add check root server healthy thread
// 
// 106   7/14/15 10:43a Li.huang
// 
// 105   4/02/15 2:15p Hui.shao
// ReadDomain to call getServerIp() to sort the IPs
// 
// 104   2/15/15 3:02p Hui.shao
// 
// 103   2/12/15 11:29a Hui.shao
// cdmi_IndicateClose()
// 
// 102   1/26/15 12:10p Hui.shao
// retryTimeout
// 
// 101   12/04/14 2:18p Hui.shao
// readAquaDomain() to output the real domainURI of execution
// 
// 100   11/19/14 8:01p Hui.shao
// userDomain
// 
// 99    11/13/14 2:21p Hui.shao
// ticket#16748 to a) cleanup the cache of move-from, b) prefer to take
// the location of move-from to create new move-to
// 
// 98    8/26/14 4:12p Hui.shao
// 
// 97    8/26/14 3:37p Hui.shao
// 
// 96    8/21/14 5:23p Hui.shao
// 
// 95    8/18/14 6:46p Hui.shao
// enable CURL_INSTANCE_KEEPALIVE
// 
// 94    8/14/14 4:46p Hui.shao
// persistent cURL connection when callRemote()
// 
// 93    7/21/14 2:46p Hongquan.zhang
// remove big lock && cache partition by uri
// 
// 92    7/01/14 1:07p Hongquan.zhang
// 
// 91    6/12/14 3:21p Hongquan.zhang
// support dynamic cache size
// 
// 90    6/04/14 6:02p Hui.shao
// stop get PathName from Aqua response
// 
// 88    5/28/14 4:52p Hui.shao
// take common::BufferList
// 
// 86    5/28/14 10:25a Hui.shao
// take smartpointer for contentbuffer 
// 
// 85    5/28/14 9:36a Li.huang
// 
// 83    5/27/14 9:40a Hui.shao
// invoke curl via buffer aggregation
// 
// 82    5/23/14 11:17a Hongquan.zhang
// 
// 81    4/30/14 10:50a Hui.shao
// swapped the low 16bit flag for curl to high-16bit
// 
// 80    4/23/14 5:00p Hui.shao
// more http status codes
// 
// 79    3/24/14 1:29p Ketao.zhang
// check in for cdmifuseLinux clone
// 
// 78    2/26/14 1:04p Hongquan.zhang
// 
// 77    2/26/14 11:19a Hui.shao
// to export the server-side parameters retrived from AquaDomain
// 
// 76    2/24/14 1:47p Hui.shao
// to call AquaClient in a DLL
// 
// 75    11/29/13 4:48p Hui.shao
// json config for sdk
// 
// 74    10/31/13 1:40p Hui.shao
// ST_MODE_BIT_OFFSET_USER
// 
// 73    9/06/13 11:16a Hongquan.zhang
// 
// 72    8/14/13 1:11p Hui.shao
// retry on some special cases of send-request failed
// 
// 71    8/13/13 3:53p Hongquan.zhang
// 
// 70    8/13/13 11:46a Hui.shao
// 
// 69    8/07/13 4:59p Hongquan.zhang
// add new configuration for cachelayer
// 
// 68    7/16/13 5:11p Hui.shao
// query home container for space domain if not yet
// 
// 67    7/16/13 3:38p Hongquan.zhang
// support write back cache
// 
// 66    7/12/13 5:19p Hongquan.zhang
// tmp check in, DO NOT release it out
// 
// 65    7/12/13 9:44a Hui.shao
// for HLSContent
// 
// 64    6/25/13 2:40p Hongquan.zhang
// fix gcc compatible issue
// 
// 63    6/19/13 4:04p Li.huang
// 
// 62    6/13/13 6:09p Hui.shao
// flags
// 
// 61    6/13/13 4:41p Hui.shao
// 
// 60    6/06/13 11:39a Li.huang
// 
// 59    6/04/13 2:34p Ketao.zhang
// 
// 58    6/03/13 7:36p Hui.shao
// ChildReader dispatch to searchByParent() and readByEnumeration() 
// 
// 57    5/30/13 6:36p Hui.shao
// calling the /cdmi_search/? for the children
// 
// 56    5/30/13 5:36p Hui.shao
// 
// 55    5/30/13 11:45a Hui.shao
// sunk fileinfo cache into CdmiFuseOps
// 
// 54    5/23/13 4:54p Hui.shao
// enh#17961 to read the space usage from aqua domain
// 
// 53    5/17/13 12:29p Hongquan.zhang
// support read cache
// 
// 52    5/11/13 3:53p Li.huang
// 
// 51    5/10/13 3:58p Li.huang
// 
// 50    5/09/13 6:25p Hui.shao
// adjustments by execising cdmi_UpdateDataObject() and
// cdmi_ReadDataObject()
// 
// 49    5/09/13 5:34p Hui.shao
// merge generateURL into assembleURL
// 
// 48    5/09/13 5:01p Hui.shao
// x-aqua-redirect-ip and x-aqua-redirect-tag
// 
// 47    5/09/13 4:35p Hui.shao
// 
// 46    5/09/13 3:46p Li.huang
// 
// 45    5/09/13 3:18p Hui.shao
// drafted retry loop
// 
// 44    5/08/13 5:22p Li.huang
// 
// 43    5/07/13 4:45p Li.huang
// 
// 42    5/07/13 2:53p Li.huang
// 
// 41    5/07/13 11:24a Li.huang
// 
// 40    5/06/13 4:42p Li.huang
// get auqa basic conifg
// 
// 39    5/06/13 2:33p Hui.shao
// draft the enh per Aqua configuration and domainURI
// 
// 38    4/19/13 9:25a Li.huang
// 
// 37    4/11/13 3:00p Li.huang
// add S3 
// 
// 36    4/09/13 10:32a Hui.shao
// draft aqua auth
// 
// 35    4/09/13 10:02a Hui.shao
// draft generateSignature()
// 
// 34    4/08/13 11:32a Hui.shao
// 
// 33    2/28/13 2:39p Hongquan.zhang
// 
// 32    2/25/13 8:31p Hui.shao
// 
// 31    2/25/13 4:11p Hui.shao
// separated rootUrl and homeContainer (a sub container under rootUrl)
// 
// 30    2/21/13 4:09p Hui.shao
// 
// 29    2/08/13 10:15a Hui.shao
// 
// 28    2/07/13 5:32p Hui.shao
// 
// 27    1/29/13 5:23p Hongquan.zhang
// 
// 26    1/28/13 3:21p Hui.shao
// Time_t
// 
// 25    1/25/13 4:57p Hongquan.zhang
// 
// 22    1/24/13 4:03p Hui.shao
// 
// 21    1/24/13 4:00p Li.huang
// 
// 20    1/23/13 10:14a Hui.shao
// wrapperd fstat into fileinfo
// 
// 19    1/16/13 10:17a Li.huang
// 
// 18    1/10/13 2:55p Li.huang
// 
// 17    1/10/13 11:16a Hui.shao
// 
// 16    1/09/13 2:58p Li.huang
// add cdmirc code
// 
// 15    1/08/13 9:24a Li.huang
// 
// 14    1/07/13 10:53p Hui.shao
// 
// 13    1/07/13 5:31p Hui.shao
// 
// 12    1/06/13 11:02a Li.huang
// 
// 11    1/06/13 10:49a Hui.shao
// removed _mountPoint
// 
// 10    1/04/13 4:07p Hui.shao
// fstat <-> metadata
// 
// 9     1/04/13 2:57p Hui.shao
// 
// 8     12/27/12 3:56p Hui.shao
// 
// 7     12/27/12 3:36p Hui.shao
// 
// 6     12/27/12 3:21p Hui.shao
// api comments
// 
// 5     12/27/12 11:23a Hui.shao
// listed the cdmi apis
// 
// 4     12/26/12 2:26p Hui.shao
// 
// 3     12/25/12 12:17p Hui.shao
// 
// 2     12/24/12 6:51p Hui.shao
// 
// 1     12/24/12 5:08p Hui.shao
// created
// ===========================================================================

#ifndef __CdmiFuseOps_H__
#define __CdmiFuseOps_H__

#include "ZQ_common_conf.h"
#define _CUSTOM_TYPE_DEFINED_
#include "sdk/AquaClient.h"
#include "Log.h"
#include "NativeThreadPool.h"
#include "CDMIHttpClient.h"
#include "LRUMap.h"
#include "cachelayer.h"

#include <json/json.h>

#include <string>
#include <vector>
#include <map>

extern "C" {
#include <sys/stat.h>
}

#define CURL_INSTANCE_KEEPALIVE

#define CDMI_DATAOBJECT_TYPE   "application/cdmi-object"
#define CDMI_CONTAINER_TYPE    "application/cdmi-container"
#define CDMI_NODE_TYPE         "application/cdmi-node"
#define CDMI_URER_TYPE         "application/cdmi-user"
#define CDMI_CONFIG_TYPE       "application/cdmi-config"
#define CDMI_Version   "1.0"

#define out
#define in

#define DEFAULT_CONTENT_MIMETYPE		"application/octet-stream"
#define DEFAULT_LOGIN_PATH              "aqua/rest/cdmi/cdmi_users/"
#define CHECK_MASTER_PATH				"aqua/rest/cdmi/cdmi_status"		//fdj

#define MIN_BULKET_SIZE (1024*1024)
#define MIN_BYTE_SIZE (MIN_BULKET_SIZE *16)

#define SEND_REQ_MAX_RETRY         (2)
#define SEND_REQ_RETRY_MIN_BODY_SZ (32*1024)
#define CACHED_FILE_INFO_TTL_SEC   (10)       // 10 sec

class AquaClient_API CdmiFuseOps;
class AquaClient_API ChildReader;

class CheckServerHealthy: public ZQ::common::NativeThread
{
public:
	CheckServerHealthy(CdmiFuseOps& cdmiOps, size_t checkTimes = 10);	//fdj
	~CheckServerHealthy();						//fdj
	int		run();								//fdj
	void	quit();								//fdj
	void	notify();
protected:
	
	CdmiFuseOps& _cdmiOps;
	bool	_bQuit;								//fdj
	size_t	_CheckTimes;						//fdj

	SYS::SingleObject _hNotify;					//fdj
	bool	checkHealth();						//fdj
};

class CacheTank : public CacheLayer::DataTank
{
public:
	CacheTank( ZQ::common::NativeThreadPool& readPool,ZQ::common::NativeThreadPool& writePool, ZQ::common::Log& logger, CdmiFuseOps& ops, CacheLayer::DataTankConf& conf);
	virtual ~CacheTank();
private:
	virtual	int directWrite( const std::string& filename, const char* buf, unsigned long long offset , size_t& size ) ;

	virtual	int	directRead( const std::string& filename, char* buf, unsigned long long offset, size_t& size ) ;

	virtual	int	directRead( const std::string& filename, unsigned long long offset, const std::vector<CacheLayer::DataBuffer>& bufs, size_t& sizeTotal );

	virtual int	directWrite( const std::string& filename, const std::vector<CacheLayer::DataBuffer>& bufs, size_t& sizeTotal);

	virtual bool isSuccess( int err , size_t* size = 0 ) const ;
private:
	CdmiFuseOps&		mOps;
};

struct FuseOpsConf
{
	FuseOpsConf()
	{
		enableCache	= 1;
		fuseFlag = 0x0f;
		attrCache_TTLsec = attrCache_childrenTTLsec = CACHED_FILE_INFO_TTL_SEC; // in sec
		attrCache_size = 10 * 1000;
		attrCache_byList = 1;
	}

	int32						enableCache;
	int32						fuseFlag;
	CacheLayer::DataTankConf	tankConf;
	int32                       attrCache_TTLsec;
	int32                       attrCache_childrenTTLsec;
	int32						attrCache_size;
	int32 						attrCache_byList;
};

// -----------------------------
// class CdmiFuseOps
// -----------------------------
/// This class impls the common CDMI operations interact with the CDMI compatible FrontEnd
/// The various FUSE implemenations inherits this may invoke these basic operations
class CdmiFuseOps
{
	friend class ChildReader;

public:
	enum {
		// the high 16bits were reserved for curl client
		flgDumpMsgBody  =FLAG(0),
		flgHexDump      =FLAG(1),

	};

	typedef std::map <std::string, std::string> Properties;
	typedef std::vector <std::string> StrList;

	CdmiFuseOps(ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, const std::string& rootUrl, const std::string& userDomain, const std::string& homeContainer, 
		   const uint32 flags = 0x0f, const FuseOpsConf& conf = FuseOpsConf(), const std::string& bindIp = "0.0.0.0", const uint retryInterval = 50);

	virtual ~CdmiFuseOps();

	virtual void setAccount(const std::string& serverLogin, const std::string& userDomain, const std::string& password="", const std::string& localRunAs="");
	void setTimeout(uint connectTimeout, uint operationTimeout, uint retryTimeout=0);

	virtual void fixupPathname(std::string& pathname) {}
//	virtual std::string generateURL(const std::string& uri, bool bContainer = false, bool includeUsername =false);
	virtual std::string pathToUri(const std::string& pathname);

	typedef enum _CdmiErrors
	{
		cdmirc_OK                 = 200,
		cdmirc_Created            = 201,
		cdmirc_Accepted           = 202, 
		cdmirc_NoContent          = 204,
		cdmirc_PartialContent     = 206,

		cdmirc_Found              = 302,

		cdmirc_BadRequest         = 400,
        cdmirc_Unauthorized       = 401,
		cdmirc_Forbidden          = 403,
		cdmirc_NotFound           = 404,
		cdmirc_NotAcceptable      = 406,
		cdmirc_Conflict           = 409,
		cdmirc_LengthRequired     = 411,
		cdmirc_UriTooLong         = 414,
		cdmirc_UnknownMediaType   = 415,
		cdmirc_InvalidRange       = 416,

		cdmirc_ServerError        = 500,
		cdmirc_NotImplemented     = 501,
		cdmirc_ServerUnavailable  = 503,
		cdmirc_HttpVersion        = 505,

		cdmirc_ExtErr             = 700,
		cdmirc_RequestFailed,
		cdmirc_RequestTimeout,
		cdmirc_AquaLocation       = 732,
		cdmirc_RetryFailed,

		cdmirc_SDK_Error, // errors determined by client SDK
		cdmirc_SDK_BadClient,
		cdmirc_SDK_BadArgument,
		cdmirc_SDK_BadReturn,
		cdmirc_SDK_Unsupported,
		
		cdmirc_MAX
	} CdmiErrors;

	typedef int CdmiRetCode;

#define CdmiRet_SUCC(_cdmiRetCode) (_cdmiRetCode >=200 && _cdmiRetCode< 300)
#define CdmiRet_FAIL(_cdmiRetCode) !CdmiRet_SUCC(_cdmiRetCode)
#define CdmiRet_ClientErr(_cdmiRetCode) (_cdmiRetCode >=400 && _cdmiRetCode< 500)
#define CdmiRet_ServerErr(_cdmiRetCode) (_cdmiRetCode >=500 && _cdmiRetCode< 700)
#define CdmiRet_CommunicationErr(_cdmiRetCode) (cdmirc_RequestFailed == _cdmiRetCode || cdmirc_RequestTimeout == _cdmiRetCode)

	enum _ACE_Const {
		// acetypes
		CDMI_ACE_ACCESS_ALLOWED_TYPE  = 0x00,
		CDMI_ACE_ACCESS_DENIED_TYPE   = 0x01,
		CDMI_ACE_SYSTEM_AUDIT_TYPE    = 0x02,

		// aceflags
		CDMI_ACE_OBJECT_INHERIT_ACE   = 0x01,
		CDMI_ACE_CONTAINER_INHERIT_ACE= 0x02,
		CDMI_ACE_NO_PROPAGATE_INHERIT_ACE=0x04,
		CDMI_ACE_INHERIT_ONLY_ACE     =0x08,

		// acemasks
		CDMI_ACE_READ_OBJECT          = 0x00000001, // Permission to read the value of a data object
		CDMI_ACE_LIST_CONTAINER       = 0x00000001, // Permission to list the children of a container object
		CDMI_ACE_WRITE_OBJECT         = 0x00000002, // Permission to modify the value of a data object
		CDMI_ACE_ADD_OBJECT           = 0x00000002, // Permission to add a new child data object or queue object to a container object
		CDMI_ACE_APPEND_DATA          = 0x00000004, // Permission to append data to the value of a data object
		CDMI_ACE_ADD_SUBCONTAINER     = 0x00000004, // Permission to create a child container object in a container object
		CDMI_ACE_READ_METADATA        = 0x00000008, // Permission to read non-ACL metadata of an object
		CDMI_ACE_WRITE_METADATA       = 0x00000010, // Permission to write non-ACL metadata of an object
		CDMI_ACE_EXECUTE              = 0x00000020, // Permission to execute an object
		CDMI_ACE_DELETE_OBJECT        = 0x00000040, // Permission to delete a child data object or queue object from a container object
		CDMI_ACE_DELETE_SUBCONTAINER  = 0x00000040, // Permission to delete a child container object from a container object
		CDMI_ACE_READ_ATTRIBUTES      = 0x00000080, // Permission to read non-metadata and non-value/children fields of an object
		CDMI_ACE_WRITE_ATTRIBUTES     = 0x00000100, // Permission to change non-metadata and non-value/children fields of an object
		CDMI_ACE_WRITE_RETENTION      = 0x00000200, // Permission to change retention attributes of an object
		CDMI_ACE_WRITE_RETENTION_HOLD = 0x00000400, // Permission to change hold attributes of an object
		CDMI_ACE_DELETE               = 0x00010000, // Permission to change hold attributes of an object
		CDMI_ACE_READ_ACL             = 0x00020000, // Permission to Read the ACL of an object
		CDMI_ACE_WRITE_ACL            = 0x00040000, // Permission to Write the ACL of an object
		CDMI_ACE_WRITE_OWNER          = 0x00080000, // Permission to change the owner of an object
		CDMI_ACE_SYNCHRONIZE          = 0x00100000, // Permission to access an object locally at the server with synchronous reads and writes

		// additional flags for those stupid "ALL"s
		CDMI_ACE_ALL_PERMS            = 0x001F07FF,
		CDMI_ACE_READ_ALL             = CDMI_ACE_READ_OBJECT | CDMI_ACE_READ_METADATA | CDMI_ACE_READ_ATTRIBUTES | CDMI_ACE_READ_ACL,
		CDMI_ACE_RW_ALL               = 0x000601DF | CDMI_ACE_DELETE,

	} ACE_Const;

	typedef struct _FlagID
	{
		uint32  flag;
		const char*   name;
	} FlagID;

	const static FlagID ACE_FlagMasks[];
	const static FlagID ACE_FlagTypes[];
	const static FlagID ACE_FlagFlags[];

	typedef struct _FileInfo
	{
#ifdef ZQ_OS_MSWIN
		struct _stat64 filestat;
#elif defined ZQ_OS_LINUX
#define _S_IFDIR S_IFDIR
		struct stat    filestat;
#endif//ZQ_OS
		int64  stampInfoKnown;  // timestamp
		int    revision; // http://192.168.87.16/mediawiki/index.php/Specify_object/container_version_-_CDMI_-_Function_Design_-_Aqua

		std::string owner;
		//TODO other extended file informations
	} FileInfo;

	static inline void FileInfo_reset(FileInfo& fi)
	{
		memset(&fi.filestat, 0, sizeof(fi.filestat));
		fi.stampInfoKnown = fi.revision =0;
		fi.owner = "";
	}

	typedef enum{
		CDMI_DATAOBJECT, CDMI_FILE =CDMI_DATAOBJECT,
		CDMI_CONTAINER,
		CDMI_NODE
	} ObjectType;

	typedef struct _ResourceLocation
	{
		std::string path, locationIp, paramsAppended;
		int64  stampInfoKnown;  // timestamp
	} ResourceLocation;

	typedef ZQ::common::LRUMap <std::string, ResourceLocation> LocationCache; // cache of uri-path to location

protected: // about the configurations
	std::string  _rootUrl, _homeContainer;
	FuseOpsConf	 _fuseOpsConf;
	uint32  _flags; // the lower 16bits is curl flags 

	std::string  _username, _password, _localRunAs;
	bool         _mdOwnerInEmailFmt;
	std::string  _domainURIOfMP, _userDomain;

	uint  _connectTimeout, _operationTimeout, _retryTimeout;
	uint  _retryInterval;

protected: // convert json data to FUSE datatypes

	virtual bool dataObjMetadataToFstat(const std::string& filename, const Json::Value& metadata, in out FileInfo& fileInfo);
	virtual bool addObjMetadataByFstat(Json::Value& metadata, in const FileInfo& fileInfo);

	virtual int parseDataObject(const std::string& path, const Json::Value& jsonDataObj, in out FileInfo& fileInfo, const char* logHint);
	std::string userString();

	ZQ::common::Log& _log;
	ZQ::common::NativeThreadPool& _thrdPool;
	ZQ::common::NativeThreadPool  _slowThrdPool;
	std::string _accessKeyId, _secretKeyId;

	bool _rootOk;			//fdj: test the next server'status 
	CheckServerHealthy*  _pCheckHealthyTrd;
    size_t				_idxServer;

//	std::string getRootURL(const std::string rootURL, bool next=false);
	std::string assembleURL(const std::string& serverIp, const std::string& uri, ObjectType objType = CDMI_FILE, bool includeUsername =false);
	
	void cacheLocation(const ResourceLocation& resLoc);
	void uncacheLocation(const std::string path);

	int  callRemote(const char* purposeStr, std::string uri, std::string& finalURL, std::string& respStatusLine, ObjectType objType, bool includeUsername,
		const Properties& requestHeaders, const ZQ::common::BufferList::Ptr pRequestBody,
		Properties& responseHeaders, in out ZQ::common::BufferList::Ptr pResponseBody,
		ZQ::common::CURLClient::HTTPMETHOD method, int clientFlags, std::string uriAffinity="");

	int  callRemote(const char* purposeStr, std::string uri, std::string& finalURL, std::string& respStatusLine, ObjectType objType, bool includeUsername,
		const Properties& requestHeaders, const std::string& requestBody, const char* reqBodyBuf, int reqBodyLen,
		Properties& responseHeaders, std::string& responseTxtBody, in out uint& len, char* recvbuff,
		ZQ::common::CURLClient::HTTPMETHOD method, int clientFlags, std::string uriAffinity="");

	uint getCurrentServerIdx();

	CdmiRetCode getFileInfo(const std::string& pathname, FileInfo& fi, bool skipGuessContainer);

	typedef struct _DirChildren
	{
		int64 stampOpen, stampLast;
		std::string dirObjectId;
		StrList _children;
	} DirChildren;

	typedef ZQ::common::LRUMap <std::string, DirChildren> DirChildrenMap; // cache of uri-path to children

	// about file info cache
	typedef ZQ::common::LRUMap< std::string, FileInfo > FileInfoMap;
	FileInfoMap    _fileInfos;
	DirChildrenMap _dirChildrens;
	ZQ::common::Mutex _lkFileInfos;

	// typedef std::map< int, ZQ::CDMIClient::CDMIHttpClient::Ptr > ClientMap;
	// ClientMap _clientMap;
	typedef std::list < ZQ::CDMIClient::CDMIHttpClient::Ptr > ClientList;
	ClientList _clients;
	ZQ::common::Mutex _lkClients;
	std::string    _bindIp;

	bool readFileInfoFromCache(const std::string& filepath, FileInfo& fileInfo);
	void cacheFileInfo(const std::string& filepath, const FileInfo& fileInfo);
	void uncacheFileInfo(const std::string& filepath);

	void cacheChildren(const std::string& path, DirChildren& children);
	// void uncacheChildren(const std::string& path);
	void stampChildren(const std::string& pathname);
	DirChildren getCachedChildren(const std::string& path);
	void cacheChild(const std::string& pathname); // cache a single child
	void uncacheChild(const std::string& pathname); // uncache a single child from the children list

	// callbacks of ChildReader
	virtual void OnChildInfo(const std::string& pathName, FileInfo& fi, const std::string& txnId) {}
	virtual void OnChildReaderStopped(int readerId, bool bCancelled, const std::string& txnId) {}

	ZQ::CDMIClient::CDMIHttpClient::Ptr openClient(const std::string& forURL, int flag =0, ZQ::common::CURLClient::HTTPMETHOD method =ZQ::common::CURLClient::HTTP_GET);
	void cacheClient(ZQ::CDMIClient::CDMIHttpClient::Ptr& pClient);

public:
	CdmiRetCode getDiskSpace(int64& freebytes, int64& totalbytes);
	std::string getAccessKey() { return _accessKeyId; }
	bool generateSignature(std::string& signature, const std::string& uri, const std::string& contentType, ZQ::common::CURLClient::HTTPMETHOD method, const std::string& xAquaDate);
	std::string getRelocatedURL(std::string& uri, const ResourceLocation& loc, ObjectType objType, bool includeUsername);
	std::string getServerIp(bool next=false, bool forceReadConfig=false);
	bool readLocationFromCache(const std::string path, ResourceLocation& resLoc);
	void setCache( bool enable );
	CdmiRetCode isExist(Json::Value& result, const std::string& uri, std::string& contentType);
	CdmiRetCode openDir(const std::string& dirPath);
	CdmiRetCode readDir(const std::string& dirPath, DirChildren& dc);

protected:
	CacheTank*	getCacheTank( const std::string& uri);

private:
	LocationCache     _locationCache;
    ZQ::common::Mutex _lkLocationCache;
    uint              _currentServerIdx;
	std::vector<CacheTank*>	_cacheTanks;
	ZQ::common::NativeThreadPool* mWriteBufferPool;
	//CacheTank		  _cacheTank;

public:
	static const char* cdmiRetStr(int retCode);  
	 // CdmiRetCode toCdmiRC(int retCode);
#ifdef ZQ_OS_MSWIN
	typedef __time64_t Time_t;
	static __time64_t time2time_t(int64 t);
	static int64      time_t2time(__time64_t t64);
#else
	typedef  long long Time_t;

#endif//ZQ_OS

public: // implementation of CDMI APIs
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
		const std::string& deserializevalue="", bool fastClone=false);

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
	virtual CdmiRetCode nonCdmi_ReadDataObject(const std::string& uri, std::string& contentType, std::string& location, uint64 startOffset, in out uint32& len, char* recvbuff, bool disableCache = false);
	virtual CdmiRetCode nonCdmi_ReadDataObject_direct(const std::string& uri, std::string& contentType, std::string& location, uint64 startOffset, in out uint32& len, char* recvbuff);
	virtual CdmiRetCode nonCdmi_ReadDataObject_direct(const std::string& uri, std::string& contentType, std::string& location, uint64 startOffset, const std::vector<CacheLayer::DataBuffer>& bufs, size_t& sizeTotal);

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
	virtual CdmiRetCode nonCdmi_UpdateDataObject(const std::string& uri, out std::string& location, const std::string& contentType, uint64 startOffset, uint len, const char* buff, int64 objectSize=-1, bool partial=false, bool disableCache = false);
	virtual CdmiRetCode nonCdmi_UpdateDataObject_direct(const std::string& uri, out std::string& location, const std::string& contentType, uint64 startOffset, uint len, const char* buff, int64 objectSize=-1, bool partial=false);
	virtual CdmiRetCode nonCdmi_UpdateDataObject_direct(const std::string& uri, out std::string& location, const std::string& contentType, const std::vector<CacheLayer::DataBuffer>& bufs , uint len, int64 objectsize = -1, bool partial=false);


	// Delete a Data Object using CDMI Content Type
	///@param[out] result   the JSON value returned from the server
	///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>
	virtual CdmiRetCode cdmi_DeleteDataObject(Json::Value& result, const std::string& uri);

	/// Delete a Data Object using a Non-CDMI Content Type
	///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>
	virtual CdmiRetCode nonCdmi_DeleteDataObject(const std::string& uri);

	/// Create a Container Object using CDMI Content Type
	///@param[out] result   the JSON value returned from the server
	///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>/
	///@param[in] metadata   metadata for the container object
	///@param[in] exports    a structure for each protocol enabled for this container object
	///@param[in] domainURI  URI of the owning domain
	///@param[in] deserialize  URI of a serialized CDMI container object that shall be deserialized to create the new container object
	///@param[in] serialize  URI of a CDMI container that shall be serialized into the new container object
	///@param[in] copy       URI of a CDMI container object or queue that shall be copied into the new container object
	///@param[in] move       URI of an existing local or remote CDMI container object (source URI) that shall be relocated to the URI specified in the PUT
	///@param[in] reference  URI of a CDMI container object that shall be redirected to by a reference.
	///@param[in] deserializevalue A container object serialized as specified in Clause 15 and encoded using base 64 encoding rules described in RFC 4648.
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

	/// Delete a Container Object using CDMI Content Type
	///@param[out] result   the JSON value returned from the server
	///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>/
	virtual CdmiRetCode cdmi_DeleteContainer(Json::Value& result, const std::string& uri);

	/// Delete a Container Object using a Non-CDMI Content Type
	///@param[in] uri       the uri appends to the rootUri, the URI on the output going HTTP request would be formatted as <rootURI> <uri>/
	virtual CdmiRetCode nonCdmi_DeleteContainer(const std::string& uri);

public:	// extended Aqua API
	//----------------------------------------------------------------
	/// read the Aqua domain extersion
	///@param[out] result      the json value parsed from the response body
	///@param[in, out] domainUri  [in] "" to read the domain of the homecontainer
	virtual CdmiRetCode cdmi_ReadAquaDomain(Json::Value& result, std::string& domainUri);

	/// indicate Aqua about a file close, so that the latter can flush the cached data
	///@param[in uri           uri of file to close
	virtual CdmiRetCode cdmi_IndicateClose(const std::string& uri);

	virtual CdmiRetCode flushdata( const std::string& uri );


public:
	typedef struct _ServerSideParams
	{
		std::string domainURI, rootURI;
		int         portHTTP, portHTTPS;
		CdmiFuseOps::StrList   serverIPs;
	} ServerSideParams;

	void getServerSideParams(ServerSideParams& ssp) const;
	bool isHealthy();
	void notifyConnected();

public:
	
	static int login(Json::Value& result, ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, const std::string& rootUrl, const std::string&userName, const std::string& userDomain, const std::string &password, uint16 curlflags = 0x0f, void* pCtx = NULL);
	static int getContainer(Json::Value& result, ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, const std::string &rootUrl, uint16 curlflags = 0x0f, std::string bindIp = "0.0.0.0");
	
	static uint32 readFlags(const Json::Value& value, const CdmiFuseOps::FlagID flagIds[], uint32 defaultVal=0);
	static std::string timeToAquaData(int64 time);

	// about configuration in json string
	static void readEnvConfig(const Json::Value& jsonConfig, std::string& logdir, int& loglevel, long& logsize, int& logcount, int& threads);
	static void readClientConfig(const Json::Value& jsonConfig, FuseOpsConf& clientConf);

	// about CURL env
	static void startCURLenv();
	static void stopCURLenv();

private: // cache of getSpace
	int64 _stampDomainReadStart, _stampDomainAsOf;
	int64 _domainFreeBytes, _domainTotalBytes;

	// thread unsafe
	static bool _getServerSideConfig(ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, const std::string& rootURL, uint16 curlflags = 0x0f, std::string bindIp = "0.0.0.0");
};

// -----------------------------
// class ChildReader
// -----------------------------
/// This utility class reads a list of children of a container, feedback by invoking callbacks 
///	CdmiFuseOps::OnChildInfo() and CdmiFuseOps::OnChildReaderStopped()
class ChildReader : public ZQ::common::ThreadRequest
{
public:
	ChildReader(CdmiFuseOps& fuse, int Id, const std::string& parentId, const std::string& parentName, 
		const CdmiFuseOps::StrList& children2read, 
		const std::string& txnId, 
		int rangeStart=0, int rangeEnd=-1, const char* acceptType =CDMI_NODE_TYPE);
	virtual ~ChildReader() {}

	void stop() { _bQuit =true; }

protected:

	CdmiFuseOps::StrList _children2read;
	CdmiFuseOps& _fuse;
	std::string  _parentId, _txnId, _uri, _parentName;
	bool         _bQuit;
	int          _Id;
	std::string  _acceptType;

protected:
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);

	int searchByParent(void);
	int readByEnumeration(void);
};

#ifndef MAPSET
#  define MAPSET(_MAPTYPE, _MAP, _KEY, _VAL) if (_MAP.end() ==_MAP.find(_KEY)) _MAP.insert(_MAPTYPE::value_type(_KEY, _VAL)); else _MAP[_KEY] = _VAL
#endif // MAPSET

#define LOGL_COND_WARN(_Cond) ((_Cond)?ZQ::common::Log::L_WARNING : ZQ::common::Log::L_DEBUG)
#define COPY_METADATA_VAL(var, metadataset, key, valtype)   if (metadataset.isMember(#key)) { var = metadataset[#key].as##valtype(); }
#define COPY_METADATA_TIMET(var, metadataset, key)          if (metadataset.isMember(#key)) { int64 t = ZQ::common::TimeUtil::ISO8601ToTime(metadataset[#key].asString().c_str()); var = CdmiFuseOps::time2time_t(t); }

#define ST_MODE_BIT_OFFSET_USER    (6)
#define ST_MODE_BIT_OFFSET_GROUP   (3)
#define ST_MODE_BIT_OFFSET_OTHER   (0)

#endif // __CdmiFuseOps_H__
