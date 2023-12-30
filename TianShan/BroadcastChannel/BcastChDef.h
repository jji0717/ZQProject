#ifndef __BcastChDef_h__
#define __BcastChDef_h__
#include "ZQ_common_conf.h"
#define ADAPTER_NAME_BcastChannel		    "BroadcastChannelAdapter"
#define DEFAULT_ENDPOINT_BcastChannel	    "default -p 11700"

#define SERVICE_NAME_BcastchannelPublisher	"BcastChannelPublisher"
#define Broadcast_APPNAME		            "BcastChannelApp"

#define err_300 300
#define err_301 301

#define err_330 330

#define err_400 400
#define err_401 401
#define err_402 402
#define err_403 403
#define err_404 404

#define err_500 500
#define err_501 501
#define err_502 502
#define err_503 503
#define err_504 504

#define err_600 600
#define err_601 601
#define err_602 602
#define err_603 603
#define err_604 604
#define err_605 605
#define err_606 606
#define err_607 607

#define err_700 700
#define err_701 701

#define err_800 800

#define err_820 820
#define err_821 821
#define err_822 822

#define MAX_PORT        65535

#define RESKEY_BcastPPName "reskey_bcastppname"

#define RESKEY_Interval    "Interval"
#define RESKEY_Iterator    "Iteration"
#define RESKEY_IpPort      "IpPort"
#define RESKEY_UpTimpe     "UpTime" 

#define BcastChannel_Type     "BcastChannel"
#define NVODChannel_Type      "NVODChannel"
#define NVODSupplChannel_Type "NVODSupplementalChannel"

#define DEFAULTVOLUME          "/$/"
#define DEFAULTWINDOWSIZE       20
#define MINIMUMPLITEMCOUNT     5

extern 
bool localTime2TianShanTime(const char* szTime, int64& lTime);
extern
bool systemTime2TianShanTime(const char* szTime, int64& lTime);

#endif /// end define __BcastChDef_h__
