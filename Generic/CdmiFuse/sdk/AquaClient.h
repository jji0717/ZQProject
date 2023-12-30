#ifndef __CDMIFUSE_AQUA_CLIENT_H__
#define __CDMIFUSE_AQUA_CLIENT_H__

#include <errno.h>
#include <map>
#include <string>
#include <vector>

#ifdef ZQ_OS_MSWIN
// AquaClient_API
// -------------
#ifdef AquaClient_EXPORTS
#  define AquaClient_API __declspec(dllexport)
#else
#  define AquaClient_API __declspec(dllimport)
#  pragma comment(lib, "AquaClient.lib")
#endif

#else

#define AquaClient_API 

#endif // ZQ_OS_MSWIN

#ifdef _LINUX_
#	ifndef _CUSTOM_TYPE_DEFINED_
#		define _USE_NEW_TYPE_DEFIEND
#	else
#		undef _USE_NEW_TYPE_DEFIEND
#	endif
#else
#	define _USE_NEW_TYPE_DEFIEND
#endif

#ifdef _USE_NEW_TYPE_DEFIEND

#if defined(_WIN32) || defined(WIN64) || defined(WIN32) || defined(_Windows) || defined(__MINGW32__)
   typedef unsigned int     uint;
   typedef unsigned __int8  uint8;
   typedef unsigned __int16 uint16;
   typedef unsigned __int32 uint32;
   typedef unsigned __int64 uint64;

#else // non-Windows
   typedef unsigned int       uint;
   typedef unsigned char      uint8;
   typedef unsigned short     uint16;
   typedef unsigned int       uint32;
   typedef unsigned long long uint64;
#endif // Other OS

#endif

namespace XOR_Media {
namespace AquaClient {

class AquaClient_API AquaClient;

// -----------------------------
// ioctl extension for cdmifuse file system
// -----------------------------

#define CDMIFUSE_CLONE_MAX_PATH        (512)
#define CDMIFUSE_CLONE_FLG_FASTCLONE   (1<<0) // 1 in this flag to perform a fast clone, 0 to perform full clone

typedef struct _CDMIFUSE_CLONE_REQ
{
	uint16 version; // should be 0
	uint16 cloneFlags;   // flags of CDMIFUSE_CLONE_FLG_???

	char destPath[CDMIFUSE_CLONE_MAX_PATH]; // the relative path under FUSE mount point, UTF-8 encoded
} CDMIFUSE_CLONE_REQ;

#define CDMIFUSE_CLONE_STATUS_COMPLETED   (0)  // the file clone has been completed already
#define CDMIFUSE_CLONE_STATUS_PROCESSING  (1)  // the file clone is being processed
#define CDMIFUSE_CLONE_STATUS_ERROR       (2)  // the file clone has been terminated with error occured

typedef struct _CDMIFUSE_CLONE_STATUS
{
	uint16 version; // should be 0
	uint16 status;  // CDMIFUSE_CLONE_STATUS_???
	uint64 processedSize; // the cloned file size in bytes
	uint8  percentCompleted; // 0~100
} CDMIFUSE_CLONE_STATUS;


typedef struct _CDMIFUSE_QoS_PARAMS
{
	uint16 version; // should be 0
	uint32 throughputKBps;  // the per replica throughput limitation in kilo-byte per second
} CDMIFUSE_QoS_PARAMS;

#define CDMIFUSE_IOCTL_BASECODE               101
#define CDMIFUSE_IOCTL_CODE_CLONE_FILE          1
#define CDMIFUSE_IOCTL_CODE_STATUS_QEURY        2
#define CDMIFUSE_IOCTL_CODE_QOS                 3

#define CDMIFUSE_CMD_CLONE_FILE     _IOW(CDMIFUSE_IOCTL_BASECODE, CDMIFUSE_IOCTL_CODE_CLONE_FILE, CDMIFUSE_CLONE_REQ)
#define CDMIFUSE_CMD_CLONE_STATUS   _IOWR(CDMIFUSE_IOCTL_BASECODE, CDMIFUSE_IOCTL_CODE_STATUS_QEURY, CDMIFUSE_CLONE_STATUS)
#define CDMIFUSE_CMD_CLONE_QOS		_IOW(CDMIFUSE_IOCTL_BASECODE, CDMIFUSE_IOCTL_CODE_QOS, CDMIFUSE_QoS_PARAMS)
// -----------------------------
// class AquaClient
// -----------------------------
/// This class impls the common CDMI operations interact with the CDMI compatible FrontEnd
/// The various FUSE implemenations inherits this may invoke these basic operations
class AquaClient
{
public:

	typedef std::map <std::string, std::string> Properties;
	typedef std::vector <std::string> StrList;

