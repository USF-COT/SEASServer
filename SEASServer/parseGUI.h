/* 
 * File:   parseGUI.h
 * Author: Michael Lindemuth
 *
 * Created on June 10, 2009, 10:52 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include "config.h"
#include "GUIprotocol.h"
#include "USB4000Manager.h"
#include "methodFileManager.h"
#include "configManager.h"
#include "SEASPeripheralCommands.h"

#ifndef _PARSEGUI_H
#define	_PARSEGUI_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef void (*GUIHandler)(int,char*);

    void parseGUI(int connection,char* command);
    void sendErrorMessageBack(int connection, char* message);

#ifdef	__cplusplus
}
#endif

#endif	/* _PARSEGUI_H */

