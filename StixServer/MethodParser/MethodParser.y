/* MethodParser.y
 *
 * Defines the grammer for parsing 
 * Mini-SEAS Method Scripts
 *
 * By: Michael Lindemuth
 */

%{
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    #include "../SEASPeripheralCommands.h"
    #include "MethodNodesTable.h"
    #define DEBUGPARSER 1

    int yylex(void);
    void yyerror (char const *);
    int yywrap(void);
    void debug(char* message);
%}

%union {
    double doubleVal;
}
%type <doubleVal> VAL 
%token PUMP LAMP VALVE HEATER ON OFF READ CALC RECORD WRITE VAL SET DWELL SPECM PARAMS SAMP WAVE CORR REF ABSO FULL SPEC CONC PCO2 PH OPEN CLOSE DATA DELAY BEG EN LOOP FIL

/* Grammer Follows */
%%

/* Meta Grammer Definition Follows */
input:  /* empty */
       | input line 
       | input error
;

line:     '\n'
        | controlExp '\n' 
        | setExp '\n'
        | readExp '\n'
        | calcExp '\n'
        | absExp '\n'
        | dataFileExp '\n'
        | writeExp '\n'
        | delayExp '\n'
        | loopExp '\n' 
;

/* Control Expression Grammers Follow */
controlExp:   PUMP ON VAL VAL { double* pumpArgs = malloc(sizeof(double)*2); pumpArgs[0] = $3; pumpArgs[1] = $4; addCommandNode(2,(void*)pumpArgs, methodPumpOn); }
            | PUMP OFF VAL { double* pumpArgs = malloc(sizeof(double)); pumpArgs[0] = $3; addCommandNode(1,(void*)pumpArgs, methodPumpOff); }
            | LAMP ON { addCommandNode(0,NULL,methodLampOn); }
            | LAMP OFF { addCommandNode(0,NULL,methodLampOff); }
            | HEATER ON VAL VAL { double* heaterArgs = malloc(sizeof(double)*2); heaterArgs[0]=$3; heaterArgs[1]=$4; addCommandNode(2,(void*)heaterArgs,methodHeaterOn); }
            | HEATER OFF VAL { double* heaterArgs = malloc(sizeof(double)); heaterArgs[0]=$3; addCommandNode(1,(void*)heaterArgs,methodHeaterOff); }             
;

/* Set Expression Grammers Follow */
setExp:    SET specParameters
         | SET DWELL VAL VAL
;

specParameters:    SPECM PARAMS VAL VAL VAL
                 | SAMP WAVE VAL VAL
                 | CORR WAVE VAL VAL
;

/* Read Expression Grammers Follow */
readExp:   READ readParameters
;

readParameters:   REF VAL
                | SAMP VAL
                | FULL SPEC VAL
;

/* Absorbance Correction Expression */
absExp:   ABSO CORR VAL VAL
;

/* Calculate Expression Grammers Follow */
calcExp: CALC calcParameters VAL
;

calcParameters:   CONC
                | PCO2
                | PH
;

/* Data File Expressions */

dataFileExp:   OPEN DATA FIL
             | CLOSE DATA FIL
;

/* Write Expressions */

writeExp:   WRITE CONC DATA VAL
          | WRITE FULL SPEC VAL
;

/* Delay Expression */

delayExp:   DELAY VAL
;

/* Loop Expressions */

loopExp:   BEG LOOP VAL VAL
         | EN LOOP VAL
;      

%%

void yyerror (char const* error){
    fprintf(stderr,"Error: %s\n",error);
}

void debug(char* message)
{
#if DEBUGPARSER
    printf("%s",message);
#endif
}
