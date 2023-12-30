/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
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

#ifndef YY_YY_ERMGRAMMER_HPP_INCLUDED
# define YY_YY_ERMGRAMMER_HPP_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

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
     NONE = 262,
     CONNECT = 263,
     OPEN = 264,
     LIST = 265,
     SET = 266,
     MSET = 267,
     PSET = 268,
     INFO = 269,
     CURR = 270,
     DESTROY = 271,
     CANCEL = 272,
     PROV = 273,
     EXPOSE = 274,
     ADD = 275,
     ADDCHANNEL = 276,
     REMOVE = 277,
     UPDATE = 278,
     POPULATE = 279,
     CREATE = 280,
     ENABLE = 281,
     IMPORT = 282,
     EXPORT = 283,
     LINK = 284,
     UNLINK = 285,
     IMPORTROUTES = 286,
     EXPORTROUTES = 287,
     CHANNEL = 288,
     ALLOCATION = 289,
     INTEGER = 290,
     TF = 291,
     NEWLINE = 292,
     EQ = 293,
     PORT = 294,
     URL = 295,
     ENDPOINT = 296,
     NAME = 297,
     EXPROTO = 298
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2058 of yacc.c  */
#line 24 "ERMGrammer.y"

	int val;
	char* str;


/* Line 2058 of yacc.c  */
#line 106 "ERMGrammer.hpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_ERMGRAMMER_HPP_INCLUDED  */
