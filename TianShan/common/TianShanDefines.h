// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: TianShanDefines.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/common/TianShanDefines.h $
// 
// 13    1/20/16 5:04p Li.huang
// 
// 12    1/11/16 5:39p Dejian.fei
// 
// 11    12/11/13 4:45p Hui.shao
// rollback
// 
// 9     2/26/13 11:11a Hui.shao
// 
// 8     2/06/13 11:28a Hui.shao
// 
// 7     12/19/12 3:56p Hui.shao
//  PRINTFLIKE
// 
// 6     9/12/12 4:57p Hui.shao
// string tokenize
// 
// 5     1/18/12 6:03p Hongquan.zhang
// 
// 4     1/17/12 11:52a Hui.shao
// 
// 3     3/08/11 2:14p Fei.huang
// migrate to linux
// 
// 2     1/11/11 12:17p Hongquan.zhang
// use ZQ::common::Semaphore in WatchDog
// 
// 92    10-08-24 16:20 Hui.shao
// added DEFAULT_BINDPORT_CDNSS_HTTP = 12000
// 
// 91    10-06-29 16:53 Li.huang
// add eventchannel listen endpoint for ContentLib  ERG and A3_Message
// plugin
// 
// 90    10-06-24 13:39 Hui.shao
// 
// 89    10-05-27 17:29 Haoyuan.lu
// 
// 88    10-05-25 16:35 Haoyuan.lu
// 
// 87    10-05-07 15:39 Hongquan.zhang
// 
// 86    10-05-06 18:57 Hui.shao
// 
// 85    10-05-06 17:46 Hui.shao
// added utilty class MemoryIndex
// 
// 84    10-03-20 14:10 Fei.huang
// 
// 83    10-03-18 11:28 Hui.shao
// 
// 82    10-02-04 17:47 Yixin.tian
// add closeBarker() function
// 
// 81    09-09-15 16:12 Li.huang
// add DEFAULT_BINDPORT_EdgeRMPHO
// 
// 80    09-09-04 16:03 Hui.shao
// moved HaoYuan's IceCurrentToStr from TianIceHelper to TianShanDefines
// 
// 79    09-08-07 12:28 Xiaoming.li
// define dumpResourceData buffer size
// 
// 78    09-08-07 11:30 Hongquan.zhang
// 
// 77    09-07-30 16:25 Junming.zheng
// maintenance NGOD accroding OSTR CR006
// 
// 71    09-06-18 16:07 Hongquan.zhang
// 
// 70    09-02-26 18:57 Hui.shao
// reformatted the endpoint definitions
// 
// 69    09-02-18 10:01 Xiaohui.chai
// change log parsing scheme
// 
// 68    08-12-11 15:15 Hongquan.zhang
// 
// 67    08-12-09 17:12 Yixin.tian
// 
// 66    08-12-09 12:40 Fei.huang
// + less algorithm for Identity sorting
// 
// 65    08-11-19 18:11 Build
// 
// 64    08-11-15 10:02 Yixin.tian
// modify macro MAPSET
// 
// 63    08-11-13 14:18 Yixin.tian
// modify to compile in Linux OS
// 
// 62    08-11-10 11:39 Hui.shao
// use the primary adapter name as the display name
//
// 61    08-10-31 16:51 Fei.huang
// + add macro ADAPTER_NAME_ContentStore
// 
// 60    08-10-28 11:32 Hui.shao
// 
// 58    08-10-16 19:44 Hui.shao
// 
// 54    08-10-15 16:55 Hui.shao
// 
// 57    08-10-15 16:49 Hui.shao
// 
// 56    08-10-08 14:17 Hui.shao
// 
// 55    08-08-19 10:54 Xiaoming.li
// 
// 54    08-08-14 15:00 Hui.shao
// merged from 1.7.10
// 
// 56    08-07-29 12:17 Hui.shao
// added macro EventFMT
// 
// 55    08-07-17 15:05 Hui.shao
// 
// 54    08-07-07 16:08 Hui.shao
// 
// 53    08-05-13 13:54 Jie.zhang
// 
// 52    08-04-23 18:13 Hui.shao
// 
// 51    08-03-25 12:18 Xiaohui.chai
// moved getServeAddress() & appendServAddrs() to public section
// 
// 50    08-03-10 17:09 Yixin.tian
// 
// 49    08-03-07 10:22 Yixin.tian
// WIN32 replaced by ZQ_OS_MSWIN
// 
// 48    08-03-05 14:35 Yixin.tian
// merge for linux
// 
// 47    08-02-20 17:10 Hui.shao
// added context pointer for the dump functions
// 
// 46    08-02-19 14:43 Hui.shao
// 
// 45    08-02-18 11:58 Hui.shao
// 
// 44    08-02-17 19:19 Hui.shao
// 
// 43    08-01-28 13:39 Hui.shao
// added ISO8601ToTime
// 
// 42    08-01-22 11:46 Hongquan.zhang
// 
// 41    08-01-22 11:41 Hongquan.zhang
// 
// 40    07-12-14 15:37 Xiaohui.chai
// 
// 39    07-11-19 12:12 Hongquan.zhang
// make _IceThrow safer by using _vsnprintf instead of vsprintf
// 
// 37    07-06-29 10:41 Hongquan.zhang
// 
// 36    07-06-15 18:01 Hongquan.zhang
// 
// 35    07-06-04 14:46 Hui.shao
// add log to Adapter
// 
// 34    07-06-01 12:23 Hui.shao
// 
// 33    07-06-01 12:07 Hui.shao
// added validate request speed utility
// 
// 32    07-05-29 18:47 Hui.shao
// rewrote the ZQAdapter
// 
// 31    07-05-23 14:50 Hui.shao
// 
// 30    07-05-23 13:28 Hui.shao
// added _IceThrow with category and errorcode
// 
// 29    07-05-22 17:30 Hui.shao
// added exporting logger information
// 
// 28    07-05-21 11:33 Hui.shao
// 
// 27    07-03-28 16:53 Hui.shao
// 
// 27    07-03-28 16:29 Hui.shao
// 
// 26    07-03-23 14:59 Hui.shao
// 
// 25    07-03-23 14:45 Hui.shao
// 
// 24    07-03-13 17:11 Hongquan.zhang
// 
// 23    06-11-21 13:37 Hongquan.zhang
// 
// 22    06-10-16 11:42 Jie.zhang
// 
// 21    06-09-28 17:07 Ken.qian
// 
// 20    9/21/06 4:34p Hui.shao
// batch checkin on 20060921
// ===========================================================================

