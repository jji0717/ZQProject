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
// Ident : $Id: AdminConsole.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Weiwoo/admin/AdminConsole.h $
// 
// 5     06-12-25 12:22 Hongquan.zhang
// ===========================================================================

#ifndef __AdminConsole_H__
#define __AdminConsole_H__


#  include "ZQ_common_conf.h"
#  include "Locks.h"


extern "C"
{
#include <time.h>
#include <stdio.h>
}

#ifdef _WIN32
#   include <io.h>
#   define isatty _isatty
#   define fileno _fileno
// '_isatty' : inconsistent dll linkage.  dllexport assumed.
#   ifdef _MSC_VER
#       pragma warning( disable : 4273 )
#   endif
#endif

#include <vector>
#include <queue>
#include <string>

typedef std::vector<std::string> Args;

class AdminConsole
{
public:
    AdminConsole();

public:

#ifdef ADMIN
    void getInput(char*, size_t&, size_t, bool withPrompt=true);
#else
    void getInput(char*, int&, size_t, bool withPrompt=true);
#endif
    void nextLine();
    void continueLine();
    const char* getPrompt();

    void error(const char* fmt, ...)  PRINTFLIKE(2, 3);
    void error(const std::string&);

    void warning(const char* fmt, ...)  PRINTFLIKE(2, 3);
    void warning(const std::string&);

    int parse(FILE*, bool);
    int parse(const std::string&, bool);

	void enableSpool(bool enable=true) { _bSpool = enable;}

protected:

	int readInput(char* buf, int maxSize, const char* prompt =NULL, bool allowEmpty=true);
	int spoolLine(char* line);

    std::string _commands;
    bool _continue;
    int _errors;
    int _currentLine;
    std::string _currentFile;

	typedef std::queue < std::string >  SpooledOutput;
	SpooledOutput						_spooledOutput;
	ZQ::common::Mutex					_lockSpooledOutput;
	bool								_bSpool;
};

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

#endif // __AdminConsole_H__
