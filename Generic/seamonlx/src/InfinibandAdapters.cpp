/** @file InfinibandAdapters.cpp
 *
 * InfinibandAdapters class constructors and member functions  implementation.
 *  
 * Infiniband Adapters object
 * 
 * This will provide a list of the PCI addresses of all infiniband adapters in the system and overall health.
 * 
 * Adapters[1..N] of
 *	PCI Address
 * lspci –D | grep –i “Infiniband? *
 * Management Overall Health (OK, Critical, Failed)
 * This will be the Infiniband Adapter object Status field 
 * 
 * Target Overall Health (OK, Critical, Failed)
 * This will be the “OR?of the Status field of each Infiniband Adapter object Status field 
 * for the interfaces defined by its PCI address. 
 *
 *
 *  Revision History
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
#include "InfinibandAdapters.h"

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
 *       Class InfinibandAdapters methods definition
 * 
 ************************************************/

InfinibandAdapters *InfinibandAdapters::ib_instance = 0;
/**
 * InfinibandAdapters constructor
 */

InfinibandAdapters::InfinibandAdapters()
{
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "InfinibandAdapters.cpp : Construct InfinibandAdapters Object.");
	
	if( ib_instance){
		throw "Attempting to create second InfinibandAdapters list";
	}

	ib_instance = this;
}

/**
 * InfinibandAdapters Destructor
 */ 
InfinibandAdapters::~InfinibandAdapters()
{
}


/**
 * Function update()
 *
 * The function that updates the InfinibandAdapters and InfinibandInterfaces data members
 */
int
InfinibandAdapters::update( void )
{
	int retval = SUCCESS;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdapters::update, Enter");
	
	// Clear out what we had before
	//
	vecAdapters.clear();
	vecInterfaces.clear();

	ZQ::common::MutexGuard gd(mlock_vecAdaptersOfInfinibandAdapters);	
	/*
	 * Populate the Infiniband Adapters and Infiniband Interfaces lists
	 */
	 if( updateInfinibandAdaptersList() != SUCCESS ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "InfinibandAdapters::update  failure.");	
			retval = FAILURE;
	}

	 if( updateInfinibandInterfacesList() != SUCCESS ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "InfinibandInterfaces::update  failure.");	
			retval = FAILURE;
	}

	
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdapters::update, Exit, retval=%d", retval);

	return retval;
}


/**
 * Function updateInfinibandAdaptersList()
 *
 * The function that updates the InfinibandAdapters data members
 */
