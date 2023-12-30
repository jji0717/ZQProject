
#ifndef _STREAM_SMITH_MODULE_H__
#define	_STREAM_SMITH_MODULE_H__

//#include "StreamSmith.h"
#include "Log.h"
#include "Guid.h"
#include "Variant.h"
//#include <vstrmuser.h>

//////////////////////////////////////////////////////////////////////////

class IConnection;
class IServerResponse;
class IStreamSmithSite;
class IClientSession;
class IServerRequest;


typedef uint32 CtrlNum;
#define INVALID_CTRLNUM ((CtrlNum)~1)

//define the defaltSite name
#define	STR_SITE_DEFAULT		"."

#define RESERVED_PREFIX			"#SYS."
#define RESERVED_ATTR(_NAME)	("#SYS." #_NAME)
#define RTSP_USER_SESSID		RESERVED_ATTR(RtspUserSessId)
#define KEY_MESSAGE_RECVTIME	"#Msg#Received#TimeStamp#"
#define	KEY_REQUEST_PRIORITY	"sys.ReqPriority"

/// module entry SSMH_AuthUser()
/// do user authentication

//
//virtual void OnVstrmSessDetected(const PVOD_SESSION_INFORMATION sessinfo);
//virtual void OnVstrmSessStateChanged(const PVOD_SESSION_INFORMATION sessinfo, const ULONG PreviousState);
//virtual void OnVstrmSessSpeedChanged(const PVOD_SESSION_INFORMATION sessinfo, const VVX_FILE_SPEED PreviousSpeed);
//virtual void OnVstrmSessProgress(const PVOD_SESSION_INFORMATION sessinfo, const TIME_OFFSET PreviousTimeOffset);

typedef enum 
{
	RTSP_MTHD_NULL = 0, // first several are most common ones
	RTSP_MTHD_ANNOUNCE, 
	RTSP_MTHD_DESCRIBE,
	RTSP_MTHD_PLAY,
	RTSP_MTHD_RECORD,
	RTSP_MTHD_SETUP,
	RTSP_MTHD_TEARDOWN,
	RTSP_MTHD_PAUSE,
	
	RTSP_MTHD_GET_PARAMETER,
	RTSP_MTHD_OPTIONS,
	RTSP_MTHD_REDIRECT,
	RTSP_MTHD_SET_PARAMETER,
	RTSP_MTHD_PING, 
	RTSP_MTHD_RESPONSE,

	//some verb is used in LSC protocol
	RTSP_MTHD_STATUS,		//used in LSCP to get current streaming status

	RTSP_MTHD_UNKNOWN        // 13
} REQUEST_VerbCode;

typedef		REQUEST_VerbCode	RTSP_VerbCode;

// RTSP transport subkeys
#define KEY_TRANS_DEST			"destination"
#define KEY_TRANS_CPORTA		"client_porta"
#define KEY_TRANS_CPORTB		"client_portb"

// -----------------------------
// Interface IClientRequest
// -----------------------------
/// represents the available methods to a SDP based request, exported from StreamSmith server core
class IClientRequest
{
public:
    virtual ~IClientRequest() {}
	/// the phase definition
	typedef enum _ProcessPhase
	{
		/// chance to adjust the raw request and do connection authentication
		PostRequestRead		= 0, 
//		/// parse uri/protocol and do virtual site association, maybe a redirection here
//		AssociateUri		= 10, 
		/// parse header based on protocol
		// virtual site start involved in here
//		ParseHeader			= 20, 
		// application start involved in here
		/// do user authentication
		AuthUser			= 30, 
		/// fixup the request, the last chance to change the request
		FixupRequest		= 40, 
		/// handle the content
		ContentHandle	    = 50, 
		/// fixup the response before it is sent thru the connection
		FixupResponse		= 70, 
		// application and virtual site stop effort here
		/// audit the response
		PostResponse	   = 100, 
	} ProcessPhase;


	virtual void addRef( ) = 0;

	/// release this request object
	virtual void release() =0;

	/// get the process phase
	///@return the current process phase of the request
	virtual ProcessPhase getPhase() =0;

	/// get the request verb, available after phase ASSOC_URI
	///@return the verb string
	virtual RTSP_VerbCode getVerb() = 0;

	///get rtsp message start line
	///@return the address of the buf,NULL if failed
	///@param buf the buffer to hold the start line content
	///@param bufLen buffer length
	virtual const char* getStartline( char *buf , int bufLen ) = 0;

	/// get the request content uri, available after phase ASSOC_URI
	///@return the uri string
	virtual const char* getUri(char* uri, int len) = 0;

	/// get the protocol, available after phase ASSOC_URI
	///@return the protocol string
	virtual const char* getProtocol(char* protocol, int len) = 0;

	/// get a header field
	///@param key the name of the header field
	///@param value buffer to receive the value as a string
	///@param maxLen specify the max buffer size in byte that the value can receive
	///@return pointer to the value if successful
	virtual const char* getHeader(IN const char* key, OUT char* value, 
		IN OUT uint16* maxLen)=0;

