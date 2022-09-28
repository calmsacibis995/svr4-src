/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)pwd:pwd.c	1.15"
/*
**	Print working (current) directory
*/

#include	<stdio.h>
#include	<unistd.h>
#include	<limits.h>
char	name[PATH_MAX+1];

main()
{
	int length;
	if (getcwd(name, PATH_MAX + 1) == (char *)0) {
		fprintf(stderr, "pwd: cannot determine current directory!\n");
		exit(2);
	}
	length = strlen(name);
	name[length] = '\n';
	write(1, name, length + 1);
	exit(0);
}
