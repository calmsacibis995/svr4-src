/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:cat.c	1.3.3.1"
/*
    NAME
	cat - concatenate two strings

    SYNOPSIS
	void cat(char *to, char *from1, char *from2)

    DESCRIPTION
	cat() concatenates "from1" and "from2" to "to"
		to	-> destination string
		from1	-> source string
		from2	-> source string
*/
#include "mail.h"
void
cat(to, from1, from2)
register char *to, *from1, *from2;
{
	for (; *from1;) *to++ = *from1++;
	for (; *from2;) *to++ = *from2++;
	*to = '\0';
}