	/// get protocol transport information through key
	///@param subkey is a sub key name of the transport header, acceptable: KEY_TRANS_DEST, KEY_TRANS_CPORTA, KEY_TRANS_CPORTB
	virtual bool getTransportParam(IN const char* subkey, OUT char* value, 
		IN OUT uint16* maxLen) = 0;
	
	/// get the content body of the request
	///@param buf buffer to receive the value as a string
	///@param maxLen specify the max buffer size in byte that the content buffer can receive
	///@return pointer to the content if successful
	virtual const char* getContent(unsigned char* buf, uint32* maxLen) = 0;

	///check the associated connection
	///@return true if connection is health, false for connection dropped
	virtual bool checkConnection() = 0;

	///get the associated response
	///@return pointer to the response associated to this request
	virtual IServerResponse* getResponse() = 0;

	///get the associated site
	///@return pointer to the site associated to this request, default site if no virtual site is bound
	virtual IStreamSmithSite* getSite() = 0;

	///get the associated client session record
	///@return pointer to the client session record associated to this request
	virtual const char* getClientSessionId() =0;
	
	typedef struct SockInfo
	{
		uint16 size;
		uint16 addrlen;
		char* ipaddr;
		uint16 port;		
	} RemoteInfo,LocalInfo;

	virtual bool getRemoteInfo(RemoteInfo& info) = 0;

	virtual bool getLocalInfo(LocalInfo& info) = 0 ;
};

// -----------------------------
// Interface IClientRequestWriter
// -----------------------------
/// represents a writeable client request inteface exported from StreamSmith server core
class IClientRequestWriter : public IClientRequest // a writeable client request
{
public:
	/// set the parsed command line fields of a RTSP request
	///@param[in] verb verb code of this request
	///@param[in] uri the required uri string
	///@param[in] protocol the protocol string
	///@return true if successfully
	virtual bool  setArgument(RTSP_VerbCode verb, const char* uri, 
		const char* protocol) = 0;

	/// set a header field value, this operation will overwrite the value if already exists
	///@param[in] key the name of the header field
	///@param[in] value a NULL-term string value for the given header field
	///@return the value successfully, NULL if the operation is failed
	virtual void setHeader(const char* key, char* value) = 0;

	/// set the content body of the request
	///@param[in] content pointer to the start position of the the content
	///@param[in] len number of bytes in this content
	///@return the beginning position of the content body
	virtual void setContent(const unsigned char* content, 
		const uint32 len) = 0;

	// read a data directly from the received request byte stream
	virtual const char* read(const uint32 offset, const unsigned char* content, 
		const uint32* plen) = 0;

	// overwrite the entire raw request, available only at phase PostRequestRead
	virtual const char* write(const unsigned char* reqcontent, 
		const uint32* plen) = 0;
	
	///set the associated site, only available at phase
	///@param[in] pSite pointer to the site associated to this request
	virtual bool setSite(IStreamSmithSite* pSite) = 0;

	/// set associated client context index for future use
	///@param[in] userCtxId a string as the index to a user context data structure defined in module space
	///@remark    userCtxId can be used as a key or an address to the user context. for example, the session id to a IClientSession
	virtual void setUserCtxIdx(const char* userCtxIdx)=0;
	
	///only access from the internal server objects
	virtual bool setPhase(ProcessPhase phase) =0;

	virtual void setContext(const char* key  , const char* value) =0;
	
	virtual const char* getContext( const char* key ) =0;
};

#define CLIENT_REQUEST_DISABLE_AUTO_DELETE "DisableAutoDelete"

// -----------------------------
// Interface IPlaylist
// -----------------------------
/// represents the playlist-oriented operations, exported from StreamSmith server core
class IPlaylist
{
public:
	// delarations
    virtual ~IPlaylist() {}

	/// the state of a playlist
	typedef enum _State
	{
		PLAYLIST_SETUP=0,
		PLAYLIST_PLAY=100,
		PLAYLIST_PAUSE=200,
		PLAYLIST_STOP=300,
		PLAYLIST_ERROR=400
	} State;
	enum SeekStartPos
	{
		SEEK_POS_CUR,
		SEEK_POS_BEGIN,
		SEEK_POS_END
	};
	enum
	{
		infoDVBCRESOURCE,
		infoPLAYPOSITION,
		infoPLAYNPTPOS,
		infoSTREAMSOURCE
	};
	enum	PlaylistErrCode
	{
		ERR_PLAYLIST_SUCCESS,					//Operation successful
			ERR_PLAYLIST_INVALID_PARA,				//Invalid input parameter
			ERR_PLAYLIST_SERVER_ERROR,				//server encounter a uncover problem
			ERR_PLAYLIST_INVALIDSTATE,				//server is in invalid state
	};
	typedef enum _InfoMask
	{
		INFOMASK_ITEMLENGTH=0,		
		INFOMASK_PLAYLISTLENGTH=1
	}InfoMask;

