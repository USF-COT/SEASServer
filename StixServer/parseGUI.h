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
    void freeResponse(GUIresponse* response);

#ifdef	__cplusplus
}
#endif

#endif	/* _PARSEGUI_H */

