// ============================================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
// This was copied from enterprise domain object sys, edos's copyright is belong to Hui Shao
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Hui Shao
// Desc  : generic definition
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EntryDB.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EntryDB.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 3     4/14/05 10:13a Hui.shao
// 
// 2     4/13/05 7:01p Hui.shao
// 
// 1     4/13/05 6:02p Hui.shao
// 
// 2     4/12/05 5:46p Hui.shao
// ============================================================================================

#include "EntryDB.h"
#include <vector>

ENTRYDB_NAMESPACE_BEGIN

const std::string endl="\r\n";

class ObjManager
{
public:
	ObjManager();
	~ObjManager();

	bool reg(PMOBJ);
	bool unreg(PMOBJ);
//	long size();

private:

	void clear();

	// about the control loop
	typedef std::vector<PMOBJ > vedos_t;
	vedos_t vedos;
	Mutex vedos_lock;
	bool bDestructor;
};

ObjManager theObjMgr;

ObjManager::ObjManager()
          : vedos_lock(), bDestructor(false)
{
}

ObjManager::~ObjManager()
{
	bDestructor = true;
	clear();
}

void ObjManager::clear()
{
	vedos_lock.enter();
	// clean up the active conns
	for (vedos_t::iterator i= vedos.begin(); i <vedos.end(); i++)
		try
		{
			(*i)->free();
		}
		catch (...){}

	vedos.clear();

	vedos_lock.leave();
}

bool ObjManager::reg(PMOBJ pedo)
{
	if (pedo == NULL)
		return false;

	bool ret=false;

	vedos_lock.enter();

	// clean up the active conns
	vedos_t::iterator i;
	for (i= vedos.begin(); i  <vedos.end(); i++)
		if (*i == pedo)
			break;
	
	if (i >=vedos.end())
	{
		vedos.push_back(pedo);
		ret=true;
	}

	vedos_lock.leave();

	return ret;
}

bool ObjManager::unreg(PMOBJ pedo)
{
	bool ret=false;

	if (pedo == NULL)
		return false;

	if (bDestructor)
		return true;

	vedos_lock.enter();

	// clean up the active conns
	vedos_t::iterator i;
	for (i= vedos.begin(); i  <vedos.end(); i++)
		if (*i == pedo)
			break;
	
	if (i <vedos.end())
	{
		vedos.erase(i); // no delete here
		ret=true;
	}

	vedos_lock.leave();

	return ret;
}

ManagedObject::ManagedObject(void)
{
	theObjMgr.reg(this);
}

ManagedObject::~ManagedObject(void)
{
	theObjMgr.unreg(this);
}

//void ManagedObject::free(void)
//{
//	delete this;
//}
#ifdef _WIN32
Mutex::Mutex()
      :mutex(NULL)
{
//	mutex = ::CreateMutex(NULL,FALSE,NULL);
	mutex = ::CreateEvent(NULL, true, false, NULL);
	if(mutex ==NULL)
		throw (const char*) "fail to create mutex";

	::ResetEvent(mutex);
	leave();
}

void Mutex::enter(void)
{
	WaitForSingleObject(mutex, INFINITE);
	::ResetEvent(mutex);
}

bool Mutex::tryEnter(timeout_t to)
{
//	to = (to<0) ? 0: to;
	bool succ = (WaitForSingleObject(mutex, to) == WAIT_OBJECT_0);
	if (succ)
		::ResetEvent(mutex);
	return succ;
}

void Mutex::leave(void)
{
//	if (!::ReleaseMutex(mutex))
//		throw (const char*) "fail to release mutex";
	
	::SetEvent(mutex);
}

Mutex::~Mutex()
{
	::CloseHandle(mutex);
}
#else // _WIN32
#  error not implemented
#endif // _WIN32

ENTRYDB_NAMESPACE_END
