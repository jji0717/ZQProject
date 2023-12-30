/** @file HealthStateChange.cpp
 *
 *  HealthStateChange member methods implementation.
 *
 *  Revision History
 *
 *  07-08-2010 mjc	Created.
 *    
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>

#include "common.h"
#include "HealthStateChange.h"

using namespace std;
using namespace seamonlx;


//
// Singleton instance
//
HealthStateChange 	*HealthStateChange::hsc_instance = 0;
long							HealthStateChange::hscHandleCount = HSC_START_HANDLE_VALUE;	


//
 // Uncomment this define for detailed TRACE_LOG tracing of this method
 //
#define SCR_DEBUG 1			// Enable detailed DEBUG 
//
#ifdef SCR_DEBUG
#define SCR_TRACE( s1, args... )  traceClass->LogTrace(ZQ::common::Log::L_INFO,  s1, ##args )
#else
#define SCR_TRACE( s1, args... )  ;
#endif
//

/**
* HealthStateChange
* Constructor
*/
HealthStateChange::HealthStateChange( string name )
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "HealthStateChange.cpp : Constructor Enter. Name=|%s|", name.c_str() );
	
	// Ensure we have only one copy!
	if( hsc_instance ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "\tAttempting to create second HealthStateChange class instance" );
		throw "Attempting to create second HealthStateChange class instance";
		}	else {
			// Initialize all the variables here if this is the first and only instance
			hsc_instance = this;
		}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "HealthStateChange.cpp : Constructor Exit." );
}

/**
* HealthStateChange
* Register
*/
string
HealthStateChange::Register( ObjectHealthState *ohs )
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "HealthStateChange.cpp : Register Enter." );
	
	hscVectorEntry		entry;
	string   					handle = "Error";							// Unique ID for the handle for this entry
	char						buffer[ BUFF64 ];
	
	if( !hsc_instance ) { 
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "\tError: HealthStateChange class not instantiated" ); 
		return handle;
	}
	
	// Mutex lock for the ID handle must be taken here
	pthread_mutex_lock( &hscMutex );		
	
	// Create the handle value
	sprintf( buffer, "%08ld", HealthStateChange::hscHandleCount++ );
	
	// Add the handle to the list with the ObjectHealthState pointer
	entry.handle = buffer;
	entry.pObjectHealthState = ohs;
	
	// Push it onto our list of registered entries
	vHscList.push_back( entry );
	
	#ifdef SCR_DEBUG
	string theState = entry.pObjectHealthState->getHealthStateString( entry.pObjectHealthState->getHealthState() );
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tPushed entry.handle=|%s|, state=%s. Vector Size=%d", entry.handle.c_str(),theState.c_str(), (int)vHscList.size());
	#endif																					
	
	// Unlock now that we are done
	pthread_mutex_unlock( &hscMutex );		
	
	// Assign it
	handle = buffer;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "HealthStateChange.cpp : Register Exit.  Returning Handle |%s|  vHscList size is %d", handle.c_str(), (int)vHscList.size() );

	return handle;		// Return it
}

/**
* HealthStateChange
* unRegister
*/
bool
HealthStateChange::unRegister( string handle )
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "HealthStateChange.cpp : unRegister Enter." );

	bool												found = false;
	vector<hscVectorEntry>::iterator	it;		
	
	ENSURE_HSC_INSTANTIATED;		/* Make sure class exists! */
	
	// Mutex lock for the ID handle must be taken here
	pthread_mutex_lock( &hscMutex );		
	
	// Search the list for the handle
	for ( it = vHscList.begin(); (it != vHscList.end()) && !found; it++ ) {	
	
		// If we find it, remove it, set our retVal flag to true
		if( it->handle.compare( handle ) == 0 ) {
			found = true;
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tEntry found. Nuking handle=|%s|", it->handle.c_str() );
			vHscList.erase( it );		// Nuke the entry
			
		}
	}
	
	// Unlock now that we are done
	pthread_mutex_unlock( &hscMutex );
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "HealthStateChange.cpp : unRegister Exit.  Returning %s",  (found==true?"TRUE":"FALSE") );

	return found;
}

/**
* HealthStateChange
* addObjectStateChange
*/
bool
HealthStateChange::addObjectStateChange( ObjectHealthState *ohs,
																ObjectHealthState::ObjHealthStateEnumerator oldState, 
																ObjectHealthState::ObjHealthStateEnumerator newState )
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "HealthStateChange.cpp : addObjectStateChange Enter." );
	
	hscQueueEntry		entry;
	string   					handle;							// Unique ID for the handle for this entry
	
	ENSURE_HSC_INSTANTIATED;
	
	// Mutex lock for the ID handle must be taken here
	pthread_mutex_lock( &hscMutex );		
	
	// Create the handle value
	
	// Add the handle to the list with the ObjectHealthState pointer
	entry.pObjectHealthState = ohs;
	entry.oldState = oldState;
	entry.newState = newState;
	
	// Add timestamp
	time ( &entry.eventTimestamp );

	
	// Push it onto our list of registered entries
	hscQueue.push( entry );
	
	#ifdef SCR_DEBUG
	traceClass->LogTrace(ZQ::common::Log::L_INFO, "seamonlx, \tPushed onto queue. Queue Size=%d",  (int)hscQueue.size());
	#endif																					
	
	// Unlock now that we are done
	pthread_mutex_unlock( &hscMutex );		
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "HealthStateChange.cpp : addObjectStateChange Exit.  Returning TRUE");

	return true;		// Return it
}

/* End HealthStateChange.cpp */

