/* A Bison parser, made by GNU Bison 2.1.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     HELP = 258,
     EXIT = 259,
     CLEAR = 260,
     CONNECT = 261,
     DISCON = 262,
     LIST = 263,
     CREATE = 264,
     PUSH = 265,
     PLAY = 266,
     PAUSE = 267,
     RESUME = 268,
     SEEK = 269,
     SEEKITEM = 270,
     SPEED = 271,
     RATE = 272,
     ERASE = 273,
     DESTROY = 274,
     SELECT = 275,
     ALIAS = 276,
     CURR = 277,
     INFO = 278,
     MACADDR = 279,
     VERBOSE = 280,
     STREAMER = 281,
     ITEM = 282,
     ALL = 283,
     INTEGER = 284,
     TF = 285,
     NL = 286,
     POS = 287,
     FRAC = 288,
     STR = 289,
     ADDR = 290
   };
#endif
/* Tokens.  */
#define HELP 258
#define EXIT 259
#define CLEAR 260
#define CONNECT 261
#define DISCON 262
#define LIST 263
#define CREATE 264
#define PUSH 265
#define PLAY 266
#define PAUSE 267
#define RESUME 268
#define SEEK 269
#define SEEKITEM 270
#define SPEED 271
#define RATE 272
#define ERASE 273
#define DESTROY 274
#define SELECT 275
#define ALIAS 276
#define CURR 277
#define INFO 278
#define MACADDR 279
#define VERBOSE 280
#define STREAMER 281
#define ITEM 282
#define ALL 283
#define INTEGER 284
#define TF 285
#define NL 286
#define POS 287
#define FRAC 288
#define STR 289
#define ADDR 290




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 23 "e:\\ZQProjs\\TianShan\\StreamSmith\\StreamSmithAdmin\\SSGrammer.y"
typedef union YYSTYPE {
	int val;
	float frac;
	char* str;
} YYSTYPE;
/* Line 1447 of yacc.c.  */
#line 114 "SSGrammer.hpp"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



