/*
 * 
 * SHASXml.h
 *
 *
 *  SHASConfigXmlrpc class definition. The xmlrpc_c responses
 *  of the SHAS Configuration object type class
 *  Derived from xmlrpc_c::method.
 *  
 *
 *
 *  Revision History
 *  
 *  04-28-2010 mjc  Created ()
 *  05-20-2010 mjc  Added buildSHAS_CL_ConfigResp() method
 *  05-26-2010  mjc  Added use of String Resource library for tag strings
 *  06-09-2010  mjc  Added Enclosure / Bay capability. 
 *								Housekeeping: Removed strings and put them in base class ShasConfig.
 *  08-12-2010  mjc  Fixed bug with PD <enclosure id> XML result. Seperated out PD handling from LD/LE.
 *  
 * 
 */

#ifndef SHAS_CONFIG_XMLRPC_H
#define SHAS_CONFIG_XMLRPC_H

#include <xmlrpc-c/base.hpp>

#include "SHASConfig.h"


using namespace std;
using namespace seamonlx;

/**
 * A class derived from xmlrpc_c::method and seamonlx::SHAS
 */ 
class SHASConfigXmlrpc : public xmlrpc_c::method, public SHASConfig {
	
  public:

	/**
	 * A constructor
	 */ 
	SHASConfigXmlrpc();

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
	xmlrpc_c::value getSHASResp() const{
		return ShasResp;
	}

	void buildSHASConfigResp(xmlrpc_c::value &result);
	void buildSHAS_PD_ConfigResp(xmlrpc_c::value &result);
	void buildSHAS_CL_ConfigResp(xmlrpc_c::value &result);


  protected:

	
  private:
	
	xmlrpc_c::value   ShasResp;
	
};

#endif /* SHAS_CONFIG_XMLRPC_H */
