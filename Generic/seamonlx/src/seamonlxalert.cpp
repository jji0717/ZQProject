///////////////////////////////////////////////////////////////////////////////
//
// seamonlxalert
//
//  Writes Alerts to the /var/log/alerts.log and broadcasts Alerts.
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>

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

#include "seamonlxalert.h"

//
// condition predicate for multi threading
//
int		ReadyToWriteLog		= 0;
int		WORDSPERLINE		= 25;

int		sockfd;					// the listener socket

int ConnArray[MAX_CONN_ALLOWED];  

char alertString[BUFF4K];

extern THREAD_INFO_ELEM ThreadInfo[MAX_START_THREADS];

// Helper to get sockaddr, for IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "get_in_addr, Enter");
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
	}	

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "get_in_addr, Exit");
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


//
// Setup the listener
//
void listenersetup()
{
	int				oldflags;

	struct addrinfo hints, *servinfo, *p;
    
    int yes=1;
    
	int badsocket = FAILURE;
	int fcntlbad = FAILURE;
	char sockerrstring[BUFF1K];
	char portstring[BUFF32];
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "listenersetup, Enter");
	//
	// use my IP
	//
	memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;				

	//
	// find correct socket
	//
	sprintf(portstring,"%d",Gsv_SEAMONLX_ALERT_PORT);
	if (!getaddrinfo(NULL, portstring, &hints, &servinfo)) {
		for(p = servinfo; p != NULL; p = p->ai_next) {
			if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) != -1) {
				if (!setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
					if (!bind(sockfd, p->ai_addr, p->ai_addrlen)) {
						//
						// found good socket to bind too break out
						//
						badsocket = SUCCESS;
						break;
					} 
				}
				//
				// be sure to close socket
				// if we can't reuse or can't bind
				//
				close(sockfd);

			} else {
				sprintf(sockerrstring, "%s: %s errno = %d",  __FUNCTION__, "socket call failed", errno);
			}
		}
		freeaddrinfo(servinfo); // all done with this structure

	} else {
		sprintf(sockerrstring, "%s: %s errno = %d", __FUNCTION__, "getaddrinfo failed", errno);
	}

	if (badsocket == SUCCESS) {
		//
		// attempt to set the socket to non blocking
		//
		if ((oldflags = fcntl(sockfd, F_GETFL)) != -1) {
			if (fcntl(sockfd, F_SETFL, oldflags | O_NONBLOCK) != -1) {
				if (listen(sockfd, BACKLOG) != -1) {
					fcntlbad = SUCCESS;
				} else {
					sprintf(sockerrstring, "%s: %s errno = %d", __FUNCTION__, "listen failed", errno);
				}
			} else {
				sprintf(sockerrstring, "%s: %s errno = %d", __FUNCTION__, "fcntl failed on F_SETFL to NON block", errno);
			}
		} else {
			sprintf(sockerrstring, "%s: %s errno = %d", __FUNCTION__, "fcntl failed  on F_GETFL", errno);
		}

		if (fcntlbad != SUCCESS) {
			close(sockfd);
			die(sockerrstring, 0, Gsv_AI_SOCKET_ERROR);
		}

	} else {
		die(sockerrstring, 0, Gsv_AI_SOCKET_ERROR);
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "listenersetup, Exit");
}

//
// add in fd to send broadcast alert , update global ConnArray[] entry
// input new_fd -- socket decriptor to bind to
//
static void addtoConnArray(int new_fd)
{
	int i;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "addtoConnArray, Enter");
	for (i=0; i < MAX_CONN_ALLOWED; i++) {
		if (ConnArray[i] == 0) {
			ConnArray[i] = new_fd;
			break;
		}
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "addtoConnArray, Exit");
}
//
// accept connections thread, maintians ConnArray[]
// 

