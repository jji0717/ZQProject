/*
 * 
 * ServerEnv.h
 *
 *
 *  ServerEnv class definition. Contains the server environmental
 *  sensors information.
 *
 *  Derived from xmlrpc_c::method.
 *  
 *
 *
 *  Revision History
 *  
 *  03-10-2010 Created ( jie.zhang@schange.com)
 *  
 * 
 */

#ifndef SERVERENVXMLRPC_H
#define SERVERENVXMLRPC_H

#include <xmlrpc-c/base.hpp>
#include "Server.h"

using namespace std;
using namespace seamonlx;

class ServerEnvXmlrpc : public xmlrpc_c::method, public ServerEnv {
  public:
	ServerEnvXmlrpc ();
	
    void execute ( xmlrpc_c::paramList const& paramList,
				   xmlrpc_c::value *   const  retvalP );

	void supportedTypeResp ( xmlrpc_c::value &result );
	
	void buildFanResp ( xmlrpc_c::value &result );
	
	void buildPowersplResp ( xmlrpc_c::value &result );
	
	void buildTempResp ( xmlrpc_c::value &result );

	void buildVoltageResp ( xmlrpc_c::value &result );

  private:

	void buildResp ( const EList &elems,
					 xmlrpc_c::value &result );
	

	
};

#endif /* SERVERENVXMLRPC_H */
