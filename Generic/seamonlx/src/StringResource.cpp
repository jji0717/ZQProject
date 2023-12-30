/*
 * 
 * StringResource.cpp
 *
 *
 * StringResource Library supporting functions
 *  
 * NOTE       //\\//\\//\\//\\//\\//\\//\\//\\//\\//
 *
 * The code in this file has a direct relation to the StringResource.ini file
 *
 * YOU HAVE BEEN WARNED !*
 *
 *  Revision History
 *  
 *  05-26-2010 mjc     Created
 *  06-09-2010 mjc     Reduced some tracing by adding a new defined macro / flag
 *  
 * 
 */

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include "StringResource.h"

using namespace std;

/**
* Defines and Variables
*/
#define	 	SEAMONLX_STRINGLIB_INIFILE			"/usr/local/seamonlx/config/StringResource.ini"	///< String Resource INI file path and name

bool     	srInitialized 	=	false;			///< String Resource Lib initialized flag
vector		<tStringResourceTable>		vMasterStResTable;					///< The master string table for all strings


 //
 // Uncomment this define for detailed TRACE_LOG tracing 
 //
//#define MORE_DEBUG 1			
//
#ifdef MORE_DEBUG
#define MORE_TRACE( s1, args... )  traceClass->LogTrace(ZQ::common::Log::L_INFO,  s1, ##args )
#else
#define MORE_TRACE( s1, args... )  ;
#endif
//

/**
 * 
 * Function:			loadStringLibrary()
 * Parameters:		string libName - Library name
 * Description:		
 * Returns:				bool true if success, false if failure
 * Caveats:
 * 
 */

bool loadStringLibrary( char *libName )
{
	bool 							retVal = true;
	ifstream 					libFileStream;	
	unsigned long			tmpFseVal;
	char							readLine[STRRES_MAX_STRING_LENGTH];
	char							errorString[STRRES_MAX_STRING_LENGTH];
	tStringResourceTable	entry;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "loadStringLibrary: Enter.    libName=%s", libName );
	
	libFileStream.open (libName, ifstream::in);
	if( libFileStream.fail() != true ) {
	
		while( libFileStream.good() ) {
	
			libFileStream.getline( readLine, STRRES_MAX_STRING_LENGTH-1 );
			//traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tI1:%03d: %s", ++lineNum, readLine );
			
			if( readLine[0] == ';' ) continue;  		// Get to the next line
			if( strlen(readLine) <= 10 ) continue;  		// Get to the next line
			
			// Grab the data
			if( sscanf( readLine, "%s %s %lx", 
										entry.componentName,
										entry.subComponentName,
										&tmpFseVal ) > 0 )	{
										
				char  *start = strstr( readLine, "\"" );
				if( start != NULL ) {
					strcpy( errorString, start+1 );		// Copy the string
					// Remove the last double quote from the string
					char  *end = strstr( errorString, "\"" );					
					if( end != NULL ) {
						*end = '\0';
					}
				}
				else errorString[0] = '\0';
				
				traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tAdding Entry:  %s, %s, %016lX, |%s|", 
										entry.componentName,
										entry.subComponentName,
										tmpFseVal,
										errorString );				
				entry.returnErrorString = errorString;
				entry.idValue.fseValue = tmpFseVal;
				
				vMasterStResTable.push_back( entry );
			}
		}  // while

		// Close the file
		libFileStream.close();
		
	}
	else {
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tI:Error opening string library" );
			// We want to put in some failsafe code here that loads a default string INI file
			retVal = false;
			return retVal;
	}
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "loadStringLibrary:  Exit,  returning %s", retVal==true?"success":"failure" );

	return retVal;
}

/**
 * 
 * Function:			StringResourceInitialize()
 * Parameters:		None
 * Description:		Initialize the string res library
 * Returns:				bool true if success, false if failure
 * Caveats:			Should be called once during initialization
 * 
 */

