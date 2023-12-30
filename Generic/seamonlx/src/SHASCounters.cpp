/** @file SHASCounters.cpp
 *
 *
 *  SHASCounters class constructors and member functions  implementation.
 *  
 *
 *
 *  Revision History
 *  
 *  06-03-2010 mjc   Created
 *  
 * 
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <dirent.h>
#include "seamonlx.h"
#include "common.h"
#include "SHASCounters.h"

using namespace std;
using namespace seamonlx;

//
// List of suppoted counter files for shas
// A special case exists for the encl_ type strings because there can be many of those
// in the encl_*_reads and encl_*_writes format
//
const char *supportedCounterList[] = {
					"bdstats",
					"buckets",
					"disk_io",
					"host_io",
					"encl_",			// special case here with multiples possible
					"hstats",
					"hstats_other",
					"hstats_reads",
					"hstats_write",
					"rc_stats",
					"rem_stats",
					"trips",
					"EOL"			// This MUST be last in the list
};


/*************************************************
 *
 *       Class SHASCounters methods definition
 * 
 ************************************************/


/**
 * SHASCounters constructor
 */
SHASCounters::SHASCounters()
{
}

/**
 * SHASCounters Destructor
 */ 
SHASCounters::~SHASCounters()
{
}


/**
 * Function update()
 *
 * The function that updates the SHASCounters data members
 */
int
SHASCounters::update( string ShasType )
{
	int retval = SUCCESS;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASCounters::update, Enter");
	
	// Clear out what we had before
	supportedCounters.clear();
	
	/*
	 * Execute the SHAS Configuration script
	 */
	 if( updateShasCounters( ShasType ) != SUCCESS ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "SHASCounters::update, updateShasCounters %s failure.", ShasType.c_str() );	
			retval = FAILURE;
	}
	 
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASCounters::update, Exit");

	return retval;
}

 //
 // Uncomment this define for detailed TRACE_LOG tracing of this method
 //
//#define SCR_DEBUG 1			// SHAS Config Response detailed DEBUG 
//
#ifdef SCR_DEBUG
#define SCR_TRACE( s1, args... )  traceClass->LogTrace(ZQ::common::Log::L_INFO,  s1, ##args )
#else
#define SCR_TRACE( s1, args... )  ;
#endif
//

/**
 * Function updateShasCounters()
 *
 * The function that updates the SHASCounters data members
 */
