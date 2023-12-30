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
// Author: Daniel Wang
// Desc  : entry database berkeley database 4 edition
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EDBb4Mgr.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EDBb4Mgr.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 10    05-07-28 18:39 Daniel.wang
// 
// 9     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 8     4/12/05 6:44p Hui.shao
// ============================================================================================


#include "EDBb4Mgr.h"
#include "cxx_int.h"

ENTRYDB_NAMESPACE_BEGIN

#define DEF_PAGE_SZ 4096

#define ENV_TEST(Env) EBDB4Env* _edbenv = (EBDB4Env*)Env;_edbenv->test()

EDBb4Mgr::EDBb4Mgr()
{
	mDBmap.clear();
}


EDBb4Mgr::~EDBb4Mgr()
{
	mLock.enter();
	for (DBmap_t::iterator i = mDBmap.begin(); i!=mDBmap.end(); i++)
	{
		EBDB4mgr_CB *pcb= i->second;
		i->second = NULL;
		_sdel(pcb);
	}
	mDBmap.clear();
	mLock.leave();
}

bool EDBb4Mgr::EBDB4mgr_CB::open(const char* dbpath, const char*errf)
{
	if (dbpath == NULL || *dbpath==0x00)
		return false;

	try
	{
		db.set_flags(DB_RECNUM);
		db.set_pagesize(DEF_PAGE_SZ);

		DBTYPE type = DB_BTREE;
		u_int32_t flags = DB_CREATE;
		int mode = 0;

		if (0 == db.open(NULL, dbpath, NULL, type, flags, mode))
			dbp = &db;

		return (dbp!=NULL);

	}
	catch (DbException e)
	{
		return false;
	}
	catch (...)
	{
		return false;
	}
}

EDBb4Mgr::EBDB4mgr_CB::EBDB4mgr_CB(): errf(NULL), db(&g_GlobalEnv.getInst(), 0), nConn(0), dbp(NULL)
{
}

EDBb4Mgr::EBDB4mgr_CB::~EBDB4mgr_CB()
{
	if (isopened())
	{
		dbp->close(0);
		//_sdel(dbp);
	}
	dbp = NULL;
}

bool EDBb4Mgr::connect(EDBb4::EDBb4_CB_t &edbcb)
{
	edbcb.errf = NULL;
	edbcb.dbp = NULL;
	//edbcb.cur = NULL;

	if (edbcb.dbpath.empty())
		return false;

	mLock.enter();
	EBDB4mgr_CB *pcb =mDBmap[edbcb.dbpath];

	if (pcb==NULL)
		pcb = new EBDB4mgr_CB;
	
	if (!pcb->isopened() && !pcb->open(edbcb.dbpath.c_str()))
	{
		mLock.leave();
		return false;
	}

	edbcb.dbp = pcb->dbp;
	edbcb.errf = pcb->errf;
	//pcb->dbp->cursor(NULL, &edbcb.cur, 0);
	//pcb->test();
	pcb->nConn++;
	mDBmap[edbcb.dbpath] = pcb;
	mLock.leave();

	return true;
}

bool EDBb4Mgr::disconnect(EDBb4::EDBb4_CB_t &edbcb)
{
	if (edbcb.dbpath.empty())
		return false;

	EBDB4mgr_CB *pcb =mDBmap[edbcb.dbpath];

	//if (edbcb.cur != NULL)
	//	edbcb.cur->close();

	if (pcb!=NULL && pcb->isopened() && --pcb->nConn <=0)
	{
		mLock.enter();
		mDBmap[edbcb.dbpath] = NULL;
		_sdel(pcb);
		mDBmap.erase(edbcb.dbpath);
		mLock.leave();
	}

	edbcb.errf = NULL;
	edbcb.dbp = NULL;
	//edbcb.cur = NULL;
	return true;
}


EBDB4Env g_GlobalEnv;

EDBb4Mgr edb4mgr;

ENTRYDB_NAMESPACE_END
