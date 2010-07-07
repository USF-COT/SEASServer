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

pump {debug("PUMP command.\n"); yylval.charVal = getProtocol(yytext); return PUMP;}
lamp {debug("LAMP command.\n"); yylval.charVal = getProtocol(yytext); return LAMP;}
valve {debug("VALVE command.\n"); yylval.charVal = getProtocol(yytext); return VALVE;}
heater {debug("HEATER command.\n"); yylval.charVal = getProtocol(yytext); return HEATER;}

on|open { debug("ON parameter.\n"); yylval.charVal=1; return ON;}
off|close { debug("OFF parameter.\n"); yylval.charVal=0; return OFF;}

set { debug("SET Command.\n"); return SET;}
read { debug("READ Command.\n"); return READ;}
calc|calculate { debug("CALC Command.\n"); return CALC;}
record { debug("RECORD Command.\n"); return RECORD; }
write { debug("WRITE Command.\n"); return WRITE; }

// Set Types
"spectrometer parameters" { debug("Spec Parameters.\n"); return SPECPARAMS;}
"absorbance wavelengths" { debug("Abs Parameters.\n"); return ABSPARAMS;}
"non-absorbance wavelength" { debug("Non-absorbance Parameters.\n"); return NONABSPARAMS;}
"dwell" { debug("Dwell Parameter.\n"); return DWELL; }

// Read Types
"reference" { debug("Reference Sample.\n"); return REF;}
"sample" { debug("Sample.\n"); return SAMPLE;}
"absorbance correction" { debug("Absorbance Correction.\n"); return ABSCOR; }
"full spectrum" { debug("Full Spectrum.\n"); return FULLSPEC; }

// Calculate Types
"concentration" { debug("Concentration.\n"); return CONC; }
"total cabon" { debug("Total Carbon.\n"); return TOTC; }
"pco2" { debug("pCO2.\n"); return PCO2; }
"ph" { debug("pH.\n"); return PH; }
"standards" { debug("Standards.\n"); return STANDARDS; }

// Data File
"data file" { debug("Data File.\n"); return DATAFILE; }

// Delay
"delay" {debug("Delay.\n"); return DELAY; }

// Loop Stuff
"begin" {debug("Begin.\n"); return BEGIN; }
"end" {debug("End.\n"); return END; }
"loop" {debug("Loop.\n"); return LOOP; }

// Miscellaneous
"beep" { debug("Beep.\n"); return BEEP; }

[+-]?{DIGIT}+ | [+-]?{DIGIT}+"."{DIGIT}* {debug("Value: %s\n",yytext); yylval.doubleVal=atof(yytext); return VAL; }

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

void debug(char* message)
{
#if DEBUGANA
    printf(message);
#endif
}

int main(void){
    return yyparse();
}
