/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamuser:lib/putgrent.c	1.3.7.1"



#include <stdio.h>
#include <grp.h>
#include <unistd.h>

void
putgrent(grpstr, to)
struct group *grpstr;	/* group structure to write */
FILE *to;		/* file to write to */
{
	register char **memptr;		/* member vector pointer */

	(void) fprintf( to, "%s:%s:%ld:", grpstr->gr_name, grpstr->gr_passwd, 
		grpstr->gr_gid);

	memptr = grpstr->gr_mem;

	while( *memptr != NULL ) {
		(void) fprintf( to, "%s", *memptr );
		memptr++;
		if( *memptr != NULL) (void) fprintf( to, "," );
	}

	(void) fprintf( to, "\n" );
}
