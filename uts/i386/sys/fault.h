/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_FAULT_H
#define _SYS_FAULT_H

#ident	"@(#)head.sys:sys/fault.h	1.6.5.1"

/*
 * Fault numbers, analagous to signals.  These correspond to
 * hardware faults.  Setting the appropriate flags in a process's
 * set of traced faults via /proc causes the process to stop each
 * time one of the designated faults occurs so that a debugger can
 * take action.  See proc(4) for details.
 */

	/* fault enumeration must begin with 1 */
#define	FLTILL		1	/* Illegal instruction */
#define	FLTPRIV		2	/* Privileged instruction */
#define	FLTBPT		3	/* Breakpoint instruction */
#define	FLTTRACE	4	/* Trace trap (single-step) */
#define	FLTACCESS	5	/* Memory access (e.g., alignment) */
#define	FLTBOUNDS	6	/* Memory bounds (invalid address) */
#define	FLTIOVF		7	/* Integer overflow */
#define	FLTIZDIV	8	/* Integer zero divide */
#define	FLTFPE		9	/* Floating-point exception */
#define	FLTSTACK	10	/* Irrecoverable stack fault */
#define	FLTPAGE		11	/* Recoverable page fault (no associated sig) */

typedef struct {		/* fault set type */
	unsigned long	word[4];
} fltset_t;

#endif	/* _SYS_FAULT_H */
