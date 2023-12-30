/*
 * 
 * RpmsXml.h
 *
 *
 *  SystemRpmsXmlrpc class definition. The xmlrpc_c responses
 *  Derived from xmlrpc_c::method.
 *  
 *
 *
 *  Revision History
 *  
 *  06-08-2010 Created ( khemphill@schange.com)
 *  
 * 
 */

#ifndef SystemRpmsXmlrpc_H
#define SystemRpmsXmlrpc_H

#include <xmlrpc-c/base.hpp>

using namespace std;


/**
 * A class derived from xmlrpc_c::method
 */ 
class SystemRpmsXmlrpc : public xmlrpc_c::method {
  
	public:

	string param1;		// Rpms Request
	
	/**
	 * A constructor
	 */ 
	SystemRpmsXmlrpc();

	/**
	 * A member function.
	 * Extends the function execute in base class xmlrpc_c::method.
	 */ 
    void execute(xmlrpc_c::paramList const& paramList,
				 xmlrpc_c::value *   const  retvalP);

	/**
	 * A member function.
	 * Accessor to the member rpmsResp
	 */
	xmlrpc_c::value getRpmsResp() const{
		return rpmsResp;
	}
	
	void buildRpmsResp(xmlrpc_c::value &result);
	
	private:

	xmlrpc_c::value rpmsResp;
};

#endif /* ALERTXXML_H */
