/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:rsnotify.d/msgs.c	1.2.2.1"

char *errmsgs[] = {
	"Option \"%c\" is invalid.\n",
	"Argument \"%s\" is invalid.\n",
	"Unable to allocate memory for reading table entry.\n",
	"Unable to read table entry number %d (return code = %d).\n",
	"Unable to assign %s value to table entry number %d (return code = %d).\n",
	"Warning: table %s has different format than expected.\n",
	"Open of table %s failed (return code = %d).\n",
};
int	lasterrmsg = sizeof( errmsgs )/sizeof( char * );
