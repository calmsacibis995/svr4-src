/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:shlib/chkid.c	1.1.3.1"

/*
 *   NAM_HASH (NAME)
 *
 *        char *NAME;
 *
 *	Return a hash value of the string given by name.
 *      Trial and error has shown this hash function to perform well
 *
 */

#include	"sh_config.h"

int nam_hash(name)
register const char *name;
{
	register int h = *name;
	register int c;
	while(c= *++name)
	{
		if((h = (h>>2) ^ (h<<3) ^ c) < 0)
			h = ~h;
	}
	return (h);
}