#ifndef __ZQTianShanDefines_H__
#define __ZQTianShanDefines_H__

//#ifdef ZQCOMMON_DLL
#  include "ZQ_common_conf.h"
#  include "Exception.h"
#  include "Locks.h"
#  include "Log.h"
#  include "NativeThreadPool.h"
//#endif ZQCOMMON_DLL

#include "TianShanIce.h"
#include "TsSRM.h"

#include "TianShanUtils.h"
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

#ifdef _DEBUG
#define LIBSUFFIX "d"
#else
#define LIBSUFFIX ""
#endif // _DEBUG

#ifdef ZQ_OS_MSWIN
#ifndef _LIB
#  pragma comment(lib, "Ice" LIBSUFFIX)
#  pragma comment(lib, "IceUtil" LIBSUFFIX)
#  pragma comment(lib, "freeze" LIBSUFFIX)
#endif // _LIB
#endif

#define PD_FIELD(_CATEGORY, _FIELD)  (#_CATEGORY "." #_FIELD)

#define DumpPrefixBufSize 1024

#if ICE_INT_VERSION / 100 >= 306

	#define  ICEAbstractMutexRLock IceUtil::AbstractMutexI<IceUtil::RecMutex>
	#define  ICEAbstractMutexWLock IceUtil::AbstractMutexI<IceUtil::RecMutex>
	#define  RWRecMutex   RecMutex
	#ifndef WLock 
	//#  define WLock(_OBJ) IceUtil::Mutex::Lock wlock((_OBJ)._mutex)
	#  define IceLock IceUtil::RecMutex::Lock
	#  define WLock IceLock
	#  define RLock IceLock
	#endif // WLock
	
	#ifndef WLockT 
	//#  define WLock(_OBJ) IceUtil::Mutex::Lock wlock((_OBJ)._mutex)
	#  define IceLockT IceUtil::LockT
	#  define WLockT IceLockT
	#  define RLockT IceLockT
	#endif // WLock

#else 
	#define  ICEAbstractMutexRLock IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
	#define  ICEAbstractMutexWLock IceUtil::AbstractMutexWriteI<IceUtil::RWRecMutex>
#endif
// -----------------------------
// default ICE definitions
// -----------------------------
// the default bind port of each known TianShan component
#define DEFAULT_WEBPORT_Sentry			10000
#define DEFAULT_WEBPORT_HttpCRG			10080
#define DEFAULT_BINDPORT_Sentry				10020
#define DEFAULT_BINDPORT_Weiwoo				10100
#define DEFAULT_BINDPORT_SiteAdminSvc		10200
#define DEFAULT_BINDPORT_PathManager		10300
#define DEFAULT_BINDPORT_ContentStore		10400
#define DEFAULT_BINDPORT_CPE				10500
#define DEFAULT_BINDPORT_StreamSmith		10700
#define DEFAULT_BINDPORT_NSS				10800
#define DEFAULT_BINDPORT_ChODSvc			10900
#define DEFAULT_BINDPORT_EventChannel		11000
#define DEFAULT_BINDPORT_MODSvc				11100
#define DEFAULT_BINDPORT_ssm_ngod2			11200
#define DEFAULT_BINDPORT_ssm_TianShan_S1	11300
#define DEFAULT_BINDPORT_EdgeRM				11400
#define DEFAULT_BINDPORT_CPC				11500
#define DEFAULT_BINDPORT_PushManager		11600
#define DEFAULT_BINDPORT_BroadcastChannel	11700
#define DEFAULT_BINDPORT_VLCVSS			    10820
#define DEFAULT_BINDPORT_EdgeRMPHO			10301
#define DEFAULT_BINDPORT_MetaLib			11990
#define DEFAULT_BINDPORT_ContentLib			11900
#define DEFAULT_BINDPORT_CotentLibEventListen	 11901
#define DEFAULT_BINDPORT_A3MessageEventListen	 11401
#define DEFAULT_BINDPORT_BcastChEventListen	     11701
#define DEFAULT_BINDPORT_ERGEventListen	         11402
#define DEFAULT_BINDPORT_CDNSS_HTTP			12000
// TIP: for the unassigned listen port or outbound client connection to the server, most
//      UNIX systems take top-end of the range of ports, such as 45000-65000, while Microsoft
//      uses the ephemeral port range of 1024-5000. To prevent the potential conflict ports,
//      the following registry may help:
//      [HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters]
//      "ReservedPorts"==hex(7):31,00,30,00,30,00,30,00,30,00,2d,00,31,00,32,00,30,00,30,00,30,00,00,00,00,00
//      which means: ReservedPorts, REG_MULTI_SZ, 10000-12000

// the macros to format the default endpoint, adapter and interface name
#define DEFAULT_ENDPOINT(_COMPONENT)        "default -p " __STR1__(DEFAULT_BINDPORT_##_COMPONENT)
#define ADAPTER_NAME(_COMPONENT)	        __STR2__(_COMPONENT)
#define INTERFACE_NAME(_MODULE)	            __STR2__(_MODULE)

