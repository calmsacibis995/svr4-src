/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:dos_mod_date.c	1.3"

/*
		dos_mod_date(displacement)

	Retrieves and converts the file modification date
	from the current sector_buffer, at the passed displacement.
	Returns a pointer to the string.
*/
#include	<stdio.h>

#include	"MS-DOS.h"

char	_dmd_ret_value[9];

char	*
dos_mod_date(i)
int	i;	/* displacement */
{
	/*
		Get Modification Date
	*/
	(void) sprintf(_dmd_ret_value, "%2d-%02d-%02d", (sector_buffer[DATE + i] & 0xF0) >> 5 | (sector_buffer[DATE + i + 1] & 0x01) << 3, (sector_buffer[DATE + i] & 0x1F), ((sector_buffer[DATE + i + 1] & 0x7E) >> 1) + 80);

	return(_dmd_ret_value);
}
