/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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
     PUMP = 258,
     LAMP = 259,
     VALVE = 260,
     HEATER = 261,
     ON = 262,
     OFF = 263,
     READ = 264,
     CALC = 265,
     RECORD = 266,
     WRITE = 267,
     VAL = 268,
     SET = 269,
     DWELL = 270,
     SPECM = 271,
     PARAMS = 272,
     SAMP = 273,
     WAVE = 274,
     CORR = 275,
     REF = 276,
     ABSO = 277,
     FULL = 278,
     SPEC = 279,
     CONC = 280,
     PCO2 = 281,
     PH = 282,
     OPEN = 283,
     CLOSE = 284,
     DATA = 285,
     DELAY = 286,
     BEG = 287,
     EN = 288,
     LOOP = 289,
     FIL = 290
   };
#endif
/* Tokens.  */
#define PUMP 258
#define LAMP 259
#define VALVE 260
#define HEATER 261
#define ON 262
#define OFF 263
#define READ 264
#define CALC 265
#define RECORD 266
#define WRITE 267
#define VAL 268
#define SET 269
#define DWELL 270
#define SPECM 271
#define PARAMS 272
#define SAMP 273
#define WAVE 274
#define CORR 275
#define REF 276
#define ABSO 277
#define FULL 278
#define SPEC 279
#define CONC 280
#define PCO2 281
#define PH 282
#define OPEN 283
#define CLOSE 284
#define DATA 285
#define DELAY 286
#define BEG 287
#define EN 288
#define LOOP 289
#define FIL 290




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 20 "MethodParser.y"
{
    double doubleVal;
}
/* Line 1489 of yacc.c.  */
#line 123 "MethodParser.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