// the program can directly use above port definitions and the marcos of DEFAULT_ENDPOINT, ADAPTER_NAME and INTERFACE_NAME.
// there is no need to declare each as below. However, since many existing component use the old macros, the following
// just to eliminate the source changes of the existing components
#define DEFAULT_ENDPOINT_StreamSmith            DEFAULT_ENDPOINT(StreamSmith)

#define ADAPTER_NAME_Weiwoo				        ADAPTER_NAME(Weiwoo)
#define DEFAULT_ENDPOINT_Weiwoo			        DEFAULT_ENDPOINT(Weiwoo)
#define SERVICE_NAME_SessionManager		        INTERFACE_NAME(SessionManager)

#define ADAPTER_NAME_SiteAdminSvc		        ADAPTER_NAME(SiteAdminSvc)
#define DEFAULT_ENDPOINT_SiteAdminSvc	        DEFAULT_ENDPOINT(SiteAdminSvc)
#define SERVICE_NAME_BusinessRouter		        INTERFACE_NAME(BusinessRouter)

#define ADAPTER_NAME_PathManager		        ADAPTER_NAME(PathManager)
#define DEFAULT_ENDPOINT_PathManager	        DEFAULT_ENDPOINT(PathManager)
#define SERVICE_NAME_PathManager		        INTERFACE_NAME(PathManager)

#define ADAPTER_NAME_ContentStore               ADAPTER_NAME(ContentStore)
#define DEFAULT_ENDPOINT_ContentStore	        DEFAULT_ENDPOINT(ContentStore)
#define SERVICE_NAME_ContentStore		        INTERFACE_NAME(ContentStore)

#define ADAPTER_NAME_CPE				        ADAPTER_NAME(CPE)
#define DEFAULT_ENDPOINT_CPE			        DEFAULT_ENDPOINT(CPE)
#define SERVICE_NAME_ContentProvisionService	INTERFACE_NAME(ContentProvision)

#define ADAPTER_NAME_CPC						ADAPTER_NAME(CPC)
#define SERVICE_NAME_ContentProvisionCluster	INTERFACE_NAME(ContentProvisionCluster)
#define DEFAULT_ENDPOINT_CPC                    DEFAULT_ENDPOINT(CPC)
#define DEFAULT_ENDPOINT_PushManager            DEFAULT_ENDPOINT(PushManager)

#define SERVICE_NAME_AppService			        INTERFACE_NAME(AppService)

#define ADAPTER_NAME_Sentry				        ADAPTER_NAME(Sentry)
#define DEFAULT_ENDPOINT_Sentry			        DEFAULT_ENDPOINT(Sentry)
#define SERVICE_NAME_Sentry				        INTERFACE_NAME(Sentry)
#define DEFAULT_GROUPADDR_Sentry		        "239.200.200.1"
#define DEFAULT_GROUPPORT_Sentry		        (65001)
#define SERVICE_NAME_AdapterCollector	        INTERFACE_NAME(AdapterCollector)

#define ADAPTER_NAME_NSS				        ADAPTER_NAME(NSS)
#define DEFAULT_ENDPOINT_NSS			        DEFAULT_ENDPOINT(NSS)
#define SERVICE_NAME_NgodStreamService	        INTERFACE_NAME(NGODStream)

#define SERVICE_NAME_ChodSvc			        INTERFACE_NAME(ChodSvc)
#define DEFAULT_ENDPOINT_ChODSvc                DEFAULT_ENDPOINT(ChodSvc)

#define SERVICE_NAME_EventChannel		        INTERFACE_NAME(EventChannel)
#define DEFAULT_ENDPOINT_EventChannel	        DEFAULT_ENDPOINT(EventChannel)

#define SERVICE_NAME_MODSvc				        INTERFACE_NAME(MODSvc)
#define DEFAULT_ENDPOINT_MODSvc			        DEFAULT_ENDPOINT(MODSvc)

#define DEFAULT_ENDPOINT_ssm_ngod2              DEFAULT_ENDPOINT(ssm_ngod2)
#define DEFAULT_ENDPOINT_ssm_TianShan_S1        DEFAULT_ENDPOINT(ssm_TianShan_S1)

#define SERVICE_NAME_BroadcastChannel			INTERFACE_NAME(BroadcastChannel)
#define DEFAULT_ENDPOINT_BroadcastChannel       DEFAULT_ENDPOINT(BroadcastChannel)

#define ADAPTER_NAME_VLCVSS				        ADAPTER_NAME(VLCVSS)
#define DEFAULT_ENDPOINT_VLCVSS			        DEFAULT_ENDPOINT(VLCVSS)
#define SERVICE_NAME_VLCVSS                     INTERFACE_NAME(VLCVSS)

#define SERVICE_NAME_CacheStore                 INTERFACE_NAME(CacheStore)

#ifdef ZQ_OS_MSWIN
#define REG_KEY_Sentry					"SOFTWARE\\ZQ Interactive\\TianShan\\CurrentVersion\\Services\\Sentry"
#else
#define TIANSHAN_CONFIG					"/etc/TianShan.xml"
#endif
#define LOOPBACK_DEFAULT_PORT			11999

#ifdef WITH_ICESTORM
#  include <IceStorm/IceStorm.h>
#ifdef ZQ_OS_MSWIN
#  pragma comment(lib, "IceStorm" LIBSUFFIX)
#endif

#  define DEFAULT_ENDPOINT_TopicManager	"default -p 10100"
#  define SERVICE_NAME_TopicManager		"TianShanEvents/TopicManager"

#endif // WITH_ICESTORM

