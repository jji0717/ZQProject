/*
 * 
 * BMCXml.h
 *
 *
 *  SystemBMCXmlrpc class definition. The xmlrpc_c responses
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

#ifndef SystemBMCXmlrpc_H
#define SystemBMCXmlrpc_H

#include <xmlrpc-c/base.hpp>

using namespace std;

/**
 * A class derived from xmlrpc_c::method
 */ 
class SystemBMCXmlrpc : public xmlrpc_c::method {
  
	public:

	string param1;		// BMC Request
	
	/**
	 * A constructor
	 */ 
	SystemBMCXmlrpc();

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
	xmlrpc_c::value getBMCResp() const{
		return BMCResp;
	}
	
	void buildBMCResp(xmlrpc_c::value &result);
	
	private:

	xmlrpc_c::value BMCResp;
};

#endif /* ALERTXXML_H */