	/// define the setup information of an item in the playlist
	typedef struct _Item  
	{
		CtrlNum					_whereInsert;	//where the item insert before
		char				_fileName[256];		//the specify filename path
		CtrlNum				_currentUserCtrlNum;//current item user control number
		uint32				_inTimeOffset;
		uint32				_outTimeOffset;
		time_t				_criticalStart;
		bool				_spliceIn;
		bool				_spliceOut;
		bool				_forceNormal;
		uint32				_flags;		
		ZQ::common::Variant		_var;			//variant for passing new data into playlist
		public:
			_Item()
		{
			_whereInsert = INVALID_CTRLNUM;
            memset(_fileName, '\0', sizeof(_fileName));
			_currentUserCtrlNum = INVALID_CTRLNUM;
			_inTimeOffset = 0;
			_outTimeOffset = 0;
			_criticalStart = 0;
			_spliceIn = false;
			_spliceOut = false;
			_forceNormal =  false;
			_flags = 0;
				_var.clear();
			}
	} Item;


public:

	/// destroy the managed playlist object from the StreamSmith server core
	virtual void destroy() =0;

	/// get the last error message recorded
	virtual const char* lastError() =0;
	
	///get the last error code
	virtual  const unsigned int		lastErrCode()=0;
	
	///@retrun the GUID of the playlist
	virtual ZQ::common::Guid& getId() =0;
	
	// playlist attributes
	// --------------------
	/// set the program number about to stream off, it could not be set after the stream is started
	/// the entire playlist use the same program number 
	///@param programNumber specify the MPEG program number to stream
	///@return true if the set go thru succesfully
	virtual bool setProgramNumber(IN unsigned short int programNumber)=0;

	/// Set stream's current bit rate and maximum bitrate.
	virtual bool setMuxRate(IN unsigned long NowRate, IN unsigned long MaxRate, IN unsigned long MinRate)=0;

	/// Set stream's destination IP and udpport
	///@param ipAddr ip addresss in the format of "nnn.nnn.nnn.nnn"
	///@param udpport the destination udp port	
	virtual bool setDestination(IN char* ipAddr, IN int udpPort)=0;

	/// Set stream's destination IP and udpport
	///@param macAddr mac addresss in the format of "a:a:a:a:a:a"	
	virtual bool setDestMac(IN char* macAddr)=0;

	/// get the information about the playlist
	virtual bool getInfo(IN unsigned long mask,ZQ::common::Variant& var)=0; //TODO: define a good output
	
	// list operations
	// --------------------
	/// insert an media item into the list
	///@param where the user control number to specify where the new item is being inserted before
	///@param filename specify the filename exist on the file system to be streamed off
	///@param userCtrlNum the user control number assigned for the this item
	///@param inTimeOffset the cue in offset in ms of the given media file
	///@param outTimeOffset the cue out offset in ms of the given media file
	///@param criticalStart specify a time point that this item must be start at, if all previous items has been played before this time, it will be played before criticalStart
	///@param spliceIn true if splice in is enabled
	///@param spliceOut true if splice out is enabled
	///@param forceNormal true if want to reset to normal speed when this item is reached, otherwise keep the old speed
	///@param flags reserved
	///@return confirm with the user control number if successful
	virtual const CtrlNum insert(IN const CtrlNum where, IN const char* fileName, IN const CtrlNum userCtrlNum,
						 IN const uint32 inTimeOffset, IN const uint32 outTimeOffset,
						 IN const time_t criticalStart=0, const bool spliceIn=false, IN const bool spliceOut=false,
						 IN const bool forceNormal=true, IN const uint32 flags=0)=0;

	/// insert an item with a setup info structure
	virtual const CtrlNum insert(Item& newitem)=0;

	/// get the full size of the list
	virtual const int size() = 0;

	/// get the un-played item count
	///@return the item left from current playing to the end()
	virtual const int left() = 0;

	/// test if the list is empty
	virtual const bool empty() const = 0;

	///@return the user control number of the current playing item
	virtual const CtrlNum current() const = 0;

	///delete a specific item from the list
	///@param where user control number to identify which item to be deleted
	///@return the user control number of the next item where it is deleted
	virtual const CtrlNum erase(const CtrlNum where) = 0;

	///append the playlist with a new item at the end
	///@param filename specify the filename exist on the filesystem to be streamed off
	///@param userCtrlNum the user control number assigned for the this item
	///@param inTimeOffset the cue in offset in ms of the given media file
	///@param outTimeOffset the cue out offset in ms of the given media file
	///@param criticalStart specify a time point that this item must be start at, if all previous items has been played before this time, it will be played before criticalStart
	///@param spliceIn true if splice in is enabled
	///@param spliceOut true if splice out is enabled
	///@param forceNormal true if want to reset to normal speed when this item is reached, otherwise keep the old speed
	///@param flags reserved
	///@return confirm with the user control number if successful
	virtual const CtrlNum push_back(const char* fileName, 
									const uint32 userCtrlNum, const uint32 inTimeOffset, 
									const uint32 outTimeOffset, const time_t criticalStart=0, 
									const bool spliceIn=false, const bool spliceOut=false, 
									const bool forceNormal=true, const uint32 flags=0) = 0;

	/// pushback at the end of the playlist with a setup info structure
	virtual const CtrlNum push_back(Item& newItem)=0;

	///flush those item before the current playing item
	///@return the user control number of the first item in the list remained
	virtual const CtrlNum flush_expired() = 0;

