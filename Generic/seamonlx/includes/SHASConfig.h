/** @file SHASConfig.h
 *
 *  SHASConfig class declaration.
 *  Defines the SHASConfig Object class.
 *  
 *  SeaChange International
 *  
 *  Revision History
 *  
 *  04-28-2010 mjc    Created ()
 *  06-09-2010 mjc    Added Enclosure / Bay capability. Some clean up.
 *  
 * 
 */

#ifndef SHASConfig_H
#define SHASConfig_H

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

	/**
	 *
	 *  Class SHASConfig
	 *  Contains the elements required to hold the Configuration of the SHAS Object
	 */
	class SHASConfig
	{

	  public:

			typedef struct
			{
				string              tag;  
				string              value;
			} TagVal;	  
			
			typedef struct
			{
				string				handle;				// PD Handle maps to this handle value
				string				enclosure;  		// Enclosure ID
				string				bay;					// Bay ID
			} EnclosureBay;	 			
		
		/**
		 * Constructor
		 * 
		 */ 
		SHASConfig();
		
		/**
		 * Destructor
		 */ 
		virtual ~SHASConfig();
		
		/**
		 * Member functions.
		 */
		virtual int		update( string shasType );
		int					updateShas( string ShasType );

		/**
		* Storage for string header data
		* This is public just to make it easy on derrived classes (SHASConfigXmlrpc)
		*
		* These strings are loaded in our constructore from the StringResource Library
		*	
		*/
		string					cdHeaderString;
		string					pdHeaderString;
		string					ldHeaderString;
		string					leHeaderString;	
		string					enclNumHeaderString;	
		string					bayNumHeaderString;	
		string					pdHandleHeaderString;
		string					handleHeaderString;		
			
		
	  protected:

		/**
		 * Data Members
		 */
		 
		 // Array of vector for CD, PD, and LD
		 vector<TagVal> 			vecDev;
		 
		 // Array of vector for PD Enclosure, Bay & Handle values
		 vector<EnclosureBay>	vecEnclBay;

		// Accessors for setting and getting the Enclosure and Bay data
		void 					setPDEnclosureAndBay( void );
		bool						getPDEnclosureAndBay( string handle, string &encl, string &bay );


	private:

	// Clear out the vector dev
		void clearVecDev( void ) {
				vecDev.clear();	
		}

	};

}
#endif /* SHASConfig_H */
