#ifndef __ZQTianShan_DEFINITION_H__
#define __ZQTianShan_DEFINITION_H__
//#include <Ice/Ice.h>
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


// Rtsp standard status line
#define	ResponseOK						"RTSP/1.0 200 OK"
#define ResponseNotFound				"RTSP/1.0 404 Not Found"
#define ResponseMethodNotAllowed		"RTSP/1.0 405 Method Not Allowed"
#define ResponseNotAcceptable		    "RTSP/1.0 406 Not Acceptable"
#define ResponseParameterNotUnderstood	"RTSP/1.0 451 Parameter Not Understood"
#define ResponseConferenceNotFound		"RTSP/1.0 452 Conference Not Found"
#define ResponseNotEnoughBandwidth		"RTSP/1.0 453 Not Enough Bandwidth"
#define	ResponseSessionNotFound			"RTSP/1.0 454 Session Not Found"
#define ResponseMethodNotValidInThisState		"RTSP/1.0 455 Method Not Valid in This State"
#define ResponseInvalidRange			"RTSP/1.0 457 Invalid Range"
#define	ResponseUnsupportedTransport	"RTSP/1.0 461 Unsupported Transport"                
#define ResponseBadRequest				"RTSP/1.0 400 Bad request"
#define ResponseUnauthorized			"RTSP/1.0 401 Unauthorized"
#define ResponseNotImplement			"RTSP/1.0 501 Not Implemented"

#define	ResponseInternalError			"RTSP/1.0 500 Internal Server Error"
#define ResponseUnexpectClientError		"RTSP/1.0 610 Unexpect Client Error" // 不是所期望的请求，如请求没有包含所需要的数据。
#define ResponseUnexpectServerError		"RTSP/1.0 620 Unexpect Server Error" // 服务器端没有预料到的异常

#define	NGOD_HEADER_SEQ						"CSeq"
#define NGOD_HEADER_TRANSPORT				"Transport"
#define NGOD_HEADER_ONDEMANDSESSIONID		"OnDemandSessionId"
#define NGOD_HEADER_SESSION					"Session"
#define NGOD_HEADER_EMBEDENCRYPT            "EmbeddedEncryptor"
#define NGOD_HEADER_ENCRYPTIONTYPE          "EncryptionType"
#define NGOD_HEADER_ENCRYPTCONTROL          "EncryptControl"
#define NGOD_HEADER_CASID                   "CAS_ID"
#define NGOD_HEADER_CONTENTTYPE				"Content-type"
#define NGOD_HEADER_CONTENTLENGTH			"Content-length"
#define NGOD_HEADER_SESSIONGROUP			"SessionGroup"
#define NGOD_HEADER_INBANDMARKER			"InBandMarker"
#define NGOD_HEADER_VOLUME					"Volume"
#define NGOD_HEADER_STREAMCONTROLPROTO		"StreamControlProto"
#define NGOD_HEADER_POLICY					"Policy"
#define NGOD_HEADER_REQUIRE					"Require"
#define NGOD_HEADER_REASON					"Reason"
#define NGOD_HEADER_SERVER					"Server"
#define NGOD_HEADER_SC_NOTICE				"SeaChange-Notice"
#define	NGOD_HEADER_MTHDCODE				"Method-Code"
#define	NGOD_HEADER_ACCEPTRANGE				"Accept-Ranges"
#define	NGOD_HEADER_RANGE					"Range"
#define	NGOD_HEADER_SCALE					"Scale"
#define	NGOD_HEADER_SC_SERVERDATA			"SeaChange-Server-Data"
#define	NGOD_HEADER_SC_TRANSPORT			"SeaChange-Transport"
#define NGOD_HEADER_USERAGENT				"User-Agent"
#define	NGOD_HEADER_TS_SERVERDAT			"TianShan-Server-Data"
#define	NGOD_HEADER_TS_TRANSPORT			"TianShan-Transport"
#define NGOD_HEADER_NOTICE					"Notice"
#define NGOD_HEADER_PUBLIC					"Public"
#define NGOD_HEADER_DATE					"Date"
#define NGOD_HEADER_USERAGENT				"User-Agent"
#define NGOD_HEADER_XREASON					"x-reason"
#define NGOD_HEADER_VOLUME					"Volume"
#define NGOD_START_POINT					"start-point"

#define NGOD_HEADER_CLIENTSESSID			"ClientSessionId"

#define METHOD_SETUP         "SETUP"
#define METHOD_TEARDOWN      "TEARDOWN"
#define METHOD_SETPARAMETER  "SETPARAMETER"
#define METHOD_GETPARAMETER  "GETPARAMETER"
#define METHOD_RESPONSE      "RESPONSE"


#define ERMI_HEADER_REQUIRE					"Require"
#define ERMI_HEADER_CLABREASON				"clab-Reason"
#define ERMI_HEADER_SESSION					"Session"
#define ERMI_HEADER_CLABCLIENTSESSIONID		"clab-clientSessionId"
#define ERMI_HEADER_CONTENTTYPE				"Content-Type"
#define ERMI_HEADER_CLABNOTICE				"clab-Notice"
#define ERMI_HEADER_TRANSPORT				"Transport"
#define ERMI_HEADER_SEQ					    "CSeq"
#define ERMI_HEADER_MTHDCODE				"Method-Code"
#define ERMI_HEADER_SESSIONGROUP			"SessionGroup"
#define ERMI_HEADER_REQUIRE_VAL				"com.cablelabs.ermi"

#define R6_HEADER_SEQ						"CSeq"
#define R6_HEADER_PROVISIONPORT				"ProvisionPort"
#define R6_HEADER_SESSION					"Session"
#define R6_HEADER_STARTCHECKING				"StartChecking"
#define R6_HEADER_REQUIRE					"Require"
#define R6_HEADER_TRANSPORT					"Transport"
#define R6_HEADER_REPTRAFFICMISMATCH		"ReportTrafficMismatch"
#define R6_HEADER_INBANDMARKER				"InbandMarker"
#define R6_HEADER_JITTERBUFFER				"JitterBuffer"
#define R6_HEADER_ONDEMANDSESSIONID			"OnDemandSessionId"
#define R6_HEADER_REQUIRE_VAL				"com.comcast.ngod.r6"
#define R6_HEADER_PROVPORT_VAL				"1"
#define R6_HEADER_CONTENTTYPE				"Content-Type"

#define R6_HEADER_STOPCHECK					"1"
#define R6_HEADER_STOPCHECKVAL				"1"

#define R6_HEADER_REASON					"Reason"
#define R6_RESPONSE_200						"200 \"User stop\""

extern  uint8 modulationStr2Int(::std::string &modulation);
extern  std::string modulationInt2Str(uint8 &modulation);
#endif //__ZQTianShan_DEFINITION_H__