	///clear the un-played items
	///@param includeInitedNext teardown the next item even if it has already been loaded
	///@return true if successful
	virtual const bool clear_pending(const bool includeInitedNext=false) = 0;

	// play list state checking
	// ----------------------------------

	/// test if the playlist has already reached its end
	virtual const bool isCompleted() = 0;

	/// test if the playlist is running
	virtual const bool isRunning() = 0;

	/// search by userCtrlNum, default from the begin of the list
	///@param userCtrlNum the user control number to look for
	///@param from the user control number where to start finding, INVALID_CTRLNUM means from the begin
	///@return confirm the given user control number if successful
	virtual const CtrlNum findItem(const CtrlNum userCtrlNum, 
									const CtrlNum from = INVALID_CTRLNUM) = 0;
	
	/// calculate the distance between from and to,return false if error
	/// @param OUT dist the distance of from and to 
	/// @param IN to the control number of destination
	/// @param IN from the control number of start,INVALID_CTRLNUM means from current
	virtual const bool distance(OUT int* dist, IN CtrlNum to, IN CtrlNum from = INVALID_CTRLNUM)=0;

	// stream control
	// --------------------
	/// play a list
	///@return false if this operation failed
	virtual bool play() = 0;

	/// change the play speed
	///@param[in] newSpeed the new speed to apply on the playlist, the actual play speed will be rounded
	///@return false if this operation failed
	virtual bool setSpeed(const float newSpeed) = 0;

	/// pause a playing list
	///@return false if this operation failed
	virtual bool pause() = 0;

	/// resume a paused playlist
	///@return false if this operation failed
	virtual bool resume() = 0;

	/// skip to a specific item in the list
	///@param[in] where the user control number to identify the item in the list
	///@param[in] bPlay play immediately even if the playlist's state is not PLAYLIST_PLAY
	///@return false if this operation failed
	virtual bool skipToItem(const CtrlNum where, bool bPlay =true) = 0;

	///Get current play list status
	virtual	State getCurrentState()=0;
	
	/// set associated client context index for future use
	///@param[in] userCtxId a string as the index to a user context data structure defined in module space
	///@remard    userCtxId can be used as a key or an address to the user context. for example, the session id to a IClientSession
	virtual void setUserCtxIdx(const char* userCtxIdx)=0;
	
	/// Get associate client session ID
	virtual const char* getUserCtxIdx()=0;

	enum ResourceAllocation
	{
		RES_GUID				=0,//type=>>tstring
		RES_PROGRAMNUMBER		=1,//type=>>int
		RES_FRENQENCY			=2,//type=>>int
		RES_CHANNEL				=3,//type=>>int
		RES_DESTIP				=4,//type=>>tstring
		RES_DESTPORT			=5,//type=>>int
		RES_DESTMAC				=6,//type=>string
		RES_QAMMODE				=7 //type=>>int
	};
	/// allocate get the relative resource with a give service group id
	///@return true if get resource successfully false if fail
	///@param[in] serviceGroupID the service group ID
	///@param[out] allocated resource
	virtual	bool allocateResource(int serviceGroupID, ZQ::common::Variant& varOut,int bandwidth=-1/*kbps*/)=0;

	///release resource
	///@return true if success false if fail
	///@param[IN] uid resource's guid
	virtual	bool releaseResource(ZQ::common::Guid& uid)=0;

	/// turn on or off the EdgeOfTail (EoT). EdgeOfTail was designed for real-time ingestion, it will protect the streaming
	/// not to reach the real-ingestion position.
	///@param[in] enable true if to turn on the EoT protection, false to disable the protection
	///@note By default, the playlist will always have EoT turned on if no enableEoT() has been invoked
	virtual void enableEoT(bool enable) =0;
};	


// -----------------------------
// Interface IServerResponse
// -----------------------------
/// represents the accesses of a server response exported from StreamSmith server core
class IServerResponse
{
public:
    virtual ~IServerResponse() {}
	/// print some content before the SDP response header fields
	///@param content the string to print before the header fields, for example: "200 OK\r\n" for http
	///@return the start offset of the printed string
	virtual void printf_preheader(const char* content) = 0;

	/// print some content after the SDP response header fields
	///@param content the string to print after the header fields, for example: "\r\n<html></html>" for http
	///@return the start offset of the printed string
	virtual void printf_postheader(const char* content) = 0;

	/// set a header field
	///@param key the field name
	///@param value the field value
	///@return pointer to the key if succes
	virtual void setHeader(const char* key, const char* value) = 0;


	///get the start line
	///@return the start line ,NULL if no start line
	///@param value the buffer to hold the start line content
	///@param valueLen the buffer size in byte
	virtual const char* get_preheader(IN OUT char* value , IN OUT uint16* valueLen ) =0;

	///get the content body
	///@return the content body,NULL if there is no content body
	///@param value the buffer to hold the body content
	///@param valueLen the buffer size in byte
	virtual const char* get_postheader(IN OUT char* value , IN OUT uint16* valueLen )=0;

	///get header field value through its key
	///@return the value ,NUll if no value is associated with the key
	///@param key the field key
	///@param value the buffer to hold the field value
	///@param valueLen the buffer size in byte
	virtual const char* getHeader(IN const char* key ,IN OUT char *value,IN OUT uint16* valueLen) =0;	


