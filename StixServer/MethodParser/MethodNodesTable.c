#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "MethodNodesTable.h"
#include "MethodNodesStack.h"

#define MAXCONTROLFLOWDEPTH 20

// Private members and methods

BOOL running = FALSE;
pthread_t runnerThread;
pthread_mutex_t nodesMutex = PTHREAD_MUTEX_INITIALIZER;

s_node* head = NULL;
s_node* current = NULL;

// Open Control Stack Parameters 
s_stack* openControlStack = NULL;

s_node* buildCommandNode(unsigned long argc, void* argv, void (*command)(unsigned long,void*))
{
    s_node* node = malloc(sizeof(s_node));
    node->argc = argc;
    node->argv = argv;
    node->function.command = command;
    node->type = COMMAND;

    // Default Pointers
    node->next = NULL;
    node->branch = NULL;

    node->closed = TRUE;

    return node;
}

s_node* buildControlNode(unsigned long argc, BOOL (*conditional)(unsigned long))
{
    s_node* node = malloc(sizeof(s_node));
    node->argc = argc;
    node->argv = NULL;
    node->function.conditional = conditional;
    node->type = CONTROL;

    // Default Pointers
    node->next = NULL;
    node->branch = NULL;

    node->closed = FALSE;

    return node;
}

void linkNode(s_node* node)
{
    if(head == NULL)
    {
        head = node;
        current = head;
    }
    else if(current->type == CONTROL && current->closed == FALSE)
    {
        current->branch = node;
        current = node;
    }
    else
    {
        current->next = node;

        current = node;
    }
}

// This should work...
// May want to look at storing a argv free function within the node
// that is more specific to what is stored in the void* pointer.
s_node* freeNode(s_node* node)
{
    s_node* next;

    // If this is the first time hitting a proper loop
    if(node->branch != NULL && node->closed == TRUE)
    {
        node->closed = FALSE;
        next = node->branch;
    }
    else
        next = node->next;

    free(node->argv);
    free(node);

    return next;
}

s_node* evaluateNode(s_node* node)
{
    switch(node->type)
    {
        case COMMAND:
            (*(node->function.command))(node->argc,node->argv);
            return node->next;
            break;
        case CONTROL:
            if((*(node->function.conditional))(node->argc))
                return node->branch;
            else
                return node->next;
            break;
    }    
}

// Traverses the node structure and applies a passed function to each node.
// The passed function does something to a node and returns what the next 
// node to be traversed should be.
//
// NOTE: This function will not traverse a node list that has an incomplete control flow.
void traverseNodes(s_node* (*nodeFunction)(s_node*))
{
    if(openControlStack != NULL)
    {
        fprintf(stderr, "Cannot traverse a node tree without a complete control flow.");
        return;
    }

    s_node* node = head;
    running = TRUE;
    while(node != NULL && running)
    {
        node = (*nodeFunction)(node);
    }
    running = FALSE;
}

void *processNodes(void* blah)
{
    pthread_mutex_lock(&nodesMutex);

    traverseNodes(evaluateNode);

    pthread_mutex_unlock(&nodesMutex);

    pthread_exit(NULL);
}

// Public methods

void addCommandNode(unsigned long argc, void* argv, void (*command)(unsigned long,void*))
{
    pthread_mutex_lock(&nodesMutex);
    s_node* node = buildCommandNode(argc, argv, command);
    linkNode(node);
    pthread_mutex_unlock(&nodesMutex);
}

void addControlNode(unsigned long argc,BOOL (*conditional)(unsigned long))
{
    pthread_mutex_lock(&nodesMutex);

    s_node* node = buildControlNode(argc, conditional);
    linkNode(node);
    
    // Track the Open Control Node
    if(openControlStack == NULL)
        openControlStack = initStack(MAXCONTROLFLOWDEPTH);

    stackPush(openControlStack,node);

    pthread_mutex_unlock(&nodesMutex);
}

void closeControlNode()
{
    pthread_mutex_lock(&nodesMutex);

    s_node* node = stackPop(openControlStack);

    if(node == NULL)
    {
        fprintf(stderr, "Invalid Call to closeControlNode() Method.  Empty Control Stack.\n");
        return;
    }

    // Close control by having it link back to the control node so that conditional is checked again
    current->next = node;

    // Mark the control node as closed
    node->closed = TRUE;

    pthread_mutex_unlock(&nodesMutex);
}

void runNodes()
{
    int rc;
    if(!running)
    {
        rc = pthread_create(&runnerThread,NULL,processNodes,NULL);
        if(rc)
        {
            fprintf(stderr,"ERROR: Unable to create runner thread.\n");
            return;
        }
    }
}

void stopNodes()
{
    running = FALSE;
}

void clearNodes()
{
    traverseNodes(freeNode);
}


