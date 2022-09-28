/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sleep:sleep.c	1.4"

/*
**	sleep -- suspend execution for an interval
**
**		sleep time
*/

#include	<stdio.h>

main(argc, argv)
char **argv;
{
	int	c, n;
	char	*s;

	n = 0;
	if(argc < 2) {
		fprintf(stderr, "usage: sleep time\n");
		exit(2);
	}
	s = argv[1];
	while(c = *s++) {
		if(c<'0' || c>'9') {
			fprintf(stderr, "sleep: bad character in argument\n");
			exit(2);
		}
		n = n*10 + c - '0';
	}
	sleep(n);
}
