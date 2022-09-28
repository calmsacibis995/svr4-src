/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)lprof:libprof/debug.h	1.2"
/*
*	file: debug.h
*	desc: Debug macros for the profiler.
*	date: 11/09/88
*/
#include "stdio.h"

#ifdef ddt
int	debug_value;
#define DEBUG(exp)	exp; fflush(stdout)
#define DEBUG_LOC(name)	printf("Location: %s\n",name); fflush(stdout)

#define NO_DEBUG(exp)
#define NO_DEBUG_LOC(name)

#else

#define DEBUG(exp)
#define DEBUG_LOC(name)

#define NO_DEBUG(exp)
#define NO_DEBUG_LOC(name)
#endif

