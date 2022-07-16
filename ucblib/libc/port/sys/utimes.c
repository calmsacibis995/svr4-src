/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/* 	Portions Copyright(c) 1988, Sun Microsystems Inc.	*/
/*	All Rights Reserved					*/

#ident	"@(#)ucblibc:port/gen/utimes.c	1.1.3.1"
#include <sys/types.h>
#include <sys/time.h>
#include <sys/utime.h>

/*
 * The error code is set by the utime() system call
 */ 
utimes(file,tvp)
        char    *file;
        struct  timeval *tvp;
{
        struct  utimbuf ut;
        int     error;

        if (tvp) {
                ut.actime = tvp->tv_sec;
                ut.modtime = (++tvp)->tv_sec;
                error = utime(file, &ut);
        } else {
                error = utime(file, 0);
        }
        return error;
}
