/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:disp_vol.c	1.3"

/*
			disp_vol(handle, directory)

	Dump volume label in MS-DOS format for the passed handle.
	Use the passed directory for the message regarding current
	directory.
*/

#include	"MS-DOS.h"

#include	<stdio.h>

int
disp_vol(handle, directory)
int	handle;
char	*directory;
{
	int	i;
	int	index;
	char	work_string[MAX_FILENAME];

	/*
		Ensure we initialized this handle
	*/
	if ((index = lookup_device(handle)) == -1) {
		(void) fprintf(stderr, "disp_vol(): Error - Handle %d not found\n", handle);
		return(-1);
	}

	/*
		Load up the first portion of the root directrory
	*/
	if (read_sector(handle, TABLE.root_base_sector) == -1) {
		(void) fprintf(stderr, "disp_vol(): Error - Failed to read root-base_sector (%ld)\n");
		return(-1);
	}

	(void) printf(" Volume in drive %c:  ", drive);

	if (get_label(handle) == -1)
		(void) printf("has no label\n");
	else 
		(void) printf("is %s\n", volume_label);

	/*
		Change all forward slashes to back slashes for MS-DOS
	*/
	(void) strcpy(work_string, directory);

	for (i = 0; work_string[i] != '\0'; i++)
		if (work_string[i] == '/')
			work_string[i] = '\\';

	(void) printf(" Directory of  %c:%s\n\n", drive, work_string);

	return(0);
}
