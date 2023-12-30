/** @file StorageAdapters.cpp
 *
 *  StorageAdapter class member methods implementation.
 *
 *  Revision History
 *
 *  03-10-2010 Created ( jie.zhang@schange.com)
 *
 *  04-29-2010  jiez
 *    - Modify the enclosure id to save both sgname and sas address.
 *    - Modify the Phy number to contain only digit.
 *
 *  05-14-2010
 *    - Added implementation of creating the children elements during updating the adapter object.
 *    - Added implementation of clearing the adapter list.
 *  
 *  05-21-2010 mjc 	Fixed missing pclose() call
 *
 *
 *
 *  todo:
 *  - Add read/write lock to synchronous reading/writing from multiple threads.
 *  - trace and error logging code may need more work when requirement is defined.
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include "common.h"
#include "StorageAdapters.h"
#include "Disks.h"
#include "EnclProcessors.h"

using namespace std;
using namespace seamonlx;

/**
 * Copy constructor of class Phy
 */ 
StorageAdapters::Adapter::Phy::Phy(const Phy& other)
{
	phynum = other.phynum;
	sas_addr = other.sas_addr;
	port = other.port;
	linkstatus = other.linkstatus;
	linkrate = other.linkrate;
	minrate = other.minrate;
	maxrate = other.maxrate;
	invalid_dword_count = other.invalid_dword_count;
	loss_of_dword_sync_count = other.loss_of_dword_sync_count;
	phy_reset_problem_count = other.phy_reset_problem_count;
	running_disparity_error_count = other.running_disparity_error_count;
}


/**
 * Assignment operator
 */
StorageAdapters::Adapter::Phy&
StorageAdapters::Adapter::Phy::operator=(const Phy& other)
{
	if( &other != this ){
		phynum = other.phynum;
		sas_addr = other.sas_addr;
		port = other.port;
		linkstatus = other.linkstatus;
		linkrate = other.linkrate;
		minrate = other.minrate;
		maxrate = other.maxrate;
		invalid_dword_count = other.invalid_dword_count;
		loss_of_dword_sync_count = other.loss_of_dword_sync_count;
		phy_reset_problem_count = other.phy_reset_problem_count;
		running_disparity_error_count = other.running_disparity_error_count;
	}
	
	return *this;
}

/**
 * Function clear
 *
 * clear all the data members of class Phy
 */ 
inline void
StorageAdapters::Adapter::Phy::clear()
{
	phynum.clear();
	sas_addr.clear();
	port.clear();
	linkstatus.clear();
	linkrate.clear();
	minrate.clear();
	maxrate.clear();
	invalid_dword_count = 0;
	loss_of_dword_sync_count = 0;
	phy_reset_problem_count = 0;
	running_disparity_error_count = 0;
}

/**
 * a copy constructor of inner class Adapter
 */
StorageAdapters::Adapter::Adapter( const StorageAdapters::Adapter &other )
{
	setPciaddr( other.getPciaddr() );
	setDrname( other.getDrname() );
	setDrver( other.getDrver() );
	setFwver( other.getFwver() );
	setStatus( other.getStatus() );
	setVendorid( other.getVendorid() );
	setDeviceid( other.getDeviceid() );
	setVendor( other.getVendor() );
	setFwver( other.getFwver() );
	setPhys( other.getPhys() );
	setChildelem( other.getChildelem() );
}




/**
 * assignment operator
 */ 
StorageAdapters::Adapter&
StorageAdapters::Adapter::operator=(const Adapter& other)
{
	if( &other != this ){
	
		setPciaddr( other.getPciaddr() );
		setDrname( other.getDrname() );
		setDrver( other.getDrver() );
		setFwver( other.getFwver() );
		setStatus( other.getStatus() );
		setVendorid( other.getVendorid() );
		setDeviceid( other.getDeviceid() );
		setVendor( other.getVendor() );
		setFwver( other.getFwver() );
		setPhys( other.getPhys() );
		setChildelem( other.getChildelem() );
	}
	
	return *this;
}



/**
 * Function setPhys
 * The mutator to set the datamember phys. The old value of phys will
 * be dropped and a copy of input vector will be assigned to phys.
 *
 * @param[IN] an array of phys to be copied.
 * 
 */