	/**
	 * Initiliaze this SDK with properties
	 * @param settings the properties to initialize the SDK<p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;the following are the defined properties:
	 * <table bolder='1'><tr><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td><th width=100>Property</th><th>ValueType</th><th>Description</th></tr>
	 * <tr><td></td><td>log.dir</td><td>String</td><td>The path name of the folder under where the log files of this SDK will be put</td></tr>
	 * <tr><td></td><td>log.level</td><td>int</td><td>The integer level to print log: 0-none, 3-ERROR, 4-WARNING, 6-INFO, 7-DEBUG</td></tr>
	 * <tr><td></td><td>log.size</td><td>int</td><td>The byte size limitation of each log file, minimal allowed 100KB </td></tr>
	 * <tr><td></td><td>log.count</td><td>int</td><td>The maximal count of log files to keep</td></tr>
	 * <tr><td></td><td>threads</td><td>int</td><td>The size of the processing thread pool</td></tr>
	 * </table>
	 * @return true if successful
	 */
	static bool initSDK(const std::string& jsonSettings);

	/**
	 * Uninitiliaze this SDK
	 * This will lead the SDK to close its log file and threads
	 */
	static void uninitSDK();

	/**
	 * Create a client connects to the Aqua front-ends.
	 * @param rootURL       to specify the root URL to connect to the Aqua front-end, in the format of <u>http://&lt;username&gt;:&lt;password&gt;@&lt;server&gt;:&lt;port&gt;/aqua/rest/cdmi.
	 * @param homeContainer to specify the home container under the root URL, all the operations of the client will be limited under such a container
	 * @param jsonProps     to specify additional properties of the client<p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;The following are the defined properties:
	 * <table bolder='1'><tr><td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td><th width=100>Property</th><th>ValueType</th><th>Description</th></tr>
	 * <tr><td></td><td>cache.enable</td><td>int</td><td>Non-zero to enable the readahead and writeback data cache</td></tr>
	 * <tr><td></td><td>cache.block.size</td><td>int</td><td>The byte size of cache blocks</td></tr>
	 * <tr><td></td><td>cache.block.count.read</td><td>int</td><td>The total amount of blocks that reserved for all data read purposes</td></tr>
	 * <tr><td></td><td>cache.block.count.write</td><td>int</td><td>The total amount of blocks that reserved for all write read purposes</td></tr>
	 * <tr><td></td><td>cache.block.count.readahead</td><td>int</td><td>The amount of blocks that will be read ahead before use for each data object</td></tr>
	 * <tr><td></td><td>cache.block.ttl.read</td><td>int</td><td>The time-to-live(TTL) in millisecond of the cached blocks for read. The blocks will be dropped when expired</td></tr>
	 * <tr><td></td><td>cache.block.ttl.write</td><td>int</td><td>The time-to-live(TTL) in millisecond of the cached blocks for write. The blocks will be flushed when expired</td></tr>
	 * <tr><td></td><td>cache.log.flags</td><td>int</td><td>The flags how to print the activities of the cache module, reserved</td></tr>
	 * <tr><td></td><td>cache.threads.flush</td><td>int</td><td>The number of threads reserved on submitting write data to Aqua front-ends</td></tr>
	 * <tr><td></td><td>client.flags</td><td>int</td><td>The flags of client, reserved</td></tr>
	 * </table>
	 * @return the client instance if successful, otherwise null
	 * @see initSDK() initSDK() must be called before instanize any AquaClients.<p>
	 * @see uinitSDK() when all the AquaClient instances are no more used, uninitSDK() should be called before program quits.
	 */
	static AquaClient* newClient(const std::string& rootURL, const std::string& userDomain, const std::string& homeContainer, const std::string& jsonProps);

	void release(void);

	/**
	* Create a Data Object Using CDMI Content Type
	* @param jsonResult   the output text formatted JSON value returned from the server
	* @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	* @param mimetype     to specify the MIME type of the data contained within the value field of the data object
	* @param jsonMetadata to specify the initial metadata, formatted in JSON text, Metadata for the data object
	* @param value        to specify the initial data object value
	* @param valuetransferencoding to specify the value transfer encoding used for the data object value
	* @param domainURI    to specify the URI of the owner's
	* @param deserialize  to specify the URI of a serialized CDMI data object that shallbe deserialized to create the new data object
	* @param serialize    to specify the URI of a serialize URI of a CDMI object that shall be serialized into the new data object
	* @param copy         to specify the URI of a CDMI data object or queue that shall be copied into the new data object
	* @param move         to specify the URI of an existing local or remote CDMI data object
	* @param reference    to specify the URI of a CDMI data object that shall be redirected to by a reference.
	* @param deserializeValue to specify a data object serialized as specified in Clause 15 and encoded using base 64 encoding rules described in RFC 4648.
	* @return HTTP status responded from the server
	*/
	int cdmi_CreateDataObject(std::string& jsonResult, const std::string& uri,
		const std::string& mimetype, const std::string& jsonMetadata, const std::string& value,
		const StrList& valuetransferencoding, const std::string& domainURI,
		const std::string& deserialize, const std::string& serialize, const std::string& copy, const std::string& move,
		const std::string& reference, const std::string& deserializeValue);

