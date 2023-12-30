#ifndef __ZQ_StreamNow_StreamSmith_H__
#define __ZQ_StreamNow_StreamSmith_H__

#ifdef STREAMSMITHAPI_EXPORTS
#define	StreamSmith_API __declspec(dllexport)
#else
#define	StreamSmith_API __declspec(dllimport)
#endif // STREAMSMITHAPI_EXPORTS

#ifdef STREAMSMITHAPI_STATIC
#undef StreamSmith_API
#define StreamSmith_API
#endif // STREAMSMITHAPI_STATIC

#include "ZQ_common_conf.h"
#include "Guid.h"
#include "Log.h"
#include "NativeThreadPool.h"

#define INVALID_CTRL_NUM			(-1)
#define DEFAULT_SS_ENDPOINT			"\\pipe\\StreamSmith"
#define DEFAULT_SS_ADDR				NULL
#define DEFAULT_SS_PROTO			"ncacn_np"
#define DEFAULT_SS_SINK_ENTRYNAME	"/.:/SeaChange/StreamSmith.Sink"
#define DEFAULT_SS_SERV_ENTRYNAME	"/.:/SeaChange/StreamSmith"
#define DEFAULT_THREADPOOL_NUM      (10)
#define MAX_SS_EVENT_THREADS        (20)

//TODO: maybe create another flags other than reservedFlags
#define PL_ITEM_MASK_SPLICEIN		(1 <<0)
#define PL_ITEM_MASK_SPLICEOUT		(1 <<1)
#define PL_ITEM_MASK_FORCENORMAL	(1 <<2)

extern ZQ::common::NativeThreadPool&			_gThreadPool;

extern "C"
{
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
}

//extern ZQ::common::Log*     pGlog;

#endif // __ZQ_StreamNow_StreamSmith_H__