void *acceptconnections(void *pParam)
{
	int new_fd;									// listen on sock_fd, new connection on new_fd
    struct sockaddr_storage their_addr;			// connector's address information
    socklen_t sin_size;
    
	char sockerrstring[BUFF1K];
	long threadnum;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "acceptconnections, Enter");
	threadnum = (long)((long *)pParam);	
	ThreadInfo[threadnum].threadstate = THREAD_STARTED;
    while (!seamonlx_shutdown) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd != -1) {
			//
			// save away in conn array
			//
			pthread_mutex_lock(&ConnArrayMutex);
			addtoConnArray(new_fd);
			pthread_mutex_unlock(&ConnArrayMutex);

			if (DEBUG) {
				char s[INET6_ADDRSTRLEN];
				inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
			}
        
		} else {
			sprintf(sockerrstring, "%s: %s errno = %d", __FUNCTION__, "accept call failed", errno);
        }
		sleep(1);
	}

	ThreadInfo[threadnum].threadstate = THREAD_STOPPED;
	pthread_exit(NULL);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "acceptconnections, Exit");
}

//
// routine to send to all connected sockets, use gloabl ConnArray[]
//

static int UniCastAlert(char *xmlstring, int len)
{
	int i;
	int retval = -1;
	int badsocket = 0;
	char sockerrstring[BUFF1K];
	struct pollfd curr;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "UniCastAlert, Enter");
	//
	// initialize poll structure
	//
	curr.events = POLLOUT;
	curr.revents = 0;
        
	for (i = 0; i < MAX_CONN_ALLOWED; i++) {
		if (ConnArray[i] != 0) {
			curr.fd =  ConnArray[i];
			badsocket = 0;

			//
			// attempt to send
			//
			if (send(curr.fd, xmlstring, len, 0) == -1) {
				if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
					//
					// wait 1 sec
					//
					retval = poll(&curr, 1, 1000);
					if (retval > 0) {
						//
						// attempt to send since poll() said curr.fd is ready
						//
						if (send(curr.fd, xmlstring, len, 0) == -1) {
							//
							// misbehaving connection, need to remove it
							//
							sprintf(sockerrstring, "%s: %s errno = %d", __FUNCTION__, "Misbehaving conn", errno);
							badsocket = 1;
						}
					} else {
						//
						// poll() timeout bad connection
						//
						sprintf(sockerrstring, "%s: %s errno = %d",  __FUNCTION__, "conn timed out", errno);
						badsocket = 1;
					} 
				} else {
						//
						// other socket error
						//
					sprintf(sockerrstring, "%s: %s errno = %d",  __FUNCTION__, "send critical error", errno);
						badsocket = 1;
				}
			}
			//
			// did we have a socket error on send() or poll() 
			// tear down conn and remove from ConnArray[]
			//
			if (badsocket) {
				pthread_mutex_lock(&ConnArrayMutex);
				close(ConnArray[i]);
				ConnArray[i] = 0;
				pthread_mutex_unlock(&ConnArrayMutex);
			}
		} 
    }

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "UniCastAlert, Exit");
    return 0;
}

