/** @file EnclProcessorXmlrpc.h
 *
 *  EnclProcessorXmlrpc class member function definition.
 *  
 *  SeaChange International
 *  
 *  Revision History
 *  
 *  03-30-2010 jiez
 *   - Created
 *
 *  05-13-2010 jiez
 *   - Modified to access Enclosure through pointers due to the
 *     data member encls' change in base class.
 *
 *  05-06-2010 jiez
 *   - Added sgname and block device name to Disk struct.
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
#include "EnclProcessorXmlrpc.h"
#include "common.h"

using namespace std;
using namespace seamonlx;

/**
 * default constructor
 */
EnclProcessorXmlrpc::EnclProcessorXmlrpc()
{
	char	locstr[1024];
	string	formattedstring;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "EnclProcessorXmlrpc::EnclProcessorXmlrpc, Enter");

	this->_signature = "A:s" ;

	sprintf(locstr, "For best viewing ensure your screen width is at least %d characters", (HELPLINEEND + 10));
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", "1) Enclosure List");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","          returns List of Enclosure Ids, i.e sgxx");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","2) Enclosure sgxx Children ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","     returns List of Subcomponents (temp, fans etc.)");
	FormatHelpString(formattedstring, locstr);
	
	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","3) Enclosure ID driveSlot Status");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","      returns Status for given disk (e.g Enclosure sg4 1 Status )");
	FormatHelpString(formattedstring, locstr);

	this->_help = formattedstring;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "EnclProcessorXmlrpc::EnclProcessorXmlrpc, Exit");
}


/**
 * Function execute()
 */
void EnclProcessorXmlrpc::execute(xmlrpc_c::paramList const& paramList,
								  xmlrpc_c::value * const retvalP)
{
	if(paramList.size() == 1){
		string param1 = paramList.getString(0);

		/* if parameter is "List", list the enclosure IDs*/
		if( param1.compare("List") == 0 ) {
			
			getEnclIds( *retvalP);
		}
		/* else, get the enclosure details with given ID */
		else if( !param1.empty() ){
			getEncl( param1, *retvalP );
		}
		else{
			throw( xmlrpc_c::fault("Invalid input parameter",
								   xmlrpc_c::fault::CODE_UNSPECIFIED));
			
		}
	}
	else if( paramList.size() == 2 && paramList.getString(0).compare("List")){
		string param1 = paramList.getString(0);
		string param2 = paramList.getString(1);
		
		if( !param1.empty() && !param2.compare("Children") ){
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessorXmlrpc.cpp : Builidng Enclosure %s Children list", param1.c_str());

			getEnclChildren( param1, *retvalP );
		}
		else{

			throw( xmlrpc_c::fault("Invalid input parameter(s)",
								   xmlrpc_c::fault::CODE_UNSPECIFIED));
		}
	}
	//xmlrpc Enclosure sgName bayNum Status
	else if (paramList.size() == 3)  {
		string param1 = paramList.getString(0);  // sgname
		string param2 = paramList.getString(1);  // baynum
		string param3 = paramList.getString(2);  // "Status"

		if ((!param1.empty()) && (!param2.empty()) && (!param3.empty()) &&
			((strcasecmp(param3.c_str(), "Status") == 0))) {

			getEnclsgstatus(param1, param2, *retvalP);
		} else{

			throw( xmlrpc_c::fault("Invalid input parameter(s)",
								   xmlrpc_c::fault::CODE_UNSPECIFIED));
		}
	}
	else{
		throw( xmlrpc_c::fault("Invalid number of parameters, method takes one or two parameters.",
							   xmlrpc_c::fault::CODE_UNSPECIFIED));
	}
}



/**
 * Function getEnclIds
 *
 * get the of the enclosure IDs and return them in an xmlrpc_c::value.
 *
 * @param[out] result the xmlrpc_c::value saves the list of enclosure
 *                    ids.
 */ 
void
EnclProcessorXmlrpc::getEnclIds( xmlrpc_c::value &result )
{
	vector<xmlrpc_c::value> idarray;

	vector<Enclosure *>::iterator it;
	for( it = encls.begin(); it != encls.end(); it ++ ){
		idarray.push_back(xmlrpc_c::value_string((*it)->getId()));
	}

	xmlrpc_c::value_array ids(idarray);
	result = ids;
}


/**
 * Function getEncl
 *
 * Function that get the information of an enclosure by giving the sgname.
 *
 * @param[in] sgname, the SEAC_ID of the enclosure.
 * @param[out] result, the xmlrpc_c::value that saves the enclosure information.
 */

