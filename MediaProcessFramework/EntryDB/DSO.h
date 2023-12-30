// ============================================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
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
// Desc  : define dynamic shared object
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/DSO.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/DSO.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 4     4/14/05 10:11a Hui.shao
// 
// 2     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 1     4/12/05 5:11p Hui.shao
// ============================================================================================

#ifndef __DSO_h__
#define __DSO_h__

#include "EntryDB.h"

ENTRYDB_NAMESPACE_BEGIN

class ENTRYDB_API DSO;
class ENTRYDB_API DSOInterface;

class DSO
{
	friend class DSOInterface;
public:
	DSO(const char* filename=NULL);
	virtual ~DSO();

	bool Load(const char* filename);
	void Free();
	bool IsLoaded();
	const char* getImageName() const;
	int32 getMappedInterfaceCount() const { return mItCount; }

protected:
	HINSTANCE library;
	char mFilename[256];
	virtual bool mapExternFunc(bool loading) { return true; }
	int32 mItCount;
};

#define DECLARE_PROC_SO(myclass, superclass) \
   public: \
		myclass(const char* filename=NULL) : superclass(filename) {	if (filename !=NULL) Load(filename); } \
       virtual ~myclass() {}

#define DEFINE_PROC(result, name, args) \
    protected: \
        typedef result (*name##_PROC) args; \
    public: \
        name##_PROC name;

#define DEFINE_PROC_SEPCAPI(result, callapi, name, args) \
    protected: \
        typedef result (callapi *name##_PROC) args; \
    public: \
        name##_PROC name;

#define PROC_TABLE_BEGIN() \
	protected: \
	   virtual bool mapExternFunc(bool loading) { bool succ =true;

#define PROC_TABLE_END()   return succ; }

#define IMPLEMENT_DSO_PROC(name) \
    IMPLEMENT_DSO_PROC_SPECIAL(name, #name)

#define IMPLEMENT_DSO_PROC_SPECIAL(name, externalname) \
    name = loading ? (name##_PROC) GetProcAddress(library, externalname) : NULL; \
	if(loading && name == NULL) \
		succ = false;

class DSOInterface : public MOBJ
{
	DECLARE_MANAGED_OBJ(DSOInterface)

public:
	DSOInterface(DSO* pdso=NULL);
	virtual ~DSOInterface();
	
	bool MapFunc(DSO* pdso);

	virtual int64 instanceCount(void);

	const char* getImageName();
	HINSTANCE getLib() const { return (pDso!=NULL) ? pDso->library: NULL;};
	virtual bool isValid();

protected:
	DSO* pDso;
	bool bMapped;
	virtual bool mapExternFunc(bool loading) { return true; }
	int64 cInstance;
};

#define DECLARE_PROC_SOI(myclass, superclass) \
   public: \
   myclass(DSO* pdso=NULL) : superclass(pdso) { if (pdso !=NULL) MapFunc(pdso); } \
       virtual ~myclass() {}

#define IMPLEMENT_DSOI_PROC(name) \
    IMPLEMENT_DSOI_PROC_SPECIAL(name, #name)

#define IMPLEMENT_DSOI_PROC_SPECIAL(name, externalname) \
    name = loading ? (name##_PROC) GetProcAddress(getLib(), externalname) : NULL; \
	if(loading && name == NULL) \
		succ = false;

ENTRYDB_NAMESPACE_END

#endif // __DSO_h__
