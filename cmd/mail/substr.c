/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:substr.c	1.4.3.1"
#include "mail.h"
/*
	This routine looks for string2 in string1.
	If found, it returns the position string2 is found at,
	otherwise it returns a -1.
*/
int
substr(string1, string2)
char *string1, *string2;
{
	register int i,j, len1, len2;

	len1 = strlen(string1);
	len2 = strlen(string2);
	for (i = 0; i < len1 - len2 + 1; i++) {
		for (j = 0; j < len2 && string1[i+j] == string2[j]; j++);
		if (j == len2) return(i);
	}
	return(-1);
}
