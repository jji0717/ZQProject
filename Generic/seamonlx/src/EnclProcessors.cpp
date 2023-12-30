/** @file EnclProcessors.cpp
 *
 *  EnclProcessors class member methods implementation.
 *
 *  Revision History
 *
 *  04-23-2010 jiez
 *   - Created
 *
 *  05-06-2010 jiez
 *   - Added sgname and block device name to Disk struct.
 *   
 *  05-12-2010 jiez
 *   - Changed data member of EnclProcessors encls to contain pointers instead of the Enclosure objects.
 *   - Changed constructor to allow only one instance of enclosure exists.
 *   - Changed outer class EnclProcessors singleton. Only one instance exists.
 *   
 *  todo:
 *  - Add read/write lock to synchronous reading/writing from multiple threads.
 *  - Add code to trace errors
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include "common.h"
#include "EnclProcessors.h"
#include "Disks.h"

using namespace std;
using namespace seamonlx;

/************************* Outer class EnclProcessors *************************/
EnclProcessors *EnclProcessors::e_instance = 0;

EnclProcessors::EnclProcessors()
{
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp : Construct EnclProcessors Object.");
	
	if( e_instance){
		throw "Attempting to create second Enclosure list";
	}
	e_instance = this;

}


/**
 * Function clear
 *
 * Function that delete all enclosures from the encls vector.
 * This function can be called to free the memory before program terminates.
 *
 */
void EnclProcessors::clear( )
{
	vector<Enclosure *>::iterator i;
	for( i = encls.begin(); i != encls.end(); i ++){
		delete *i;
	}
	encls.clear();
}


/**
 * Function update
 *
 * Function updates the enclosure list with the current values.
 *
 * @return status of updating, SUCCESS or FAILURE.
 * 
 */
int EnclProcessors::update()
{

	traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp update(): Updating enclosures START.");

	int retval = SUCCESS;

	/**
	 * get the enclosure sgnames. 
	 */ 
	string listcmd = "/usr/bin/sg_map -x | awk '{if ($6 ~ /13/) print $1}'";
	FILE *stream = popen(listcmd.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", listcmd.c_str());
	if ( stream == NULL ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp update(): Failed getting enclosure list.");
		return FAILURE;
	}

	/**
	 * find the enclosures and add them to the list.
	 */
	char text[255];

	string localid;
	while(fgets(text, sizeof(text), stream)){
		localid = text;
		
		/**
		 * assign the sgname to the element.
		 */ 
		trimSpaces(localid);
		size_t pos = localid.find_last_of("/");
		string enclsg = localid.substr( pos+1 );
		addEncl(enclsg);
	}

	pclose( stream );
	
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp update(): Updating enclosures DONE.");
	
	return retval;
}


/**
 * Function addEncl
 *
 * Function that add one enclosure to the encls vector
 *
 * @param[in] enclsg the sgname of the enclosure.
 * @param[in] parentP the pointer to the parent device
 * @return status of updating, SUCCESS or FAILURE.
 * 
 */
int EnclProcessors::addEncl( string enclsg, const StorageAdapters::Adapter *parentP )
{
	/**
	 * If the enclosure exists, return.
	 */
	int enclExists = 0;
	vector<Enclosure *>::iterator dit;
	for ( dit = encls.begin(); dit != encls.end(); dit ++){
		if ( ! (*dit)->getId().compare(enclsg)){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp EnclProcessors::addEncl(): Enclosure %s exists, parent is %s",
				  enclsg.c_str(), (*dit)->getParent()->getPciaddr().c_str());
			enclExists = 1;
		}
	}
	if(enclExists){
		return SUCCESS;
	}

	Enclosure *element = new Enclosure(enclsg);

	/* set the parent pointer */
	if( parentP ){
		element->setParent(parentP);
	}

	/* build the disk and insert it to disks vector*/	
	encls.push_back(element);
	
	return SUCCESS;
}

/** 
 * constructor that takes an sgname as input
 * The enclosures built this way contains all the detailed
 * data and children elements data.
 *
 */
EnclProcessors::Enclosure::Enclosure( const string sgname )
{
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp EnclProcessors::Enclosure(): Creating Enclosure %s", sgname.c_str());
	
	setId(sgname);
	update();
}

/**
 * Function setDisks
 * The mutator to set the data member disks.
 * The old value of disks will be dropped and a copy of input
 * vector will be assigned to it
 */ 