int
InfinibandAdapters::updateInfinibandAdaptersList( void )
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "updateInfinibandAdaptersList, Enter");
	
	int 				retval = SUCCESS;
	char				line[ BUFF256 ];
	AdapterInfo			adapterEntry;
	string				fullLineString;
	char				pciAddr[ BUFF80 ];
	char				interface_i[ BUFF8 ];
	char				description[ BUFF256 ];
	char 				sTag[BUFF80];

	/**
	 * The program name and params
	 */
	string command = "/sbin/lspci -D | /bin/grep -i 'InfiniBand'";
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tcommand=|%s|", command.c_str() );
	FILE *stream = popen(command.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", command.c_str());
	if ( stream == NULL ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "updateInfinibandAdaptersList: Failed to get stream handle from command.");
		return FAILURE;
	}
	
	// Loop through lines and gather our network interfaces
	while( fgets(line, sizeof(line), stream ) != NULL )	{

		// Line must be at least 33 characters long to be a valid line
		// i.e.
		// 0000:0c:00.0 InfiniBand: Mellanox Technologies MT26428 [ConnectX VPI PCIe 2.0 5GT/s - IB QDR / 10GigE] (rev b0)
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
		
		
		// 0000:0c:00.0 InfiniBand: Mellanox Technologies MT26428 Domain here is the .0 value.
		// We assume that there is only one digit that represents the domain value. This will cover 10 interfaces, but
		// will break if there are more than that.
		// Offset 12 includes the domain. Offset 12-2 is just the pciAddress without the domain.
		// Offset 11 is the digit for the Infiniband Interface
		pciAddr[12] = '\0';   // Append NULL char
		adapterEntry.pciAddressWithDomain = pciAddr;		// Assign the FULL pciAddressWithDomain 
		
		strcpy( interface_i, &pciAddr[12-2] );		// Copy the interface number into it's own storage space
		pciAddr[12-2] = '\0';   					// Chop off domain and use that as the pciAddress
		adapterEntry.pciAddress = pciAddr;			// Assign the pciAddress
		
		// Copy the Infiniband Adapter Descirption starting at position 25
		size_t len = fullLineString.copy( description, BUFF256-1, 25 ) ;
		if( len == 0 )	{
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "\tCopy of Line for Description failed!  Line=|%s|", fullLineString.c_str());
			//continue;		// Just continue and ignore this line.
		}
		description[len] = '\0';   // Append NULL char
		adapterEntry.Description.assign(description);		// Assign it

		adapterEntry.interfaceName.assign( associatePCIAddrToInterfaceName( adapterEntry.pciAddressWithDomain.c_str() ) );
		
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tadapterEntry.InterfaceName =|%s|", adapterEntry.interfaceName.c_str() );	
		
		if( adapterAlreadyExists( pciAddr ) ) {
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t**** adapterAlreadyExists TRUE pciAddr=|%s|, clearAdapterInfo and move on!", pciAddr);
			// Clear it and move on as we dont need to add duplicates
				clearAdapterInfo( adapterEntry );
				continue;
		}

		//
		// get adapter name
		//
		command.clear();
		command = "/usr/sbin/ibstat | grep CA | awk -F \"'\" '{print $2}' ";
		GetStringFromCmd(command, adapterEntry.adapterName);
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\adapterName=|%s|", adapterEntry.adapterName.c_str() );
		//
		// Determine the network adapters' HEALTH STATE
		//
		DetermineHealthState( adapterEntry.adapterName, adapterEntry.healthState );

		/**
		 *  get adapter info, use 
		 */
		 
		//driverName
		command.clear();
		command = "ls -l /sys/bus/pci/devices/";
		command.append(adapterEntry.pciAddressWithDomain.c_str());
		command.append("/driver");
		GetStringFromCmd(command, fullLineString);
		char *tmpName = strrchr( fullLineString.c_str(), '/' );		
		if( tmpName != NULL ) {		
			adapterEntry.driverName.assign( tmpName+1 );		
		}
		
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tdriverName=|%s|", adapterEntry.driverName.c_str() );
		// modinfo adapterEntry.driverVersion
		command.clear();
		command = "/sbin/modinfo ";
		command.append(adapterEntry.driverName.c_str());
		command.append(" | grep -e '^version:' | awk -F \":\" '{print $2}' ");
		GetStringFromCmd(command, adapterEntry.driverVersion);
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t driverVersion=|%s| ", adapterEntry.driverVersion.c_str());
			
		//
		// ibstat
		//

		// Firmware vers
		command.clear();
		command = "/usr/sbin/ibstat | grep -i 'Firmware version:' | awk -F \":\" '{print $2}' ";
		GetStringFromCmd(command, adapterEntry.firmwareVersion);
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tfirmwareVersion=|%s|", adapterEntry.firmwareVersion.c_str() );
		// numPorts
		command.clear();
		command = "/usr/sbin/ibstat | grep -i 'Number of ports:' | awk -F \":\" '{print $2}' ";
		GetStringFromCmd(command, adapterEntry.numPorts);
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tfirmwareVersion=|%s|", adapterEntry.numPorts.c_str() );
		for (int portnum = 1; portnum <= atoi(adapterEntry.numPorts.c_str()); portnum++) {
			//
			// Create a Infiniband Interface entry for this one.
			//
			createInfinibandInterface( pciAddr, interface_i, adapterEntry.interfaceName.c_str(), portnum );
		}
			
		/*
		* Get more  Adapter data using
		* 	lspci -vm -s <PCI Address>
		* Assumption: The command response is less than BUFF256 bytes long!
		*/

		command.clear();
		command = "/sbin/lspci -vm -s  ";
		command.append( adapterEntry.pciAddress );
		GetBlockOutput(command, fullLineString);
		
		strcpy( sTag, "Class:" );
		GetTagValue(fullLineString, sTag, adapterEntry.aClass);
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tclass=|%s|", adapterEntry.aClass.c_str() );
		strcpy( sTag, "Vendor:" );
		GetTagValue(fullLineString, sTag, adapterEntry.manufacturer);
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tmanufacturer=|%s|", adapterEntry.manufacturer.c_str() );
		strcpy( sTag, "Rev:" );
		GetTagValue(fullLineString, sTag, adapterEntry.revision);
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\trevision=|%s|", adapterEntry.revision.c_str() );
		//
		// Update string for unique parsing, "Device:" found twice
		//
		fullLineString.replace(3, fullLineString.find("Device"), "XYZ");
		strcpy( sTag, "Device:" );
		GetTagValue(fullLineString, sTag, adapterEntry.model);
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tmodel=|%s|", adapterEntry.model.c_str() );
			
		// Get the entry onto the list of adapters
		vecAdapters.push_back( adapterEntry );

		// Clear it our for next one
		clearAdapterInfo( adapterEntry );
	
	} // End while

	pclose(stream); 		// Close the stream as we are done with it	

	//	 Should have out adapter list now all set, size of which is our PortCount
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "updateInfinibandAdaptersList, Exit. retval=%d. vecAdapters size (PortCount)=%d", retval, (int) vecAdapters.size() );

	return( retval );
}

