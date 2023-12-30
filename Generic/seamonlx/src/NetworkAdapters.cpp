/** @file NetworkAdapers.cpp
 *
 * NetworkAdapters class constructors and member functions  implementation.
 *  
 * Network Adapters object
 * 
 * This will provide a list of the PCI addresses of all network adapters in the system and overall health.
 * 
 * Adapters[1..N] of
 *	PCI Address
 * lspci –D | grep –i “Ethernet controller? *
 * Management Overall Health (OK, Critical, Failed)
 * This will be the Network Adapter object Status field for the Management Port interface defined by its PCI address. The Management Port interface is defined in the Manufacturing configuration file.
 * a) read Management port interface from Configuration file (network.*.management.interface).
 * b) see algorithm - Associate a Network Interface to an Ethernet PCI Address (PCIAddress)
 * c) get Network Adapter object (PCIaddress)
 * d) get the object status
 * 
 * Target Overall Health (OK, Critical, Failed)
 * This will be the “OR?of the Status field of each Network Adapter object Status field for the interfaces (that are not the Management interface) defined by its PCI address. The Management Port interface is defined in the Manufacturing configuration file.
 * a) read Management port interface from Configuration file (network.*.management.interface).
 * b) see algorithm - Associate a Network Interface to an Ethernet PCI Address (PCIAddress)
 * c) get Network Adapter object for all non-Management port interfaces (PCIaddress)
 * d) get the object status
 * e) If any of the item statuses is Critical the overall health should be Critical. If any of the item statuses is Failed the overall health should be Critical. The Condition description should indicate which items caused the problem.
 * 
 *
 *
 *  Revision History
 *  
 *  06-18-2010 mjc   Created
 *  07-06-2010 mjc   Handled special case for Chelsio adapter and how it lists multiple interfaces
 *  
 * 
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <dirent.h>
#include "seamonlx.h"
#include "common.h"
#include "NetworkAdapters.h"

using namespace std;
using namespace seamonlx;


extern   CONFIG_STRUCT	SysConfigData;

 //
 // Uncomment this define for detailed TRACE_LOG tracing of this method
 //
//#define SCR_DEBUG 1			// SHAS Config Response detailed DEBUG 
//
#ifdef SCR_DEBUG
#define SCR_TRACE( s1, args... )  traceClass->LogTrace(ZQ::common::Log::L_INFO,  s1, ##args )
#else
#define SCR_TRACE( s1, args... )  ;
#endif
//


/*************************************************
 *
 *       Class NetworkAdapters methods definition
 * 
 ************************************************/

NetworkAdapters *NetworkAdapters::n_instance = 0;
/**
 * NetworkAdapters constructor
 */

NetworkAdapters::NetworkAdapters()
{
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "NetworkAdapters.cpp : Construct NetworkAdapters Object.");
	
	if( n_instance){
		throw "Attempting to create second NetworkAdapters list";
	} 
	n_instance = this;
}

/**
 * NetworkAdapters Destructor
 */ 
NetworkAdapters::~NetworkAdapters()
{

}


/**
 * Function update()
 *
 * The function that updates the NetworkAdapters and NetworkInterfaces data members
 */
int
NetworkAdapters::update( void )
{
	
	int retval = SUCCESS;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdapters::update, Enter");
	
	// Clear out what we had before
	//
	ZQ::common::MutexGuard gd(mlock_vecAdapters);
	vecAdapters.clear();
	vecInterfaces.clear();
	
	/*
	 * Populate the Network Adapters and Network Interfaces lists
	 */
	 if( updateNetworkAdaptersList() != SUCCESS ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "NetworkAdapters::update  failure.");	
			retval = FAILURE;
	}

	 if( updateNetworkInterfacesList() != SUCCESS ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "NetworkInterfaces::update  failure.");	
			retval = FAILURE;
	}

	
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdapters::update, Exit, retval=%d", retval);

	return retval;
}


/**
 * Function updateNetworkAdaptersList()
 *
 * The function that updates the NetworkAdapters data members
 */
