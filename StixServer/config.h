/* 
 * File:   config.h
 * Author: Michael Lindemuth
 *
 * Created on June 3, 2009, 1:00 PM
 */

#ifndef _CONFIG_H
#define	_CONFIG_H

#define MAXCONFIGLINE 128
#define MAXCONFIGPREFIX 10
#define MAXCONFIGSUFFIX 118

#ifdef	__cplusplus
extern "C" {
#endif

char readConfig(char* path);

#ifdef	__cplusplus
}
#endif

#endif	/* _CONFIG_H */

