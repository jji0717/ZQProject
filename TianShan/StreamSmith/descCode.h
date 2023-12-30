#ifndef _DESCRIPTION_CODE_H__
#define	_DESCRIPTION_CODE_H__

// RTSP response PreHeader
#define	RESPONSE_CONTINUE				"RTSP/1.0 100 Continue"
#define	RESPONSE_OK						"RTSP/1.0 200 OK"
#define	RESPONSE_BAD_REQUEST			"RTSP/1.0 400 Bad request"
#define	RESPONSE_UNAUTHORIZED			"RTSP/1.0 401 Unauthorized"
#define RESPONSE_CHANNEL_NOTFOUND		"RTSP/1.0 404 Channel Not Found"
#define RESPONSE_Object_NOTFOUND		"RTSP/1.0 404 Object Not Found"
#define REPSONSE_REQUEST_NOT_ACCEPTABLE "RTSP/1.0 406 Not Acceptable"
#define	RESPONSE_REQUEST_TIMEOUT		"RTSP/1.0 408 Request Time-out"
#define RESPONSE_BAD_PARAMETER			"RTSP/1.0 451 Bad Parameter"
#define	RESPONSE_NOT_ENOUGH_BW			"RTSP/1.0 453 Not Enough Bandwidth"
#define	RESPONSE_SESSION_NOTFOUND		"RTSP/1.0 454 Session Not Found"
#define RESPONSE_INVALID_RANGE			"RTSP/1.0 457 Invalid Range"
#define	RESPONSE_INTERNAL_ERROR			"RTSP/1.0 500 Internal Server Error"
#define	RESPONSE_NOT_IMPLEMENT			"RTSP/1.0 501 Not Implemented"
#define RESPONSE_INVALID_STATE			"RTSP/1.0 455 Method Not Valid In This State"
#define RESPONSE_INVALID_PARAMETER		"RTSP/1.0 451 Invalid Parameter"
#define RESPONSE_SERVICE_UNAVAILABLE	"RTSP/1.0 503 Service Unavailable"

#define RESPONSE_SSF_NORESPONSE			"RTSP/1.0 770 ServerSetupFailed No Response"
#define RESPONSE_SSF_ASSET_NOT_FOUND	"RTSP/1.0 771 ServerSetupFailed AssetNotFound"
#define RESPONSE_SSF_SOP_NOT_AVAILABLE	"RTSP/1.0 772 ServerSetupFailed SOPNotAvailable"
#define RESPONSE_SSF_UNKNOWN_SOPGROUP	"RTSP/1.0 773 ServerSetupFailed UnknownSOPGroup"
#define RESPONSE_SSF_UKNOWN_SOPNAMES	"RTSP/1.0 774 ServerSetupFailed UnknownSOPNames"
#define RESPONSE_SSF_NO_VOLUMEBANDWIDTH	"RTSP/1.0 775 ServerSetupFailed InsufficientVolumeBandwidth"
#define RESPONSE_SSF_NO_NETWORKBANDWIDTH "RTSP/1.0 776 ServerSetupFailed InsufficientNetworkBandwidth"
#define RESPONSE_SSF_INVALID_REQUEST	"RTSP/1.0 777 ServerSetupFailed InvalidRequest"

#define	RESPONSE_ANNOUCE				"ANNOUNCE * RTSP/1.0"

// Indicates type of RTSP header, SeaChange or TianShan
#define HEADER_TYPE						"Header-Type"
#define HEADER_TYPE_SEACHANGE			"SeaChange-RtspField"
#define HEADER_TYPE_TIANSHAN			"TianShan-RtspField"

// RTSP common field
#define	HEADER_SEQ						"CSeq"
#define	HEADER_SESSION					"Session"
#define	HEADER_TRANSPORT				"Transport"
#define HEADER_SERVER					"Server"
#define	HEADER_MTHDCODE					"Method-Code"
#define	HEADER_ACCEPTRANGE				"Accept-Ranges"
#define	HEADER_RANGE					"Range"
#define	HEADER_SCALE					"Scale"
#define HEADER_USERAGENT				"User-Agent"
#define HEADER_CONTENT_BODY				"Content-Body"
#define HEADER_CONTENT_TYPE				"Content-Type"
#define HEADER_CONTENT_LENGTH			"Content-Length"
#define	HEADER_REASON                   "x-reason"

// Some important value
#define	NODE_GROUP_ID_VALUE			"node-group-id"
#define	SMARTCARD_ID_VALUE			"smartcard-id"
#define	DEVICE_ID_VALUE				"device-id"
#define	HOME_ID_VALUE				"home-id"
#define	PROGRAM_NUMBER_VALUE		"program-number"
#define	FREQUENCY_VALUE				"frequency"
#define	QAM_MODE_VALUE				"qam-mode"

// SeaChange RTSP header field
#define	HEADER_SC_NOTICE				"SeaChange-Notice"
#define	HEADER_SC_SERVERDATA			"SeaChange-Server-Data"
#define	HEADER_SC_MODDATA				"SeaChange-Mod-Data"
#define	HEADER_SC_TRANSPORT				"SeaChange-Transport"

// TianShan RTSP header field
#define HEADER_TS_VERSION				"TianShan-Version"
#define HEADER_TS_SERVICEGROUP			"TianShan-ServiceGroup"
#define HEADER_TS_APPDATA				"TianShan-AppData"
#define HEADER_TS_TRANSPORT				"TianShan-Transport"
#define HEADER_TS_CLIENTTIMEOUT			"TianShan-ClientTimeout"
#define HEADER_TS_NOTICE				"TianShan-Notice"
#define HEADER_TS_NOTICEPARAM			"TianShan-NoticeParam"
#define	HEADER_TS_SERVERDATA			"TianShan-Server-Data"

#endif//_DESCRIPTION_CODE_H__