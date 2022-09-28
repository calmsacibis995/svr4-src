/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)spell:hashmake.c	1.3"

#include "hash.h"

main()
{
	char word[30];
	long h;
	hashinit();
	while(gets(word)) {
		printf("%.*lo\n",(HASHWIDTH+2)/3,hash(word));
	}
	return(0);
}