	/**
	* Create a Data Object using a Non-CDMI Content Type
	* @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	* @param contentType  to specify the content type of the data to be stored as a data object.
	* @param value        to specify the initial data object value, range [0, value.limit()) will be taken as the available data to post to the server
	* @return HTTP status responded from the server
	*/
	int nonCdmi_CreateDataObject(const std::string& uri, const std::string& contentType, const char value[], uint32 len);

	/**
	 * Read a Data Object using CDMI Content Type
	 * @param jsonResult  the output text formatted JSON value returned from the server
	 * @param uri         to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @param location    the output location URI that indicates the reference redirecting to if the object is a reference.
	 * @return HTTP status responded from the server
	 */
	int cdmi_ReadDataObject(std::string& jsonResult, const std::string& uri,std::string& location);
	
	/**
	 * Read a Data Object using Non-CDMI Content Type
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @param contentType  the output content type of the data that the data object contains.
	 * @param location     the output location URI that indicates the reference redirecting to if the object is a reference.
	 * @param startOffset  to specify the offset to start reading
	 * @param byteRead     the output number of bytes that have been read
	 * @param buffer       the buffer that the read data would be filled to. The space of this buffer is required to be direct-allocated and the capacity of the buffer will be taken as the maximal bytes that can be filled
	 * @param direct	   true if want to skip the built-in cache layer
	 * @return HTTP status responded from the server
	 */
	int nonCdmi_ReadDataObject(const std::string& uri, std::string& contentType,
		std::string& location, long startOffset, long& byteRead,
			char buffer[], uint32 len, bool direct);
	/**
	 * Update a Data Object using a CDMI Content Type
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @param location     the output location URI that indicates the reference redirecting to if the object is a reference.
	 * @param jsonMetadata to specify the metadata of the data object, formatted in JSON text
	 * @param startOffset  to specify the offset to start writting
	 * @param value        to specify the value of data object, range [0, value.limit()) will be taken as the available data to post to the server
	 * @param base_version to specify the version of the data object that will be based on to modify, server may reject the update if there is version conflicts
	 * @param partial      filling true to indicates that the object is in the process of being updated, and has not yet been fully updated. Maps to HTTP header X-CDMI-Partial
	 * @param valuetransferencoding to specify the value transfer encoding used for the data object value.
	 * @param domainURI    to specify the URI of the owning domain
	 * @param deserialize  to specify the URI of a serialized CDMI data object that shall be deserialized to create the new data object
	 * @param copy         to specify the URI of a CDMI data object or queue that shall be copied into the new data object
	 * @param deserializeValue to specify a data object serialized as specified in Clause 15 and encoded using base 64 encoding rules described in RFC 4648.
	 * @return HTTP status responded from the server
	 */
	int cdmi_UpdateDataObject(const std::string& uri, std::string& location, const std::string& jsonMetadata,
		long startOffset, const std::string& value, int base_version, bool partial, const StrList& valuetransferencoding,
		const std::string& domainURI, const std::string& deserialize, const std::string& copy,
		const std::string& deserializevalue);

		/**
	 * Update a Data Object using a Non-CDMI Content Type
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @param location     the output location URI that indicates the reference redirecting to if the object is a reference.
	 * @param contentType  to specify the content type of the data to be stored as a data object.
	 * @param startOffset  to specify the offset to start updating from
	 * @param objectSize   to indicate the total size of the data object for the purpose of truncating, negative means is not specified
	 * @param buffer       to specify the data object value, range [0, buffer.limit()) of the buffer will be taken as the available data to post to the server
	 * @param partial      filling true to indicates that the object is in the process of being updated, and has not yet been fully updated. Maps to HTTP header X-CDMI-Partial
	 * @param direct       fill true if want to skip the built-in cache layer
	 * @return HTTP status responded from the server
	 */
	int nonCdmi_UpdateDataObject(const std::string& uri, std::string& location, const std::string& contentType, long startOffset, long objectSize, const char buffer[], uint32 len, bool partial, bool direct);

