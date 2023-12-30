/** @file InfinibandAdapters.h
 *
 *  InfinibandAdapters class declaration.
 *  Defines the InfinibandAdapters Object class.
 *  
 *  SeaChange International
 *  
 *  Revision History
 *  
 *  06-03-2010 mjc    Created ()
 *  
 * 
 */

#ifndef InfinibandAdapters_H
#define InfinibandAdapters_H

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
	 * Class InfinibandAdapters
	 * Maintains the list of network adapters in the system and their health state
	 */
	class InfinibandAdapters
	{
	  public:

		/**
		 * Type definitions
		 */
		typedef struct
		{
			//
			// ifconfig
			//
			string              			ifName;				// ib0
			string              			pciAddress;  		// The PCI Address
			string              			domain;
			string              			ipAddress;

			//
			// ibstat calls
			//
			string							adapterName;		// ml4x_0, mthca0, etc
			string							ibstate;			// Active, Down
			int								portNum;			// 1 or 2 which IB port
			string							linkStatus;			// physical status
			string							linkRate;
			ObjectHealthState				healthState;		// state field
			string							lmc;
			string							baseLID;
			string							smLID;
			
			// perfquery call returns these
			string							portCounters;
			
		} InterfaceInfo;	 
		
		typedef struct
		{
			//
			// lspci -D | grep -i 'Infiniband'
			//
			string              			pciAddress;
			string              			pciAddressWithDomain;
			string              			Description;
			string							interfaceName; // ib0

			//
			// ls -l /sys/bus/pci/devices/0000:0f:00.0/driver
			//
			string							driverName;

			//
			// modinfo driverName
			//
			string							driverVersion;
			

			//
			// ibstat
			//
			string							firmwareVersion;
			string							adapterName;		// ml4x_0, mthca0, etc
			string							numPorts;			// 1 or 2
			
			//
			// lspci -vm -s pciAddressWithDomain
			//
			string							aClass;
			string							manufacturer;
			string							model;
			string							revision;

			ObjectHealthState				healthState;	
		} AdapterInfo;	 

	
		/**
		 * A constructor
		 * 
		 */ 
		InfinibandAdapters();
		
		/**
		 * destructor
		 */ 
		virtual ~InfinibandAdapters();
		
		/**
		 * Member functions.
		 * 
		 */
		vector<AdapterInfo>		getVecAdapters()	const { return vecAdapters; }
		vector<InterfaceInfo>	getVecInterfaces()	const { return vecInterfaces; }
		
		virtual int		update( void );
		int				updateInfinibandAdaptersList( void );
		int				updateInfinibandInterfacesList( void );
		void			createInfinibandInterface( char *pciAddr, char *domain, const char *name, int portnum );
		bool			adapterAlreadyExists( char *pciAddr );
		void			DetermineHealthState( string ifName, ObjectHealthState &state );
		string			getManagementOverallHealth( void );
		string 			associatePCIAddrToInterfaceName( const char *pciAddress  );
		
		inline void		clearAdapterInfo( AdapterInfo &ai ) {
			ai.pciAddress.clear();
			ai.Description.clear();
			ai.interfaceName.clear();
			ai.driverName.clear();
			ai.driverVersion.clear();
			ai.firmwareVersion.clear();
			ai.aClass.clear();
			ai.manufacturer.clear();
			ai.model.clear();
			ai.revision.clear();
		}

		/**
		 * Function to get the pointer to the instance of InfinibandAdapters class
		 */ 
		static InfinibandAdapters  *instance() {
			return ib_instance;
		}
	
	  protected:
		//
		//data members
		//
		vector<AdapterInfo>		vecAdapters;		
		vector<InterfaceInfo>	vecInterfaces;	

		static InfinibandAdapters   *ib_instance;
		ZQ::common::Mutex	mlock_vecAdaptersOfInfinibandAdapters;		
	private:
	
	};
	
	
	/**
	 * Class InfinibandAdapter
	 * Methods for the single network interface
	 */
	class InfinibandAdapter
	{
	  public:

		/**
		 * A constructor
		 * 
		 */ 
		InfinibandAdapter();
		
		/**
		 * destructor
		 */ 
		virtual ~InfinibandAdapter();
		
		/**
		 * Member functions.
		 * 
		 */
		virtual int		update( void );
		
	
	  protected:

		/**
		 * data members
		 */


	private:
	

	};
	

}
#endif /* InfinibandAdapters_H */
