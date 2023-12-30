 
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
// Name  : MetaRecord.cpp
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-5-9
// Desc  : record
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/SRMAPI/MetaRecord.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 55    05-08-31 18:35 Jie.zhang
// 
// 54    05-08-23 15:13 Jie.zhang
// 
// 53    05-08-10 10:55 Jie.zhang
// 
// 52    05-07-29 15:29 Daniel.wang
// 
// 51    05-07-29 15:15 Daniel.wang
// 
// 50    05-07-22 13:42 Daniel.wang
// 
// 49    05-07-18 16:07 Daniel.wang
// 
// 48    05-07-18 16:03 Daniel.wang
// 
// 47    05-07-18 15:37 Daniel.wang
// 
// 46    05-07-15 11:51 Daniel.wang
// 
// 45    05-07-14 17:12 Daniel.wang
// 
// 44    05-06-28 7:02p Daniel.wang
// 
// 43    05-06-28 5:08p Daniel.wang
// 
// 42    05-06-28 11:39a Daniel.wang
// 
// 41    05-06-27 6:25p Daniel.wang
// 
// 40    05-06-27 11:35a Daniel.wang
// 
// 39    05-06-27 10:10a Daniel.wang
// 
// 38    05-06-24 9:12p Daniel.wang
// 
// 37    05-06-24 5:11p Daniel.wang
// 
// 36    05-06-23 9:44p Daniel.wang
// 
// 35    05-06-23 5:53p Daniel.wang
// 
// 34    05-06-22 4:01p Daniel.wang
// 
// ===========================================================================

#include "MetaSession.h"
#include "MetaTask.h"
#include "mpfexception.h"
#include "MPFCommon.h"

using namespace ZQ::MPF::utils;


SRM_BEGIN

#define PRINT_TAB(count,print_proc) {for(int _TAB_SPEC_COUNT = 0;_TAB_SPEC_COUNT<(count);++_TAB_SPEC_COUNT)\
	(print_proc)("\t");}

void SRMStartup(const char* dbfile, const char* factorypath,
				size_t sessionleaseterm, size_t taskleaseterm,
				const char* sessionentry, const char* taskentry)
{
	if (NULL != g_mpf_database_instance)
		return;	//make sure that SRMStartup has run only one time

	g_mpf_database_instance = MPFDatabase::CreateInstance(dbfile, factorypath);
	if (NULL == g_mpf_session_manager_instance)
	{
		g_mpf_session_manager_instance = new SessionManager(sessionleaseterm, sessionentry);
		g_mpf_session_manager_instance->beginLeaseTerm();
	}
	if (NULL == g_mpf_task_manager_instance)
	{
		g_mpf_task_manager_instance = new TaskManager(taskleaseterm, taskentry);
		g_mpf_task_manager_instance->beginLeaseTerm();
	}
}

void SRMCleanup()
{
	if (0 == MPFDatabase::CountInstance())
		return; //clean up has done in other place

	if (MPFDatabase::CountInstance() < 3)
	{
		throw SRMException("[SRMCleanup]: Database exception: instance count error,\
			maybe there is a memory overflow exception in your application,\
			please verify that all the MetaRecord/RecordManager/MetaTask/TaskManager/MetaSession/\
			/SessionManager/MetaResource/ResourceManager/MetaNode/NodeManager instances have been clean up");
	}

	/// when comes here, the instance count should be 3: 1 for global db, 1 for session manager
	/// and the other for task manager
	if (3 == MPFDatabase::CountInstance())
	{
		//_sdel(g_mpf_database_instance);
		_sdel(g_mpf_session_manager_instance);
		_sdel(g_mpf_task_manager_instance);
	}
	MPFDatabase::Release();
	g_mpf_database_instance = NULL;
}

bool isStartup()
{ 
	return NULL != g_mpf_database_instance; 
}

size_t MPFDatabase::s_nInstanceCount	= 0;
MPFDatabase* MPFDatabase::s_pInstance	= NULL;
ZQ::common::Mutex MPFDatabase::s_mutex;
ZQ::common::Mutex MPFDatabase::s_instancemutex;
MPFDatabase* g_mpf_database_instance	= NULL;