void
EnclProcessors::Enclosure::setDisks( const vector<Disk> in )
{
	disks.clear();
	unsigned int i;
	for( i = 0; i != in.size(); i ++ ){
		Disk localdisk;
		localdisk.seacid = in[i].seacid;
		localdisk.baynumber = in[i].baynumber;
		localdisk.status = in[i].status;
		localdisk.sgname = in[i].sgname;
		localdisk.bdname = in[i].bdname;
		disks.push_back(localdisk);
	}
	
}



/**
 * Function setFans
 * The mutator to set the data member fans
 * The old value will be dropped and a copy of input
 * vector will be assigned to it
 */ 
void
EnclProcessors::Enclosure::setFans( const vector<Envelemt> in )
{
	fans.clear();
	unsigned int i;
	for( i = 0; i != in.size(); i ++ ){
		Envelemt local;
		local.seacid = in[i].seacid;
		local.desc = in[i].desc;
		local.reading = in[i].reading;
		local.status = in[i].status;

		fans.push_back(local);
	}
}

/**
 * Function setPowers
 * The mutator to set the data member powers
 * The old value will be dropped and a copy of input
 * vector will be assigned to it
 */ 
void
EnclProcessors::Enclosure::setPowers( const vector<Envelemt> in )
{
	powers.clear();
	unsigned int i;
	for( i = 0; i != in.size(); i ++ ){
		Envelemt local;
		local.seacid = in[i].seacid;
		local.desc = in[i].desc;
		local.reading = in[i].reading;
		local.status = in[i].status;

		powers.push_back(local);
	}
}



/**
 * Function setTemps
 * The mutator to set the data member temps
 * The old value will be dropped and a copy of input
 * vector will be assigned to it
 */ 
void
EnclProcessors::Enclosure::setTemps( const vector<Envelemt> in )
{
	temps.clear();
	unsigned int i;
	for( i = 0; i != in.size(); i ++ ){
		Envelemt local;
		local.seacid = in[i].seacid;
		local.desc = in[i].desc;
		local.reading = in[i].reading;
		local.status = in[i].status;

		temps.push_back(local);
	}
}


/**
 * Function setPhys
 * The mutator to set the data member Phys
 * The old value will be dropped and a copy of input
 * vector will be assigned to it
 */ 
void
EnclProcessors::Enclosure::setPhys( const vector<Phy> in )
{
	phys.clear();
	unsigned int i;
	for( i = 0; i != in.size(); i ++ ){
		Phy local;
		local.desc = in[i].desc;
		local.phynumber= in[i].phynumber;
		local.sas = in[i].sas;
		local.linkrate = in[i].linkrate;
		local.invDword = in[i].invDword;
		local.disparty = in[i].disparty;
		local.syncLoss = in[i].syncLoss;
		local.rsetProb = in[i].rsetProb;
		
		phys.push_back(local);
	}
}


/**
 * Function update
 *
 * The function that updates one enclosures with
 * the current values.
 *
 * @return status of updateing, SUCCESS or FAILURE
 */
int
EnclProcessors::Enclosure::update()
{
	int retval = SUCCESS;

	/**
	 * IF sgname is not defined log error and return failure.
	 */ 
	if( id.empty() ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::update(): Error, sgname of enclosure not defined.");
		return FAILURE;
	}

	traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::update(): Updating enclosure %s", id.c_str());

	/**
	 * update enclosure information
	 */ 
	pthread_mutex_lock(&EncMutex);
	if( updateEnclinfo() != SUCCESS ){
	
		retval = FAILURE;
	}
	
	/**
	 * update the disks
	 */
	if( updateDisklist() != SUCCESS ){
	
		retval = FAILURE;
	}


	/**
	 * create the disk childrens if they don't
	 * exits.
	 */
	if( createDiskChildren() != SUCCESS ){
		retval = FAILURE;
	}
	
	/**
	 * update the environmental elements
	 */ 
	if( updateFans() != SUCCESS ){
		retval = FAILURE;
	}
	if( updatePowers() != SUCCESS ){
		retval = FAILURE;
	}
	if( updateTemps() != SUCCESS ){
		retval = FAILURE;
	}

	/**
	 * update the phys.
	 */
	if( updatePhys() != SUCCESS ){
		retval = FAILURE;
	}
	
	pthread_mutex_unlock(&EncMutex);

	return retval;
	
}



/**
 * Function updateEnclInfo()
 *
 * Function that updates the disk drives attached to this enclosure.
 *
 * @return status of updating, SUCCESS or FAILURE.
 */ 
