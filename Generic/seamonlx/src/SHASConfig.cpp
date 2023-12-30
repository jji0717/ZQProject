/** @file SHASConfig.cpp
 *
 *
 *  SHASConfigObj class constructors and member functions
 *  implementation.
 *  
 *  
 *
 *
 *  Revision History
 *  
 *  05-07-2010 mjc   Created
 *  05-19-2010 mjc   Fixed Jira SMIS-79 SHAS Config does not work when seamonlx is deamonized
 *							  this was due to the missing full path info for the perl script
 *  06-08-2010 mjc   Added Enclosure:Bay logic for LE's. Also some housekeeping clean up.
 *  
 * 
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include "common.h"
#include "SHASConfig.h"

using namespace std;
using namespace seamonlx;


/*************************************************
 *
 *       Class SHASConfig methods definition
 * 
 ************************************************/


/**
 * SHASConfig constructor
 */
SHASConfig::SHASConfig()
{
}

/**
 * SHASConfig Destructor
 */ 
SHASConfig::~SHASConfig()
{
}


/**
 * Function update()
 *
 * The function that updates the SHASConfig data members
 */
int
SHASConfig::update( string ShasType )
{
	int retval = SUCCESS;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASConfig::update, Enter");
	
	/*
	 * Execute the SHAS Configuration script
	 */
	 if( updateShas( ShasType ) != SUCCESS ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "SHASConfig::update, updateShas %s failure.", ShasType.c_str() );	
			retval = FAILURE;
	}
	 
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASConfig::update, Exit");

	return retval;
}

/**
 * Function updateShas()
 *
 * The function that updates the SHASConfig data members
 */
int
SHASConfig::updateShas( string ShasType )
{
	/**
	 * The script name and params
	 */
	string command = "/usr/local/seamonlx/bin/getShasConfig.pl ";  	// The type name is appended to the string
	command.append( ShasType );
	
	FILE *stream = popen(command.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", command.c_str());
	if ( stream == NULL ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "updateShas(): Failed to get SHAS %s information from script.", ShasType.c_str() );
		return FAILURE;
	}
	
	// Clear the Vector Dev list
	clearVecDev();
	
	// Need to populate our local data structure / vector /array of maps with the data from this pipe
	char						text[255];
	string					lineText;
	vector<string>		lineTagVal;
	TagVal					tempTagVal;

	// Loop thu all lines of the script pipe
	while ( fgets(text, sizeof(text), stream) ) {
	
		//printf( "pipe fgets %d:  text=|%s|\n", count++, text );
		
		lineText = text;
		
		if ( !lineText.empty() ) {

		    // Split the string based on the equals sign
			// Note - need to handle the case where a string contains an equals sign. May not be an issue at all.
			stringSplit( lineText, "=", lineTagVal );    // stringSplit function lives in common.cpp
			
			// Assign the two values
			tempTagVal.tag = lineTagVal.front();
			tempTagVal.value = lineTagVal.back();

			// Remove the two entries from the vector so they dont accumulate in this loop
			lineTagVal.clear();
			
			//printf("tempTagVal  tag=|%s|   val=|%s|\n", tempTagVal.tag.c_str(), tempTagVal.value.c_str() );
			
            vecDev.push_back( tempTagVal );
		
		} // End if !dn.empty

	}	// End while

	pclose(stream);

	return( SUCCESS );
}



 //
 // Uncomment this define for detailed TRACE_LOG tracing of the following methods
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
 * setPDEnclosureAndBay()
 *
 * The function that sets the PD's Enclosure and Bay for future LD / LE requests
 * This should only be called once when this class constructor is invoked.
 */
