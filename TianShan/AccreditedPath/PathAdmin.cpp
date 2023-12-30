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
// Ident : $Id: PathAdminConsole.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/PathAdmin.cpp $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 2     06-09-05 12:53 Hui.shao
// 
// 1     06-06-30 14:21 Hui.shao
// the console to admin paths
// ===========================================================================

#include "PathAdmin.h"

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

#ifdef HAVE_READLINE
#   include <readline/readline.h>
#   include <readline/history.h>
#endif

#ifdef YYDEBUG
extern int yydebug;
#endif

using namespace std;

extern FILE* yyin;

// PathAdminConsole* gPathAdmin = NULL;

PathAdminConsole::PathAdminConsole()
{
	int i =0;
	_ic = Ice::initialize(i, NULL);
}

void PathAdminConsole::usage()
{
    cout <<
        "help             Print this message.\n"
        "exit, quit       Exit this program.\n"
        "list <Type>	  list the objects of type: servicegroup, storage, streamer.\n"
        "update <Type>	  .\n";
}

#define ASSET_CONNECTION if (!_adminPrx) \
{ error("no PathManager has been currently connected, run 'connect' first"); return; }

void PathAdminConsole::connect(const Args& args)
{
    if(args.empty())
    {
		error("'connect' requires the endpoint to the PathManager");
		return;
    }
	
	// build up the endpoint string
	std::string endpoint = "PathManager:";
	for (Args::const_iterator it = args.begin(); it !=args.end(); it++)
		endpoint += *it + " ";
	
	try {
		_adminPrx = TianShanIce::AccreditedPath::PathAdminPrx::checkedCast(_ic->stringToProxy(endpoint));
	}catch(...) {}
	
	if (!_adminPrx)
		error("failed to connect to %s\n", endpoint);
}

void PathAdminConsole::listServiceGroups()
{
	ASSET_CONNECTION;
	
	::TianShanIce::AccreditedPath::ServiceGroups sgs = _adminPrx->listServiceGroups();
	
	printf("ID    Description\n");
	printf("----  ----------------------\n");
	for (::TianShanIce::AccreditedPath::ServiceGroups::iterator it = sgs.begin(); it < sgs.end(); it++)
		printf("%4d  %s\n", it->id, it->desc.c_str());
	printf("    %d ServiceGroup(s) found\n", sgs.size());
}

void PathAdminConsole::listStorages()
{
	ASSET_CONNECTION;
	
	::TianShanIce::AccreditedPath::Storages collection = _adminPrx->listStorages();
	
	printf("ID    Type           Endpoint                        Description\n");
	printf("----  -------------  ------------------------------  -------------------\n");
	for (::TianShanIce::AccreditedPath::Storages::iterator it = collection.begin(); it < collection.end(); it++)
		printf("%4s  %13s  %30s  %s\n", it->netId.c_str(), it->type.c_str(), it->ifep.c_str(), it->desc.c_str());
	printf("    %d Storage(s) found\n", collection.size());
}

void PathAdminConsole::listStreamers()
{
	ASSET_CONNECTION;
	
	::TianShanIce::AccreditedPath::Streamers collection = _adminPrx->listStreamers();
	
	printf("ID    Type           Endpoint                        Description\n");
	printf("----  -------------  ------------------------------  -------------------\n");
	for (::TianShanIce::AccreditedPath::Streamers::iterator it = collection.begin(); it < collection.end(); it++)
		printf("%4d  %13s  %30s  %s\n", it->id, it->type.c_str(), it->ifep.c_str(), it->desc.c_str());
	printf("    %d Streamer(s) found\n", collection.size());
}

void PathAdminConsole::listStorageLinks()
{
/*
ASSET_CONNECTION;

  ::TianShanIce::AccreditedPath::ServiceGroups sgs = _adminPrx->listServiceGroups();
  
	printf("GroupID    Description\n");
	for (::TianShanIce::AccreditedPath::ServiceGroups::iterator it = sgs.begin(); it < sgs.end(); it++)
	{
	printf("%8d   %s\n", it->id, it->desc.c_str());
	}
	printf("    %d ServiceGroup(s) found\n", sgs.size());
	*/
}

void PathAdminConsole::listStreamLinks()
{
/*
ASSET_CONNECTION;

  ::TianShanIce::AccreditedPath::ServiceGroups sgs = _adminPrx->listServiceGroups();
  
	printf("GroupID    Description\n");
	for (::TianShanIce::AccreditedPath::ServiceGroups::iterator it = sgs.begin(); it < sgs.end(); it++)
	{
	printf("%8d   %s\n", it->id, it->desc.c_str());
	}
	printf("    %d ServiceGroup(s) found\n", sgs.size());
	*/
}