void
StorageAdapters::Adapter::setPhys(const vector<Phy> in)
{
	phys.clear();
	unsigned int i;
	for (i = 0; i != in.size(); i ++ ){
		Phy myphy =  in[i];
		phys.push_back(myphy);
	}
}



/**
 * Function setChildelem
 * The mutator to set the datamember childelem. The old value of childelem will
 * be dropped and a copy of input vector will be assigned to it.
 *
 * @param[IN] an array of childelem to be copied.
 * 
 */

void
StorageAdapters::Adapter::setChildelem( const vector<Child> in)
{
	childelem.clear();
	unsigned int i;
	for ( i = 0; i != in.size(); i ++){
		Child myelem;
		myelem.id = in[i].id;
		myelem.type = in[i].type;
		childelem.push_back(myelem);
	}
	
}


/**
 * Function update
 *
 * The function that updates one adapters with the up-to-date
 * values.
 *
 * @return status of updating, SUCCESS or FAILURE.
 */
int
StorageAdapters::Adapter::update()
{
		
	int retval = SUCCESS;

	if( pciaddr.empty() ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "StorageAdapters.cpp Adapter::update(): Error updating adapter, PCI address empty.");
		return FAILURE;
	}

	traceClass->LogTrace(ZQ::common::Log::L_INFO, "StorageAdapters.cpp Adapter::update(): Updating adapter %s", pciaddr.c_str());
	
	/**
	 * update the driver name
	 */
 	if ( updateDrinfo() != SUCCESS ){
 		retval = FAILURE;
 	}

	/**
	 * update vendor name
	 */ 
	if ( updateVendor() != SUCCESS ){
		retval = FAILURE;
	}
	
	/**
	 * update Firmware version
	 */ 
	if ( updateFwver() != SUCCESS ){
		retval = FAILURE;
	}
	
	/**
	 * update adapter status
	 */ 
	if ( updateStatus() != SUCCESS ){
 		retval = FAILURE;
 	}
	
	/**
	 * update phys information
	 */ 
	if ( updatePhys() != SUCCESS ){
 		retval = FAILURE;
 	}

	/**
	 * update child element list
	 */
	if ( updateChildelem() != SUCCESS ){
		retval = FAILURE;
	}

	/**
	 * create children elements
	 */
	if ( createChildelems() != SUCCESS ){
		retval = FAILURE;
	}
	
	
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "StorageAdapters.cpp Adapter::update(): Update adapter %s DONE.", pciaddr.c_str());
	
	return retval;
}




/**
 * Function updateDrname
 *
 * The function that updates the driver name of the adapter
 *
 * @return status of updating, SUCCESS or FAILURE.
 */
int
StorageAdapters::Adapter::updateDrinfo()
{
	int retval = SUCCESS;
		
	string name;	
	/**
	 * update the driver name
	 */
	string command = "ls -l /sys/bus/pci/devices/";
	command.append(pciaddr);
	command.append("/driver");

traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", command.c_str());
	FILE *stream = popen(command.c_str(), "r");
	if ( stream == NULL ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "StorageAdapters.cpp Adapter::updateDrinfo(): Failed updating driver information.");
		return FAILURE;
	}

	
	char text[255];
	
	string dn;
	while (fgets(text, sizeof(text), stream)){ 
		dn = text;
		if ( !dn.empty() ){
			trimSpaces(dn);
			size_t pos = dn.find_last_of ("/");
			name = dn.substr(pos+1);
		}
	}
	setDrname( name );
	pclose(stream);

	/**
	 * update the driver version
	 */ 
	if(  name.size() == 0 ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "StorageAdapters.cpp Adapter::updateDrinfo(): Failed -- driver name not defined.");
		return FAILURE;
	}

	string vercmd = "/sbin/modinfo ";
	vercmd.append(name);
	vercmd.append(" 2>&1 | grep \"^version: \"");

traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", vercmd.c_str());
	FILE *vstream = popen(vercmd.c_str(), "r");
	if ( vstream == NULL ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "StorageAdapters.cpp Adapter::updateDrinfo(): Failed executing modinfo.");
		return FAILURE;
	}

	char vtext[255];
	
	string dv;
	string verstr;
	
	while (fgets(vtext, sizeof(vtext), vstream)){ 
		dv = vtext;
		trimSpaces(dv);
		
		if ( ! dv.empty() ){
			size_t pos = dv.find_first_of (":");
			verstr.assign( dv.substr(pos+1) );
			
			trimSpaces(verstr);
		}
	}

	setDrver( verstr );
	pclose(vstream);
	
	return retval;
}




