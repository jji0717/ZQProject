/** @file DiskXmlrpc.h
 *
 *  DiskXmlrpc class member function definition.
 *  
 *  SeaChange International
 *  
 *  Revision History
 *  
 *  04-28-2010 Created by jiez
 *
 *  05-06-2010 jiez
 *    - Modify to provide block device name as part of disk display ID
 *
 *  05-13-2010 jiez
 *    - Modify to access DiskDrive through pointer due to the data member
 *      disks's change in base class.
 *    - Added "Standalone" parameter to execute function to get a disk list that
 *      doesn't attached to enclosure or adapter.
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
#include "DiskXmlrpc.h"
#include "common.h"

using namespace std;
using namespace seamonlx;

/**
 * default constructor
 */
DiskXmlrpc::DiskXmlrpc()
{
	char	locstr[1024];
	string	formattedstring;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "DiskXmlrpc::DiskXmlrpc, Enter");

	this->_signature = "A:s" ;

	sprintf(locstr, "For best viewing ensure your screen width is at least %d characters", (HELPLINEEND + 10));
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s", "Disk [List, StandAlone]");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","1) Disk List");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","       returns List of Disks");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","2) Disk StandAlone");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","       returns List of StandAlone Disks");
	FormatHelpString(formattedstring, locstr);

	this->_help = formattedstring;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "DiskXmlrpc::DiskXmlrpc, Exit");
}


/**
 * Function execute()
 */
void DiskXmlrpc::execute(xmlrpc_c::paramList const& paramList,
								  xmlrpc_c::value * const retvalP)
{
	paramList.verifyEnd(1);
	string param1 = paramList.getString(0);
	
	/* if parameter is "List", list the disk IDs*/
	if( param1.compare("List") == 0 ) {
		
		getDiskIds( *retvalP);
	}
	else if( param1.compare("Standalone") == 0 ) {
		
		getStandaloneDiskIds( *retvalP);
	}
	/* else, get the disk details with given ID */
	else if( !param1.empty() ){
		getDisk( param1, *retvalP );
	}
	else{
		throw( xmlrpc_c::fault("Invalid input parameter",
							   xmlrpc_c::fault::CODE_UNSPECIFIED));
		
	}
}



/**
 * Function getDiskIds
 *
 * get the of the disk IDs and return them in an xmlrpc_c::value.
 *
 * @param[out] result the xmlrpc_c::value saves the list of disk ids
 */ 
void
DiskXmlrpc::getDiskIds( xmlrpc_c::value &result )
{
	vector<xmlrpc_c::value> idarray;

	vector<DiskDrive *>::iterator it;
	for( it = disks.begin(); it != disks.end(); it ++ ){
		string seacid = (*it)->getId();
		string bdname = (*it)->getBdname();
		if( seacid.compare(bdname) ){
			seacid.append("(");
			seacid.append(bdname);
			seacid.append(")");
		}
		
		idarray.push_back(xmlrpc_c::value_string(seacid));
	}

	xmlrpc_c::value_array ids(idarray);
	result = ids;
}


/**
 * Function getStandaloneDiskIds
 *
 * get the of the disk IDs and return them in an xmlrpc_c::value.
 *
 * @param[out] result the xmlrpc_c::value saves the list of disk ids
 */ 
void
DiskXmlrpc::getStandaloneDiskIds( xmlrpc_c::value &result )
{
	vector<xmlrpc_c::value> idarray;

	vector<DiskDrive *>::iterator it;
	for( it = disks.begin(); it != disks.end(); it ++ ){
		if( ! ((*it)->getPEncl() || (*it)->getPAdpt()) ){
			string seacid = (*it)->getId();
			string bdname = (*it)->getBdname();
			if( seacid.compare(bdname) ){
				seacid.append("(");
				seacid.append(bdname);
				seacid.append(")");
			}
			
			idarray.push_back(xmlrpc_c::value_string(seacid));
		}
		
	}
	xmlrpc_c::value_array ids(idarray);
	result = ids;
}

	
/**
 * Function getDisk
 *
 * Function that get the information of an disk by given the id.
 *
 * @param[in] seacid, the unique id of the disk, the SEAC_ID of the disk.
 * @param[out] result, the xmlrpc_c::value that saves the disk information.
 */

void
DiskXmlrpc::getDisk(string seacid, xmlrpc_c::value &result)
{
	vector<DiskDrive *>::iterator it;
	map<string, xmlrpc_c::value> diskMap;

	/**
	 * Find the disk that has the id
	 */
	for( it = disks.begin(); it != disks.end(); it ++){
		if( !((*it)->getId().compare(seacid) ) ){

//			it->update();

			// add the basic information
			
			diskMap.insert(make_pair("Block Device Name",
									 xmlrpc_c::value_string((*it)->getBdname())));

			string buffer = (*it)->getSgname();
			if( ! buffer.empty() ){
				diskMap.insert(make_pair("SG Name",
										 xmlrpc_c::value_string(buffer)));
			}

			buffer = (*it)->getId();
			if( ! buffer.empty() ){
				diskMap.insert(make_pair("Logical Address",
										 xmlrpc_c::value_string(buffer)));
			}

			buffer = (*it)->getScsiaddr();
			if( ! buffer.empty() ){
				diskMap.insert(make_pair("SCSI Address",
										 xmlrpc_c::value_string(buffer)));
			}

			buffer = (*it)->getVendor();
			if( ! buffer.empty() ){
				
				diskMap.insert(make_pair("Manufacturer",
										 xmlrpc_c::value_string(buffer)));
			}

			buffer = (*it)->getModel();
			if( ! buffer.empty() ){
				diskMap.insert(make_pair("Model Number",
										 xmlrpc_c::value_string(buffer)));
			}
			
			buffer = (*it)->getSerialnum();
			if( ! buffer.empty() ){
				diskMap.insert(make_pair("Serial Number",
										 xmlrpc_c::value_string(buffer)));
			}
			
			buffer = (*it)->getFwver();
			if( ! buffer.empty() ){
				diskMap.insert(make_pair("Firmware Version",
										 xmlrpc_c::value_string(buffer)));
			}

			buffer = (*it)->getSize();
			if( ! buffer.empty() ){
				diskMap.insert(make_pair("Capacity",
										 xmlrpc_c::value_string(buffer)));
			}
			
			buffer = (*it)->getTemp();
			if( ! buffer.empty() ){
				diskMap.insert(make_pair("Temperature",
										 xmlrpc_c::value_string(buffer)));
			}

			buffer = (*it)->getState();
			if( ! buffer.empty() ){
				diskMap.insert(make_pair("State",
										 xmlrpc_c::value_string(buffer)));
			}

			buffer = (*it)->getIoerr();
			if( !buffer.empty() ){
				diskMap.insert(make_pair("I/O Error Count",
										 xmlrpc_c::value_string(buffer)));
			}

			buffer = (*it)->getCa();
			if( !buffer.empty() ){
				diskMap.insert(make_pair("Cache Attribute",
										 xmlrpc_c::value_string(buffer)));
			}

			buffer = (*it)->getBlkr();
			if( !buffer.empty() ){
				diskMap.insert(make_pair("Blocks Read",
										 xmlrpc_c::value_string(buffer)));
			}

			buffer = (*it)->getBlkw();
			if ( ! buffer.empty() ){
				
				diskMap.insert(make_pair("Blocks Written",
										 xmlrpc_c::value_string(buffer)));
			}
			
		}
			
	}

	xmlrpc_c::value_struct diskStruct( diskMap );
	result = diskStruct;
}