MPFDatabase::MPFDatabase(const char* dbfile)
{
	if (NULL == dbfile)
		throw SRMException("[MPFDatabase::MPFDatabase] database file name is empty");

	if (!connect(dbfile))
		throw SRMException("[MPFDatabase::MPFDatabase] can not connect database file,\
		please check that the edbb4.edm plugin file in special dirctory");
}

MPFDatabase::~MPFDatabase()
{
}

void MPFDatabase::enter()
{
	s_mutex.enter();
	m_strLastEntry = getCurrentEntry();
}

void MPFDatabase::leave()
{
	if (!m_strLastEntry.empty())
	{
		openEntry(m_strLastEntry.c_str());
		m_strLastEntry = "";
	}
	s_mutex.leave();
}

MPFDatabase* MPFDatabase::CreateInstance(const char* dbfile, const char* factorypath)
{
	common::Guard<common::Mutex> instLock(s_instancemutex);

	if (0 == s_nInstanceCount++)
	{
		std::string strModulePath;
		if (NULL != factorypath)
			strModulePath = factorypath;
		else
			strModulePath = utils::FilePath::getModulePath();

		EntryDB::gblAddinManager.populate(strModulePath.c_str());
		pFactory = new EntryDB::EDBFactory(EntryDB::gblAddinManager);
		if (NULL == pFactory)
			throw SRMException("[MetaRecord::MetaRecord] can not malloc the edos factory, memory error");

		s_pInstance = new MPFDatabase(dbfile);
	}

	return s_pInstance;
}

void MPFDatabase::Release()
{
	common::Guard<common::Mutex> instLock(s_instancemutex);

	if (0 == --s_nInstanceCount)
	{
		_sdel(pFactory);
		_sdel(s_pInstance);
	}
}
size_t MPFDatabase::CountInstance()
{
	return s_nInstanceCount;
}

bool MetaRecord::setHiddenAttr(const char* key, const char* value)
{
	if (isReadOnly())
		return false;

	if (NULL == key)
	{
		assert(false);
		return false;
	}

	char strTime[MAX_INT32_STR_LEN] = {0};
	const char* strTemp = value;
	if (NULL == value)
	{
		time_t curtime;
		_snprintf(strTime, MAX_INT32_STR_LEN-1, "%ld", (long)time(&curtime));
		strTemp = strTime;
	}

	bool bRtn = false;
	if (isImmediately())
	{
		DBLOCK;
		if (!setCurrent(true))
		{
			return false;
		}
		bRtn = m_pDatabase->setAttribute(key, strTemp);
	}
	else
		bRtn = setAttribute(key, strTemp);

	return bRtn;
}

bool MetaRecord::setCurrent(bool create)
{
	return (m_pDatabase->openEntry(m_strEntry.c_str(), create));
}


MetaRecord::MetaRecord(const char* entry, unsigned int property/*PM_PROP_LAST_SAVE_TIME*/)
:m_uiProp(property), m_uiState(ENTRY_STATE_FREE)
{
	if (!isStartup())
		throw SRMException("[MetaRecord::MetaRecord] SRM is not start up, you must run SRMStartup before all the DB Record process");

	if (NULL == entry)
		throw SRMException("[MetaRecord::MetaRecord] Entry path is empty");

	m_pDatabase = MPFDatabase::CreateInstance(NULL, NULL);
	m_strEntry = entry;

	if (!isImmediately())
	{
		if (!connect(DB_MEMORY_PATH))
		{
			throw SRMException("[MetaRecord::MetaRecord] Can not create in-memory db");
		}
		load();
	}
	else
	{
		DBLOCK;
		if (!setCurrent(!isReadOnly()))
			throw SRMException("[MetaRecord::MetaRecord] can not set current entry, please sure that this record entry is exist if readonly");
	}

	if (isSaveCreateTime())
		setHiddenAttr(CREATE_TIME);
	
	if (isSaveLastAccess())
		setHiddenAttr(LAST_ACCESS);
}

MetaRecord::~MetaRecord()
{
	if (!isImmediately())
		save();
	
	MPFDatabase::Release();
}

