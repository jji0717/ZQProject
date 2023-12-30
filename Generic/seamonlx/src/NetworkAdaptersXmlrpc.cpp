/* @File NetworkAdapersXmlrpc.cpp
 *
 *
 *  SHAS Counters object XMLRPC Support methods implementation.
 *  
 *
 *
 *  Revision History
 *  
 *  06-11-2010 mjc  Created
 *  07-06-2010 mjc  Handled special case for Chelsio adapter and how it lists multiple interfaces
 *  07-07-2010 mjc  Added Interface Name to the fields of the structure for buildNetworkInterfacesResp
 * 							to uniquely identify the dumplicate PCI Address for the Chelsio interfaces.
 * 
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include "NetworkAdaptersXmlrpc.h"
#include "common.h"

using namespace std;
using namespace seamonlx;

/**
 * We need a mutex to remain thread safe
 */
pthread_mutex_t 				NetworkAdaptersMutex = PTHREAD_MUTEX_INITIALIZER; 


/**
 * default constructor
 */
NetworkAdaptersXmlrpc::NetworkAdaptersXmlrpc()
{
	char	locstr[1024];
	string	formattedstring;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdaptersXmlrpc::NetworkAdaptersXmlrpc, Enter");

	this->_signature = "A:s" ;

	sprintf(locstr, "For best viewing ensure your screen width is at least %d characters", (HELPLINEEND + 10));
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", "1) NetworkAdapters List");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","   returns List of NetworkAdapters, i.e pci addr value");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","2) NetworkAdapters pciaddrxx ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","   returns data for this NetworkAdapters (e.g. NetworkAdapters 0000:07:00.0)");
	FormatHelpString(formattedstring, locstr);
	
	this->_help = formattedstring;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdaptersXmlrpc::NetworkAdaptersXmlrpc, Exit");
}

/**
 * Function execute()
 * 
 */

void 
NetworkAdaptersXmlrpc::execute(xmlrpc_c::paramList const& paramList,
				xmlrpc_c::value *   const  retvalP)
{

	string param1 =  paramList.getString(0);
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdaptersXmlrpc::execute, Enter.    paramList size is %d", paramList.size() );

	if( param1.size() <= 0 )	{
		throw(xmlrpc_c::fault("Additional parameter not specified and is required",
							  xmlrpc_c::fault::CODE_UNSPECIFIED));	
	}
	
   /* parse the parameter */
	//traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\texecute: calling update with |%s|\n", param1.c_str() );

	pthread_mutex_lock( &NetworkAdaptersMutex );			
	
    update();
	
	if( param1.compare( "List" ) == 0 ) {
		buildNetworkAdaptersResp(*retvalP );		
	}
	else {
			if( param1.compare( "Interfaces" ) == 0 ) {
			
				//paramList.verifyEnd(2);
				if( paramList.size() == 2 ) {

					string param2 =  paramList.getString(1);
					//traceClass->LogTrace(ZQ::common::Log::L_INFO,  "***** Param 2 is |%s|", param2.c_str() );
					buildNetworkInterfaceResp( param2, *retvalP );		
			
				} else {
			
				buildNetworkInterfacesResp(*retvalP );		
				
				}
			}
			else {
				buildNetworkAdapterResp( param1, *retvalP );		
			}
	}
	
	pthread_mutex_unlock( &NetworkAdaptersMutex );			

	//catch {
	//	pthread_mutex_unlock( &NetworkAdaptersMutex );			
	//}
	
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdaptersXmlrpc::execute, Exit");
}

/**
 * NetworkAdaptersXmlrpc::buildNetworkAdaptersResp()            
 *
 * Private method to get the info for the response
 * 
 */

void
NetworkAdaptersXmlrpc::buildNetworkAdaptersResp( xmlrpc_c::value &result )
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdaptersXmlrpc::buildNetworkAdaptersResp, Enter."  );

	vector <AdapterInfo>	 :: iterator		it;
	map<string, xmlrpc_c::value> respStruct;
	vector<xmlrpc_c::value> 		respArray;
	string									state;

	for ( it = vecAdapters.begin(); it != vecAdapters.end(); it++ ) {	

		respStruct.insert(make_pair("PCI Address",
											xmlrpc_c::value_string(it->pciAddressWithDomain)));
											
		respStruct.insert(make_pair("Management Overall Health",
											xmlrpc_c::value_string( getManagementOverallHealth() )));		
		
		state = it->healthState.getHealthStateString(it->healthState.getHealthState());									
											
		respStruct.insert(make_pair("Target Overall Health",
											xmlrpc_c::value_string(state)));

		xmlrpc_c::value_struct entry(respStruct);	
		respArray.push_back( entry );
		respStruct.clear();

	}
	
	xmlrpc_c::value_array array(respArray);
	result = array;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdaptersXmlrpc::buildNetworkAdaptersResp, Exit");
}

/**
 * NetworkAdaptersXmlrpc::buildNetworkAdapterResp()            
 *
 * Private method to get the info for a single adapter
 * 
 */
