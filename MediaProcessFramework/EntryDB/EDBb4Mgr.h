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
// Author: Daniel Wang
// Desc  : entry database Berkeley database 4 ed
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EDBb4Mgr.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EDBb4Mgr.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 10    05-07-28 18:39 Daniel.wang
// 
// 9     4/13/05 7:03p Hui.shao
// ============================================================================================


#ifndef __EDBb4Mgr_h__
#define __EDBb4Mgr_h__

#include "../../Common/Locks.h"
#include "EDBb4.h"

ENTRYDB_NAMESPACE_BEGIN




extern EBDB4Env g_GlobalEnv;

class EDBb4Mgr
{
public:
	EDBb4Mgr();
	~EDBb4Mgr();
	bool connect(EDBb4::EDBb4_CB_t &edbcb);
	bool disconnect(EDBb4::EDBb4_CB_t &edbcb);

private:

	class EBDB4mgr_CB
	{
	public:
		EBDB4mgr_CB();
		~EBDB4mgr_CB();
		bool open(const char* dbpath, const char*errf=NULL);
		bool isopened() {return (dbp!=NULL); }

		FILE*		errf;
		EBDB4Env	env;
		Db*			dbp;
		int			nConn;
	private:
		Db  db;
	};

	typedef std::map<std::string, EBDB4mgr_CB*> DBmap_t;
	DBmap_t mDBmap;
	Mutex mLock;
};

extern EDBb4Mgr edb4mgr;

ENTRYDB_NAMESPACE_END

#endif //  __EDBb4Mgr_h__
