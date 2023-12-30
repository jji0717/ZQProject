/*
 * 
 * SvcsXml.h
 *
 *
 *  SystemSvcsXmlrpc class definition. The xmlrpc_c responses
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

#ifndef SystemSvcsXmlrpc_H
#define SystemSvcsXmlrpc_H

#include <xmlrpc-c/base.hpp>

using namespace std;


/**
 * A class derived from xmlrpc_c::method
 */ 
class SystemSvcsXmlrpc : public xmlrpc_c::method {
  
	public:

	string param1;		// Svcs Request
	
	/**
	 * A constructor
	 */ 
	SystemSvcsXmlrpc();

	/**
	 * A member function.
	 * Extends the function execute in base class xmlrpc_c::method.
	 */ 
    void execute(xmlrpc_c::paramList const& paramList,
				 xmlrpc_c::value *   const  retvalP);

	/**
	 * A member function.
	 * Accessor to the member svcsResp
	 */
	xmlrpc_c::value getSvcsResp() const{
		return svcsResp;
	}
	
	void buildSvcsResp(xmlrpc_c::value &result);
	
	private:

	xmlrpc_c::value svcsResp;
};

#endif /* ALERTXXML_H */
