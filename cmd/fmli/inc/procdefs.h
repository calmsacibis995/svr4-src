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

#ident	"@(#)fmli:inc/procdefs.h	1.1"

typedef int proc_id;

/* arguments for the flags field of the process open calls */

#define PR_NOPROMPT	(1)		/* never prompt the user on proc termination */
#define PR_ERRPROMPT	(2)	/* only prompt if nonzero exit code from proc */
#define PR_CLOSING	(4)		/* process must end */