int
EnclProcessors::Enclosure::updateEnclinfo()
{
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updateEnclinfo(): Updating enclosure basic information START.");

	/**
	 * If sgname is not defined log error and return failure.
	 */ 
	if( id.empty() ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updateEnclinfo(): Error, sgname of enclosure not defined.");
		return FAILURE;
	}

	string cmd = "/usr/local/seamonlx/bin/enclInfo.pl ";
	cmd.append(id);

	FILE *stream = popen(cmd.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
	if ( stream == NULL ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updateEnclinfo(): Failed getting enclosure information.");
		return FAILURE;
	}

	/* get the current attached disks */
	char text[255];
	while(fgets(text, sizeof(text), stream)){
		string line = text;

		size_t pos = line.find_first_of(":");
		string key = line.substr(0, pos);
		string value = line.substr(pos+1);
		trimSpaces(key);
		trimSpaces(value);
		
		if( !key.compare("Unit Serial Number") ){
			setUnitsn( value );
			
		}
		else if( !key.compare("SAS Address") ){
			setSasaddr( value );
			
		}
		else if( !key.compare("SCSI Address") ){
			setScsiaddr( value );
			continue;
			
		}
		else if( !key.compare("Vendor") ){
			setVendor( value );
			continue;

		}
		else if( !key.compare("Product ID") ){
			setProdid( value );
			continue;
			
		}
		else if( !key.compare("Revision") ){
			setRevision( value );
			continue;
			
		}
		else if( !key.compare("Firmware Version") ){
			setFwversion( value );
			continue;
			
		}
		else if( !key.compare("Overall Status") ){
			setStatus( value );
			continue;
			
		}
		else if( !key.compare("Number of Drive Bays") ){
			setNumofbays( atoi(value.c_str()) );
			continue;

		}
		
	}

	pclose( stream );
	
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updateEnclinfo(): Updating enclosure basic information DONE.");

	return SUCCESS;
	
}

	

	
/**
 * Function updateDisklist()
 *
 * Function that updates the disk drives attached to this enclosure.
 *
 * @return status of updating, SUCCESS or FAILURE.
 */ 
int
EnclProcessors::Enclosure::updateDisklist()
{

	traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updateDisklist(): Updating enclosure %s disk list START", getId().c_str());

	/**
	 * If sgname is not defined log error and return failure.
	 */ 
	if( id.empty() ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updateDisklist(): Error, sgname of enclosure not defined.");
		return FAILURE;
	}

	
	string cmd = "/usr/local/seamonlx/bin/findEnclChildelems.pl ";
	cmd.append(id);

	
	FILE *stream = popen(cmd.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
	if ( stream == NULL ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updateDisklist(): Failed getting enclosure disks.");
		return FAILURE;
	}

	/*clear the old data */
	disks.clear();

	/* get the current attached disks */
	char text[255];

	string diskstr;
	while(fgets(text, sizeof(text), stream)){
		diskstr = text;
		
		Disk local;

		/**
		 * get the id and bay number
		 */ 
		trimSpaces(diskstr);
		size_t pos = diskstr.find_first_of(":");
		string number = diskstr.substr(0, pos);
		trimSpaces(number);
		local.baynumber = number;
		
		size_t spos = diskstr.find_first_of(";");
		size_t sg_start = diskstr.find_first_of("(");
		size_t sg_end = diskstr.find_first_of("|");
		size_t bd_end = diskstr.find_first_of(")");
		
		local.seacid = diskstr.substr(pos+1, (sg_start - pos - 1));
		local.sgname = diskstr.substr(sg_start + 1, (sg_end -sg_start - 1));
		local.bdname = diskstr.substr(sg_end + 1, (bd_end - sg_end -1));
		local.status = diskstr.substr(spos + 1);
		
		trimSpaces(local.seacid);
		trimSpaces(local.status);
		trimSpaces(local.sgname);
		trimSpaces(local.bdname);
		

//		traceClass->LogTrace(ZQ::common::Log::L_INFO, "Bay: %s, SAS: %s, SGNAME: %s, STATUS: %s", local.baynumber.c_str(), local.seacid.c_str(), local.sgname.c_str(), local.status.c_str());

		disks.push_back(local);
		
	}

	pclose( stream );
	
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updateDisklist(): Updating enclosure disk list DONE.");
	
	return SUCCESS;
}


/**
 * Function createDiskChildren()
 *
 * Function that creates the disk childrens in the Disks::disks list
 * and set the parent pointer of the children disks.
 *
 * @return status of updating, SUCCESS or FAILURE.
 */ 
int
EnclProcessors::Enclosure::createDiskChildren()
{


	vector<Disk> disklist = getDisks();
	Disks *disklistP = Disks::instance();

	for( unsigned int i = 0; i < disklist.size(); i ++){
		disklistP->addDisk(disklist[i].sgname, getAddress());

		/* if EnclProcessors::Enclosure::Disks struct is merged with the Disks::DiskDrive
		 * class, some code needed here to add the field in the struct to the object.*/
	}
	return SUCCESS;
	
}


/**
 * Function updateFans()
 *
 * Function that updates the information of cooling fans on the enclosure.
 *
 * @return status of updating, SUCCESS or FAILURE.
 */ 
int
EnclProcessors::Enclosure::updateFans()
{
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updateFans(): Updating enclosure cooling Fans START.");
//	int retval = updateElemType( "fan" );
	
	/**
	 * If sgname is not defined log error and return failure.
	 */ 
	if( id.empty() ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updateFans(): Error, sgname of enclosure not defined.");
		return FAILURE;
	}

	
	string cmd = "/usr/local/seamonlx/bin/enclElements.pl ";
	cmd.append(id);
	cmd.append(" fan");
	
	FILE *stream = popen(cmd.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
	if ( stream == NULL ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updateFans(): Failed getting enclosure cooling fans.");
		return FAILURE;
	}

	/*clear the old data */
	fans.clear();

	/* get the new value */
	char text[255];

	string fanstr;
	while(fgets(text, sizeof(text), stream)){
		fanstr = text;
		trimSpaces( fanstr );
		if( fanstr.empty() ){
			continue;
		}
		
		// get the seacid and descriptor
		size_t nameend = fanstr.find_first_of(":");
		if( nameend != string::npos){

			Envelemt local;

			string name = fanstr.substr(0, nameend);
			trimSpaces( name );
			
			size_t descstart = name.find_first_of("(");
			size_t descend = name.find_last_of(")");
			
			if ( (descstart != string::npos) && (descend != string::npos)){
				local.seacid = name.substr(0, descstart);
				local.desc = name.substr(descstart+1, descend-descstart-1);
				trimSpaces(local.desc);
			}
			else{
				local.seacid = name;
			}
			fanstr.erase(0, nameend+1);
			

			// get the status string
			size_t stend = fanstr.find("Actual speed");
			if( stend != string::npos ){
				local.status = fanstr.substr(0, stend);
				fanstr.erase(0, stend);
			}
			
			// get the reading
			size_t spdstart = fanstr.find("=");
			local.reading = fanstr.substr(spdstart+1);
			
			trimSpaces(local.seacid);
			trimSpaces(local.status);
			trimSpaces(local.reading);

			fans.push_back(local);

//			traceClass->LogTrace(ZQ::common::Log::L_INFO, "%s & %s & %s & %s\n", local.seacid.c_str(), local.desc.c_str(), local.status.c_str(), local.reading.c_str());
		}
	}

	pclose(stream);
	
	
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updateFans(): Updating enclosure cooling Fans DONE.");

	return SUCCESS;
//	return retval;
}


/**
 * Function updatePowers()
 *
 * Function that updates the information of power supplies on the enclosure.
 *
 * @return status of updating, SUCCESS or FAILURE.
 */ 
int
EnclProcessors::Enclosure::updatePowers()
{
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updatePowers(): Updating enclosure Power Supplies START.");
	
	/**
	 * If sgname is not defined log error and return failure.
	 */ 
	if( id.empty() ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updatePowers(): Error, sgname of enclosure not defined.");
		return FAILURE;
	}

	
	string cmd = "/usr/local/seamonlx/bin/enclElements.pl ";
	cmd.append(id);
	cmd.append(" power");
	
	FILE *stream = popen(cmd.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
	if ( stream == NULL ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updatePowers(): Failed getting enclosure power supplies.");
		return FAILURE;
	}

	/*clear the old data */
	powers.clear();

	/* get the new value */
	char text[255];

	string pstr;
	while(fgets(text, sizeof(text), stream)){
		pstr = text;
		trimSpaces( pstr );
		if( pstr.empty() ){
			continue;
		}
		
		// get the seacid and descriptor
		size_t nameend = pstr.find_first_of(":");
		if( nameend != string::npos){

			Envelemt local;

			string name = pstr.substr(0, nameend);
			trimSpaces( name );
			
			size_t descstart = name.find_first_of("(");
			size_t descend = name.find_last_of(")");
			
			if ( (descstart != string::npos) && (descend != string::npos)){
				local.seacid = name.substr(0, descstart);
				local.desc = name.substr(descstart+1, descend-descstart-1);
				trimSpaces(local.desc);
			}
			else{
				local.seacid = name;
			}
			pstr.erase(0, nameend+1);

			// get the status string
			local.status = pstr;
			
			trimSpaces(local.seacid);
			trimSpaces(local.status);
			trimSpaces(local.reading);

			powers.push_back(local);

//			traceClass->LogTrace(ZQ::common::Log::L_INFO, "%s & %s & %s & %s\n", local.seacid.c_str(), local.desc.c_str(), local.status.c_str(), local.reading.c_str());
		}
	}

	pclose(stream);
	
	
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updatePowers(): Updating enclosure power supplies DONE.");

	return SUCCESS;
	
}




/**
 * Function updateTemps()
 *
 * Function that updates the inforamtion of temperature sensors on the enclosure.
 *
 * @return status of updating, SUCCESS or FAILURE.
 */ 
int
EnclProcessors::Enclosure::updateTemps()
{
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updateTemps(): Updating enclosure temperature sensors START.");
	
	/**
	 * If sgname is not defined log error and return failure.
	 */ 
	if( id.empty() ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updateTemps(): Error, sgname of enclosure not defined.");
		return FAILURE;
	}

	
	string cmd = "/usr/local/seamonlx/bin/enclElements.pl ";
	cmd.append(id);
	cmd.append(" temperature");
	
	FILE *stream = popen(cmd.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
	if ( stream == NULL ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updateTemps(): Failed getting enclosure temperature sensors.");
		return FAILURE;
	}

	/*clear the old data */
	temps.clear();

	/* get the new value */
	char text[255];

	string tempstr;
	while(fgets(text, sizeof(text), stream)){
		tempstr = text;
		trimSpaces( tempstr );
		if( tempstr.empty() ){
			continue;
		}
		
		// get the seacid and descriptor
		size_t nameend = tempstr.find_first_of(":");
		if( nameend != string::npos){

			Envelemt local;

			string name = tempstr.substr(0, nameend);
			trimSpaces( name );
			
			size_t descstart = name.find_first_of("(");
			size_t descend = name.find_last_of(")");
			
			if ( (descstart != string::npos) && (descend != string::npos)){
				local.seacid = name.substr(0, descstart);
				local.desc = name.substr(descstart+1, descend-descstart-1);
				trimSpaces(local.desc);
			}
			else{
				local.seacid = name;
			}
			tempstr.erase(0, nameend+1);

			// get the status string
			size_t stend = tempstr.find("Temperature=");
			if( stend != string::npos ){
				local.status = tempstr.substr(0, stend);
				tempstr.erase(0, stend);
			}
			
			// get the reading
			size_t spdstart = tempstr.find("=");
			local.reading = tempstr.substr(spdstart+1);
			
			trimSpaces(local.seacid);
			trimSpaces(local.status);
			trimSpaces(local.reading);

			temps.push_back(local);

//			traceClass->LogTrace(ZQ::common::Log::L_INFO, "%s & %s & %s & %s\n", local.seacid.c_str(), local.desc.c_str(), local.status.c_str(), local.reading.c_str());
		}
	}

	pclose(stream);
	
	
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updateTemps(): Updating enclosure temperature sensors DONE.");

	return SUCCESS;
	
}



/**
 * Function updateElemType()
 *
 * Function that updates the inforamtion of one type of elements on the enclosure.
 *
 * @param string type
 *        fan  -- the cooling fan
 *        power -- the power supply
 *        temperature -- the temperature sensor
 * 
 * @return status of updating, SUCCESS or FAILURE.
 */ 
int
EnclProcessors::Enclosure::updateElemType( string type )
{
	
	/**
	 * If sgname is not defined log error and return failure.
	 */ 
	if( id.empty() ){
		return FAILURE;
	}

	
	string cmd = "/usr/local/seamonlx/bin/enclElements.pl ";
	cmd.append(id);
	cmd.append(" ");
	cmd.append(type);
	
	FILE *stream = popen(cmd.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
	if ( stream == NULL ){
		return FAILURE;
	}

			
	if( !type.compare("fan") ){
		fans.clear();
		
	}
	else if( !type.compare("temperature") ){
		temps.clear();
		
		
	}
	else if( !type.compare("power") ){
		powers.clear();
	}

	/* get the new value */
	char text[255];

	string elemstr;
	while(fgets(text, sizeof(text), stream)){
		elemstr = text;
		trimSpaces( elemstr );
		if( elemstr.empty() ){
			continue;
		}
		
		// get the seacid and descriptor
		size_t nameend = elemstr.find_first_of(":");
		if( nameend != string::npos){

			Envelemt local;

			string name = elemstr.substr(0, nameend);
			trimSpaces( name );
			
			size_t descstart = name.find_first_of("(");
			size_t descend = name.find_last_of(")");
			
			if ( (descstart != string::npos) && (descend != string::npos)){
				local.seacid = name.substr(0, descstart);
				local.desc = name.substr(descstart+1, descend-descstart-1);
				trimSpaces(local.desc);
			}
			else{
				local.seacid = name;
			}
			elemstr.erase(0, nameend+1);

			// get the status string
			string delim;
			if( !type.compare("fan") ){
				delim = "Actual speed";
			}
			else if( !type.compare("temperature") ){
				delim = "Temperature=";
			}
			
			if( delim.empty() ){
				local.status = elemstr;
			}
			else{
				size_t stend = elemstr.find(delim);
				if( stend != string::npos ){
					local.status = elemstr.substr(0, stend);
					elemstr.erase(0, stend);
				}
			}
			
			// get the reading
			size_t spdstart = elemstr.find("=");
			local.reading = elemstr.substr(spdstart+1);


			
			trimSpaces(local.seacid);
			trimSpaces(local.desc);
			trimSpaces(local.status);
			trimSpaces(local.reading);
			
			if( !type.compare("fan") ){
				fans.push_back(local);
			}
			else if( !type.compare("temperature") ){
				temps.push_back(local);
				
			}
			else if( !type.compare("power") ){
				powers.push_back(local);
			}
			
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "%s & %s & %s & %s\n", local.seacid.c_str(), local.desc.c_str(), local.status.c_str(), local.reading.c_str());
		}
	}
	
	pclose(stream);
	return SUCCESS;
	
}



/**
 * Function updatePhys()
 *
 * Function that updates the information of phys of the enclosure.
 *
 * @return status of updating, SUCCESS or FAILURE.
 */ 
int
EnclProcessors::Enclosure::updatePhys()
{
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updatePhys(): Updating enclosure phys START.");
	
	/**
	 * If sgname is not defined log error and return failure.
	 */ 
	if( id.empty() ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updatePowers(): Error, sgname of enclosure not defined.");
		return FAILURE;
	}

	
	string cmd = "/usr/local/seamonlx/bin/enclPhys.pl ";
	cmd.append(id);
	
	FILE *stream = popen(cmd.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
	if ( stream == NULL ){
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updatePhy(): Failed getting enclosure phys.");
		return FAILURE;
	}

	/*clear the old data */
	phys.clear();

	/* get the new value */
	char text[255];

	string pstr;
	while(fgets(text, sizeof(text), stream)){
		pstr = text;
		trimSpaces( pstr );
		if( pstr.find("Phy Desc") != string::npos){
			continue;
		}
		
		// get the seacid and descriptor
		vector<string> result;
		stringSplit(pstr, "|", result);
		
		Phy local;
//		traceClass->LogTrace(ZQ::common::Log::L_INFO, "the array size %d",  (int)result.size());

		
 		local.desc = result[0];
 		local.phynumber = result[1];
 		local.sas = result[2];
 		local.linkrate = result[3];
		local.invDword = atoi(result[4].c_str());
		local.disparty = atoi(result[5].c_str());
		local.syncLoss = atoi(result[6].c_str());
		local.rsetProb = atoi(result[7].c_str());
		
		phys.push_back(local);

//		traceClass->LogTrace(ZQ::common::Log::L_INFO, "%s | %s | %s | %s | %d | %d | %d | %d\n", local.desc.c_str(), local.phynumber.c_str(), local.sas.c_str(), local.linkrate.c_str(), local.invDword, local.disparty, local.syncLoss, local.rsetProb);
	}


	pclose(stream);
	
	
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "EnclProcessors.cpp Enclosure::updatePhys(): Updating enclosure phys DONE.");

	return SUCCESS;
	
}