/**
 * Function updateStatus
 *
 * The function that update the status of the adapter
 *
 * @return status of updating, SUCCESS or FAILURE.
 */
int
StorageAdapters::Adapter::updateStatus()
{
	int retval = SUCCESS;
	
	if ( !drname.empty() ) {
		
		string statuscmd = "/sbin/lsmod | grep -w ";
		statuscmd.append("\"^");
		statuscmd.append(drname);
		statuscmd.append(" \"");

		int exitcode;
		exitcode = system(statuscmd.c_str());
		int exit = WEXITSTATUS(exitcode);
		
		if(exit == 0 ){
			
			setStatus( "OK" );
		}
		else if (exit == 1){
			
			setStatus( "Critical" );
		}
		else{
			traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "StorageAdapters.cpp Adapter::updateStatus(): Unknown exit code of lsmod.");
			return FAILURE;
		}
		
	}	
	else{
		retval = FAILURE;
	}

	return retval;
}



/**
 * Function updateVendor
 * The function that update the Vendor, vendor id and
 * device id of adapter
 *
 * @return status of updating, SUCCESS or FAILURE.
 */
int
StorageAdapters::Adapter::updateVendor()
{
	int retval = SUCCESS;
			
	if ( !pciaddr.empty() ){
		string cmd = "/sbin/lspci -Dn | grep ";
		cmd.append(pciaddr);
		cmd.append(" | awk '{print $3}'");		
		
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
		FILE *stream = popen(cmd.c_str(), "r");
		if ( stream == NULL ){
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "StorageAdapters.cpp Adapter::updateVendor(): Failed updating Vendor information.");
			return FAILURE;
		}
		
		char vtext[255];
		string vendorstr;
		
		while (fgets(vtext, sizeof(vtext), stream)){ 
			vendorstr = vtext;
			trimSpaces(vendorstr);
			if ( ! vendorstr.empty() ){
				size_t pos = vendorstr.find_first_of (":");
				setVendorid( vendorstr.substr(0, pos) );
				setDeviceid( vendorstr.substr(pos+1) );
			}
		}

		pclose( stream );
	}
	
	else{
		retval = FAILURE;
	}

	/**
	 *  translate vendor ID number to vendor name
	 */ 
	if ( !vendorid.compare("117c")){
		setVendor( "Atto Technology" );
		
	}
	
	else if ( ! vendorid.compare( "8086" ) ){
		setVendor( "Intel Corporation" );
	}

	else if ( ! vendorid.compare("103c") ){
		setVendor( "Hewlett-Packard Company" );
	}
	
	else if ( ! vendorid.compare("1000") ){
		setVendor( "LSI Logic/Symbios Logic" );
	}

	// todo: add other supported types
	
	else{
		setVendor( "Unknown" );
	}
	
	return retval;
}



/**
 * Function updateFwver
 * The function that update the firmware version
 * only lsi and atto card are supported.
 *
 * @return status of updating, SUCCESS or FAILURE.
 */
int
StorageAdapters::Adapter::updateFwver()
{

	int retval = SUCCESS;
	
	string cmd;
	
	/* LSI SAS1068E PCI-Express Fusion-MPT SAS adapter */
	if( (! vendorid.compare("1000")) && (! deviceid.compare("0058")) ){
		cmd = "/usr/local/seamonlx/bin/lsi_info.pl | grep ";
		cmd.append(pciaddr);
		cmd.append(" | grep \"Firmware Version\"");
	}

	/* ATTO adapter */
	else if( (! vendorid.compare("117c")) && (! deviceid.compare("0042")) ){
		string channel = attoGetChannel();

		/* command to get the firmware information */
		cmd = "/usr/local/atto/h6xx/bin/atinfo_x64 -i all -c ";
		cmd.append( channel );
		cmd.append(" | grep \"Firmware Version\"");
	}
	else{
		/* other adapter type not supported for firmware version for now */
		//todo: log error
		return FAILURE;
		
	}
	

	/**
	 * Both outputs has the format "name : value". Parse the string to get the
	 * firmware version value.
	 */ 
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
	FILE *stream = popen(cmd.c_str(), "r");
	if ( stream == NULL ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "StorageAdapters.cpp Adapter::updateFwver(): Failed updating firmware version.");
		return FAILURE;
	}
	
	char text[255];
	string fwstr;
	
	while( fgets(text, sizeof(text), stream) ){
		fwstr = text;
		size_t pos = fwstr.find_last_of(":");
		fwver = fwstr.substr(pos+1);
		trimSpaces(fwver);
	}
	
	pclose( stream );
	
	
	return retval;
	
}