	/// post a server response on the given connection
	///@param pConn the connection that the response want to be sent via
	///@return count of the byte sent
	virtual uint32 post() =0;
};

// -----------------------------
// Interface IClientSession
// -----------------------------
/// represents a client session context exported from StreamSmith server core
///@note the data access implementation in the server core space will be based on ZQ::common::Variant
class IClientSession
{
public:

    virtual ~IClientSession() {}
	/// type of the client session
	typedef enum _SessType
	{
		///local handled session
		LocalSession =0,
			///remote handled session, this server worked as a proxy
		ProxySession,
	} SessType;
	
	///query the type of the client session
	///@return type of the session
	virtual SessType getType() =0;
	
	///retrieve the Session ID
	///@return id of the session
	virtual const char* getSessionID() = 0;
	
	// virtual void release() =0;
	
	//user context methods
	///get the access to the pre-set user's per-session context
	///@return pointer to the user context
	virtual void*    getUserContext() =0;
	
	///set the user's per-session context
	///@param pContext the pointer to user's per-session context in plugin space,\n
	///                NULL is used to reset the association in session in the server core space
	///@return pointer to the user context
	virtual void*    setUserContext(void* pContext=NULL) = 0;
	
	///get a session attribute
	///@param attributeName the attribute name about to query, ASCII NULL-terminate string allowed
	///@return the value of the attribute, Variant::Nil if the attribute doesn't exist
	virtual const ZQ::common::Variant get(const char* attributeName) = 0;
	
	//set a session attribute
	///@param attributeName the attribute name about to set, ASCII NULL-terminate string allowed
	///@param value the new value of the attribute about to set, Variant::Nil and overwrite=ture will remove the attribute
	///@param overwrite true if need to overwrite the exist one, otherwise will return false if the same attribute has existed
	///@return true if the attribute has been successfully updated
	virtual bool set(const char* attributeName, 
		const ZQ::common::Variant& value, bool overwrite=true) = 0;
	
	virtual std::string getProp( const std::string& key ) const = 0;

	virtual  bool		setProp( const std::string& key, const std::string& value ) = 0;

	/// release this ClientSession object
	///@remark if user finishes accessing this ClientSession instance, release() must be called
	virtual long release() = 0;
	
};
// -----------------------------
// Interface IServerRequest
// -----------------------------
/// represents a server-to-client request, exported from StreamSmith server core
///@note A IServerRequest will always be bound with a IClientSession so that it can assocate the connection thru which
/// the message can be passed to the client
class IServerRequest
{
public:
	
    virtual ~IServerRequest() {}
	/// output a commnad line
	///@param[in] cmdline the NULL-term command line, the 1st line, in the request
	///@return number of charaters has be outputed
	virtual int printCmdLine(const char* cmdline) = 0;
	
	/// output a header field
	///@param[in] header the header field name
	///@param[in] value the header field value
	///@return number of characters has be outputed
	virtual int printHeader(char* header, char* value) = 0;
	
	/// output content body of request
	///@param[in] msg content body of request
	///@return number of characters has be outputed
	virtual int printMsgBody(char* msg) = 0;
	
	/// post the message to the client
	///@return number of characters has be sent
	virtual int post() = 0;
	
	/// release itself, must be called if the accessing to this object is finished
	virtual void release() = 0;
	
	/// close the relative connection of server request object
	virtual int closeConnection() = 0;

	///retrive the client session id of current serverRequest
	virtual const char*	getClientSessionID() = 0;
};


typedef enum 
{
	E_NULL					=0,
	//edge events
	E_EDGE_EVENT_LOST		=1<<0,
	E_EDGE_REMOTE_SESSION_AVAILABLE=1<<1,
	E_EDGE_NO_LISTENER_FOUND=1<<2,
	
	//vod events
	E_VOD_EVENT_LOST		=1<<3,
	E_VOD_FORWARD_SPEED_CHANGES_DISABLED=1<<4,
	E_VOD_FORWARD_SPEED_CHANGES_ENABLED=1<<5,
	E_VOD_SPEED_CHANGE		=1<<6,	
	
	//events about playlist
	E_PLAYLIST_STARTED				= 1<<8,
	E_PLAYLIST_STATECHANGED			= 1<<9,			//vstrm session state changed
	E_PLAYLIST_SPEEDCHANGED			= 1<<10,			//vstrm port speed changed
	E_PLAYLIST_INPROGRESS			= 1<<11,		//item progress
	E_PLAYLIST_ITEMLOADED			= 1<<12,		
	E_PLAYLIST_ITEMDONE				= 1<<13,		//reach end of item	
	E_PLAYLIST_END					= 1<<14,		//reach playlist end
	E_PLAYLIST_BEGIN				= 1<<15,		//reach playlist begin
	E_PLAYLIST_SESSEXPIRED			= 1<<16,		//vstrm session expire abnormally
	E_PLAYLIST_DESTROYED			= 1<<17,		//playlist destroyed
	E_PLAYLIST_PAUSETIMEOUT			= 1<<18,		//session pause time out event
	E_PLAYLIST_REPOSITION			= 1<<19			// in item reposition

} EventType;