unsigned int MetaRecord::getProp() const
{
	return m_uiProp;
}

unsigned int MetaRecord::getState() const
{
	return m_uiState;
}

bool MetaRecord::isReadOnly() const
{
	return PM_PROP_READ_ONLY&m_uiProp;
}

bool MetaRecord::isImmediately() const
{
	return RM_PROP_IMMEDIATELY_FLUSH&m_uiProp;
}

bool MetaRecord::isSaveLastAccess() const
{
	return PM_PROP_LAST_ACCESS_TIME&m_uiProp;
}

bool MetaRecord::isSaveLastUpdate() const
{
	return PM_PROP_LAST_UPDATE_TIME&m_uiProp;
}

bool MetaRecord::isSaveLastStorage() const
{
	return PM_PROP_LAST_SAVE_TIME&m_uiProp;
}

bool MetaRecord::isSaveCreateTime() const
{
	return PM_PROP_CREATE_TIME&m_uiProp;
}

size_t MetaRecord::listChildren(char** entry, size_t bufmax, size_t countmax)
{
	DBLOCK;
	if (!setCurrent(false))
		return 0;

	size_t count = 0;
	if (m_pDatabase->openFirstChild())
	{
		for (; count < countmax; ++count)
		{
			const char* strentry = m_pDatabase->getCurrentEntry();
			sfstrncpy(entry[count], strentry, bufmax);

			if (!m_pDatabase->openNextSibling())
				break;
		}
		return ++count;
	}

	return 0;
}

char** MetaRecord::listChildren(size_t& count)
{
	DBLOCK;
	if (!setCurrent(false))
	{
		count = 0;
		return NULL;
	}

	std::vector<char*> childlist;

	if (m_pDatabase->openFirstChild())
	{
		do 
		{
			const char* pstrEntry = m_pDatabase->getCurrentEntry();
			if (!pstrEntry)
				continue;
			
			char* pStr = new char[strlen(pstrEntry)+1];
			strcpy(pStr, pstrEntry);
			childlist.push_back(pStr);
		}while (m_pDatabase->openNextSibling());
	}

	count = childlist.size();
	if (0 == count)
		return NULL;

	char** result = new char*[count];
	for (int i = 0; i < count; ++i)
	{
		result[i] = childlist[i];
	}

	return result;
}

void MetaRecord::deleteList(char** pstrentry, size_t count)
{
	if (NULL != pstrentry)
	{
		for (int i = 0; i < count; ++i)
		{
			if (NULL != pstrentry[i])
				delete[] pstrentry[i];
		}

		delete[] pstrentry;
	}
}

const char* MetaRecord::getEntry() const
{
	return m_strEntry.c_str();
}

char* MetaRecord::get(const char* key, char* value, size_t max)
{
	const char* strTemp = NULL;
	if (isImmediately())
	{
		DBLOCK;
		if (!setCurrent())
			return NULL;
		strTemp = m_pDatabase->getAttribute(key);

		if (NULL == strTemp)
		return NULL;
		sfstrncpy(value, strTemp, max);
	}
	else
	{
		strTemp = getAttribute(key);
		
		if (NULL == strTemp)
		return NULL;
		sfstrncpy(value, strTemp, max);
	}

	if (isSaveLastAccess())
		setHiddenAttr(LAST_ACCESS);

	return value;
}

size_t MetaRecord::get(const char* key)
{
	char strTemp[MAX_INT32_STR_LEN] = {0};
	if (NULL == get(key, strTemp, MAX_INT32_STR_LEN))
		return -1;

	size_t result = 0;
	sscanf(strTemp, "%ui", &result);
	return result;
}

char* MetaRecord::getKey(size_t index, char* buf, int bufsize)
{
	if (isImmediately())
	{
		DBLOCK;
		if (!setCurrent())
			return NULL;
		const char* strTemp = m_pDatabase->getAttrName(index);
		if (NULL == strTemp)
			return NULL;
		return sfstrncpy(buf, strTemp, bufsize);
	}
	else
	{
		const char* strTemp = getAttrName(index);
		return sfstrncpy(buf, strTemp, bufsize);
	}
}

