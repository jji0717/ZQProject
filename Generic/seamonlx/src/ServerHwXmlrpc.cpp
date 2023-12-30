/** @File ServerHwXmlrpc.cpp
 *
 *
 *  ServerObj class constructors and member functions
 *  implementation.
 *  
 *
 *
 *  Revision History
 *  
 *  03-10-2010 Created ( jie.zhang@schange.com)
 *  06-14-2010  mjc  Fixed memory leak: 	Do not call result.instantiate(), rather use result = finalResp;
 *  06-16-2010  mjc  Fixed SMIS-102; Do not re-create data after first instantiation, just re-use it. Added Traces.
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
#include "ServerHwXmlrpc.h"
#include "common.h"

using namespace std;
using namespace seamonlx;


/**
 * default constructor
 */

ServerHwXmlrpc::ServerHwXmlrpc()
{
	char	locstr[1024];
	string	formattedstring;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "ServerHwXmlrpc::ServerHwXmlrpc, Enter");

	this->_signature = "A:s" ;

	sprintf(locstr, "For best viewing ensure your screen width is at least %d characters", (HELPLINEEND + 10));
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s", "Server [Chassis, Baseboard, Bios, Processor, Memory]");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","1) Server Chassis");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","          returns Chassis data");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","2) Server Baseboard");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","          returns Baseboard data");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","3) Server Bios");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","          returns Bios data");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","4) Server Memory");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","          returns array of memory data");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","4) Server Processor");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","          returns array of Processor data");
	FormatHelpString(formattedstring, locstr);

	this->_help = formattedstring;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "ServerHwXmlrpc::ServerHwXmlrpc, Enter");

}




/**
 * Function execute()
 * 
 */

void 
ServerHwXmlrpc::execute(xmlrpc_c::paramList const& paramList,
				xmlrpc_c::value *   const  retvalP){

	paramList.verifyEnd(1);
	string param1 =  paramList.getString(0);
	xmlrpc_c::value			tempVal;
	
	
	/* parse the parameter */
	if( param1.compare("Chassis") == 0 ){

		tempVal = getChassisResp();
		
		if( !tempVal.isInstantiated() ) {		// Only do work if first tme in
			updateChassis(); 
			buildChassisResp();
		}
 		*retvalP = getChassisResp();	
 	}
	
	else if ( param1.compare("Baseboard") == 0 ){

		tempVal = getBaseboardResp();

		if( !tempVal.isInstantiated() ) {		
			updateBaseboard();
			buildBaseboardResp();
		}
		*retvalP = getBaseboardResp();
	}
	
	else if ( param1.compare("Bios") == 0 ){

	tempVal = getBiosResp();
		
		if( !tempVal.isInstantiated() ) {		
			updateBios(); 
			buildBiosResp();
		}
		*retvalP = getBiosResp();
	}
	
	else if ( param1.compare("Processor") == 0 ){
		tempVal = getProcessorResp();
		
		if( !tempVal.isInstantiated() ) {		
			updateProcessor();
			buildProcResp();
		}
		*retvalP = getProcessorResp();
	}
	
 	else if ( param1.compare("Memory") == 0 ){

		tempVal = getMemoryResp();
		
		if( !tempVal.isInstantiated() ) {
			updateMemory();
			buildMemoryResp();
		}
 		*retvalP = getMemoryResp();	
 	}
		
	else{
		
		throw(xmlrpc_c::fault("Information specified by parameter not supported",
							  xmlrpc_c::fault::CODE_UNSPECIFIED));
	}
	
}




/**
 * ServerHwXmlrpc::buildChassisResp()            
 *
 * Private method to get the info of chassis and build
 * the data member chassisResp.
 * 
 */

void
ServerHwXmlrpc::buildChassisResp()
{
	buildMapResp( getChassis(), chassisResp );
}




/**
 * ServerHwXmlrpc::buildBaseboardResp()            
 *
 * Private method to get the info of baseboard and build
 * the data member baseboardResp.
 * 
 */

void
ServerHwXmlrpc::buildBaseboardResp()
{
	buildMapResp( getBaseboard(), baseboardResp );
}



/**
 * ServerHwXmlrpc::buildBiosResp()            
 *
 * Private method to get the info of bios and build
 * the data member biosResp.
 * 
 */

void
ServerHwXmlrpc::buildBiosResp()
{
	buildMapResp( getBios(), biosResp );
}