/**
 * Function updatePhy
 * The function that update the phy information.
 * 
 * 
 * This function need to be called to get real time information when
 * an request is received because some data are not static.
 *
 * @return status of updating, SUCCESS or FAILURE.
 */
int
StorageAdapters::Adapter::updatePhys()
{
	int retval = SUCCESS;
	phys.clear();
	
	
	/* LSI SAS1068E PCI-Express Fusion-MPT SAS adapter */
	if( (! vendorid.compare("1000")) && (! deviceid.compare("0058")) ){
		
		string cmd = "/usr/local/seamonlx/bin/lsi_info.pl | grep ";
		cmd.append(pciaddr);
		cmd.append(" | grep Phy");

traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
		FILE *stream = popen(cmd.c_str(), "r");
		if ( stream == NULL ){
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "StorageAdapters.cpp Adapter::updatePhys(): Failed updating phys.");
			return FAILURE;
		}
		
		char vtext[255];
		string phystr;
		
		while (fgets(vtext, sizeof(vtext), stream)){ 
			phystr = vtext;
			trimSpaces(phystr);

			if ( ! phystr.empty() ){
				vector<string> token;
				stringSplit(phystr, "|", token);

				Phy myphy;
				myphy.phynum = token[1];
				myphy.sas_addr = token[2];
				myphy.port = token[3];
				myphy.linkstatus = token[4];
				myphy.linkrate = token[5];
				myphy.minrate = token[6];
				myphy.maxrate = token[7];
				myphy.invalid_dword_count = atoi(token[8].c_str());
				myphy.loss_of_dword_sync_count = atoi(token[9].c_str());
				myphy.phy_reset_problem_count = atoi(token[10].c_str());
				myphy.running_disparity_error_count = atoi(token[11].c_str());

				phys.push_back(myphy);
			}
			
		}

		pclose( stream );
	}

	
	/* ATTO adapter */
	else if( (! vendorid.compare("117c")) && (! deviceid.compare("0042")) ){
		string channel = attoGetChannel();

		string cmd = "/usr/local/atto/h6xx/bin/atsasphy_x64 -i -c ";
		cmd.append(channel);

traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
		FILE *stream = popen(cmd.c_str(), "r");
		if ( stream == NULL ){
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "StorageAdapters.cpp Adapter::updatePhys(): Failed updating phys.");
			return FAILURE;
		}
		
			
		Phy myphy;
		unsigned int numOfPhysToSave = 0;
		
		char text[255];
		string buf;
		
		while (fgets(text, sizeof(text), stream)){ 
			buf = text;
			trimSpaces(buf);

			/* the line contains phy number */
			if ( buf.find("Information") != string::npos ){
				if( numOfPhysToSave > 0 ){
					phys.push_back(myphy);
					myphy.clear();
					numOfPhysToSave = 0;
				}
				
				size_t pos = buf.find("Information");
				buf.resize(pos); /* remove "Information" from string*/
				trimSpaces(buf);
				
				myphy.phynum = buf.erase(0, 4); /* Remove 'PHY ' from string*/
				numOfPhysToSave ++;
			}

			/* get port id */
			else if( buf.find("Port ID") != string::npos ){
				size_t pos = buf.find_first_of(":");
				if ( pos != string::npos ){
					myphy.port = buf.substr(pos+1);
					trimSpaces(myphy.port);
				}
			}

			/* get negotiated link rate */
			else if( buf.find("Negotiated Rate") != string::npos ){
				size_t pos = buf.find_first_of(":");
				if ( pos != string::npos ){
					myphy.linkrate = buf.substr(pos+1);
					trimSpaces(myphy.linkrate);
				}
			}

			/* get Minimum Rate */
			else if( buf.find("Minimum Rate") != string::npos ){
				size_t pos = buf.find_first_of(":");
				if( pos != string::npos ){
					myphy.minrate = buf.substr(pos+1);
					trimSpaces(myphy.minrate);
				}
			}
			
			/* get Maximum Rate */
			else if( buf.find("Maximum Rate") != string::npos ){
				size_t pos = buf.find_first_of(":");
				if( pos != string::npos ){
					myphy.maxrate = buf.substr(pos+1);
					trimSpaces(myphy.maxrate);
				}
			}

			/* get Invalid Dword Count */
			else if( buf.find("Invalid Dword Count") != string::npos ){
				size_t pos = buf.find_first_of(":");
				if( pos != string::npos ){
					string idc = buf.substr(pos+1);
					trimSpaces( idc );
					myphy.invalid_dword_count = atoi( idc.c_str());
					
				}
			}

			/* Disparity Error Count */
			else if( buf.find("Disparity Error Count") != string::npos ){
				size_t pos = buf.find_first_of(":");
				if( pos != string::npos ){
					string rdec = buf.substr(pos+1);
					trimSpaces(rdec);
					myphy.running_disparity_error_count = atoi( rdec.c_str() );
				}
			}
			
			/* Loss Of Dword Sync Count */
			else if( buf.find("Loss Of Dword Sync Count") != string::npos ){
				size_t pos = buf.find_first_of(":");
				if ( pos != string::npos ){
					string ldsc = buf.substr(pos+1);
					trimSpaces(ldsc);
					myphy.loss_of_dword_sync_count = atoi( ldsc.c_str() );
				}
			}
			

			/* PHY Reset Error Count */
			else if( buf.find("PHY Reset Error Count") != string::npos ){
				size_t pos = buf.find_first_of(":");
				if( pos != string::npos ){
					string prec = buf.substr(pos+1);
					trimSpaces( prec );
					myphy.phy_reset_problem_count = atoi( prec.c_str() );
				}
			}
			else{
				continue;
			}
		}
		
		pclose( stream );		

		/* push back the last phy element */
		if( numOfPhysToSave ){
			phys.push_back(myphy);
			myphy.clear();
		}
	}
	return retval;

}

