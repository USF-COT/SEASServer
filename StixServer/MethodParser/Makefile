# Makefile for Method Parser Test Program
# By: Michael Lindemuth

testMethod: MethodParser.tab.c MethodLexAna.lex.c testMethod.c MethodNodesStack.c MethodNodesTable.c ../LONDispatch.c ../SEASPeripheralCommands.c 
	gcc -lpthread ../LONDispatch.c ../SEASPeripheralCommands.c MethodNodesStack.c MethodNodesTable.c MethodParser.tab.c MethodLexAna.lex.c testMethod.c -o testMethod

PARSER: MethodParser.tab.c MethodLexAna.lex.c
	gcc -I../ -L../ ../LONDispatch.c MethodNodesStack.c MethodNodesTable.c ../SEASPeripheralCommands.c MethodParser.tab.c MethodLexAna.lex.c -lfl -lpthread -o parser

MethodLexAna.lex.c: MethodParser.tab.c MethodLexAna.y
	flex -o MethodLexAna.lex.c -i MethodLexAna.y

MethodParser.tab.c: MethodParser.y
	bison -d MethodParser.y