void
EnclProcessorXmlrpc::getEncl(string seacid, xmlrpc_c::value &result)
{
	vector<Enclosure *>::iterator it;
	map<string, xmlrpc_c::value> enclMap;

	/**
	 * Find the enclosure that has the sgname.
	 */
	for( it = encls.begin(); it != encls.end(); it ++){
		if( !((*it)->getId().compare(seacid) ) ){

//			(*it)->updateEnclinfo();

			// add the basic information
			
			enclMap.insert(make_pair("SEAC_ID",
									 xmlrpc_c::value_string(seacid) ));
			enclMap.insert(make_pair("SAS Address",
									 xmlrpc_c::value_string((*it)->getSasaddr())));
			enclMap.insert(make_pair("SCSI Address",
									 xmlrpc_c::value_string((*it)->getScsiaddr())));
			enclMap.insert(make_pair("Manufacturer",
									 xmlrpc_c::value_string((*it)->getVendor())));
			enclMap.insert(make_pair("Product ID",
									 xmlrpc_c::value_string((*it)->getProdid())));
			enclMap.insert(make_pair("Revision",
									 xmlrpc_c::value_string((*it)->getRevision())));
			enclMap.insert(make_pair("Firmware Version",
									 xmlrpc_c::value_string((*it)->getFwversion())));
			enclMap.insert(make_pair("Status",
									 xmlrpc_c::value_string((*it)->getStatus())));
			enclMap.insert(make_pair("Number of Drive Bays",
									 xmlrpc_c::value_int((*it)->getNumofbays())));
			enclMap.insert(make_pair("Unit Serial Number",
									 xmlrpc_c::value_string((*it)->getUnitsn())));
//			(*it)->updatePhys();
			vector<Enclosure::Phy> local = (*it)->getPhys();
	 		unsigned int numPhys = local.size();
			
 			if ( numPhys > 0 ){
 			traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessorXmlrpc.cpp::getEncl(): Builidng Fan response. number of Phys %d", numPhys);
 				vector<xmlrpc_c::value> phyarray;
				
				unsigned int i;
 				for ( i = 0; i < numPhys; i ++ ){
 					map<string, xmlrpc_c::value> phymap;
					phymap.insert(make_pair("Phy Descriptor",
											xmlrpc_c::value_string( local[i].desc )));
					phymap.insert(make_pair("Phy Number",
											xmlrpc_c::value_string( local[i].phynumber )));
					phymap.insert(make_pair("Attached Address",
											xmlrpc_c::value_string( local[i].sas )));
					phymap.insert(make_pair("Link Rate",
											xmlrpc_c::value_string( local[i].linkrate )));
					phymap.insert(make_pair("Invalid Dword Count",
											xmlrpc_c::value_int( local[i].invDword )));
					phymap.insert(make_pair("Running Disparity Error Count",
											xmlrpc_c::value_int( local[i].disparty )));
					phymap.insert(make_pair("Loss of Dword Sync Count",
											xmlrpc_c::value_int( local[i].syncLoss )));
					phymap.insert(make_pair("Phy Reset Problem Count",
											xmlrpc_c::value_int( local[i].rsetProb )));
					
 					xmlrpc_c::value_struct const phystruct(phymap);
 					phyarray.push_back(phystruct);
 				}
				
 				enclMap.insert(make_pair("Phys",
 										 xmlrpc_c::value_array(phyarray)));
 			}
		}
	}

	xmlrpc_c::value_struct enclStruct( enclMap );
	result = enclStruct;
}


/**
 * Function getEnclChildren
 *
 * Function that returns the children scsi disks attached to the enclosure
 * 
 * @param[in] pciaddr the SEAC_ID of the enclusre
 * @param[out] result the xmlrpc_c::value that saves the scsi disks infomation.
 * 
 */
