#ifndef _tianshan_ngod_rtsp_message_header_file_h__
#define _tianshan_ngod_rtsp_message_header_file_h__


#define	RESPONSE_CONTINUE					"RTSP/1.0 100 Continue"
#define	RESPONSE_OK							"RTSP/1.0 200 OK"
#define	RESPONSE_BAD_REQUEST				"RTSP/1.0 400 Bad request"
#define	RESPONSE_UNAUTHORIZED				"RTSP/1.0 401 Unauthorized"
#define RESPONSE_CHANNEL_NOTFOUND			"RTSP/1.0 404 Channel Not Found"
#define RESPONSE_Object_NOTFOUND			"RTSP/1.0 404 Object Not Found"
#define REPSONSE_REQUEST_NOT_ACCEPTABLE		"RTSP/1.0 406 Not Acceptable"
#define	RESPONSE_REQUEST_TIMEOUT			"RTSP/1.0 408 Request Time-out"
#define RESPONSE_BAD_PARAMETER				"RTSP/1.0 451 Bad Parameter"
#define	RESPONSE_NOT_ENOUGH_BW				"RTSP/1.0 453 Not Enough Bandwidth"
#define	RESPONSE_SESSION_NOTFOUND			"RTSP/1.0 454 Session Not Found"
#define RESPONSE_INVALID_RANGE				"RTSP/1.0 457 Invalid Range"
#define	RESPONSE_INTERNAL_ERROR				"RTSP/1.0 500 Internal Server Error"
#define	RESPONSE_NOT_IMPLEMENT				"RTSP/1.0 501 Not Implemented"
#define RESPONSE_INVALID_STATE				"RTSP/1.0 455 Method Not Valid In This State"
#define RESPONSE_INVALID_PARAMETER			"RTSP/1.0 451 Invalid Parameter"
#define RESPONSE_SERVICE_UNAVAILABLE		"RTSP/1.0 503 Service Unavailable"
#define RESPONSE_OPTIONS_NOT_SUPPORT		"RTSP/1.0 551 Option Not Supported"
#define RESPONSE_TRICK_RESTRICTION			"RTSP/1.0 403 Forbidden"

#define RESPONSE_SSF_NORESPONSE				"RTSP/1.0 770 ServerSetupFailed No Response"
#define RESPONSE_SSF_ASSET_NOT_FOUND		"RTSP/1.0 771 ServerSetupFailed AssetNotFound"
#define RESPONSE_SSF_SOP_NOT_AVAILABLE		"RTSP/1.0 772 ServerSetupFailed SOPNotAvailable"
#define RESPONSE_SSF_UNKNOWN_SOPGROUP		"RTSP/1.0 773 ServerSetupFailed UnknownSOPGroup"
#define RESPONSE_SSF_UKNOWN_SOPNAMES		"RTSP/1.0 774 ServerSetupFailed UnknownSOPNames"
#define RESPONSE_SSF_NO_VOLUMEBANDWIDTH		"RTSP/1.0 775 ServerSetupFailed InsufficientVolumeBandwidth"
#define RESPONSE_SSF_NO_NETWORKBANDWIDTH	"RTSP/1.0 776 ServerSetupFailed InsufficientNetworkBandwidth"
#define RESPONSE_SSF_INVALID_REQUEST		"RTSP/1.0 777 ServerSetupFailed InvalidRequest"


#define	HeaderSequence				"CSeq"
#define HeaderServer				"Server"
#define	HeaderSession				"Session"
#define HeaderOnDemandSessId		"OnDemandSessionId"
#define HeaderRequire				"Require"
#define HeaderSessionGroup			"SessionGroup"
#define HeaderVolume				"Volume"
#define HeaderStartpoint			"StartPoint"
#define	HeaderTransport				"Transport"
#define HeaderServer				"Server"
#define	HeaderMethodCode			"Method-Code"
#define	HeaderRange					"Range"
#define	HeaderScale					"Scale"
#define HeaderUserAgent				"User-Agent"
#define HeaderContentType			"Content-Type"
#define HeaderContentLength			"Content-Length"
#define	HeaderXReason				"x-reason"
#define	HeaderNotice				"Notice"
#define	HeaderPublic				"Public"
#define HeaderAccept				"Accept"
#define HeaderCacheControl			"Cache-Control"
#define	HeaderSeaChangeNotice		"SeaChange-Notice"
#define	HeaderSeaChangeServerData	"SeaChange-Server-Data"
#define	HeaderSeaChangeModData		"SeaChange-Mod-Data"
#define	HeaderSeaChangeTransport	"SeaChange-Transport"
#define HeaderSeaChangeMayNotify	"SeaChange-MayNotify"
#define HeaderSeaChangeVersion		"SeaChange-Version"
#define HeaderTianShanVersion		"TianShan-Version"
#define HeaderTianShanServiceGroup	"TianShan-ServiceGroup"
#define HeaderTianShanAppData		"TianShan-AppData"
#define HeaderTianShanTransport		"TianShan-Transport"
#define HeaderTianShanClientTimeout	"TianShan-ClientTimeout"
#define HeaderTianShanNotice		"TianShan-Notice"
#define HeaderTianShanNoticeParam	"TianShan-NoticeParam"
#define	HeaderTianShanServerData	"TianShan-Server-Data"
#define HeaderSupported             "Supported"
#define HeaderUnsupported           "Unsupported"
#define HeaderStopPoint				"StopPoint"
#define HeaderClientSessId			"ClientSessionId"
#define	HeaderFinalNPT				"FinalNPT"

