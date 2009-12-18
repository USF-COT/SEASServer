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
%}

%union {
    double doubleVal;
    int intVal;
    unsigned char charVal;
}
%token <doubleVal> DVAL
%token <intVal> IVAL 
%token <charVal> DEVICE SWITCH ID

/* Grammer Follows */
%%
input:  /* empty */
       | input line
;

line:  '\n'
      | controlExp '\n'
//      | setExp '\n'
//      | readExp '\n'
//      | calcExp '\n'
//      | fileExp '\n'
//      | writeExp '\n'
;

controlExp:  DEVICE SWITCH IVAL { printf("Sending command: %02x %02x %02x",$1,$2,$3); }
            | DEVICE SWITCH ID IVAL { printf("Sending command: %02x %02x %02x %i",$1,$2,$3,$4); }
            | DEVICE SWITCH ID DVAL { printf("Sending command: %02x %02x %02x %g",$1,$2,$3,$4); } 

%%

