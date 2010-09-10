#ifndef METHODFILEMANAGER_H
#define METHODFILEMANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/syslog.h>
#include <fcntl.h>
#include "parseGUI.h"
#include "GUIprotocol.h"

/* ReceiveMethodFile - Responsible for parsing filename and contents from a character buffer
 * in a specified format.  Lines are denoted by the reserved '\n' character.
 * 
 * Buffer format:
 * Line 1: Filename
 * Line 2-EOF: Contents
 */
GUIresponse* receiveMethodFile(char* contentBuffer);
GUIresponse* getMethodFileList();
GUIresponse* readMethodFile(char* contentBuffer);

#endif
