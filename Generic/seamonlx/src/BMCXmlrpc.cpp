/** @File BMCXmlrpc.cpp
 *
 *
 *  SystemBMCXmlrpc class constructors and member functions
 *  implementation.
 *  
 *
 *
 *  Revision History
 *  
 *  06-08-2010 Created ( khemphill@schange.com)
 *  06-14-2010  mjc  Fixed memory leak: 	Do not call result.instantiate(), rather use result = finalResp;
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
#include "BMCXmlrpc.h"
#include "common.h"

extern BMC_STRUCT BmcStructArray[2];

using namespace std;

/**
 * default constructor
 */

SystemBMCXmlrpc::SystemBMCXmlrpc()
{
	char	locstr[1024];
	string	formattedstring;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemBMCXmlrpc::SystemBMCXmlrpc, Enter");

	this->_signature = "A:s" ;

	sprintf(locstr, "For best viewing ensure your screen width is at least %d characters", (HELPLINEEND + 10));
	FormatHelpString(formattedstring, locstr);
	
	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s", "SystemBMC BMC");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","     As is, no additional params, returns BMC data");
	FormatHelpString(formattedstring, locstr);

	this->_help = formattedstring;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemBMCXmlrpc::SystemBMCXmlrpc, Exit");
}

/**
 * Function execute() basically a dispatch for type of XML RPC BMC request
 * 
 */

void 
SystemBMCXmlrpc::execute(xmlrpc_c::paramList const& paramList,
				xmlrpc_c::value *   const retvalP){

	param1 = paramList.getString(0);		// BMC									
	if (param1.compare("BMC") == 0) {
		buildBMCResp(*retvalP);
	} else{
		throw(xmlrpc_c::fault("Information specified by parameter not supported", xmlrpc_c::fault::CODE_UNSPECIFIED));
	}
}


/**
 * SystemBMCXmlrpc::buildBMCResp()            
 *
 * Private method to get the info of bios and build
 * the data member biosResp.
 * 
 */

void
SystemBMCXmlrpc::buildBMCResp(xmlrpc_c::value &result)
{
	vector<xmlrpc_c::value> elements;

	char						tmpstr[BUFF80];
	string						locstr;
	int							i = 0;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemBMCXmlrpc::buildBMCResp, Enter");

	for (i = 0; i < 2; i++) {
		map<string, xmlrpc_c::value> rsp; 

		sprintf(tmpstr,"%s", BmcStructArray[i].IPMIvers);				
		locstr = tmpstr;
		rsp.insert(make_pair("IPMI Version", xmlrpc_c::value_string(locstr)));

		sprintf(tmpstr,"%s", BmcStructArray[i].FirmwareVers); 
		locstr = tmpstr;
		rsp.insert(make_pair("Firmware Version", xmlrpc_c::value_string(locstr)));

		sprintf(tmpstr,"%s", BmcStructArray[i].WatchdogTimerStatus);  
		locstr = tmpstr;
		rsp.insert(make_pair("IPMI Watchdog Timer Status", xmlrpc_c::value_string(locstr)));

		sprintf(tmpstr,"%s", BmcStructArray[i].WatchdogTimerInterval); 
		locstr = tmpstr;
		rsp.insert(make_pair("IPMI Watchdog Timer Interval", xmlrpc_c::value_string(locstr)));

		sprintf(tmpstr,"%s", BmcStructArray[i].PrimaryNodeType);  
		locstr = tmpstr;
		rsp.insert(make_pair("Primary Node", xmlrpc_c::value_string(locstr)));

		sprintf(tmpstr,"%s", BmcStructArray[i].BMCIpAddr); 
		locstr = tmpstr;
		rsp.insert(make_pair("BMC IP Address", xmlrpc_c::value_string(locstr)));

		sprintf(tmpstr,"%s", BmcStructArray[i].IPAddrSource); 
		locstr = tmpstr;
		rsp.insert(make_pair("BMC IP Address Source", xmlrpc_c::value_string(locstr)));

		sprintf(tmpstr,"%s", BmcStructArray[i].DefGatewayAddr);  
		locstr = tmpstr;
		rsp.insert(make_pair("Gateway IP Address", xmlrpc_c::value_string(locstr)));

		sprintf(tmpstr,"%s", BmcStructArray[i].UserName); 
		locstr = tmpstr;
		rsp.insert(make_pair("Username", xmlrpc_c::value_string(locstr)));
		
		/* don't send pw across wire
		sprintf(tmpstr,"%s", BmcStructArray[i].Password);  
		locstr = tmpstr;
		rsp.insert(make_pair("Password", xmlrpc_c::value_string(locstr)));
		*/

		sprintf(tmpstr,"%s", BmcStructArray[i].Status); 
		locstr = tmpstr;
		rsp.insert(make_pair("Status", xmlrpc_c::value_string(locstr)));

		elements.push_back(xmlrpc_c::value_struct( rsp )); 
	}

	xmlrpc_c::value_array param(elements);
	result = param;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemBMCXmlrpc::buildBMCResp, Exit");
}

