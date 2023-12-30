/** @file NetworkAdapters.h
 *
 *  NetworkAdapters class declaration.
 *  Defines the NetworkAdapters Object class.
 *  
 *  SeaChange International
 *  
 *  Revision History
 *  
 *  06-03-2010 mjc    Created ()
 *  
 * 
 */

#ifndef NetworkAdapters_H
#define NetworkAdapters_H

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
	 * Class NetworkAdapters
	 * Maintains the list of network adapters in the system and their health state
	 */
	class NetworkAdapters
	{
	  public:

		/**
		 * Type definitions
		 */
		typedef struct
		{
			string              			pciAddress;  			// The PCI Address
			string              			domain;
			string              			ifName;
			string              			ipAddress;
			string							hwAddress;
			ObjectHealthState				healthState;
			string							linkStatus;
			string							linkRate;
			int								portCount;			// The count of interfaces available
			int								locating;
			string							portCounters;
			
		} InterfaceInfo;	 
		
		typedef struct
		{
			string              			pciAddress;  
			string              			pciAddressWithDomain;  
			string              			Description;
			string							interfaceName;
			ObjectHealthState				healthState;
			string							driverName;
			string							driverVersion;
			string							firmwareVersion;
			string							aClass;
			string							manufacturer;
			string							model;
			string							revision;
			
		} AdapterInfo;	 

	
		/**
		 * A constructor
		 * 
		 */ 
		NetworkAdapters();
		
		/**
		 * destructor
		 */ 
		virtual ~NetworkAdapters();
		
		/**
		 * Member functions.
		 * 
		 */
		vector<AdapterInfo> getVecAdapters() const 
		{ 
			ZQ::common::MutexGuard gd(mlock_vecAdapters);
			return vecAdapters; 	
		}
		vector<InterfaceInfo> getVecInterfaces() const { return vecInterfaces; }
		
		virtual int		update( void );
		int				updateNetworkAdaptersList( void );
		int				updateNetworkInterfacesList( void );
		void			createNetworkInterface( char *pciAddr, char *domain, const char *name );
		bool			adapterAlreadyExists( char *pciAddr );
		void			DetermineHealthState( string ifName, ObjectHealthState &state );
		string			getManagementOverallHealth( void );
		string 			associatePCIAddrToInterfaceName( const char *pciAddress  );
		
		inline void		clearAdapterInfo( AdapterInfo &ai ) {
			ai.pciAddress.clear();
			ai.pciAddressWithDomain.clear();
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
		 * Function to get the pointer to the instance of NetworkAdapters class
		 */ 
		static NetworkAdapters  *instance() {
			return n_instance;
		}
	
	  protected:
		//
		//data members
		//
		vector<AdapterInfo>		vecAdapters;		
		vector<InterfaceInfo>	vecInterfaces;	
		
		static NetworkAdapters   *n_instance;
		
		ZQ::common::Mutex	mlock_vecAdapters;
	private:
	
	};
	
	
	/**
	 * Class NetworkAdapter
	 * Methods for the single network interface
	 */
	class NetworkAdapter
	{
	  public:

		/**
		 * A constructor
		 * 
		 */ 
		NetworkAdapter();
		
		/**
		 * destructor
		 */ 
		virtual ~NetworkAdapter();
		
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
#endif /* NetworkAdapters_H */
