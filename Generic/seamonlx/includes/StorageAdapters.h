/** @file StorageAdapters.h
 *
 *  StorageAdapters class declaration.
 *  Defines the base class StorageAdapters.
 *  
 *  SeaChange International
 *  
 *  Revision History
 *  
 *  03-29-2010 Created ( jie.zhang@schange.com)
 *
 *  05-14-2010 jiez
 *    - Changed StorageAdapter class singleton
 *    - Changed outer class data member encls to contain pointers instead of objects.
 *    - Added function to create the children elements during updating the adapter object
 *      and set the parent pointer of the children.
 *    - Added function of clear the adapter list.
 */

#ifndef STORAGEADAPTERS_H
#define STORAGEADAPTERS_H

#include <stdlib.h>
#include <pthread.h>
#include <map>
#include <vector>

using namespace std;

/**
 * namespace seamonlx
 *
 */
namespace seamonlx
{
	
	/**
	 * A base class StorageAdapters. Contains a list of 
	 * storage adapters
	 */
	class StorageAdapters
	{
	  public:

		/**
		 * nested class Adapter 
		 */
		class Adapter{
		  public:

			Adapter(){}

			/**
			 * overload assignment operator
			 */
			Adapter& operator=(const Adapter& other);

			/**
			 * copy constructor
			 */ 
			Adapter( const Adapter &orig );

			/**
			 * A class Phy within the class Adapter
			 */ 
			class Phy
			{
			  public:
				Phy(){}
				Phy( const Phy& other );
				Phy& operator=(const Phy& other);

				void clear();
								
				string phynum;     /* the id of phy */
				string sas_addr;
				string port;
				string linkstatus;
				string linkrate;
				string minrate;
				string maxrate;
				
				/* error counters*/
				unsigned int invalid_dword_count;
				unsigned int loss_of_dword_sync_count;
				unsigned int phy_reset_problem_count;
				unsigned int running_disparity_error_count;
			};
			
			typedef struct
			{
				string              id;  /* the unique id of child element*/
				string              type;/* the type of child element, enclosure or disk*/
			} Child;
			
			/**
			 * Accessors
			 */
			string getPciaddr() const { return pciaddr; }
			string getDrname() const { return drname; }
			string getDrver() const { return drver; }
			string getFwver() const { return fwver; }
			string getStatus() const { return status; }
			string getVendor() const { return vendor; }
			string getVendorid() const { return vendorid; }
			string getDeviceid() const{ return deviceid; }
			vector<Phy> getPhys() const{ return phys; }
			vector<Child> getChildelem() const { return childelem; }
			

			/**
			 * mutators
			 */
			void setPciaddr(const string id){pciaddr = id;}
			void setDrname(const string dname){drname = dname;}
			void setDrver(const string dv){ drver = dv;}
			void setFwver(const string fver){ fwver = fver;}
			void setStatus(const string st){ status = st;}
			void setVendor(const string vd){ vendor = vd; }
			void setVendorid(const string vid){ vendorid = vid; }
			void setDeviceid(const string did){ deviceid = did; }
			void setPhys(const vector<Phy> in);
			void setChildelem(const vector<Child> in);
			
			/**
			 * function to update the adapter
			 */
			int update();
			int updateDrinfo();
			int updateStatus();
			int updateVendor();
			int updateFwver();
			int updatePhys();
			int updateChildelem();
			
			/**
			 * Function to create the children element instance of type
			 * Enclosure or DiskDrive.
			 */ 
			int createChildelems();


			/**
			 * function to get the address of the instance
			 */
			const Adapter * getAddress() const{ return this;}
			
		  protected:

			/*
			 *data members
			 */
			string pciaddr;             /* pciaddr */
			string drname;              /* driver name */
			string drver;               /* driver version */
			string fwver;               /* firmware version */
			string status;              /* the health status */ 
			
			/* vendor id and device id

			   vendorid              vendor             
			   --------     ----------------------      
			     8086       Inel Corporation            
			     103c       Hewlett-Packard Company     
			     1000       LSILogic/Sybios Logic
			     117c       Atto Technology  

			   vendorid   deviceid            device
			   --------   --------	  ---------------------
			     8086       269e       631xESB IDE Controller
			     103c       3230       Smart Array Controller
			     1000       0058       SAS1068E PCI-Express Fusion-MPT SAS
			     117c  
			*/

			string vendorid;            /* the vendor id */
			string deviceid;            /* the device id */
			string vendor;              /* vendor name */

			vector<Phy> phys;                /* the phy links */
			
			vector<Child>  childelem;   /* child element id<->type pair.
									       * enclosure or disk drives */
			
		  private:
			/**
			 * Private helper function to get the atto card channel number.
			 */ 
			string attoGetChannel();
						
		};

		
	  protected:
		/**
		 * data members
		 */ 
		vector<Adapter *> adapters;  /*An array of adapters.*/
		static StorageAdapters *s_instance; /* The instance of the object */

	  public:
		/**
		 * constructor
		 */
		StorageAdapters();

/* 		/\** */
/* 		 * copy cnstructor */
/* 		 *\/ */
/* 		StorageAdapters( const StorageAdapters & orig ){ */
/* 			setAdapters ( orig.adapters ); */
/* 		} */
		
/* 		/\** */
/* 		 * overload assignment operator */
/* 		 *\/ */
/* 		StorageAdapters& operator=(const StorageAdapters& other){ */
/* 			if( &other != this ){ */
/* 				setAdapters( other.adapters ); */
/* 			} */
/* 			return *this; */
/* 		} */

		/**
		 * destructor
		 */ 
		virtual ~StorageAdapters(){}

		
		/**
		 * functions that update the data member adapters
		 */
		virtual int update();

		/**
		 * accessor to get the adapters
		 */
		vector<Adapter *> getAdapters() const{ return adapters; }
		

		/**
		 * mutator
		 */
		void setAdapters(const vector<Adapter *> in);

		
		/**
		 * Function to get the pointer to the instance of EnclProcessors class
		 */ 
		static StorageAdapters * instance(){
			return s_instance;
		}
		
		/* function to clear all the enclosures, can be called to free the memory before program terminates. */	
		void clear();
	};
}


#endif /* STORAGEADAPTERS_H */
