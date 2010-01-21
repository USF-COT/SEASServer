#include <stdio.h>
#include <stdlib.h>
#include "MethodNodesTable.h"

// Stack Code Derived From:
// http://www.cs.bu.edu/teaching/c/stack/array/

s_stack* initStack(int maxSize)
{
    s_stack* stack;
    s_node **newContents;

    /* Allocate memory for array and contents */
    stack = (s_stack*) malloc(sizeof(s_stack));
    newContents = (s_node**)malloc(sizeof(s_node*)*maxSize);

    if(newContents == NULL || stack == NULL)
    {
        fprintf(stderr, "Insufficient Memory, Cannot Create Stack.\n");
        exit(1);
    }

    stack->contents = newContents;
    stack->maxSize = maxSize;
    stack->top = -1;
    return stack;
}

void destroyStack(s_stack* stack)
{
    free(stack->contents);
    free(stack);
}

BOOL stackIsEmpty(s_stack* stack)
{
    if(stack == NULL)
        return TRUE;
    else if(stack->top == -1)
        return TRUE;
    else
        return FALSE;
}

BOOL stackIsFull(s_stack* stack)
{
    if(top >= (maxSize - 1))
        return TRUE;
    else
        return FALSE;
}

void stackPush(s_stack* stack,s_node* node)
{
   if (stackIsFull(stack))
   {
       fprintf(stderr, "Cannot push element on stack: stack is full.\n");
   }
   
   /* Put information in array; update top. */
   stack->contents[++stack->top] = node; 
}

s_node* stackPop(s_stack* stack)
{
    if(stackIsEmpty(stack))
    {
        fprintf(stderr, "Cannot pop element from and empty stack.\n");
        return NULL;
    }
    
    /* Return top element and decrement top */
    return stack->contents[stack->top--];
}

