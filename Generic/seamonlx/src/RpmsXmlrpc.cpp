/** @File RpmsXmlrpc.cpp
 *
 *
 *  SystemRpmsXmlrpc class constructors and member functions
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
#include "RpmsXmlrpc.h"
#include "common.h"

extern RPM_NAME RPMNameArray[MAX_RPM_ALLOWED];

using namespace std;

/**
 * default constructor
 */

SystemRpmsXmlrpc::SystemRpmsXmlrpc()
{
	char	locstr[1024];
	string	formattedstring;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemRpmsXmlrpc::SystemRpmsXmlrpc, Enter");
	this->_signature = "A:s" ;

	sprintf(locstr, "For best viewing ensure your screen width is at least %d characters", (HELPLINEEND + 10));
	FormatHelpString(formattedstring, locstr);
	
	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s", "SystemRpms Rpms");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","     As is, no additional params, returns list of critcal Rpms");
	FormatHelpString(formattedstring, locstr);

	this->_help = formattedstring;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemRpmsXmlrpc::SystemRpmsXmlrpc, Exit");
}

/**
 * Function execute() basically a dispatch for type of XML RPC Rpms request
 * 
 */

void 
SystemRpmsXmlrpc::execute(xmlrpc_c::paramList const& paramList,
				xmlrpc_c::value *   const retvalP){

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemRpmsXmlrpc::execute, Enter");
	param1 = paramList.getString(0);		// Rpms									
	if (param1.compare("Rpms") == 0) {
		buildRpmsResp(*retvalP);
	} else{
		throw(xmlrpc_c::fault("Information specified by parameter not supported", xmlrpc_c::fault::CODE_UNSPECIFIED));
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemRpmsXmlrpc::execute, Exit");
}


/**
 * SystemRpmsXmlrpc::buildRpmsResp()            
 *
 * Private method to get the info of bios and build
 * the data member biosResp.
 * 
 */

void
SystemRpmsXmlrpc::buildRpmsResp(xmlrpc_c::value &result)
{
	vector<xmlrpc_c::value>		elements;
	char						tmpstr[BUFF256];
	string						locstr;
	int							i = 0;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemRpmsXmlrpc::buildRpmsResp, Enter");
	while (RPMNameArray[i].name[0] != '\0') {
		map<string, xmlrpc_c::value> rsp; 

		sprintf(tmpstr,"%s", RPMNameArray[i].name);				
		locstr = tmpstr;
		rsp.insert(make_pair("Name", xmlrpc_c::value_string(locstr)));

		sprintf(tmpstr,"%s", RPMNameArray[i].summary);
		locstr = tmpstr;
		rsp.insert(make_pair("Summary", xmlrpc_c::value_string(locstr)));

		elements.push_back(xmlrpc_c::value_struct( rsp )); 
			
		i++;
	}

	//
	// Make it an array and get it activated for the return result
	//
	xmlrpc_c::value_array param(elements);
	result = param;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemRpmsXmlrpc::buildRpmsResp, Exit");
}
