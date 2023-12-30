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
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EDBb2Mgr.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EDBb2Mgr.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 5     4/13/05 7:00p Hui.shao
// 
// 4     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 3     4/12/05 5:59p Hui.shao
// ============================================================================================

#ifndef __EDBb2Mgr_h__
#define __EDBb2Mgr_h__

#include "EDBb2.h"
#include "../../Common/Locks.h"

ENTRYDB_NAMESPACE_BEGIN

class EDBb2Mgr
{
public:
	EDBb2Mgr();
	~EDBb2Mgr();
	bool connect(EDBb2::EDBb2_CB_t &edbcb);
	bool disconnect(EDBb2::EDBb2_CB_t &edbcb);

private:
	class EBDB2mgr_CB
	{
	public:
		EBDB2mgr_CB(): errf(NULL), dbp(NULL), nConn(0){}
		~EBDB2mgr_CB();
		bool open(const char* dbpath, const char*errf=NULL);
		bool isopened() {return (dbp!=NULL); }

		FILE* errf;
		Db*  dbp;
		int nConn;
	};

	typedef std::map<std::string, EBDB2mgr_CB*> DBmap_t;
	DBmap_t mDBmap;
	Mutex mLock;
};

extern EDBb2Mgr edb2mgr;

ENTRYDB_NAMESPACE_END

#endif //  __EDBb2Mgr_h__
