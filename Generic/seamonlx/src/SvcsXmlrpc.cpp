/** @File SvcsXmlrpc.cpp
 *
 *
 *  SystemSvcsXmlrpc class constructors and member functions
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
#include "SvcsXmlrpc.h"
#include "common.h"

extern SERVICE_NAME ServiceNameArray[MAX_SVC_ALLOWED];

using namespace std;

/**
 * default constructor
 */

SystemSvcsXmlrpc::SystemSvcsXmlrpc()
{
	char	locstr[1024];
	string	formattedstring;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemSvcsXmlrpc::SystemSvcsXmlrpc, Enter");

	this->_signature = "A:s" ;

	sprintf(locstr, "For best viewing ensure your screen width is at least %d characters", (HELPLINEEND + 10));
	FormatHelpString(formattedstring, locstr);
	
	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s", "SystemSvcs Svcs");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","     As is, no additional params, returns list of critcal Svs");
	FormatHelpString(formattedstring, locstr);

	this->_help = formattedstring;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemSvcsXmlrpc::SystemSvcsXmlrpc, Exit");
}

/**
 * Function execute() basically a dispatch for type of XML RPC Svcs request
 * 
 */

void 
SystemSvcsXmlrpc::execute(xmlrpc_c::paramList const& paramList,
				xmlrpc_c::value *   const retvalP){

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemSvcsXmlrpc::execute, Enter");
	param1 = paramList.getString(0);		// Svcs									
	if (param1.compare("Svcs") == 0) {
		buildSvcsResp(*retvalP);
	} else{
		throw(xmlrpc_c::fault("Information specified by parameter not supported", xmlrpc_c::fault::CODE_UNSPECIFIED));
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemSvcsXmlrpc::execute, Exit");
}


/**
 * SystemSvcsXmlrpc::buildSvcsResp()            
 *
 * Private method to get the info of bios and build
 * the data member biosResp.
 * 
 */

void
SystemSvcsXmlrpc::buildSvcsResp(xmlrpc_c::value &result)
{
	int							i = 0;
	char						tmpstr[BUFF256];
	string						locstr;

	vector<xmlrpc_c::value> elements;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemSvcsXmlrpc::buildSvcsResp, Enter");
	while (ServiceNameArray[i].name[0] != '\0') {		
		sprintf(tmpstr,"%s", ServiceNameArray[i].name);
		locstr = tmpstr;
		elements.push_back(xmlrpc_c::value_string( locstr )); 
		i++;
	}

	//
	// Make it an array and get it activated for the return result
	//
	xmlrpc_c::value_array param(elements);
	result = param;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemSvcsXmlrpc::buildSvcsResp, Enter");
}