/**
 * Function createInfinibandInterface()
 *
 * This method will add or update an existing Infiniband Interface
 * If it's not found it will be added.
 * If it is found, then the port count value will be incremented.
 * 
 * Returns:
 *		Void.
 */
void
InfinibandAdapters::createInfinibandInterface( char *pciAddress, char *domain, const char *ifName, int portnum )
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "createInfinibandInterface, Enter. pciAddr=|%s|  Domain=|%s|  ifName=|%s|",
							pciAddress, domain, ifName );

	vector <InterfaceInfo>	 :: iterator		it;
	InterfaceInfo								ifEntry;
	bool										found = false;

	//
	// Loop thru all interfaces
	//
	for ( it = vecInterfaces.begin(); (it != vecInterfaces.end()) && !found; it++ ) {	
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tComparing vecInterfaces |%s| Domain |%s|     to pciAddress |%s| Domain |%s|", \
			it->pciAddress.c_str(), it->domain.c_str(), pciAddress, domain );
		
		// Search for the exact entry only
		if( (it->pciAddress.compare( pciAddress ) == 0) && (it->domain.compare( domain ) == 0) ) {
		
			// Found a duplicate entry, so we just bump the port count value
			//it->portCount++;		// We also forgo bumping the count - see next comment for found=true below
			//found = true;		// We force the adding of the interface here for the Chelsio case
										// This may break in future platforms so BE FOREWARNED!!!!!!
			//traceClass->LogTrace(ZQ::common::Log::L_INFO, "\t\t FOUND EXISTING ENTRY: Name=%s  Update Port Count to %d ", it->ifName.c_str(), it->portCount );
			//SCR_TRACE("\t\tFOUND EXISTING ENTRY: Name=%s  Update Port Count to %d", it->ifName.c_str(), it->portCount  );
		}
			
	} // End for loop

	// If we did not find it, we add the entry
	if( ! found ) {
		ifEntry.pciAddress = pciAddress;
		ifEntry.domain = domain;
		ifEntry.ifName = ifName; 
		
		string Command1 = "/usr/sbin/ibstat | grep CA | awk -F \"'\" '{print $2}' ";
		GetStringFromCmd(Command1, ifEntry.adapterName);
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\adapterName=|%s|", ifEntry.adapterName.c_str() );
			
		ifEntry.portNum = portnum;

		vecInterfaces.push_back( ifEntry );
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tADDING ENTRY:  Name=%s vecInterfaces count = %d", ifEntry.ifName.c_str(), (int)vecInterfaces.size() );
	}
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "createInfinibandInterface, Exit");

}

/**
 * Function adapterAlreadyExists()
 *
 * 	This method will check for the existance of an adapter in the vecAdapters list
 * Returns:
 *				bool - TRUE if found, FALSE if not
 */
bool
InfinibandAdapters::adapterAlreadyExists( char *pciAddress )
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
 * Function updateInfinibandInterfacesList()
 *
 * The function that updates the InfinibandInterfaces data members
 */
