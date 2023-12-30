/*
 * 
 * SHASCountersXmlrpc.h
 *
 *
 *  SHASCountersXmlrpc class definition. The xmlrpc_c responses
 *  of the SHAS Configuration object type class
 *  Derived from xmlrpc_c::method.
 *  
 *
 *
 *  Revision History
 *  
 *  06-03-2010 mjc  Created ()
 *  
 * 
 */

#ifndef SHAS_COOUNTERS_XMLRPC_H
#define SHAS_COUNTERS_XMLRPC_H

#include <xmlrpc-c/base.hpp>

#include "SHASCounters.h"


using namespace std;
using namespace seamonlx;

/**
 * A class derived from xmlrpc_c::method and seamonlx::SHAS
 */ 
class SHASCountersXmlrpc : public xmlrpc_c::method, public SHASCounters {
	
  public:

	/**
	 * A constructor
	 */ 
	SHASCountersXmlrpc();

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

	void buildSHASCountersResp(string shasType, xmlrpc_c::value &result);
	void buildSHASSupportedCountersResp(xmlrpc_c::value &result);


  protected:

	
	
  private:
	
	xmlrpc_c::value   ShasResp;
		
	
};

#endif /* SHAS_COUNTERS_XMLRPC_H */