void PathAdminConsole::updateServiceGroup(const Args& args)
{
	ASSET_CONNECTION;
	
	std::string endpoint;
	for (Args::const_iterator it = args.begin(); it !=args.end(); it++)
		endpoint += *it + " ";
	
	_adminPrx->updateServiceGroup(atoi(args[0].c_str()), args[1]);
}

void PathAdminConsole::getInput(char* buf, int& result, int maxSize)
{
    if(!_commands.empty())
    {
		if(_commands == ";")
		{
			result = 0;
		}
		else
		{
#if defined(_MSC_VER) && !defined(_STLP_MSVC)
			// COMPILERBUG: Stupid Visual C++ defines min and max as macros
			result = _MIN(maxSize, static_cast<int>(_commands.length()));
#else
			result = min(maxSize, static_cast<int>(_commands.length()));
#endif
			strncpy(buf, _commands.c_str(), result);
			_commands.erase(0, result);
			if(_commands.empty())
			{
				_commands = ";";
			}
		}
    }
    else if(isatty(fileno(yyin)))
    {
#ifdef HAVE_READLINE
		
        const char* prompt = gPathAdmin.getPrompt();
		char* line = readline(const_cast<char*>(prompt));
		if(!line)
		{
			result = 0;
		}
		else
		{
			if(*line)
			{
				add_history(line);
			}
			
			result = strlen(line) + 1;
			if(result > maxSize)
			{
				free(line);
				error("input line too long");
				result = 0;
			}
			else
			{
				strcpy(buf, line);
				strcat(buf, "\n");
				free(line);
			}
		}
		
#else
		
		cout << gPathAdmin.getPrompt() << flush;
		
		string line;
		while(true)
		{
			char c = static_cast<char>(getc(yyin));
			if(c == EOF)
			{
				if(line.size())
				{
					line += '\n';
				}
				break;
			}
			
			line += c;
			
			if(c == '\n')
			{
				break;
			}
		}
		
		result = static_cast<int>(line.length());
		if(result > maxSize)
		{
			error("input line too long");
			buf[0] = EOF;
			result = 1;
		}
		else
		{
			strcpy(buf, line.c_str());
		}
		
#endif
    }
    else
    {
		if(((result = static_cast<int>(fread(buf, 1, maxSize, yyin))) == 0) && ferror(yyin))
		{
			error("input in flex scanner failed");
			buf[0] = EOF;
			result = 1;
		}
    }
}

void PathAdminConsole::nextLine()
{
    _currentLine++;
}

void PathAdminConsole::continueLine()
{
    _continue = true;
}

const char*
PathAdminConsole::getPrompt()
{
    assert(_commands.empty() && isatty(fileno(yyin)));
	
    if(_continue)
    {
		_continue = false;
		return "(cont) ";
    }
    else
    {
		return "PathAdmin> ";
    }
}

void PathAdminConsole::error(const char* s)
{
    if(_commands.empty() && !isatty(fileno(yyin)))
    {
		cerr << _currentFile << ':' << _currentLine << ": " << s << endl;
    }
    else
    {
		cerr << "error: " << s << endl;
    }
    _errors++;
}

void
PathAdminConsole::error(const string& s)
{
    error(s.c_str());
}

void
PathAdminConsole::warning(const char* s)
{
    if(_commands.empty() && !isatty(fileno(yyin)))
    {
		cerr << _currentFile << ':' << _currentLine << ": warning: " << s << endl;
    }
    else
    {
		cerr << "warning: " << s << endl;
    }
}

void PathAdminConsole::warning(const string& s)
{
    warning(s.c_str());
}

int PathAdminConsole::parse(FILE* file, bool debug)
{
    _errors = 0;
    _commands.empty();
    yyin = file;
    assert(yyin);
	
    _currentFile = "";
    _currentLine = 0;
    _continue = false;
    nextLine();
	
	//    _foundContacts.clear();
	//    _current = _foundContacts.end();
	
    int status = yyparse();
    if(_errors)
    {
		status = EXIT_FAILURE;
    }
	
    return status;
}

int PathAdminConsole::parse(const string& commands, bool debug)
{
    _errors = 0;
    _commands = commands;
    assert(!_commands.empty());
    yyin = 0;
	
    _currentFile = "<command line>";
    _currentLine = 0;
    _continue = false;
    nextLine();
	
	//    _foundContacts.clear();
	//    _current = _foundContacts.end();
	
    int status = yyparse();
    if(_errors)
    {
		status = EXIT_FAILURE;
    }
	
    return status;
}

