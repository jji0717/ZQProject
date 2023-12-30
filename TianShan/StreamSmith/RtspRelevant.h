#ifndef __RtspRelevant_H__
#define	__RtspRelevant_H__


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

#define ResponseServiceUnavailable       "RTSP/1.0 503 Service Unavailable" 

#define	ResponseInternalError			"RTSP/1.0 500 Internal Server Error"
#define ResponseUnexpectClientError		"RTSP/1.0 610 Unexpect Client Error" // 不是所期望的请求，如请求没有包含所需要的数据。
#define ResponseUnexpectServerError		"RTSP/1.0 620 Unexpect Server Error" // 服务器端没有预料到的异常
#define	ResponseQamNameNotFound			"RTSP/1.0 676 Qam Name Not Found"

#define	HeaderSequence				"CSeq"
#define	HeaderSession				"Session"
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
#define	HeaderRequire				"Require"
#define	HeaderReason				"Reason"
#define	HeaderOnDemandSessionId		"OnDemandSessionId"
#define	HeaderClientSessionId		"ClientSessionId"
#define HeaderADNoticeParam			"AD-NoticeParam"		//new
#define	HeaderXUserID				"x-userID"
#define	HeaderEntitlementCode		"EntitlementCode"
#define	HeaderAuthorization			"Authorization"

#define ClientRequestPrefix			"ClientRequest#"

#define ClientSessionID				"clientSessionId"
#define	NodeGroupID					"node-group-id"
#define	SmartCardID					"smartcard-id"
#define	DeviceID					"device-id"
#define MacAddress					"mac-address"
#define	HomeID						"home-id"
#define	ProgramNumber				"program-number"
#define	Frequency					"frequency"
#define	QamMode						"qam-mode"
#define BillingID					"billing-id"
#define PurchaseTime				"purchase-time"
#define	TimeRemaining				"time-remaining"
#define PurchaseID					"purchase-id"
#define PackageID					"package-id"
#define OperatingMode				"operating-mode"
#define SupercasID					"supercas-id"
#define Destination					"destination"
      
#define Source                      "source"
#define ServerPort                  "server_port"
#define ClientPort					"client_port"
#define ClientMac					"client_mac"
#define BandWidth					"bandwidth"
#define ApplicationID				"application-id"
#define ChannelID					"channelId"
#define ProviderId					"provider-id"
#define ProviderAssetId				"provider-asset-id"
#define AssetId						"asset-id"
#define OriginalUrl					"orginalUrl"
#define clientAddress				"clientAddress"

// used inside of plugin
#define HeaderNeedResponse			"UseInside#NeedResponse"
#define HeaderFormatType			"UseInside#FormatType"
#define TianShanFormat				"TianShanFormat"
#define SeaChangeFormat				"SeaChangeFormat"
#define VLCFormat					"VLC"
#define NGODFormat					"NGODFormat"
#define HeaderVLCTransport			"RAW/RAW/UDP"

#define SPEC_NGOD_SeaChange			1
#define SPEC_NGOD_TianShan			2
#define SPEC_NGOD_S1				3

#endif //__RtspRelevant_H__