void
NetworkAdaptersXmlrpc::buildNetworkAdapterResp( string pciAddr, xmlrpc_c::value &result )
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdaptersXmlrpc::buildNetworkAdapterResp, Enter.  pciAddr = %s", pciAddr.c_str()  );

	vector <AdapterInfo>	 :: iterator		adIt;
	map<string, xmlrpc_c::value> 				respStruct;
	map<string, xmlrpc_c::value> 				ifStruct;
	vector <xmlrpc_c::value>					ifArray;				// Interface array
	vector <InterfaceInfo>	 :: iterator		ifIt;
	string												state;
	bool													found = false;

	for ( adIt = vecAdapters.begin(); (adIt != vecAdapters.end()) && !found; adIt++ ) {	
	
		// Search for the exact entry only
		if( adIt->pciAddressWithDomain.compare( pciAddr ) == 0 ) {
		
			found = true;

			respStruct.insert(make_pair("PCI Address",
												xmlrpc_c::value_string(pciAddr)));
			respStruct.insert(make_pair("Name",
												xmlrpc_c::value_string(adIt->interfaceName)));
												
			state = adIt->healthState.getHealthStateString(adIt->healthState.getHealthState());									
												
			respStruct.insert(make_pair("Status",
												xmlrpc_c::value_string(state)));
			respStruct.insert(make_pair("Driver name",
												xmlrpc_c::value_string(adIt->driverName)));
			respStruct.insert(make_pair("Driver version",
												xmlrpc_c::value_string(adIt->driverVersion)));
			respStruct.insert(make_pair("Firmware version",
												xmlrpc_c::value_string(adIt->firmwareVersion)));
			respStruct.insert(make_pair("Class",
												xmlrpc_c::value_string(adIt->aClass)));
			respStruct.insert(make_pair("Manufacturer",
												xmlrpc_c::value_string(adIt->manufacturer)));
			respStruct.insert(make_pair("Model",
												xmlrpc_c::value_string(adIt->model)));
			respStruct.insert(make_pair("Revision",
												xmlrpc_c::value_string(adIt->revision)));
											
		}  // End if
	}  // End while

	// Check if we found the adapter, and if not, throw an exception
	if( !found ) {
			pthread_mutex_unlock( &NetworkAdaptersMutex );		
			throw(xmlrpc_c::fault("PCI address of network adapter not found.",
							  xmlrpc_c::fault::CODE_UNSPECIFIED));	
	}
	else {			// We found it, tack on the interfaces related to this adapter
	
		string										tempString;
		char 									    pciBuffer[BUFF64];

		// Strip off the .0 digits and use that for comparison
		if( pciAddr.copy( pciBuffer, 10 ) == 0  ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "\tCopy of 10 chars for pciAddr failed!  pciAddr is |%s|", pciAddr.c_str() );
			pciBuffer[0] = '\0';		// Force buffer to empty
		}
		else { 
			pciBuffer[10] = '\0'; 		// Make sure we are null terminated where we need to be
		}		
		
		// Loop and process each array entry
		for ( ifIt = vecInterfaces.begin(); pciBuffer[0] != '\0' && ifIt != vecInterfaces.end(); ifIt++ ) {	
			
			//traceClass->LogTrace(ZQ::common::Log::L_INFO, "\tCompare pciAddress |%s| to pciBuffer |%s|", ifIt->pciAddress.c_str(), pciBuffer );

			if( ifIt->pciAddress.compare( pciBuffer ) == 0 ) {			
			
				tempString = ifIt->pciAddress;
				tempString.append( ifIt->domain );
				ifStruct.insert(make_pair("PCI address",
													xmlrpc_c::value_string(tempString)));
				ifStruct.insert(make_pair("Interface name",
													xmlrpc_c::value_string(ifIt->ifName)));													
				ifStruct.insert(make_pair("IP address",
													xmlrpc_c::value_string(ifIt->ipAddress)));
				ifStruct.insert(make_pair("HW address",
													xmlrpc_c::value_string(ifIt->hwAddress)));
				ifStruct.insert(make_pair("Link status",
													xmlrpc_c::value_string(ifIt->linkStatus)));
				ifStruct.insert(make_pair("Link rate",
													xmlrpc_c::value_string(ifIt->linkRate)));
				ifStruct.insert(make_pair("Port count",
													xmlrpc_c::value_int(ifIt->portCount)));
				ifStruct.insert(make_pair("Locating",
													xmlrpc_c::value_int(ifIt->locating)));
				ifStruct.insert(make_pair("Port counters",
													xmlrpc_c::value_string(ifIt->portCounters)));
													
				tempString.clear();
				tempString = ifIt->healthState.getHealthStateString(ifIt->healthState.getHealthState());									
													
				ifStruct.insert(make_pair("Status",
													xmlrpc_c::value_string(tempString)));

				xmlrpc_c::value_struct entry(ifStruct);	
				ifArray.push_back( entry );
				ifStruct.clear();
				
				//traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tPushed ifArray entry. Size is %d", (int) ifArray.size() );  
				
			} // End IF to match PCI Address
			
		}

		// Place it into the respStruct
		respStruct.insert(make_pair("Interfaces",
												xmlrpc_c::value_array(ifArray)));

		xmlrpc_c::value_struct resp(respStruct);
		result = resp;
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdaptersXmlrpc::buildNetworkAdapterResp, Exit");
}

