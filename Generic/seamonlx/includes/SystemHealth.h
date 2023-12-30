/** @file SystemHealth.h
 *
 *  SystemHealth class declaration.
 *  Defines the SystemHealth Object class.
 *  
 *  SeaChange International
 *  
 *  Revision History
 *  
 *  04-12-2010 mjc    Created ()
 *  05-18-2010 mjc	  Removed InfiniBand() from the list
 *  
 * 
 */

#ifndef SYSTEM_HEALTH_H
#define SYSTEM_HEALTH_H

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

	/**
	 * A base class SystemHealth. It contains the following health status information
	 * for the following system objects:
	 *
	 * 01 - Server Environmentals
	 * 02 - Management Port
	 * 03 - Target Ports
	 * 04 - InfiniBand  (REMOVED)
	 * 05 - OpenIB - Subnet Mgmt.
	 * 06 - SeamonLX
	 * 07 - Enclosure Environmentals (UML)
	 * 08 - Storage Interconnect (UML)
	 * 09 - Storage Configuration (UML)
	 * 10 - SHAS State (UML)
	 * 11 - HyperFS (UML)
	 * 12 - IPStor (UML)
	 * 13 - CIFS
	 * 14 - FTP
	 * 15 - Software Configuration
	 * 16 - System Services
	 * 17 - StreamSmith (UMS, UMG)
	 * 18 - VFlow (UMS)
	 * 19 - SeaFS (UMS)
	 * 20 - Sprase Cache (UMS)
	 * 21 - Distibuted Cache (UMS)
	 * 22 - Sentry Service (UMS, UMG)
	 * 23 - C2 Server (UML, UMG)
	 *
	 * Refer to the "working SeaSIM_Design V1_0.doc" document from Mike Woodside for information on each of these.
	 * 
	 * 
	 */
	class SystemHealth
	{
	  public:

		/**
		 * Type definition
		 */
		typedef struct
		{
			string seacid;
			string value;
			string status;
		} HealthElem;
		typedef vector<HealthElem> HList;
		
		/**
		 * A constructor
		 * 
		 */ 
		SystemHealth();
		

		/**
		 * copy constructor
		 */ 
		SystemHealth(const SystemHealth& orig);

		/**
		 * assignment operator
		 */ 
		SystemHealth& operator=(const SystemHealth& other);
		
		/**
		 * destructor
		 */ 
		virtual ~SystemHealth();
		
		/**
		 * Member functions.
		 * 
		 */
		virtual int   update();
		
		/**
		 * Accessors
		 */
		HealthElem   getServerEnv( void ) { return ServerEnv;	}
		HealthElem   getMgmtPort( void ) { return ManagementPort;	}						
		HealthElem   getTargetPorts( void ) { 	return TargetPorts;	}							
		//HealthElem   getInfiniBand( void ) { 	return InfiniBand;	}							
		HealthElem   getOpenIB( void ) { 	return OpenIB;	}							
		HealthElem   getSeamonLX( void ) { 	return SeamonLX;	}							
		HealthElem   getEnclosureEnv( void ) { 	return EnclosureEnv;	}							
		HealthElem   getStorageInterconnect( void ) { 	return StorageInterconnect;	}							
		HealthElem   getStorageConfiguration( void ) { 	return StorageConfiguration;	}							
		HealthElem   getSHASState( void ) { 	return SHASState;	}							
		HealthElem   getHyperFS( void ) { 	return HyperFS;	}							
		HealthElem   getIPStor( void ) { 	return IPStor;	}							
		HealthElem   getCIFS( void ) { 	return CIFS;	}							
		HealthElem   getFTP( void ) { 	return FTP;	}							
		HealthElem   getSoftwareConfiguration( void ) { 	return SoftwareConfiguration;	}							
		HealthElem   getSystemServices( void ) { 	return SystemServices;	}							
		HealthElem   getStreamSmith( void ) { 	return StreamSmith;	}							
		HealthElem   getVFlow( void ) { 	return VFlow;	}							
		HealthElem   getSeaFS( void ) { 	return SeaFS;	 }							
		HealthElem   getSparseCache( void ) { 	return SparseCache; }							
		HealthElem   getDistibutedCache( void ) { 	return DistributedCache;	}							
		HealthElem   getSentryService( void ) { 	return SentryService;	}							
		HealthElem   getC2Server( void ) { 	return C2Server;	}							

		
	  protected:

		/**
		 * data members
		 */
		HealthElem			ServerEnv;
		HealthElem			ManagementPort;
		HealthElem			TargetPorts;
		//HealthElem			InfiniBand;
		HealthElem			OpenIB;
		HealthElem			SeamonLX;
		HealthElem			EnclosureEnv;
		HealthElem			StorageInterconnect;
		HealthElem			StorageConfiguration;
		HealthElem			SHASState;
		HealthElem			HyperFS;
		HealthElem			IPStor;
		HealthElem			CIFS;
		HealthElem			FTP;
		HealthElem			SoftwareConfiguration;
		HealthElem			SystemServices;
		HealthElem			StreamSmith;
		//HealthElem			StreamSmith_UMG;
		HealthElem			VFlow;
		HealthElem			SeaFS;
		HealthElem			SparseCache;
		HealthElem			DistributedCache;
		HealthElem			SentryService;
		//HealthElem			SentryService_UMG;
		HealthElem			C2Server;
		//HealthElem			C2Server_UMG;		

	};

}
#endif /* SYSTEM_HEALTH_H */
