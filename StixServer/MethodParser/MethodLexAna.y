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
    #include "MethodNodesTable.h"
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

on { debug("On parameter.\n"); return ON;}
off { debug("Off parameter.\n"); return OFF;}

set { debug("Set command.\n"); return SET;}
spectrometer { debug("Spectrometer keyword.\n"); return SPECM;}
parameters { debug("Parameters keyword.\n"); return PARAMS;}
sample { debug("Sample keyword.\n"); return SAMP;}
wavelength { debug("Wavelength keyword.\n"); return WAVE;}
correction { debug("Correction keyword.\n"); return CORR;}
dwell { debug("Dwell keyword.\n"); return DWELL;}

read { debug("Read keyword.\n"); return READ;}
reference { debug("Reference keyword.\n"); return REF;}
absorbance { debug("Absorbance keyword.\n"); return ABSO;}
full { debug("Full keyword.\n"); return FULL;}
spectrum { debug("Spectrum keyword.\n"); return SPEC;}

calculate { debug("Calculate keyword.\n"); return CALC;}
concentration { debug("Concentration keyword.\n"); return CONC;}
pCO2 { debug("pCO2 keyword.\n"); return PCO2;}
pH { debug("pH keyword.\n"); return PH;}

open { debug("Open keyword.\n"); return OPEN;}
close { debug("Close keyword.\n"); return CLOSE;}
data { debug("Data keyword.\n"); return DATA;}
file { debug("File keyword.\n"); return FIL;}

write { debug("Write keyword.\n"); return WRITE;}

delay { debug("Debug keyword.\n"); return DELAY;}

begin { debug("Begin keyword.\n"); return BEG;}
end { debug("End keyword.\n"); return EN;}
loop { debug("Loop keyword.\n"); return LOOP;}

[+-]?{DIGIT}+"."?{DIGIT}* {printf("Value: %s\n",yytext); yylval.doubleVal=atof(yytext); return VAL; }

"#"[a-z0-9 \t]*|"//"[a-z0-9 \t]*"\n"|"/*"[a-z0-9 \t\n]*"*/" /* eat up comments */

\n {return '\n';}

[ \t]+ /* eat up whitespace */

%%

int main(void){
    int retVal;
    yyparse();
    runNodes();
    sleep(2);
    clearNodes();
    sleep(2);
    runNodes();
    while(1){};
}