//
// format the alert message, then call UniCastAlert
//
int buildUnicastxmlString(ALERT_STRUCT *alert)
{
	char xmlstring[BUFF4K];
	char tmpstr[BUFF2K];
	int len;
	int rtn = FAILURE;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "buildUnicastxmlString, Enter");
	memset(xmlstring, 0, sizeof(xmlstring));
	sprintf(xmlstring,"%s", "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>");

	strcat(xmlstring, "<ALERT>");
	memset(tmpstr, 0, sizeof(tmpstr));
	sprintf(tmpstr, "<SEQ>%ld</SEQ>", alert->seqnum);
	strcat(xmlstring, tmpstr);

	memset(tmpstr, 0, sizeof(tmpstr));
	sprintf(tmpstr, "<ID>%d</ID>", alert->alertid);
	strcat(xmlstring, tmpstr);

	memset(tmpstr, 0, sizeof(tmpstr));
	sprintf(tmpstr, "<SEVERITY>%s</SEVERITY>", alert->severity);
	strcat(xmlstring, tmpstr);

	memset(tmpstr, 0, sizeof(tmpstr));
	sprintf(tmpstr, "<FACILITY>%s</FACILITY>", alert->facility);
	strcat(xmlstring, tmpstr);

	memset(tmpstr, 0, sizeof(tmpstr));
	sprintf(tmpstr, "<COMPONENT>%s</COMPONENT>", alert->componentname);
	strcat(xmlstring, tmpstr);

	memset(tmpstr, 0, sizeof(tmpstr));
	sprintf(tmpstr, "<DESCRIPTION>%s</DESCRIPTION>", alert->description);
	strcat(xmlstring, tmpstr);
	
	memset(tmpstr, 0, sizeof(tmpstr));
	sprintf(tmpstr, "<RECOMMENDATION>%s</RECOMMENDATION>", alert-> recommendation);
	strcat(xmlstring, tmpstr);

	memset(tmpstr, 0, sizeof(tmpstr));
	sprintf(tmpstr, "<TIMESTAMP>%s</TIMESTAMP>", alert->timestamp);
	strcat(xmlstring, tmpstr);

	strcat(xmlstring,"</ALERT>");
	
	//
	// broadcast the alert
	//
	len = strlen(xmlstring);
	rtn = UniCastAlert(xmlstring, len+1);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "buildUnicastxmlString, Exit");
	return rtn;
}

//
// Build up the alert struct to be used to UNICAST the Alert
// if necessary
//
static int ParseAndBuildAlert(char *buf, ALERT_STRUCT *alert)
{
	int rtn = FAILURE;
	char seps[]   = " ,;:";
	char *token;
	char *saveptr;
	char timestamp[BUFF32];
	char description[BUFF1K];
	char recommendation[BUFF1K];
	char *pos;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "ParseAndBuildAlert, Enter");
	memset(description,0, BUFF1K);
	memset(recommendation,0, BUFF1K);
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "the parameter of the function ParseAndBuildAlert :buf=%s ",buf);
	//
	// remove last new line not really part of alerrt
	//
	pos = strrchr(buf,'\n');
	if (pos != NULL) {
		*pos = '\0';
	}
	else
		return rtn;
	//
	// extract timestamp
	// 
	memset(timestamp, 0, sizeof(timestamp));
	token = strtok_r(buf, seps, &saveptr);					// MON
	if (token != NULL) 
	{
		strcat(timestamp, token);	
		strcat(timestamp," ");
		token = strtok_r(NULL,seps, &saveptr);				// DD
		strcat(timestamp, token);
		strcat(timestamp," ");
		token = strtok_r(NULL,seps, &saveptr);				// HH
		strcat(timestamp, token);
		strcat(timestamp,":");
		token = strtok_r(NULL,seps, &saveptr);				// MM
		strcat(timestamp, token);
		strcat(timestamp,":");
		token = strtok_r(NULL,seps, &saveptr);				// SS
		strcat(timestamp, token);
		sprintf(alert->timestamp,"%s", timestamp);

		//
		// extract host source name 
		//
		token = strtok_r(NULL, seps, &saveptr);
		sprintf(alert->hostsrc, "%s", token);

		//
		// SequenceNum
		// 
		token = strtok_r(NULL,seps, &saveptr);
		if (token != NULL) 
			alert->seqnum = atol(token);

		//
		// Alert ID
		//
		token = strtok_r(NULL,seps, &saveptr);
		if (token != NULL) 
			alert->alertid = atoi(token);

		//
		// Facility
		//
		token = strtok_r(NULL,seps, &saveptr);
		if (token != NULL) 
			sprintf(alert->facility,"%s", token);

		// 
		// Severity
		//
		token = strtok_r(NULL,seps, &saveptr);
		if (token != NULL) 
			sprintf(alert->severity,"%s", token);

		// 
		// Component
		//
		token = strtok_r(NULL,seps, &saveptr);
		if (token != NULL) 
			sprintf(alert->componentname,"%s", token);
	
		//
		// Description
		//
		token = strtok_r(NULL,seps, &saveptr);
		while (token != NULL) {
			strcat(description, token);
			token = strtok_r(NULL, seps, &saveptr);
			
			//
			// in case of spaces
			// must have delimeter of "+++" for beginning of Recc
			//
			if (token != NULL) {
				strcat(description," ");
				if (strcasecmp(token,"+++") == 0) {
					break;
				} 
			}
		}
		sprintf(alert->description,"%s", description);

		//
		// recommendation
		//
		memset(recommendation, 0, sizeof(recommendation));
		token = strtok_r(NULL,seps, &saveptr);
		while (token != NULL) {
			strcat(recommendation, token);
			token = strtok_r(NULL, seps, &saveptr);
			//
			// in case of spaces
			//
			if (token != NULL) {
				strcat(recommendation," ");
			}
		}
		sprintf(alert->recommendation,"%s", recommendation);

		rtn = SUCCESS;

	} 
	else 
		rtn = FAILURE;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "ParseAndBuildAlert, Exit");
	return rtn;
}


