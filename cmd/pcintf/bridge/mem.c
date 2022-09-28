/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/mem.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)mem.c	3.3	LCC);	/* Modified: 13:15:32 7/14/87 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

#include	<stdio.h>
#include	"const.h"


extern char
	*realloc(),			/* Standard C memory reallocator*/
	*malloc();			/* Standard C memory allocator	*/
extern long	ulimit();
	

char *
memory(amount)
register int	amount;
{
    register char	
	*newmem;		/* Newly allocated memory	*/

    if ((newmem = malloc((unsigned) amount)) == NULL)
	fatal("memory: Can't get %d bytes\n", amount);

    return newmem;
}



char *
morememory(ptr, amount)
char
	*ptr;
register int
	amount;
{
register char	
	*newmem;		/* Newly allocated memory	*/

	if ((newmem = realloc(ptr, (unsigned) amount)) == NULL)
		fatal("memory: Can't resize to %d\n", amount);

	return newmem;
}