/**
 * ServerHwXmlrpc::buildProcResp()            
 *
 * Private method to get the info from processor and build
 * the data member procResp.
 * 
 */

void
ServerHwXmlrpc::buildProcResp()
{
	vector<xmlrpc_c::value> procs;
	vector<map<string, string> >::iterator it;

	ElemList processorcp = getProcessor();
	for(it = processorcp.begin(); it != processorcp.end(); it++){
		
		xmlrpc_c::value procelem;
		buildMapResp(*it, procelem);
		procs.push_back(procelem);
	}

	xmlrpc_c::value_array param(procs);
	processorResp = param;
}

/**
 * ServerHwXmlrpc::buildMemoryResp()            
 *
 * Private method to get the info of memory and build
 * the data member memoryResp.
 * 
 */

void
ServerHwXmlrpc::buildMemoryResp()
{
	vector<xmlrpc_c::value> memList;
	
	vector<Memory>::iterator memit;
	
	vector<Memory> memorycp = getMemory();
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "ServerHwXmlrpc::buildMemoryResp - Enter. Sizes memorycp=%d", (int)memorycp.size() );
	
	for(memit = memorycp.begin(); memit != memorycp.end(); memit++ ){
		xmlrpc_c::value memelem;

		/* get the physical memory array xmlrpc value */
		buildMapResp(memit->marray, memelem);

		/* build the memory device array in value_array format */
		vector<xmlrpc_c::value> devs;
		vector<map<string, string> > devlist( memit->dimms );
		vector<map<string, string> >::iterator it;
		for(it = devlist.begin(); it != devlist.end(); it++ ){
			xmlrpc_c::value device;
			buildMapResp(*it, device);
			devs.push_back(device);
		}
		xmlrpc_c::value_array devarray(devs);

		/* build the pair of memory devices */
		pair<string, xmlrpc_c::value> devices("Memory Devices",
											  devarray);

		/* insert the memory device pair to the memlry element map */
		map<string, xmlrpc_c::value>  memorymap =
			static_cast<map<string, xmlrpc_c::value> >(
				xmlrpc_c::value_struct(memelem));
		memorymap.insert(devices);

		/* Add the memory element struct to the memList*/
		xmlrpc_c::value_struct memoryelement(memorymap);
		memList.push_back(memoryelement);
	}
	
	xmlrpc_c::value_array param(memList);
	memoryResp = param;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "ServerHwXmlrpc::buildMemoryResp - Exit. Sizes memoryList=%d", (int)memList.size() );
	
}


/**
 * ServerHwXmlrpc::buildMapResp()            
 *
 * Private method to  build the xmlrpc struct response from
 * a map data member.
 *
 * @param[IN] in the input map
 * @param[OUT] result reference to the result struct.
 * 
 */

void
ServerHwXmlrpc::buildMapResp(map<string, string> in, xmlrpc_c::value &result)
{
	map<string, xmlrpc_c::value> retmap;	
	map<string, string>::iterator it;	

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "ServerHwXmlrpc::buildMapResp - Enter." );
	
	/**
	 * iterate through the ret map and convert each
	 * pair to type pair<string, xmlrpc_c::value>.
	 */
	for(it = in.begin(); it != in.end(); it ++){
		string key = it->first;
		string value = it->second;

		/**
		 * store the value to a vector if value is a string of
		 * vseperator seperated values.
		 */
		if(value.find(vseperator, 0) != string::npos){
			/**
			 * remove the last vseperator.
			 */
			value.substr(0, value.length()-vseperator.length());
			
			vector<string> array;
			stringSplit(value, vseperator, array);

			vector<xmlrpc_c::value> varray;
			vector<string>::iterator vit;
			for(vit = array.begin(); vit != array.end(); vit++){
				varray.push_back(xmlrpc_c::value_string(*vit));
			}
			
			pair<string, xmlrpc_c::value> member(
				key, xmlrpc_c::value_array(varray));
			retmap.insert(member);
			
		}
		else{
			
			pair<string, xmlrpc_c::value> member(
				key, xmlrpc_c::value_string(value));
			retmap.insert(member);
			
		}
		
	}
	
	xmlrpc_c::value_struct param(retmap);
	result = param;
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "ServerHwXmlrpc::buildMapResp - Exit. Sizes   retmap=%d", 	(int)retmap.size() );
	
}

