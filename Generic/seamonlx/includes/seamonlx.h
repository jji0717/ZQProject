///////////////////////////////////////////////////////////////////////////////
//
//  Seamonlx.h
//
///////////////////////////////////////////////////////////////////////////////

#ifndef SEAMONLX_H // {

#define SEAMONLX_H		1 

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


#include "seamonlxUDEV.h"
#include "seamonlxSHAS.h"
#include "seamonlxXMLRPC.h"
#include "seamonlxalert.h"

void die(char *s, int killit, int alertid);

void signal_handler(int signo);



// includes for other threads

/*

#include "seamonlxDRIVE.h"
#include "seamonlxHBA.h"
#include "seamonlxIPMI.h"
#include "seamonlxNETWORK.h"
#include "seamonlxINFINIBAND.h"
#include "seamonlxENCLOSURE.h"
*/




#endif // }
