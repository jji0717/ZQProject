/* @File SystemHealthXmlrpc.cpp
 *
 *
 *  SystemHealth Object  class constructors and member functions
 *  implementation.
 *  
 *
 *
 *  Revision History
 *  
 *  04-12-2010 mjc  Created ()
 *  05-05-2010 mjc  Fixed status and value as they were reversed; removed getInfiniBand() from list as it is just a stub returning no data
 *  05-06-2010 mjc  Added the remaining system object stubs
 *  05-18-2010 mjc  Removed InfiniBand and added auto-increment variable to the list to make it dynamic
 *  05-19-2010 mjc  Fixed a faux pas with passing an incremeting argument to the EINSERT macro. 
 *  06-04-2010 mjc  Jira SMIS-90; Overall Health has StreamSmith component listed twice 
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
#include "SystemHealthXmlrpc.h"
#include "common.h"

using namespace std;
using namespace seamonlx;

#define EINSERT( elems, num ) \
	element[num].insert(make_pair( \
						   "component", \
						   xmlrpc_c::value_string(elems.seacid))); \
	element[num].insert(make_pair( \
						   "summary", \
						   xmlrpc_c::value_string(elems.status))); \
	element[num].insert(make_pair( \
						   "status", \
						   xmlrpc_c::value_string(elems.value))); \
	elements.push_back(xmlrpc_c::value_struct( element[num++] )); 

	
/**
 * default constructor
 */

SystemHealthXmlrpc::SystemHealthXmlrpc()
{
	char	locstr[1024];
	string	formattedstring;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemHealthXmlrpc::SystemHealthXmlrpc, Enter");

	this->_signature = "A:s" ;

	sprintf(locstr, "For best viewing ensure your screen width is at least %d characters", (HELPLINEEND + 10));
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", "SystemHealth OverallHealth");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","   returns List of major components and their OverallHealth");
	FormatHelpString(formattedstring, locstr);

	this->_help = formattedstring;
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemHealthXmlrpc::SystemHealthXmlrpc, Enter");

	pServerEnvObject = NULL;
	
}

/**
 * Function execute()
 * 
 */

void 
SystemHealthXmlrpc::execute(xmlrpc_c::paramList const& paramList,
				xmlrpc_c::value *   const  retvalP)
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemHealthXmlrpc::execute, Enter");

	paramList.verifyEnd(1);
	string param1 =  paramList.getString(0);
	
    //traceClass->LogTrace(ZQ::common::Log::L_INFO, "SystemHealthXmlrpc::param1 compare string is |%s|, expecting 'OverallHealth'", param1.c_str() );

    /* parse the parameter */
	if( param1.compare("OverallHealth") == 0 ){
		
    	// Update all of our persistent objects
	    updateAllHealth();
		buildHealthResp( *retvalP );
 	}
	else{
		
		throw(xmlrpc_c::fault("Information specified by parameter not supported",
							  xmlrpc_c::fault::CODE_UNSPECIFIED));
	}
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemHealthXmlrpc::execute, Exit");
}

/**
 * SystemHealthXmlrpc::updateAllHealth()            
 *
 * Private method to gets the health of all objects and stores each into
 * the persistent objects we have in our storage class.
 * 
 */
