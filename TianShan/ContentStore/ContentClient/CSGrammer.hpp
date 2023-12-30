
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     HELP = 258,
     EXIT = 259,
     CLOSE = 260,
     CLEAR = 261,
     SLEEP = 262,
     NONE = 263,
     ADDACCESSCOUNT = 264,
     ACCESSCOUNT = 265,
     LISTMISSED = 266,
     SETCACHEWINDOW = 267,
     CACHEWINDOW = 268,
     STOREDISTANCE = 269,
     LOCALFN = 270,
     LISTHOT = 271,
     SYSCOMMAND = 272,
     CONNECT = 273,
     OPEN = 274,
     LIST = 275,
     SET = 276,
     MSET = 277,
     PSET = 278,
     SYNC = 279,
     INFO = 280,
     CURR = 281,
     DESTROY = 282,
     CANCEL = 283,
     PROV = 284,
     EXPOSE = 285,
     UPDATE = 286,
     TIMER = 287,
     TSTART = 288,
     TSTOP = 289,
     UP = 290,
     EXPORT = 291,
     CACHE = 292,
     HASHFOLDER = 293,
     HASHFOLDERNAME = 294,
     VOLUME = 295,
     FOLDER = 296,
     INTEGER = 297,
     TF = 298,
     NL = 299,
     EQ = 300,
     URL = 301,
     ENDPOINT = 302,
     NAME = 303,
     EXPROTO = 304,
     SUBFILE = 305
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{


	int val;
	char* str;



} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


