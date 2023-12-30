/*
 * 
 * trace.h
 *
 *
 * Common trace utility file.
 *  
 *
 *
 *  Revision History
 *  
 *  04-09-2010 	jz     	Created (jie.zhang@schange.com)
 *  04-12-2010 	mjc  	Updated to be used in the common code base
 *  
 * 
 */
#ifndef SEAMONLX_TRACE_H
#define SEAMONLX_TRACE_H

#include <fstream>
#include <cstdarg>
#include <iostream>
#include <stdlib.h>
#include <time.h>

#include "ZQ_common_conf.h"
#include "Log.h"
#include "FileLog.h"

#define SEAMONLXLOG_RELOCATE  "/var/log"

// Defines
#define TRACE_LOG   traceClass->LogTrace



using namespace std;

namespace seamonlx {

	
	class Trace {
		
	public:
		
		Trace ( const char *trace ) : tracefile(0)
		{
			if (!traceIsActive) 
			{
				bzero(filename,sizeof(filename));
				sprintf(filename,"%s/%s",SEAMONLXLOG_RELOCATE,(char*)trace) ;
				tracefile = new string(filename);
				
				/*trace_file = new ofstream (tracefile->c_str(), ios::app);
				if (!trace_file->is_open())
				{
					// the file could not be opened
					 delete tracefile;
					 printf("Trace fatal error: log could not be opened!\n"); 
				}
				else//"seamonlx_trace.log"
				{
					traceIsActive = 1;
					*trace_file << "---- TRACE LOG STARTED ---- : " <<  trace << endl;
				}*/
				traceFileLog =new ZQ::common::FileLog((const char*)(tracefile->c_str()), ZQ::common::Log::L_DEBUG,5, 1024*1024*50);
				if(traceFileLog)
				{
					traceIsActive = 1;
					(*traceFileLog)(ZQ::common::Log::L_INFO,CLOGFMT(seamonlx, "---- TRACE LOG STARTED ---- : %s"),trace);	
				}
				else
				{
					delete tracefile;
					printf("Trace fatal error: log could not be opened!\n"); 
				}
			}
		}

			// Destructor
		~Trace () {
			if (traceIsActive){
				(*traceFileLog)(ZQ::common::Log::L_INFO,CLOGFMT(seamonlx, "---- TRACE LOG ENDED ---- "));	
					
				//*trace_file << "---- TRACE LOG ENDED ----  "  << endl;
				
				// Close the trace file
				//trace_file->close();
				
				if( tracefile) delete tracefile;

				//if( trace_file ) delete trace_file;
					
				if(traceFileLog) { traceFileLog->flush(); delete  traceFileLog;}
					
				traceIsActive = 0;
			}
		}

		//
		// Methods for logging.
		// Eventually we would want to use common system log capabilities here, like the logfile semantics of
		// facility, severity, and the like.
		//

		// log debugging information.
		void debug(const char *msg);

		// The LogTrace method to log traces to the logfile
		void LogTrace(const char *msg, ...);
		void LogTrace(int level, const char *msg, ...);
		// log errors
		void log_error( const char *msg );
		// log warnings
		void log_warning( const char *msg );

		// other methods ...

		static bool traceIsActive;
		
		
	private:
		char filename[256];
		ofstream *trace_file;
		ZQ::common::Log  *traceFileLog;
		string *tracefile; // which file to log the trace.
	};
}




#endif