#ifdef ZQCOMMON_DLL
#  define CONN_TRACE(_CURRENT, _CLASS, _FUNC) if (_CURRENT.con) glog(ZQ::common::Log::L_DEBUG, CLOGFMT(_CLASS, #_FUNC "() by \"%s\""), c.con->toString().c_str());
#else
#  define CONN_TRACE(_CURRENT, _CLASS, _FUNC) ;
#endif

#ifndef max
#  define max(_X, _Y) (_X >_Y ? _X :_Y)
#endif //max

#define SYS_PROP_PREFIX  "sys."
#define USER_PROP_PREFIX "user."
#define SYS_PROP(_V) SYS_PROP_PREFIX #_V
#define USER_PROP(_V) USER_PROP_PREFIX #_V

///
/// Define the Ice configuration term for register key and default value.
/// All these setting is under the application's sub key, 
/// the sub key name is defined by macro ICE_SUBKEY_NAME
/// 
/// P_XXX_XXX_XXX   - for the property name
/// R_XXX_XXX_XXX   - for the registry key name
/// V_XXX_XXX_XXX   - for default value
///
 
#define ICE_SUBKEY_NAME                 L"ICE"

/// Ice Trace Properties
#define P_ICE_TRACE_NETWORK				"Ice.Trace.Network"
#define R_ICE_TRACE_NETWORK             L"TraceNetwork"
#define V_ICE_TRACE_NETWORK				0	// 0 - No network trace. (default).
                                            // 1 - Trace successful connection establishment and closure.
                                            // 2 - Like 1, but also traces attempts to bind, connect, and disconnect sockets.
                                            // 3 - Like 2, but also trace data transfer.

#define P_ICE_TRACE_PROTOCOL			"Ice.Trace.Protocol"
#define R_ICE_TRACE_PROTOCOL			L"TraceProtocol"
#define V_ICE_TRACE_PROTOCOL			0   // 0 - No protocol trace. (default)
                                            // 1 - Trace Ice protocol messages.

#define P_ICE_TRACE_RETRY				"Ice.Trace.Retry"
#define R_ICE_TRACE_RETRY				L"TraceRetry"
#define V_ICE_TRACE_RETRY				0   // 0 - No request retry trace. (default)
                                            // 1 - Trace Ice operation call retries.
									        // 2 - Also trace Ice endpoint usage.

#define P_ICE_TRACE_SLICING				"Ice.Trace.Slicing"
#define R_ICE_TRACE_SLICING				L"TraceSlicing"
#define V_ICE_TRACE_SLICING				0   // 0 - No trace of slicing activity. (default)
                                            // 1 - Trace all exception and class types that are unknown to the receiver and therefore sliced.

/// Ice Warning Properties
#define P_ICE_WARN_CONNECTIONS		    "Ice.Warn.Connections"
#define R_ICE_WARN_CONNECTIONS		    L"WarnConnections"
#define V_ICE_WARN_CONNECTIONS		    0   // 0   - No warning of connection. (default)
                                            // > 0 - Print warning messages for certain exceptional conditions in connections.

#define P_ICE_WARN_ENDPOINTS		    "Ice.Warn.Endpoints"
#define R_ICE_WARN_ENDPOINTS		    L"WarnEndpoints"
#define V_ICE_WARN_ENDPOINTS            1   // 0 - No printing of endpoints issue
                                            // 1 - Print warning message if the proxy's endpoint can not be parsed. (default)

/// Ice Object Adapter Properties
#define P_ADAPTER_THREADPOOL_SIZE(name)      name".ThreadPool.Size"
#define R_ADAPTER_THREADPOOL_SIZE            L"AdapterThreadPoolSize"
#define V_ADAPTER_THREADPOOL_SIZE            0     // 0 - The object adapter use the communicator's server thread pool. (default)

#define P_ADAPTER_THREADPOOL_SIZEMAX(name)   name".ThreadPool.SizeMax"
#define R_ADAPTER_THREADPOOL_SIZEMAX         L"AdapterThreadPoolSizeMax"
  // No Micro definition for SizeMax, 
  // its default value should be same to R_ADAPTER_THREADPOOL_SIZE's value which get from register

/// Ice Thread Pool Properties
#define P_ICE_THREADPOOL_CLIENT_SIZE         "Ice.ThreadPool.Client.Size"
#define R_ICE_THREADPOOL_CLIENT_SIZE         L"ThreadPoolClientSize"
#define V_ICE_THREADPOOL_CLIENT_SIZE         1

#define P_ICE_THREADPOOL_CLIENT_SIZEMAX      "Ice.ThreadPool.Client.SizeMax"
#define R_ICE_THREADPOOL_CLIENT_SIZEMAX      L"ThreadPoolClientSizeMax"
  // No Micro definition for SizeMax, 
  // its default value should be same to V_ICE_THREADPOOL_CLIENT_SIZE's value which get from register

#define P_ICE_THREADPOOL_SERVER_SIZE         "Ice.ThreadPool.Server.Size"
#define R_ICE_THREADPOOL_SERVER_SIZE         L"ThreadPoolServerSize"
#define V_ICE_THREADPOOL_SERVER_SIZE         1

#define P_ICE_THREADPOOL_SERVER_SIZEMAX     "Ice.ThreadPool.Server.SizeMax"
#define R_ICE_THREADPOOL_SERVER_SIZEMAX     L"ThreadPoolServerSizeMax"
  // No Micro definition for SizeMax, 
  // its default value should be same to V_ICE_THREADPOOL_SERVER_SIZE's value which get from register

/// Ice Default and Override Properties
#define P_ICE_OVERRIDE_TIMEOUT              "Ice.Override.Timeout"
#define R_ICE_OVERRIDE_TIMEOUT              L"OverrideTimeout"
#define V_ICE_OVERRIDE_TIMEOUT              -1		// No timeout by default

