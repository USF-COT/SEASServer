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
    unsigned char charVal;
}
%token <doubleVal> DVAL ID
%token <charVal> DEVICE SWITCH 
%type <charVal> controlExp

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

controlExp:  DEVICE SWITCH DVAL { printf("Sending command: %02x %02x %g",$1,$2,$3); $$=1}
            | DEVICE SWITCH ID DVAL { printf("Sending command: %02x %02x %g %g",$1,$2,$3,$4); $$=1}

%%

void yyerror (char const* error){
    fprintf(stderr,"Error: %s\n",error);
}

