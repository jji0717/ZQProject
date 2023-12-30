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
// Desc  : entry db mgr implementation with berkeley database 2
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EDBb2Mgr.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EDBb2Mgr.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 3     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 2     4/12/05 5:59p Hui.shao
// ============================================================================================

#include "EDBb2Mgr.h"

ENTRYDB_NAMESPACE_BEGIN

#define DEF_PAGE_SZ 4096

EDBb2Mgr::EDBb2Mgr()
{
	mDBmap.clear();
}

EDBb2Mgr::~EDBb2Mgr()
{
	mLock.enter();
	for (DBmap_t::iterator i = mDBmap.begin(); i!=mDBmap.end(); i++)
	{
		EBDB2mgr_CB *pcb= i->second;
		i->second = NULL;
		if (pcb!=NULL)
			delete pcb;
	}
	mDBmap.clear();
	mLock.leave();
}

bool EDBb2Mgr::EBDB2mgr_CB::open(const char* dbpath, const char*errf)
{
	if (dbpath == NULL || *dbpath==0x00)
		return false;

	try
	{
		DbInfo dbi;
		dbi.set_flags(DB_RECNUM);
		dbi.set_pagesize(DEF_PAGE_SZ);
	
		dbp = NULL;
		Db::open(dbpath, DB_BTREE, DB_CREATE, 0664, NULL, &dbi, &(dbp));

		if (dbp==NULL)
			return false;

		return true;
	}
	catch (...)
	{
		return false;
	}
}

EDBb2Mgr::EBDB2mgr_CB::~EBDB2mgr_CB()
{
	if (isopened())
		dbp->close(0);
	dbp = NULL;
}

bool EDBb2Mgr::connect(EDBb2::EDBb2_CB_t &edbcb)
{
	edbcb.errf = NULL;
	edbcb.dbp = NULL;
	edbcb.cur = NULL;

	if (edbcb.dbpath.empty())
		return false;

	mLock.enter();
	EBDB2mgr_CB *pcb =mDBmap[edbcb.dbpath];

	if (pcb==NULL)
		pcb = new EBDB2mgr_CB;

	if (!pcb->isopened() && !pcb->open(edbcb.dbpath.c_str()))
	{
		mLock.leave();
		return false;
	}

	edbcb.dbp = pcb->dbp;
	edbcb.errf = pcb->errf;
	pcb->dbp->cursor(NULL, &edbcb.cur, 0);
	pcb->nConn++;
	mDBmap[edbcb.dbpath] = pcb;
	mLock.leave();

	return true;
}

bool EDBb2Mgr::disconnect(EDBb2::EDBb2_CB_t &edbcb)
{
	if (edbcb.dbpath.empty())
		return false;

	EBDB2mgr_CB *pcb =mDBmap[edbcb.dbpath];

	if (edbcb.cur != NULL)
		edbcb.cur->close();

	if (pcb!=NULL && pcb->isopened() && --pcb->nConn <=0)
	{
		mLock.enter();
		mDBmap[edbcb.dbpath] = NULL;
		delete pcb;
		mDBmap.erase(edbcb.dbpath);
		mLock.leave();
	}

	edbcb.errf = NULL;
	edbcb.dbp = NULL;
	edbcb.cur = NULL;
	return true;
}

EDBb2Mgr edb2mgr;

ENTRYDB_NAMESPACE_END
