/*
 * 
 * trace.cpp
 *
 *
 * Common trace utility file.
 *  
 *
 *
 *  Revision History
 *  
 *  04-15-2010 	mjc  	Created
 *  
 * 
 */

#include "common.h"    // This pulls in trace.h as well
#include "trace.h"
using namespace seamonlx;

/*
 * 
 * The LogTrace method to log traces to the logfile
 *
 */
void Trace::LogTrace(const char *msg, ...)
{
    va_list arguments;                     // A place to store the list of arguments
	char    buf[1024];
	//char    dt[128];
	//time_t  now = time(NULL);
	//struct  tm  *t = localtime(&now);

	// If we are not tracing, then do nothing
	if( !traceIsActive )
		return;
	
	/* Get the date and time */
	// With FILE and LINE removed
	//sprintf( dt, "%02d/%02d/%04d | %02d:%02d:%02d | ", t->tm_mon+1, t->tm_mday, t->tm_year+1900,
	//		 t->tm_hour, t->tm_min, t->tm_sec );
	va_start ( arguments, msg );    // Initializing arguments to store all values after msg
	vsnprintf( buf, sizeof(buf), msg, arguments );
	va_end ( arguments );           // Cleans up the list

	// Output the final buffer
	//*trace_file << dt << buf << endl;
	(*traceFileLog)(ZQ::common::Log::L_INFO,CLOGFMT(seamonlx," %s"),buf);
}
/*
*	The LogTrace method to log traces to the logfile
*	the function Trace::LogTrace overloaded.
*/
void Trace::LogTrace(int level, const char *msg, ...)
{
    va_list arguments;                     // A place to store the list of arguments
	char    buf[1024];
	//char    dt[128];
	//time_t  now = time(NULL);
	//struct  tm  *t = localtime(&now);

	// If we are not tracing, then do nothing
	if( !traceIsActive )
		return;
	
	/* Get the date and time */
	// With FILE and LINE removed
	//sprintf( dt, "%02d/%02d/%04d | %02d:%02d:%02d | ", t->tm_mon+1, t->tm_mday, t->tm_year+1900,
	//		 t->tm_hour, t->tm_min, t->tm_sec );
	va_start ( arguments, msg );    // Initializing arguments to store all values after msg
	vsnprintf( buf, sizeof(buf), msg, arguments );
	va_end ( arguments );           // Cleans up the list

	// Output the final buffer
	//*trace_file << dt << buf << endl;
	(*traceFileLog)(level,CLOGFMT(seamonlx," %s"),buf);
}
	
/* End of file trace.cpp */
