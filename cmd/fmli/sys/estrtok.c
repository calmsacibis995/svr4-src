/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:sys/estrtok.c	1.1"

#include	<stdio.h>
#include	<string.h>

char *
estrtok(env, ptr, sep)
char	**env;
char	*ptr;
char	sep[];
{
	if (ptr == NULL)
		ptr = *env;
	else
		*env = ptr;
	if (ptr == NULL || *ptr == '\0')
		return NULL;
	ptr += strspn(ptr, sep);
	*env = ptr + strcspn(ptr, sep);
	if (**env != '\0') {
		**env = '\0';
		(*env)++;
	}
	return ptr;
}
