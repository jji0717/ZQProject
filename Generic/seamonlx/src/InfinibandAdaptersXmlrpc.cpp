/* @File InfinibandAdapersXmlrpc.cpp
 * Supports reporting on InfiniBand Adapters and their associated ports
 * the ports are represented as a vector of AdapterInterfaces.
 *
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include "InfinibandAdaptersXmlrpc.h"
#include "common.h"

using namespace std;
using namespace seamonlx;

/**
 * We need a mutex to remain thread safe
 */
pthread_mutex_t 				InfinibandAdaptersMutex = PTHREAD_MUTEX_INITIALIZER; 


/**
 * default constructor
 */
InfinibandAdaptersXmlrpc::InfinibandAdaptersXmlrpc()
{
	char	locstr[1024];
	string	formattedstring;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdaptersXmlrpc::InfinibandAdaptersXmlrpc, Enter");

	this->_signature = "A:s" ;

	sprintf(locstr, "For best viewing ensure your screen width is at least %d characters", (HELPLINEEND + 10));
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", "1) InfinibandAdapters List");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","   returns List of InfinibandAdapters, i.e pci addr value");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","2) InfinibandAdapters pciaddrxx ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","   returns data for this InfinibandAdapters (e.g. InfinibandAdapters 0000:07:00.0)");
	FormatHelpString(formattedstring, locstr);
	
	this->_help = formattedstring;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdaptersXmlrpc::InfinibandAdaptersXmlrpc, Exit");
}

/**
 * Function execute()
 * 
 */

void 
InfinibandAdaptersXmlrpc::execute(xmlrpc_c::paramList const& paramList,
				xmlrpc_c::value *   const  retvalP)
{

	string param1 =  paramList.getString(0);
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdaptersXmlrpc::execute, Enter.    paramList size is %d", paramList.size() );

	if( param1.size() <= 0 )	{
		throw(xmlrpc_c::fault("Additional parameter not specified and is required",
							  xmlrpc_c::fault::CODE_UNSPECIFIED));	
	}

	pthread_mutex_lock( &InfinibandAdaptersMutex );			
	
    update();
	
	if( param1.compare( "List" ) == 0 ) {
		buildInfinibandAdaptersResp(*retvalP );		
	}
	else {
			if( param1.compare( "Interfaces" ) == 0 ) {
			
				//paramList.verifyEnd(2);
				if( paramList.size() == 2 ) {

					string param2 =  paramList.getString(1);
					//traceClass->LogTrace(ZQ::common::Log::L_INFO,  "***** Param 2 is |%s|", param2.c_str() );
					buildInfinibandInterfaceResp( param2, *retvalP );		
			
				} else {
			
				buildInfinibandInterfacesResp(*retvalP );		
				
				}
			}
			else {
				buildInfinibandAdapterResp( param1, *retvalP );		
			}
	}
	
	pthread_mutex_unlock( &InfinibandAdaptersMutex );			

	//catch {
	//	pthread_mutex_unlock( &InfinibandAdaptersMutex );			
	//}
	
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdaptersXmlrpc::execute, Exit");
}

/**
 * InfinibandAdaptersXmlrpc::buildInfinibandAdaptersResp()            
 *
 * Private method to get the info for the response
 * 
 */

void
InfinibandAdaptersXmlrpc::buildInfinibandAdaptersResp( xmlrpc_c::value &result )
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdaptersXmlrpc::buildInfinibandAdaptersResp, Enter."  );

	vector	<AdapterInfo>	 ::iterator		it;
	map		<string, xmlrpc_c::value>		respStruct;
	vector	<xmlrpc_c::value> 				respArray;
	string									state;

	for ( it = vecAdapters.begin(); it != vecAdapters.end(); it++ ) {	

		respStruct.insert(make_pair("PCI Address",
											xmlrpc_c::value_string(it->pciAddressWithDomain)));

		state = it->healthState.getHealthStateString(it->healthState.getHealthState());									
											
		respStruct.insert(make_pair("Target Overall Health",
											xmlrpc_c::value_string(state)));

																			
		xmlrpc_c::value_struct entry(respStruct);	
		respArray.push_back( entry );
		respStruct.clear();

	}
	
	xmlrpc_c::value_array array(respArray);
	result = array;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdaptersXmlrpc::buildInfinibandAdaptersResp, Exit");
}

/**
 * InfinibandAdaptersXmlrpc::buildInfinibandAdapterResp()            
 *
 * Private method to get the info for a single adapter
 * 
 */
