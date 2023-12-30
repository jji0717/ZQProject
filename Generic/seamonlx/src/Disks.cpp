/** @file Disks.cpp
 *
 *  Disks class member methods implementation.
 *
 *  Revision History
 *
 *  04-23-2010 jiez
 *    - Created.
 *    
 *  05-10-2010 
 *   - Changed data member of class Disks to contain pointers instead of the DiskDrive objects.
 *   - Changed constructor to allow only one instance of Disks exists.
 *   - Added d_instance pointer to Disks object and ref_count to keep tracking of the references
 *     of the instance
 *   - Added member functions addDisk(), deleteDisk() and clear() to outer class Disks
 *   - Added constructor that takes sgname of disk of inner class DiskDrive
 *   - Added parent pointers to the parent device instance in the inner class DiskDrive
 *
 *  05-21-2010 mjc 	Fixed missing pclose() calls
 *    
 *  todo:
 *  - Add read/write lock to synchronous reading/writing from multiple threads.
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include "common.h"
#include "Disks.h"

using namespace std;
using namespace seamonlx;



/************************* Outer class Disks *************************/
Disks *Disks::d_instance = 0;

Disks::Disks()
{
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "Disks.cpp : Construct Disks Object.");
	
	if( d_instance){
		throw "Attempting to create second Disk list";
	}
	d_instance = this;

}


/**
 * Function update
 *
 * Function updates the disk list with the current values.
 *
 * @return status of updating, SUCCESS or FAILURE.
 * 
 */
int Disks::update()
{

	traceClass->LogTrace(ZQ::common::Log::L_INFO, "Disks.cpp update(): Updating disks START.");

	int retval = SUCCESS;

	/**
	 * get the disk sgnames. 
	 */ 
	string listcmd = "/usr/bin/sg_map -x | awk '{if ($6 ~ /0/) print $1}'";
	FILE *stream = popen(listcmd.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", listcmd.c_str());
	if ( stream == NULL ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "Disks.cpp update(): Failed getting disk list.");
		return FAILURE;
	}

	
	/**
	 * find the disks and add them to the list.
	 */
	char text[255];

	string localid;
	while(fgets(text, sizeof(text), stream)){
		localid = text;
		trimSpaces(localid);

		/**
		 * get the block device name 
		 */ 
		size_t pos = localid.find_last_of("/");
		string disksgname = localid.substr( pos+1 );

		addDisk(disksgname);
	}

	pclose( stream );
	
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "Disks.cpp update(): Updating disks DONE.");
	
	return retval;
}

/**
 * Function addDisk
 *
 * Function that add one disk to the Disks vector
 *
 * @param[in] disksg the sgname of the disk.
 * @param[in] parentP the pointer to the parent device
 * @return status of updating, SUCCESS or FAILURE.
 * 
 */
int Disks::addDisk( string disksg, const EnclProcessors::Enclosure *parentP )
{
	/**
	 * If the disk exists, return.
	 */
	int diskExists = 0;
	vector<DiskDrive *>::iterator dit;
	for ( dit = disks.begin(); dit != disks.end(); dit ++){
		if ( ! (*dit)->getSgname().compare(disksg)){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "Disks.cpp Disks::addDisk(): Disk %s exists.", disksg.c_str());
			diskExists = 1;
		}
	}
	if(diskExists){
		return SUCCESS;
	}

	DiskDrive *element = new DiskDrive(disksg);

	/* set the parent pointer */
	if( parentP ){
		element->setParent(parentP);
	}

	/* build the disk and insert it to disks vector*/	
	disks.push_back(element);
	
	return SUCCESS;
}


/**
 * Function addDisk
 *
 * Function that add one disk to the Disks vector
 *
 * @param[in] diskbd the block device name of the disk.
 * @param[in] parentP the pointer to the parent device
 * @return status of updating, SUCCESS or FAILURE.
 * 
 */
