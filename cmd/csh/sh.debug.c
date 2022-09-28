/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)csh:sh.debug.c	1.2.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley Software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include "sh.h"


#ifdef TRACE
#include <stdio.h>
FILE *trace;
/*
 * Trace routines
 */
#define TRACEFILE "/tmp/trace.XXXXXX"

/*
 * Initialie trace file.
 * Called from main.
 */
trace_init()
{
	extern char *mktemp();
	char name[128];
	char *p;

	strcpy(name, TRACEFILE);
	p = mktemp(name);
	trace = fopen(p, "w");
}

/*
 * write message to trace file
 */
/*VARARGS1*/
tprintf(fmt,a,b,c,d,e,f,g,h,i,j)
     char *fmt;
{
	if (trace) {
		fprintf(trace, fmt, a,b,c,d,e,f,g,h,i,j);
		fflush(trace);
	}
}
#endif