int
InfinibandAdapters::updateInfinibandInterfacesList( void )
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "updateInfinibandInterfacesList, Enter");
	
	int 										retval = SUCCESS;
	vector <InterfaceInfo>	 :: iterator		it;
	string										command;
	string										fullLineString;
	char										sTag[BUFF32];
	char										tempstr[BUFF512];
	
	// Loop thru all interfaces and get all the data we need
	for ( it = vecInterfaces.begin(); (it != vecInterfaces.end()); it++ ) {	
		
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tUpdating Interfaces ENTRY pciAddr=%s Name=%s", it->pciAddress.c_str(), it->ifName.c_str() );
		//
		// Search for IP Address, ifconfig with the interface name gives us what we want
		//
		command.clear();
		sprintf(tempstr,
			"/sbin/ifconfig %s | grep 'inet addr:' | awk -F ':' '{print $2}' | awk -F ' ' '{print $1}' ", 
			it->ifName.c_str());

		command.assign(tempstr);
		GetStringFromCmd(command, it->ipAddress);
		
		//
		// Now onto Link Status
		//
		command.clear();
		sprintf(tempstr, "/usr/sbin/ibstat %s %d",  it->adapterName.c_str(), it->portNum);
		command.assign(tempstr);
		GetBlockOutput(command, fullLineString);

		string junk = fullLineString;
		strcpy( sTag, "State:" );
		GetTagValue(junk, sTag, it->ibstate);

		strcpy( sTag, "Physical state:" );
		GetTagValue(fullLineString, sTag, it->linkStatus);
		
		//
		// Search for Link Status (Link Detected) and and Link Rate fields
		//
		// Next field - LMC
		
		strcpy( sTag, "LMC:" );
		GetTagValue(fullLineString, sTag, it->lmc);
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tLMC=|%s|", it->lmc.c_str() );
		// Next field - SM lid
		strcpy( sTag, "SM lid:" );
		GetTagValue(fullLineString, sTag, it->smLID);
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tSM LID=|%s|", it->smLID.c_str() );

		// Next field - Base lid
		strcpy( sTag, "Base lid:" );
		GetTagValue(fullLineString, sTag, it->baseLID);
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tBase LID=|%s|", it->baseLID.c_str() );
			
		//
		// get linkRate with values
		//
		command.clear();
		sprintf(tempstr,"/usr/sbin/ibstatus %s:%d | grep rate | awk -F \":\" '{print $2}' ", 
			it->adapterName.c_str(), it->portNum);
		command.assign(tempstr);
		GetStringFromCmd(command, it->linkRate);
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tlinkRate=|%s|", it->linkRate.c_str() );
		//
		// Now onto Port Counters
		//
		//perfquery -C mthca0 -P 1 | sed 's/\./ /g'  -- note: always use 1 appended
		command.clear();
		sprintf(tempstr, "/usr/sbin/perfquery -C %s %d 1 | sed 's/\\./ /g' ", it->adapterName.c_str(), it->portNum);
		command.assign(tempstr);
		it->portCounters.clear();
		GetBlockOutput(command, it->portCounters);
		
		//
		// Determine the network interfaces' HEALTH STATE
		//
		DetermineHealthState( it->ifName, it->healthState );	
	
	} // End for loop	

	//	 Should have our interfaces list now all updated with data
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "updateInfinibandInterfacesList, Exit. retval=%d", retval );

	return( retval );
}

/**
 * InfinibandAdapter::DetermineHealthState
 * Given the ifName, return the Health State of the interface based on a number of factors 
 *
 * To determine health:
 * 	Simply determine if PORT 1 has LinkUp, since PORT 2 is irrelevant 
 *
 * Returns: The HealthState enumerator field is set in the ObjectHealthState class instance 
 *				passed in by reference.
 */
void InfinibandAdapters::DetermineHealthState( string adapterName, ObjectHealthState &state )
{
	char			line[BUFF256];
	string			linestr;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdapters::DetermineHealthState, Enter.  adapterName = %s", adapterName.c_str() );
	
	ObjectHealthState::ObjHealthStateEnumerator tempState =  ObjectHealthState::STATE_OK;
	
	// "/usr/sbin/ibstatus  adapterName:1 | grep -i 'phys state' | awk -F \":\" '{print $3}'"
	
	string command = "/usr/sbin/ibstatus ";
	command.append( adapterName );
	command.append( ":1 | grep -i 'phys state' | awk -F \":\" '{print $3}' " );
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tcommand=|%s|", command.c_str() );
	
	FILE *stream = popen(command.c_str(), "r");
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", command.c_str());
	if ( stream == NULL ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "InfinibandAdapters DetermineHealthState: Failed to get stream handle");
	} else {
		if (fgets(line, BUFF256, stream) != NULL) {
			if( strlen( line ) > 0 ) {
				line[strlen(line) - 1] = '\0';
			}

			linestr.assign(line);
			trimSpaces(linestr);
			if (strcasecmp(linestr.c_str(),"LINKUP") != 0) {
				tempState =  ObjectHealthState::STATE_CRITICAL;
				traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\adapterName=|%s| healthState = Critical", adapterName.c_str());
			}
		}
		pclose( stream );	// Close the stream
	}
	
	state.setHealthState( tempState );			// Set the now determined state

	if( TRACING ) {
		string theState = state.getHealthStateString( tempState );
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "InfinibandAdapters::DetermineHealthState, Exit.  Returning State = %s", theState.c_str());	
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdapters::DetermineHealthState, Exit");

}

