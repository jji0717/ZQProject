///////////////////////////////////////////////////////////////////////////////
//
//  SEAMONLXALERT.h 
//
///////////////////////////////////////////////////////////////////////////////

#ifndef SEAMONLXALERT_H // {
#define SEAMONLXALERT_H		1 

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
#include <syslog.h>

#include "common.h"
#include "seamonlx.h"

#define PRIMARYFILEPATH			"/var/log/alert.log"
#define SECONDARYFILEPATH		"/var/log/alert.log.1"

#define BACKLOG					10					// how many pending connections queue will hold
#define	MAX_CONN_ALLOWED		100

// used to set up requests to listen for UNICAST alerts
		
void listenersetup(void);

typedef struct _ALERT_STRUCT
{
	char	timestamp[BUFF32];					// should match alert.log timestamp
	char	hostsrc[BUFF32];					// name written by alert
	long	seqnum;								// SeqNum wriiten
	int		alertid;							// add, remove enum, from enum		
	char	severity[BUFF8];					// what we decide
	char	facility[BUFF32];					// which seamonlx monitoring (UDEV, RC, seamonlx etc.) 
	char    componentname[BUFF32];				// component name
	char	recommendation[BUFF1K];				// what do we recommend for this alert
	char	description[BUFF1K];				// raw message from UDEV, RC shas, seamonlx etc.	
}ALERT_STRUCT, *PALERT_STRUCT;


typedef struct _ALERT_RANGE
{
	long	OldestSeqNum;							// FirstSeqNum wriiten
	long	NewestSeqNum;							// LastSeqNum wriiten
}ALERT_RANGE, *PALERT_RANGE;

#endif // }