bool MetaRecord::set(const char* key, const char* value)
{
	bool bRtn = setHiddenAttr(key, value);

	if (bRtn)
	{
		if (isSaveLastAccess())
			setHiddenAttr(LAST_ACCESS);
		if (isSaveLastUpdate())
			setHiddenAttr(LAST_UPDATE);
	}

	return bRtn;
}

bool MetaRecord::set(const char* key, size_t value)
{
	char strTemp[MAX_INT32_STR_LEN] = {0};
	_snprintf(strTemp, MAX_INT32_STR_LEN-1, "%d", value);

	return set(key, strTemp);
}

bool MetaRecord::save()
{
	if (isReadOnly())
		return false;

	if (isSaveLastStorage())
		setHiddenAttr(LAST_SAVE);

	DBLOCK;
	if (!setCurrent(true))
		return false;
	return m_pDatabase->import(*this, NULL, 0);
}

bool MetaRecord::load()
{
	DBLOCK;
	if (!setCurrent(!isReadOnly()))
		return false;

	return (m_pDatabase->export(*this, NULL, 0));
}

void MetaRecord::print(void (*PRINT_PROC)(const char* message), int depth)
{
	for (int i = 0; true; ++i)
	{
		char strKey[MAX_DB_ENTRY_LEN] = {0};
		if (NULL == getKey(i, strKey, MAX_DB_ENTRY_LEN))
			break;

		PRINT_TAB(depth, PRINT_PROC);
		
		char strValue[MAX_DB_VALUE_LEN] = {0};
		if (NULL == get(strKey, strValue, MAX_DB_VALUE_LEN))
			continue;

		char strPrint[MAX_DB_ENTRY_LEN+MAX_DB_VALUE_LEN] = {0};
		_snprintf(strPrint, MAX_DB_ENTRY_LEN+MAX_DB_VALUE_LEN-1, "%s - %s", strKey, strValue);
		
		PRINT_PROC(strPrint);
	}

	size_t count;
	char** pstrEntry = listChildren(count);
	for (i = 0; i < count; ++i)
	{
		MetaRecord mr(pstrEntry[i], PM_PROP_READ_ONLY);
		mr.print(PRINT_PROC, depth+1);
	}

	deleteList(pstrEntry, count);
}

bool MetaRecord::remove(const char* entry)
{
	DBLOCK;

	bool bRet;
	try
	{
		bRet = m_pDatabase->deleteEntry(entry);
	}
	catch(ZQ::MPF::EntryDB::EDBException & e)
	{
		MPFLog(MPFLogHandler::L_ERROR, "remove entry: %s fail, %s", entry, e.getString());
		bRet = false;
	}
	
	return bRet;
}

RecordManager::LeaseTermThread::LeaseTermThread(RecordManager& manager, size_t zLeaseTerm)
:m_manager(manager), m_zLeaseTerm(zLeaseTerm)
{
}

int RecordManager::LeaseTermThread::run(void)
{
	while (ZQ::common::NativeThread::RUNNING == getStatus())
	{
		Sleep(m_zLeaseTerm/3);

		size_t count;
		char** pstrEntry = m_manager.listChildren(count);
		for (size_t j = 0; j < count; ++j)
		{
			if (!m_manager.OnLeaseTerm(pstrEntry[j], m_zLeaseTerm))
				m_manager.OnLostLeaseTerm(pstrEntry[j], m_zLeaseTerm);
		}
		
		if (pstrEntry)
			m_manager.deleteList(pstrEntry, count);
	}
	return 0;
}

void RecordManager::OnLostLeaseTerm(const char* entry, size_t leaseterm)
{
	remove(entry);
}

bool RecordManager::OnLeaseTerm(const char* entry, size_t leaseterm)
{
	return true;
}

RecordManager::RecordManager(const char* entry, unsigned int property, size_t leaseterm)
:m_thread(*this, leaseterm), MetaRecord(entry, property)
{
}

RecordManager::~RecordManager()
{
}

bool RecordManager::beginLeaseTerm()
{
	return m_thread.start();
}

SRM_END