int Disks::addDisk( string diskbd, const StorageAdapters::Adapter *parentP )
{
	/**
	 * If the disk exists, return.
	 */
	int diskExists = 0;
	vector<DiskDrive *>::iterator dit;
	for ( dit = disks.begin(); dit != disks.end(); dit ++){
		if ( ! (*dit)->getBdname().compare(diskbd)){
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "Disks.cpp Disks::addDisk(): Disk %s exists.", diskbd.c_str());
			diskExists = 1;
		}
	}
	if(diskExists){
		return SUCCESS;
	}

	DiskDrive *element = new DiskDrive;
	element->setBdname(diskbd);
	element->update();
	
	/* set the parent pointer */
	if( parentP ){
		element->setParent(parentP);
	}
	
	/* build the disk and insert it to disks vector*/	
	disks.push_back(element);
	
	return SUCCESS;
}


/**
 * Function addDisk
 *
 * Function that add one disk to the Disks vector
 *
 * @param[in] diskbd the sgname of the disk.
 * @return status of updating, SUCCESS or FAILURE.
 * 
 */
int Disks::addDisk( string disksg)
{

    /**
	 * If the disk exists, return.
	 */
	int diskExists = 0;
	vector<DiskDrive *>::iterator dit;
	for ( dit = disks.begin(); dit != disks.end(); dit ++){
		if ( ! (*dit)->getSgname().compare(disksg)){

			// trace the parent
			string parentId;
			if((*dit)->getPEncl()){
				parentId = (*dit)->getPEncl()->getId();
			}
			else if((*dit)->getPAdpt()){
				parentId = (*dit)->getPAdpt()->getPciaddr();
			}
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "*** Disks.cpp Disks::addDisk(): Disk %s exists. parent is %s",
					  disksg.c_str(), parentId.c_str());
			diskExists = 1;

			//test delete disk function 
 	// 		if(!disksg.compare("sg12")){
//  				deleteDisk(disksg);
// 				diskExists = 0;
//  			}
			// test done
			
		}
	}
	if(diskExists){
		return SUCCESS;
	}

	DiskDrive *element = new DiskDrive(disksg);

	/**
	 * find the parent device if there is any and set the parent pointers.
	 */
	
	string cmd = "/usr/local/seamonlx/bin/findDiskParent.pl ";
	cmd.append(disksg);
	FILE *stream = popen(cmd.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
	if ( stream != NULL ){
		char text[255];
		string line;
		
		while(fgets(text, sizeof(text), stream)){
			line = text;
			size_t pos = line.find_first_of(":");
			string key = line.substr(0, pos);
			string value = line.substr(pos+1);
			trimSpaces(key);
			trimSpaces(value);
			
			/* Now only enclosure parents can be detected.*/
			if( ! key.compare("Parent Enclosure") ){
				traceClass->LogTrace(ZQ::common::Log::L_INFO, "Disks.cpp addDisk(): Add disk %s, updating the parent children list %s",
						  disksg.c_str(), value.c_str());
				EnclProcessors *parP = EnclProcessors::instance();
				vector<EnclProcessors::Enclosure *> encls = parP->getEncls();
				for (unsigned int i = 0; i < encls.size(); i++ ){
					if ( !( encls[i]-> getId()).compare(value)){
						encls[i]->updateDisklist();
						element->setParent(encls[i]->getAddress());
					}
				}
			}
			/**
	         * todo: Add code to detect other type of parents.
			 */ 
		}
		pclose(stream);
		
	}
	
	/* build the disk and insert it to disks vector*/	
	disks.push_back(element);

	return SUCCESS;
}


/**
 * Function deleteDisk
 *
 * Function that delete one disk from the Disks vector.
 * This function will be called when a drive is pulled out.
 *
 * @param[IN] disksgname the sgname of the disk to be deleted.
 */
void Disks::deleteDisk( string disksgname )
{
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "Disks.cpp deleteDisk(): Delete Disk %s", disksgname.c_str());

	vector<DiskDrive *>::iterator it;
	
	for (it = disks.begin(); it != disks.end(); it ++){
		if( ! ((*it)->getSgname()).compare(disksgname) ){

			
			/**
			 *  code to update the parent's child list.
			 */
			if( (*it)->getPEncl() ){
				traceClass->LogTrace(ZQ::common::Log::L_INFO, "Disks.cpp deleteDisk(): Deleting disk %s, updating the parent children list %s", (*it)->getSgname().c_str(), (*it)->getPEncl()->getId().c_str());
				(*it)->getPEncl()->updateDisklist();
			}
			else if( (*it)->getPAdpt() ){
				(*it)->getPAdpt()->updateChildelem();
			}
			
			/* erase the disk from the disks list */
			delete *it;
			disks.erase(it);
		}
	}
}