#define P_ICE_OVERRIDE_CONNECTTIMEOUT       "Ice.Override.ConnectTimeout"
#define R_ICE_OVERRIDE_CONNECTTIMEOUT       L"OverrideConnectTimeout"
#define V_ICE_OVERRIDE_CONNECTTIMEOUT       -1		// No timeout by default

/// Ice Miscellaneous Properties
#define P_ICE_LOGGER_TIMESTAMP              "Ice.Logger.Timestamp"
#define R_ICE_LOGGER_TIMESTAMP              L"LoggerTimestamp"
#define V_ICE_LOGGER_TIMESTAMP              1       // 0 - No timestamp.
                                                    // 1 - With timestamp. (default)

/// Ice evictor size
#define R_ICE_EVICTOR_SIZE(EvictorName)		EvictorName L"EvictorSize"



namespace ZQTianShan {

// -----------------------------
// schema definitions
// -----------------------------
/// schema definitions
typedef struct _ConfItem
{
	const char*			keyname;
	::TianShanIce::ValueType			type;
	//bool				optional;
	bool				optional2;
	const char*			hintvalue;
	bool				bRange;
} ConfItem;

typedef ::std::vector< Ice::Identity > IdentCollection;

// -----------------------------
// TianShanIce data type dump func
// -----------------------------
typedef void (*fDumpLine)(const char* line, void* pCtx);
void printLine(const char* line, void* pCtx=NULL);
void dumpResourceMap(const ::TianShanIce::SRM::ResourceMap& resources, const char* linePreffix=NULL, fDumpLine =printLine, void* pCtx=NULL);
void dumpResource(const ::TianShanIce::SRM::Resource& res, const char* linePreffix=NULL, fDumpLine =printLine, void* pCtx=NULL);
void dumpValueMap(const ::TianShanIce::ValueMap& vmap, const char* linePreffix=NULL, fDumpLine =printLine, void* pCtx=NULL);
void dumpVariant(const ::TianShanIce::Variant& val, const char* linePreffix=NULL, fDumpLine =printLine, void* pCtx=NULL);

const char* ObjStateStr(const ::TianShanIce::State state);
::TianShanIce::State StrToStateId(const char* stateStr);
const char* ResourceStatusStr(const ::TianShanIce::SRM::ResourceStatus status);
const char* ResourceAttrStr(const ::TianShanIce::SRM::ResourceAttribute attr);
const char* ResourceTypeStr(const ::TianShanIce::SRM::ResourceType& type);

::TianShanIce::Variant& PDField(::TianShanIce::ValueMap& PD, const char* field);

bool InterRestrictResource(const ::TianShanIce::SRM::Resource& res1, const ::TianShanIce::SRM::Resource& res2, ::TianShanIce::SRM::Resource& result);

std::string IceCurrentToStr(const ::Ice::Current& c);

size_t tokenize(TianShanIce::StrValues& tokens, const char* str, const char* delimiters=" \t\n\r");

#define TIMEINFINITE		(44148153599999)
//This value is equal to 2999-12-31 23��59��59.999
/// return the current GMT time in msec(win os),sec(linux os)
::Ice::Long now();

/// check if this is a DoS attack. test request speed, when it exceed the allowed speed, return false
///@param[in,out] timeEdge the last validate time stamp as the input, the current timestamp as the output
///@param[in,out] reqCount the count of requests for speed testing
///@param[in] windowSize the speed test time windows in second
///@param[in] maxRequests the max request allowed within the above time window
///@return false if the request speed has exceeded the limitation.
bool isDoSAttack(::Ice::Long& timeEdge, int& reqCount, int windowSize, int maxRequests);

/// return the UTC formatted time string
const char* TimeToUTC(::Ice::Long time, char* buf, const int maxlen , bool bLocalZone = false );
::Ice::Long ISO8601ToTime(const char* ISO8601Str);

/// return the full filename of this process
const char* getProgramPath();

/// return the root path of this process
const char* getProgramRoot();

/// return the module/plugin path of this process
const char* getModulesPath();

/// create DB folder and prepare the DB_CONFIG
std::string createDBFolder(ZQ::common::Log& log, const char* serviceName, const char* dataRoot, const char* databaseName,
		    const long cacheSizeKB = 160*1000, const long maxLocks=100*1000, const long maxObjects=100*1000, const long maxLockers=100*1000);
bool createDBFolderEx(ZQ::common::Log& log, const std::string& path, 
		    const long cacheSizeKB = 160*1000, const long maxLocks=100*1000, const long maxObjects=100*1000, const long maxLockers=100*1000);

#define ASSETVARIANT(_VAL, _TYPE) if (_TYPE != _VAL.type) ::ZQ::common::_throw<::TianShanIce::InvalidParameter> ("variant type %x doesn't match %x", _VAL.type, _TYPE);
#define OBJLOGFMT(_C, _X) CLOGFMT(_C, "%s[%s] " _X), c.id.category.c_str(), c.id.name.c_str() // this marco borrows (... const ::Ice::Current& c)
#define OBJEXPFMT(_C, _CODE, _X) EXPFMT(_C, _CODE, "%s[%s] " _X), c.id.category.c_str(), c.id.name.c_str() // this marco borrows (... const ::Ice::Current& c)

class AdapterBarker;
// -----------------------------
// Adapter shell with entries to Sentry Service
// -----------------------------
class Adapter : virtual public ::Ice::ObjectAdapter
{
public:

	// no public constructor, use static create() to create adapter
	virtual ~Adapter();

	typedef ::IceInternal::Handle< Adapter > Ptr;
	static Adapter::Ptr create(ZQ::common::Log& log, ::Ice::CommunicatorPtr& communicator, const char* name, const char* endpoint);