/**
 * NetworkAdaptersXmlrpc::buildNetworkIbnterfacesResp()            
 *
 * Private method to get the info for the response
 * 
 */

void
NetworkAdaptersXmlrpc::buildNetworkInterfacesResp( xmlrpc_c::value &result )
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdaptersXmlrpc::buildNetworkInterfacesResp, Enter."  );

	vector <InterfaceInfo>	 :: iterator		it;
	map<string, xmlrpc_c::value> respStruct;
	vector<xmlrpc_c::value> 		respArray;
	string									tempString;

	for ( it = vecInterfaces.begin(); it != vecInterfaces.end(); it++ ) {	

		
		tempString = it->pciAddress;
		tempString.append( it->domain );
		respStruct.insert(make_pair("PCI address",
											xmlrpc_c::value_string(tempString)));
											
		tempString.clear();
		tempString = it->healthState.getHealthStateString(it->healthState.getHealthState());									
											
		respStruct.insert(make_pair("Overall Health",
											xmlrpc_c::value_string(tempString)));

		respStruct.insert(make_pair("Interface name",
											xmlrpc_c::value_string(it->ifName)));
											
											
		xmlrpc_c::value_struct entry(respStruct);	
		respArray.push_back( entry );
		respStruct.clear();
											
	}
	
	xmlrpc_c::value_array array(respArray);
	result = array;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdaptersXmlrpc::buildNetworkInterfacesResp, Exit");
}

/**
 * NetworkAdaptersXmlrpc::buildNetworkInterfaceResp()            
 *
 * Private method to get the info for the response
 * 
 */

void
NetworkAdaptersXmlrpc::buildNetworkInterfaceResp( string pciAddr, xmlrpc_c::value &result )
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdaptersXmlrpc::buildNetworkInterfaceResp, Enter. pciAddr=%s", pciAddr.c_str()  );

	vector <InterfaceInfo>	 :: iterator		it;
	vector <xmlrpc_c::value>					ifArray;				// Interface array
	map<string, xmlrpc_c::value> 			respStruct;
	string												tempString;
	bool													found = false;

	for ( it = vecInterfaces.begin(); it != vecInterfaces.end(); it++ ) {	

		tempString = it->pciAddress;
		tempString.append( it->domain );
			
		//traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tCompare tempString |%s| to pciAddr |%s|", tempString.c_str(), pciAddr.c_str() );
		
		// Search for the exact entry only
		if( tempString.compare( pciAddr ) == 0 ) {
		
			found = true;		
			respStruct.insert(make_pair("Interface name",
											xmlrpc_c::value_string(it->ifName)));
			respStruct.insert(make_pair("PCI address",
											xmlrpc_c::value_string(tempString)));
			respStruct.insert(make_pair("IP address",
											xmlrpc_c::value_string(it->ipAddress)));
			respStruct.insert(make_pair("HW address",
											xmlrpc_c::value_string(it->hwAddress)));
			respStruct.insert(make_pair("Link status",
											xmlrpc_c::value_string(it->linkStatus)));
			respStruct.insert(make_pair("Link rate",
											xmlrpc_c::value_string(it->linkRate)));
			respStruct.insert(make_pair("Port count",
											xmlrpc_c::value_int(it->portCount)));
			respStruct.insert(make_pair("Locating",
												xmlrpc_c::value_int(it->locating)));
			respStruct.insert(make_pair("Port counters",
											xmlrpc_c::value_string(it->portCounters)));
											
			tempString.clear();
			tempString = it->healthState.getHealthStateString(it->healthState.getHealthState());									
											
			respStruct.insert(make_pair("Status",
											xmlrpc_c::value_string(tempString)));

			xmlrpc_c::value_struct entry(respStruct);	
			ifArray.push_back( entry );
			respStruct.clear();
		}
											
	}
	
	if( !found ) {
			pthread_mutex_unlock( &NetworkAdaptersMutex );		
			throw(xmlrpc_c::fault("PCI address of network interface not found.",
							  xmlrpc_c::fault::CODE_UNSPECIFIED));		
	}

	xmlrpc_c::value_array array(ifArray);	
	result = array;
	

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdaptersXmlrpc::buildNetworkInterfacesResp, Exit");
}



/**
 * NetworkAdaptersXmlrpc::setLocatingFlag()
 *
 * Private method to set the Locating (Ident) bit for a network Interface
 * 
 */
void NetworkAdaptersXmlrpc::setLocatingFlag( bool value )
{
	value = value; 	// This eliminated the compiler warning for unused parameter
	
	// Need to locate the specific Network Interface and then set this bit.
	// How to do that needs to be determined.
	
}
 
 

/* End NetworkAdaptersXmlrpc.cpp */
