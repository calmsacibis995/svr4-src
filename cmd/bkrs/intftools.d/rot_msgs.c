/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:intftools.d/rot_msgs.c	1.2.2.1"

char *errmsgs[] = {
	"Option \"%c\" is invalid.\n",
	"Argument \"%s\" is invalid.\n",
	"Cannot open table %s (%d).\n",
	"Cannot allocate memory.\n",
	"Cannot %s ROTATION period in %s.\n",
	"Cannot %s ROTATION STARTED in %s.\n",
	"Unknown error %d returned from %s().\n",
};
int	lasterrmsg = sizeof( errmsgs )/sizeof( char * );
