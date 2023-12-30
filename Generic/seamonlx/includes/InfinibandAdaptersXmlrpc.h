/*
 * 
 * InfinibandAdaptersXmlrpc.h
 *
 *
 *  InfinibandAdaptersXmlrpc class definition. The xmlrpc_c responses  of the Infiniband Adapters
 *  Derived from xmlrpc_c::method.
 *  
 *
 *
 *  Revision History
 *  
 *  06-11-2010 mjc  Created ()
 *  
 * 
 */

#ifndef InfinibandAdapters_XMLRPC_H
#define InfinibandAdapters_XMLRPC_H

#include <xmlrpc-c/base.hpp>

#include "InfinibandAdapters.h"


using namespace std;
using namespace seamonlx;

/**
 * A class derived from xmlrpc_c::method and seamonlx::SHAS
 */ 
class InfinibandAdaptersXmlrpc : public xmlrpc_c::method, public InfinibandAdapters {
	
  public:

	/**
	 * A constructor
	 */ 
	InfinibandAdaptersXmlrpc();

	/**
	 * A member function.
	 * Extends the function execute in base class xmlrpc_c::method.
	 */ 
    void execute(xmlrpc_c::paramList const& paramList,
				 xmlrpc_c::value *   const  retvalP);

	/**
	 * Accessor to the members
	 */

	void buildInfinibandAdaptersResp(xmlrpc_c::value &result);
	void buildInfinibandAdapterResp(string pciAddr, xmlrpc_c::value &result);

	void buildInfinibandInterfacesResp(xmlrpc_c::value &result);
	void buildInfinibandInterfaceResp(string pciAddr, xmlrpc_c::value &result);
	
  protected:

	
	
  private:
	
	//xmlrpc_c::value   InfinibandAdaptersResp;
		
	
};

#endif /* InfinibandAdapters_XMLRPC_H */