void
EnclProcessorXmlrpc::getEnclChildren( string seacid, xmlrpc_c::value &result)
{
	
	vector<Enclosure *>::iterator it;
	/**
	 * Find the enclosure that has the sgname.
	 */
	for( it = encls.begin(); it != encls.end(); it ++){
		if( !((*it)->getId().compare(seacid) ) ){

			map<string, xmlrpc_c::value> childMap;
			// add the fans
			(*it)->updateFans();
			vector<Enclosure::Envelemt> localFans = (*it)->getFans();
	 		unsigned int numFans = localFans.size();
 			traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessorXmlrpc.cpp::getEnclChildren():Builidng Fan List. number of Fans %d", numFans);
			
 			if ( numFans > 0 ){
 				vector<xmlrpc_c::value> fanarray;
				
 				unsigned int i;
 				for ( i = 0; i < numFans; i ++ ){
 					map<string, xmlrpc_c::value> fanmap;
					fanmap.insert(make_pair("SEAC_ID",
											xmlrpc_c::value_string( localFans[i].seacid )));
					fanmap.insert(make_pair("Descriptor",
											xmlrpc_c::value_string( localFans[i].desc )));
					fanmap.insert(make_pair("Status",
											xmlrpc_c::value_string( localFans[i].status )));
					fanmap.insert(make_pair("Current Speed",
											xmlrpc_c::value_string( localFans[i].reading )));
 					xmlrpc_c::value_struct const fanstruct(fanmap);
 					fanarray.push_back(fanstruct);
 				}
				
 				childMap.insert(make_pair("Cooling Fans",
 										 xmlrpc_c::value_array(fanarray)));
 			}
			
			// add the power supplies
			(*it)->updatePowers();
			vector<Enclosure::Envelemt> localpowers = (*it)->getPowers();
	 		unsigned int numpowers = localpowers.size();
 			traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessorXmlrpc.cpp::getEnclChildren():Builidng Power list. number of Powers %d", numpowers);
			
 			if ( numpowers > 0 ){
 				vector<xmlrpc_c::value> powerarray;
				
 				unsigned int i;
 				for ( i = 0; i < numpowers; i ++ ){
 					map<string, xmlrpc_c::value> powermap;
					powermap.insert(make_pair("SEAC_ID",
											xmlrpc_c::value_string( localpowers[i].seacid )));
					powermap.insert(make_pair("Descriptor",
											xmlrpc_c::value_string( localpowers[i].desc )));
					powermap.insert(make_pair("Status",
											xmlrpc_c::value_string( localpowers[i].status )));
 					xmlrpc_c::value_struct const powerstruct(powermap);
 					powerarray.push_back(powerstruct);
 				}
				
 				childMap.insert(make_pair("Power Supplies",
 										 xmlrpc_c::value_array(powerarray)));
 			}

			
			// add the temperature sensors
			(*it)->updateTemps();
			vector<Enclosure::Envelemt> localtemps = (*it)->getTemps();
	 		unsigned int numtemps = localtemps.size();
 			traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessorXmlrpc.cpp::getEnclChildren():Builidng Temps list. number of temperature sensors %d", numtemps);
			
 			if ( numtemps > 0 ){
 				vector<xmlrpc_c::value> temparray;
				
 				unsigned int i;
 				for ( i = 0; i < numtemps; i ++ ){
 					map<string, xmlrpc_c::value> tempmap;
					tempmap.insert(make_pair("SEAC_ID",
											xmlrpc_c::value_string( localtemps[i].seacid )));
					tempmap.insert(make_pair("Descriptor",
											xmlrpc_c::value_string( localtemps[i].desc )));
					tempmap.insert(make_pair("Status",
											xmlrpc_c::value_string( localtemps[i].status )));
					tempmap.insert(make_pair("Temperature",
											xmlrpc_c::value_string( localtemps[i].reading )));
					xmlrpc_c::value_struct const tempstruct(tempmap);
 					temparray.push_back(tempstruct);
 				}
				
 				childMap.insert(make_pair("Temperature Sensors",
 										 xmlrpc_c::value_array(temparray)));
 			}


			//(*it)->updateDisklist();
			vector<xmlrpc_c::value> cearray;
			unsigned int ceidx;
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessorXmlrpc.cpp::getEnclChildren():Builidng Disks list.");

			for(ceidx = 0; ceidx < (*it)->getDisks().size(); ceidx++ ){
				map<string, xmlrpc_c::value> diskmap;
				string seac_id = (*it)->getDisks()[ceidx].seacid;
				string bd_name = (*it)->getDisks()[ceidx].bdname;
				seac_id.append("(");
				seac_id.append(bd_name);
				seac_id.append(")");
				
				
				diskmap.insert(make_pair("SEAC_ID",
										 xmlrpc_c::value_string(seac_id)));
				diskmap.insert(make_pair("Bay Number",
										 xmlrpc_c::value_string((*it)->getDisks()[ceidx].baynumber)));
				diskmap.insert(make_pair("Status",
										 xmlrpc_c::value_string((*it)->getDisks()[ceidx].status)));
				diskmap.insert(make_pair("Sgname",
										 xmlrpc_c::value_string((*it)->getDisks()[ceidx].sgname)));
			
				xmlrpc_c::value_struct const diskstruct(diskmap);
				cearray.push_back(diskstruct);
			}
			childMap.insert(make_pair("Disk Drives",
									  xmlrpc_c::value_array(cearray)));
			
			xmlrpc_c::value_struct childStruct( childMap );
			result = childStruct;
		}
	}
}

