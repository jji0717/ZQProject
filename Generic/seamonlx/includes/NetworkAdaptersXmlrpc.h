/*
 * 
 * NetworkAdaptersXmlrpc.h
 *
 *
 *  NetworkAdaptersXmlrpc class definition. The xmlrpc_c responses  of the Network Adapters
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

#ifndef NETWORK_ADAPTERS_XMLRPC_H
#define NETWORK_ADAPTERS_XMLRPC_H

#include <xmlrpc-c/base.hpp>

#include "NetworkAdapters.h"


using namespace std;
using namespace seamonlx;

/**
 * A class derived from xmlrpc_c::method and seamonlx::SHAS
 */ 
class NetworkAdaptersXmlrpc : public xmlrpc_c::method, public NetworkAdapters {
	
  public:

	/**
	 * A constructor
	 */ 
	NetworkAdaptersXmlrpc();

	/**
	 * A member function.
	 * Extends the function execute in base class xmlrpc_c::method.
	 */ 
    void execute(xmlrpc_c::paramList const& paramList,
				 xmlrpc_c::value *   const  retvalP);

	/**
	 * Accessor to the members
	 */

	void buildNetworkAdaptersResp(xmlrpc_c::value &result);
	void buildNetworkAdapterResp(string pciAddr, xmlrpc_c::value &result);

	void buildNetworkInterfacesResp(xmlrpc_c::value &result);
	void buildNetworkInterfaceResp(string pciAddr, xmlrpc_c::value &result);
	
	void setLocatingFlag( bool value );


  protected:

	
	
  private:
	
	//xmlrpc_c::value   NetworkAdaptersResp;
		
	
};

#endif /* NETWORK_ADAPTERS_XMLRPC_H */
