/** @file StorageAdapterXmlrpc.h
 *
 *  StorageAdapterXmlrpc class declaration.
 *  Defines the class StorageAdapterXmlrpc that extends
 *  the class xmlrpc_c::method and StorageAdaters..
 *  
 *  SeaChange International
 *  
 *  Revision History
 *  
 *  03-30-2010 Created ( jie.zhang@schange.com)
 *  
 * 
 */

#ifndef STORAGEADAPTERXMLRPC_H
#define STORAGEADAPTERXMLRPC_H

#include <xmlrpc-c/base.hpp>
#include "StorageAdapters.h"

using namespace std;
using namespace seamonlx;


/**
 * A class devrived from seamonlx::StorageAdapters
 * and xmlrpc_c::method
 */
class StorageAdapterXmlrpc : public xmlrpc_c::method, public StorageAdapters
	
{
  public:

	/**
	 * A constructor
	 */ 
	StorageAdapterXmlrpc();

	/**
	 * A member function.
	 * Extends the function execute in base class xmlrpc_c::method
	 */
	void execute(xmlrpc_c::paramList const& paramList,
				 xmlrpc_c::value *   const  retvalP);

	/**
	 * Member functions.
	 */ 
	void getAdapterIds(xmlrpc_c::value &result);
	void getAdapter(string seacid, xmlrpc_c::value &result);
	void getAdapterChildren(string seacid, xmlrpc_c::value &result);
	
	/**
	 * overload = operator
	 */ 
	StorageAdapterXmlrpc& operator=(const StorageAdapterXmlrpc& other){
		if( &other != this ){
			setAdapters( other.adapters );
			//	setAdaptermap( other.adapterMap );
		}
		return *this;
	}

};

#endif /*STORAGEADAPTERXMLRPC_H*/
