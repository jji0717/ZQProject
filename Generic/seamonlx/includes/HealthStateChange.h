/** @file HealthStateChange.h
 *
 *  HealthStateChange class declaration.
 *  Defines the base class HealthStateChange
 *  
 *  SeaChange International
 *  
 *  Revision History
 *  
 *  07-08-2010 mjc   created
 *
 */

#ifndef HEALTH_STATE_CHANGE_H
#define  HEALTH_STATE_CHANGE_H

#include <stdlib.h>
#include <pthread.h>
#include <map>
#include <vector>
#include <queue>

#include "common.h"

/**
* Some defines
*/
#define			ENSURE_HSC_INSTANTIATED	if( !hsc_instance ) { \
																	TRACE_LOG("\tError: HealthStateChange class not instantiated" ); \
																	return false; \
																	}
																	
#define			HSC_START_HANDLE_VALUE		0L																	



using namespace std;


/**
 * namespace seamonlx
 *
 */
namespace seamonlx
{
	
	/**
	 * A base class Disks. Contains a list of 
	 * disks
	 */
	class HealthStateChange
	{
	  public:

			typedef struct
			{
				ObjectHealthState 			*pObjectHealthState;  
				string              				handle;
				
			} hscVectorEntry;		

			
			typedef struct
			{
				ObjectHealthState 			*pObjectHealthState;  
				ObjectHealthState::ObjHealthStateEnumerator oldState;
				ObjectHealthState::ObjHealthStateEnumerator newState;
				time_t              				eventTimestamp;
				long								reportedCounter;
				
			} hscQueueEntry;					
		
		
	  protected:
		/**
		 * data members
		 */ 
		static HealthStateChange					*hsc_instance;  		/* The instance of the object */
		static long										hscHandleCount;  		
		
		
		vector<hscVectorEntry>			vHscList;	 							/* The list of all Registrants */
		queue<hscQueueEntry>			hscQueue;								/* The queue of all events	*/

	  public:
		
		/**
		 * constructor
		 */
		HealthStateChange( string name );

		/**
		 * destructor
		 */ 
		virtual ~HealthStateChange() { }
		
		/**
		 * Function to get the pointer to the instance of Disks class
		 */ 
		static HealthStateChange * getInstance() {
			return hsc_instance;
		}
		
		//
		// For using from within the ObjectHealthState class only
		// Register methods - to add an object to our complete list of objects, regardless of
		// whether they change stat or not. 
		// External to the ObjectHealthState class, it will allow for a system-wide 
		// query of all objects.
		// 
		string			Register( ObjectHealthState *ohs );
		bool				unRegister( string handle );
		//bool			addCallbackForStateChanges( );
		
		//
		// Object methods - UNKNOWN AND UNUSED AT THIS TIME
		//
		bool				addObject( string objectName );
		bool				deleteObject( string objectName );

		//
		// To add a state change to the queue of state changes
		//
		bool				addObjectStateChange( ObjectHealthState *ohs,
																ObjectHealthState::ObjHealthStateEnumerator oldState, 
																ObjectHealthState::ObjHealthStateEnumerator newState );


	};
}



#endif /* HEALTH_STATE_CHANGE_H */
