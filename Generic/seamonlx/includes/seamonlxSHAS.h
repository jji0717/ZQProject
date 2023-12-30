///////////////////////////////////////////////////////////////////////////////
//
//  seamonlxSHAS.h
//
//  uses shasvc service that monitors the system for drive inserts and removals
//  and forwards those events into SHAS.  This enables SHAS to dynamically 
//  respond to drive insertions and removals.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef SEAMONLXSHAS_H // {
#define SEAMONLXSHAS_H		1 

//
// critical for using RC calls, crazy internal build thing
//
#define RC_SW_PLATFORM_TYPE 14
#define RC_SW_DEBUG off

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <ctype.h>
#include <pthread.h>

#include <sys/types.h>
#include <dirent.h>

#include <errno.h>

#include "rc_types.h"
#include "rctl_api.h"
#include "bcapi.h"

#include "seamonlx.h"
#include "common.h"

extern pthread_t			monitorSHASThread;

void *MonitorSHAS(void *);

extern RC_UINT32 wideToSingle(RC_CHAR *, const RC_WCHAR *);

extern int parseSHASmsg(RC_CHAR *,RC_INT32 *,RC_INT32 *,RC_CHAR *,RC_UINT32,RC_UINT32);

extern void processAlertMsg(char *fac, char *sev, char *component, char *desc, int alertid);

extern int seamonlx_shutdown;
extern int rchaIsLoaded;
extern int bccfgIsLoaded;

#endif // }
