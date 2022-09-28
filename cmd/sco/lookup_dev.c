/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:lookup_dev.c	1.3"

/*
			lookup_device(handle)

	Search for an entry in our device table using
	the passed handle. Return its index - if found.
	Return -1 on failure.
*/
#include	"MS-DOS.h"

int
lookup_device(handle)
int	handle;
{
	int	index;

	/*
		Look for our specified handle.
	*/
	for (index = 0; index < MAX_DEVICES; index++)
		if (TABLE.handle == handle)
			break;

	/*
		If we reached end of our table, no match
	*/
	if (index == MAX_DEVICES)
		index = -1;

	return(index);
}
