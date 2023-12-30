///////////////////////////////////////////////////////////////////////////////
//
// SEAMONLX.cpp 
//
//  is a service that monitors the system, provides client apps with on demand
//  state data and additionally broadcast timely alerts.
//
///////////////////////////////////////////////////////////////////////////////
// yao wget -q "http://localhost:3000/callhome/mail?subject='test one'&message='this is a test'"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>
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
#include <sys/time.h>
#include <sys/resource.h>

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

#include <sys/types.h>
#include <dirent.h>

#include <errno.h>

#include "common.h"
#include "seamonlx.h"
#include "seamonlx_init.h"

using namespace std;
using namespace seamonlx;


///////////////////////////////////////////////////////////////////////////////
///
//  die
///
/// \param  s   - Message to enter into log.
///		killit  - is used to set gloab value seamonlx_shutdown
///     alertid - indicates what alert it failed on
///
///////////////////////////////////////////////////////////////////////////////

void die(char *s, int killit, int alertid)
{
	//
	// CRITICAL ERROR, BASICALLY FAILS INITIALIZATION
	//
	if (killit == 1) {
		openlog("seamonlx",(LOG_CONS | LOG_PERROR | LOG_PID), LOG_DAEMON);
		syslog(LOG_ERR,"%s\n",s);
		closelog();

	} else {
		//
		// write other alert to alert.log
		//
		processAlertMsg("seamonlx", "ERR", CN_SeaMon_LX, s, alertid);
	}

	seamonlx_shutdown = killit;
}


///////////////////////////////////////////////////////////////////////////////
///
//  main:
//  Check cmd line opts
//  Demonize process if not debugging
//  do initialization
//  loop waiting to write alerts,
//  if we get a signal to stop via unrecoverable error or ctrl ^C exit 
//
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	FILE *fd;
	char cmd[80];
	char test[BUFF8];
	int rtn		= FAILURE;
	int NumInstance = 0;
	bool DoChecklspci = true;

	
	//
	// check cmd line options
	//
	checkCmdLine(argc, argv);
		
	/* Instantiate the Trace class */
	//if( TRACING ) {
		traceClass = new Trace( "seamonlx_trace.log" );
		assert( traceClass );
	//}
	//Trace  ini;
	struct rlimit rlim;
    
    rlim.rlim_cur = RLIM_INFINITY;
    rlim.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &rlim);
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "main, Enter");
	//
	// check to see if process already started
	//
	sprintf (cmd, "/usr/bin/pgrep seamonlx | wc -l");
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd);
	if ((fd = (FILE *)popen(cmd,"r")) != NULL) {
		if (fgets(test, BUFF8, fd) != NULL) {
			NumInstance = atoi(test);
		}
		pclose(fd);
	}

	//
	// NumInstance = 2 and argc = 1 means started as a service 
	// special case due to inherent fork from daemonize call within init.d
	//
	if ((NumInstance == 2) && (argc == 1)) {
		NumInstance = 1;
	}

	//
	// If cmdline then we don't allow a prior instance running
	//
	if (NumInstance < 2) {
		//
		// daemonize process
		//
		if (!DEBUG) {
			daemon(0,0);
			sleep (1);
			openlog("seamonlx",(LOG_CONS | LOG_PERROR | LOG_PID), LOG_DAEMON);
			syslog(LOG_INFO,"seamonlx daemon started\n");
			closelog();

		} else {
			openlog("seamonlx",(LOG_CONS | LOG_PERROR | LOG_PID), LOG_DAEMON);
			syslog(LOG_INFO,"seamonlx started but not as daemon\n");
			closelog();
		}

		//
		// Init the String Resource Library
		//
		if( StringResourceInitialize() != true )
		{
			openlog("seamonlx",(LOG_CONS | LOG_PERROR | LOG_PID), LOG_DAEMON);
			syslog(LOG_ERR,"String Resource Library Initialization failure!");
			closelog();		
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "StringResourceInitialize failed. Exiting." );	
			return FAILURE;
		}		
			
		// Dump them all out for tracing; 
		if( TRACING ) { 
			string errString;
			DumpStringResources(); 
			StringResourceLookup( FACILITY_SEAMONLX_COMMON,
											FAC_SEA_COM_ERROR_21,
											errString );
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "Called StringResourceLookup:1: errString=|%s|", errString.c_str() );
			errString.clear();
			StringResourceLookup( FAC_SEA_COM_ERROR_21_FULL,
											errString );
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "Called StringResourceLookup:2: errString=|%s|", errString.c_str() );
		}			
			
		//
		// get data and threads ready
		//
		rtn = InitSeamonlx();
		if (rtn == SUCCESS) {

			//
			// loop until we get a shutdown request or we encounter an unrecoverable error
			//
			while (!seamonlx_shutdown) {
				//
				// start threads for each service accordingly
				//
				startthreads();

				if (DoChecklspci) {
					ChecklspciDiffs();
					DoChecklspci = false;
				}
				
				//traceClass->LogTrace(ZQ::common::Log::L_INFO,  "main while ! shutdown = %d",  seamonlx_shutdown);					
				sleep(10);
			}
		}

		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, deleting traceClass and exiting main");	
		if( traceClass )
			delete traceClass;	
		//
		// write the sutdown message to our Alert Log
		//
		openlog("seamonlx",(LOG_CONS | LOG_PERROR | LOG_PID), LOG_DAEMON);
		syslog(LOG_INFO,"Shutdown complete");
		closelog();
		sleep(2);
	
		exit(EXIT_SUCCESS);

	} else {
		//
		// don't start another instance of seamonlx
		// write the sutdown message to our Alert Log
		//
		openlog("seamonlx",(LOG_CONS | LOG_PERROR | LOG_PID), LOG_DAEMON);
		syslog(LOG_INFO,"Will not start a second instance of seamonlx");
		closelog();
			
		exit(EXIT_FAILURE);
	}
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "main, Exit");

	// Dear gcc: shut up
	return 0;
}


