/** @file EnclProcessorXmlrpc.h
 *
 *  EnclProcessorXmlrpc class declaration.
 *  Defines the class EnclProcessorXmlrpc that extends
 *  the class xmlrpc_c::method and EnclProcessors.
 *  
 *  SeaChange International
 *  
 *  Revision History
 *  
 *  04-26-2010 Created by jiez
 */

#ifndef ENCLPROCESSORXMLRPC_H
#define ENCLPROCESSORXMLRPC_H

#include <xmlrpc-c/base.hpp>
#include "EnclProcessors.h"


using namespace std;
using namespace seamonlx;


/**
 * A class devrived from seamonlx::EnclProcessors
 * and xmlrpc_c::method
 */

class EnclProcessorXmlrpc : public xmlrpc_c::method, public EnclProcessors
{

  public:
	
	/**
	 * A constructor
	 */
	EnclProcessorXmlrpc();

	/**
	 * A member function.
	 * Extends the function execute in base class xmlrpc_c::method
	 */
	void execute(xmlrpc_c::paramList const& paramList,
				 xmlrpc_c::value *   const  retvalP);

	/**
	 * Member functions.
	 */
	void getEnclIds(xmlrpc_c::value &result);
	void getEncl(string seacid, xmlrpc_c::value &result);
	void getEnclChildren( string seacid, xmlrpc_c::value &result);
	void getEnclsgstatus( string sgname, string baynum, xmlrpc_c::value &result);

};


#endif /* ENCLPROCESSORXMLRPC_H*/
