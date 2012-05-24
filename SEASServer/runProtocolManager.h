/* Run Protocol Manager - Responsible for Translating Method Nodes into Run Protocol Messages.
 *      Handles a socket and node, looks up the correct run protocol response, and sends it.
 *
 * By: Michael Lindemuth
 */

#ifndef RUNPROTOMAN_H
#define RUNPROTOMAN_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include "SEASPeripheralCommands.h"
#include "USB4000Manager.h"
#include "MethodNodesTable.h"
#include "runProtocol.h"

typedef void (*RUNNODEHandler)(int,s_node*);

void sendRunProtocolMessage(int connection, s_node* node);

#ifdef __cplusplus
 }
#endif

#endif
