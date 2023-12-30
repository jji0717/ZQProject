%{
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
// Ident : $Id: ConsoleGrammar.y $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: ConsoleGrammar.y $
// ===========================================================================

#include "PathAdmin.h"

void yyerror(const char* s)
{
    gPathAdmin.error(s);
}

%}

%pure_parser

%token TOK_HELP
%token TOK_EXIT
%token TOK_CONNECT
%token TOK_UPDATE
%token TOK_LIST
%token TOK_LINK

%token TOK_SVCGRP
%token TOK_STORAGE
%token TOK_STREAMER

%token TOK_INTEGRER
%token TOK_FLOAT
%token TOK_STRING

%%

// ----------------------------------------------------------------------
input: //empty
	| input command
;

// ----------------------------------------------------------------------
command: TOK_EXIT ';'
		{
			exit(0);
		}
	| TOK_HELP ';'
		{
			gPathAdmin.usage();
		}
	| TOK_CONNECT strings ';'
		{
			gPathAdmin.connect($2);
		}
	| TOK_LIST TOK_SVCGRP ';'
		{
			gPathAdmin.listServiceGroups();
		}
	| TOK_LIST TOK_STORAGE ';'
		{
			gPathAdmin.listStorages();
		}
	| TOK_LIST TOK_STREAMER ';'
		{
			gPathAdmin.listStreamers();
		}
	| TOK_UPDATE TOK_SVCGRP strings ';'
		{
			gPathAdmin.updateServiceGroup($3);
		}
	| error ';'
		{
			yyerrok;
		}
	| ';'
		{
		}
;

// ----------------------------------------------------------------------
strings:
	strings TOK_STRING
	{
		$$ = $1;
		$$.push_back($2.front());
	}
	| TOK_STRING
	{
		$$ = $1;
	}
	;

%%
