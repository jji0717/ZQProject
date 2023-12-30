%{
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
// Ident : $Id: ConsoleScanner.y $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: ConsoleScanner.y $
// ===========================================================================

#include "admin.h"
#include "ConsoleGrammar.h"

#if defined(_MSC_VER) && defined(ICE_64)
//
// 'initializing' : conversion from '__int64' to 'int', possible loss of data
// Puts a pointer-difference into an int
//
#   pragma warning( 4 : 4244 )
#endif

using namespace std;

#define YY_INPUT(buf, result, maxSize) gAdmin.getInput(buf, result, maxSize)

%}

WS	[ \t\v\f\r]
DIGIT [0-9]
NL	[\n]

%option noyywrap

%%

"//" {
    // C++-style comment
    int c;
    do
    {
		c = yyinput();
		if(c == '\n')
			gAdmin.nextLine();
    }
    while(c != '\n' && c != EOF);
}

"/*" {
    // C-style comment
    while(true)
    {
		int c = yyinput();
		if (c == EOF)
		{
			gAdmin.warning("EOF in comment");
			break;
		}

		if (c == '\n')
			gAdmin.nextLine();
		else if (c == '*')
		{
			int next = yyinput();
			if (next == '/')
				break;
			else
				unput(next);
		}
    }
}

"help" {
    return TOK_HELP;
}

"quit"|"exit" {
    return TOK_EXIT;
}

"connect" {
    return TOK_CONNECT;
}

"subscribe"|"sub" {
    return TOK_SUBSCRIBE;
}

"update"|"add" {
    return TOK_UPDATE;
}

"list" {
    return TOK_LIST;
}

"remove"|"delete"|"erase" {
    return TOK_REMOVE;
}

"show" {
    return TOK_SHOW;
}

"set" {
    return TOK_SET;
}

'=' {
    return TOK_EQ;
}

"dump" {
    return TOK_DUMP;
}

"mount" {
    return TOK_MOUNT;
}

"unmount" {
    return TOK_UNMOUNT;
}

"site" {
    return TOK_SITE;
}

"application"|"app"|"appl" {
    return TOK_APP;
}

"On"|"ON"|"on" {
	return TOK_ON;
}

"Off"|"OFF"|"off" {
	return TOK_OFF;
}

"spool" {
	return TOK_SPOOL;
}

{WS}*(\\{WS}*{NL})? {
    size_t len = strlen(yytext);
    for(size_t i = 0; i < len; ++i)
    {
		if(yytext[i] == '\\')
			gAdmin.continueLine();
		else if(yytext[i] == '\n')
			gAdmin.nextLine();
    }
}

{NL}|; {
    size_t len = strlen(yytext);
    for(size_t i = 0; i < len; ++i)
    {
		if(yytext[i] == '\n')
			gAdmin.nextLine();
    }
    return ';';
}

\" {
    // "..."-type strings
    string s;
    while(true)
    {
		char c = static_cast<char>(yyinput());
		if(c == '"')
			break;

		if(c == EOF)
		{
			gAdmin.warning("EOF in string");
			break;
		}

		if(c == '\n')
		{
			s += c;
			gAdmin.nextLine();
		}
		else if(c == '\\')
		{
			char next = static_cast<char>(yyinput());
			switch(next)
			{
			case '\\':
			case '"':
				s += next;
				break;
	    
			case 'n':
				s += '\n';
				break;
	    
			case 'r':
				s += '\r';
				break;

			case 't':
				s += '\t';
				break;
	    
			case 'v':
				s += '\v';
				break;
	    
			case 'f':
				s += '\f';
				break;
	    
			default:
				s += c;
				unput(next);
			}
		}
		else
			s += c;
    }

    yylvalp->clear();
    yylvalp->push_back(s);
    return TOK_STRING;
}

\' {
    // '...'-type strings
    string s;
    while(true)
    {
		char c = static_cast<char>(yyinput());
		if(c == '\'')
			break;

		if(c == EOF)
		{
			gAdmin.warning("EOF in string");
			break;
		}
	
		if(c == '\n')
		{
			s += c;
			gAdmin.nextLine();
		}
		else
			s += c;
    }

    yylvalp->clear();
    yylvalp->push_back(s);
    return TOK_STRING;
}

. {
    // Simple strings
    string s;
    s += yytext[0];
    while(true)
    {
		char c = static_cast<char>(yyinput());
		if(c == EOF)
			break;
		
		if(isspace(c) || c == ';')
		{
			unput(c);
			break;
		}
	
		s += c;
    }

    yylvalp->clear();
    yylvalp->push_back(s);
    return TOK_STRING;
}

%%
