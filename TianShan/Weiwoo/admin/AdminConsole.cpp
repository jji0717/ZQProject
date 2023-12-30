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
// Ident : $Id: AdminConsole.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/admin/AdminConsole.cpp $
// 
// 5     3/03/16 4:04p Dejian.fei
// 
// 4     1/28/16 2:22p Dejian.fei
// 
// 3     1/20/16 5:24p Dejian.fei
// 
// 2     1/13/16 9:34a Dejian.fei
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 5     06-07-24 18:34 Hui.shao
// 
// 4     06-07-21 12:49 Hui.shao
// added command subscribe
// 
// 3     06-07-10 15:28 Hui.shao
// 
// 2     06-07-10 11:33 Hui.shao
// 
// 1     06-07-07 14:25 Hui.shao
// 
// 3     06-07-06 20:05 Hui.shao
// 
// 2     06-07-05 19:53 Hui.shao
// ===========================================================================

#include "AdminConsole.h"

extern "C"
{
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
}

#ifdef HAVE_READLINE
#   include <readline/readline.h>
#   include <readline/history.h>
#endif

#ifdef YYDEBUG
extern int yydebug;
#endif

using namespace std;

extern FILE* yyin;

AdminConsole::AdminConsole()
:_bSpool(false)
{
}

//void AdminConsole::getInput(char* buf, int& result, int maxSize, bool withPrompt)
#ifdef ADMIN
void AdminConsole::getInput(char* buf, size_t& result, size_t maxSize, bool withPrompt)
#else
void AdminConsole::getInput(char* buf, int& result, size_t maxSize, bool withPrompt)
#endif
{
	{
		ZQ::common::MutexGuard gd(_lockSpooledOutput);
		while (!_spooledOutput.empty())
		{
			printf("(spool)> %s\n", _spooledOutput.front().c_str());
			_spooledOutput.pop();
		}
	}
	
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
			result = min(maxSize, static_cast<int>(_commands.length()));
#else
			//result = min(maxSize, static_cast<int>(_commands.length()));
			result = min(maxSize, static_cast<size_t>(_commands.length()));
#endif
			strncpy(buf, _commands.c_str(), result);
			_commands.erase(0, result);
			if(_commands.empty())
				_commands = ";";
		}
    }
    else if(isatty(fileno(yyin)))
    {
		if (withPrompt)
			printf("%s", getPrompt());
		
		string line;
		while(true)
		{
			char c = static_cast<char>(getc(yyin));
			if(c == EOF)
			{
				if(line.size() >0)
					line += '\n';
				break;
			}
			
			line += c;
			
			if(c == '\n')
				break;
		}
		
		result = static_cast<int>(line.length());
		if(result > maxSize)
		{
			error("input line too long");
			buf[0] = EOF;
			result = 1;
		}
		else
			strcpy(buf, line.c_str());
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

void AdminConsole::nextLine()
{
    _currentLine++;
}

void AdminConsole::continueLine()
{
    _continue = true;
}

const char* AdminConsole::getPrompt()
{
    if (!_commands.empty() || !isatty(fileno(yyin)))
		return NULL;
	
    if(_continue)
    {
		_continue = false;
		return "(cont)> ";
    }
    else return "     > ";
}

void AdminConsole::error(const char* fmt, ...)
{
	char msg[2048];
	va_list args;

	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);

    if(_commands.empty() && !isatty(fileno(yyin)))
		fprintf(stderr, "%s(%d): error: %s\n", _currentFile.c_str(), _currentLine, msg);
    else
		fprintf(stderr, "error: %s\n", msg);

    _errors++;
}

void AdminConsole::error(const string& s)
{
    error(s.c_str());
}

void AdminConsole::warning(const char* fmt, ...)
{
	char msg[2048];
	va_list args;

	va_start(args, fmt);
	vsprintf(msg, fmt, args);
	va_end(args);

    if(_commands.empty() && !isatty(fileno(yyin)))
		fprintf(stderr, "%s(%d): warning: %s\n", _currentFile.c_str(), _currentLine, msg);
    else
		fprintf(stderr, "error: %s\n", msg);
}

void AdminConsole::warning(const string& s)
{
    warning(s.c_str());
}

int AdminConsole::parse(FILE* file, bool debug)
{
    _errors = 0;
    _commands.empty();
    yyin = file;
    if(!yyin)
		return 0;
	
    _currentFile = "";
    _currentLine = 0;
    _continue = false;
    nextLine();
	
	//    _foundContacts.clear();
	//    _current = _foundContacts.end();
	
    int status = yyparse();
    if(_errors)
		status = EXIT_FAILURE;
	
    return status;
}

int AdminConsole::parse(const string& commands, bool debug)
{
    _errors = 0;
    _commands = commands;
    if(_commands.empty())
		return 0;
	
    yyin = 0;
	
    _currentFile = "<command line>";
    _currentLine = 0;
    _continue = false;
    nextLine();
	
	//    _foundContacts.clear();
	//    _current = _foundContacts.end();
	
    int status = yyparse();
    if(_errors)
		status = EXIT_FAILURE;
	
    return status;
}

int AdminConsole::readInput(char* buf, int maxSize, const char* prompt, bool allowEmpty)
{
	if (NULL != prompt)
		printf(prompt);
#ifdef ADMIN
	size_t c;
#else
	int c;
#endif
	char* pos;
	do
	{
		getInput(buf, c, (size_t)maxSize, false);

		while ((pos = strrchr(buf, '\n')) !=NULL || (pos = strrchr(buf, '\r')) !=NULL)
			*pos = '\0';
		c = strlen(buf);

	} while (!allowEmpty && c <=0); // endpoint must not be empty

	return c;
}

int AdminConsole::spoolLine(char* line)
{
	if (NULL == line || !_bSpool || _spooledOutput.size() >100)
		return 0;

	ZQ::common::MutexGuard gd(_lockSpooledOutput);
	_spooledOutput.push(line);

	return strlen(line);
}
