/** @file MgmtPort.h
 *
 *  MgmtPort class declaration.
 *  Defines the MgmtPort Object class.
 *  
 *  SeaChange International
 *  
 *  Revision History
 *  
 *  04-12-2010 mjc    Created ()
 *  
 * 
 */

#ifndef MGMT_PORT_H
#define MGMT_PORT_H

#include <stdlib.h>
#include <map>
#include "common.h"

using namespace std;


/**
 * namespace seamonlx
 *
 */
namespace seamonlx
{

	/*
	 *
	 * Refer to the "working SeaSIM_Design V1_0.doc" document from Mike Woodside for information on each of these.
	 * 
	 * 
	 */
	class MgmtPort
	{
	  public:

		
		/**
		 * A constructor
		 * 
		 */ 
		MgmtPort();
		

		/**
		 * copy constructor
		 */ 
		MgmtPort(const MgmtPort& orig);

		/**
		 * assignment operator
		 */ 
		MgmtPort& operator=(const MgmtPort& other);
		
		/**
		 * destructor
		 */ 
		virtual ~MgmtPort();
		
		/**
		 * Member functions.
		 * 
		 */
		virtual int   update();
		
		/**
		 * Accessors
		 */
		ObjectHealthState::ObjHealthStateEnumerator getHealthState()
		{
		    return healthObj.getHealthState();
			
		}		
		
		string getHealthReasonDescription()
		{
		    return( healthObj.getHealthReasonDescription() );
		}			
		
	  protected:

		/**
		 * data members
		 */

		/*
		 * Object Health State
		 */
		ObjectHealthState    healthObj;		

	};

}
#endif /* MGMT_PORT_H */
