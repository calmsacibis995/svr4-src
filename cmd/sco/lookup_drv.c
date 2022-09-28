/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:lookup_drv.c	1.3"

/*
		lookup_drive(handle)

	This routine returns the index into the hardware table
	for the passed drive spcification.

	Returns index on success, -1 on error.
*/

#include	"MS-DOS.h"

#include	<stdio.h>

lookup_drive(local_drive)
char	local_drive;
{
	int	index;

	for (index = 0; HARDWARE.device_letter != '\0' && index < MAX_DEVICES; index++)
		if (HARDWARE.device_letter == local_drive)
			break;

	if (index == MAX_DEVICES) {
		(void) fprintf(stderr, "lookup_drive(): Error Drive '%c' invalid\n", local_drive);
		return(-1);
	}

	return(index);
}