//
// Helper function positions the FP at the the beinning of the current line
//

static void seek_line_start(FILE *fp)
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "seek_line_start, Enter");
    /* after a gets() the file pointer will point to the character after the newline
       or at EOF.  Before we can start to look for the beginning of the current line
       we need to start at the end of the line which is marked by '\n'.  The following
       check makes the adjustment.  
	*/
    if (!fseek(fp, -1, SEEK_CUR)) {
        if (fgetc(fp) == 0xa)
            fseek(fp, -1, SEEK_CUR);
    }

    /* seek from '\n' to the next occurrence of '\n'. The last fgetc() call we make
       will point us at the next character after the last '\n' that was encountered.  
	*/
    while (!fseek(fp, -1, SEEK_CUR)) {
        if (fgetc(fp) == 0xa)
            break;
        else
            fseek(fp, -1, SEEK_CUR);
    }

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "seek_line_start, Exit");
}

//
// Helper function positions the fp at beginning of previous line
//
int seek_prev_line(FILE *fp)
{
	int rtn = FAILURE;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "seek_prev_line, Enter");

    // go to beginning of current line
    seek_line_start(fp);

    // check for beginning of file 
    if (!fseek(fp, -1, SEEK_CUR)) {
        // go to beginning of prev line 
        seek_line_start(fp);
        rtn = SUCCESS;
    }

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "seek_prev_line, Exit");
    return rtn; 
}

//
// get first line of alert.log file
//
int headAlertLog(ALERT_STRUCT *alert)
{
	char locbuf[BUFF2K];
	int rtn = FAILURE;
	FILE *fp;
	char msg[BUFF256];

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "headAlertLog, Enter");
	fp = fopen(PRIMARYFILEPATH, "r");
	if (fp != NULL) {
		
		memset(locbuf,0, sizeof(locbuf));
		// seek to first line 
		fseek(fp, 0, SEEK_SET);
			
		// seek to beginning of last line
		seek_line_start(fp);
				
		// now parse the log alert entry
		if (fgets(locbuf, BUFF2K, fp) != NULL) {
			rtn = ParseAndBuildAlert(locbuf, alert);
		} else {
			rtn = FAILURE;
		}	

		fclose(fp);

	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "fopen() failed");
		die(msg, 0,Gsv_AI_FOPEN_ERROR);
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "headAlertLog, Exit");
	return rtn;
}

