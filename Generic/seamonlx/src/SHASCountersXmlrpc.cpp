/* @File SHASCountersXmlrpc.cpp
 *
 *
 *  SHAS Counters object XMLRPC Support methods implementation.
 *  
 *
 *
 *  Revision History
 *  
 *  06-03-2010 mjc  Created
 *  06-10-2010  mjc  Added mutex to make this thread safe. 
 *  06-14-2010  mjc  Fixed memory leak: 	Do not call result.instantiate(), rather use result = finalResp;
 *  07-01-2010 mjc  Release mutex on throw case when error occurs
 * 
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include "SHASCountersXmlrpc.h"
#include "common.h"

using namespace std;
using namespace seamonlx;

/**
 * default constructor
 */
SHASCountersXmlrpc::SHASCountersXmlrpc()
{
	char	locstr[1024];
	string	formattedstring;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASCountersXmlrpc::SHASCountersXmlrpc, Enter");

	this->_signature = "A:s";

	sprintf(locstr, "For best viewing ensure your screen width is at least %d characters", (HELPLINEEND + 10));
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s", "SHASCounters List");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","     returns a List of fields values that can be enquired on");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s", "SHASCounters anyfieldvalue");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","     returns a List of fields values that can be enquired on");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s", "(e.g. SHASCounters diskio)");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","     returns a table of values");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s", "(e.g. SHASCounters hstats)");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","     returns a plethora of values");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s", "There are too many values to list individuallly");
	FormatHelpString(formattedstring, locstr);
	
	this->_help = formattedstring;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASCountersXmlrpc::SHASCountersXmlrpc, Exit");
	
}

/**
 * Function execute()
 * 
 */

void 
SHASCountersXmlrpc::execute(xmlrpc_c::paramList const& paramList,
				xmlrpc_c::value *   const  retvalP)
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASCountersXmlrpc::execute, Enter");

	paramList.verifyEnd(1);
	string param1 =  paramList.getString(0);
	
	if( param1.size() <= 0 )	{
		throw(xmlrpc_c::fault("Additional parameter not specified and is required",
							  xmlrpc_c::fault::CODE_UNSPECIFIED));	
	}
	
   /* parse the parameter */
	traceClass->LogTrace(ZQ::common::Log::L_INFO,  "execute: calling update with |%s|\n", param1.c_str() );

	pthread_mutex_lock( &ShasCountersMutex );			
    update( param1 );
	if( param1.compare( LIST_COMMAND_STRING ) == 0 ) {
	
		buildSHASSupportedCountersResp( *retvalP );		
		
	} else {
	
			buildSHASCountersResp( param1, *retvalP );		
	
	}
	pthread_mutex_unlock( &ShasCountersMutex );			

    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASCountersXmlrpc::execute, Exit");
}

/**
 * SHASCountersXmlrpc::buildSHASCountersResp()            
 *
 * Private method to get the info for the response
 *
 * 
 */

void
SHASCountersXmlrpc::buildSHASCountersResp( string shasType, xmlrpc_c::value &result )
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASCountersXmlrpc::buildSHASCountersResp, Enter. Type = %s", shasType.c_str() );

	bool															found = false;
	vector <tSupportedCounters>	 :: iterator		it;
	
	for ( it = supportedCounters.begin(); it != supportedCounters.end(); it++ ) {	
		// Compare the current iterator to the one passed in
		if( it->type.compare( shasType ) == 0 ) {

			traceClass->LogTrace(ZQ::common::Log::L_INFO, "\tFound match of %s", shasType.c_str());
			found = true;
			// Found a match
			// Create and instantiate the final response
			xmlrpc_c::value_string finalResp(it->data.c_str());
			result = finalResp;
		}
	}

	if( !found) {
		pthread_mutex_unlock( &ShasCountersMutex );	
		throw(xmlrpc_c::fault("Counter not found or not supported",  xmlrpc_c::fault::CODE_UNSPECIFIED));
	}
		
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASCountersXmlrpc::buildSHASCountersResp, Exit");
}

/**
 * SHASCountersXmlrpc::buildSHASSupportedCountersResp()            
 *
 * Private method to get the info for the response
 *
 * 
 */

void
SHASCountersXmlrpc::buildSHASSupportedCountersResp( xmlrpc_c::value &result )
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASCountersXmlrpc::buildSHASSupportedCountersResp, Enter");

	vector<xmlrpc_c::value>		 					respVec;
	vector <tSupportedCounters>	 :: iterator		it;
	
	for ( it = supportedCounters.begin(); it != supportedCounters.end(); it++ ) {	

		// Insert the structure tag value pair
		respVec.push_back( xmlrpc_c::value_string(it->type.c_str()) );
	}

	// Create and instantiate the final response
	xmlrpc_c::value_array finalResp(respVec);
	result = finalResp;	
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASCountersXmlrpc::buildSHASSupportedCountersResp, Exit");
}


/* End SHASCountersXmlrpc.cpp */
