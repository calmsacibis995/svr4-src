/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/incl/utime.h	1.1"
/*ident	"@(#)cfront:incl/utime.h	1.4"*/

#ifndef UTIMEH
#define UTIMEH

/* <sys/types.h> must be included */

#ifndef utimbuf
struct utimbuf {
          time_t actime;
          time_t modtime;
       };
#endif

extern int utime (const char*, utimbuf*);

#endif
