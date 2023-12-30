
/** @file EnclProcessors.h
 *
 *  EnclProcessors class declaration.
 *  Defines the base class EnclProcessors
 *  
 *  SeaChange International
 *  
 *  Revision History
 *  
 *  04-23-2010 jiez
 *  - Created
 *
 *  05-06-2010 jiez
 *   - Added sgname and block device name to Disk struct.
 *
 *  05-12-2010 jiez
 *   - Changed data member of EnclProcessors encls to contain pointers instead of the Enclosure objects.
 *   - Changed constructor to allow only one instance of enclosure exists.
 *   - Changed outer class EnclProcessors singleton. Only one instance exists.
 *   
 */

#ifndef ENCLPROCESSORS_H
#define ENCLPROCESSORS_H

#include <stdlib.h>
#include <pthread.h>
#include <map>
#include <vector>
#include "StorageAdapters.h"

using namespace std;

/**
 * namespace seamonlx
 *
 */
namespace seamonlx
{
	
	/**
	 * A base class EnclProcessors. Contains a list of 
	 * enclosure processors
	 */
	class EnclProcessors
	{
	  public:

		/**
		 * nested class Enclosure
		 */
		class Enclosure{
		  public:

			/* Default constructor */
			Enclosure():pParent(NULL){}

			/**
			 * Construct an enclosure with given id.
			 */
			Enclosure( const string id );

			
			typedef struct
			{
				string seacid;
				string desc;
				string reading;
				string status;
				
			} Envelemt;

			typedef struct
			{
				string desc;
				string phynumber;
				string sas;
				string linkrate;
				unsigned int invDword;
				unsigned int disparty;
				unsigned int syncLoss;
				unsigned int rsetProb;
			} Phy;
			

			/* a brief struct of disk drives. The can be merged with the Disks::DiskDrive to make
			 * the code look cleaner */
			typedef struct
			{
				string seacid;     /* logical id of the disk */
				string baynumber;  /* bay number of the disk */
				string sgname;     /* the sgname of the disk */
				string bdname;     /* the block device name of disk */
				/*The ses status bits */
				string status;     /* SES status bits */  
			} Disk;

			
			/**
			 * Accessors
			 */
			string getId() const { return id; }
			string getSasaddr() const { return sasaddr; }
			string getScsiaddr() const { return scsiaddr; }
			string getVendor() const { return vendor; }
			string getProdid() const { return prodid; }
			string getRevision() const { return revision; }
			string getFwversion() const { return fwversion; }
			string getStatus() const { return status; }
			unsigned int getNumofbays() const { return numofbays; }
			string getUnitsn() const { return unitsn; }
			vector<Phy> getPhys() const { return phys; }
			vector<Envelemt> getFans() const { return fans; }
			vector<Envelemt> getPowers() const { return powers; }
			vector<Envelemt> getTemps() const { return temps; }
			
			vector<Disk> getDisks() const { return disks; }
			StorageAdapters::Adapter *getParent() const{ return pParent; }
			

			/**
			 * mutators
			 */
			
			void setId(const string in ){ id = in;}
			void setSasaddr(const string sas){ sasaddr = sas; }
			void setScsiaddr( const string scsi ){ scsiaddr = scsi; }
			void setVendor( const string vd ){ vendor = vd; }
			void setProdid( const string pid ){ prodid = pid; }
			void setRevision( const string rev ){ revision = rev; }
			void setFwversion( const string fw ){ fwversion = fw; }
			void setStatus( const string st ){ status = st; }
			void setNumofbays( const unsigned int nb ){ numofbays = nb; }
			void setUnitsn( const string usn ){ unitsn = usn; }
			void setDisks( const vector<Disk> );
			void setFans( const vector<Envelemt> );
			void setPowers( const vector<Envelemt> );
			void setTemps( const vector<Envelemt> );
			void setPhys( const vector<Phy> );
			void setParent( const StorageAdapters::Adapter * in){ pParent = const_cast<StorageAdapters::Adapter *>(in); }
			
			/**
			 * function to update the enclosures
			 */
			int update();
			int updateEnclinfo();
			int updateFans();
			int updatePowers();
			int updateTemps();
			int updateElemType( const string );
			int updateDisklist();
			int updatePhys();
			int createDiskChildren();
			
			/**
			 * function to get the address of the instance
			 */
			const Enclosure * getAddress() const{ return this;}
			
			
		  protected:

			/*
			 *data members
			 */
			string        id;        /* the sgname */
			string        sasaddr;   /* the sas address */
			string        scsiaddr;  /* the scsi location */
			string        vendor;    /* the vendor name */
			string        prodid;    /* product identification */
			string        revision;  /* product revision */
			string        fwversion; /* firmware version */
			string        status;    /* overall status */
			unsigned int  numofbays; /* number of bays */
			string        unitsn;    /* unit serial number*/
			vector<Phy>      phys;   /* phys */
			vector<Envelemt> fans;   /* the cooling fans */
			vector<Envelemt> powers; /* the power supplies */ 
			vector<Envelemt> temps;  /* the temperature sensors */
			
			vector<Disk>  disks;     /* child element id<->type pair.
									  * enclosure or disk drives */
			StorageAdapters::Adapter * pParent;     /* the pointer to the parent adapter*/	
		  private:
						
		};

		
	  protected:
		/**
		 * data members
		 */ 
		vector<Enclosure *>   encls;         /* An array of enclosure processors.*/
		static EnclProcessors *e_instance;   /* The instance of the object */

	  public:
		/**
		 * constructor
		 */
		EnclProcessors();

		
		/**
		 * Function to get the pointer to the instance of EnclProcessors class
		 */ 
		static EnclProcessors * instance(){
			return e_instance;
		}


/* 		/\** */
/* 		 * copy cnstructor */
/* 		 *\/ */
/* 		EnclProcessors ( const EnclProcessors & orig ){ */
/* 			setEncls ( orig.encls ); */
/* 		} */
		
/* 		/\** */
/* 		 * overload assignment operator */
/* 		 *\/ */
/* 		EnclProcessors& operator=(const EnclProcessors& other){ */
/* 			if( &other != this ){ */
/* 				setEncls( other.encls ); */
/* 			} */
/* 			return *this; */
/* 		} */

		/**
		 * destructor
		 */ 
		virtual ~EnclProcessors(){}

		
		/**
		 * functions that update the data member encls
		 */
		virtual int update();

		/**
		 * accessor to get the enclosures
		 */
		vector<Enclosure *> getEncls() const{ return encls; }
		
/* 		/\** */
/* 		 * mutator */
/* 		 *  */
/* 		 *\/ */
/* 		void setEncls(const vector<Enclosure *> in); */

		/**
		 * functions add a enclosure to the data member encls. It will be called to add enclosure
		 * when adapters getting updated.
		 */ 
		int addEncl( string disksgname, const StorageAdapters::Adapter *parentPointer = 0);
		
		/* function to clear all the enclosures, can be called to free the memory before program terminates. */
		void clear();
	};
}


#endif /* ENCLPROCESSORS_H */
