#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "MethodNodesTable.h"
#include "MethodParser.tab.h"

extern FILE* yyin;

int main(){
    yyin = fopen("test.m","r");
    yyparse();
    printf("Running Nodes.\n");
    runNodes();
    printf("Nodes run.\n");
    sleep(10);
    printf("Clearing nodes.\n");
    clearNodes();
    printf("Nodes clear.\n");
    printf("CTRL-C ME!\n");
    while(1){};
}