bool StringResourceInitialize()
{
	bool retVal = true;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "StringResourceInitialize: Enter" );
	
	if( srInitialized == true ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tALREADY INITIALIZED!" );
		retVal = false;
		return retVal;
	}
	
	// Open our string resource file and load up all the strings
	if( loadStringLibrary( SEAMONLX_STRINGLIB_INIFILE ) != true ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO,  "StringResourceInitialize: loadStringLibrary failed" );
		retVal = false;
	}
	
	// Check if we were successful
	if( retVal == true ) {
		srInitialized = true;		// Set the flag
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "StringResourceInitialize: Exit, returning %s", retVal==true?"success":"failure");

	return retVal;

}


/**
 * 
 * Function:			StringResourceLookup
 * Parameters:		unsigned long id, string &returnString 
 * Description:		Given ID, return the string associated with it
 * Returns:				bool true if success, false if not found
 * Caveats:			This function returns a string class reference
 * 
 */

bool StringResourceLookup(unsigned long id, string &returnString )
{
	bool 														retVal = false;
	vector <tStringResourceTable>::iterator		it;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "StringResourceLookup:2: Enter. ID=0x%016lX", id, id );
	
	if( srInitialized == false ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tLib Not initialized. Returning dummy string." );
		returnString = STRRES_DEFAULT_STRING;
		return retVal;
	}	
	
	// Loop thru all of the entries in the vector and compare
	for ( it = vMasterStResTable.begin(); it != vMasterStResTable.end(); it++ )  {
				if( it->idValue.fseValue == id ) {
					returnString = it->returnErrorString;		// Copy 
					retVal = true;
					break;
				}
				traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tSearching Entry: %s, %s, %016X (Fac=%X, Sev=%X, Err=%X)",\
										it->componentName, \
										it->subComponentName, \
										it->idValue.fseValue, \
										it->idValue.idStruct.facility, \
										it->idValue.idStruct.severity, \
										it->idValue.idStruct.error );	

			/*	MORE_TRACE( "\tSearching Entry: %s, %s, %016X (Fac=%X, Sev=%X, Err=%X)", 
										it->componentName,
										it->subComponentName,
										it->idValue.fseValue,
										it->idValue.idStruct.facility,
										it->idValue.idStruct.severity,
										it->idValue.idStruct.error );		*/
										

	}	

	if( retVal == false ) {
			returnString = STRRES_DEFAULT_STRING;
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "StringResourceLookup:2:  Exit,  returning %s, returnString=|%s|", retVal==true?"success":"failure", returnString.c_str() );
	return retVal;

}

/**
 * 
 * Function:			StringResourceLookup
 * Parameters:		unsigned long id
 *							unsigned long error
 *							string &returnString 
 * Description:		Given ID, return the string associated with it
 * Returns:				bool true if success, false if not found
 * Caveats:			This function returns a string class reference
 * 
 */

bool StringResourceLookup(unsigned long facility, unsigned long errId, string &returnString )
{
	bool 														retVal = false;
	vector <tStringResourceTable>::iterator		it;	
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "StringResourceLookup:3: Enter.     Facility=0x%lX   ID=0x%lX", facility, errId );

	if( srInitialized == false ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tLib Not initialized. Returning dummy string." );
		returnString = STRRES_DEFAULT_STRING;
		retVal = false;
		return retVal;
	}	

	// Get our values into one
	unsigned long id = MAKE_STRRES( facility, errId );
	traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tMAKE_STRRES macro:  id is %016lX", id ); 
	
	// Loop thru all of the entries in the vector and compare
	for ( it = vMasterStResTable.begin(); it != vMasterStResTable.end(); it++ )  {
				if( it->idValue.fseValue == id ) {
					returnString = it->returnErrorString;		// Copy 
					retVal = true;
				}	
				traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tSearching Entry: %s, %s, %016lX (Fac=%X, Sev=%X, Err=%X)", \
										it->componentName, \
										it->subComponentName, \
										it->idValue.fseValue, \
										it->idValue.idStruct.facility, \
										it->idValue.idStruct.severity, \
										it->idValue.idStruct.error );	

			/*	MORE_TRACE( "\tSearching Entry: %s, %s, %016lX (Fac=%X, Sev=%X, Err=%X)", 
										it->componentName,
										it->subComponentName,
										it->idValue.fseValue,
										it->idValue.idStruct.facility,
										it->idValue.idStruct.severity,
										it->idValue.idStruct.error );		*/			
	}
	
	if( retVal == false ) {
			returnString = STRRES_DEFAULT_STRING;
	}	
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "StringResourceLookup:3: Exit,   returning %s, returnString=|%s|", retVal==true?"success":"failure", returnString.c_str() );

	return retVal;

}