int
NetworkAdapters::updateNetworkAdaptersList( void )
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "updateNetworkAdaptersList, Enter");
	
	int 				retval = SUCCESS;
	char				line[ BUFF256 ];
	AdapterInfo   adapterEntry;
	string			fullLineString;
	char				pciAddr[ BUFF80 ];
	char				interface[ BUFF8 ];
	char				description[ BUFF256 ];

	/**
	 * The program name and params
	 */
	string command = "/sbin/lspci -D | /bin/grep -i 'Ethernet controller'";

	traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tcommand=|%s|", command.c_str() );
		
	FILE *stream = popen(command.c_str(), "r");
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", command.c_str());
	if ( stream == NULL ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "updateNetworkAdaptersList: Failed to get stream handle from command.");
		return FAILURE;
	}
	
	// Loop through lines and gather our network interfaces
	while( fgets(line, sizeof(line), stream ) != NULL )	
	{

		// Line must be at least 33 characters long to be a valid line
		// i.e.
		//	0000:01:00.0 Ethernet controller: Intel Corporation 82576 Gigabit Network Connection (rev 01)
		// 0000:01:00.1 Ethernet controller: Intel Corporation 82576 Gigabit Network Connection (rev 01)
		// 0000:07:00.0 Ethernet controller: Chelsio Communications Inc Unknown device 0036

		// Remove '\r' from end of line (may need to check if it actually is a CR here first!)
		if( strlen( line ) > 0 ) {
			line[ strlen(line) - 1 ] = '\0';
		}
		
		fullLineString = line;			// Assign our buffer to a class string

		// Check full length
		if( fullLineString.length() < 33 )	{
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "\tLine length is short & is ignored! %d characters. Line=|%s|", fullLineString.length(), fullLineString.c_str());
			continue;		// Just continue and ignore this line.
		}

		// Copy the PCI Address into our temporary char array
		if( fullLineString.copy( pciAddr, 12 ) == 0 )	{
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "\tCopy of Line for pciAddr failed!  Line=|%s|", fullLineString.c_str());
			continue;		// Just continue and ignore this line.
		}
		
		// We need to remove the domain from the PCI Address. The domain is denoted by a period. For example:
		// 	0000:01:00.1 Ethernet controller: Intel Corporation. Domain here is the .1 value.
		// We assume that there is only one digit that represents the domain value. This will cover 10 interfaces, but
		// will break if there are more than that.
		// Offset 12 includes the domain. Offset 12-2 is just the pciAddress without the domain.
		// Offset 11 is the digit for the Network Interface
		pciAddr[12] = '\0';   // Append NULL char
		adapterEntry.pciAddressWithDomain = pciAddr;		// Assign the FULL pciAddressWithDomain 
		
		strcpy( interface, &pciAddr[12-2] );		// Copy the interface number into it's own storage space
		pciAddr[12-2] = '\0';   							// Chop off domain and use that as the pciAddress
		adapterEntry.pciAddress = pciAddr;		// Assign the pciAddress
		
		// Copy the Network Adapter Descrption starting at position 34
		size_t len = fullLineString.copy( description, BUFF256-1, 34 ) ;
		if( len == 0 )	{
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "\tCopy of Line for Description failed!  Line=|%s|", fullLineString.c_str());
			//continue;		// Just continue and ignore this line.
		}
		description[len] = '\0';   // Append NULL char
		adapterEntry.Description = description;		// Assign it

		
		adapterEntry.interfaceName.assign( associatePCIAddrToInterfaceName( adapterEntry.pciAddressWithDomain.c_str() ) );
		
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tadapterEntry.InterfaceName =|%s|",adapterEntry.interfaceName.c_str());
		//
		// Create a Network Interface entry for this one.
		//
		createNetworkInterface( pciAddr, interface, adapterEntry.interfaceName.c_str() );
		
		if( adapterAlreadyExists( pciAddr ) ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t**** adapterAlreadyExists TRUE pciAddr=|%s|, clearAdapterInfo and move on!", pciAddr );
				
			// Clear it and move on as we dont need to add duplicates
				clearAdapterInfo( adapterEntry );
				continue;
		}

		//
		// Determine the network adapters' HEALTH STATE
		//
		DetermineHealthState( adapterEntry.interfaceName, adapterEntry.healthState );


		/**
		 * Get more  interface data using
		 * 	ethtool with a "-i" added for specific information
		 * Assumption: The command response is not greater than BUFF256 bytes long!
		 */
		 int  			bytesRead;	// Num bytes read from command output
		size_t		location;		// Temporary string buffer size location for search values
		 
		 
		command = "/sbin/ethtool -i ";
		command.append( adapterEntry.interfaceName );

		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tCommand 2=|%s|", command.c_str() );
			
		FILE *ifStream = popen(command.c_str(), "r");
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", command.c_str());
		if ( ifStream == NULL ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "NetworkAdapters updateNetworkAdaptersList:2: Failed to get stream handle");
		}

		if( (bytesRead = fread( line, 1, BUFF256-1, ifStream )) > 0 ) {
				traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tCOMMAND 2 BYTES READ = %d",  (int)bytesRead );
					
		}
		
		pclose( ifStream );			// Close the stream	

		fullLineString = line;		// Assign to a string class
		
		//
		// Search for Driver, Version, and Firmware info 
		// Assumption here is that none of these fields will exceed BUFF80 length!
		//
		char 		sTag[BUFF80];
		size_t 		crpos;
		
		strcpy( sTag, "driver:" );
		if( (location=fullLineString.find( sTag )) == string::npos ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find driver string!" );
		} else {	// We found it.
			crpos = fullLineString.find( NEWLINE_CHARACTER, location+strlen(sTag)+1 );
			len = crpos - (location+strlen(sTag)+1);
			adapterEntry.driverName = fullLineString.substr( location+strlen(sTag)+1, len );
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tdriverName=|%s|", adapterEntry.driverName.c_str());
				
		}		
		
		strcpy( sTag, "version:" );
		if( (location=fullLineString.find( sTag )) == string::npos ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find version string!" );
		} else {	// We found it.
			crpos = fullLineString.find( NEWLINE_CHARACTER, location+strlen(sTag)+1 );
			len = crpos - (location+strlen(sTag)+1);
			adapterEntry.driverVersion = fullLineString.substr( location+strlen(sTag)+1, len );
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tdriverVersion=|%s|", adapterEntry.driverVersion.c_str() );
				
		}		
		
		strcpy( sTag, "firmware-version:" );
		if( (location=fullLineString.find( sTag )) == string::npos ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find firmware-version string!" );
		} else {	// We found it.
			crpos = fullLineString.find( NEWLINE_CHARACTER, location+strlen(sTag)+1 );
			len = crpos - (location+strlen(sTag)+1);
			adapterEntry.firmwareVersion = fullLineString.substr( location+strlen(sTag)+1, len );
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tfirmwareVersion=|%s|", adapterEntry.firmwareVersion.c_str() );
				
		}		


		/**
		* Get more  interface data using
		* 	lspci -vm -s <PCI Address>
		* Assumption: The command response is less than BUFF256 bytes long!
		*/
		 
		 
		command = "/sbin/lspci -vm -s  ";
		command.append( adapterEntry.pciAddress );

		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tCommand 3=|%s|", command.c_str() );
			
		ifStream = popen(command.c_str(), "r");
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", command.c_str());
		if ( ifStream == NULL ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "NetworkAdapters updateNetworkAdaptersList:3: Failed to get stream handle");
		}

		if( (bytesRead = fread( line, 1, BUFF256-1, ifStream )) > 0 ) {
				traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tCOMMAND 3 BYTES READ = %d", (int)bytesRead);
					
		}
		
		pclose( ifStream );			// Close the stream	
		fullLineString = line;		// Assign to a string class
		
		//
		// Search for Class, Manufacturer(Vendor), Model, and Revision fields
		// Assumption here is that none of these fields will exceed BUFF80 length!
		//
		strcpy( sTag, "Class:" );
		if( (location=fullLineString.find( sTag )) == string::npos ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find Class string!" );
		} else {	// We found it.
			crpos = fullLineString.find( NEWLINE_CHARACTER, location+strlen(sTag)+1 );
			len = crpos - (location+strlen(sTag)+1);
			adapterEntry.aClass = fullLineString.substr( location+strlen(sTag)+1, len );
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tclass=|%s|", adapterEntry.aClass.c_str() );
		}		
		
		strcpy( sTag, "Vendor:" );
		if( (location=fullLineString.find( sTag )) == string::npos ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find Vendor string!" );
		} else {	// We found it.
			crpos = fullLineString.find( NEWLINE_CHARACTER, location+strlen(sTag)+1 );
			len = crpos - (location+strlen(sTag)+1);
			adapterEntry.manufacturer = fullLineString.substr( location+strlen(sTag)+1, len );
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tmanufacturer=|%s|", adapterEntry.manufacturer.c_str());
				
		}		
		
		//
		// special case, 2 occurances of "Device:" interested in 2nd one
		//
		fullLineString.replace(3, fullLineString.find("Device"), "XYZ");
		strcpy( sTag, "Device:" );
		if( (location=fullLineString.find( sTag )) == string::npos ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find Model string!" );
		} else {	// We found it.
			crpos = fullLineString.find( NEWLINE_CHARACTER, location+strlen(sTag)+1 );
			len = crpos - (location+strlen(sTag)+1);
			adapterEntry.model = fullLineString.substr( location+strlen(sTag)+1, len );
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tmodel=|%s|", adapterEntry.model.c_str() );
				
		}		

		strcpy( sTag, "Rev:" );
		if( (location=fullLineString.find( sTag )) == string::npos ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find Rev string!" );
		} else {	// We found it.
			crpos = fullLineString.find( NEWLINE_CHARACTER, location+strlen(sTag)+1 );
			len = crpos - (location+strlen(sTag)+1);
			adapterEntry.revision = fullLineString.substr( location+strlen(sTag)+1, len );
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\trevision=|%s|", adapterEntry.revision.c_str() );
				
		}		
		
		
		// Get the entry onto the list of adapters
		vecAdapters.push_back( adapterEntry );

		// Clear it our for next one
		clearAdapterInfo( adapterEntry );
	
	} // End while

	pclose(stream); 		// Close the stream as we are done with it	

	//	 Should have out adapter list now all set, size of which is our PortCount
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "updateNetworkAdaptersList, Exit. retval=%d. vecAdapters size (PortCount)=%d", retval, (int) vecAdapters.size() );

	return( retval );
}

