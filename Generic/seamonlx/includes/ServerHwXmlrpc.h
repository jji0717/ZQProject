/*
 * 
 * ServerHwXml.h
 *
 *
 *  ServerHwXmlrpc class definition. The xmlrpc_c responses
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

#ifndef SERVERHWXMLRPC_H
#define SERVERHWXMLRPC_H

#include <xmlrpc-c/base.hpp>
#include <Server.h>

using namespace std;
using namespace seamonlx;

/**
 * A class derived from xmlrpc_c::method and seamonlx::ServerHw
 */ 
class ServerHwXmlrpc : public xmlrpc_c::method, public ServerHw {
  public:

	/**
	 * A constructor
	 */ 
	ServerHwXmlrpc();

	/**
	 * A member function.
	 * Extends the function execute in base class xmlrpc_c::method.
	 */ 
    void execute(xmlrpc_c::paramList const& paramList,
				 xmlrpc_c::value *   const  retvalP);

	/**
	 * A member function.
	 * Accessor to the member chassisResp
	 */
	xmlrpc_c::value getChassisResp() const{
		return chassisResp;
	}
	
	/**
	 * A member function.
	 * Accessor to the member baseboardResp
	 */
	xmlrpc_c::value getBaseboardResp() const{
		return baseboardResp;
	}
	
	/**
	 * A member function.
	 * Accessor to the member biosResp
	 */	
	xmlrpc_c::value getBiosResp() const{
		return biosResp;
	}
	
	/**
	 * A member function.
	 * Accessor to the member processorResp
	 */	
	xmlrpc_c::value getProcessorResp() const{
		return processorResp;
	}
	
	/**
	 * A member function.
	 * Accessor to the member memoryResp
	 */	
	xmlrpc_c::value getMemoryResp() const{
		return memoryResp;
	}


	void buildMapResp(map<string, string> in, xmlrpc_c::value &result);

	void buildChassisResp();
	void buildBaseboardResp();
	void buildBiosResp();
	void buildMemoryResp();
	void buildProcResp();
	
  private:
	xmlrpc_c::value chassisResp;
	xmlrpc_c::value baseboardResp;	
	xmlrpc_c::value biosResp;
	xmlrpc_c::value processorResp;
	xmlrpc_c::value memoryResp;
};

#endif /* SERVERHWXML_H */
