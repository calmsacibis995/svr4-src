/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lprof:libprof/symintUtil.c	1.4"

#include "hidelibc.h"		/* uses "_" to hide libc functions */
#include "hidelibelf.h"		/* uses "_" to hide libelf functions */

/*
*	file: symintUtil.c
*	desc: utilities for symint code
*	date: 11/08/88
*/
#include <stdio.h>
#include <sys/types.h>
#include "debug.h"

/*
*	_Malloc and _Realloc are used to monitor the allocation
*	of memory.  If failure occurs, we detect it and exit.
*/

void *
_Malloc(item_count, item_size)
uint item_count;
uint item_size;
{
	char *malloc();
	register void *p;

	if ((p = (void *) calloc(item_count, item_size)) == NULL) {
		DEBUG(printf("- size=%d, count=%d\n", item_size, item_count));
		_err_exit("calloc: Out of space");
	}
	return (p);
}

void *
_Realloc(pointer, size)
void *pointer;
uint size;
{
	char *realloc();
	register void *p;

	if ((p = (void *) realloc(pointer, size)) == NULL) {
		_err_exit("realloc: Out of space");
	}
	return (p);
}

