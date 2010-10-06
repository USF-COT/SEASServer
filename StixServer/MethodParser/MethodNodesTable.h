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
#include "MethodNodesStack.h"

typedef enum type{COMMAND,CONTROL}e_type;

typedef struct node{
    e_type type;
    unsigned long argc;
    void* argv;
    union
    {
        void (*command)(unsigned long,void*);
        BOOL (*conditional)(unsigned long);
    } function;
    struct node* next;
    struct node* branch;
    BOOL closed;
}s_node;

void addCommandNode(unsigned long argc, void* argv, void (*command)(unsigned long,void*));
void addControlNode(unsigned long argc,BOOL (*conditional)(unsigned long));
void closeControlNode();
void runNodes();
void stopNodes();
void clearNodes();

#endif
