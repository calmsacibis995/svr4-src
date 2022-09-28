/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:uu_copy.c	1.3"

/* #define		DEBUG		1	/* */

/*
		uu_copy(handle, displacement, conversion, output_fptr)

	Pass this routine the displacement within buffer
	of a directory entry for an MS-DOS file, and it will open 
	the file and write it to output_fptr. Return -1 on error.
*/

#include	"MS-DOS.h"

#include	<stdio.h>

uu_copy(handle, conversion, output)
int	handle;
int	conversion;
FILE	*output;
{
	char	buffer[1024];
	int	bytes_read;
	int	last_was_cr = 0;
	int	work_int;

#ifdef DEBUG
	(void) fprintf(stderr, "uu_copy(): DEBUG - Entered uu_copy()  handle: %d conversion: %d\n", handle, conversion);
#endif

	while ((bytes_read = read(handle, buffer, 1024)) > 0) {
#ifdef DEBUG
		(void) fprintf(stderr, "uu_copy(): DEBUG - Read %d bytes from UNIX input file\n", bytes_read);
#endif
		for (work_int = 0; work_int < bytes_read; work_int++) {
			if (conversion == 1 && buffer[work_int] == '\r') {
				last_was_cr = 1;
				continue;
			}

			if (last_was_cr && buffer[work_int] != '\n')
				(void) fprintf(output, "\r");

			(void) fprintf(output, "%c", buffer[work_int]);

			last_was_cr = 0;
		}
	}
}
