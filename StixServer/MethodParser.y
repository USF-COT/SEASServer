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
    int yylex(void);
    void yyerror (char const *);
    int yywrap(void);
%}

%union {
    double doubleVal;
    unsigned char charVal;
    int intVal;
}
%token <doubleVal> DVAL 
%token <intVal> IVAL
%token <charVal> PUMP LAMP VALVE HEATER 
%token <charVal> ON OFF
%token SET READ CALC  
%type <charVal> controlExp pumpOnExp lampExp valveOnExp heaterOnExp devOffExp device

/* Grammer Follows */
%%

/* Meta Grammer Definition Follows */
input:  /* empty */
       | input line 
       | input error
;

line:     '\n'
        | controlExp '\n' 
//      | setExp '\n'
//      | readExp '\n' 
//      | calcExp '\n'
//      | fileExp '\n'
//      | writeExp '\n'
;

/* Control Expression Grammers Follow */
controlExp:   pumpOnExp 
            | lampExp
            | valveOnExp
            | heaterOnExp 
            | devOffExp
;

device:   PUMP {$$=$1}
        | VALVE {$$=$1}
        | HEATER {$$=$1}

devOffExp: device OFF IVAL { printf("Sending command: %02x %02x %02x\n",$1,$2,(unsigned char)$3); $$=1;
}
;

pumpOnExp: PUMP ON IVAL IVAL { printf("Sending command: %02x %02x %02x %d\n",$1,$2,(unsigned char)$3,$4); $$=1;}
;

lampExp:   LAMP ON {printf("Sending command: %02x %02x\n",$1,$2); $$=1;}
         | LAMP OFF {printf("Sending command: %02x %02x\n",$1,$2); $$=1;}
;

valveOnExp: VALVE ON IVAL {printf("Sending command: %02x %02x %02x\n",$1,$2,(unsigned char)$3);$$=1;}
;

heaterOnExp: HEATER ON IVAL DVAL {printf("Sending command: %02x %02x %02x %g\n",$1,$2,(unsigned char)$3,$4); $$=1;}
;

/* Setting Expression Grammers Follow */
setExp:   specParams
        | absWavesParams
        | nonAbsWavesParams

specParams:  SET SPECPARAMS IVAL IVAL IVAL {printf("Setting Spectrometer %d Parameters: %d Integration Time, %d Boxcar Scans.\n",$3,$4,$5); $$=1;}

iValSeries: IVAL
            | iValSeries IVAL
;

absWaveParams: SET ABSWAVES IVAL iValSeries 

%%

void yyerror (char const* error){
    fprintf(stderr,"Error: %s\n",error);
}


