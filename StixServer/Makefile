# Makefile for Method Parser Test Program
# By: Michael Lindemuth

# Build the Server
SEASServer: MethodParser/MethodLexAna.lex.c MethodParser/MethodParser.tab.c
	gcc -IMethodParser/ -lusb -lpthread -lm ./*.c MethodParser/MethodLexAna.lex.c MethodParser/MethodParser.tab.c

MethodParser/MethodLexAna.lex.c: MethodParser/MethodParser.tab.c MethodParser/MethodLexAna.y
	flex -o MethodParser/MethodLexAna.lex.c -i MethodParser/MethodLexAna.y

MethodParser/MethodParser.tab.c: MethodParser/MethodParser.y
	bison -d MethodParser/MethodParser.y