typedef enum _RequestProcessResult
{
	RequestError         = -100, /// error occured during processing the request
	RequestUnrecognized  =-1,
	RequestProcessed	 =0,   /// request has been processed but need further process in the same phase
	RequestPhaseDone     =20,  /// request has been done for this phase, ready to be processed in next phase
	RequestDone          =100, /// request has been done, no further process needed
} RequestProcessResult;

/// module entry SSMH_PostRequestRead()
/// chance to adjust the raw request and do connection authentication
// typedef RequestProcessResult (*SSMH_PostRequestRead) (IN IStreamSmithSite* pDefaultSite, IN OUT IClientRequestWriter* pReq, IN const unsigned char* buf, IN OUT uint32* len);

// /// module entry SSMH_AssociateUri()
// /// parse uri/protocol and do virtual site association, maybe a redirection here
// typedef RequestProcessResult (*SSMH_AssociateUri) (IStreamSmithSite* pSite, IClientRequestWriter* pReq);

// /// module entry SSMH_ParseHeader()
// /// parse header based on protocol
// /// virtual site start involved in here
// typedef RequestProcessResult (*SSMH_ParseHeader) (IStreamSmithSite* pSite, IClientRequestWriter* pReq);
typedef RequestProcessResult (*SSMH_AuthUser) (IStreamSmithSite* pSite, IClientRequestWriter* pReq);

/// module entry SSMH_FixupRequest()
/// fixup the request, the last chance to change the request
/// application start involved in here
typedef RequestProcessResult (*SSMH_FixupRequest) (IStreamSmithSite* pSite, IClientRequestWriter* pReq);

/// module entry SSMH_ContentHandle()
/// handle the content
typedef RequestProcessResult (*SSMH_ContentHandle) (IStreamSmithSite* pSite, IClientRequestWriter* pReq);

/// module entry SSMH_FixupResponse() fixup the response before it is sent thru the connection
typedef RequestProcessResult (*SSMH_FixupResponse) (IStreamSmithSite* pSite, IClientRequest* pReq);

/// module entry SSMH_PostResponse() audit the transaction 
// application and virtual site stop effort here
typedef RequestProcessResult (*SSMH_PostResponse) (IStreamSmithSite* pSite, IClientRequest* pReq);


///module entry fixupAnnounce() audit the announce before send out to client
typedef RequestProcessResult (*SSMH_FixupServerRequest)(IStreamSmithSite* pSite , IServerRequest* pReq);


//some known event field will be passed thru SSMH_EventSink() entry
#define EventField_SourceNetId			"sourceNetId"
#define	EventField_SessionId			"sessionID"
#define	EventField_PrevState			"prevState"
#define	EventField_CurrentState			"currentState"
#define	EventField_PrevSpeed			"prevSpeed"
#define	EventField_CurrentSpeed			"currentSpeed"
#define	EventField_PrevTimeOffset		"prevTimeOffset"
#define	EventField_CurrentTimeOffset	        "currentTimeOffset"
#define EventField_TotalTimeOffset		"TotalTimeOffset"
#define EventField_ExitReason	                "ExitReason"
#define EventField_ExitCode	                "ExitCode"
#define EventField_AnnounceLastErrorCode        "AnnounceLastErrorCode"
#define EventField_AnnounceLastItemName	        "AnnounceLastItemName"
#define EventField_AnnounceLastErrDesc	        "AnnounceLastErrDescription"
#define EventField_ExtraProperties				"ExtraProperties"
#define EventField_perRequested           "perRequested"

#define	EventField_runningByteOffset	        "runningByteOffset"
#define	EventField_TotalbyteOffset		"totalByteOffset"
#define	EventField_currentStep			"playListcurrentStep"
#define	EventField_totalStep			"playlistTotalStep"

//every speed value contain a denominator and a numerator
#define	EventField_SpeedDenom			"denominator"
#define	EventField_SpeedNumer			"numerator"

#define	EventField_ClientSessId			"ClientSessionID"
#define	EventField_PlaylistGuid			"playlistGuid"
#define EventField_ItemExitReason		"ItemExitReason"
#define EventField_ItemExitCode			"ItemExitCode"
#define EventField_ItemIndex			"EventField_ItemIndex"
#define EventField_ItemOffset			"EventField_ItemInnerTimeOffset"
#define	EventField_UserCtrlNum			"userCtrlNumber"
#define EventField_NextUserCtrlNum		"NextUserCtrlNumber"
#define	EventField_ItemFileName			"ItemFileName"
#define EventField_PlayScale			"Scale"
#define EventField_ItemOtherFileName	        "ItemFileName-Other"
#define EventField_CurrentItemTimeOffset        "currentItemTimeOffset"
#define EventField_PrevItemTimeOffset	        "PrevisousItemTimeOffset"
#define EventField_StampUTC			"EventStamplUTC"
#define EventField_EventCSEQ			"EventSequence"
#define	EventField_Reason			"EventReason"

#define	EventField_prevProviderId		"previousProviderId"
#define EventField_prevProviderAssetId	        "previouseProviderAssetId"
#define	EventField_currentPoviderId		"currentProviderId"
#define EventField_currentProviderAssetId       "currentProviderAssetId"