//
// get last line of alert.log file
//
int tailAlertLog(ALERT_STRUCT *alert, char *filepath)
{
	char locbuf[BUFF2K];   // 2k max len
	int rtn = FAILURE;
	FILE *fp;
	char msg[BUFF80];
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "tailAlertLog, Enter");
	fp = fopen(filepath, "r");
	if (fp != NULL) {
		memset(locbuf,0, sizeof(locbuf));
		
		// seek to eof 
		fseek(fp, 0L, SEEK_END);
				
		// seek to beginning of last line
		seek_line_start(fp);
			
		// now parse the log alert entry
		if (fgets(locbuf, BUFF2K, fp) != NULL) {
			rtn = ParseAndBuildAlert(locbuf, alert);
		} else {
			rtn = FAILURE;
		}

		fclose(fp);

	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "fopen() failed");
		die(msg, 0, Gsv_AI_FOPEN_ERROR);
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "tailAlertLog, Exit");
	return rtn;
}

//
// Get Sequence Numbers Oldest and newest in current alert.log file
//
int getAlertRange(ALERT_RANGE *alertrange)
{
	ALERT_STRUCT alert;
	int rtn = FAILURE;
	char msg[BUFF256];

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "getAlertRange, Enter");
	rtn = tailAlertLog(&alert, PRIMARYFILEPATH);
	if (rtn == SUCCESS) {

		alertrange->NewestSeqNum = alert.seqnum;

		rtn = headAlertLog(&alert);
		if (rtn == SUCCESS) {
			alertrange->OldestSeqNum = alert.seqnum;
		} else {
			sprintf(msg, "%s: %s", __FUNCTION__, "failed to get OldestSeqNum");
			die(msg, 0, Gsv_AI_ALERT_RANGE_ERROR);
			rtn = FAILURE;
		}

	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "failed to get NewestSeqNum");
		die(msg, 0, Gsv_AI_ALERT_RANGE_ERROR);
		rtn = FAILURE;
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "getAlertRange, Exit");
	return rtn;
}
//
// did we find this sequence number in the alert.log
//
int IsSeqNumMatch(char *locbuf, char *seqnum)
{
	char tmpstr[80];
	int rtn = FAILURE;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "IsSeqNumMatch, Enter");
	sprintf(tmpstr,": %s ", seqnum);
	if (strcasestr(locbuf, tmpstr) != NULL) {
		rtn = SUCCESS;
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "IsSeqNumMatch, Exit");
	return rtn;
}
//
// helper range check function
//
int IsSeqNumInRange(char *seqnum, ALERT_RANGE *alr)
{
	int rtn = FAILURE;
	long locseq;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "IsSeqNumInRange, Enter");
	locseq = atol(seqnum);
	if ((locseq <= alr->NewestSeqNum) && (locseq >= alr->OldestSeqNum)) {
		rtn = SUCCESS;
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "IsSeqNumInRange, Exit");
	return rtn;
}

