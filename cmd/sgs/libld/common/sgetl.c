/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libld:common/sgetl.c	1.5"
/*
 * The intent here is to provide a means to make the value of
 * bytes in an io-buffer correspond to the value of a long
 * in the memory while doing the io a `long' at a time.
 * Files written and read in this way are machine-independent.
 *
 */
#include <values.h>
#include	"synsyms.h"

long
sgetl(buffer)
register const char *buffer;
{
	register long w = 0;
	register int i = BITSPERBYTE * sizeof(long);

	while ((i -= BITSPERBYTE) >= 0)
		w |= (long) ((unsigned char) *buffer++) << i;
	return (w);
}
