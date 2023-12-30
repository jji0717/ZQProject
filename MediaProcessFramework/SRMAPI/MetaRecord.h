
// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Dev  : Microsoft Developer Studio
// Name  : MetaRecord.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-5-8
// Desc  : record
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/SRMAPI/MetaRecord.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 38    05-07-29 15:29 Daniel.wang
// 
// 37    05-07-29 15:15 Daniel.wang
// 
// 36    05-07-18 17:22 Daniel.wang
// 
// 35    05-07-18 16:03 Daniel.wang
// 
// 34    05-07-18 15:37 Daniel.wang
// 
// 33    05-07-15 10:05 Daniel.wang
// 
// 32    05-07-14 17:14 Daniel.wang
// 
// 31    05-06-28 7:02p Daniel.wang
// 
// 30    05-06-28 3:05p Daniel.wang
// 
// 29    05-06-28 11:39a Daniel.wang
// 
// 28    05-06-27 10:10a Daniel.wang
// 
// 27    05-06-24 9:12p Daniel.wang
// 
// 26    05-06-24 5:11p Daniel.wang
// 
// 25    05-06-23 9:44p Daniel.wang
// 
// 24    05-06-23 5:53p Daniel.wang
// 
// 23    05-06-22 4:01p Daniel.wang
// 
// ===========================================================================


#ifndef _ZQ_METARECORD_H_
#define _ZQ_METARECORD_H_

#include "edb.h"
#include "SRMCommon.h"

SRM_BEGIN

class DLL_PORT MPFDatabase;
class DLL_PORT MetaRecord;
class DLL_PORT RecordManager;


//record manager property
#define RM_PROP_IMMEDIATELY_FLUSH	1
#define PM_PROP_LAST_ACCESS_TIME	2
#define PM_PROP_LAST_UPDATE_TIME	4
#define PM_PROP_LAST_SAVE_TIME		8
#define PM_PROP_READ_ONLY			16
#define PM_PROP_FULL_LEVEL			32
#define PM_PROP_TRANSACTION			64
#define PM_PROP_CREATE_TIME			128

//database entry state
//not support now
#define ENTRY_STATE_FREE			1
#define ENTRY_STATE_READOPEN		2
#define ENTRY_STATE_OPEN			4
#define ENTRY_STATE_READYDELETE		8

/// MPFDatabase
/// database 
class MPFDatabase : public EntryDB::EDB
{
private:
	static ZQ::common::Mutex	s_mutex;
	static ZQ::common::Mutex	s_instancemutex;
	static size_t				s_nInstanceCount;
	static MPFDatabase*			s_pInstance;
	std::string					m_strLastEntry;

	///constructor
	MPFDatabase(const char* dbfile);
public:

	///destructor
	virtual ~MPFDatabase();

	void enter();
	void leave();

	static MPFDatabase* CreateInstance(const char* dbfile, const char* factorypath);
	static void Release();
	static size_t CountInstance();
};

extern DLL_PORT MPFDatabase*	g_mpf_database_instance;


#define DEF_DB_FILE_PATH "edbb4://localhost/c:\\srm.db"

///start SRM API dll, it can be called only one time 
///@param dbfile - database file name
///@param factorypath - database factory path
///@param sessionleaseterm - session lease term time
///@param taskleaseterm - task lease term time
///@param sessionentry - default session manager entry
///@param taskentry - default task entry manager entry
DLL_PORT void SRMStartup(const char* dbfile = DEF_DB_FILE_PATH,
						 const char* factorypath = NULL,
						 size_t sessionleaseterm = DEF_SESSION_CLEAR_TIME,
						 size_t taskleaseterm = DEF_TASK_CLEAR_TIME,
						 const char* sessionentry = DB_SESSION_ROOT,
						 const char* taskentry = DB_TASK_ROOT);

///clean SRM API dll, it can be called only one time
DLL_PORT void SRMCleanup();

///return true if SRM API is start up
DLL_PORT bool isStartup();

//database access lock
#define DBLOCK ZQ::common::Guard<ZQ::MPF::SRM::MPFDatabase> _db_lock_(*g_mpf_database_instance)

/// a record in database\n
///Record is an attributes set as an entry of database
class MetaRecord : protected EntryDB::EDB
{
private:
	std::string		m_strEntry;
	unsigned int	m_uiProp;
	unsigned int	m_uiState;
	MPFDatabase*	m_pDatabase;

