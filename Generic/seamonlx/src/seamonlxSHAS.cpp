///////////////////////////////////////////////////////////////////////////////
//
// seamonlxSHAS.cpp
//
//  monitors the system, for SHAS events  (drive puls / adds) etc.
//  and additionally UNICAST alerts.
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
#include "seamonlxSHAS.h"


extern THREAD_INFO_ELEM ThreadInfo[MAX_START_THREADS];
//
// routine takes a wide char string converts to char string
//
RC_UINT32 wideToSingle(RC_CHAR* s, const RC_WCHAR* w) 
{
    RC_UINT32 i;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "wideToSingle, Enter");
    for (i = 0;; i++) {
        s[i] = (char) (w[i]&0xff);
        if (!s[i])
            break;
    }

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "wideToSingle, Exit");
    return (i);
}
//
// Lifted from RC sample code
//
static int parseSHASmsg( RC_CHAR *buffer,
        RC_INT32 *error_number,
        RC_INT32 *error_priority,
        RC_CHAR *error_string,
        RC_UINT32 error_string_len,
        RC_UINT32 *params) 
{
    RC_CHAR *p;
    RC_UINT32 sequence = 0;
    RC_UINT32 flags = 0;
	int i = 0;

	char tmpstr[80];

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "parseSHASmsg, Enter");
    p = buffer;

    // Parse the sequence number
    while (*p && *p != ':')  {
        if (*p < '0' || *p > '9') {
			die("parseSHASmsg: can't parse sequence number invalid digit\n", 0, Gsv_AI_SHAS_PARSE_MSG_ERROR);
            return -1;
        }
        sequence = sequence * 10 + *p++ -'0';
    }

    if (*p++ != ':') {
		die("parseSHASmsg: can't parse sequence number no trailing \":\"\n", 0, Gsv_AI_SHAS_PARSE_MSG_ERROR);
        return -1;
    }

    // Parse the flags field
    while (*p && *p != '[') { 
		p++; 
	}

    if (*p++ != '[') {
		die("parseSHASmsg: can't parse flags no initial \"[\"\n", 0, Gsv_AI_SHAS_PARSE_MSG_ERROR);
        return -1;
    }

    while (*p && *p != ']') {
        if (*p < '0' || *p > '9') {
			die("parseSHASmsg: can't parse flags invalid digit\n", 0, Gsv_AI_SHAS_PARSE_MSG_ERROR);
            return -1;
        }
        flags = flags * 10 + *p++ -'0';
    }

    if (*p++ != ']') {
		die("parseSHASmsg: can't parse flags no trailing \"]\"\n", 0, Gsv_AI_SHAS_PARSE_MSG_ERROR);
        return -1;
    }

    // Parse the error number
    while (*p && *p != '(') {
		p++;
	}

    if (*p++ != '(') {
		die("parseSHASmsg: can't parse error number no initial \"(\"\n", 0, Gsv_AI_SHAS_PARSE_MSG_ERROR);
        return -1;
    }

    *error_number = 0;

    while (*p && *p != ')') {
        if (*p < '0' || *p > '9') {
			die("parseSHASmsg: can't parse error number invalid digit\n", 0, Gsv_AI_SHAS_PARSE_MSG_ERROR);
            return -1;
        }
        *error_number = (*error_number * 10) + *p - '0';
        p++;
    }

    if (*p++ != ')') {
		die("parseSHASmsg: can't parse error number no trailing \")\"\n", 0, Gsv_AI_SHAS_PARSE_MSG_ERROR);
        return -1;
    }

    // Parse the severity
    while (*p && *p == ' ') {
		p++;
	}

    *error_priority = -1;

    if (*p == 'V' && *(p + 1) == 'L')
        *error_priority = 0;
    else if (*p == 'I' && *(p + 1) == 'N')
        *error_priority = 1;
    else if (*p == 'W' && *(p + 1) == 'A')
        *error_priority = 2;
    else if (*p == 'C' && *(p + 1) == 'R')
        *error_priority = 3;
    else if (*p == 'F' && *(p + 1) == 'O')
        *error_priority = 4;
    else if (*p == 'L' && *(p + 1) == 'G')
        *error_priority = 5;
    else 
	{
		sprintf(tmpstr, "parseSHASmsg: can't parse severity %c%c\n", *p, *(p + 1));
        die(tmpstr, 0, Gsv_AI_SHAS_PARSE_MSG_ERROR);
        return -1;
    }
    p += 2;

    // The text message itself.
    while (*p && *p == '-') {
		p++;
	}

    p += 3;
    
    while (*p && *p != '.') {
        *error_string++ = *p++;
        if (i++ == (int) error_string_len)
            break;
    }

    if (*p++ != '.') {
		die("parseSHASmsg: can't parse event missing \".\"\"\n", 0, Gsv_AI_SHAS_PARSE_MSG_ERROR);
        return -1;
    }
    *error_string = '\0';

    // parse args
    for (i = 0; i < 8; i++) {
        while (*p && *p != '(') p++;
        if (*p++ != '(') {
			die("parseSHASmsg: can't parse argument missing initial \"(\"\n", 0, Gsv_AI_SHAS_PARSE_MSG_ERROR);
            return -1;
        }

        params[i] = 0;
        while ((*p >= '0') && (*p <= '9')) {
            params[i] = (params[i] * 10)+ *p++ -'0';
        }

        if (*p++ != ')') {
			die("parseSHASmsg: can't parse argument missing trailing \")\"\n", 0, Gsv_AI_SHAS_PARSE_MSG_ERROR);
            return -1;
        }
    }

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "parseSHASmsg, Exit");
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//  SetCoreInfo
//
//  Lifted from RC sample; Sets the core info structure based on platform
//
//
///////////////////////////////////////////////////////////////////////////////

