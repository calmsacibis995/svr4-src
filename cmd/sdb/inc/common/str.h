/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/str.h	1.2"
#ifndef STR_H
#define STR_H

#if defined(__STDC__) || defined(__cplusplus)
extern char *str( char * );
extern char *strn( char *, int );
extern char *sf( char *, ... );
#else
extern char *str();
extern char *strn();
extern char *sf();
#endif /* __STDC__ || __cplusplus */

#endif /* STR_H */
