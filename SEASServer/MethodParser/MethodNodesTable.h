/* MethodNodesTable.h - Defines the structures and functions to be created by parsing through a method script.  Also provides a method to run through the linked list of nodes and run the function defined in each.
 *
 * By: Michael Lindemuth
 */

#ifndef METHODNODESTABLE_H
#define METHODNODESTABLE_H 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include "../globalIncludes.h"
#include "../runProtocol.h"
#include "MethodNodesStack.h"

typedef enum type{COMMAND,CONTROL}e_type;

typedef struct node{
    e_type type;
    unsigned long argc;
    void* argv;
    union
    {
        void (*command)(unsigned long,void*);
        BOOL (*conditional)(unsigned long*,unsigned long*);
    } function;
    struct node* next;
    struct node* branch;
    BOOL closed;
    unsigned char commandID; // For run protocol lookup
}s_node;

void addComandNode(unsigned long argc, void* argv, void (*command)(unsigned long,void*),unsigned char commandID);
void addControlNode(unsigned long argc,BOOL (*conditional)(unsigned long*,unsigned long*),unsigned char commandID);
s_node* evaluateNode(s_node* node);
void closeControlNode();
// Control Command Functions
BOOL methodDelay(unsigned long* delayInSeconds,unsigned long* originalValue);
BOOL decCounterToZero(unsigned long* counter,unsigned long* originalValue);
s_node* getHeadNode();
void runNodes();
void stopNodes();
void clearNodes();

#endif
