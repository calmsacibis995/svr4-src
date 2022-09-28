/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:intftools.d/cpmsgs.c	1.1.2.1"

char *errmsgs[] = {
	"Option \"%c\" is invalid.\n",
	"Illegal number of arguments.\n",
	"Invalid option or argument \"%s\".\n",
	"Cannot open file %s.\n",
	"\"%s\" is not a valid field separator.\n",
	"Invalid invocation argument \"%s\".\n",
};
int	lasterrmsg = sizeof( errmsgs )/sizeof( char * );

