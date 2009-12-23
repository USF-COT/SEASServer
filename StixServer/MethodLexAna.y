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
    #include <stdlib.h>
    #include "protocol.h"
    #include "MethodParser.tab.h"

    unsigned char getProtocol(char * name);
%}

DIGIT [0-9]

%%

pump {printf("PUMP command.\n"); yylval.charVal = getProtocol(yytext); return PUMP;}
lamp {printf("LAMP command.\n"); yylval.charVal = getProtocol(yytext); return LAMP;}
valve {printf("VALVE command.\n"); yylval.charVal = getProtocol(yytext); return VALVE;}
heater {printf("HEATER command.\n"); yylval.charVal = getProtocol(yytext); return HEATER;}

on|open { printf("ON parameter.\n"); yylval.charVal=1; return ON;}
off|close { printf("OFF parameter.\n"); yylval.charVal=0; return OFF;}

set { printf("SET Command.\n"); return SET;}
read { printf("READ Command.\n"); return READ;}
calc|calculate { printf("CALC Command.\n"); return CALC;}

[+-]?{DIGIT}+"."{DIGIT}*f? {printf("Double Value: %s\n",yytext); yylval.doubleVal=atof(yytext); return DVAL; }

[+-]?{DIGIT}+i? {printf("Int Value: %s\n",yytext); yylval.intVal=atoi(yytext); return IVAL;}

"//"[a-z0-9]*"\n"|"/*"[a-z0-9]*"*/" /* eat up comments */

\n {return '\n';}

[ \t]+ /* eat up whitespace */

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


lookupRec const* lookupKeywordProtocol(char * keyword)
{
    int i;

    // Convert Keyword to Lower Case
    for(i=0; i < strlen(keyword); i++)
        keyword[i] = tolower(keyword[i]);

    for(i=0; lookups[i].protocol != 0; i++){
        if(strcmp(lookups[i].name,keyword) == 0)
            return &lookups[i];
    }
    return (lookupRec*)0;
}

unsigned char getProtocol(char * name)
{
    lookupRec const* record = lookupKeywordProtocol(name);    
    if(record != (lookupRec*) 0)
        return record->protocol;
    else
        return 0;
}

int main(void){
    return yyparse();
}