void
InfinibandAdaptersXmlrpc::buildInfinibandAdapterResp( string pciAddr, xmlrpc_c::value &result )
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdaptersXmlrpc::buildInfinibandAdapterResp, Enter.  pciAddr = %s", pciAddr.c_str()  );

	vector <AdapterInfo>	 :: iterator		adIt;
	map<string, xmlrpc_c::value> 				respStruct;
	map<string, xmlrpc_c::value> 				ifStruct;
	vector <xmlrpc_c::value>					ifArray;				// Interface array
	vector <InterfaceInfo>	 :: iterator		ifIt;
	string										state;
	bool										found = false;

	for ( adIt = vecAdapters.begin(); (adIt != vecAdapters.end()) && !found; adIt++ ) {	
	
		// Search for the exact entry only
		if( adIt->pciAddressWithDomain.compare( pciAddr ) == 0 ) {
		
			found = true;

			respStruct.insert(make_pair("PCI Address",
												xmlrpc_c::value_string(pciAddr)));
			respStruct.insert(make_pair("Name",
												xmlrpc_c::value_string(adIt->adapterName)));
												
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
			pthread_mutex_unlock( &InfinibandAdaptersMutex );		
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
				
				char mystring[BUFF256];
				sprintf(mystring,"%s - port num %d", ifIt->ifName.c_str(), ifIt->portNum);
				tempString.assign(mystring);
				ifStruct.insert(make_pair("Interface name",
													xmlrpc_c::value_string(tempString)));

				ifStruct.insert(make_pair("IP address",
													xmlrpc_c::value_string(ifIt->ipAddress)));
				ifStruct.insert(make_pair("Link status",
													xmlrpc_c::value_string(ifIt->linkStatus)));
				
				tempString.clear();
				tempString = ifIt->ibstate;
				ifStruct.insert(make_pair("State", xmlrpc_c::value_string(tempString)));
				
				ifStruct.insert(make_pair("Link rate",
													xmlrpc_c::value_string(ifIt->linkRate)));
				ifStruct.insert(make_pair("Port Number",
													xmlrpc_c::value_int(ifIt->portNum)));
				ifStruct.insert(make_pair("LMC",
													xmlrpc_c::value_string(ifIt->lmc)));
				ifStruct.insert(make_pair("Base LID",
													xmlrpc_c::value_string(ifIt->baseLID)));
				ifStruct.insert(make_pair("SM LID",
													xmlrpc_c::value_string(ifIt->smLID)));

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

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdaptersXmlrpc::buildInfinibandAdapterResp, Exit");
}

/**
 * InfinibandAdaptersXmlrpc::buildInfinibandIbnterfacesResp()            
 *
 * Private method to get the info for the response
 * 
 */

void
InfinibandAdaptersXmlrpc::buildInfinibandInterfacesResp( xmlrpc_c::value &result )
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdaptersXmlrpc::buildInfinibandInterfacesResp, Enter."  );

	vector	<InterfaceInfo>	 :: iterator		it;
	map		<string, xmlrpc_c::value>			respStruct;
	vector	<xmlrpc_c::value> 					respArray;
	string										tempString;

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

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdaptersXmlrpc::buildInfinibandInterfacesResp, Exit");
}

/**
 * InfinibandAdaptersXmlrpc::buildInfinibandInterfaceResp()            
 *
 * Private method to get the info for the response
 * 
 */

void
InfinibandAdaptersXmlrpc::buildInfinibandInterfaceResp( string pciAddr, xmlrpc_c::value &result )
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdaptersXmlrpc::buildInfinibandInterfaceResp, Enter. pciAddr=%s", pciAddr.c_str()  );

	vector	<InterfaceInfo>	 :: iterator		it;
	vector	<xmlrpc_c::value>					ifArray;				// Interface array
	map		<string, xmlrpc_c::value> 			respStruct;
	string										tempString;
	bool										found = false;

	for ( it = vecInterfaces.begin(); it != vecInterfaces.end(); it++ ) {	

		tempString = it->pciAddress;
		tempString.append( it->domain );
			
		//traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tCompare tempString |%s| to pciAddr |%s|", tempString.c_str(), pciAddr.c_str() );
		
		// Search for the exact entry only
		if( tempString.compare( pciAddr ) == 0 ) {
		
			respStruct.insert(make_pair("PCI address",
											xmlrpc_c::value_string(tempString)));
			found = true;

			tempString = it->ifName;
			char mystring[BUFF256];
			sprintf(mystring," port num %d", it->portNum);
			tempString.append(mystring);
			
			respStruct.insert(make_pair("Interface name",
											xmlrpc_c::value_string(tempString)));
			respStruct.insert(make_pair("IP address",
											xmlrpc_c::value_string(it->ipAddress)));

			respStruct.insert(make_pair("Link status",
											xmlrpc_c::value_string(it->linkStatus)));
			
			tempString.clear();
			tempString = it->ibstate;
			respStruct.insert(make_pair("State", xmlrpc_c::value_string(tempString)));
			
			respStruct.insert(make_pair("Link rate",
											xmlrpc_c::value_string(it->linkRate)));
			respStruct.insert(make_pair("Port Number",
											xmlrpc_c::value_int(it->portNum)));
			respStruct.insert(make_pair("LMC",
											xmlrpc_c::value_string(it->lmc)));	
			respStruct.insert(make_pair("Base LID",
											xmlrpc_c::value_string(it->baseLID)));
			respStruct.insert(make_pair("SM LID",
											xmlrpc_c::value_string(it->smLID)));
			
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
			pthread_mutex_unlock( &InfinibandAdaptersMutex );		
			throw(xmlrpc_c::fault("PCI address of network interface not found.",
							  xmlrpc_c::fault::CODE_UNSPECIFIED));		
	}

	xmlrpc_c::value_array locarray(ifArray);	
	result = locarray;
	

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdaptersXmlrpc::buildInfinibandInterfacesResp, Exit");
}

/* End InfinibandAdaptersXmlrpc.cpp */

