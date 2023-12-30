/** @File ConfigXmlrpc.cpp
 *
 *
 *  SystemConfigXmlrpc class constructors and member functions
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
#include "ConfigXmlrpc.h"
#include "common.h"

extern CONFIG_STRUCT SysConfigData;
using namespace std;

/**
 * default constructor
 */

SystemConfigXmlrpc::SystemConfigXmlrpc()
{
	char	locstr[1024];
	string	formattedstring;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemConfigXmlrpc::SystemConfigXmlrpc, Enter");
	this->_signature = "A:s" ;

	sprintf(locstr, "For best viewing ensure your screen width is at least %d characters", (HELPLINEEND + 10));
	FormatHelpString(formattedstring, locstr);
	
	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s", "SystemConfig Config");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","     As is, no additional params, returns System Configuration info");
	FormatHelpString(formattedstring, locstr);

	this->_help = formattedstring;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemConfigXmlrpc::SystemConfigXmlrpc, Exit");
}

/**
 * Function execute() basically a dispatch for type of XML RPC Config request
 * 
 */

void 
SystemConfigXmlrpc::execute(xmlrpc_c::paramList const& paramList,
				xmlrpc_c::value *   const retvalP){

	param1 = paramList.getString(0);		// Config									
	if (param1.compare("Config") == 0) {
		buildConfigResp(*retvalP);
	} else{
		throw(xmlrpc_c::fault("Information specified by parameter not supported", xmlrpc_c::fault::CODE_UNSPECIFIED));
	}
}


/**
 * SystemConfigXmlrpc::buildConfigResp()            
 *
 * Private method to get the info of bios and build
 * the data member biosResp.
 * 
 */

void
SystemConfigXmlrpc::buildConfigResp(xmlrpc_c::value &result)
{
	vector<xmlrpc_c::value> elements;
	map<string, xmlrpc_c::value> rsp;

	char						tmpstr[BUFF80];
	string						locstr;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemConfigXmlrpc::buildConfigResp, Enter");

	sprintf(tmpstr,"%s", SysConfigData.PartnerHostName);  // Hostname for partner node
	locstr = tmpstr;
	rsp.insert(make_pair("Partner Host Name", xmlrpc_c::value_string(locstr)));

	sprintf(tmpstr,"%s", SysConfigData.ProductType);  // really product type
	locstr = tmpstr;
	rsp.insert(make_pair("Product type", xmlrpc_c::value_string(locstr)));

	sprintf(tmpstr,"%s", SysConfigData.NodeType);  // node type
	locstr = tmpstr;
	rsp.insert(make_pair("Node type", xmlrpc_c::value_string(locstr)));

	sprintf(tmpstr,"%s", SysConfigData.Description); 
	locstr = tmpstr;
	rsp.insert(make_pair("Description", xmlrpc_c::value_string(locstr)));

	sprintf(tmpstr,"%s", SysConfigData.ManagementIf); 
	locstr = tmpstr;
	rsp.insert(make_pair("Management interface", xmlrpc_c::value_string(locstr)));

	sprintf(tmpstr,"%s", SysConfigData.BMCPrimaryIpAddr); 
	locstr = tmpstr;
	rsp.insert(make_pair("BMC primary Ip address", xmlrpc_c::value_string(locstr)));

	sprintf(tmpstr,"%s", SysConfigData.BMCSecondaryIpAddr);
	locstr = tmpstr;
	rsp.insert(make_pair("BMC secondary Ip address", xmlrpc_c::value_string(locstr)));

	sprintf(tmpstr,"%s", SysConfigData.BMCGatewayAddr); 
	locstr = tmpstr;
	rsp.insert(make_pair("BMC gateway address", xmlrpc_c::value_string(locstr)));

	sprintf(tmpstr,"%s", SysConfigData.BMCUserName); 
	locstr = tmpstr;
	rsp.insert(make_pair("BMC username", xmlrpc_c::value_string(locstr)));

	sprintf(tmpstr,"%s", SysConfigData.BMCpassword); 
	locstr = tmpstr;
	rsp.insert(make_pair("BMC password", xmlrpc_c::value_string(locstr)));

	sprintf(tmpstr,"%s", SysConfigData.NICType); 
	locstr = tmpstr;
	rsp.insert(make_pair("NIC type", xmlrpc_c::value_string(locstr)));

	sprintf(tmpstr,"%s", SysConfigData.StorageType); 
	locstr = tmpstr;
	rsp.insert(make_pair("storage type", xmlrpc_c::value_string(locstr)));

	sprintf(tmpstr,"%s", SysConfigData.SystemType); 
	locstr = tmpstr;
	rsp.insert(make_pair("system type", xmlrpc_c::value_string(locstr)));

	xmlrpc_c::value_struct param(rsp);
	result = param;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemConfigXmlrpc::buildConfigResp, Exit");
}



