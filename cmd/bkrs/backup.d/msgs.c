/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:backup.d/msgs.c	1.6.2.1"

char *errmsgs[] = {
	"option \"%c\" is invalid.\n",
	"argument \"%s\" is invalid.\n",
	"\"%s\" is not a valid user name.\n",
	"All option arguments are invalid.\n",
	"\"%s\" is an invalid argument for the %c option.\n",
	"File name \"%s\" is too long.\n",
	"A week or day range may not be specified with the \"%c\" option.\n",
	"Cannot use both the \"%c\" and \"%c\" options with the \"%c\" option.\n",
	"Backup daemon process has unexpectedly terminated - trying to restart this backup\n",
	"Backup daemon process has unexpectedly terminated - cannot %s those backups.\n",
	"Cannot keep backup daemon process alive - exiting.\n",
	"The backup jod id is back-%ld\n",
	"Unable to get current working directory\n",
	"Path name for file %s/%s is too long\n",
	"Cannot access %s\n",
	"Cannot RESUME/CANCEL/SUSPEND someone else's backups.\n",
	"Unable to invoke bkoper: %s\n",
	"Unable to open backup register %s - %s.\n",
	"Table has no rotation period.\n",
	"Rotation not found, set to 1.\n",
	"Rotation %d greater than %d, set to %d.\n",
	"No ROTATION STARTED in register table.\n",
	"Tag %s has invalid week/day specified.\n",
	"Tag: %s specified priority %s illegal, set to %d.\n",
	"Tag: %s tag has appeared previously, first used.\n",
	"Tag: %s week %d day %d illegal combination.\n",
	"Tag: %s  oname: %s  odevice: %s\n\tdup backups as follows:\n\t%s .\n",
	"TLread of bkreg returned %d - %s .\n",
	"Tag: %s depends on nonexistent tag %s .\n",
	"Tag: %s cannot execute due to dependency conflicts amoung:\n\t",
	"%s .\n",
	"No valid entries in table. \n",
	"There are no backup operations to %s.\n",
	""
};
int	lasterrmsg = sizeof( errmsgs )/sizeof( char * );
