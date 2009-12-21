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
%token <charVal> DEVICE SWITCH 
%token <intVal> IVAL
%type <charVal> controlExp

/* Grammer Follows */
%%
input:  /* empty */
       | input line 
       | input error 
;

line:  '\n'
      | controlExp '\n'
//      | setExp '\n'
//      | readExp '\n'
//      | calcExp '\n'
//      | fileExp '\n'
//      | writeExp '\n'
;

controlExp:  DEVICE SWITCH IVAL { printf("Sending command: %02x %02x %02x\n",$1,$2,(unsigned char)$3); $$=1}
            | DEVICE SWITCH IVAL DVAL { printf("Sending command: %02x %02x %02x %g\n",$1,$2,(unsigned char)$3,$4); $$=1}
            | DEVICE SWITCH IVAL IVAL { printf("Sending command: %02x %02x %02x %d\n",$1,$2,(unsigned char)$3,$4); $$=1}

%%

void yyerror (char const* error){
    fprintf(stderr,"Error: %s\n",error);
}

int yywrap(){
    return 1;
}

