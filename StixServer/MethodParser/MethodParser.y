/* MethodParser.y
 *
 * Defines the grammer for parsing 
 * Mini-SEAS Method Scripts
 *
 * By: Michael Lindemuth
 * 
 * Linked-List of Doubles derived from: http://stackoverflow.com/questions/1429794/how-to-build-an-array-with-bison-yacc-and-a-recursive-rule
 */

%{
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    #include "configManager.h"
    #include "SEASPeripheralCommands.h"
    #include "MethodNodesTable.h"
    #define DEBUGPARSER 1

    extern int yylineno;
    extern char* yytext;

    int yylex(void);
    void yyerror (char const *);
    int yywrap(void);
    void debug(char* message);

    // Variable Length Double Linked List
    typedef struct doubleNode{
        double value;
        struct doubleNode* next;
    }DOUBLENODE;

    double* drainListToArray(DOUBLENODE* head); 
%}

%union {
    double doubleVal;
    struct doubleNode* node;
}
%type <doubleVal> VAL 
%type <node> arrayExp
%token PUMP LAMP VALVE HEATER ON OFF READ CALC RECORD WRITE VAL SET DWELL SPECM PARAMS SAMP WAVE CORR REF ABSO NON FULL SPEC CONC PCO2 PH OPEN CLOSE DATA DELAY BEG EN LOOP FIL

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
controlExp:   PUMP ON VAL VAL { double* pumpArgs = malloc(sizeof(double)*2); pumpArgs[0] = $3; pumpArgs[1] = $4; addCommandNode(2,(void*)pumpArgs, methodPumpOn); printf("Pump ON!");}
            | PUMP OFF VAL { double* pumpArgs = malloc(sizeof(double)); pumpArgs[0] = $3; addCommandNode(1,(void*)pumpArgs, methodPumpOff); }
            | LAMP ON { addCommandNode(0,NULL,methodLampOn); }
            | LAMP OFF { addCommandNode(0,NULL,methodLampOff); }
            | HEATER ON VAL VAL { double* heaterArgs = malloc(sizeof(double)*2); heaterArgs[0]=$3; heaterArgs[1]=$4; addCommandNode(2,(void*)heaterArgs,methodHeaterOn); }
            | HEATER OFF VAL { double* heaterArgs = malloc(sizeof(double)); heaterArgs[0]=$3; addCommandNode(1,(void*)heaterArgs,methodHeaterOff); }             
;

/* Set Expression Grammers Follow */
setExp:    SET SPECM PARAMS VAL VAL VAL VAL {double* params = malloc(sizeof(double)*4); params[0] = $4; params[1] = $5; params[2] = $6; params[3] = $7; addCommandNode(4,(void*)params,methodSetSpectrometerParameters);}
         | SET SAMP WAVE arrayExp {double* params = drainListToArray($4);  addCommandNode((unsigned int)params[0],(void*)params,methodSetAbsorbanceWavelengths);} 
         | SET NON ABSO WAVE VAL VAL {double* params = malloc(sizeof(double*2)); params[0] = $5; params[1] = $6; addCommandNode(2,(void*)params,methodSetNonAbsorbanceWavelength);}
         | SET CORR WAVE VAL VAL 
         | SET DWELL VAL VAL {double* params = malloc(sizeof(double*2)); params[0] = $3; params[1] = $4; addCommandNode(2,(void*)params,methodSetDwell);}
;

arrayExp:    VAL arrayExp {$$ = malloc(sizeof(DOUBLENODE)); $$->next=$2; $$->value=$1;}
           | VAL {$$=malloc(sizeof(DOUBLENODE)); $$->next=NULL; $$->value=$1;}
        

/* Read Expression Grammers Follow */
readExp:   READ REF VAL
         | READ SAMP VAL
         | READ FULL SPEC VAL 
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
    fprintf(stderr,"Error on line %d: %s\n",yylineno,error);
}

void debug(char* message)
{
#if DEBUGPARSER
    printf("%s",message);
#endif
}

void addDoubleNode(double value){
    DOUBLENODE newNode = malloc(sizeof(DOUBLENODE));
    newNode->value = value;
    newNode->next = NULL;

    if(head == NULL){
        head = newNode;
    } else {
        current->next = newNode;
    }
    current = newNode;
    numDoubleNodes++;
}

// Creates a double array numDoubleNodes+1 long.  The first element is the number of elements and the rest of the elements in the array are the double values stored in the linked list.  This function also clears the linked list created by calling addDoubleNode.
double* drainListToArray(DOUBLENODE* head){
    DOUBLENODE* nodeTemp;
    DOUBLENODE* prevNode;
    double* retVal = NULL;
    unsigned int i = 0;

    unsigned int numDoubleNodes = 0;

    // Calculate the number of nodes
    nodeTemp = head;
    while(nodeTemp){
        numDoubleNodes++;
        nodeTemp = nodeTemp->next;
    }

    if(numDoubleNodes > 0){
        retVal = malloc(sizeof(double)*(numDoubleNodes+1));
        retVal[i++] = numDoubleNodes;
        nodeTemp = head;
        while(nodeTemp && i < numDoubleNodes+1){
            retVal[i++] = nodeTemp->value;
            prevNode = nodeTemp;
            nodeTemp = nodeTemp->next;
            free(prevNode);
        }
    }
    return retVal;
}
