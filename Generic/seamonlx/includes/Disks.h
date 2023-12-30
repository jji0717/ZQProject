/** @file Disks.h
 *
 *  Disks class declaration.
 *  Defines the base class Disks
 *  
 *  SeaChange International
 *  
 *  Revision History
 *  
 *  05-03-2010 jiez
 *    - created
 *
 *  05-10-2010 jiez
 *   - Changed data member of class Disks to contain pointers instead of the DiskDrive objects.
 *   - Changed constructor to allow only one instance of Disks exists.
 *   - Added d_instance pointer to Disks object and ref_count to keep tracking of the references
 *     of the instance
 *   - Added member functions addDisk(), deleteDisk() and clear() to outer class Disks
 *   - Added constructor that takes sgname of disk of inner class DiskDrive
 *   - Added parent pointers to the parent device instance in the inner class DiskDrive
 */

#ifndef DISKS_H
#define DISKS_H

#include <stdlib.h>
#include <pthread.h>
#include <map>
#include <vector>
#include "EnclProcessors.h"
#include "StorageAdapters.h"

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
	class Disks
	{
	  public:

		/**
		 * nested class Disk
		 */
		class DiskDrive{
		  public:

			/* Default constructor */
			DiskDrive():pEncl(NULL), pAdpt(NULL){}

			/**
			 * construct a disk with given id. 
			 */ 
			DiskDrive( const string sgname );
			
			/**
			 * Accessors
			 */
			string getId() const{ return id;}
			string getSgname() const { return sgname;}
			string getBdname() const { return bdname;}
			string getScsiaddr() const { return scsiaddr;}
			string getState() const { return state;}
			string getSize() const { return size;}
			string getCa() const { return ca;}
			string getVendor() const { return vendor;}
			string getModel() const { return model;}
			string getSerialnum() const { return serialnum;}
			string getFwver() const { return fwver;}
			string getTemp() const { return temp;}
			string getIoerr() const { return ioerr;}
			string getBlkw() const { return blkw;}
			string getBlkr() const { return blkr;}
			EnclProcessors::Enclosure *getPEncl() const{  return pEncl; }
			StorageAdapters::Adapter * getPAdpt() const { return pAdpt; }
			
			
			/**
			 * mutators
			 */
			
			void setId(const string in){ id = in;} 
			void setSgname(const string in){ sgname = in;}
			void setBdname(const string in){ bdname = in;}
			void setScsiaddr(const string in){ scsiaddr = in;}
			void setState(const string in){ state = in;}
			void setSize(const string in){ size = in;}
			void setCa(const string in){ ca = in;} 
			void setVendor(const string in){ vendor = in;}
			void setModel(const string in){ model = in;}
			void setSerialnum(const string in){ serialnum = in;}
			void setFwver(const string in){ fwver = in;}
			void setTemp(const string in){ temp = in;}
			void setIoerr(const string in){ ioerr = in;}
			void setBlkw(const string in){ blkw = in;}
			void setBlkr(const string in){ blkr = in;}
			void setParent(const EnclProcessors::Enclosure * in){ pEncl = const_cast<EnclProcessors::Enclosure *> (in);}
			void setParent(const StorageAdapters::Adapter * in){ pAdpt = const_cast<StorageAdapters::Adapter *>(in); }
			
			
			/**
			 * function to update the disk
			 */
			int update();

			/**
			 * parent device type
			 */  
			enum p_type
				{
					none,
					Enclosure,
					Adapter
				};
			
			
		  protected:
			
			/*
			 *data members
			 */
			string  id;        /* logical Address, either sas address or scsi name string */
			string  sgname;    /* the os name */
			string  bdname;    /* block device name SD name or MD name*/
			string  scsiaddr;  /* SCSI Address */
			string  state;     /* context specific state, SCSI device state for SD, else for MD*/
			string  size;      /* capacity */
			string  ca;        /* cache attribute */
			string  vendor;    /* manufacturer */
			string  model;     /* model number */
			string  serialnum; /* serial number */  
			string  fwver;     /* firmware version */    
			string  temp;      /* drive temperature */
			string  ioerr;     /* I/O errors*/
			string  blkw;      /* blocks written */
			string  blkr;      /* blocks read */

			/* parent pointers, they are mutual execlusive */
			EnclProcessors::Enclosure *  pEncl;   /* the pointer to the parent enclosure */
			StorageAdapters::Adapter * pAdpt;     /* the pointer to the parent adapter*/
		};
		
		
		
	  protected:
		/**
		 * data members
		 */ 
		vector<DiskDrive *>  disks;        /* An array of disks */
		static Disks         *d_instance;  /* The instance of the object */
		

	  public:
		
		/**
		 * constructor
		 */
		Disks();

		/**
		 * destructor
		 */ 
		virtual ~Disks(){}
		
		
		/**
		 * functions that update the data member disks as well as each element
		 * of the disks vector.
		 */
		virtual int update();

		/**
		 * accessor to get the disk list
		 */
		vector<DiskDrive *> getDisks() { return disks; }

		/**
		 * Function to get the pointer to the instance of Disks class
		 */ 
		static Disks * instance(){
			return d_instance;
		}

		/**
		 * functions add a disk to the data member disks. They will be called to add new disks either
		 * when program starts or when a new disk drive is inserted.
		 */ 
		int addDisk( string disksgname );
		int addDisk( string disksgname, const EnclProcessors::Enclosure *parentPointer);
		int addDisk( string diskbdname, const StorageAdapters::Adapter *parentPointer);

		/**
		 * function removes a disk from data member disks, be called when a drive is pulled out.
		 */
		void deleteDisk( string disksgname );

		/* function to clear all the disks, can be called to free the memory before program terminates. */
		void clear();
	};
}



#endif /* DISKS_H */