    // forwarded ObjectAdapter methods
	virtual ::std::string getName() const { return _theAdapter->getName(); }
    virtual ::Ice::CommunicatorPtr getCommunicator() const { return _theAdapter->getCommunicator(); }
    //!!! virtual ::Ice::ObjectPrx add(const ::Ice::ObjectPtr& obj, const ::Ice::Identity& id);
	//!!! virtual void activate();
    virtual void hold() { _theAdapter->hold(); }
    virtual void waitForHold() { _theAdapter->waitForHold(); }
    virtual void deactivate() { _theAdapter->deactivate(); }
    virtual void waitForDeactivate() { _theAdapter->waitForDeactivate(); }
    virtual ::Ice::ObjectPrx addFacet(const ::Ice::ObjectPtr& obj, const ::Ice::Identity& id, const ::std::string& facet) { return _theAdapter->addFacet(obj, id, facet);}
    virtual ::Ice::ObjectPrx addWithUUID(const ::Ice::ObjectPtr& obj) { return _theAdapter->addWithUUID(obj);}
    virtual ::Ice::ObjectPrx addFacetWithUUID(const ::Ice::ObjectPtr& obj, const ::std::string& facet)  { return _theAdapter->addFacetWithUUID(obj, facet);}
    virtual ::Ice::ObjectPtr remove(const ::Ice::Identity& id) { return _theAdapter->remove(id); }
    virtual ::Ice::ObjectPtr removeFacet(const ::Ice::Identity& id, const ::std::string& facet) { return _theAdapter->removeFacet(id, facet); }
    virtual ::Ice::FacetMap removeAllFacets(const ::Ice::Identity& id) { return _theAdapter->removeAllFacets(id); }
    virtual ::Ice::ObjectPtr find(const ::Ice::Identity& id) const  { return _theAdapter->find(id); }
    virtual ::Ice::ObjectPtr findFacet(const ::Ice::Identity& id, const ::std::string& facet) const { return _theAdapter->findFacet(id, facet); }
    virtual ::Ice::FacetMap findAllFacets(const ::Ice::Identity& id) const { return _theAdapter->findAllFacets(id); }
    virtual ::Ice::ObjectPtr findByProxy(const ::Ice::ObjectPrx& obj) const { return _theAdapter->findByProxy(obj); }
    virtual  void addServantLocator(const ::Ice::ServantLocatorPtr& sl, const ::std::string& name) { _theAdapter->addServantLocator(sl, name); }
    virtual ::Ice::ServantLocatorPtr findServantLocator(const ::std::string& s) const { return _theAdapter->findServantLocator(s); }
    virtual ::Ice::ObjectPrx createProxy(const ::Ice::Identity&id) const { return _theAdapter->createProxy(id); }
    virtual ::Ice::ObjectPrx createDirectProxy(const ::Ice::Identity&id) const  { return _theAdapter->createDirectProxy(id); }
    virtual ::Ice::ObjectPrx createIndirectProxy(const ::Ice::Identity&id) const  { return _theAdapter->createIndirectProxy(id); }
//new
	virtual void addDefaultServant(const ::Ice::ObjectPtr&, const ::std::string&){}
	virtual ::Ice::ObjectPtr removeDefaultServant(const ::std::string&){ return NULL;}
	virtual ::Ice::ServantLocatorPtr removeServantLocator(const ::std::string&){return NULL;}
	virtual ::Ice::ObjectPtr findDefaultServant(const ::std::string&) const {return NULL;}
	virtual ::Ice::LocatorPrx getLocator() const {return NULL;}
	virtual ::Ice::EndpointSeq getEndpoints() const {::Ice::EndpointSeq seq; return seq;}
	virtual ::Ice::EndpointSeq getPublishedEndpoints() const {::Ice::EndpointSeq seq; return seq;}

#if ICE_INT_VERSION / 100 < 303
    virtual ::Ice::ObjectPrx createReverseProxy(const ::Ice::Identity&id) const { return _theAdapter->createReverseProxy(id); }
#endif // ICE_INT_VERSION
    virtual void setLocator(const ::Ice::LocatorPrx& l) { _theAdapter->setLocator(l); }

#if ICE_INT_VERSION / 100 >= 302
	virtual void destroy() { _theAdapter->destroy(); }
	virtual bool isDeactivated() const { return _theAdapter->isDeactivated(); }
#endif // ICE_INT_VERSION

#if ICE_INT_VERSION / 100 >= 303
    virtual void refreshPublishedEndpoints() { return _theAdapter->refreshPublishedEndpoints(); }
#endif // ICE_INT_VERSION

    // override ObjectAdapter methods
    virtual void activate();

    // replaced ObjectAdapter methods
    virtual ::Ice::ObjectPrx add(const ::Ice::ObjectPtr& obj, const ::std::string& interfaceName);

    static bool publishLogger(const char* logfilename, const char* logsyntaxfile, const char* syntaxKey, const char*logtype = NULL, const ::TianShanIce::Properties& ctx = ::TianShanIce::Properties());
	static bool unpublishLogger(const char* logfilename);
    
    static const ::TianShanIce::StrValues& getServeAddress();
	static bool appendServAddrs(::std::string& endpoint);

	Ice::Long   getActivateTime() const;

	void closeBarker();
protected:
	
	Ice::ObjectAdapterPtr		_theAdapter;
	ZQ::common::Log&			_log;

	// forbidden ObjectAdapter methods
    virtual ::Ice::ObjectPrx add(const ::Ice::ObjectPtr& obj, const ::Ice::Identity& id) { throw "forbidden ObjectAdapter method"; }

