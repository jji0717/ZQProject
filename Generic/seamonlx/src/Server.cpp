/** @file ServerHwXmlrpc.cpp
 *
 *
 *  ServerObj class constructors and member functions
 *  implementation.
 *  
 *
 *
 *  Revision History
 *  
 *  03-10-2010 Created ( jie.zhang@schange.com)
 *  
 * 
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include "common.h"
#include "Server.h"

using namespace std;
using namespace seamonlx;


/*************************************************
 *
 *       Class ServerHw methods definition
 * 
 ************************************************/


/**
 * ServerHw constructor
 */
ServerHw::ServerHw():vseperator(",,")
{
}


/**
 * ServerHw Destructor
 */ 
ServerHw::~ServerHw()
{
}




/**
 * Function update()
 *
 * The function that updates the ServerHw data members
 */
int
ServerHw::update()
{
	int retval = SUCCESS;
	
	if ( updateChassis() != SUCCESS ){
		retval = FAILURE;
	}
	
	if ( updateBaseboard() != SUCCESS ){
		retval = FAILURE;
	}
	
	if ( updateBios() != SUCCESS ){
		retval = FAILURE;
	}
	
	if ( updateMemory() != SUCCESS ){
		retval = FAILURE;
	}
	
	if ( updateProcessor() != SUCCESS ){
		retval = FAILURE;
	}
	return retval;
}


/**
 * Function updateChassis()
 *
 * The function that updates the chassis datamember.
 * 
 */
int
ServerHw::updateChassis()
{
//	chassislock.write_lock();
	
	chassis.clear();
	
	ElemList chassis_array;
	if (updateArrayElem(3, chassis_array) != SUCCESS){
		return FAILURE;
	}
	
	if (chassis_array.size() == 1){
		chassis = chassis_array.back();
	}
	chassis.insert(make_pair("SEAC_ID", "Chassis"));

	/*unlock */
//	chassislock.write_unlock();

	return SUCCESS;
}


/**
 * Function updateBaseboard()
 *
 * The function that updates the chassis datamember.
 * 
 */
int
ServerHw::updateBaseboard()
{
//	baseboardlock.write_lock();
	
	baseboard.clear();
	
	ElemList bb_array;
	if( updateArrayElem(2, bb_array) != SUCCESS){
		return FAILURE;
	}
	
	if (bb_array.size() == 1){
		baseboard = bb_array.back();
	}
	baseboard.insert(make_pair("SEAC_ID", "Baseboard"));

//	baseboardlock.write_unlock();

	return SUCCESS;
}



/**
 * Function updateBios()
 *
 * The function that updates the Bios datamember.
 * 
 */

int
ServerHw::updateBios()
{
//	bioslock.write_lock();
	
	bios.clear();
	ElemList bios_array;
	if (updateArrayElem(0, bios_array) != SUCCESS){
		return FAILURE;
	}
	
	if (bios_array.size() == 1){
		bios = bios_array.back();
	}
	bios.insert(make_pair("SEAC_ID", "Bios"));

//	bioslock.write_unlock();

	return SUCCESS;
}



/**
 * Function updateProcessor()
 *
 * The function that updates the processor datamember.
 * 
 */

int
ServerHw::updateProcessor()
{
//	processorlock.write_lock();
	
	processor.clear();

	if (updateArrayElem(4, processor, "{Handle 0x|Processor Information|Socket Designation:|Type:|Manufacturer:|ID:|Version:|External Clock:|Cache Handle:|External Clock:|}") != SUCCESS){
		return FAILURE;
	}

	/* add SEAC_ID*/
	vector<map<string, string> >::iterator j;
	for(j = processor.begin(); j != processor.end(); j++ ){
		
		map<string, string>::iterator pid = j->find("Socket Designation");
		j->insert(make_pair("SEAC_ID", pid->second));
	}

//	processorlock.write_unlock();
	
	return SUCCESS;
}



/**
 * Function updateMemory()
 *
 * The function that updates the memory datamember.
 * 
 */

