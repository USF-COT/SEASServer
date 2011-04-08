#include "MethodNodesTable.h"

#define MAXCONTROLFLOWDEPTH 20

// Private members and methods

static volatile sig_atomic_t traversingNodes = 0;
static pthread_attr_t attr;
static pthread_t runnerThread;
static pthread_mutex_t nodesMutex = PTHREAD_MUTEX_INITIALIZER;

static s_node* head = NULL;
static s_node* current = NULL;

// Open Control Stack Parameters 
s_stack* openControlStack = NULL;

s_node* buildCommandNode(unsigned long argc, void* argv, void (*command)(unsigned long,void*),unsigned char commandID)
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
    node->commandID = commandID;

    return node;
}

s_node* buildControlNode(unsigned long argc, BOOL (*conditional)(unsigned long*,unsigned long*), unsigned char commandID)
{
    s_node* node = malloc(sizeof(s_node));
    node->argc = argc;
    node->argv = malloc(sizeof(unsigned long));
    ((unsigned long*)node->argv)[0] = argc;
    node->function.conditional = conditional;
    node->type = CONTROL;

    // Default Pointers
    node->next = NULL;
    node->branch = NULL;

    node->closed = FALSE;
    node->commandID = commandID;

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
            printf("Evaluating command node.\n");
            (*(node->function.command))(node->argc,node->argv);
            return node->next;
            break;
        case CONTROL:
            if((*(node->function.conditional))(&node->argc,(unsigned long*)node->argv))
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
    int i=0;
    if(openControlStack != NULL)
    {
        fprintf(stderr, "Cannot traverse a node tree without a complete control flow.");
        return;
    }

    s_node* node = head;
    traversingNodes = 1;
    pthread_mutex_lock(&nodesMutex);
    printf("Start nodes.\n");
    while(node != NULL && traversingNodes)
    {
        printf("Running node %d.\n",i++);
        node = (*nodeFunction)(node);
    }
    printf("No more nodes.\n");
    pthread_mutex_unlock(&nodesMutex);
    traversingNodes = 0;
}

void *processNodes(void* blah)
{
    traverseNodes(evaluateNode);

    pthread_exit(NULL);
}

// Public methods
void addCommandNode(unsigned long argc, void* argv, void (*command)(unsigned long,void*),unsigned char commandID)
{
    pthread_mutex_lock(&nodesMutex);
    s_node* node = buildCommandNode(argc, argv, command, commandID);
    linkNode(node);
    pthread_mutex_unlock(&nodesMutex);
}

void addControlNode(unsigned long argc,BOOL (*conditional)(unsigned long*,unsigned long*),unsigned char commandID)
{
    pthread_mutex_lock(&nodesMutex);

    s_node* node = buildControlNode(argc, conditional, commandID);
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
        syslog(LOG_DAEMON|LOG_ERR,"Invalid Call to closeControlNode() Method.  Empty Control Stack.\n");
        return;
    }

    // Close control by having it link back to the control node so that conditional is checked again
    if(node == current){ // Empty loop points branches back to itself
        current->branch = current;
    } else { // Last node in branch chain points to control node top
        current->next = node;
    }

    // Mark the control node as closed
    node->closed = TRUE;

    current = node;

    pthread_mutex_unlock(&nodesMutex);
}

// Control Command Functions
BOOL methodDelay(unsigned long* delayInSeconds,unsigned long* originalValue){
    sleep(1);
    (*delayInSeconds)--;
    if(*delayInSeconds != 0){
        return TRUE;
    } else {
        *delayInSeconds = *originalValue;
        return FALSE;
    }
}

BOOL decCounterToZero(unsigned long* counter,unsigned long* originalValue){
    (*counter)--;
    if(*counter != 0){
        return TRUE;
    } else {
        *counter = *originalValue;
        return FALSE;
    }
}

s_node* getHeadNode(){
    // Check to Make Sure There is a Closed Execution Path
    if(!stackIsEmpty(openControlStack)){
        syslog(LOG_DAEMON|LOG_ERR,"Control stack not empty!");
    }

    // Return head node
    return head;
}

void runNodes()
{
    int rc;

    if(!traversingNodes)
    {
        printf("Creating thread.\n");
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
        rc = pthread_create(&runnerThread,&attr,processNodes,NULL);
        if(rc)
        {
            fprintf(stderr,"ERROR: Unable to create runner thread.\n");
            return;
        }
    }
}

void stopNodes()
{
    traversingNodes = 0;
}

void clearNodes()
{
    traverseNodes(freeNode);
    head = NULL;
}


