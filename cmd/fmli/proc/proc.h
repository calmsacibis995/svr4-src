/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:proc/proc.h	1.2"

#define MAX_PROCS	5
#define MAX_ARGS	10

#ifndef NOPID			/* EFT abs k16 */
#define NOPID	(pid_t)(-1)	/* EFT abs k16 */
#endif
#define ST_RUNNING	0
#define ST_DEAD		1
#define ST_SUSPENDED	2

struct proc_rec {
	char *name;
	char *argv[MAX_ARGS+2];
	int status;			/* running, dead, or suspended */
	int flags;			/* prompt at end */
	pid_t pid;			/* actual process id.    EFT k16 */
	pid_t respid;			/* process id to resume  EFT k16 */
	struct actrec *ar;	/* activation record proc is in */
};
