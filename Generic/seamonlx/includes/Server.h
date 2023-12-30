/** @file Server.h
 *
 *  Server class declaration.
 *  Defines the ServerHardware class and ServerEnv class.
 *  
 *  SeaChange International
 *  
 *  Revision History
 *  
 *  03-19-2010 Created ( jie.zhang@schange.com)
 *  
 * 
 */

#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <pthread.h>
#include <map>
#include <vector>
#include "common.h"
#include <Locks.h>

using namespace std;


/**
 * namespace seamonlx
 *
 */
namespace seamonlx
{

	#define HANDLE_LEN      6
	
	/**
	 * A base class ServerHw. contains the following server
	 * hardware information:
	 * 
	 *   chassis
	 *   baseboard
	 *   bios
	 *   processors
	 *   memory
	 */
	class ServerHw
	{
	  public:
		
        #define SETHEALTHSTATE(hs) healthObj->setHealthState(hs)
		
		typedef vector<map<string, string> > ElemList;
		typedef struct
		{
			string              handle;   /**
										   * a unique id, allows physical
										   * memory arrays and dimms to
										   * reference each other
										   */
			map<string, string> marray;  /* physical memory array info*/
			ElemList            dimms;    /* dimms */
		} Memory;
		
		/**
		 * A constructor
		 * 
		 */ 
		ServerHw();
		
		/**
		 * destructor
		 */ 
		virtual ~ServerHw();
		
		/**
		 * Member function.
		 * Function to update the Server hardware data members
		 */
		virtual int update();
		virtual int updateChassis();
		virtual int updateBaseboard();
		virtual int updateBios();
		virtual int updateProcessor();
		virtual int updateMemory();

		/**
		 * Accessors
		 */
		map<string, string> getChassis() const {
			return chassis;
		}
		map<string, string> getBaseboard() const {
			return baseboard;
		}
		map<string, string> getBios() const {
			return bios;
		}
		vector<Memory> getMemory() const{
			return memory;
		}
		
		ElemList getProcessor() const 
		{
			return processor;
		}
		
	  protected:
		int updateArrayElem(int type, ElemList &result, const char* filter=NULL);
		

		/**
		 * data members
		 */
		map<string, string>  chassis;
		map<string, string>  baseboard;	
		map<string, string>  bios;
		vector<Memory>       memory;
		ElemList             processor;

		/**
		 * If a field have multiple values, the values are 
		 * seperated by vseperator and saved in a string.
		 */
		string         vseperator;

		/**
		 * The read write lock to protect the data
		 */
		rw_lock   chassis_lock;
		rw_lock   baseboard_lock;
		rw_lock   bios_lock;
		rw_lock   memory_lock;
		rw_lock   processor_lock;

	};


	
	/**
	 * A base class ServerEnv. Contains the following server
	 * environmental information:
	 *
	 *   fans
	 *   power supplies
	 *   temperature sensors
	 *   
	 */ 
	class ServerEnv : public ZQ::common::Mutex
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
		} Envelem;
		typedef vector<Envelem> EList;

		/**
		 * constructor
		 */ 
		ServerEnv(){
		}

		/**
		 * virtual functions that update the
		 * data members
		 */
		virtual int update();
		virtual int updateFans();
		virtual int updatePowers();
		virtual int updateTempsensors();
		virtual int updateVoltages();
		
		/***
		 * Accessor functions
		 */
		EList getFans() const
		{
			ZQ::common::MutexGuard gd(*this);
			return fans;
		}

		EList getPowers() const
		{
			ZQ::common::MutexGuard gd(*this);
			return powers;
		}

		EList getTempsensors() const
		{
			ZQ::common::MutexGuard gd(*this);
			return tempsensors;
		}
		EList getVoltages() const
		{
			ZQ::common::MutexGuard gd(*this);
			return voltages;
		}

		ObjectHealthState::ObjHealthStateEnumerator getHealthState()
		{
			ZQ::common::MutexGuard gd(*this);
		    return( healthObj.getHealthState() );
		}		
		void setHealthState( ObjectHealthState::ObjHealthStateEnumerator state )
		{
				ZQ::common::MutexGuard gd(*this);
		    healthObj.setHealthState( state );
		}			
		string getHealthReasonDescription()
		{
			ZQ::common::MutexGuard gd(*this);
		    return( healthObj.getHealthReasonDescription() );
		}		
		void appendHealthReasonDescription( string typeString )
		{
			ZQ::common::MutexGuard gd(*this);
		    healthObj.appendHealthReasonDescription( typeString );
		}			
		
	  protected:
		int updateElems(string type, EList &elemList);
		
		virtual ~ServerEnv(){
		}
		
		/**
		 * Data members
		 */
		EList fans;
		EList powers;
		EList tempsensors;
		EList voltages;
		
		private:
		/*
		 * Object Health State
		 */
		ObjectHealthState    healthObj;		
	};
	
}
#endif /* SERVER_H */