#define	EventField_clusterId			"mediaclusterId"
#define	EventField_PrevStreamingSource	        "PrevItemStreamingSource"
#define	EventField_CurStreamingSource	        "CurItemStreamingSource"
#define EventField_playlistExitStatus	        "playlistExitStatus"


#define EventField_StreamSourceIp		"StreamingSourceIp"
#define EventField_StreamSourcePort		"StreamingSourcePort"

#define EventField_PreviousePlayPos		"StreamingPreviousPlayPosition"
#define EventField_PreviousePlayPosPrimary  "StreamPrevItemNptPrimary" 
#define EventField_PrevPlayTime         "StreamingPreviousPlayTime"
#define EventField_CurrPlayTime         "StreamingCurrentPlayTime"
#define EventField_PrevFlag             "StreamingPreviousFlag"
#define EventField_CurrFlag             "StreamingCurrentFlag"
#define EventField_TotalDuration        "StreamingTotalDuration"
#define EventField_TotalVideoDuration   "StreamingTotalVideoDuration"

#define EventField_CurrentPlayPos		"StreamingCurrentPlayPosition"

#define ENCRYPTION_ENABLE				"ecmEnable"
#define ENCRYPTION_VENDOR				"ecmVendor"
#define ENCRYPTION_ECM_PID				"ecmPid"
#define ENCRYPTION_CYCLE1				"ecmCycle1"
#define ENCRYPTION_CYCLE2				"ecmCycle2"
#define ENCRYPTION_FREQ1				"ecmFreq1"
#define ENCRYPTION_FREQ2				"ecmFreq2"
#define ENCRYPTION_DATACOUNT			"ecmDataCount"
#define ENCRYPTION_DATAPREFIX			"ecmDataPrefix"
#define ENCRYPTION_PNOFFSETPREFIX		"ecmDataPNoffsetPrefix"

#define	VSTRM_ITEM_PID					"vtrsm_item_PID"					//ushort

#define VSTRM_ITEM_PAUSELASTUTILNEXT	"vstrm_Pause_last_util_next"		//ushort


#define STORAGE_LIBRARY_URL				"StorageLibraryUrl"
#define ITEMDATA_PROVIDERID				"providerId"
#define ITEMDATA_PROVIDERASSETID		"providerAssetId"

/// module entry
/// when receive a event sink with a event type defined as EVENTSINK_TYPE,it need to decode the parameter params use the defined as SINKKEY_XXXX
///return true indicate everything is ok,return false if you need to resend it again!
typedef bool				(*SSMH_EventSink)(IStreamSmithSite* pSite, EventType Type, ZQ::common::Variant& params);

/// module entry SSHM_ModuleInit
/// module should register all its entry in this function
/// And the entry name should be	ModuleInit
typedef void				(*SSHM_ModuleInit)(IStreamSmithSite* pSite);


///module entry SSHM_ModuleInitEx
///Note:that this entry is optional
///But if you export this entry,the entry SSHM_ModuleInit will be ignored.
///So don't do anything in SSHM_ModuleInit if you export this entry
/// And the entry name should be	ModuleInitEx
typedef void				(*SSHM_ModuleInitEx)(IStreamSmithSite* pSite,const char* pConfString);

/// Module entry SSM_ModuleUninit
/// Module should clear all it's resource in this function
/// And the entry name should be	ModuleUninit
typedef void				(*SSM_ModuleUninit)(IStreamSmithSite* pSite);

typedef void				(* SSMH_SessionDrop)(const char* sessionId);

// -----------------------------
// Interface IStreamSmithSite
// -----------------------------
/// represents the accesses to an associated virtual site, exported from StreamSmith server core
class IStreamSmithSite
{
public:
	enum InformationType
	{
		INFORMATION_LSCP_PORT,
		INFORMATION_RTSP_PORT,
		INFORMATION_SNMP_PROCESS
	};

public:
    virtual ~IStreamSmithSite() {}

	///@return the name of the site
	virtual const char* getSiteName() =0;

	/// allow a request to be re-queued in the process stack
	///@param pReq the request or a sub-request to be queued
	///@param @phase specify the process stack to insert in, forward to the default site if the phase is before PARSE_HEADER
	///@return true if successful
	virtual bool postRequest(IClientRequestWriter* pReq, const IClientRequest::ProcessPhase phase) =0;

	///@return get the access to the default site
	static  IStreamSmithSite* getDefaultSite();
 
	///open an existing playlist instance or create a new playlist
	///@param playlistGuid the Guid used to identify a playlist
	///@param bCreateIfNotExist true if create a new playlist with the give guid if it does not exists
	///@return pointer to the playlist instance
	virtual IPlaylist* openPlaylist(const ZQ::common::Guid& playlistGuid, const bool bCreateIfNotExist, const char* clientSessionID =NULL) =0;


