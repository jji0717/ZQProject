// ===========================================================================
// Copyright (c) 2004 by
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
// $Log: /ZQProjs/TianShan/AccreditedPath/PathAdmin.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// ===========================================================================

#ifndef __PathAdmin_H__
#define __PathAdmin_H__

#include "ZQ_common_Conf.h"
#include "TsPathAdmin.h"

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

class PathAdminConsole // : public ::IceUtil::SimpleShared
{
public:

    void usage();
    void connect(const Args& args);

    // about serive groups
	void listServiceGroups();
    void updateServiceGroup(const Args& args);

	/// about storages
	void listStorages();

	/// about streamers
	void listStreamers();

	/// about StorageLinks
	void listStorageLinks();

	/// about treamLinks
	void listStreamLinks();

	/*
    void addContacts(const std::list<std::string>&);
    void findContacts(const std::list<std::string>&);
    void nextFoundContact();
    void printCurrent();
    void setCurrentName(const std::list<std::string>&);
    void setCurrentAddress(const std::list<std::string>&);
    void setCurrentPhone(const std::list<std::string>&);
    void removeCurrent();
    void setEvictorSize(const std::list<std::string>&);
    void shutdown();
*/

    void getInput(char*, int&, int);
    void nextLine();
    void continueLine();
    const char* getPrompt();

    void error(const char*);
    void error(const std::string&);

    void warning(const char*);
    void warning(const std::string&);

    int parse(FILE*, bool);
    int parse(const std::string&, bool);

    PathAdminConsole();

private:

	Ice::CommunicatorPtr _ic;
	TianShanIce::AccreditedPath::PathAdminPrx _adminPrx;

    std::string _commands;
    bool _continue;
    int _errors;
    int _currentLine;
    std::string _currentFile;
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