//
// Verify a legitimate alert seq num request
//
int getAlert(char *seqnum, ALERT_STRUCT *alert, FILE **fp, int *fileisopen, int doclose)
{
	char locbuf[BUFF2K];				// 2k max len
	char locfilepath[BUFF80];
	int rtn = FAILURE;
	int found = 0;
	
	ALERT_RANGE alertrange;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "getAlert, Enter");
	rtn = getAlertRange(&alertrange);
	if (rtn == SUCCESS) {
		if (IsSeqNumInRange(seqnum, &alertrange) == SUCCESS) {
			memset(locbuf,0, BUFF2K);

			//
			// primary file search
			//
			if (!(*fileisopen)) {
				sprintf(locfilepath,"%s", PRIMARYFILEPATH);
				*fp = fopen(locfilepath, "r");
				if (*fp != NULL) {
					(*fileisopen) = 1;
					// seek to start of file
					fseek(*fp, 0, SEEK_SET);
				}
			}

			//
			// now loop until we find the correct record
			//
			while (!found) {
				if (fgets(locbuf, BUFF2K, *fp) != NULL) {
					if (IsSeqNumMatch(locbuf, seqnum) == SUCCESS) {
						rtn = ParseAndBuildAlert(locbuf, alert);
						found = 1;
						break;
					} else {
						continue;
					}
				} else if (strcasestr(locfilepath, SECONDARYFILEPATH) != NULL) {
					break;

				} else {
					// means we reached eof of primary
					fclose(*fp);
					(*fileisopen) = 0;
					sprintf(locfilepath,"%s", SECONDARYFILEPATH);
					break;
				}
			}

			if (strcasestr(locfilepath, SECONDARYFILEPATH) != NULL) {
				if (!fileisopen) {
					*fp = fopen(locfilepath, "r");
					if (*fp != NULL) {
						(*fileisopen) = 1;
						//
						// seek to end of file, 
						// seek to beginning of last line
						//
						fseek(*fp, 0L, SEEK_END);
						seek_line_start(*fp);
					}
				}

				while (!found) {
					//
					// check for seqnum in secondary file
					//
					if (fgets(locbuf, BUFF2K, *fp) != NULL) {
						if (IsSeqNumMatch(locbuf, seqnum) == SUCCESS) {
							rtn = ParseAndBuildAlert(locbuf, alert);
							found = 1;
							break;
						} else {
							//
							// seek begin of curr line
							// go up back a line
							//
							seek_line_start(*fp);
							if (seek_prev_line(*fp) != SUCCESS) {
								break;
							}
						}
					}
				}
			}

			if ((doclose) && (*fileisopen)) {
				fclose(*fp);
				(*fileisopen) = 0;
			}

		} else {
			rtn = FAILURE;
		}

	} else {
		rtn = FAILURE;
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "getAlert, Exit");

	return rtn;
}

//
// Formats the part of the mail message insert tabs and newline chars accordingly
//
void formatmessage(char *stringtosearch, char *stringtortn)
{
	char seps[]   = " ";
	char *token;
	char *saveptr;
	int  wordcount = 0;

	token = strtok_r(stringtosearch, seps, &saveptr);
	if (token != NULL) {
		sprintf(stringtortn,"%s", token);
		wordcount++;
		while ((token = strtok_r(NULL,seps, &saveptr)) != NULL) {
			//
			// 25 words per line
			//
			if (wordcount < WORDSPERLINE) {
				//
				// each word seaprated by spaces
				//
				strcat(stringtortn, " ");
				strcat(stringtortn, token);
				wordcount++;
			} else {
				strcat(stringtortn, "\n\t");
				wordcount = 0;
			}
		}
	}
	//
	// prior to rtn append
	//
	strcat(stringtortn,"\n");
}
//
// Format up the call home reuest and post via wget http cmd
// Input: is the alert so we format and send 
//
void IssueCallHome(ALERT_STRUCT *alert)
{
	char	cmd[BUFF2K];
	char	descr[BUFF1K];
	char	recc[BUFF1K];
	char	subjecttext[BUFF64];
	
	formatmessage(alert->description, descr);
	formatmessage(alert->recommendation, recc);
	
	sprintf(subjecttext,"Fac: %s, Sev: %s", alert->facility, alert->severity);

	sprintf(cmd, 
		"wget -q \"http://localhost:%d/callhome/mail?subject=%s&message=\n\n   \n\nSrc Node: %s \n\nDescription: %s\n\nRecommendation: %s\n\n\" ",
		Gsv_CALLHOME_PORTNUM, subjecttext, Gsv_hostname, descr, recc);

	system(cmd);
				
}

///////////////////////////////////////////////////////////////////////////////
///
//  processAlertMsg
///
/// Process uevents or RC events  from the system.
///
/// \params  buf     - facility (UDEV, SHAS, Seamonlx, etc.)
///          sev	 - severity (warn, error, etc)
///			desc	- msg description
///         alertid - ID of this type of alert, also serves as mapping to recommendation
///
///////////////////////////////////////////////////////////////////////////////

