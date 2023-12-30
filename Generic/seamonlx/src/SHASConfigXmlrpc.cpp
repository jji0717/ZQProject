/* @File SHASConfigXmlrpc.cpp
 *
 *
 *  SHAS Configuration Xml Object  class constructors and member functions
 *  implementation.
 *  
 *  Revision History
 *  
 *  05-07-2010  mjc  Created
 *  05-18-2010  mjc  Fixed LE's so they are an array within the LD's
 *  05-20-2010  mjc  Added CL (Cluster) object support. Also did some cleanup houskeeping.
 *  05-26-2010  mjc  Added use of String Resource library for tag strings
 *  06-09-2010  mjc  Added Enclosure / Bay capability. Some clean up.
 *  06-10-2010  mjc  Added mutex to make this thread safe.
 *  06-14-2010  mjc  Fixed memory leak: 	Do not call result.instantiate(), rather use result = finalResp;
 *  08-06-2010  mjc  Added PD <enclosure id> capability
 *  08-12-2010  mjc  Fixed bug with PD <enclosure id> XML result. Seperated out PD handling from LD/LE.
 *  
 * 
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include "SHASConfigXmlrpc.h"
#include "common.h"

using namespace std;
using namespace seamonlx;


/**
 * default constructor
 */
SHASConfigXmlrpc::SHASConfigXmlrpc()
{
	char	locstr[1024];
	string	formattedstring;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASConfigXmlrpc::SHASConfigXmlrpc, Enter");

	this->_signature = "A:s";

	sprintf(locstr, "For best viewing ensure your screen width is at least %d characters", (HELPLINEEND + 10));
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", "SHASConfig [CD, PD, LD, CL]");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","1) SHASConfig CD");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","     returns array of Enclosure data");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","2) SHASConfig PD");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","     returns an array of Physical disk data");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","3) SHASConfig LD");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","     returns an array of Logical disk data");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","4) SHASConfig CL");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","     returns Cluster data");
	FormatHelpString(formattedstring, locstr);
	

	this->_help = formattedstring;

	// Using the String Resource Library API
	// Load up all the strings used for header comparisons. Note that these strings are
	// defined in the ShasConfig class (not in ShasConfigXmlrpc)
	//
	if( StringResourceLookup( FACILITY_SHAS_CONFIG,
											FAC_SHAS_CONFIG_CD_HDR_STRING,
											cdHeaderString ) != true ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "SHASConfigXmlrpc::constructor, CD hdr String failed. Using default");
		cdHeaderString = "Enclosure Number";
	}

	if( StringResourceLookup( FACILITY_SHAS_CONFIG,
											FAC_SHAS_CONFIG_PD_HDR_STRING,
											pdHeaderString ) != true ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "SHASConfigXmlrpc::constructor, PD hdr String failed. Using default");
		pdHeaderString = "Disk Number";
	}
	
	if( StringResourceLookup( FACILITY_SHAS_CONFIG,
											FAC_SHAS_CONFIG_LD_HDR_STRING,
											ldHeaderString ) != true ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "SHASConfigXmlrpc::constructor, LD hdr String failed. Using default");
		ldHeaderString = "Logical Drive Number";
	}	

	if( StringResourceLookup( FACILITY_SHAS_CONFIG,
											FAC_SHAS_CONFIG_LE_HDR_STRING,
											leHeaderString ) != true ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "SHASConfigXmlrpc::constructor, LE hdr String failed. Using default");
		leHeaderString = "Logical Element Number";
	}	
	
	if( StringResourceLookup( FACILITY_SHAS_CONFIG,
											FAC_SHAS_CONFIG_HANDLE_HDR_STRING,
											handleHeaderString ) != true ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "SHASConfigXmlrpc::constructor, handle hdr String failed. Using default");
		handleHeaderString = "Handle";
	}		
	
	// Enclosure : Bay
	if( StringResourceLookup( FACILITY_SHAS_CONFIG,
											FAC_SHAS_CONFIG_ENCLOSURE_HDR_STRING,
											enclNumHeaderString ) != true ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "SHASConfigXmlrpc::constructor, Encl Num String failed. Using default");
		enclNumHeaderString = "Enclosure Number";
	}	

	if( StringResourceLookup( FACILITY_SHAS_CONFIG,
											FAC_SHAS_CONFIG_BAY_HDR_STRING,
											bayNumHeaderString ) != true ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "SHASConfigXmlrpc::constructor, Bay Num String failed. Using default");
		bayNumHeaderString = "Bay Number";
	}		

	// Set the PD's Enclosure and Bay data based on Handle for later reference by the LD/LE
	setPDEnclosureAndBay();

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASConfigXmlrpc::SHASConfigXmlrpc, Exit");
	
}

