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
    #include <syslog.h>
    #include "dataFileManager.h"
    #include "configManager.h"
    #include "SEASPeripheralCommands.h"
    #include "USB4000Manager.h"
    #include "MethodNodesTable.h"
    #include "runProtocol.h"
    #include "globalIncludes.h"
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
    double* allocateParamArray(uint16_t size);
%}

%union {
    double doubleVal;
    struct doubleNode* node;
}
%type <doubleVal> VAL 
%type <node> arrayExp
%token PUMP LAMP VALVE HEATER ON OFF READ CALC RECORD WRITE VAL SET DWELL SPECM PARAMS SAMP WAVE CORR REF ABSO NON FULL SPEC CONC PCO2 PH OPEN CLOSE DATA DELAY BEG EN LOOP FIL BEEP

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
        | BEEP '\n' { soundBeep(); }
;

/* Control Expression Grammers Follow */
controlExp:   PUMP ON VAL VAL { double* pumpArgs =  allocateParamArray(2); pumpArgs[0] = $3; pumpArgs[1] = $4; addCommandNode(2,(void*)pumpArgs, methodPumpOn,PUMP_ON_RUNTIME_CMD);}
            | PUMP OFF VAL { double* pumpArgs = allocateParamArray(1); pumpArgs[0] = $3; addCommandNode(1,(void*)pumpArgs, methodPumpOff,PUMP_OFF_RUNTIME_CMD); }
            | LAMP ON { addCommandNode(0,NULL,methodLampOn,LAMP_ON_RUNTIME_CMD); }
            | LAMP OFF { addCommandNode(0,NULL,methodLampOff,LAMP_OFF_RUNTIME_CMD); }
            | HEATER ON VAL { double* heaterArgs = allocateParamArray(1); heaterArgs[0]=$3; addCommandNode(1,(void*)heaterArgs,methodHeaterOn,HEATER_ON_RUNTIME_CMD); }
            | HEATER OFF { addCommandNode(0,NULL,methodHeaterOff,HEATER_OFF_RUNTIME_CMD); }             
;

/* Set Expression Grammers Follow */
setExp:    SET SPECM PARAMS VAL VAL VAL VAL {double* params =  allocateParamArray(4); params[0] = $4; params[1] = $5; params[2] = $6; params[3] = $7; addCommandNode(4,(void*)params,methodSetSpectrometerParameters,-1);}
         | SET SAMP WAVE arrayExp {double* params = drainListToArray($4);  addCommandNode((unsigned int)params[0],(void*)params,methodSetAbsorbanceWavelengths,-1);} 
         | SET NON ABSO WAVE VAL VAL {double* params =  allocateParamArray(2); params[0] = $5; params[1] = $6; addCommandNode(2,(void*)params,methodSetNonAbsorbanceWavelength,-1);}
         | SET CORR WAVE VAL VAL {} 
         | SET DWELL VAL VAL {double* params =  allocateParamArray(2); params[0] = $3; params[1] = $4; addCommandNode(2,(void*)params,methodSetDwell,-1);}
;

arrayExp:    VAL arrayExp {$$ = malloc(sizeof(DOUBLENODE)); if($$){ $$->next=$2; $$->value=$1;}else{yyerror("Could Not Allocate Double Node.");}}
           | VAL {$$=malloc(sizeof(DOUBLENODE)); if($$){ $$->next=NULL; $$->value=$1;}else{yyerror("Could Not Allocate Double Node.");}}
;

/* Read Expression Grammers Follow */
readExp:   READ REF VAL { double* params = allocateParamArray(1); params[0] = $3; addCommandNode(1,(void*)params,methodReadReference,READ_REFERENCE_RUNTIME_CMD); }
         | READ SAMP VAL { double* params = allocateParamArray(1); params[0] = $3; addCommandNode(1,(void*)params,methodReadSample,READ_SAMPLE_RUNTIME_CMD); }
         | READ FULL SPEC VAL { double* params = allocateParamArray(1); params[0] = $4; addCommandNode(1,(void*)params,methodReadFullSpec,READ_FULL_SPECTRUM_RUNTIME_CMD); }
;

/* Absorbance Correction Expression */
absExp:   ABSO CORR VAL VAL { double* params = allocateParamArray(2); params[0] = $3; params[1] = $4; addCommandNode(1,(void*)params,methodAbsCorr,-1);}
;

/* Calculate Expression Grammers Follow */
calcExp: CALC CONC VAL { double* params = allocateParamArray(1); params[0] = $3; addCommandNode(1,(void*)params,methodCalcConc,CAL_CONCENTRATION_RUNTIME_CMD);}
         CALC PCO2 VAL { double* params = allocateParamArray(1); params[0] = $3; addCommandNode(1,(void*)params,methodCalcPCO2,CAL_PCO2_RUNTIME_CMD); }
         CALC PH VAL { double* params = allocateParamArray(1); params[0] = $3; addCommandNode(1,(void*)params,methodCalcPH,CAL_PH_RUNTIME_CMD);}
;

/* Data File Expressions */

dataFileExp:   OPEN DATA FIL { addCommandNode(0,NULL,methodOpenDataFile,-1); }
             | CLOSE DATA FIL { addCommandNode(0,NULL,methodCloseDataFile,-1); }
;

/* Write Expressions */

writeExp:   WRITE CONC DATA VAL { addCommandNode(0,NULL,methodWriteConcData,-1); }
          | WRITE FULL SPEC VAL { addCommandNode(0,NULL,methodWriteFullSpec,-1)}
;

/* Delay Expression */

delayExp:   DELAY VAL { double* params = allocateParamArray(1); params[0] = $2; addCommandNode(0,NULL,methodDelay,DELAY_RUNTIME_CMD); }
;

/* Loop Expressions */

loopExp:   BEG LOOP VAL VAL { addControlNode(((unsigned long)$4),decCounterToZero); }
         | EN LOOP VAL { closeControlNode(); }
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

// Creates a double array numDoubleNodes+1 long.  The first element is the number of elements and the rest of the elements in the array are the double values stored in the linked list. 
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

double* allocateParamArray(uint16_t size){
    double* retVal = malloc(sizeof(double)*size);
    if(retVal == NULL){
        yyerror("Cannot allocate parameters array.");
    }
    return retVal;
}
