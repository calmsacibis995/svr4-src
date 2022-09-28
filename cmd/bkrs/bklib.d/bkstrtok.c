/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:bklib.d/bkstrtok.c	1.1.2.1"

#include <sys/param.h>
#include <sys/types.h>

static char *cont_ptr;

char *
bkstrtok(st, dlm)
char *st;
char *dlm;
{
	char *strchr(), *ret;

	if(st)			/* new string */
		cont_ptr = st;

	if(!cont_ptr)		/* NULL pointer */
		return(NULL);

	ret = cont_ptr;

	cont_ptr = strchr(cont_ptr, *dlm);

	if( cont_ptr) {		/* found another dlm */
		*cont_ptr++ = 0;	/* end this string */
	}

	return(ret);			/* was dgroup */

}
