/*  MethodLexAna.y
 * 
 * Defines the Lexical Analyzer
 * for interpreting Mini-Seas
 * method scripts.
 *
 * By: Michael Lindemuth
 */

%{
    #include <stdio.h>
    #include <ctype.h>
    #include "protocol.h"
    #include "MethodParser.tab.h"

    unsigned char getProtocol(char const * name);
%}

DIGIT [0-9]

%%

pump|lamp|valve|heater { printf("Keyword: %s\n",yytext); yylval.charVal=getProtocol(yytext); return DEVICE; }

on { printf("Switch: %s\n",yytext); yylval.charVal=1; return SWITCH;}
off { printf("Switch: %s\n",yytext); yylval.charVal=0; return SWITCH;}

{DIGIT}+"."{DIGIT}*|{DIGIT}+ {printf("Double Value: %s\n",yytext); yylval.doubleVal=atof(yytext); return DVAL; }

"//"[a-z0-9]*"\n"|"/*"[a-z0-9]*"*/" /* eat up comments */

[ \t\n]+ /* eat up whitespace */

%%

typedef struct lookupRec lookupRec;
struct lookupRec{
    char const *name;
    unsigned char protocol;
};

struct lookupRec const lookups[] =
{
    "pump",PMP,
    "lamp",LTE,
    "valve",VLV,
    "heater",HTR,
    "0",0
};


lookupRec* lookupKeywordProtocol(char const* keyword)
{
    int i;

    for(i=0; lookups[i].protocol != 0; i++){
        if(strcmp(lookups[i].name,keyword) == 0)
            return &lookups[i];
    }
    return (lookupRec*)0;
}

unsigned char getProtocol(char const * name)
{
    lookupRec* record = lookupKeywordProtocol(name);    
    if(record != (lookupRec*) 0)
        return record->protocol;
    else
        return 0;
}

int main()
{
    yyin = stdin;
    return yyparse();
}