/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)x286emul:h/brk.h	1.1"

/* commands for brkctl */
#define	BR_ARGSEG	0001	/* specified segment */
#define	BR_NEWSEG	0002	/* new segment */
#define	BR_IMPSEG	0003	/* implied segment - last data segment */
#define BR_FREESEG	0004	/* free the specified segment */
#define BR_HUGE		0100	/* do the specified command in huge context */