/**
 * Function execute()
 * 
 */

void 
SHASConfigXmlrpc::execute(xmlrpc_c::paramList const& paramList,
				xmlrpc_c::value *   const  retvalP)
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASConfigXmlrpc::execute, Enter");

	//paramList.verifyEnd(1);
	string param1 =  paramList.getString(0);

	if( param1.size() <= 0 )	{
		throw(xmlrpc_c::fault("Additional parameter not specified and is required",
							  xmlrpc_c::fault::CODE_UNSPECIFIED));	
	}	

    traceClass->LogTrace(ZQ::common::Log::L_INFO, "\tAsking for %s", param1.c_str() );
	
   /* parse the parameter */
	if( (param1.compare("CD") == 0) ||
        (param1.compare("PD") == 0 ) ||
		(param1.compare("LD") == 0 ) ) {
		
		// For the PD <enclosure_id> case, we append the 2nd param data to the first param
		// It aint that pretty but it's a cheap way of doing it.
		if( (param1.compare("PD") == 0) && (paramList.size() == 2) ) {
					string param2 =  paramList.getString(1);
					traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\t***** Param 2 is |%s|", param2.c_str() );
					param1.append( " " + param2 );
					//param1.append(  );
		}
					
    	// Update all of our persistent objects
		pthread_mutex_lock( &ShasConfigMutex );		
	    update( param1 );
		if( (param1.compare(0,2,"PD") == 0) ) {
			buildSHAS_PD_ConfigResp( *retvalP );
		}
		else	{
			buildSHASConfigResp( *retvalP );
		}
		pthread_mutex_unlock( &ShasConfigMutex );		
		
 	}	else {
			if( (param1.compare("CL") == 0) ) {

				// Cluster is just a tag=value list
				pthread_mutex_lock( &ShasConfigMutex );		
				update( param1 );
				buildSHAS_CL_ConfigResp( *retvalP );			
				pthread_mutex_unlock( &ShasConfigMutex );		
				
			}
			else {
					traceClass->LogTrace(ZQ::common::Log::L_INFO, "SHASConfigXmlrpc::execute; throwing error and exiting");
					throw(xmlrpc_c::fault("Information specified by parameter not supported",
										xmlrpc_c::fault::CODE_UNSPECIFIED));
			}
		}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASConfigXmlrpc::execute, Exit");
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
 * SHASConfigXmlrpc::buildSHAS_PD_ConfigResp()            
 *
 * Private method to get the info for the response for the CL object
 * For cluster data (CL), it is structure of the cluster tag=value pairs
 *
 */