	// this is not a thread safe function, it must run behind DBLOCK
	bool setCurrent(bool create = false);

public:
	///constructor
	///@param entry - record entry
	///@param property - record property
	MetaRecord(const char* entry, unsigned int property = PM_PROP_LAST_SAVE_TIME);

	// transaction record
	//MetaRecord(Transaction& trans, const char* entry, unsigned int property = PM_PROP_LAST_SAVE_TIME);

	///destructor
	virtual ~MetaRecord();

	/// set hidden attribute
	///@param key - attribute key
	///@param value - attribute value, if value is NULL, set current time as value
	bool setHiddenAttr(const char* key, const char* value = NULL);
	
	///get record prop
	unsigned int getProp() const;

	///get record current state
	unsigned int getState() const;

	///@return true if record is readonly
	bool isReadOnly() const;
	///@return true if record is immediately flush
	bool isImmediately() const;
	///@return true if record will save last access time
	bool isSaveLastAccess() const;
	///@return true if record will save last update time
	bool isSaveLastUpdate() const;
	///@return true if record will save last storage time
	bool isSaveLastStorage() const;
	///@return true if record will save create time
	bool isSaveCreateTime() const;

	///list children entries of this record- this must be allocate by user
	///@param entry - entries buffer
	///@param bufmax - max of entry buffer length
	///@param countmax - max of entries buffer count
	///@return entries count
	size_t listChildren(char** entry, size_t bufmax, size_t countmax);

	///list children entries of this record- this allocated by itself
	char** listChildren(size_t& count);

	static void deleteList(char** pstrentry, size_t count);

	/// getEntry
	/// @return current entry path string
	const char* getEntry() const;

	///get attribute(string)
	///@param key - attribute key
	///@param value - attribute value buffer
	///@param max - max buffer length
	///@return NULL if error, else return value buffer pointer
	char* get(const char* key, char* value, size_t max);

	///get attribute(integer)
	///@param key - attribute key
	///@return 0xffffffff if error, else return value
	size_t get(const char* key);

	///get keys list
	///@param index - key number
	///@param buf - key buffer
	///@param bufsize - buffer size
	///@return NULL if error, else return key buffer pointer
	char* getKey(size_t index, char* buf, int bufsize);

	///set attribute(string)
	///@param key - attribute key 
	///@param value - attribute value
	///@return true if ok
	bool set(const char* key, const char* value);
	
	///set attribute(integer)
	///@param key - attribute key 
	///@param value - attribute value
	///@return true if ok
	bool set(const char* key, size_t value);

	///save memory record to database
	///@return true if ok
	bool save();

	///load memory record from database
	///@return true if ok
	bool load();

	// test function: print database entries to screen or other
	void print(void (*PRINT_PROC)(const char* message), int depth = 0);

	///remove record 
	///@param entry - entry string
	///@return true if ok
	bool remove(const char* entry);
};

/// record manager
class RecordManager : public MetaRecord
{
private:
	class DLL_PORT LeaseTermThread : public ZQ::common::NativeThread
	{
	private:
		RecordManager&			m_manager;
		size_t					m_zLeaseTerm;
		//size_t					m_starttime;
		//size_t					m_booktime;

	public:
		LeaseTermThread(RecordManager& manager, size_t zLeaseTerm);
		int run(void);
	} m_thread;

public:
	///constructor
	///@param entry - record manager entry
	///@param property - record property
	///@param leaseterm - lease term time, do not use lease term if the "leaseTerm" is 0
	RecordManager(const char* entry, unsigned int property = PM_PROP_LAST_SAVE_TIME, size_t leaseterm = 0);

	///destructor
	virtual ~RecordManager();

	///call back function which called when lease term timer
	///@param entry - record entry
	///@param leaseterm - lease term time
	///@return false if you want delete this record
	virtual bool OnLeaseTerm(const char* entry, size_t leaseterm);

	///call back function which caled when lease term return false
	///@param entry - record manager entry
	///@param leaseterm - lease term time
	virtual void OnLostLeaseTerm(const char* entry, size_t leaseterm);

	///run lease term timer thread
	///@return true if ok
	bool beginLeaseTerm();
};

SRM_END

#endif//_ZQ_METARECORD_H_

