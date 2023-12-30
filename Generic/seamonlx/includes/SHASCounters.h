/** @file SHASCounters.h
 *
 *  SHASCounters class declaration.
 *  Defines the SHASCounters Object class.
 *  
 *  SeaChange International
 *  
 *  Revision History
 *  
 *  06-03-2010 mjc    Created ()
 *  
 * 
 */

#ifndef SHASCounters_H
#define SHASCounters_H

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

	/*
	 *
	 * 
	 * 
	 */
	class SHASCounters
	{

	#define				LIST_COMMAND_STRING			"List"
	
	  public:

		/**
		 * Type definitions
		 */
		typedef struct
		{
			string							type;  
			bool								supported;
			string							data;
		} tSupportedCounters;
		
		typedef struct
		{
			string              tag;  
			string              value;
		} TagVal;	 

	
		/**
		 * A constructor
		 * 
		 */ 
		SHASCounters();
		
		/**
		 * destructor
		 */ 
		virtual ~SHASCounters();
		
		/**
		 * Member functions.
		 * 
		 */
		virtual int		update( string shasType );
		int					updateShasCounters( string ShasType );
		
	
	  protected:

		/**
		 * data members
		 */
		 
		 // Supported Counter vector
		 vector <tSupportedCounters>		supportedCounters;
	 

	private:
	

		// Dump the counters vector
		void printSupportedCounters( void ) {
			char buffer[42];
			vector <tSupportedCounters>	 :: iterator   it;
			unsigned int j;
			printf( "\n---- SUPPORTED COUNTERS VECTOR DUMP ----\n" );
			for ( it = supportedCounters.begin(); it != supportedCounters.end(); it++ ) {	
					printf( "\tType:  %20s\tBuffer: Len=%d   Data=|", it->type.c_str(), (int)it->data.size() );
					if( it->data.size() > 0 )	{
						strncpy( buffer, it->data.c_str(), 40 );
					}
					for( j=0; j < it->data.size() && j < 40; j++ ) {
						printf( "%c", buffer[j] );
					}
					printf( "|\n");
					
			}
			printf( "---- END OF SUPPORTED COUNTERS VECTOR DUMP ----\n" );
		}

		
	};

}
#endif /* SHASCounters_H */
