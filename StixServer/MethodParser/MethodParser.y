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
%token PUMP LAMP VALVE HEATER ON OFF SET READ CALC RECORD WRITE VAL

/* Grammer Follows */
%%

/* Meta Grammer Definition Follows */
input:  /* empty */
       | input line 
       | input error
;

line:     '\n'
        | controlExp '\n' 
;

/* Control Expression Grammers Follow */
controlExp:   PUMP ON VAL VAL
            | PUMP OFF VAL
            | LAMP ON
            | LAMP OFF
            | HEATER ON VAL VAL
            | HEATER OFF VAL {printf("Control expression recognized!");}             
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
