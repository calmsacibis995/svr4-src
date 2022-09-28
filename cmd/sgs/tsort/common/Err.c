/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tsort:Err.c	1.1"

/*	Set default values in Err global structure.
*/

#include	"errmsg.h"

static	char	deftofix[] = "Refer to help error database or manual.";
static	char	*defsevmsg[] = {	/* default severity messages */
		"INFORM: ",
		"WARNING: ",
		"ERROR: ",
		"HALT: ",
		0
	};

struct Err	Err = {
					/* verbosity flags */
		/* vbell */	ENO,
		/* vprefix */	EYES,
		/* vsource */	EYES,
		/* vsevmsg */	EYES,
		/* vsyserr */	EDEF,
		/* vfix */	EYES,
		/* vtag */	EYES,
		/* vtext */	EYES,
					/* message content */
		/* prefix */	0,
		/* envsource */	0,
		/* source */	0,
		/* severity */	0,
		/* sevmsg */	defsevmsg,
		/* tofix */	deftofix,
		/* tagnum */	0,
		/* tagstr */	0,
		/* exit */	1,
};