void
SHASConfigXmlrpc::buildSHAS_PD_ConfigResp( xmlrpc_c::value &result )
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASConfigXmlrpc::buildSHAS_PD_ConfigResp, Enter");
	
	vector<TagVal>::iterator 		tvit;					// TagVal iterator
	map<string, xmlrpc_c::value>	respMap;				// Our building map response structure
	vector<xmlrpc_c::value> 		resp;					// Our final response array	
	bool							first = true;

    // Go thru all entries in the vector and create a response to intantiate
	for ( tvit = vecDev.begin(); tvit != vecDev.end(); tvit++ ) {	

		if (first) {
			// Insert the structure tag value pair, first PD value
			respMap.insert(make_pair(xmlrpc_c::value_string(tvit->tag.c_str()),
											xmlrpc_c::value_string(tvit->value.c_str() )));
			first = false;

		} else if ((tvit->tag.compare(pdHeaderString) == 0) ) {
			//
			// Push the structure onto the response, since we've reached next struct
			//
			SCR_TRACE("\tPD Pushing respMap onto resp and clearing respMap");
			xmlrpc_c::value_string disktype = respMap["Disk Type"];
			xmlrpc_c::value_struct const respStruct(respMap);
			resp.push_back(respStruct);		
			respMap.clear();		// Clear all the entries in this map to start anew
			
//			if (disktype == "Core") // skip the core disk
//				continue; 

			//
			// insert next PD pair
			//
			respMap.insert(make_pair(xmlrpc_c::value_string(tvit->tag.c_str()),
											xmlrpc_c::value_string(tvit->value.c_str() )));	
		
		} else {
			//
			// insert fieelds aspart of resp struct
			//
			respMap.insert(make_pair(xmlrpc_c::value_string(tvit->tag.c_str()),
											xmlrpc_c::value_string(tvit->value.c_str() )));			
		}

	}  // End for tvit

	//
	// Since there's no more etries
	// Push the last structure onto the response
	//
	SCR_TRACE("\tPD Pushing respMap onto resp and clearing respMap");
	xmlrpc_c::value_struct const respStructLast(respMap);
	resp.push_back(respStructLast);		
	
	// Push the final array structure on
	xmlrpc_c::value_array const respArray(resp);
	result = respArray;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASConfigXmlrpc::buildSHAS_PD_ConfigResp, Exit");
	
}  // End buildSHAS_PD_ConfigResp

/**
 * SHASConfigXmlrpc::buildSHAS_CL_ConfigResp()            
 *
 * Private method to get the info for the response for the CL object
 * For cluster data (CL), it is structure of the cluster tag=value pairs
 *
 */

void
SHASConfigXmlrpc::buildSHAS_CL_ConfigResp( xmlrpc_c::value &result )
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASConfigXmlrpc::buildSHAS_CL_ConfigResp, Enter");
	
	vector<TagVal>::iterator 		tvit;					// TagVal iterator
	map<string, xmlrpc_c::value> respMap;			// Our building map response structure		

    // Go thru all entries in the vector and create a response to intantiate
	for ( tvit = vecDev.begin(); tvit != vecDev.end(); tvit++ ) {	
		
		// Insert the structure tag value pair
		respMap.insert(make_pair(xmlrpc_c::value_string(tvit->tag.c_str()),
											xmlrpc_c::value_string(tvit->value.c_str() )));
	
	}  // End for tvit

	// Push the final array structure on
	xmlrpc_c::value_struct const respStruct(respMap);
	result = respStruct;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASConfigXmlrpc::buildSHAS_CL_ConfigResp, Exit");
	
}  // End buildSHAS_CL_ConfigResp


/**
 * SHASConfigXmlrpc::buildSHASConfigResp()            
 *
 * Private method to get the info for the response
 *
 * For cluster data (CL), it is handled by the buildSHAS_CL_ConfigResp() method
 * For controller data (CD), it is an array of controller structures
 * Same goes for Physical Disk Data, but for Logical Drives, the data is thus:
 *   Toplevel is array of LD
 *   For each LD, you have a struct to hold LD name/value pair, and a array of LE
 *   For each LE, you have a struct to hold LE name/value pair
 *
 * NOTES:  This method is pretty ugly. It was originally written nice and clean, and
 *	unfortunately, some of the requirements changed, and it became a state machine.
 * Certainly that was not what was intended, and this can be re-written as a full, non-
 * generic object and it would be far easier. For eaxmple, the LD and LE's can be ushered
 * into their own class data elements and accessors and mutators added for access. This 
 * method's XMLRPC response creation could be much simpler as a result.
 * Someday I guess. Lesson learned and it will not be repeated again!
 *
 */