/**
 * Function updateChildelem
 *
 * Get the child elements' type and ids.
 * @return status of updating, SUCCESS or FAILURE.
 */
int
StorageAdapters::Adapter::updateChildelem()
{
	int retval = SUCCESS;
	
	childelem.clear();
	
	string cmd = "/usr/local/seamonlx/bin/findAdapterChildelem.pl ";
	cmd.append(pciaddr);
	
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
	FILE *stream = popen(cmd.c_str(), "r");
	if ( stream == NULL ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "StorageAdpaters.cpp::updateChildelem(): Failed updating child elements");
		return FAILURE;
	}
	
	
	char text[255];
	string buf;
	
	while (fgets(text, sizeof(text), stream)){ 
		buf = text;
		trimSpaces(buf);

		size_t pos = buf.find_first_of(":");
		if ( pos != string::npos ){
			
			string mytype = buf.substr(0, pos);
			string myid = buf.substr(pos+1);
			trimSpaces(mytype);
			trimSpaces(myid);
//			traceClass->LogTrace(ZQ::common::Log::L_INFO, "child element: id:%s, type:%s", myid.c_str(), mytype.c_str());
			// get the sg_name
//			size_t pos = myid.find_first_of("(");
//			string sgname= myid.substr(0, pos);
			
			Child mychild;
//			mychild.id = sgname;
			mychild.id = myid;
			mychild.type = mytype;
			childelem.push_back(mychild);
		}
		
	}
	
	pclose( stream );		
	
	return retval;
}


/**
 * Function createChildelems()
 *
 * Function that creates the child elements of the adapter
 * and set the parent pointer of the child elements.
 *
 * @return status of updating, SUCCESS or FAILURE.
 */ 
