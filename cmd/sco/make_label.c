/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:make_label.c	1.3"

/* #define		DEBUG		1		/* */

/*
		make_label(handle, buffer)

	Creates/Overwrites a volume label for the specified handle.
	Returns -1 on failure.
*/

#include	"MS-DOS.h"

#include	<stdio.h>

make_label(handle, buffer)
char	*buffer;
{
	int	c;

	if (lookup_device(handle) == -1) {
		(void) fprintf(stderr, "make_label(): Error - Handle %d not found in device table\n", handle);
		return(-1);
	}

	/*
		Let's make sure we don't already have a label
	*/
	if (del_label(handle) == -1) {
		(void) fprintf(stderr, "make_label(): Label \"%s\" not created\n", buffer);
		return(-1);
	}

	/*
		Now the user may have just hit <RETURN>.
		If so, then the length of the reply is one.

		In this case the user wants NO LABEL. Since
		we have already deleted the label (if one existed),
		then we just ignore the rest of this code.
	*/
	if (strlen(buffer) < 2)
		return(0);

	/*
		Construct the actual file name
	*/
	buffer[strlen(buffer) - 1] = '\0';

	for (c = strlen(buffer) - 1; c > 7; c--)
		buffer[c + 1] = buffer[c];

	if (c == 7)
		buffer[8] = '.';

	(void) strupr(buffer);

#ifdef DEBUG
	(void) fprintf(stderr, "make_label(): DEBUG - Label with dot = \"%s\"\n", buffer);
#endif

	/*
		Create the label
	*/
	if (Mkdir(handle, buffer, LABEL) == -1) {
		(void) fprintf(stderr, "dosformat: Failed to create label \"%s\"\n", buffer);
		return(-1);
	}


	return(0);
}