/**
 * Function createNetworkInterface()
 *
 * This method will add or update an existing Network Interface
 * If it's not found it will be added.
 * If it is found, then the port count value will be incremented.
 * 
 * Returns:
 *		Void.
 */
void
NetworkAdapters::createNetworkInterface( char *pciAddress, char *domain, const char *ifName )
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "createNetworkInterface, Enter. pciAddr=|%s|  Domain=|%s|  ifName=|%s|",
							pciAddress, domain, ifName );

	vector <InterfaceInfo>	 :: iterator		it;
	InterfaceInfo										ifEntry;
	bool													found = false;

	// Loop thru all interfaces
	for ( it = vecInterfaces.begin(); (it != vecInterfaces.end()) && !found; it++ ) {	
	
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tComparing vecInterfaces |%s| Domain |%s|     to pciAddress |%s| Domain |%s|",it->pciAddress.c_str(), it->domain.c_str(), pciAddress, domain);
			
		// Search for the exact entry only
		if( (it->pciAddress.compare( pciAddress ) == 0) && (it->domain.compare( domain ) == 0) ) {
		
			// Found a duplicate entry, so we just bump the port count value
			//it->portCount++;		// We also forgo bumping the count - see next comment for found=true below
			//found = true;		// We force the adding of the interface here for the Chelsio case
										// This may break in future platforms so BE FOREWARNED!!!!!!
		
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tFOUND EXISTING ENTRY: Name=%s  Update Port Count to %d", it->ifName.c_str(), it->portCount );
		}
			
	} // End for loop
	
	// If we di dnot find it, we add the entry
	if( ! found ) {
		ifEntry.pciAddress = pciAddress;
		ifEntry.domain = domain;
		ifEntry.ifName = ifName;
		ifEntry.portCount = 1;
		vecInterfaces.push_back( ifEntry );
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tADDING ENTRY:  Name=%s vecInterfaces count = %d", ifEntry.ifName.c_str(), (int)vecInterfaces.size() );
	}
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "createNetworkInterface, Exit");

}

