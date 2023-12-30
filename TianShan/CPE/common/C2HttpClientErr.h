#ifndef _C2Client_ERROR_CODE_HEADER_
#define _C2Client_ERROR_CODE_HEADER_

/// errorcode return by C2HttpClient

#define ERRCODE_C2HTTPClient_InvalidParameter        400
#define ERRCODE_C2HTTPClient_ConnectionFailed        503
#define ERRCODE_C2HTTPClient_BeginReceiveFailed      500
#define ERRCODE_C2HTTPClient_ContinueReceiveFailed   500
#define ERRCODE_C2HTTPClient_EndReceiveFailed        500
#define ERRCODE_C2HTTPClient_InvalidResponse         416 
#define ERRCODE_C2HTTPClient_InvalidAviRange         500 
#define ERRCODE_C2HTTPClient_CreateFileFailed        500 
#define ERRCODE_C2HTTPClient_CallBackError           500 
#define ERRCODE_C2HTTPClient_UnkownError             500 

/// errorcode return by httpCRG
/// 400, "Bad Request",
/// 500, "Internal Server Error",
/// 503, "Service Unavailable",
/// 404, "Not Found",
/// 416, "Requested Range Not Satisfiable"

#endif//_C2Client_ERROR_CODE_HEADER_
