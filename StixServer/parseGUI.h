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
#include "GUI protocol.h"
#include "USB4000Manager.h"
#include "methodFileManager.h"

#ifndef _PARSEGUI_H
#define	_PARSEGUI_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct GUIRESPONSE{
        void* response;
        unsigned int length;
    }GUIresponse;

    GUIresponse* parseGUI(char* command);
    GUIresponse* createResponse(unsigned int length, void* response);
    GUIresponse* createResponseString(char* string);
    void freeResponse(GUIresponse* response);

#ifdef	__cplusplus
}
#endif

#endif	/* _PARSEGUI_H */

