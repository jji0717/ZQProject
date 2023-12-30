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
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EntryDB.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EntryDB.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 6     05-04-14 23:04 Daniel.wang
// 
// 5     4/14/05 10:13a Hui.shao
// 
// 2     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 1     4/13/05 6:02p Hui.shao
// 
// 3     4/12/05 5:46p Hui.shao
// ============================================================================================

#ifndef __EDOS_h__
#define __EDOS_h__

#include "../MPFCommon.h"
#include <exception>

#ifdef ENTRYDB_STATIC
#  define ENTRYDB_API
#elif defined(WIN32) || defined (_WIN32)
#  ifdef EDOS_EXPORTS
#     define ENTRYDB_API __declspec(dllexport)
#  else 
#     define ENTRYDB_API __declspec(dllimport)
#     pragma comment(lib, "edos.lib")
#  endif
#endif

#define ENTRYDB_NAMESPACE ZQ::MPF::EntryDB

#define ENTRYDB_WITH_NAMESPACE
#ifdef ENTRYDB_WITH_NAMESPACE
#  define ENTRYDB_NAMESPACE_BEGIN		namespace ZQ { namespace MPF { namespace EntryDB {
#  define ENTRYDB_NAMESPACE_END		}}};
#else
#  define ENTRYDB_NAMESPACE_BEGIN
#  define ENTRYDB_NAMESPACE_END
#endif

#define ADDIN_MOD_EXT "edm"

#include <string>

ENTRYDB_NAMESPACE_BEGIN

class ENTRYDB_API ManagedObject;
class ENTRYDB_API EDBException;
class ENTRYDB_API Mutex;

class ManagedObject
{
public:
	ManagedObject(void);
	virtual ~ManagedObject(void);
	virtual void free(void) =0;
};

#define DECLARE_MANAGED_OBJ(MOClass) \
   public: \
		virtual void free() { delete this; } 

typedef ManagedObject MOBJ;
typedef ManagedObject* PMOBJ;

const extern std::string endl;

class EDBException : public std::exception 
{
private:
//	std::string _what;
	char	_what[512];

public:
//	EDBException(const std::string& what_arg) throw(): _what(what_arg) {}
	EDBException(const std::string& what_arg) throw()
	{strcpy(_what, what_arg.c_str());}
	EDBException(const char* what_arg) throw()
	{strcpy(_what, what_arg);}

	virtual ~EDBException() throw() {}
	virtual const char *getString() const {	return _what; }
};

class Mutex
{

public:
	Mutex();

	virtual ~Mutex();

	void enter(void);
	bool tryEnter(timeout_t to =0);

	void leave(void);

private:

#ifndef WIN32
#  ifndef	PTHREAD_MUTEXTYPE_RECURSIVE
	volatile int _level;
	volatile Thread *_tid;
#  endif
	pthread_mutex_t	_mutex;
#  else
	HANDLE mutex;
#endif

};

ENTRYDB_NAMESPACE_END

#endif // __EDOS_h__