/**
 * Function adapterAlreadyExists()
 *
 * 	This method will check for the existance of an adapter in the vecAdapters list
 * Returns:
 *				bool - TRUE if found, FALSE if not
 */
bool
NetworkAdapters::adapterAlreadyExists( char *pciAddress )
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "adapterAlreadyExists, Enter");

	vector <AdapterInfo>	 :: iterator		it;
	bool													found = false;

	// Loop thru all interfaces
	for ( it = vecAdapters.begin(); (it != vecAdapters.end()) && !found; it++ ) {	
	
		// Search for the exact entry only
		if( it->pciAddress.compare( pciAddress ) == 0 ) {
			found = true;
		}
			
	} // End for loop
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "adapterAlreadyExists, Exit");
	
	return( found );

}


/**
 * Function updateNetworkInterfacesList()
 *
 * The function that updates the NetworkInterfaces data members
 */
int
NetworkAdapters::updateNetworkInterfacesList( void )
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "updateNetworkInterfacesList, Enter");
	
	int 													retval = SUCCESS;
	FILE													*stream;
	int													bytesRead;
	unsigned int									i;	
	vector <InterfaceInfo>	 :: iterator		it;
	string												command;
	char													line[ BUFF2K ];
	string												fullLineString;
	char													sTag[BUFF32];
	size_t												location;		// Temporary string buffer size location for search values

	// Loop thru all interfaces and get all the data we need
	for ( it = vecInterfaces.begin(); (it != vecInterfaces.end()); it++ ) {	

		memset( (void *)line, '\0', sizeof( line ) );
		
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tUpdating Interfaces ENTRY pciAddr=%s Name=%s", it->pciAddress.c_str(), it->ifName.c_str() );
				
		// ifconfig with the interface name gives us what we want
		command = "/sbin/ifconfig ";
		command.append( it->ifName );

		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tCommand1 =|%s|", command.c_str() );
			
		stream = popen(command.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", command.c_str());
		if ( stream == NULL ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "NetworkAdapters updateNetworkInterfacesList:1: Failed to get stream handle");
		}

		if( (bytesRead = fread( line, 1, BUFF1K-1,  stream )) > 0 ) {
				traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tCOMMAND BYTES READ = %d",  (int)bytesRead );
					
		}
		
		pclose( stream );			// Close the stream	
		
		
		
		// Start looking for the IP Address field
		fullLineString = line;		// Assign to a string class
		
		//
		// Search for IP Address and HW Address
		//
		strcpy( sTag, "inet addr:" );
		if( (location=fullLineString.find( sTag )) == string::npos ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find inet addr string!" );
		} else {	// We found it.

			// Now we crawl thru each character until we find a space, and then copy that string into ipAddress
			// and then put a null character on the end of it

			for( i=(int)location+strlen(sTag);  fullLineString.at(i) != ' ' && fullLineString.at(i) != NEWLINE_CHARACTER; i++ ) {
			
					;			// Do nothing, just walk the characters to, hopefully, find the blank space, indication end of IP address
			}

			//SCR_TRACE("\t\ti=%d    location=%d   (location+strlen(sTag))=%d    (i-(ocation+strlen(sTag)))=%d", i, location, location+strlen(sTag), i-(location+strlen(sTag))  );

			// We found a string if this is true
			if( i > location+strlen(sTag) ) {
				it->ipAddress.assign( fullLineString, location + strlen(sTag), i-(location+strlen(sTag)) );		// Copy the data in
				traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tipAddress=|%s|", it->ipAddress.c_str() );	
					
			}
			else {
				// Something is broken. Report it.
				traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find IP address!. Skipping it." );
			}
		}		

		// Start again to look for ther HW Address
		fullLineString = line;		// Assign to a string class
		
		strcpy( sTag, "HWaddr " );	// NOTE space on end of string must be there ! !
		
		if( (location=fullLineString.find( sTag )) == string::npos ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find HWaddr string!" );
		} else {	// We found it.

			// Now we crawl thru each character until we find a space, and then copy that string into ipAddress
			// and then put a null character on the end of it

			for( i=(int)location+strlen(sTag);  fullLineString.at(i) != ' ' && fullLineString.at(i) != NEWLINE_CHARACTER; i++ ) {
			
					;			// Do nothing, just walk the characters to, hopefully, find the blank space, indication end of value
			}

			//SCR_TRACE("\t\ti=%d    location=%d   (location+strlen(sTag))=%d    (i-(ocation+strlen(sTag)))=%d", i, location, location+strlen(sTag), i-(location+strlen(sTag))  );

			// We found a string if this is true
			if( i > location+strlen(sTag) ) {
				it->hwAddress.assign( fullLineString, location + strlen(sTag), i-(location+strlen(sTag)) );		// Copy the data in
				traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\thwAddress=|%s|", it->hwAddress.c_str() );	
					
			}
			else {
				// Something is broken. Report it.
				traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find HW address!. Skipping it." );
			}
		}		

		//
		// Now onto Link Status, Link Rate
		//		
		command = "/sbin/ethtool ";
		command.append( it->ifName );

		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tCommand2 =|%s|", command.c_str());	
			
		stream = popen(command.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", command.c_str());
		if ( stream == NULL ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "NetworkAdapters updateNetworkInterfacesList:2: Failed to get stream handle");
		}

		if( (bytesRead = fread( line, 1, BUFF1K-1,  stream )) > 0 ) {
				traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tCOMMAND BYTES READ = %d",  (int)bytesRead );	
					
		}
		
		pclose( stream );			// Close the stream	
		
		fullLineString = line;		// Assign to a string class
		
		//
		// Search for Link Status (Link Detected) and and Link Rate fields
		//
		strcpy( sTag, "Link detected: " );
		if( (location=fullLineString.find( sTag )) == string::npos ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find 'Link detected:' string!" );
		} else {	// We found it.

			// Now we crawl thru each character until we find a space, and then copy that string
			// and then put a null character on the end of it

			for( i=(int)location+strlen(sTag);  fullLineString.at(i) != ' ' && fullLineString.at(i) != NEWLINE_CHARACTER; i++ ) {
			
					;			// Do nothing, just walk the characters to, hopefully, find the blank space, indication end of value
			}

			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\ti=%d    location=%d   (location+strlen(sTag))=%d    (i-(ocation+strlen(sTag)))=%d", i, location, location+strlen(sTag), i-(location+strlen(sTag)) );	
				
			// We found a string if this is true
			if( i > location+strlen(sTag) ) {
				it->linkStatus.assign( fullLineString, location + strlen(sTag), i-(location+strlen(sTag)) );		// Copy the data in
				traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tlinkStatus=|%s|", it->linkStatus.c_str() );	
					
			}
			else {
				// Something is broken. Report it.
				traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find Link status!. Skipping it." );
			}
		}		

		// Next field - the Link Rate
		fullLineString = line;		// Assign to a string class
		
		//
		// Search for Link Status (Link Detected) and and Link Rate fields
		//
		strcpy( sTag, "Speed: " );
		if( (location=fullLineString.find( sTag )) == string::npos ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find Speed string!" );
		} else {	// We found it.

			// Now we crawl thru each character until we find a space, and then copy that string
			// and then put a null character on the end of it

			for( i=(int)location+strlen(sTag);  fullLineString.at(i) != ' ' && fullLineString.at(i) != NEWLINE_CHARACTER; i++ ) {
			
					;			// Do nothing, just walk the characters to, hopefully, find the NL, indicating end of value
			}
			
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\ti=%d    location=%d   (location+strlen(sTag))=%d    (i-(ocation+strlen(sTag)))=%d", i, location, location+strlen(sTag), i-(location+strlen(sTag)));		
			// We found a string if this is true
			if( i > location+strlen(sTag) ) {
				it->linkRate.assign( fullLineString, location + strlen(sTag), i-(location+strlen(sTag)) );		// Copy the data in

				traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tipAddress=|%s|", it->linkRate.c_str() );	
					
			}
			else {
				// Something is broken. Report it.
				traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find Link rate!. Skipping it." );
			}
		}		

		//
		// Now onto Port Counters
		//		
		command = "/sbin/ethtool -S ";
		command.append( it->ifName );

		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tCommand3 =|%s|", command.c_str());	
			
		stream = popen(command.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", command.c_str());
		if ( stream == NULL ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "NetworkAdapters updateNetworkInterfacesList:3: Failed to get stream handle");
		}

		if( (bytesRead = fread( line, 1, BUFF2K-1,  stream )) > 0 ) {
				traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tCOMMAND BYTES READ = %d",  (int)bytesRead);	
					
		}
		
		pclose( stream );			// Close the stream	
		
		it->portCounters = line;		// Assign the whole thing at a whack
	
		//
		// Determine the network interfaces' HEALTH STATE
		//
		DetermineHealthState( it->ifName, it->healthState );	
	
	} // End for loop	

	//	 Should have our interfaces list now all updated with data
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "updateNetworkInterfacesList, Exit. retval=%d", retval );

	return( retval );
}