int
SHASCounters::updateShasCounters( string ShasType )
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "updateSHASCounters::update, Enter");
	
	int 			retval = SUCCESS;
	char   		path[BUFF256];
	char         statisticsContents[BUFF4K];

	/**
	* Order of items:
	* 		1) Determine which host the shas counters are located on (i.e. /sys/class/scsi_host/host1)
	*			Perform a "service shas status" and the first line of that output has the host
	*
	*		2) Using that path, determine which of the following files exist
	*
	*				bdstats - provides SHAS bottom driver statistics including the state of each drive, the error counts and the 
	*									number of active outstanding requests per drive.
	*				buckets - provides internal latency bins in terms of MS for functions internal to SHAS
	*				disk_io - tracks the number and size of IO's submitted by SHAS to physical disks.
	* 				host_io - tracks the number and size of IO's submitted by filesystems to SHAS logical drives.
	*				encl_1_reads - tracks the read latencies to each endpoint device (disk) attached to enclosure 1 in the bcadm <ENCLOSURE LIST>.  
	*									There will one of these files for every enclosure found in the <ENCLOSURE LIST>.    
	*									Each column in the file represents the number of IOs that completed in that binned time.  For example the column " 0-24" 
	*									represents the number of IO's that completed in 0-24 ms.  The Ave and Max columns are not counts 
	*									they represent the average and max latencies for a particular channel (disk drive).
	*				encl_1_writes - same as encl_1_reads but tracks write latencies.
	*				hstats - tracks OSIC scheduling statistics and overall latencies from the time an IO was submitted by the SHAS driver through the Core.
	*				hstats_other - tracks latencies for non-data commands (i.e. management commands).
	*				hstats_reads - tracks READ command latencies to logical drives.
	*				hstats_write - tracks WRITE command latencies to logical drives.
	*				rc_stats - displays internal Core counters.
	*				rem_stats - displays internal mirroring statistics.
	*				trips - tracks Core resource blocking points.
	*
	*		3) For each file, open it and assign it's contents to the counter array type
	*
	**/
	
	/**
	 * The script name and params
	 */
	string command = "/sbin/service shas status";  	// The type name is appended to the string

	FILE *stream = popen(command.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", command.c_str());
	if ( stream == NULL ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "updateShasCounters: Failed to get stream handle from service shas status command.");
		return FAILURE;
	}
	
	if( fgets(path, sizeof(path), stream ) == NULL )	{
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "updateShasCounters: Failed to read SHAS SCSI Host from service shas status.");
		pclose(stream);
		return FAILURE;
	}

	pclose(stream); 		// Close the stream as we are done with it	

	// Remove '\r' from end of line
	if( strlen( path ) > 0 ) {
		path[ strlen(path) - 1 ] = '\0';
	}
	
	// Determine which statistic files exist now that we have the path
	SCR_TRACE("\tPath to SCSI Host is |%s|", path );

	//
	// Local variables for directory and file processing
	//
	DIR 								*dirp;
	struct dirent 				*dirEntry;
	int								i;
	tSupportedCounters		counterEntry;
	bool								found;
	char 							fullFName[ BUFF256 ];
	FILE 								*pFile;
	size_t 							bytesRead;	
	#ifdef SCR_DEBUG 
	tSupportedCounters		tempBack;
	#endif
	
	// Get the directory entries for all the shas counters
	if( (dirp = opendir(path)) )	 {
		while( (dirEntry = readdir(dirp)) )  {
		
			//SCR_TRACE("\tDName: %20s\tType: %X\n", dirEntry->d_name, dirEntry->d_type);

			i = 0;						// Reset the starting index
			found = false;			// and the found flag
			
			//
			// Check the name against the supported list of possible entries
			//
			while( (strcmp( supportedCounterList[i], "EOL" ) != 0)  && !found ) {
			
				// If type is DT_REG (Regular File)
				if( dirEntry->d_type == DT_REG ) {

					//SCR_TRACE("\tReg File %20s; compare to %s", dirEntry->d_name, supportedCounterList[i] );
				
					// Do we minimally match a substring of the full string
					if( strstr( dirEntry->d_name, supportedCounterList[i] ) != NULL ) {

						// We found a match
						found = true;
						SCR_TRACE("\tFOUND MATCH: Add %20s \tto list",  dirEntry->d_name );

						// Copy the name as it's type
						counterEntry.type = dirEntry->d_name;
						
						// We skip the reading of file data if it's a LIST command
						if( ShasType.compare( LIST_COMMAND_STRING ) != 0 ) {

							// We dont have a "List" (LIST_COMMAND_STRING) command, so we save off all the contents of the counters
							strcpy( fullFName, path );
							strcat( fullFName, "/" );
							strcat( fullFName, dirEntry->d_name );
							SCR_TRACE("\tREAD STATS FILE %s",  fullFName );

							// Open the file and grab it's contents
							pFile = fopen( fullFName,"r");
							if( pFile != NULL ) {
								if( (bytesRead = fread( statisticsContents, 1, BUFF4K, pFile )) > 0 ) {
									counterEntry.data.assign( statisticsContents, bytesRead );
									SCR_TRACE("\t\tBYTES READ = %d, data.size=%d",  (int)bytesRead, (int)counterEntry.data.size() );
									fclose( pFile );
								}
								else {
									traceClass->LogTrace(ZQ::common::Log::L_INFO, "\t\tERROR reading stats file %s, bytes read %d",  fullFName, (int)bytesRead );
								}
							}
							else	{
								traceClass->LogTrace(ZQ::common::Log::L_INFO, "\t\tERROR opening file %s",  fullFName );
							}
						}
						
						// Push the counter onto our vector.
						supportedCounters.push_back( counterEntry );
						
						#ifdef SCR_DEBUG 
						tempBack = supportedCounters.back();
						#endif
						SCR_TRACE("\tADD VERIFY  Type: %20s  Buf Len : %d\n",  tempBack.type.c_str(), (int)tempBack.data.size() );

						// Clear the data for the next entry now that we are done with it here
						counterEntry.type.clear();
						counterEntry.data.clear();
					}
				}
			
				i++;			// Inc to next array element
			}
		} // End while
		
		closedir(dirp);
		
	}  // End if
	
	// Trace output of our counters list
	//if( TRACING ) { printSupportedCounters(); }
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "updateSHASCounters::update, Exit. retval=%d", retval );

	return( retval );
}


/* End of SHASCounters.cpp */