	Adapter(ZQ::common::Log& log, ::Ice::ObjectAdapterPtr& iceAdapter, const char* endpoint);
    Ice::ObjectAdapterPtr& operator=(const ::Ice::ObjectAdapterPtr& adpater);

	friend class AdapterBarker;

	
	typedef ::IceInternal::Handle< AdapterBarker > AdapterBarkerPtr;
	AdapterBarkerPtr _pBarker;

	static Ice::Identity                    _gAdapterCBIdent;
	static ::Ice::ObjectAdapterPtr			_gCallbackAdapter;
	static uint32		                    _gPid;
	static ::TianShanIce::StrValues			_gTsNetIfs;

};

#ifndef _INDEPENDENT_ADAPTER
    #define ZQADAPTER_CREATE(_IC, _NM, _EP, _LOG) ZQTianShan::Adapter::create(_LOG, _IC, _NM, _EP)
    #define ZQADAPTER_ADD(_IC, _OBJ, _IF) add(_OBJ, _IF)
    #define ZQADAPTER_DECLTYPE ZQTianShan::Adapter::Ptr
#else
    #define ZQADAPTER_CREATE(_IC, _NM, _EP, _LOG) _IC->createObjectAdapterWithEndpoints(_NM, _EP)
    #define ZQADAPTER_ADD(_IC, _OBJ, _IF) add(_OBJ, _IC->stringToIdentity(_IF))
    #define ZQADAPTER_DECLTYPE Ice::ObjectAdapterPtr
#endif // _INDEPENDENT_ADAPTER

#define MAPSET(_MAPTYPE, _MAP, _KEY, _VAL) if (_MAP.end() ==_MAP.find(_KEY)) _MAP.insert(_MAPTYPE::value_type(_KEY, _VAL)); else _MAP[_KEY] = _VAL


template <class _EX>
class _IceThrow
{
public:
	
	_IceThrow(const char* category, int errorcode, const char* fmt, ...) PRINTFLIKE(4, 5)
	{
		char msg[2048];
		va_list args;

		va_start(args, fmt);
	//	vsnprintf(msg, 2040, fmt, args);
#ifdef ZQ_OS_MSWIN
		::_vsnprintf(msg,sizeof(msg)-1, fmt, args);
#else
		vsnprintf(msg,sizeof(msg)-1, fmt, args);
#endif
		va_end(args);

		throw _EX(category?category:"n/a", errorcode, msg);
	}

	_IceThrow(ZQ::common::Log& logger, const char* category, int errorcode, const char* fmt, ...)  PRINTFLIKE(5, 6)
	{
		char msg[2048];
		va_list args;

		va_start(args, fmt);
	//	vsnprintf(msg, 2040, fmt, args);
#ifdef ZQ_OS_MSWIN
		::_vsnprintf(msg,sizeof(msg)-1, fmt, args);
#else
		vsnprintf(msg,sizeof(msg)-1, fmt, args);
#endif
		va_end(args);

		try{ logger(ZQ::common::Log::L_ERROR, msg); } catch(...) {}

		throw _EX(category?category:"n/a", errorcode, msg);
	}

/*
	_IceThrow(const ::TianShanIce::BaseException& ex)
	{
		_EX newex(ex.category, ex.errorCode, ex.message);
		newex.ice_throw(); 
	}
*/
};

#define _IceReThrow(_EXTYPE, _EX) ::ZQTianShan::_IceThrow<_EXTYPE> (_EX.category.c_str(), _EX.errorCode, _EX.message.c_str())


#ifndef EXPFMT
#  ifdef _DEBUG
#	ifdef __FUNCTION__
#     define EXPFMT(_CATEGORY, _ERRCODE, _X) #_CATEGORY, _ERRCODE, "%s(%03d) %-18s " _X, __FUNCTION__, __LINE__, #_CATEGORY
#   else
#     define EXPFMT(_CATEGORY, _ERRCODE, _X) #_CATEGORY, _ERRCODE, "%s(%03d) %-18s " _X, __FILE__, __LINE__, #_CATEGORY
#   endif // __FUNCTION__
#  else
#    define EXPFMT(_CATEGORY, _ERRCODE, _X) #_CATEGORY, _ERRCODE, "%-18s " _X, #_CATEGORY
#  endif // _DEBUG
#  define EXPFMT0(_CATEGORY, _X) EXPFMT(_CATEGORY, 0, _X)
#endif //LOGFMT

#ifndef EventFMT // log based events, parsed by the sentry service and published as TianShanIce::GenericEvent(Sink)
#define EventFMT(_NETID, _CATEGORY, _EVENT, _EVENTCODE, _X) "Event[%s::%s(%d)] NetId[%s]: " _X, #_CATEGORY, #_EVENT, _EVENTCODE, _NETID
#endif //EventFMT

template<class T>
class DummyAmiCb : public T
{
public:
    
	DummyAmiCb(ZQ::common::Log& logger, const std::string& logPrefix ="") :
        _logger(logger), _logPrefix(logPrefix)
    {
    }
    
    virtual void ice_response()
    {
        // ok, success
    }
    
    virtual void ice_exception(const Ice::Exception& ex)
    {
        _logger(ZQ::common::Log::L_ERROR, CLOGFMT(DummyAmiCb, "%s caught exception[%s]"), _logPrefix.c_str(), ex.ice_name().c_str());
    }
    
private:
    ZQ::common::Log& _logger;
	std::string _logPrefix;
};

#define WatchDog_MAX_IDLE_INTERVAL (5 * 60 * 1000)

// -----------------------------
// class TimerWatchDog
// -----------------------------
/// classes to be under watching by this watchdog must be defined "class XXXX implements TianShanUtils::TimeoutObj" in ICE
class TimerWatchDog : public ZQ::common::ThreadRequest
{
	friend class TimeoutCmd;
public:
	/// constructor
    TimerWatchDog(ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, ZQADAPTER_DECLTYPE& adapter, const char* name=NULL, const int idleInterval =WatchDog_MAX_IDLE_INTERVAL);
	virtual ~TimerWatchDog();
	