	/**
	 * Delete a Data Object using CDMI Content Type
	 * @param jsonResult   the output text formatted JSON value returned from the server
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @return HTTP status responded from the server
	 */
	int cdmi_DeleteDataObject(std::string& jsonResult, const std::string& uri);
	/**
	 * Delete a Data Object using a Non-CDMI Content Type
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @return HTTP status responded from the server
	 */
	int nonCdmi_DeleteDataObject(const std::string& uri);
	/**
	 * Create a Container Object using CDMI Content Type
	 * @param jsonResult   the output text formatted JSON value returned from the server
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @param jsonMetadata to specify the initial metadata, formatted in JSON text, Metadata for the container
	 * @param jsonExports  to specify the structure, formatted in JSON text, for each protocol enabled for this container object
	 * @param domainURI    to specify the URI of the owning domain
	 * @param deserialize  to specify the URI of a serialized CDMI container object that shall be deserialized to create the new container object
	 * @param copy         to specify the URI of a CDMI container object or queue that shall be copied into the new container object
	 * @param move         to specify the URI of an existing local or remote CDMI container object (source URI) that shall be relocated to the URI specified in the PUT
	 * @param reference    to specify the URI of a CDMI container object that shall be redirected to by a reference.
	 * @param deserializeValue A container object serialized as specified in Clause 15 and encoded using base 64 encoding rules described in RFC 4648.
	 * @return HTTP status responded from the server
	 */
	int cdmi_CreateContainer(std::string& jsonResult, const std::string& uri, const std::string& jsonMetadata,
		const std::string& jsonExports, const std::string& domainURI, const std::string& deserialize, 
		const std::string& copy, const std::string& move, const std::string& reference, 
		const std::string& deserializeValue);
	/**
	 * Create a Container Object using a Non-CDMI Content Type
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @return HTTP status responded from the server
	 */
	///@param[in] uri  the container name appends to the rootUri, will be appended with a '/' if not available
	int nonCdmi_CreateContainer(const std::string& uri);
	/**
	 * Read a Container Object using CDMI Content Type
	 * @param jsonResult  the output text formatted JSON value returned from the server
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @return HTTP status responded from the server
	 */
	int cdmi_ReadContainer(std::string& jsonResult, const std::string& uri);
	/**
	 * Update a Container Object using CDMI Content Type
	 * @param jsonResult   the output text formatted JSON value returned from the server
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @param location     the output location URI that indicates the reference redirecting to if the object is a reference.
	 * @param jsonMetadata to specify the metadata, formatted in JSON text,metadata for the container
	 * @param jsonExports  to specify the structure, formatted in JSON text, for each protocol enabled for this container object
	 * @param domainURI    to specify the URI of the owning domain
	 * @param deserialize  to specify the URI of a serialized CDMI container object that shall be deserialized to create the new container object
	 * @param copy         to specify the URI of a CDMI container object or queue that shall be copied into the new container object
	 * @param snapshot     to specify the name of the snapshot to be taken
	 * @param deserializeValue A container object serialized as specified in Clause 15 and encoded using base 64 encoding rules described in RFC 4648.
	 * @return HTTP status responded from the server
	 */
	int cdmi_UpdateContainer(std::string& jsonResult, const std::string& uri, std::string& location,
			const std::string& jsonMetadata, const std::string& jsonExports,
		const std::string& domainURI, const std::string& deserialize, const std::string& copy,
		const std::string& snapshot, const std::string& deserializeValue);
	 /**
	 * Delete a Container Object using CDMI Content Type
	 * @param jsonResult   the output text formatted JSON value returned from the server
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @return HTTP status responded from the server
	 */
	 int cdmi_DeleteContainer(std::string& jsonResult, const std::string& uri);
	 /**
	 * Delete a Container Object using a Non-CDMI Content Type
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @return HTTP status responded from the server
	 */
	int nonCdmi_DeleteContainer(const std::string&  uri);
	/**
	 * Read the Aqua domain extension parameters
	 * @param jsonResult   the output text formatted JSON value returned from the server
	 * @param domainURI    to specify the URI of the domain to query for
	 * @return HTTP status responded from the server
	 */
	int cdmi_ReadAquaDomain(std::string& jsonResult, const std::string& domainURI);
	/**
	 * Flush the cached content of a data object
	 * @param uri          to specify the relative uri appends to the rootUri and homeContainer that will be operated, the URI on the output going HTTP request would be formatted as <u>&lt;rootURI&gt;&lt;uri&gt;</u>
	 * @return HTTP status responded from the server
	 */
	int flushDataObject(const std::string& uri );

/*
	virtual void setTimeout(uint connectTimeout, uint timeout);

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
		CDMI_ACE_READ_ALL             = 0x00000009,
		CDMI_ACE_RW_ALL               = 0x000601DF | CDMI_ACE_DELETE,

	} ACE_Const;
*/
};

}} // namespace

#endif // #ifndef __CDMIFUSE_AQUA_CLIENT_H__
