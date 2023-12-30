/*
 * 
 * MgmtPortXml.h
 *
 *
 *  MgmtPortXmlrpc class definition. The xmlrpc_c responses
 *  of system health information
 *  Derived from xmlrpc_c::method.
 *  
 *
 *
 *  Revision History
 *  
 *  04-19-2010 mjc  Created ()
 *  
 * 
 */

#ifndef MGMTPORTXMLRPC_H
#define MGMTPORTXMLRPC_H

#include <xmlrpc-c/base.hpp>

#include "MgmtPort.h"
#include "ServerHwXmlrpc.h"
#include "MgmtPortXmlrpc.h"


using namespace std;
using namespace seamonlx;

/**
 * A class derived from xmlrpc_c::method and seamonlx::MgmtPort
 */ 
class MgmtPortXmlrpc : public xmlrpc_c::method, public MgmtPort {
	
  public:

	/**
	 * A constructor
	 */ 
	MgmtPortXmlrpc();

	/**
	 * A member function.
	 * Extends the function execute in base class xmlrpc_c::method.
	 */ 
    void execute(xmlrpc_c::paramList const& paramList,
				 xmlrpc_c::value *   const  retvalP);

	/**
	 * A member function.
	 * Accessor to the member healthResp
	 */
	xmlrpc_c::value getMgmtPortResp() const{
		return mgmtPortResp;
	}

	void buildMgmtPortResp(xmlrpc_c::value &result);


  protected:

	
	
  private:
	
	xmlrpc_c::value    mgmtPortResp;
		
	
};

#endif /* MGMTPORTXMLRPC_H */
