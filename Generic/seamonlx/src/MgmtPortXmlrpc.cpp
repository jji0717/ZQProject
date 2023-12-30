/* @File MgmtPortXmlrpc.cpp
 *
 *
 *  MgmtPort Object  class constructors and member functions
 *  implementation.
 *  
 *
 *
 *  Revision History
 *  
 *  04-19-2010 mjc  Created
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
#include "MgmtPortXmlrpc.h"
#include "common.h"

using namespace std;
using namespace seamonlx;

/**
 * default constructor
 */
MgmtPortXmlrpc::MgmtPortXmlrpc()
{
	this->_signature = "A:s";
	this->_help = "This method returns Management Port health information.";
	
}

/**
 * Function execute()
 * 
 */

void 
MgmtPortXmlrpc::execute(xmlrpc_c::paramList const& paramList,
				xmlrpc_c::value *   const  retvalP)
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MgmtPortXmlrpc::execute, Enter");

	paramList.verifyEnd(1);
	string param1 =  paramList.getString(0);
	
	if( &retvalP != NULL ) ;  // Just to get rid of the warning
	
    traceClass->LogTrace(ZQ::common::Log::L_INFO, "MgmtPortXmlrpc::param1 compare string is |%s|, expecting 'OverallHealth'", param1.c_str() );

    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MgmtPortXmlrpc::execute, Exit");
}

/**
 * MgmtPortXmlrpc::buildMgmtPortResp()            
 *
 * Private method to get the info for the Mgmt Port health
 * 
 */

void
MgmtPortXmlrpc::buildMgmtPortResp( xmlrpc_c::value &result )
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MgmtPortXmlrpc::buildMgmtPortResp, Enter");

	if( &result != NULL ) ;  // Remove warning
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MgmtPortXmlrpc::buildMgmtPortResp, Exit");
}


/* End MgmtPortXmlrpc.cpp */