void
SHASConfig::setPDEnclosureAndBay( void )
{
	vector<TagVal>::iterator 		tvit;								// TagVal iterator
	bool										foundE, foundB, foundH = false;	// Flags for Enc, Bay, Handle found
	EnclosureBay							tempEncBay;					// Temp EncBay entry to push

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASConfig::setPDEnclosureAndBay, Enter. Calling updateShas('PD').");	

	// First update the object to get the PD data. This must be done before we do anything else.
	updateShas( "PD" );
	
    // Go thru all entries in the vector and create a response to intantiate
	for ( tvit = vecDev.begin(); tvit != vecDev.end() && (!foundH || !foundE || !foundB); tvit++ ) {	

		SCR_TRACE("\tComparing |%s|  to  |%s|", tvit->tag.c_str(), pdHeaderString.c_str() );	
		
		if( tvit->tag.compare( pdHeaderString ) == 0 ) {		// Match PD header ?

			// We continue to iterate thru the same list, now under the "Disk Number" heading
			for ( tvit++; tvit != vecDev.end() && (!foundH || !foundE || !foundB); tvit++ ) {	
			
				SCR_TRACE("\t\tInner tvit Comparing |%s|  to  pdHandle |%s|", tvit->tag.c_str(), handleHeaderString.c_str() );	

				if( tvit->tag.compare( handleHeaderString ) == 0 ) {		// Match Handle within PD
					tempEncBay.handle = tvit->value;								// Copy the handle value
					SCR_TRACE("\t\tFound HANDLE  %s", tvit->value.c_str() );	
					foundH = true;
				}
		
				if( tvit->tag.compare( enclNumHeaderString ) == 0 ) {		// Match Encl header ?
					tempEncBay.enclosure = tvit->value;								// Copy the enclosure
					SCR_TRACE("\t\tFound ENCL  %s", tvit->value.c_str() );	
					foundE = true;
				}
				
				if( tvit->tag.compare( bayNumHeaderString ) == 0 ) {		// Match Bay header ?
					SCR_TRACE("\t\tFound BAY  %s", tvit->value.c_str() );	
					tempEncBay.bay = tvit->value;										// Copy the bay
					foundB = true; 
				}
				
				// Check for error case where data fields are missing. This is bad but we must recover.
				if( tvit->tag.compare( pdHeaderString ) == 0 ) {	
					// If we are onto our next PD header, then something is wrong here
					// if we have not found all fields we are looking for. This is true for PD's of type Disk.
					// We need to reset the flags for the next iteration
					traceClass->LogTrace(ZQ::common::Log::L_INFO, "\t*** DATA MISSING ***, Flags Are: Handle=%s Encl=%s Bay=%s",
											(foundH?"true":"false"),(foundE?"true":"false"),(foundB?"true":"false") );
					foundE = foundB = foundH = false; 
					
					tvit--;			// Back up one so we dont blow past it!
					break;			// Break out without incrementing the iterator
				}
				
			} // End for 2nd tvit iterator

			// If we found both, we add it onto the vector
			if( foundE && foundB && foundE ) {
				traceClass->LogTrace(ZQ::common::Log::L_INFO, "\tAdding Handle=|%s|     Enc=|%s|    Bay=|%s|", 
									tempEncBay.handle.c_str(), tempEncBay.enclosure.c_str(), 
									tempEncBay.bay.c_str()	 );	
									
				vecEnclBay.push_back( tempEncBay );			// Push it onto the list
				tempEncBay.handle.clear();							// Clear it for the next one through
				tempEncBay.enclosure.clear();
				tempEncBay.bay.clear();
				foundE = foundB = foundH = false; 								// Reset flags

				// For debugging purposes
				#if defined SCR_DEBUG
				EnclosureBay 	tempBack;					// Temp EncBay entry to push
				tempBack = vecEnclBay.back();
				SCR_TRACE("\tAdd Verify  Handle=|%s|  Enclosure=|%s|  Bay=|%s|\n",  
									tempBack.handle.c_str(),
									tempBack.enclosure.c_str(), 
									tempBack.bay.c_str() );
				#endif

			}  // End if ALL found
				
		}  // End if Disk Number header match
		
	}  // End for tvit	

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASConfig::setPDEnclosureAndBay, Exit");	
}

/**
 * getPDEnclosureAndBay()
 *
 * The function that returns the PD's Enclosure and Bay value give the LE's handle value
 * The LD / LE handle field maps to the PD's Disk Number field.
 */
bool
SHASConfig::getPDEnclosureAndBay( string handle, string &Enc, string &Bay )
{
	vector<EnclosureBay>::iterator 		tvit;					// EnclosureBay iterator
	bool												found = false;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASConfig::getPDEnclosureAndBay, Enter. Handle=|%s|", handle.c_str() );	
	
    // Go thru all entries in the vector and create a response to intantiate
	for ( tvit = vecEnclBay.begin(); tvit != vecEnclBay.end() && !found; tvit++ ) {	

		SCR_TRACE("\tvecEnclBay search: compare it |%s| to desired handle |%s|", tvit->handle.c_str(), handle.c_str() );	
		
		if( tvit->handle.compare( handle ) == 0 ) {		// Match PD handle to the passed in handle
		
			SCR_TRACE("\tvecEnclBay MATCH FOUND! : handle=%s  Enc=%s Bay=%s", 
					tvit->handle.c_str(), 
					tvit->enclosure.c_str(),
					tvit->bay.c_str() );	

			// Assign the enclosure and bay values
			Enc = tvit->enclosure;
			Bay = tvit->bay;
			
			found = true;		// Set found flag
		}

	}  // End for tvit	

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASConfig::getPDEnclosureAndBay, Exit with %s. Encl=|%s|  Bay=|%s|", 
						(found?"success":"failure"), Enc.c_str(), Bay.c_str() );		
	
	return( found );
}

/* End of SHASConfig.cpp */