/**
 * NetworkAdapter::DetermineHealthState
 * Given the ifName, return the Health State of the interface based on a number of factors 
 *
 * To determine health:
 * 	ethtool interfaceName [then Link detected:, Speed: , Duplex: field. 
 *			Link detected must be yes, 
 *			Duplex must be Full, 
 *			Speed must be a valid speed. 
 *			If any of the 3 are no or unknown then “Critical? If all have a value or yes then “OK? *
 * Returns: The HealthState enumerator field is set in the ObjectHealthState class instance 
 *				passed in by reference.
 */
void NetworkAdapters::DetermineHealthState( string ifName, ObjectHealthState &state )
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdapters::DetermineHealthState, Enter.  Interface = %s", ifName.c_str() );
	
	char				fullBuffer[ BUFF4K ];
	string			fullBufferString;
	int				bytesRead;
	ObjectHealthState::ObjHealthStateEnumerator tempState =  ObjectHealthState::STATE_OK;
	
	/**
	 * ethtool with a grep for the fields we want related to STATE
	 */
	string command = "/sbin/ethtool ";
	command.append( ifName );

	traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tcommand=|%s|", command.c_str() );	
		
	FILE *stream = popen(command.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", command.c_str());
	if ( stream == NULL ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "NetworkAdapters DetermineHealthState: Failed to get stream handle");
	}

	if( (bytesRead = fread( fullBuffer, 1, BUFF4K, stream )) > 0 ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tBYTES READ = %d",  (int)bytesRead);	
				
	}
	
	pclose( stream );	// Close the stream
	
	// Process the buffer
	fullBufferString = fullBuffer;		// Get it into a string class as it's easier to work with (imho)

	size_t		location;		// Temporary string buffer size location for search values
	
	/**
	*  The next three searches make up the health state of the interface 
	*/
	
	//
	// Search for Link State
	//
	char sLinkDet[] = "Link detected:";
	if( (location=fullBufferString.find( sLinkDet )) == string::npos ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find Link detected string!" );
	} else {	// We found it.
		string   value = fullBufferString.substr( location+strlen(sLinkDet)+1, 3 );
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tLink state compare: Got |%s| comparing to 'yes' ", value.c_str());	
			
		if( value.compare( "yes" ) != 0 ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\t|%s| not yes ", value.c_str());	
				
			tempState =  ObjectHealthState::STATE_CRITICAL;
		}
	}
	
	//
	// Search for Duplex
	//
	char sDuplex[] = "Duplex:";
	if( (location=fullBufferString.find( sDuplex )) == string::npos ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find Duplex  string!" );
	} else {	// We found it.
		string   value = fullBufferString.substr( location+strlen(sDuplex)+1, 4 );
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tDuplex compare: Got |%s| comparing to 'Full' ", value.c_str());	
			
		if( value.compare( "Full" ) != 0 ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\t|%s| Not Full", value.c_str());	
			tempState =  ObjectHealthState::STATE_CRITICAL;
		}
	}	
	
	//
	// Search for Speed
	//
	char sSpeed[] = "Speed:";
	if( (location=fullBufferString.find( sSpeed )) == string::npos ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tDid not find Speed  string!" );
	} else {	// We found it.
		string   value = fullBufferString.substr( location+strlen(sSpeed)+1, 6 );
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tSpeed compare: Got |%s| comparing to '10' ", value.c_str());	
			
		if( value.compare( "10" ) <= 0 ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\t|%s| Less than or zero compare to '10' ", value.c_str());	
			tempState =  ObjectHealthState::STATE_CRITICAL;
		}
	}	
	
	state.setHealthState( tempState );			// Set the now determined state

	if( TRACING ) {
		string theState = state.getHealthStateString( tempState );
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "NetworkAdapters::DetermineHealthState, Exit.  Returning State = %s", theState.c_str());	
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdapters::DetermineHealthState, Exit");
	
}

