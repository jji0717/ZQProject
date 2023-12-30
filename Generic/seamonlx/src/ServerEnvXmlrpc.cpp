/*
 * 
 * ServerObj.cpp
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
 * 
 *  #ipmitool sdr type fan
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include "ServerEnvXmlrpc.h"
#include "common.h"

using namespace std;



/******************************************************
 * 
 *              default constructor
 * 
 *****************************************************/

ServerEnvXmlrpc::ServerEnvXmlrpc()
{
	char	locstr[1024];
	string	formattedstring;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "ServerEnvXmlrpc::ServerEnvXmlrpc, Enter");

	this->_signature = "A:s" ;

	/**
	 * An initial update of server environmental data
	 */
	if (update() == FAILURE ){
		
		cout << "Failed updating the elements." << endl;
	}
	
	sprintf(locstr, "For best viewing ensure your screen width is at least %d characters", (HELPLINEEND + 10));
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", "ServerEnv [List, Fans, PowerSupplies, Temperatures, Voltages]");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","ServerEnv List");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","          returns List of Server Enrionmentals");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","1) ServerEnv Fans");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","          returns List of Fan data");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","2) ServerEnv PowerSupplies");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","          returns List of PowerSupply data");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","3) ServerEnv Temperatures");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","          returns List of Temperatures sensors data");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","1) ServerEnv Voltages");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","          returns List of Voltage data");
	FormatHelpString(formattedstring, locstr);

	this->_help = formattedstring;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "ServerEnvXmlrpc::ServerEnvXmlrpc, Exit");

}




/*******************************************************
 *
 *              The execute function
 *
 ******************************************************/

void 
ServerEnvXmlrpc::execute(xmlrpc_c::paramList const& paramList,
				xmlrpc_c::value *   const  retvalP){
	
	paramList.verifyEnd(1);
	
	string param1 = paramList.getString(0);

	/**
	 * todo: the data members will be updated by another thread
	 * in a certain interval.
	 */ 
	if (update() == FAILURE ){
		
		cout << "Failed updating the elements." << endl;
	}

	/**
	 * Parse the parameter and build the xml response based
	 * on the type.
	 * 
	 */
	if ( param1.compare("List") == 0 ){

		supportedTypeResp( *retvalP );
	}
	
			
	else if( param1.compare("Fans") == 0 ){

		buildFanResp( *retvalP );
	}
	
	else if ( param1.compare("PowerSupplies") == 0 ){

		buildPowersplResp( *retvalP );
	}
	
	else if ( param1.compare("Temperatures") == 0 ){
		
		buildTempResp( *retvalP );
	}
	
	else if ( param1.compare("Voltages") == 0 ){

		buildVoltageResp( *retvalP );
	}
	
	else{
		throw(xmlrpc_c::fault("Information specified by parameter not supported",
							  xmlrpc_c::fault::CODE_UNSPECIFIED));
	}
	
}



/**
 * Function supportedTypeResp
 * Response that returns the supported environmental element types.
 *
 * @return an xmlrpc value_array of supported types.
 */
void
ServerEnvXmlrpc::supportedTypeResp( xmlrpc_c::value &result)
{
	ZQ::common::MutexGuard gd(*this);
	vector<xmlrpc_c::value> supported;

	if ( fans.size() ){
		supported.push_back( xmlrpc_c::value_string("Fans") );
	}

	if ( powers.size() ){
		supported.push_back( xmlrpc_c::value_string("PowerSupplies") );
	}

	if ( tempsensors.size() ){
		supported.push_back( xmlrpc_c::value_string("Temperatures") );
	}
	
	if ( voltages.size() ){
		supported.push_back( xmlrpc_c::value_string("Voltages") );
	}

	xmlrpc_c::value_array typearray(supported);
	result = typearray;
}



/**
 * Function buildFanResp
 * Build the xml response that contains the fan information.
 *
 * @param[OUT] xml respons of a list of fans.
 */
void
ServerEnvXmlrpc::buildFanResp( xmlrpc_c::value &result)
{	ZQ::common::MutexGuard gd(*this);
	buildResp(fans, result);
}



/**
 * Function buildPowersplResp
 * Build the xml response that contains the fan information.
 *
 * @param[OUT] xml response of a list of power supplies
 */
void
ServerEnvXmlrpc::buildPowersplResp( xmlrpc_c::value &result)
{	ZQ::common::MutexGuard gd(*this);
	buildResp(powers, result);
}


/**
 * Function buildTempResp
 * Build the xml response that contains the fan information.
 *
 * @param[OUT] xml response of a list of 
 */
void
ServerEnvXmlrpc::buildTempResp( xmlrpc_c::value &result)
{	ZQ::common::MutexGuard gd(*this);
	buildResp(tempsensors, result);
}


/**
 * Function buildVoltageResp
 * Build the xml response that contains the fan information.
 *
 * @param[OUT] xml response of a list of power supplies
 */
void
ServerEnvXmlrpc::buildVoltageResp( xmlrpc_c::value &result)
{	ZQ::common::MutexGuard gd(*this);
	buildResp(voltages, result);
}



/**
 * Function buildFanResp
 * Build the xml response that contains the fan information.
 *
 * @param[IN]  The source used to build the response
 * @param[IN]  The id of the Element
 *             if id is "all", the response is a list of IDs.
 * @param[OUT] xml response of IDs or the detail of fan with
 *             specified ID.
 */
void
ServerEnvXmlrpc::buildResp(const EList &elems, xmlrpc_c::value &result)
{
		ZQ::common::MutexGuard gd(*this);
	vector<xmlrpc_c::value> elements;
    size_t nsize =elems.size() ;
    int index =0 ;
    traceClass->LogTrace(ZQ::common::Log::L_INFO, "the size of elems is: %d ......",nsize);
	vector<Envelem>::const_iterator it;
	for (it = elems.begin(); it != elems.end(); it ++){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "buildResp::index=%d : seacid=%s,value=%s,status=%s",index,(*it).seacid.c_str(),(*it).value.c_str(),(*it).status.c_str());
		map<string, xmlrpc_c::value> element;
		element.insert(make_pair(
						   "SEAC_ID",
						   xmlrpc_c::value_string(it->seacid)));
		element.insert(make_pair(
						   "value",
						   xmlrpc_c::value_string(it->value)));
		element.insert(make_pair(
						   "status",
						   xmlrpc_c::value_string(it->status)));
	
		elements.push_back(xmlrpc_c::value_struct( element ));
		index ++ ;
	}
	
	xmlrpc_c::value_array array(elements);
	result = array;
}
