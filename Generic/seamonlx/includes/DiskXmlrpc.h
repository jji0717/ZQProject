/** @file DiskXmlrpc.h
 *
 *  DiskXmlrpc class declaration.
 *  Defines the class DiskXmlrpc that extends
 *  the class xmlrpc_c::method and Disks.
 *  
 *  SeaChange International
 *  
 *  Revision History
 *  
 *  04-26-2010 Created by jiez
 */

#ifndef DISKXMLRPC_H
#define DISKXMLRPC_H

#include <xmlrpc-c/base.hpp>
#include "Disks.h"


using namespace std;
using namespace seamonlx;


/**
 * A class devrived from seamonlx::Disks
 * and xmlrpc_c::method
 */

class DiskXmlrpc : public xmlrpc_c::method, public Disks
{

  public:
	
	/**
	 * A constructor
	 */
	DiskXmlrpc();

	/**
	 * A member function.
	 * Extends the function execute in base class xmlrpc_c::method
	 */
	void execute(xmlrpc_c::paramList const& paramList,
				 xmlrpc_c::value *   const  retvalP);

	/**
	 * Member functions.
	 */
	/*function to get the disk detail*/
	void getDisk(string seacid, xmlrpc_c::value &result);

	/* function to get all the disk ids */
	void getDiskIds(xmlrpc_c::value &result);

	/* function to get the ids of disks not attached to enclosure or adapters */
	void getStandaloneDiskIds( xmlrpc_c::value &result);
};


#endif /* DISKXMLRPC_H */