static void SetCoreInfo(struct RC_Core_Info *core_info)
{
    struct RC_Core_Info CoreInfoBuffer[8];
    RC_UINT32 hResult;
    RC_UINT32 Count;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SetCoreInfo, Enter");
    // assume there are NO instances
    Count = 0;

    try{
        hResult = RC_APIGetCoreInstances( CoreInfoBuffer, sizeof(CoreInfoBuffer), &Count);
    }

    catch( ... ){
        Count = 0;
        hResult = 0;
    }

    if( Count > 0 ){
        memcpy( core_info, CoreInfoBuffer, sizeof(struct RC_Core_Info) );
    }

    if( hResult != 1 ){
		die( "SetCoreInfo: Unable to access core, terminating\n", 0, Gsv_AI_SHAS_FAIL_TO_GET_CORE_INFO);
    }

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SetCoreInfo, Exit");
}

// 
// MonitorUDEV: Thread which monitors SHAS events
// if (rcha & bccfg modules running) Checks using bccfg API
//
void *MonitorSHAS(void *pParam)
{
	int result = -1;

	struct RC_Core_Info core_info;
 
    RC_CHAR             Message[BUFF1K];
    RC_WCHAR            UserMessage[BUFF1K];

	RC_UINT32			status;
    
    RC_INT32			error_priority;
    RC_INT32			error_number;
    RC_UINT32			params[8];
    RC_CHAR				error_string[BUFF256];
	
	RC_INT32			count = 0;
	RC_UINT32           StartSequence = 0;
	RC_CHAR             err_string[BUFF16];

	long threadnum;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "MonitorSHAS: thread Enter" );
	
	threadnum = (long)((long *)pParam);	
	ThreadInfo[threadnum].threadstate = THREAD_STARTED;
	//
    // Setup a core info structure
    //
    SetCoreInfo(&core_info);

	while (!seamonlx_shutdown) {
		//
		// check for presence of rcha, bccfg
		//		
		if ((rchaIsLoaded) && (bccfgIsLoaded)) {
			//
			// calls, get the buffer
			//
			
			memset(error_string, 0, sizeof(error_string));

			status = RC_ApiGetUserMessage(&core_info, (RC_WCHAR *) UserMessage, sizeof (UserMessage) / sizeof (RC_WCHAR), &StartSequence, 1);
				
			if (status == RC_STS_VALID_USER_MESSAGE) {
				count = wideToSingle( (RC_CHAR *)Message, (const RC_WCHAR *)UserMessage );

				result = parseSHASmsg((RC_CHAR *) Message, 
					&error_number, &error_priority, (RC_CHAR *) & error_string, 
					(RC_UINT32)sizeof (error_string), (RC_UINT32 *) & params);

				sprintf(err_string,"%d", error_priority); 

				traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tMonitorSHAS: processAlert string=|%s|", err_string );
				processAlertMsg("SHAS", err_string, CN_SHAS_State, error_string, (int)error_number);
			} 
		} 
		/**
		* Don't think we really want to break here. If we do break, the thread just keeps restarting.
		* Commenting out for now. MJC 7/1/10
		*
		 else {
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tMonitorSHAS: rchaIsLoaded=%d & bccfgIsLoaded=%d; BREAKING OUT", rchaIsLoaded, bccfgIsLoaded );
			break;
		}
		*/
		
		sleep(1);
	}

	ThreadInfo[threadnum].threadstate = THREAD_STOPPED;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "MonitorSHAS: thread Exit" );
	
	pthread_exit(NULL);
}
