///////////////////////////////////////////////////////////////////////////////
//
//  seamonlxUDEV.h
//
//  uses UDEV service that monitors the system for drive inserts and removals
//  and forwards those events via UNICAST alert msg.  This enables seamonlx to dynamically 
//  respond to drive insertions and removals.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef SEAMONLXUDEV_H	// {
#define SEAMONLXUDEV_H		1

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

extern pthread_t   monitorUDEVThread;

#include "seamonlx.h"
#include "common.h"

void *MonitorUDEV(void *);

extern int IsRealUevent(char *);

extern void processAlertMsg(char *fac, char *sev, char *component, char *desc, int alertid);

extern int seamonlx_shutdown;
extern int rchaIsLoaded;
extern int bccfgIsLoaded;

#endif // }
