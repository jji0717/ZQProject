// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: PathAdmin.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/admin/PathAdmin.h $
// 
// 2     4/18/11 5:26p Fei.huang
// + migrated to linux
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 8     07-09-30 11:07 Hui.shao
// declared commands "dump" and "import" for TianYiXin to implement
// 
// 7     07-04-20 15:14 Hongquan.zhang
// 
// 6     10/16/06 3:29p Hui.shao
// 
// 5     9/21/06 6:09p Hui.shao
// 
// 3     06-09-19 19:01 Hui.shao
// ===========================================================================

#ifndef __PathAdmin_H__
#define __PathAdmin_H__

#include "TianShanDefines.h"
#include "TsPathAdmin.h"
#include "AdminConsole.h"

#include <vector>

#ifdef _WIN32
#   include <io.h>
#   define isatty _isatty
#   define fileno _fileno
// '_isatty' : inconsistent dll linkage.  dllexport assumed.
#   ifdef _MSC_VER
#       pragma warning( disable : 4273 )
#   endif
#endif

typedef std::vector<std::string> Args;

class PathAdminConsole  : public AdminConsole
{
public:

	PathAdminConsole();
	virtual ~PathAdminConsole(){}

    void usage();
    void connect(const Args& args);

    // about serive groups
	void listServiceGroups();
    void updateServiceGroup(const Args& args);
	void removeServiceGroup(const Args& args);

	/// about storages
	void listStorages();
	void updateStorage(const Args& args);
	void removeStorage(const Args& args);

	/// about streamers
	void listStreamers();
	void showStreamer(const Args& args);
	void updateStreamer(const Args& args);
	void removeStreamer(const Args& args);

	void listTickets();

	void linkStorage(void);
	void relinkStorage(const Args& args);

	void linkStreamer(void);
	void relinkStreamer(const Args& args);

	void dump(const Args& args);
	void import(const Args& args);

	virtual const char* getPrompt();
///////////////////////////////////////////////

	/// about StorageLinks
	void listStorageLinks();

	/// about streamLinks
	void listStreamLinks();

private:

	Ice::CommunicatorPtr _ic;
	TianShanIce::Transport::PathAdminPrx _adminPrx;
};

extern PathAdminConsole gPathAdmin; // The current parser for bison/flex

// Stuff for flex and bison
#define YYSTYPE Args
#define YY_DECL int yylex(YYSTYPE* yylvalp)

int yylex(YYSTYPE* yylvalp);
int yyparse();

// I must set the initial stack depth to the maximum stack depth to
// disable bison stack resizing. The bison stack resizing routines use
// simple malloc/alloc/memcpy calls, which do not work for the
// YYSTYPE, since YYSTYPE is a C++ type, with constructor, destructor,
// assignment operator, etc.
#define YYMAXDEPTH  20000 // 20000 should suffice. Bison default is 10000 as maximum.
#define YYINITDEPTH YYMAXDEPTH // Initial depth is set to max depth, for the reasons described above.

// Newer bison versions allow to disable stack resizing by defining yyoverflow.
#define yyoverflow(a, b, c, d, e, f) yyerror(a)

#endif // __PathAdmin_H__
