/*
 * 
 * ConfigXml.h
 *
 *
 *  SystemConfigXmlrpc class definition. The xmlrpc_c responses
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

#ifndef SystemConfigXmlrpc_H
#define SystemConfigXmlrpc_H

#include <xmlrpc-c/base.hpp>

using namespace std;

/**
 * A class derived from xmlrpc_c::method
 */ 
class SystemConfigXmlrpc : public xmlrpc_c::method {
  
	public:

	string param1;		// Config Request
	
	/**
	 * A constructor
	 */ 
	SystemConfigXmlrpc();

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
	xmlrpc_c::value getConfigResp() const{
		return ConfigResp;
	}
	
	void buildConfigResp(xmlrpc_c::value &result);
	
	private:

	xmlrpc_c::value ConfigResp;
};

#endif /* ALERTXXML_H */
