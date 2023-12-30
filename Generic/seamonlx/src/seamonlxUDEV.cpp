///////////////////////////////////////////////////////////////////////////////
//
// SEAMONLX 
//
//  is a service that monitors the system, provides client apps with on demand
//  data and additionally broadcast alerts.
//
///////////////////////////////////////////////////////////////////////////////

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
#include "common.h"
#include "Disks.h"
#include "seamonlxUDEV.h"

extern THREAD_INFO_ELEM ThreadInfo[MAX_START_THREADS];

using namespace seamonlx;
//
// IsRealUevent
//
// Helper routine; Tests for add remove events, for modules and disks
// additionally set for bccfg or 
//
// output rtn = SUCCESS, FAILURE 
//
int IsRealUevent(char *msg)
{
	int rtn = FAILURE;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "IsRealUevent, Enter");
	//
	// only write Add remove to Alert Log
	//
	if ((strcasestr(msg,"add") != NULL) || (strcasestr(msg,"remove") != NULL)) {

		if (strcasestr(msg, "module") != NULL) {
			rtn = SUCCESS;
		} else {
			if (strcasestr(msg, "scsi_device") != NULL) {
				rtn = SUCCESS;
			}
		}
		
		//
		// test specifically for need to watch shas events
		//
		if (strcasestr(msg, "rcha") != NULL) {
			if (strcasestr(msg,"add") != NULL) {
				rchaIsLoaded = 1;
			} else {
				rchaIsLoaded = 0;
			}
		}

		//
		// both modules (rcha, bccfg) must be load to monitor shas
		//
		if (strcasestr(msg, "bccfg") != NULL) {
			if (strcasestr(msg,"add") != NULL) {
				bccfgIsLoaded = 1;
			} else {
				bccfgIsLoaded = 0;
			}
		}
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "IsRealUevent, Exit");
	return rtn;
}

//
// UpdatePhysicalDisks, helper to handle add and reove of Disk Objects
//
int UpdatePhysicalDisks(char *buf)
{
	char	dsikstringname[BUFF32];
	Disks *mydisk = Disks::instance();
	char *pos;
	int rtn = FAILURE;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "UpdatePhysicalDisks, Enter");
	pos = strrchr(buf,'/');
	if ((pos != NULL) && (pos++ != NULL)){
		sprintf(dsikstringname, "%s", pos);
	
		if (strcasestr(buf,"add@") != NULL) {
			mydisk->addDisk(dsikstringname);
		} else {
			mydisk->deleteDisk(dsikstringname);
		}
		//
		// update disk array
		//
		mydisk->update();
		rtn = SUCCESS;
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "UpdatePhysicalDisks, Exit");
	return rtn;
}

// 
// MonitorUDEV: Thread which is always monitoring for UDEV events
// Checks the socket for incoming messgaes
//
void *MonitorUDEV(void *pParam)
{
	struct sockaddr_nl nls;
	struct pollfd pfd;
	int	len;
	int alertid;
	char recv_buf[BUFF4K];
	char msg[BUFF80];
	int socketcreated = FAILURE;
	int updatediskrtn = FAILURE;
	
	
	long threadnum;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorUDEV, Enter");
	threadnum = (long)((long *)pParam);	
	ThreadInfo[threadnum].threadstate = THREAD_STARTED;
	
	//
	// open uevent, utilize existing netlink socket
	//
	memset(&nls,0,sizeof(struct sockaddr_nl));
	nls.nl_family = AF_NETLINK;
	nls.nl_pid = getpid();
	nls.nl_groups = -1;

	while (!seamonlx_shutdown) {
	
		//
		// only do once
		//
		if (socketcreated != SUCCESS) {
			pfd.events = POLLIN;
			pfd.fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
		}
		
		if (pfd.fd != -1) {
			socketcreated = SUCCESS;
			// 
			// Listen for uevents
			//
			if (!bind(pfd.fd, (struct sockaddr *) &nls, sizeof(struct sockaddr_nl))) {
				while (-1 != poll(&pfd, 1, -1)) {
					memset(recv_buf, 0, BUFF4K);
					len = recv(pfd.fd, recv_buf, BUFF4K, MSG_DONTWAIT);
					//
					// check for events written to socket
					//
					if ((recv_buf != NULL) && (len > 0) && (IsRealUevent(recv_buf) == SUCCESS)) {
						
						if (strcasestr(recv_buf,"scsi_generic") != NULL) {
							updatediskrtn = UpdatePhysicalDisks(recv_buf);
							if (updatediskrtn != SUCCESS) {
								sprintf(msg, "%s: %s", __FUNCTION__, "UpdatePhysicalDisks failed");
								die(msg, 0, Gsv_AI_FAIL_ON_UPDATE_DISKS);
							}
						}

						if (strcasestr(recv_buf,"add@") != NULL) {
							if (strcasestr(recv_buf,"module") != NULL) {
								alertid = Gsv_AI_UDEV_ADD_MODULE;
							} else {
								alertid = Gsv_AI_UDEV_ADD_DISK;
							}
						} else if (strcasestr(recv_buf,"remove@") != NULL) {
							if (strcasestr(recv_buf,"module") != NULL) {
								alertid = Gsv_AI_UDEV_REMOVE_MODULE;
							} else {
								alertid = Gsv_AI_UDEV_REMOVE_DISK;
							}
						} else {
							alertid = Gsv_AI_UDEV_UNKNOWN_STATUS; // unknown
						}

						processAlertMsg("UDEV", "WARN", CN_System_Services, recv_buf, alertid);
						break;
					}
				} // end poll loop

			} else {
				sprintf(msg, "%s: %s", __FUNCTION__, "Socket Bind failed");
				die(msg, 0, Gsv_AI_SOCKET_ERROR);
				close(pfd.fd);
				socketcreated = FAILURE;
			}
				
		} else {
			socketcreated = FAILURE;
			sprintf(msg, "%s: %s", __FUNCTION__, "Failed to create socket");
			die(msg, 0, Gsv_AI_SOCKET_ERROR);
		}
		//
		// give other threads a chance
		//
		sleep(1);
	}

	if (pfd.fd != -1)
		close(pfd.fd);

	ThreadInfo[threadnum].threadstate = THREAD_STOPPED;
	pthread_exit(NULL);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorUDEV, Exit");
}