/**
 * Function clear
 *
 * Function that delete all disk from the Disks vector.
 * This function can be called to free the memory before program terminates.
 *
 */
void Disks::clear( )
{
	vector<DiskDrive *>::iterator i;
	for( i = disks.begin(); i != disks.end(); i ++){
		delete *i;
	}
	disks.clear();
}


/*************************** Inner class DiskDrive ****************************/


/** 
 * constructor that takes an sgname as input
 *
 */
Disks::DiskDrive::DiskDrive( const string sgname )
{
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "Disks.cpp Disks::DiskDrive(): Creating disk %s", sgname.c_str());
	
	/**
	 * get the disk block device names. 
	 */ 
	string cmd = "/usr/bin/sg_map -x | grep -w ";
	cmd.append("'");
	cmd.append(sgname);
	cmd.append(" '");
	cmd.append("| awk '{print $7}'");
	
	
	FILE *stream = popen(cmd.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
	if ( stream == NULL ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "Disks.cpp addDisk(): Failed getting the block device name.");
		return;
	}

	char text[255];
	string localid;
	while(fgets(text, sizeof(text), stream)){
		localid = text;
		
		trimSpaces(localid);

		size_t pos = localid.find_last_of("/");
		setBdname ( localid.substr( pos+1 ) );
	}
	pclose(stream);
	
	update();

}

/**
 * Function update
 *
 * The function that updates one disks with
 * the current values.
 *
 * @return status of updateing, SUCCESS or FAILURE
 */
int
Disks::DiskDrive::update()
{
	/**
	 * IF block device name is not defined log error and return failure.
	 */ 
	if( bdname.empty() ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "Disks.cpp DiskDrive::update(): Error, bd name of disk not found.");
		return FAILURE;
	}
	
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "Disks.cpp DiskDrive::update(): Updating disk %s START", bdname.c_str());

	string cmd = "/usr/local/seamonlx/bin/diskInfo.pl ";
	cmd.append(bdname);

	FILE *stream = popen(cmd.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
	if ( stream == NULL ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "Disks.cpp DiskDrive::update(): Failed getting disk information.");
		return FAILURE;
	}
	
	/* get the disk data */
	char text[255];
	while(fgets(text, sizeof(text), stream)){
		string line = text;

		size_t pos = line.find_first_of(":");
		string key = line.substr(0, pos);
		string value = line.substr(pos+1);
		trimSpaces(key);
		trimSpaces(value);

		if( !key.compare("SG Name") ){
			setSgname( value );
			
		}
		else if( !key.compare("SCSI Address") ){
			setScsiaddr( value );
			
		}
		else if( !key.compare("Vendor") ){
			setVendor( value );
			continue;

		}
		else if( !key.compare("Model") ){
			setModel( value );
			continue;
			
		}
		else if( !key.compare("Logical Address") ){
			setId( value );
			continue;
			
		}
		else if( !key.compare("Serial Number") ){
			setSerialnum( value );
			continue;
			
		}
		else if( !key.compare("Firmware Version") ){
			setFwver( value );
			continue;
			
		}
		else if( !key.compare("Capacity") ){
			setSize( value );
			continue;
			
		}
		else if( !key.compare("Temperature") ){
			setTemp( value );
			continue;
			
		}
		else if( !key.compare("State") ){
			setState( value );
			continue;
			
		}
		else if( !key.compare("I/O Error") ){
			setIoerr( value );
			continue;
			
		}
		else if( !key.compare("Blocks Read") ){
			setBlkr( value );
			continue;
			
		}
		else if( !key.compare("Blocks Written") ){
			setBlkw( value );
			continue;
			
		}
		else if( !key.compare("Cache Attribute") ){
			setCa( value );
			continue;
			
		}
		else{
			continue;
		}
	}

	if ( getId().empty() ){
		setId(bdname);
	}

	pclose(stream);
	
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "Disks.cpp DiskDrive::update(): Updating disk %s DONE", bdname.c_str());
	return SUCCESS;
	
}


