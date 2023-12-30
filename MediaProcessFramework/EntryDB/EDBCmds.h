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
// Desc  : entry database command line util
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/EDBCmds.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/EDBCmds.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 4     4/14/05 10:13a Hui.shao
// 
// 3     4/12/05 5:50p Hui.shao
// ============================================================================================

#ifndef __EDBCmds_h__
#define __EDBCmds_h__

#include "EDB.h"
#include "Addins.h"

#include <string>
#include <vector>

using namespace ENTRYDB_NAMESPACE;

class EDBCmds
{
public:
	EDBCmds();
	~EDBCmds();

	void performs();

protected:
	void help();
	void quit();

	void load();
	void connect();
	void disconnect();
	
	void list();
	void chentry();
	void mkentry();
	void set();
	void delentry();
	void edms();
	void export();

	void xexport();

private:

	bool readln(bool prompt=true, const char* line=NULL);
	bool shift(int num=1);

	bool bContinue;
	typedef std::vector< std::string > args_t;
	args_t args;
	std::string argsln;
	static const char* cmds[];

	EDB mEdb;
	EDBFactory *factory;
};

#endif // __EDBCmds_h__