void
SystemHealthXmlrpc::updateAllHealth( void )
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemHealthXmlrpc:updateAllHealth, Enter");

	ServerEnv.seacid =  "Server Environmentals";
	ServerEnv.value  = getHealthStateString(getServerEnvHealth( ServerEnv.status ));
	
	ManagementPort.seacid  = "Management Port";
	ManagementPort.value    =  getHealthStateString(getMgmtPortHealth( ManagementPort.status ));

	TargetPorts.seacid  = "Target Ports";
	TargetPorts.value    =  getHealthStateString(getTargetPortsHealth( TargetPorts.status ));

	OpenIB.seacid  = "Cluster Interconnect - InfiniBand";
	OpenIB.value    =  getHealthStateString(getOpenIBHealth( OpenIB.status ));

	SeamonLX.seacid  = "SeamonLX";
	SeamonLX.value    =  getHealthStateString(getSeamonLXHealth( SeamonLX.status ));

	EnclosureEnv.seacid  = "Enclosure Environmentals";
	EnclosureEnv.value    =  getHealthStateString(getEnclosureEnvHealth(EnclosureEnv.status));

	StorageInterconnect.seacid  = "Storage Interconnect";
	StorageInterconnect.value    =  getHealthStateString(getStorageInterconnectHealth(StorageInterconnect.status));

	StorageConfiguration.seacid  = "Storage Configuration";
	StorageConfiguration.value    =  getHealthStateString(getStorageConfigurationHealth(StorageConfiguration.status));

	SHASState.seacid  = "SHAS State";
	SHASState.value    =  getHealthStateString(getSHASStateHealth( SHASState.status));

	HyperFS.seacid  = "Hyper FS";
	HyperFS.value    =  getHealthStateString(getHyperFSHealth(HyperFS.status ));

	IPStor.seacid  = "IP Stor";
	IPStor.value    =  getHealthStateString(getIPStoreHealth(IPStor.status));

	CIFS.seacid  = "CIFS";
	CIFS.value    =  getHealthStateString(getCIFSHealth(CIFS.status));

	FTP.seacid  = "FTP";
	FTP.value    =  getHealthStateString(getFTPHealth(FTP.status ));

	SoftwareConfiguration.seacid  = "Software Configuration";
	SoftwareConfiguration.value    =  getHealthStateString(getSoftwareConfigHealth(SoftwareConfiguration.status));

	SystemServices.seacid  = "System Services";
	SystemServices.value    =  getHealthStateString(getSystemServicesHealth(SystemServices.status));

	StreamSmith.seacid  = "StreamSmith";
	StreamSmith.value    =  getHealthStateString(getStreamSmithHealth(StreamSmith.status));

	VFlow.seacid  = "VFlow";
	VFlow.value    =  getHealthStateString(getVFlowHealth(VFlow.status));

	SeaFS.seacid  = "SeaFS";
	SeaFS.value    =  getHealthStateString(getSeaFSHealth(SeaFS.status));

	SparseCache.seacid  = "Sparse Cache";
	SparseCache.value    =  getHealthStateString(getSparseCacheHealth(SparseCache.status));

	DistributedCache.seacid  = "Distributed Cache";
	DistributedCache.value    =  getHealthStateString(getDistributedCacheHealth(DistributedCache.status));

	SentryService.seacid  = "Sentry Service";
	SentryService.value    =  getHealthStateString(getSentryServiceHealth(SentryService.status));

	C2Server.seacid  = "C2 Server";
	C2Server.value    =  getHealthStateString(getC2ServerHealth(C2Server.status));

    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemHealthXmlrpc::updateAllHealth, Exit");	
}


/**
 * SystemHealthXmlrpc::buildHealthResp()            
 *
 * Private method to get the info for the system health
 * 
 */

void
SystemHealthXmlrpc::buildHealthResp( xmlrpc_c::value &result )
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemHealthXmlrpc::buildHealthResp, Enter");
	
	int     									position = 0;
	vector<xmlrpc_c::value> 			elements;
	map<string, xmlrpc_c::value> 	element[25];     // Watch this subscript and change it if this list grows!
	HealthElem  							elems;

    elems = getServerEnv();
	EINSERT( elems, position );				// This macro also increments "position" by ONE

    elems = getMgmtPort();
	EINSERT( elems, position );
	
	elems = getTargetPorts();
	EINSERT( elems, position );

	elems = getOpenIB();
	EINSERT( elems, position );
	
	elems = getSeamonLX();
	EINSERT( elems, position );
	
	elems = getEnclosureEnv();
	EINSERT( elems, position );
	
	elems = getStorageInterconnect();
	EINSERT( elems, position );
	
	elems = getStorageConfiguration();
	EINSERT( elems, position );
	
	elems = getSHASState();
	EINSERT( elems, position );
	
	elems = getHyperFS();
	EINSERT( elems, position );
	
	elems = getIPStor();
	EINSERT( elems, position );
	
	elems = getCIFS();
	EINSERT( elems, position );
	
	elems = getFTP();
	EINSERT( elems, position );
	
	elems = getSoftwareConfiguration();
	EINSERT( elems, position );
	
	elems = getSystemServices();
	EINSERT( elems, position );
	
	elems = getStreamSmith();
	EINSERT( elems, position );
	
	elems = getVFlow();
	EINSERT( elems, position );
	
	elems = getSeaFS();
	EINSERT( elems, position );
	
	elems = getSparseCache();
	EINSERT( elems, position );
	
	elems = getDistibutedCache();
	EINSERT( elems, position );
	
	elems = getSentryService();
	EINSERT( elems, position );
	
	elems = getC2Server();
	EINSERT( elems, position );
	
	// If more are added, must adjust array size declared above!
	
	// Make it an array and get it activated for the return result
	xmlrpc_c::value_array array(elements);
	result = array;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemHealthXmlrpc::buildHealthResp, Exit");
}


/* End SystemHealthXmlrpc.cpp */