	///open an exist playlist instance or create a new playlist with resource
	///@param playlistGuid the Guid used to identify a playlist
	///@param bCreateIfNotExist true if create a new playlist with the give guid if it does not exists
	///@param serviceGroupID the target service group id
	///@param maxBitRate bandwith the playlist need	
	///@return pointer to the playlist instance
	virtual IPlaylist* openPlaylist(const ZQ::common::Guid& playlistGuid,const bool bCreateIfNotExist,
									const int serviceGroupID,const int maxBitRate,
									const char* pCLientSessionID=NULL)=0;
	
	///create an un-existing client session instance or create a new session
	///@param sessId the session id used to identify a client session	
	///@return pointer to the client session instance
	///@remark IClientSession::release() must be called if accessing to the ClientSession instance is completed
	virtual IClientSession* createClientSession(const char* sessId, const char* uri,
								const IClientSession::SessType sessType = IClientSession::LocalSession) = 0;

	/// find the ClientSession with session ID return NULL if can't find it!
	virtual IClientSession* findClientSession(const char* sessId, 
								const IClientSession::SessType sessType = IClientSession::LocalSession) = 0;


	/// force to destroy a ClientSession object in the server core.
	///@param sessId the session id used to identify a client session
	///@reture true if successful
	virtual bool		destroyClientSession(const char* sessionID) = 0;

	/// create a RTSP request for the requests from server to client
	///NOTE after you use IServerRequest,you should call release routinue to free it's resource
	///@param sessId the id to identify a client session
	///@param ConnectionID connection identity string
	///@return pointer to a ServerRequest instance, it must be free-ed by calling IServerRequest::release() after use
	///@remark the server core will attach this ServerRequest onto the connection associated with this session
	virtual IServerRequest*	newServerRequest(const char* sessionID,const std::string& ConnectionID="") = 0;

	///post announce to another phase so that the new phase can audit the announce data	
	///@param pReq the server request you want to post
	virtual void			PostServerRequest(IServerRequest* pReq)  = 0;


	///get the access URL string to the asset dictionary
	virtual const char*		getAssetDictoryUrl()=0;

	//get the application's log folder
	//and this method is only available for default site when application load plugin
	//and it's intialize routine 
	virtual const char*		getApplicationLogFolder( ) = 0;

	///get core information
	virtual ZQ::common::Variant getInfo( int32 infoType ) = 0;

public:

	/// hook a FixupRequest entry
	///@param enFixupRequest the fixup request process entry
	virtual	void	RegisterFixupRequest(const SSMH_FixupRequest& enFixupRequest)=0;

	/// hook a FixupResponse entry
	///@param enFixupResponse the fixup response process entry
	virtual void	RegisterFixupResponse(const SSMH_FixupResponse& enFixupResponse)=0;
	
	/// hook a ContentHandle entry
	///@param handlerName the exported handler name
	///@param enContentHandle the content handle process entry
	virtual	void	RegisterContentHandle(const char* handlerName, const SSMH_ContentHandle& enContentHandle)=0;	


	///hook a fixup serverRequest
	///@param enFixupServerRequest fixup Server Request routniue
	virtual void	RegisterFixeupServerRequest(const SSMH_FixupServerRequest& enFixupServerRequest) = 0;

	// added by Cary
	// registry a callback for session deleting
	virtual	void	RegisterSessionDrop(SSMH_SessionDrop sessionDropHandle) = 0;

	/// log a message in the server core's log file
	virtual void	CoreLog(ZQ::common::Log::loglevel_t level,char* fmt,...) =0;
	
	/// sink the server core's event
	///@param dwEventMask a collection of EventType as flags
	///@param EventHandler the event handle process entry
	virtual void	SinkEvent(const uint32 dwEventMask, const SSMH_EventSink& EventHandler)=0;

};


#ifdef NEED_EVENTSINK_RAWDATA
typedef struct _tagSINKPROGRESS
{
	PVOD_SESSION_INFORMATION	pSessionInfo;
	TIME_OFFSET					timeOffset;	
}SINKPROGRESS;

typedef struct _tagSINKSPEENCHANGED
{
	PVOD_SESSION_INFORMATION	pSessionInfo;
	VVX_FILE_SPEED				prevFileSpeed;
}SINKSPEEDCHANGED;

typedef struct _tagSESSDETECTED 
{
	PVOD_SESSION_INFORMATION	pSessInfo;
}SESSDETECTED;

typedef struct _tagSESSSTATECHANGED 
{
	PVOD_SESSION_INFORMATION	pSessInfo;
	ULONG						PrevState;	
}SESSSTATECHANGED;
#endif

//Method not allowed in this state
//This may be the same meaning of InvalidStateOfArt but used for vstrm API_DISABLE thing
const int32 EXT_ERRCODE_METHOD_NOT_VALID		=	-10;	// 455

const int32 EXT_ERRCODE_BANDWIDTH_EXCEEDED		=	-11;	// 453 , but this can only be used in setup stage

const int32 EXT_ERRCODE_INVALID_RANGE			=	-12;	// 457

const int32 EXT_ERRCODE_SERVICEUNAVAIL			=	-13;	// 503

#define	StrmCtrlErr_NoSession   (-100)

#define StrmCtrlErr_Connection  (-101)

#define StrmCtrlErr_AsynCall    (-102)

#define StrmCtrlErr_Timeout     (-103)

#define StrmCtrlErr_Response    (-104)



#endif