static const char* ArraySchema[] = {
	"Attributes", "Cache Attribute", "Copy from ID", "Copy to ID", "Device Size", "Distributed Spare",
	"First Count", "Generation Number", "Handle", "Identifier", "Logical Device State", "Logical Device Type",
	"Logical Drive Number", "Name", "OS Name", "Owner", "Owner Core ID", "Preferred Element", "Reference Count",
	"Route", "Second Count", "Task %", "Task Priority", "Task Protect Size", "Task Protect Start", "Task Status", 
	"Task Type", "LE", "Array Name", NULL };
	
static const char* LESchema[] = {
	"Bay Number", "Data Offset", "Data Size", "Enclosure Number", "Errors", "Handle", "Identifier",
	"Logical Element Number", "Route", "Space Offset", "Space Size", NULL };

static const char* CDSchema[] = {
        "Enclosure Number", "Enclosure Name", "Serial Number", "Enclosure ID", "License parameters", "BIOS Version",
	"Size", "Number of Bays", "Vendor ID", "Device ID", NULL };

	
map<string, xmlrpc_c::value> buildUpStructPerSchema(const map<string, xmlrpc_c::value>& inputMap, const char* schema[])
{
	map<string, xmlrpc_c::value> mapToExport;
	for (const char** fieldName= schema; *fieldName; fieldName ++)
	{
		map<string, xmlrpc_c::value>::const_iterator it = inputMap.find(*fieldName);
		if (inputMap.end() == it)
		{
			SCR_TRACE("\tbuildUpStructPerSchema [%s] missed", *fieldName);
			continue;
		}
					
		mapToExport.insert(*it);
	}
	
	return xmlrpc_c::value_struct(mapToExport);
}

