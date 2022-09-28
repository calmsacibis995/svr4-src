/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)make:prtmem.c	1.4"
/*	@(#)make:prtmem.c	1.3 of 11/22/85	*/

#ifdef GETU

#include <stdio.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#define udsize uu[0]
#define ussize uu[1]

prtmem()		/* print memory */
{
	unsigned	uu[2];

	if (getu( &((struct user *)NULL)->u_dsize, &uu, sizeof uu) > 0) {
		register int i;

		udsize *= 64;
		ussize *= 64;
		printf("mem: data = %u(0%o) stack = %u(0%o)\n",
		    udsize, udsize, ussize, ussize);
/*
 *	The following works only when `make' is compiled
 *	with I&D space separated (i.e. cc -i ...).
 *	(Notice the hard coded `65' below!)
 */
		udsize /= 1000;
		ussize /= 1000;
		printf("mem:");
		for (i = 1; i <= udsize; i++) {
			if ( !(i % 10) )
				printf("___");
			printf("d");
		}
		for (; i <= 65 - ussize; i++) {
			if ( !(i % 10) )
				printf("___");
			printf(".");
		}
		for (; i <= 65; i++) {
			if ( !(i % 10) )
				printf("___");
			printf("s");
		}
		printf("\n");
		(void)fflush(stdout);
	}
}
#endif
