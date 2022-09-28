/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/from822ad.c	1.3.3.1"
#include <stdio.h>
#include "s_string.h"


/*
 *	convert domain to `bang' format. 
 *	   @x,@y:d%c%b@a ->	x!y!a!b!c!d
 *	   d%c%b@a ->	a!b!c!d
 *	   c!d%b@a ->	a!b!uucp!c!d
 */
extern char *
convertaddr(from)
char *from;
{
	static string *buf;
	char *sp, *ep;
	int end;
	int elems=0;

	if(!buf)
		buf = s_new();
	s_restart(buf);

	/*
	 * parse leading @a,@b,@c:
	 */
	while(*from=='@'){
		/* find end of string */
		for(ep= ++from; *ep!=':' && *ep!=',' && *ep!='\0'; ep++)
			;
		end = *ep;
		*ep = '\0';
		s_append(buf, from);
		s_append(buf, "!");
		elems++;
		from = end ? ep+1 : ep;
	}

	/*
	 *  parse the rest (whatever it may be)
	 */
	for (sp = from + strlen(from); sp >= from; sp--) {
		if (*sp == '@' || *sp == '%') {
			/* hack to get rid of forwarding crap */
			s_append(buf, sp+1);
			s_append(buf, "!");
			elems++;
			*sp = '\0';
		}
	}

	/*
	 *  if the from address was a '!' format address, remember that
	 */
	if(elems && (sp=strchr(from, '!')) && (ep=strchr(from, '.')) && ep<sp)
		s_append(buf, "uucp!");
	s_append(buf, from);

	return s_to_c(buf);
}
