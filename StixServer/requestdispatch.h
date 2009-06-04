/* 
 * File:   reqhandler.h
 * Author: seraph
 *
 * Created on June 3, 2009, 12:59 PM
 */

#ifndef _REQHANDLER_H
#define	_REQHANDLER_H

#ifdef	__cplusplus
extern "C" {
#endif

#define MAXREQUEST 512

typedef enum{LON,GUI}REQUESTTYPE;

char dispatchRequest(REQUESTTYPE type, char* request,unsigned int length);

#ifdef	__cplusplus
}
#endif

#endif	/* _REQHANDLER_H */

