/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bkoper.d/msgs.c	1.4.4.1"

char *errmsgs[] = {
	"Unable to understand this command.\n",
	"There are new backup operations requiring service.\n",
	"No more backup operations are WAITING for operator action at this time.\nType q to quit:\n",
	"Backup operation %d, jobid %s tag %s no longer needs service.\n",
	"Unable to read entry number %d in status table.\n",
	"Unexpected error: %s\n",
	"Unable to open backup status log (%s): %s\n",
	"Unable to open backup status log (%s): %d\n",
	"%s is not a valid user.\n",
	"There are no backup operations WAITING for operator action at this time.\n",
	"Which backup operation do you want to respond to?\nType q to quit from bkoper\nType h to display the list of backup operations\nType a number or RETURN to service a backup operation\n",
	"? ",
	"Current backup operation number: %d\n",
	"Unable to understand this '%c' command.\n",
	"Backup operation %d does not exist.\n",
	"Unable to send volume information to the method: %s\n",
	"Unknown or inaccessible device: %s\n",
	"The volume label, %s, is incorrect.\n",
	""
};
int	lasterrmsg = sizeof( errmsgs )/sizeof( char * );
