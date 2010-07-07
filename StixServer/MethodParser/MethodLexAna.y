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
    #define DEBUGANA 1

    void debug(char* message);
    unsigned char getProtocol(char * name);
%}

DIGIT [0-9]

%%

pump {debug("Pump command.\n"); return PUMP;}
lamp {debug("Lamp command.\n"); return LAMP;}
valve {debug("Valve command.\n"); return VALVE;}
heater {debug("Heater command.\n"); return HEATER;}

on|open { debug("On parameter.\n"); return ON;}
off|close { debug("Off parameter.\n"); return OFF;}

[+-]?{DIGIT}+"."?{DIGIT}* {printf("Value: %s\n",yytext); yylval.doubleVal=atof(yytext); return VAL; }

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