/**
 * 
 * Function:			StringResourceLookup
 * Parameters:		unsigned long id, char *returnString 
 * Description:		Given ID, return the character string associated with it
 * Returns:				bool true if success, false if not found
 * Caveats:			This function returns a char * buffer with the data
 *							The char buffer should be declared prior to calling this 
 *	 						function and the array size should be the following define:
 *							STRRES_MAX_STRING_LENGTH   (defined in StringResource.h)
 * 
 */

bool StringResourceLookup(unsigned long id, char *returnString )
{
	bool				retVal;
	string			theString;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "StringResourceLookup:4: Enter. ID=0x%016lX", id, id );
	
	retVal = StringResourceLookup( id, theString );
	strcpy( returnString, theString.c_str() );
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "StringResourceLookup:4:  Exit,  returning %s, returnString=|%s|", retVal==true?"success":"failure", returnString );
	return retVal;
}

/**
 * 
 * Function:			StringResourceLookup
 * Parameters:		unsigned long id
 *							unsigned long severity (currently ignored)
 *							unsigned long error
 * 							char *returnString 
 * Description:		Given ID, return the character string associated with it
 * Returns:				bool true if success, false if not found
 * Caveats:			This function returns a char * buffer with the data
 *							The char buffer should be declared prior to calling this 
 *	 						function and the array size should be the following define:
 *							STRRES_MAX_STRING_LENGTH   (defined in StringResource.h)
 * 
 */

bool StringResourceLookup(unsigned long facility, unsigned long errId, char *returnString )
{
	bool						retVal;
	string					theString;
	unsigned long		id;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "StringResourceLookup:5: Enter." );
	
	id = MAKE_STRRES( facility, errId );
	retVal = StringResourceLookup( id, theString );
	strcpy( returnString, theString.c_str() );
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "StringResourceLookup:5:  Exit,  returning %s, returnString=|%s|", retVal==true?"success":"failure", returnString );
	return retVal;
}

/**
 * 
 * Function:			DumpStringResources
 * Parameters:		None
 * Description:		Dumps into the TRACE_LOG all of the String Resources
 * Returns:				void
 * Caveats:			Used for debugging only.
 * 
 */

void DumpStringResources( void )
{
	vector <tStringResourceTable>::iterator		it;
	int														count = 0;			
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "DumpStringResources, Enter" );
	
	// Quick check for us being initialized
	if( srInitialized == false ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tLib Not initialized." );
	}	

	// Loop thru all of the entries in the vector
	for ( it = vMasterStResTable.begin(); it != vMasterStResTable.end(); it++ )  {
				traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tEntry %d:  %s, %s, %016lX (Fac=%X, Sev=%X, Err=%X), |%s|", 
										++count,
										it->componentName,
										it->subComponentName,
										it->idValue.fseValue,
										it->idValue.idStruct.facility,
										it->idValue.idStruct.severity,
										it->idValue.idStruct.error,
										it->returnErrorString.c_str() );		
	}	
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "DumpStringResources, Exit" );
}


/* End of file StringResource.cpp */
