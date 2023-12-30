#ifndef __RpcGeekCommon_H__
#define __RpcGeekCommon_H__

#include "ZQ_common_conf.h"
#include "Variant.h"
#include "Guid.h"

#ifdef RPCGEEK_EXPORTS
#define	RPCGEEK_API __declspec(dllexport)
#else
#define	RPCGEEK_API __declspec(dllimport)
#endif // TESTAPI_EXPORTS

#ifdef RPCGEEK_STATIC
#undef  RPCGEEK_API
#define RPCGEEK_API
#endif // RPCGEEK_STATIC

#ifdef _DEBUG
#define XMPRPCLIBEXT "D.lib"
#else
#define XMPRPCLIBEXT ".lib"
#endif

#define CLIENT_NAME "RpcGeek Client"
#define PROTOCOL_VERSION "1.0"

#define DEFAULT_MAX_CONN    20
#define DEFAULT_SERV_IP     "LOCALANY"
#define DEFAULT_SERV_PORT   3450
#define DEFAULT_SERV_END_POINT "http://" DEFAULT_SERV_IP ":3450/RPC2" 
#define DEFAULT_SERVOBJ_LEASETERM (10*60*1000) // 10 min for default object lease term

// reserved fields
#define ERROR_CODE        "ERROR_CODE"
#define ERROR_MSG         "ERROR_MSG"
#define LEASE_TERM		  "LEASE_TERM"
#define EXEC_RESULT		  "EXEC_RESULT"
#define OBJ_GUID          "OBJ_GUID"
#define OBJ_TIMEOUT       "OBJ_TIMEOUT"

// error codes
#define ERROR_SUCC        "ERROR_SUCC"
#define ERROR_API_FAIL    "ERROR_API_FAIL"
#define ERROR_REQUEST     "ERROR_REQUEST"
#define ERROR_RESPONSE    "ERROR_RESPONSE"
#define ERROR_UID         "WRONG_GUID"
#define ERROR_MEM         "OUT_OF_MEMORY"
#define ERROR_NOTFOUND    "NOT_FOUND"
#define ERROR_INTERNAL    "ERROR_INTERNAL"
#define ERROR_IO_EXP      "IO_EXCEPTION"
#define ERROR_IO_FAIL     "ERROR_IO_FAIL"
#define ERROR_UNKNOWN_EVENT "ERROR_UNKNOWN_EVENT"


#ifdef _DEBUG
#  define TRACE_VAR(_VAR)	{ ZQ::common::Serializer srlzr(_VAR, std::cout); srlzr.serialize(); std::cout <<std::endl;}
#else
#  define TRACE_VAR(_VAR)
#endif //_DEBUG

#endif // __RpcGeekCommon_H__