/**
 * InfinibandAdapter::getManagementOverallHealth
 *
 *	This will be the Infiniband Adapter object Status field for the Management Port interface defined by its PCI address. 
 *		The Management Port interface is defined in the Manufacturing configuration file.
 * 	a) read Management port interface from Configuration file (network.*.management.interface).
 *		b) see algorithm - Associate a Infiniband Interface to an Ethernet PCI Address (PCIAddress)
 *
 * ASSOCIATE A NETWORK INTERFACE TO AN ETHERNET PCI ADDRESS
 * o	Find PCI address of the card. Save value as PCI_address
 * # lspci –D | grep –i Ethernet (or InfiniBand)
 * o	This will show a link with the interface name. Save value as Interface_name. 
  * If there are more than one interfaces then the count of those should be kept as Port_count.
 * # ls -l /sys/bus/pci/devices/ PCI_address/net*
 *
  * 	c) get Infiniband Adapter object (PCIaddress)
 * 	d) get the object status
 *
 */
string InfinibandAdapters::getManagementOverallHealth( void )
{
	string 												state = "unknown";
	vector <AdapterInfo>	 :: iterator		adIt;
	bool													found = false;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdapters::getManagementOverallHealth, Enter");

	// Get mgmt interface name from config file
	if( strlen( SysConfigData.ManagementIf ) <= 0 ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "\t ==> MgmtIf is blank, so returning string |%s|", state.c_str() );
		return state;		// Unknown
	}
	
	// Associate the name
	// Loop thru all Infiniband Adapters, match the ManagementIf name to one in that vector
	for ( adIt = vecAdapters.begin(); (adIt != vecAdapters.end()) && !found; adIt++ ) {	
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tCompare MgmtIf |%s| to adapter |%s|", SysConfigData.ManagementIf, adIt->interfaceName.c_str());
		if( adIt->interfaceName.compare( SysConfigData.ManagementIf ) == 0 ) {

			found = true;		// Set the flag

			// Get the Infiniband Object status
			state = adIt->healthState.getHealthStateString(adIt->healthState.getHealthState());
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \t\tFOUND! the state of that Adapter is %s", state.c_str() );
		}
	}  // End for
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InfinibandAdapters::getManagementOverallHealth, Exit; HealthState is |%s|", state.c_str() );
	return  state;
}


/**
 * InfinibandAdapter::associatePCIAddrToInterfaceName
 *
 *
 */
string InfinibandAdapters::associatePCIAddrToInterfaceName( const char *pciAddress  )
{
	string theString;
	int	count = 0;
	char  fileName[BUFF256];
	char  line[BUFF256];

	traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tassociatePCIAddrToInterfaceName, pciAddr=%s", pciAddress );
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
			
	} else {		
		
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
			
				char 												copyPciAddr[BUFF64];	

				strcpy( copyPciAddr, pciAddress );						// Make a copy

				// Special case where we are adding a raw PCI Address without the appended digit
				// We need to ensure it has no appended digit, as in the case of a InfinibandAdapters <PCI Address>
				// request coming in
				char 	*dotNum = strrchr( copyPciAddr, '.' );		// Search for last period
				if( dotNum != NULL ) {
						*dotNum = '\0';		// Truncate at the period
				}

				// The ".0" must be added to ensure we have a domain of some sort, even if it's a duplicate!
				//createInfinibandInterface( copyPciAddr, ".0", devName+1 ); 
			}
			
		}

		pclose( ifStream );		// Close the open stream

	}
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "seamonlx, \tassociatePCIAddrToInterfaceName, exit; returning string |%s|", theString.c_str() );

	return( theString );
		
}

/* End of InfinibandAdapters.cpp */