/**
 * Function getEnclsgstatus
 *
 * Function that returns the status of sgname baynum attached to the enclosure
 * 
 * @param[in] sgname = /dev/sgxx
 * @param[in] baynum -- same as drive slot num
 * @param[out] result the xmlrpc_c::value that saves the status struct.
 * 
 */
void
EnclProcessorXmlrpc::getEnclsgstatus( string sgname, string baynum, xmlrpc_c::value &result)
{
	char cmd[BUFF512];
	char locbuf[BUFF512];
	FILE *fd;
	char *testfgets;
	map<string, xmlrpc_c::value> statusmap;

	string overallstat;
	string disabled;
	string faultsensed;
	string ident;
	string predfailure;
	string swap;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "EnclProcessorXmlrpc::getEnclsgstatus():Enter");

	//
	// status:OK Disabled=0 Fault sensed=0 Ident=0 Predicted Failure=0 Swap=0
	// Note: baynum is same as drive num; terminlogy issue
	//
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "%s --show --drive %s --dev %s | grep  '   %s:' ",
		MATRIXUTILITY, baynum.c_str(), sgname.c_str(), baynum.c_str());

	if ((fd = (FILE *)popen(cmd,"r")) != NULL) {
		testfgets = fgets(locbuf, BUFF512, fd);
		pclose(fd);

		if (testfgets != NULL) {
			//
			// multi status values
			//
			if (strcasestr(locbuf, "OK") != NULL) {
				overallstat.assign("OK");
			} else if (strcasestr(locbuf, "Critical") != NULL) {
				overallstat.assign("Critical");
			} else if (strcasestr(locbuf, "Noncritical") != NULL) {
				overallstat.assign("Noncritical");
			}
			else if (strcasestr(locbuf, "Unrecoverable") != NULL) {
				overallstat.assign("Unrecoverable");
			} else if (strcasestr(locbuf, "Not Installed") != NULL) {
				overallstat.assign("Not Installed");
			}
			else if (strcasestr(locbuf, "Unrecoverable") != NULL) {
				overallstat.assign("Unrecoverable");
			} else if (strcasestr(locbuf, "Unsupported") != NULL) {
				overallstat.assign("Unsupported");
			} else if (strcasestr(locbuf, "Unknown") != NULL) {
				overallstat.assign("Unknown");
			} else {
				overallstat.assign("Unavailable");
			}

			//
			// ses bits
			//
			if (strcasestr(locbuf, "[ReqstIdent]") != NULL) {
				ident.assign("1");
			} else {
				ident.assign("0");
			}

			if (strcasestr(locbuf, "[ReqstFault]") != NULL) {
				faultsensed.assign("1");
			} else {
				faultsensed.assign("0");
			}

			if (strcasestr(locbuf, "[Swap]") != NULL) {
				swap.assign("1");
			} else {
				swap.assign("0");
			}

			if (strcasestr(locbuf, "[Disabled]") != NULL) {
				disabled.assign("1");
			} else {
				disabled.assign("0");
			}

			if (strcasestr(locbuf, "[PrdFailed]") != NULL) {
				predfailure.assign("1");
			} else {
				predfailure.assign("0");
			}
		}
	}

	statusmap.insert(make_pair("Status", xmlrpc_c::value_string(overallstat)));
	statusmap.insert(make_pair("Disabled", xmlrpc_c::value_string(disabled)));
	statusmap.insert(make_pair("Fault sensed", xmlrpc_c::value_string(faultsensed)));
	statusmap.insert(make_pair("Ident", xmlrpc_c::value_string(ident)));
	statusmap.insert(make_pair("Predicted Failure", xmlrpc_c::value_string(predfailure)));
	statusmap.insert(make_pair("Swap", xmlrpc_c::value_string(swap)));


	xmlrpc_c::value_struct rtnStruct(statusmap);
	result = rtnStruct;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "EnclProcessorXmlrpc::getEnclsgstatus():Exit");
}