	///@param[in] contentIdent identity of the object
	///@param[in] timeout the timeout to wake up timer to check the specified object
	void watch(const ::Ice::Identity& contentIdent, long timeout =0);
	
	//quit watching
	void quit();
	
protected: // impls of ThreadRequest
	
	virtual bool init(void);
	virtual int run(void);
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false);
	
	void wakeup();
	
protected:
	
	typedef std::map <Ice::Identity, Ice::Long > ExpirationMap; // sessId to expiration map
	ZQ::common::Mutex   _lockExpirations;
	ExpirationMap		_expirations;
	
	ZQ::common::Log& _log;
	ZQ::common::NativeThreadPool& _thrdPool;
	ZQADAPTER_DECLTYPE& _adapter;

	bool		  _bQuit;
	ZQ::common::Semaphore	_semWakeup;
	::Ice::Long   _nextWakeup;

	int			  _idleInterval;

private:
	::std::string _name;
};

struct LessIndentity
{
	bool operator()(const Ice::Identity& A, const Ice::Identity& B)
	{
		return (A.name.compare(B.name) <0);
	}
};

template<class KeyT>
//class MemoryIndex : virtual protected IceUtil::AbstractMutexReadI<IceUtil::RWRecMutex>
class MemoryIndex : virtual protected ICEAbstractMutexRLock
{
public:
	MemoryIndex() {}
	virtual ~MemoryIndex()
	{
		WLock sync(*this);
		_index.clear();
	}

	void insert(const KeyT key, ::Ice::Identity& ident)
	{
		WLock sync(*this);
		_index.insert(typename Index::value_type(key, ident));
	}

	void erase(const KeyT key, ::Ice::Identity& ident)
	{
		std::pair < typename Index::iterator, typename Index::iterator> range;

		WLock sync(*this);
		range = _index.equal_range(key);
		for (typename Index::iterator it = range.first; it != range.second; ++it)
		{
			if (it->second != ident)
				continue;

			_index.erase(it);
			break;
		}
	}

	ZQTianShan::IdentCollection find(const KeyT key)
	{
		ZQTianShan::IdentCollection result;
		std::pair <typename Index::iterator, typename Index::iterator> range;

		RLock sync(*this);
		range = _index.equal_range(key);
		for (typename Index::iterator it = range.first; it != range.second; ++it)
			result.push_back(it->second);

		return result;
	}

	bool has(const KeyT key, ::Ice::Identity& ident)
	{
		std::pair < typename Index::iterator, typename Index::iterator> range;

		RLock sync(*this);
		range = _index.equal_range(key);
		for ( typename Index::iterator it = range.first; it != range.second; ++it)
		{
			if (ident == it->second)
				return true;
		}

		return false;
	}

	int size() const
	{
		RLock sync(*this);
		return _index.size();
	}

protected:
	typedef std::multimap <KeyT, ::Ice::Identity > Index;
	Index _index;
};


class MemoryServantLocator;
class MemoryServantLocatorIterator : public Freeze::EvictorIterator
{
public:
	MemoryServantLocatorIterator(MemoryServantLocator& locater);
	virtual bool hasNext();
	virtual ::Ice::Identity next();
private:
	std::vector<Ice::Identity> mIds;
	size_t						mIndex;
};
typedef IceUtil::Handle<MemoryServantLocatorIterator> MemoryServantLocatorIteratorPtr;

class MemoryServantLocator : public Freeze::Evictor
{
public:	

	MemoryServantLocator( Ice::ObjectAdapterPtr a , const std::string& name);

	virtual void setSize(::Ice::Int size){mSize = size;}
	virtual ::Ice::Int getSize(){return mSize;}
	virtual ::Ice::ObjectPrx add(const ::Ice::ObjectPtr&, const ::Ice::Identity&);	
	virtual ::Ice::ObjectPtr remove(const ::Ice::Identity&);
	virtual void keep(const ::Ice::Identity&);	
	virtual void release(const ::Ice::Identity&);	
	virtual bool hasObject(const ::Ice::Identity&);	
	virtual ::Freeze::EvictorIteratorPtr getIterator(const ::std::string&, ::Ice::Int);

	virtual ::Ice::ObjectPrx addFacet(const ::Ice::ObjectPtr&, const ::Ice::Identity&, const ::std::string&){return NULL;}
	virtual ::Ice::ObjectPtr removeFacet(const ::Ice::Identity&, const ::std::string&) {return NULL;}
	virtual void keepFacet(const ::Ice::Identity&, const ::std::string&) {}
	virtual void releaseFacet(const ::Ice::Identity&, const ::std::string&) {}
	virtual bool hasFacet(const ::Ice::Identity&, const ::std::string&) {return false;}

	std::vector<Ice::Identity> getIds() const;

protected:
	virtual ::Ice::ObjectPtr locate(const ::Ice::Current&, ::Ice::LocalObjectPtr&) ;

	virtual void finished(const ::Ice::Current&, const ::Ice::ObjectPtr&, const ::Ice::LocalObjectPtr&);

	virtual void deactivate(const ::std::string&) ;

private:
	Ice::ObjectAdapterPtr		mAdapater;
	ZQ::common::Mutex			mLocker;	
	std::map<std::string , Ice::ObjectPtr>  mServants;
	int					mSize;//dummy
	std::string			mDbName;
};
typedef IceUtil::Handle<MemoryServantLocator> MemoryServantLocatorPtr;

} // namespace

#endif // __ZQTianShanDefines_H__
