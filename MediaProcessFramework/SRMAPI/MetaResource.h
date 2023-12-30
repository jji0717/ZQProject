
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
// Name  : MetaResource.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-4-7
// Desc  : resouces manager for database
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/MediaProcessFramework/SRMAPI/MetaResource.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 25    05-07-29 15:29 Daniel.wang
// 
// 24    05-07-29 15:15 Daniel.wang
// 
// 23    05-07-27 17:33 Daniel.wang
// 
// 22    05-07-26 14:50 Daniel.wang
// 
// 21    05-06-28 3:05p Daniel.wang
// 
// 20    05-06-28 11:39a Daniel.wang
// 
// 19    05-06-24 5:11p Daniel.wang
// 
// 18    05-06-14 4:58p Daniel.wang
// 1     05-04-08 10:40 Daniel.wang
// create
// ===========================================================================

#ifndef _ZQ_DBRESOURCE_H_
#define _ZQ_DBRESOURCE_H_

#include "MetaSession.h"
#include "book.h"

#define RESOURCE_TIME_OUT_KEY	"ResourceTimeOut"

SRM_BEGIN

class DLL_PORT ResourceManager;
class DLL_PORT MetaResource;

class ResourceBooker : public Booker
{
private:
	ResourceManager&	m_rm;

protected:
	///call back function which called at the start book time
	///@param beginTime - begin time of book period
	///@param endTime - end time of book period
	///@param strResource - resource id string
	///@return the book period will be remove if return true, else the false
	virtual bool OnStartAction(size_t time, string resId, const ActionList& subResList);

	virtual void OnEndAction(size_t time, string resId, const ActionList& subResList);

public:
	///timeout
	ResourceBooker(ResourceManager& rm, const char* type);
};

// -----------------------------
//ResourceManager
// -----------------------------
/// manager for resource in database
class ResourceManager : public RecordManager
{
private:
	std::string	m_strRootEntry;
	std::string	m_strTypeEntry;

	//ResourceBooker*		m_pBooker;
	bool				m_bBook;
	
protected:
	///score\n
	///get resource score number for allocate
	///@param resourceentry - resource entry path
	///@param sessionentry - session entry path
	///@return - score number
	virtual BYTE score(const char* resourceentry, const char* sessionentry);

	///call back function which called at lease term time
	///@param entry - resource entry
	///@param leaseterm - lease term time
	///@return return true if this resource is dead
	bool OnLeaseTerm(const char* entry, size_t leaseterm);

	///compose the url string for resource
	///@param entry - resource entry
	///@param url - url string buffer
	///@param size - max size of url string buffer
	virtual char* compose(const char* entry, char* url, size_t size);

public:
	///constructor
	///@param type - resource type
	///@param leaseterm - lease term time
	///@param entry - resource manager entry
	ResourceManager(const char* type, size_t leaseterm, bool book = false, const char* entry = DB_RESOURCE_ROOT);

	///constructor
	///@param type - resource type
	ResourceManager(const char* type);

	virtual ~ResourceManager();

	///allocate\n
	///allocate a resource for user request
	///@param sessionentry - session entry
	///@param url - url string buffer
	///@param size - max size of url string buffer
	///@return - resource url string
	virtual char* allocate(const char* sessionentry, char* url, size_t size,
		ActionList& subResList, size_t beginTime = 0, size_t endTime = 0);


	virtual bool OnStartAction(size_t time, string resId, const ActionList& subResList)
	{ return true; }

	virtual void OnEndAction(size_t time, string resId, const ActionList& subResList)
	{ }

	///free\n
	///delete the resource witch allocate by ResourceManager::allocate
	///@param entry - resource entry
	virtual void free(const char* entry);

	///get resource root entry
	const char* getRootEntry() const;

	///get current type resource entry
	const char* getTypeEntry() const;
};

// -----------------------------
//MetaResource
// -----------------------------
/// resource in database
class MetaResource : public MetaRecord
{
public:
	///constructor
	///@param mgr - resource manager
	///@param id - resource id string
	MetaResource(const ResourceManager& rm, const char* resid, size_t timeout/*second*/, unsigned int property = PM_PROP_LAST_SAVE_TIME);

	MetaResource(const char* rootentry, const char* resid, size_t timeout/*second*/, unsigned int property = PM_PROP_LAST_SAVE_TIME);
};

SRM_END

#endif//_ZQ_DBRESOURCE_H_
