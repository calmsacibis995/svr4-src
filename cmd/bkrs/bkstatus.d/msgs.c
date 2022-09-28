/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkstatus.d/msgs.c	1.5.2.1"

char *errmsgs[] = {
	"Option \"%c\" is invalid.\n",
	"Argument \"%s\" is invalid.\n",
	"No other options may be specified with the \"%c\" option.\n",
	"The \"%c\" and \"%c\" options are mutually exclusive.\n",
	"Warning: \"%s\" is not a valid field separator, using default.\n",
	"Unable to insert ROTATION comment into %s (return code = %d).\n",
	"Open of table %s failed (return code = %d).\n",
	"Unable to read table entry number %d (return code = %d).\n",
	"Unable to allocate memory for login-to-userid conversion.\n",
	"Warning: table %s has different format than expected.\n",
	"Warning: invalid argument %s ignored.\n",
	"Warning: invalid state %c ignored.\n",
	"All option arguments are invalid.\n",
	"Illegal state character \"%c\" encountered.\n",
	"Unable to allocate memory for table entry.\n",
	"Table file %s does not exist or is not accessible.\n",
	"Period value must be at least 1 and not greater than %d.\n",
	"Unable to assign status field value in table entry %d (return code = %d).\n",
};
int	lasterrmsg = sizeof( errmsgs )/sizeof( char * );
