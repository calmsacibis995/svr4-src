/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:sys/strappend.c	1.1"

#include	<stdio.h>
#include	"wish.h"
#include	"var_arrays.h"

/*
 * add string to the end of a var_array string
 */
struct v_array *
array_strappend(array, string)
struct v_array	array[];
char	*string;
{
	register struct v_array	*ptr;

	ptr = v_header(array_grow(array, strlen(string)));
	if (string != NULL)
		strcat(v_body(ptr), string);
	return v_body(ptr);
}

/*
 * do the same with a character
 */
struct v_array *
array_chappend(array, ch)
struct v_array	array[];
char	ch;
{
	register char	*cp;
	register struct v_array	*ptr;

	ptr = v_header(array_grow(array, 1));
	cp = ((char *) v_body(ptr)) + strlen(v_body(ptr));
	*cp++ = ch;
	*cp = '\0';
	return v_body(ptr);
}
