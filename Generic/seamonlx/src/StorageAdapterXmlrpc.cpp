/** @File StorageAdapterXmlrpc.cpp
 * 
 *  StorageAdapterXmlrpc class member function definition.
 *
 *  Revision History
 *  
 *  03-30-2010 Created ( jie.zhang@schange.com)
 *
 *  04-29-2010 jiez
 *      - Added the "SAS_Address" to adapter child elements
 *        which are enclosures
 *
 *   05-13-2010 jiez
 *      - Modify to access adapter objects through pointers.
 *
 *  06-14-2010  mjc  Fixed memory leak: 	Do not call result.instantiate(), rather use result = finalResp;
 *
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include "StorageAdapterXmlrpc.h"
#include "common.h"
#include <fstream>

using namespace std;
using namespace seamonlx;


/**
 * We need a mutex to remain thread safe
 */
pthread_mutex_t 				StorageAdaptersMutex = PTHREAD_MUTEX_INITIALIZER; 

/**
 * default constructor
 */
StorageAdapterXmlrpc::StorageAdapterXmlrpc()
{
	char	locstr[1024];
	string	formattedstring;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "StorageAdapterXmlrpc::StorageAdapterXmlrpc, Enter");

	this->_signature = "A:s" ;

	sprintf(locstr, "For best viewing ensure your screen width is at least %d characters", (HELPLINEEND + 10));
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", "1) StorageAdapter List");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","   returns List of StorageAdapters, i.e pci addr value");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","2) StorageAdapter pciaddrxx ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","   returns data for this StorageAdapter (e.g. StorageAdapter 0000:00:1f.2)");
	FormatHelpString(formattedstring, locstr);
	
	this->_help = formattedstring;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "StorageAdapterXmlrpc::StorageAdapterXmlrpc, Exit");
}

/**
 * Function execute()
 */ 
void
StorageAdapterXmlrpc::execute(xmlrpc_c::paramList const& paramList,
							  xmlrpc_c::value *   const  retvalP)
{
	if(paramList.size() == 1){
		
		string param1 = paramList.getString(0);
		
		/* if parameter is "List", list the storage adapter IDs*/
		if( param1.compare("List") == 0 ) {
			
			getAdapterIds( *retvalP);
		}
		/* else, get the adapter with given ID */
		else if( !param1.empty() ){
			getAdapter( param1, *retvalP );
		}
		else{
			throw( xmlrpc_c::fault("Invalid input parameter",
								   xmlrpc_c::fault::CODE_UNSPECIFIED));
			
		}
	}
	else if( paramList.size() == 2){
		string param1 = paramList.getString(0);
		string param2 = paramList.getString(1);
		
		if( ! param1.compare("List") ){
			throw( xmlrpc_c::fault("Invalid input parameter(s)",
								   xmlrpc_c::fault::CODE_UNSPECIFIED));
		}
		else if ( !param1.empty() && !param2.compare("Children") ){
			getAdapterChildren( param1, *retvalP);
		}
		else{
			throw(xmlrpc_c::fault("Invalid input parameter",
								   xmlrpc_c::fault::CODE_UNSPECIFIED));
		}
	}
	else{
		throw( xmlrpc_c::fault("Invalid number of parameters, method takes one or two parameters.",
							   xmlrpc_c::fault::CODE_UNSPECIFIED));
	}
}


/**
 * Function getAdapterIds
 *
 * Get and return an array of the adapter IDs.
 *
 * @param[out] result the xmlrpc_c::value that saves the list
 *             of the ids.
 */ 
void
StorageAdapterXmlrpc::getAdapterIds( xmlrpc_c::value &result )
{
	vector<xmlrpc_c::value> idarray;


	vector<Adapter *>::iterator it;
	for (it = adapters.begin(); it != adapters.end(); it ++){
		idarray.push_back(xmlrpc_c::value_string((*it)->getPciaddr()));
	}

	xmlrpc_c::value_array ids(idarray);
	result = ids;
}



/**
 * Function getAdapter
 *
 * Function that returns the information of the adapter has the
 * given ID.
 * 
 * @param[in] pciaddr the SEAC_ID of the adapter
 * @param[out] result the xmlrpc_c::value that saves the adapter
 *             information.
 */

