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
// Ident : $Id: ConsoleGrammar.y $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: ConsoleGrammar.y $
// ===========================================================================

#include "admin.h"

#ifdef _MSC_VER
#  pragma warning (disable:4065)
#endif // _MSC_VER

void yyerror(const char* s)
{
    gAdmin.error(s);
}

%}

%pure_parser

%token TOK_HELP
%token TOK_EXIT
%token TOK_CONNECT
%token TOK_SUBSCRIBE

%token TOK_UPDATE
%token TOK_LIST
%token TOK_SHOW
%token TOK_REMOVE
%token TOK_MOUNT
%token TOK_UNMOUNT
%token TOK_DUMP
%token TOK_TRACE

%token TOK_ON
%token TOK_OFF

%token TOK_SPOOL

%token TOK_SET
%token TOK_EQ

%token TOK_SITE
%token TOK_APP
%token TOK_TXN

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
			gAdmin.usage();
		}
	| TOK_CONNECT strings ';'
		{
			gAdmin.connect($2);
		}
	| TOK_SUBSCRIBE strings ';'
		{
			gAdmin.subscribe($2);
		}
	| TOK_LIST TOK_SITE ';'
		{
			gAdmin.listSites();
		}
	| TOK_LIST TOK_APP ';'
		{
			gAdmin.listApps();
		}
	| TOK_UPDATE TOK_SITE strings ';'
		{
			gAdmin.updateSite($3);
		}
	| TOK_UPDATE TOK_APP TOK_STRING ';'
		{
			gAdmin.updateApp($3);
		}
	| TOK_SHOW TOK_SITE TOK_STRING ';'
		{
			gAdmin.showSite($3);
		}
	| TOK_REMOVE TOK_SITE TOK_STRING ';'
		{
			gAdmin.removeSite($3);
		}
	| TOK_REMOVE TOK_APP TOK_STRING ';'
		{
			gAdmin.removeApp($3);
		}
	| TOK_MOUNT strings ';'
		{
			gAdmin.mount($2);
		}
	| TOK_UNMOUNT strings ';'
		{
			gAdmin.unmount($2);
		}
	| TOK_SET TOK_SITE TOK_STRING ';'
		{
			gAdmin.setSiteProp($3);
		}
	| TOK_SPOOL TOK_ON ';'
		{
			gAdmin.enableSpool();
		}
	| TOK_SPOOL TOK_OFF ';'
		{
			gAdmin.enableSpool(false);
		}
	| TOK_LIST TOK_TXN strings ';'
		{
			gAdmin.listTxn($3);
		}
	| TOK_TRACE strings ';'
		{
			gAdmin.trace($2);
		}
	| TOK_DUMP "xml" TOK_STRING ';'
		{
			gAdmin.dumpTxnXml($3);
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
