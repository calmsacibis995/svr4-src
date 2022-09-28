/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkreg.d/msgs.c	1.7.3.1"

char *errmsgs[] = {
	"Option \"%c\" is invalid.\n",
	"Argument \"%s\" is invalid.\n",
	"All option arguments are invalid.\n",
	"\"%s\" is an invalid argument for the %c option.\n",
	"Could not save table %s (return code %d).\n",
	"Cannot allocate memory.\n",
	"Could not read ROTATION period in %s (return code %d).\n",
	"Could not read ROTATION STARTED in %s (return code %d).\n",
	"Could not read table entry number %d (return code %d).\n",
	"Unable to add this backup.\n",
	"Cannot open table %s (return code %d)\n",
	"Tag %s already exists in table %s.\n",
	"Tag %s does not exist in table %s.\n",
	"Unable to assign value \"%s\" to entry number %d (return code %d).\n",
	"Unable to remove entry %d (return code %d).\n",
	"Unable to insert %s comment into table %s (return code %d).\n",
	"Unable to sort table %s on originating device field, errno = %d.\n",
	"Warning: unable to remove temporary file %s, errno = %d.\n",
	"The %c argument must be greater than 0 and less than or equal to %d.\n",
	"Search of table %s failed (return code %d).\n",
	"Cannot open table %s (errno = %d)\n",
	"Could not save table %s (errno = %d).\n",
        "Warning: All the week(s) of backup should be less than ROTATION period = %d.\n",
};
int	lasterrmsg = sizeof( errmsgs )/sizeof( char * );
