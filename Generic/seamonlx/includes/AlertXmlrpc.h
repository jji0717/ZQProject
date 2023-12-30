/*
 * 
 * AlertXml.h
 *
 *
 *  SystemAlertXmlrpc class definition. The xmlrpc_c responses
 *  of server hardware information.
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

#ifndef SystemAlertXmlrpc_H
#define SystemAlertXmlrpc_H

#include <xmlrpc-c/base.hpp>

#include "seamonlxalert.h"

using namespace std;

#define   REASON_CODE_BARF_ERROR  	"Error"

/**
 * A class derived from xmlrpc_c::method
 */ 
class SystemAlertXmlrpc : public xmlrpc_c::method {
  public:

	string param1;		// Alert Request Operation (Alert, AlertRange, AlertSequence, AlertComponent)
	string param2;		// First Seq Num, or Name of Component
	string param3;		// Last Seq Num

	/**
	 * A constructor
	 */ 
	SystemAlertXmlrpc();

	/**
	 * A member function.
	 * Extends the function execute in base class xmlrpc_c::method.
	 */ 
    void execute(xmlrpc_c::paramList const& paramList,
				 xmlrpc_c::value *   const  retvalP);

	/**
	 * A member function.
	 * Accessor to the member alertRangeResp
	 */
	xmlrpc_c::value getAlertRangeResp() const{
		return alertRangeResp;
	}
	
	/**
	 * A member function.
	 * Accessor to the member alertSequenceResp
	 */
	xmlrpc_c::value getAlertSequenceResp() const{
		return alertSequenceResp;
	}

	/**
	 * A member function.
	 * Accessor to the member alertResp
	 */
	xmlrpc_c::value getAlertResp() const{
		return alertResp;
	}
	
	void buildAlertResp(xmlrpc_c::value &result);
	void buildAlertSequenceResp(xmlrpc_c::value &result);
	void buildAlertRangeResp(xmlrpc_c::value &result);
	void buildSortedComponentResp(xmlrpc_c::value &result);
	
  private:
	xmlrpc_c::value alertResp;
	xmlrpc_c::value alertRangeResp;
	xmlrpc_c::value alertSequenceResp;	
};

#endif /* ALERTXXML_H */