void processAlertMsg(char *fac, char *sev, char *component, char *desc, int alertid)
{
	int rtn = FAILURE;
	char msg[BUFF256];
	char reccString[BUFF1K];
	bool				retVal;
	unsigned long reccid;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "processAlertMsg, Enter");
	//
	// check for throttling
	//
	//
	// set the alertString, and signal to write alert
	//
	if (pthread_mutex_lock(&AlertMutex) == 0) {

		ALERT_STRUCT alert;
		//
		// need to get alert sequence num from last alert written
		//
		rtn = tailAlertLog(&alert, PRIMARYFILEPATH);
		
		if (rtn == SUCCESS) {
			alert.seqnum++;
		} else {
			alert.seqnum = 1;
		}
		
		alert.alertid = alertid;
		sprintf(alert.facility,"%s", fac);
		sprintf(alert.severity,"%s", sev);
		sprintf(alert.componentname,"%s", component);
		sprintf(alert.description,"%s", desc);
		
		//
		// need to prepare reccid to be used by StringResourceLookup()
		//
		reccid = FACILITY_SEMONLX_RECC + (unsigned long) alertid;
		retVal = StringResourceLookup(reccid, reccString);

		if (retVal != true) {
			sprintf(reccString, "%s", "Nothing to recommend");
		}

		sprintf(alert.recommendation, "%s", reccString);
		//
		// terminate description with "+++"
		//
		sprintf(alertString, "%ld %d %s %s %s %s +++ %s", 
			alert.seqnum, alert.alertid , fac, sev, component, desc, reccString);
		
		//
		// send the call home message for errors only
		//
		if (strcasestr(alert.severity,"ERR") != NULL) {
			IssueCallHome(&alert);
		}

		//
		// Unicast the alert to all waiting accepted socket cons
		//
		rtn = buildUnicastxmlString(&alert);

		ReadyToWriteLog = 1;
		pthread_cond_signal(&AlertCond);
		pthread_mutex_unlock(&AlertMutex);
	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "Failed to write the Alert Msg to the alert.log");
		//
		// krh could be issue maybe just trace
		//
		die(msg, 0, Gsv_AI_FAIL_TO_LOG);
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "processAlertMsg, Exit");
}

// 
// This routine uses several globals to accomplish
// writing to the Alert Log.
// AlertCond, AlertMutex, and the alertString
// utilizing syslog() it writes to LOG_LOCAL6 specific to seamonlx alerts only
//
int writeAlert()
{
	int		rtn = SUCCESS;
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "writeAlert():Enter");	

	// 
	// use ReadyToWriteLog to handle spurious wakeups
	//
	pthread_mutex_lock(&AlertMutex);

	while (!ReadyToWriteLog) {
		pthread_cond_wait(&AlertCond, &AlertMutex);
	}

	traceClass->LogTrace(ZQ::common::Log::L_INFO, "writeAlert():ReadyToWriteLog is TRUE");	
	
	if (!seamonlx_shutdown) {		
		//
		// necessary because close log will act on LOG_LOCAL6 here
		//
		openlog(" ",(LOG_CONS | LOG_NDELAY), LOG_LOCAL6);
		syslog(LOG_ERR,"%s", alertString);
		closelog();
		//
		// update so BuildComponentList Thread will catch next one
		//
		AlertLogParseComplete = FAILURE;
	}
	// 
	// reset for pthread_cond_wait for spurious wakeups
	//
	ReadyToWriteLog = 0;
	pthread_mutex_unlock(&AlertMutex);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "writeAlert():Exit with value %d", rtn);	

	return rtn;
}

//
// helper to cause cond signal so we can exit abyss 
// 
void shutdownwriteAlert()
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "shutdownwriteAlert, Enter");
	ReadyToWriteLog = 1;
	pthread_cond_signal(&AlertCond);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "shutdownwriteAlert, Exit");
}