/**
 * NetworkAdapter::getManagementOverallHealth
 *
 *	This will be the Network Adapter object Status field for the Management Port interface defined by its PCI address. 
 *		The Management Port interface is defined in the Manufacturing configuration file.
 * 	a) read Management port interface from Configuration file (network.*.management.interface).
 *		b) see algorithm - Associate a Network Interface to an Ethernet PCI Address (PCIAddress)
 *
 * ASSOCIATE A NETWORK INTERFACE TO AN ETHERNET PCI ADDRESS
 * o	Find PCI address of the card. Save value as PCI_address
 * # lspci –D | grep –i Ethernet (or InfiniBand)
 * o	This will show a link with the interface name. Save value as Interface_name. 
  * If there are more than one interfaces then the count of those should be kept as Port_count.
 * # ls -l /sys/bus/pci/devices/ PCI_address/net*
 *
  * 	c) get Network Adapter object (PCIaddress)
 * 	d) get the object status
 *
 */
string NetworkAdapters::getManagementOverallHealth( void )
{
	string 												state = "unknown";
	vector <AdapterInfo>	 :: iterator		adIt;
	bool													found = false;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdapters::getManagementOverallHealth, Enter");

	// Get mgmt interface name from config file
	if( strlen( SysConfigData.ManagementIf ) <= 0 ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "\t ==> MgmtIf is blank, so returning string |%s|", state.c_str() );
		return state;		// Unknown
	}
	
	// Associate the name
	// Loop thru all Network Adapters, match the ManagementIf name to one in that vector
	for ( adIt = vecAdapters.begin(); (adIt != vecAdapters.end()) && !found; adIt++ ) {	
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tCompare MgmtIf |%s| to adapter |%s|", SysConfigData.ManagementIf, adIt->interfaceName.c_str());	
			
		if( adIt->interfaceName.compare( SysConfigData.ManagementIf ) == 0 ) {

			found = true;		// Set the flag

			// Get the Network Object status
			state = adIt->healthState.getHealthStateString(adIt->healthState.getHealthState());
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tFOUND! the state of that Adapter is %s",state.c_str());
				
		}
	}  // End for
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "NetworkAdapters::getManagementOverallHealth, Exit; HealthState is |%s|", state.c_str() );
	return  state;
}


