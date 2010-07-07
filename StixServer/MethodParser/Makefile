# Makefile for Method Parser Test Program
# By: Michael Lindemuth

PARSER: MethodParser.tab.c lex.yy.c
	gcc -I../ MethodParser.tab.c lex.yy.c -lfl -o parser

lex.yy.c: MethodParser.tab.c MethodLexAna.y
	flex -i MethodLexAna.y

MethodParser.tab.c: MethodParser.y
	bison -d MethodParser.y
