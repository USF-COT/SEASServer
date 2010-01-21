/* MethodNodesStack.h - Creates a stack of pointers to node records.  Used mainly for tracking loops and maybe someday conditional start and end.
 *
 * By: Michael Lindemuth
 *
 * Code Derived From:
 * http://www.cs.bu.edu/teaching/c/stack/array/
 */

#ifndef METHODNODESSTACK_H
#define METHODNODESTACK_H

#include "MethodNodesTable.h" // Used for s_node struct and BOOL array

typedef struct stack
{
    s_node** contents;
    int top;
    int maxSize;
}s_stack;

s_stack* initStack(int maxSize);
void destroyStack(s_stack* stack);
BOOL stackIsEmpty(s_stack* stack);
BOOL stackIsFull(s_stack* stack);
void stackPush(s_stack* stack, s_node* node);
s_node* stackPop(s_stack* stack);