/**
 * NetworkAdapter::associatePCIAddrToInterfaceName
 *
 *
 */
string NetworkAdapters::associatePCIAddrToInterfaceName( const char *pciAddress  )
{
	string theString;
	int	count = 0;
	char  fileName[BUFF64];
	char  line[BUFF256];

	traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tassociatePCIAddrToInterfaceName, pciAddr=%s",pciAddress);
	//  We need to set each adapters STATE information here based on interface
	// ASSOCIATE A NETWORK INTERFACE TO AN ETHERNET PCI ADDRESS
	// 	Find PCI address of the card. Save value as PCI_address
	//  	lspci –D | grep –i Ethernet (or InfiniBand)
	// 	This will show a link with the interface name. Save value as Interface_name. 
	// If there are more than one interfaces then the count of those should be kept as Port_count.
	// 	ls -l /sys/bus/pci/devices/ PCI_address/net*
	sprintf( fileName, "ls -l /sys/bus/pci/devices/%s/net*", pciAddress );

	FILE *ifStream = popen(fileName, "r");
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", fileName);
	if ( ifStream == NULL ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "associatePCIAddrToInterfaceName: Failed to get ifStream handle from command.");
			// Fill in some default name here for the field  
			
	}		

	// Loop thru the lines, ensure we have a line length that is more than just a CR/LF
	while( (fgets(line, sizeof(line), ifStream ) != NULL) && (strlen( line ) > 5 )) {
	
		// We got the line
		char *devName = strrchr( line, '/' );		// Search for last / character in the path
		if( devName != NULL ) {
			devName[ strlen( devName+1 ) ] = '\0';		// Remove CR at the end of the line
			
			// Assign the returned string only to the first interface name, not any subsequent name
			if( count == 0 ) {
				theString.assign( devName+1 );		// We have the name here
			}
		}
		else	{
			theString = "Unknown";		// Else we have to say something
		}
		
		//
		// Special case here if we get more than one interface in this directory list.  7/6/10
		// So far, this means that we have found a Chelsio adapter that does the name assignments differently than the
		// Intel adapters.  We handle this Chelsio case here with this call to create another interface for the same adapter.
		// This, in effect, becomes a duplicate network adapter (which in the lesser of two evils, is okay compared to 
		// the alternative of having a completely missing interface from the list).
		//
		if( count++ > 0 ) {
		
			char 		copyPciAddr[BUFF64];	

			strcpy( copyPciAddr, pciAddress );						// Make a copy

			// Special case where we are adding a raw PCI Address without the appended digit
			// We need to ensure it has no appended digit, as in the case of a NetworkAdapters <PCI Address>
			// request coming in
			char 	*dotNum = strrchr( copyPciAddr, '.' );		// Search for last period
			if( dotNum != NULL ) {
					*dotNum = '\0';		// Truncate at the period
			}

			// The ".0" must be added to ensure we have a domain of some sort, even if it's a duplicate!
			createNetworkInterface( copyPciAddr, ".0", devName+1 ); 
		}
		
	}
	
	pclose( ifStream );		// Close the open stream

	traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tassociatePCIAddrToInterfaceName, exit; returning string |%s|",theString.c_str());
	return( theString );
		
}

/* End of NetworkAdapters.cpp */