#define NGOD_ANNOUNCE_ENDOFSTREAM							"2101"
#define NGOD_ANNOUNCE_ENDOFSTREAM_STRING					"End-of-Stream Reached"

#define NGOD_ANNOUNCE_TRANSITION							"2103"
#define NGOD_ANNOUNCE_TRANSITION_STRING						"Transition"

#define NGOD_ANNOUNCE_BEGINOFSTREAM							"2104"
#define NGOD_ANNOUNCE_BEGINOFSTREAM_STRING					"Start-of-Stream Reached"

#define NGOD_ANNOUNCE_PAUSETIMEOUT							"2105"
#define NGOD_ANNOUNCE_PAUSETIMEOUT_STRING					"Pause Timeout Reached"

#define NGOD_ANNOUNCE_TRICK_NO_CONSTRAINED				    "2201" 
#define NGOD_ANNOUNCE_TRICK_NO_CONSTRAINED_STRING			"Trick play no longer constrained"

#define NGOD_ANNOUNCE_TRICK_CONSTRAINED						"2204" 
#define NGOD_ANNOUNCE_TRICK_CONSTRAINED_STRING				"Trick play constrained"

#define NGOD_ANNOUNCE_SKIP_ITEM							    "2205" 
#define NGOD_ANNOUNCE_SKIP_ITEM_STRING					    "Skipped play list item"

#define NGOD_ANNOUNCE_CLIENTSESSIONTERMINATED				"5402"
#define NGOD_ANNOUNCE_CLIENTSESSIONTERMINATED_STRING		"Client Session Terminated"

#define NGOD_ANNOUNCE_SESSIONINPROGRESS						"5700"
#define NGOD_ANNOUNCE_SESSIONINPROGRESS_STRING				"Session In Progress"

#define NGOD_TIANSHAN_ANNOUNCE_STATE_CHANGE					"8802"
#define NGOD_TIANSHAN_ANNOUNCE_STATE_CHANGE_STRING			"State Changed"

#define NGOD_TIANSHAN_ANNOUNCE_SCALE_CHANGE					"8801"
#define NGOD_TIANSHAN_ANNOUNCE_SCALE_CHANGE_STRING			"Scale Changed"


#define NGOD_ANNOUNCE_ERROR_READING_CONTENT					"4400"
#define NGOD_ANNOUNCE_ERROR_READING_CONTENT_STRING			"Error Reading Content Data"

#define NGOD_ANNOUNCE_DOWNSTREAM_FAILURE					"5401"
#define NGOD_ANNOUNCE_DOWNSTREAM_FAILURE_STRING				"Downstream Failure"

#define NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR					"5502"
#define NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING			"Internal Server Error"

#define NGOD_ANNOUNCE_BANDWIDTH_EXCEEDED_LIMIT				"5602"
#define NGOD_ANNOUNCE_BANDWIDTH_EXCEEDED_LIMIT_STRING		"Bandwidth Exceeded Limit"

#define NGOD_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE			"5200" 
#define NGOD_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE_STRING	"Server Resources Unavailable"

#define NGOD_ANNOUNCE_STREAM_BANDWIDTH_UNAVAILABLE			"6001" 
#define NGOD_ANNOUNCE_STREAM_BANDWIDTH_UNAVAILABLE_STRING	"Stream Bandwidth Exceeds That Available"

#define NGOD_ANNOUNCE_DOWNSTREAM_UNREACHABLE				"6004"
#define NGOD_ANNOUNCE_DOWNSTREAM_UNREACHABLE_STRING			"Downstream Destination Unreachable"

#define NGOD_ANNOUNCE_UNABLE_ENCRPT							"6005" 
#define NGOD_ANNOUNCE_UNABLE_ENCRPT_STRING					"Unable to Encrypt one or more Components"



#endif//_tianshan_ngod_rtsp_message_header_file_h__