void
SHASConfigXmlrpc::buildSHASConfigResp( xmlrpc_c::value &result )
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASConfigXmlrpc::buildSHASConfigResp, Enter");
	
	vector<TagVal>::iterator 		tvit;					// TagVal iterator
	bool  								first = true; 		// To indicate first time thru flag
	bool  								leState = false;	// To indicate we are in an LE loop state
	bool  								leAdded = false; // To indicate we are in an LE loop state
	vector<xmlrpc_c::value> 		resp;					// Our final response array
	map<string, xmlrpc_c::value> respMap;			// Our building map response structure		
	map<string, xmlrpc_c::value> leMap;			// Our le map response structure		
	vector<xmlrpc_c::value> 		leArray;				// List of LE entries

    // Go thru all entries in the vector and create a response to intantiate
	// Note: This loop contains a two stage-array for the LD and LE types.
	//          Other than the "first" flag, the other CD and PD types are stateless.
	//
	bool isCD = false;         
	for ( tvit = vecDev.begin(); tvit != vecDev.end(); tvit++ ) {	

		// We check the heading for each LE array element
		if ( !first && 
						(tvit->tag.compare( cdHeaderString ) == 0 ||
						tvit->tag.compare( ldHeaderString )  == 0 ) )  {

			isCD = (tvit->tag.compare( cdHeaderString ) == 0 );
						
			// Push the LE array onto the value tag
			if( leState == true && leAdded == false ) {

			SCR_TRACE("\tInserting leArray to respMap and clearing leArray");			
			
				leMap.insert(make_pair(xmlrpc_c::value_string(tvit->tag.c_str()),
											xmlrpc_c::value_string(tvit->value.c_str() )));
				
				xmlrpc_c::value_struct leStruct= buildUpStructPerSchema(leMap, LESchema);
				leMap.clear();
				
				leArray.push_back(leStruct);		
				respMap.insert(make_pair("LE", xmlrpc_c::value_array(leArray)));					

				leAdded = true;
				leState = false;

				leArray.clear();
			}			
			SCR_TRACE("\tPushing respMap onto resp and clearing respMap");			

			// Push the structure onto the response
			
			xmlrpc_c::value_struct respStruct = buildUpStructPerSchema(respMap, (isCD ? CDSchema : ArraySchema));
			respMap.clear();		// Clear all the entries in this map to start anew	

			resp.push_back(respStruct);		
		}
		
		if( tvit->tag.compare( leHeaderString )  == 0 ) {
		
			SCR_TRACE("\ttag is Logical Element Number. leState=%s leAdded=%s", leState?"true":"false", leAdded?"true":"false");			
			
			leState = true;

			// Push the LE structure onto the response
			if( leMap.size() > 0 )	{
				SCR_TRACE("\tleMap size is %d, Pushing leMap onto leArray; Clearing leMap; set leAdded=FALSE", leMap.size() );			
				xmlrpc_c::value_struct leStruct = buildUpStructPerSchema(leMap, LESchema);
				leMap.clear();		// Clear all the entries in this map to start anew		

				leArray.push_back(leStruct);		
				leAdded = false;
			}				
		}

		if( leState == true ) {

			// Do we have a tag matching "Handle" - which is our Disk Number Handle in the PD for the Enclosure:Bay
			if( tvit->tag.compare( handleHeaderString ) == 0 ) 	{
				string enclosure, bay;
				
				// Locate the Enclosre and Bay, if they are present
				if( getPDEnclosureAndBay( tvit->value, enclosure, bay ) ) {

					SCR_TRACE("\tleState=TRUE - HANDLE HEADER FOUND. Adding Enclosure=%s Bay=%s ", enclosure.c_str(), bay.c_str() );

					// Add the encl and bay fields as tag=value pairs
					leMap.insert(make_pair(xmlrpc_c::value_string(enclNumHeaderString),
										xmlrpc_c::value_string(enclosure.c_str() )));				

					leMap.insert(make_pair(xmlrpc_c::value_string(bayNumHeaderString),
										xmlrpc_c::value_string(bay.c_str() )));				
				} // end if
				else {
					SCR_TRACE("\tleState=TRUE - NO HANDLE HEADER FOUND." );
				}
			}
		
			SCR_TRACE("\tleState=TRUE  inserting into leMap tag=|%s|  val=|%s|", tvit->tag.c_str(), tvit->value.c_str());			
			leMap.insert(make_pair(xmlrpc_c::value_string(tvit->tag.c_str()),
										xmlrpc_c::value_string(tvit->value.c_str() )));
										
		}
		else	{
			SCR_TRACE("\tleState=FALSE  inserting into respMap tag=|%s|  val=|%s|", tvit->tag.c_str(), tvit->value.c_str());			
			// Insert the structure tag value pair
			respMap.insert(make_pair(xmlrpc_c::value_string(tvit->tag.c_str()),
													xmlrpc_c::value_string(tvit->value.c_str() )));
		}
		
		first = false;			// Clear our "first time looping" flag
	
	}  // End for tvit
	
	// Push the LE array onto the value tag for the final LD LE
	if( leState == true && leAdded == false ) {

		SCR_TRACE("\tFINAL Operation: Inserting leArray to respMap and clearing leArray");			
			
//		xmlrpc_c::value_struct const leStruct(leMap);
		xmlrpc_c::value_struct leStruct = buildUpStructPerSchema(leMap, LESchema);
		leMap.clear();

		leArray.push_back(leStruct);		
											
		respMap.insert(make_pair("LE", xmlrpc_c::value_array(leArray)));					
	}		
	
	SCR_TRACE("\tPush final struct on");			
	// Push the final array structure on
	xmlrpc_c::value_struct respStruct = buildUpStructPerSchema(respMap, (isCD ? CDSchema : ArraySchema));
	respMap.clear();		// Clear all the entries in this map to start anew	

	resp.push_back(respStruct);			
	
	// Create and instantiate the final response
	xmlrpc_c::value_array finalResp(resp);
	result = finalResp;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SHASConfigXmlrpc::buildSHASConfigResp, Exit");
}
 
 
/* End SHASConfigXmlrpc.cpp */