int
StorageAdapters::Adapter::createChildelems()
{
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "StorageAdapters.cpp StorageAdapters::Adapter::createChildelems() Creating the Child elements ......");

	vector<Child> childlist = getChildelem();

	for( unsigned int i = 0; i < childlist.size(); i ++){
		string myid = childlist[i].id;
		size_t pos = myid.find_first_of("(");
		string id= myid.substr(0, pos);
		
		if( ! childlist[i].type.compare("SCSI Disk")){
			Disks *disklistP = Disks::instance();
			disklistP->addDisk(id, getAddress());
		}
		else if( ! childlist[i].type.compare("Enclosure")){
			EnclProcessors *encllistP = EnclProcessors::instance();
			encllistP->addEncl(id, getAddress());
		}
		
		/**
		 * todo
		 * Add other type of children 
		 */
	}
	return SUCCESS;
	
}



/**
 * Function getAttoChannel
 *
 * Helper function used to get the channel of atto card
 * @return channel string
 */
string
StorageAdapters::Adapter::attoGetChannel()
{

	/* convert pci_address to decimal */
	int  domain, bus, slot, func;
	sscanf(pciaddr.c_str(), "%x:%x:%x.%x", &domain, &bus, &slot, &func);

	char pci[20];
	sprintf(pci, "%d:%d.%d", bus, slot, func);

	string pciloc(pci);
	
	/* run the atto utility to get the channel number */
	string cmd = "/usr/local/atto/h6xx/bin/atinfo_x64 -l | grep ";
	cmd.append(pciloc);
	cmd.append(" | awk '{print $1}'");

	string channel;
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
	FILE *stream = popen(cmd.c_str(), "r");
	if ( stream != NULL ){
		char vtext[255];
		while (fgets(vtext, sizeof(vtext), stream)){ 
			channel = vtext;
			trimSpaces(channel);				
		}
		
		pclose( stream );			
		
	}
	
	return channel;
}

/***************************** StorageAdapters *******************************/

StorageAdapters *StorageAdapters::s_instance = 0;

/**
 * Constructor
 */ 
StorageAdapters::StorageAdapters()
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "StorageAdapters.cpp : Construct StorageAdapters Object.");
	if( s_instance){
		traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "StorageAdapters.cpp : Whoops! Throwing!");
		throw "Attempting to create second StorageAdapter List";
	}
	s_instance = this;
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "StorageAdapters.cpp : Construct StorageAdapters Object EXIT");
}

/**
 * Function clear
 *
 * Function that delete all enclosures from the encls vector.
 * This function can be called to free the memory before program terminates.
 *
 */
void StorageAdapters::clear( )
{
	vector<Adapter *>::iterator i;
	for( i = adapters.begin(); i != adapters.end(); i ++){
		delete *i;
	}
	adapters.clear();
}



/**
 * Function update 
 *
 * The function that updates the adapters list with the up-to-date
 * value.
 *
 * @return status of updating, SUCCESS or FAILURE.
 */
int
StorageAdapters::update()
{

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "StorageAdapters.cpp update(): Updating adapters START.");
	
	int retval = SUCCESS;
	
	/**
	 * execute command to get the storage adapter ids.
	 */ 
	string listcmd = "lspci -Dn | awk '{if ($2 ~ /^01[0-9]+:$/) print $1}'";
	
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", listcmd.c_str());
	FILE *stream = popen(listcmd.c_str(), "r");
	if ( stream == NULL ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "StorageAdapters.cpp update(): Failed getting adapter list.");
		return FAILURE;
	}

	/**
	 * clear the old data in the adapters list.
	 */ 
	adapters.clear();

	/**
	 * find the adapters and add them to the adapters list.
	 */
	char text[255];
	
	string id;
	while (fgets(text, sizeof(text), stream)){ 
		id = text;
		if ( !id.empty() ){
			trimSpaces(id);
			StorageAdapters::Adapter *element = new Adapter;
			element->setPciaddr(id);
			retval = element->update();
			adapters.push_back(element);
		}
	}

	pclose(stream);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "StorageAdapters.cpp update(): Updating adapters DONE.");

	return retval;
}

				
void
StorageAdapters::setAdapters(const vector<Adapter *> in)		
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "StorageAdapters.cpp setAdapters(): Enter");
	adapters.assign( in.begin(), in.end() );
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "StorageAdapters.cpp setAdapters(): Exit");
}