void
StorageAdapterXmlrpc::getAdapter(string pciaddr,
								 xmlrpc_c::value &result)
{
	vector<Adapter *>::iterator it;
	map<string, xmlrpc_c::value> adapterMap;
	
	/**
	 *Find the adapter with the pciaddr and build the xml response
	 */
	for (it = adapters.begin(); it != adapters.end(); it ++){
		if ( ! ((*it)->getPciaddr()).compare(pciaddr) ){
			
			adapterMap.insert(make_pair(
								  "SEAC_ID",
								  xmlrpc_c::value_string(pciaddr)));
			adapterMap.insert(make_pair(
								  "PCI Address",
								  xmlrpc_c::value_string(pciaddr)));
			if( ! (*it)->getDrname().empty() ){
				adapterMap.insert(make_pair(
									  "Driver Name",
									  xmlrpc_c::value_string((*it)->getDrname() )));
			}

			if( ! (*it)->getDrver().empty() ){
				adapterMap.insert(make_pair(
									  "Driver Version",
									  xmlrpc_c::value_string((*it)->getDrver() )));
			}

			if( ! (*it)->getStatus().empty() ){
				adapterMap.insert(make_pair(
									  "Status",
									  xmlrpc_c::value_string((*it)->getStatus() )));
			}
			
			if( ! (*it)->getVendor().empty() ){
				adapterMap.insert(make_pair(
									  "Vendor",
									  xmlrpc_c::value_string((*it)->getVendor() )));
			}
			if( ! (*it)->getVendorid().empty() ){
				adapterMap.insert(make_pair(
									  "Vendor ID",
									  xmlrpc_c::value_string((*it)->getVendorid() )));
			}
			if( ! (*it)->getDeviceid().empty() ){
				adapterMap.insert(make_pair(
									  "Device ID",
									  xmlrpc_c::value_string((*it)->getDeviceid() )));
			}
			
			if( ! (*it)->getFwver().empty() ){
				adapterMap.insert(make_pair(
									  "Firmware Version",
									  xmlrpc_c::value_string((*it)->getFwver() )));
			}
			
			/* update phys to get the real time data*/

			(*it)->updatePhys();

			/* add phy information to response */
			
			vector<xmlrpc_c::value> phyArray;
			unsigned int phyi;
			for (phyi = 0; phyi < (*it)->getPhys().size(); phyi ++){

				map<string, xmlrpc_c::value> phyelem;
				phyelem.insert(make_pair("Phy Number",
										 xmlrpc_c::value_string((*it)->getPhys()[phyi].phynum) ));
				if( ! ((*it)->getPhys()[phyi].sas_addr).empty() ){

					phyelem.insert(make_pair("SAS Address",
											 xmlrpc_c::value_string((*it)->getPhys()[phyi].sas_addr) ));
				}

				if( ! ((*it)->getPhys()[phyi].port).empty() ){

					phyelem.insert(make_pair("Port ID",
											 xmlrpc_c::value_string((*it)->getPhys()[phyi].port) ));
				}

				if( ! ((*it)->getPhys()[phyi].linkstatus).empty() ){
					
					phyelem.insert(make_pair("Link Status",
											 xmlrpc_c::value_string((*it)->getPhys()[phyi].linkstatus) ));
				} else {
					phyelem.insert(make_pair("Link Status",
											 xmlrpc_c::value_string((*it)->getPhys()[phyi].linkstatus.assign("UNKNOWN"))));
				}


				if( !((*it)->getPhys()[phyi].linkrate).empty() ){
					
					phyelem.insert(make_pair("Negotiated Rate",
											 xmlrpc_c::value_string((*it)->getPhys()[phyi].linkrate) ));
				}

				if( !((*it)->getPhys()[phyi].minrate).empty() ){
					
					phyelem.insert(make_pair("Minimum Rate",
											 xmlrpc_c::value_string((*it)->getPhys()[phyi].minrate) ));
				}

				if( !((*it)->getPhys()[phyi].maxrate).empty() ){
					
					phyelem.insert(make_pair("Maximum Rate",
											 xmlrpc_c::value_string((*it)->getPhys()[phyi].maxrate) ));
				}

				phyelem.insert(make_pair("Invalid Dword Count",
										 xmlrpc_c::value_int((*it)->getPhys()[phyi].invalid_dword_count) ));
				
									
				phyelem.insert(make_pair("Loss of Dword Sync Count",
										 xmlrpc_c::value_int((*it)->getPhys()[phyi].loss_of_dword_sync_count) ));
				
				phyelem.insert(make_pair("Phy Reset Problem Count",
										 xmlrpc_c::value_int((*it)->getPhys()[phyi].phy_reset_problem_count) ));

					
				phyelem.insert(make_pair("Running Disparity Error Count",
										 xmlrpc_c::value_int((*it)->getPhys()[phyi].running_disparity_error_count) ));
				
				xmlrpc_c::value_struct const phystruct(phyelem);
				phyArray.push_back(phystruct);
				
			}
			if( ! phyArray.empty() ){
				
				adapterMap.insert(make_pair("Phys",
											xmlrpc_c::value_array(phyArray)));
			}
		}
	}
	
	xmlrpc_c::value_struct adStruct(adapterMap);
	result = adStruct;
}


/**
 * Function getAdapterChildren
 *
 * Function that returns the children elements attached to the adapter.
 * 
 * @param[in] pciaddr the SEAC_ID of the adapter
 * @param[out] result the xmlrpc_c::value that saves the children infomation.
 * 
 */

void
StorageAdapterXmlrpc::getAdapterChildren(string pciaddr,
								 xmlrpc_c::value &result)
{

	vector<Adapter *>::iterator it;
	for (it = adapters.begin(); it != adapters.end(); it ++){
		if ( ! ((*it)->getPciaddr()).compare(pciaddr) ){
			
			/* update childelemts */
			(*it)->updateChildelem();

			vector<xmlrpc_c::value> cearray;
			unsigned int ceidx;
			for( ceidx = 0; ceidx < (*it)->getChildelem().size(); ceidx++ ){
				
				map<string, xmlrpc_c::value> cemap;

				// seperate the sgname and sas address
				string myid = (*it)->getChildelem()[ceidx].id;
				size_t pos1 = myid.find_first_of("(");
				size_t pos2 = myid.find_last_of(")");
				
				string sgname= myid.substr(0, pos1);
				string sas = myid.substr(pos1+1, pos2-pos1-1);
				trimSpaces(sgname);
				trimSpaces(sas);
				
				
//				cemap.insert(make_pair("SEAC_ID",
//									   xmlrpc_c::value_string(it->getChildelem()[ceidx].id)));
				cemap.insert(make_pair("SEAC_ID",
									   xmlrpc_c::value_string(sgname)));

				cemap.insert(make_pair("SAS_Address",
									   xmlrpc_c::value_string(sas)));
				
				cemap.insert(make_pair("Type",
									   xmlrpc_c::value_string((*it)->getChildelem()[ceidx].type)));
								  
				xmlrpc_c::value_struct const cestruct(cemap);
				cearray.push_back(cestruct);
			}

			xmlrpc_c::value_array children(cearray);
			result = children;
		}
	}
}