int
ServerHw::updateMemory()
{

//	memorylock.write_lock();
	
	/**
	 * Clear the old values
	 */
	memory.clear();
	
	string marray_cmd = "/usr/sbin/dmidecode -t 16";

	/**
	 * Get the Physical memory array.
	 */
	FILE *stream = popen(marray_cmd.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", marray_cmd.c_str());
	if( stream == NULL ){
		return FAILURE;
	}

	int index = 0;  /* Index of physical memory arrays*/
	char text[255];
	string buf;
	bool singleval = true;
	string keyname; /* key name of the  multiple value field*/
	string values;  /* values string of multiple value field */
	map<string, string> pelem; /* Map that hold one memory info */
	Memory melem;
	
	while ( fgets(text, sizeof(text), stream) ){
		buf = text;
		string line(buf);
		trimSpaces(buf);
		size_t seppos = buf.find(":", 0);
		

		if ( seppos != string::npos ){

			/**
			 * If the line contains ":", and it's not at
			 * the end of the line. get the key value pair.
			 */
			if ( seppos != buf.length()-1){

				/**
				 * update the key value pair if any,
				 * reset the keyname and value.
				 */
				if( !keyname.empty() ){
					pelem.insert(make_pair(keyname, values));
				}
				singleval = true;
				keyname.clear();
				values.clear();
				
				/**
				 * insert the pair to result map.
				 */ 
				pair<string, string> member = getPair(buf);
				pelem.insert(member);

			}
			
			/**
			 * if the ":" is at the end of the line, get the key
			 * only. This field has either multiple or no values.
			 */
			else{
				if(! keyname.empty()){
					pelem.insert(make_pair(keyname, values));
				}
				keyname.clear();
				values.clear();
				
				keyname = buf.substr(0, seppos);
				singleval = false;
			}
		}
		
		/**
		 * If the line doesn't contain ":". It will
		 * be one of the following case:
		 *
		 * 1. empty line. The end of the result element.
		 * 2. the line contains description at the beginning of
		 *    each element.
		 * 3. the line contains the value of a multiple value
		 *    field.
		 * 
		 */
		else{

			if (!buf.empty()){

				/**
				 * If the line starts with "Handle", it's a new
				 * element.
				 */
				string pattern("Handle");
				if(! line.compare(0, pattern.size(), "Handle")){
					if( ! keyname.empty() ){
						pelem.insert(make_pair(keyname, values));						
					}
					
					if( ! pelem.empty() ){

						/* add element to data member memory*/
						melem.marray = pelem;
						memory.push_back(melem);
					}
					
					singleval = true;
					
					/* reset the keyname and value.*/
					keyname.clear();
					values.clear();
					pelem.clear();
					melem.handle.clear();
					melem.marray.clear();

					index ++;
					/* add SEAC_ID to map pelem */
					string seacid = "Memory ";
					stringstream idbuf;
					idbuf << seacid << index;
					seacid = idbuf.str();
					pelem.insert(make_pair("SEAC_ID", seacid));

					/*save the handle*/
					size_t pos = buf.find_first_of("0x");
					if(pos != string::npos){
						melem.handle = buf.substr(pos, HANDLE_LEN);
					}
				}
				
				/**
				 * Not started with "Handle". append the value to values string
				 * if the it's a value of a multi-valu field.
				 */ 
				else{
					
					if( !singleval ){
						values.append(buf);
						values.append(vseperator);
					}
					
				}
				
			}
			continue;
		}
	}

	/**
	 * save the last entry
	 */
	if( ! keyname.empty() ){
		pelem.insert(make_pair(keyname, values));						
	}

	/**
	 * save the physical memory array
	 */
	if( ! pelem.empty() ){
		/**
		 * save system memory only.
		 */
		map<string, string>::iterator elemit = pelem.find("Use");
		if ( elemit != pelem.end() ){
			if ( ! (elemit->second).compare("System Memory") ){
				melem.marray = pelem;
				memory.push_back(melem);
			}
			
		}
	}
	
	

	/**
	 * Update the memory device.
	 *
	 */
	ElemList memdevs;
	if (updateArrayElem(17, memdevs) != SUCCESS){
		return FAILURE;
	}


	/**
	 * Associate the memory devices with physical memory array
	 * using the field Array Handle.
	 */
	vector<map<string, string> >::iterator i;
	for( i = memdevs.begin(); i != memdevs.end(); i++ ){
		
		map<string, string>::iterator mapit = i->find("Array Handle");
		string handle;

		if ( mapit != i->end() ){
			handle = mapit->second;
		
			vector<Memory>::iterator j;
			for(j = memory.begin(); j != memory.end(); j++ ){
				if ( ! (handle.compare(j->handle) ) ){
					/*update the SEAC_ID to contain size*/
					map<string, string>::iterator pid = i->find("Locator");
					map<string, string>::iterator psize = i->find("Size");
					string md_seacid = pid->second;
					md_seacid.append("(");
					md_seacid.append(psize->second);
					md_seacid.append(")");
					i->insert(make_pair("SEAC_ID", md_seacid));
					j->dimms.push_back(*i);
				}
			}
		}
	}

	pclose(stream);
//	memorylock.write_unlock();
	
	return SUCCESS;
}





/**
 * Function updateArrayElem()
 *
 * The function that updates the element list of type "type".
 * This function can be used to update the vector elements
 * that contains a list of map<string, string>. Used
 * for updating member processor and memory.dimms
 * 
 * @param[IN] type the type of the element.
 * @param[OUT] result map saves the result
 */
int
ServerHw::updateArrayElem(int type,  ElemList &result, const char* filter)
{
	string cmd = "/usr/sbin/dmidecode -t ";
	stringstream Streambuf;
	Streambuf << cmd<< type;
	if (NULL != filter && strlen(filter) >0)
		Streambuf << "| grep -E \"" << filter << "\"";
 
	cmd = Streambuf.str();
	string seacid;
	
	/**
	 * Parse the result of command dmidecode to
	 * get result information.
	 */
	FILE *stream = popen(cmd.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
	if( stream == NULL ){
		return FAILURE;
	}

	char text[255];
	string buf;
	bool singleval = true;
	string keyname; /* key name of the  multiple value field*/
	string values;  /* values string of multiple value field */
	map<string, string> elem; /* Map that hold one element info */

	while ( fgets(text, sizeof(text), stream) ){
		buf = text;
		string line(buf);
		trimSpaces(buf);
		size_t seppos = buf.find(":", 0);


		if ( seppos != string::npos ){

			/**
			 * If the line contains ":", and it's not at
			 * the end of the line. get the key value pair.
			 */
			if ( seppos != buf.length()-1){
				if( !keyname.empty() ){
					elem.insert(make_pair(keyname, values));
				}
				singleval = true;
				keyname.clear();
				values.clear();
				
				/**
				 * insert the pair to result map.
				 */ 
				pair<string, string> member = getPair(buf);
				elem.insert(member);

			}
			
			/**
			 * if the ":" is at the end of the line, get the
			 * key only. This field has multiple or no values.
			 */
			else{
				if(! keyname.empty()){
					elem.insert(make_pair(keyname, values));
				}
				keyname.clear();
				values.clear();
				
				keyname = buf.substr(0, seppos);
				singleval = false;
			}

		}
		
		/**
		 * If the line doesn't contain ":". It will
		 * be one of the following case:
		 *
		 * 1. empty line. The end of the result element.
		 * 2. the line contains description.
		 * 3. the line contains the value of a multiple value
		 *    field.
		 *
		 */ 
		else{

			if (!buf.empty()){

				/**
				 * If the line starts with "Handle", it's a new
				 * element.
				 */
				string pattern("Handle");
				if(! line.compare(0, pattern.size(), "Handle")){
					if( ! keyname.empty() ){
						elem.insert(make_pair(keyname, values));						
					}
					if( ! elem.empty() ){
						result.push_back(elem);
					}
					
					singleval = true;
					
					/* reset the keyname and value.*/
					keyname.clear();
					values.clear();
					elem.clear();
				}
				
				/**
				 * not a new element, append the value to values string
				 * if the it's a value of a multifield value.
				 */ 
				else{
					
					if( !singleval ){
						values.append(buf);
						values.append(vseperator);
					}
					
				}
				
			}
			continue;
		}
	}
	
	/**
	 * save the last entry
	 */
	if( ! keyname.empty() ){
		elem.insert(make_pair(keyname, values));						
	}
	if( ! elem.empty() ){
		result.push_back(elem);
	}

	pclose(stream);
	return SUCCESS;
}



/*************************************************
 *
 *       Class ServerEnv methods definition
 * 
 ************************************************/

/**
 * Function update()
 *
 * The function that updates the ServerEnv data members.
 * 
 */
int
ServerEnv::update()
{
	ZQ::common::MutexGuard gd(*this);
	int retval = SUCCESS;
	
	// Set state to OK
	setHealthState( ObjectHealthState::STATE_OK );
	pthread_mutex_lock( &ServerEnvMutex );	
	if ( updateFans() == FAILURE ){
		retval = FAILURE;
	}

	if ( updatePowers() == FAILURE ){
		retval = FAILURE;
	}

	if ( updateTempsensors() == FAILURE ){
		retval = FAILURE;
	}

    if ( updateVoltages() == FAILURE ){
		retval = FAILURE;
	}

	pthread_mutex_unlock( &ServerEnvMutex );
	return retval;
}



/**
 * Function updateFans
 *
 * The function that updates the ServerEnv data member
 * fans.
 * 
 */
int
ServerEnv::updateFans()
{		ZQ::common::MutexGuard gd(*this);
	return updateElems("fan", fans);
}



/**
 * Function updatePowers
 *
 * The function that updates the ServerEnv data member
 * powers
 * 
 */
int
ServerEnv::updatePowers()
{		ZQ::common::MutexGuard gd(*this);
	return updateElems("\"power supply\"", powers);
}



/**
 * Function updateTempsensors
 *
 * The function that updates the ServerEnv data member
 * tempsensors
 * 
 */
int
ServerEnv::updateTempsensors()
{		ZQ::common::MutexGuard gd(*this);
	return updateElems("temperature", tempsensors);
}



/**
 * Function updateVoltages
 *
 * The function that updates the ServerEnv data member
 * voltages
 * 
 */
int
ServerEnv::updateVoltages()
{	
	ZQ::common::MutexGuard gd(*this);
	return updateElems("voltage", voltages);
}



/**
 *Function updateType()
 *
 * The function that updates the data member of given
 * type.
 *
 * @param[IN] type they type of the objects to collect.
 * @param[OUT] elemList the list of element to return.
 *
 */ 
int
ServerEnv::updateElems(string type, EList &elemList)
{
	ZQ::common::MutexGuard gd(*this);
	/**
	 * clear the old data in elemList.
	 */
	if( ! elemList.empty() ){
		elemList.clear();
	}
	
	/**
	 * Run the ipmitool command and catch output.
	 */
	string cmd = "/usr/bin/ipmitool sdr type ";
	cmd.append( type );
	
	FILE *stream = popen(cmd.c_str(), "r");
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd.c_str());
	if ( stream == NULL ){
		return FAILURE;
	}

	/**
	 * Creates buffers to save the element information.
	 */
	char text[255];
	string buf;

	/**
	 * Parse each line of the output and save the infomaiton.
	 */
	while (fgets(text, sizeof(text), stream)) {
		Envelem element;
		vector<string> elemInfo;
		buf = text;

		//debug
		// cout << "the line is : " << buf << endl;
		
		stringSplit(buf, "|", elemInfo);
		element.seacid = elemInfo[0];
		element.value = elemInfo[4];
	
		// For more info on these, perform a "man ipmitool"
		if( ( elemInfo[2].compare("lnr") == 0 ) ||		// Lower Non-Recoverable
			( elemInfo[2].compare("unr") == 0 ) ){		// Upper Non-Recoverable
			element.status = "Failed";
			setHealthState( ObjectHealthState::STATE_DEGRADED );
		}
			
		else if ( ( elemInfo[2].compare("lnc") == 0 ) ||	// Lower Non-Critical (should be a DEGRADED state)
				  ( elemInfo[2].compare("ucr") == 0 ) ||		// Upper Critical
				  ( elemInfo[2].compare("unc") == 0 ) ||	// Upper Non-Critical (DEGRADED)
				  ( elemInfo[2].compare("lcr") == 0 ) ){		// Lower Critical
			element.status = "Critical";
			setHealthState( ObjectHealthState::STATE_CRITICAL );
		}
		else if ( ( elemInfo[2].compare("ok") == 0 ) ||		// A-OK status
					 ( elemInfo[2].compare("ns") == 0 ) )			// No state (like no reading attained)
		{
			element.status = "OK";
			// We do not override the previously set state in case it's not the OK state
		}
		else{
			// Should never get here, but we have to set some state....
			element.status = "Unknown";
			setHealthState( ObjectHealthState::STATE_UNKNOWN );
		}

		elemList.push_back(element);
		elemInfo.clear();
		
		// We update our reason description if what we just did
		if( element.status.size() > 0 && element.status.compare( "OK" ) != 0 )
		{
				healthObj.appendHealthReasonDescription( type );
	    }
		
	}  // End while

	pclose(stream);
	return SUCCESS;
}

