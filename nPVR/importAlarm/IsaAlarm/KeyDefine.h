

#ifndef _INFOCOL_KEY_DEFINE_
#define _INFOCOL_KEY_DEFINE_

#define		KD_SN_GLOBAL			"Global"
#define		KD_SN_SOURCE			"Source"
#define		KD_SN_HANDLERGROUP		"HandlerGroup"
#define		KD_SN_CHANNEL			"Channel"

#define		KD_SN_SH_CHANNEL		"C"
#define		KD_SN_SH_HANDLERGROUP	"G"
#define		KD_SN_SH_RECEIVER		"R"
#define		KD_SN_SH_HANDLER		"H"

#define		KD_KN_CHANNELCOUNT		"ChannelCount"
#define		KD_KN_SOURCECOUNT		"SourceCount"
#define		KD_KN_HANDLERCOUNT		"HandlerCount"
#define		KD_KN_RECEIVERCOUNT		"ReceiverCount"

#define		KD_KN_ENABLE_ITEM		"Enable"



#define		KD_KN_HANDLERGROUPID	"HandlerGroupID"
#define		KD_KN_TYPE				"Type"
#define		KD_KN_SYNTAX			"Syntax"
#define		KD_KN_CHANNELID			"ChannelID"

#define		KD_KN_OUTPUT			"Output"
#define		KD_KN_FILENAME			"Filename"


//////////////////////////////////////////////////////////////////////////
// for JMS Sender
#define			JMS_NAMING_CONTEXT		("JmsNamingContext")
#define			JMS_SERVER_IPPORT		("JmsServerIpPort")
#define			JMS_DEST_NAME			("JmsDestinationName")
#define			JMS_CONN_FACTORY		("ConnectionFactory")
#define			JMS_KEEPALIVE_TIME		("MsgKeepAliveTime")

#define			JMS_RECONNECT_COUNT		("ReConnectCount")
#define			JMS_RECONNECT_INTERVAL	("ReConnectInterval")
#define			JMS_STORAGE_FILEPATH	("StrorageFile")
#define			JMS_FLUSH_COUNT			("FlushToFileCount")
///////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// FOR type KD_KV_RECEIVERTYPE_UNIXLOG, require
// KD_KN_FILENAME KD_KN_MAXFILESIZE KD_KN_MAXFILENUMBER
// when file size increased to MaxFileSize, then will rename current file 
// and create a new file with Filename
#define		KD_KN_MAXFILESIZE		"MaxFileSize"
#define		KD_KN_MAXFILENUMBER		"MaxFileNumber"


#define		KD_KV_SOURCETYPE_SCLOG	"SCLOG"

#define		KD_KV_RECEIVERTYPE_TEXTFILE		"TEXT"
#define		KD_KV_RECEIVERTYPE_ISAORBEVENTCHANNEL		"ISAEVENTCH"
#define		KD_KV_RECEIVERTYPE_UNIXLOG		"UNIXLOG"
#define		KD_KV_RECEIVERTYPE_JMSSENDER	"JMSSENDER"

#define		KD_KV_HANDLERTYPE_BOOSTREG		"BOOSTREG"


//////////////////////////////////////////////////////////////////////////
//
#define KD_KN_ORB_DOMAIN			"Domain"
#define KD_KN_ORB_NAMESERVICE		"NameServer"
#define KD_KN_ORB_ENDPOINT			"EndPoint"
#define KD_KN_ORB_EVENTCHANNELNAME	"EventChannelName"

// not required, not set will use default "NameService" & "NotifyEventChannelFactory"
#define KD_KN_ORB_NAMING_SERVICE			"NamingService"
#define KD_KN_ORB_NOTIFYCATION_SERVICE		"NotifyService"



// keys for isa event channel
#define KD_KN_ORBEHF_EVENTTYPE		"EventType"
#define KD_KN_ORBEHF_EVENTNAME		"EventName"
#define KD_KN_ORBEHF_TIMEOF			"HeaderTimeOf"
#define KD_KN_ORBEHF_EVENTCODE		"HeaderEventCode"
#define KD_KN_ORBEHF_SEV			"HeaderSev"
#define KD_KN_ORBEHF_COMP			"HeaderComp"
#define KD_KN_ORBEHF_ADDR			"HeaderAddr"
#define KD_KN_ORBEHF_TASK			"HeaderTask"
#define KD_KN_ORBEHF_MP				"HeaderMp"
#define KD_KN_ORBEHF_TRANS			"HeaderTrans"

//EVENT_STR_PERF_*** defined for Performance Event Elements
#define KD_KN_ORBEHF_ENDTIME		"HeaderEndTime"
#define KD_KN_ORBEHF_FOR			"HeaderFor"
#define KD_KN_ORBEHF_DUR			"HeaderDur"
#define KD_KN_ORBEHF_END			"HeaderEnd"
#define KD_KN_ORBEHF_STRT			"HeaderStrt"

#define KD_KN_ORBEBF_OP				"BodyOp"
#define KD_KN_ORBEBF_DD				"BodyDd"
#define KD_KN_ORBEBF_SDESC			"BodySDesc"
#define KD_KN_ORBEBF_DUR			"BodyDur"
#define KD_KN_ORBEBF_XR				"BodyXr"



#define		KD_KEY_MISSING_FORMAT	"Key %s of session %s missing"
#define		KD_KEY_ERROR_FORMAT		"Key %s of session %s, value [%s] error"
#define		KD_KEY_ERROR_FORMAT_INT	"Key %s of session %s, value [%d] error"


#endif