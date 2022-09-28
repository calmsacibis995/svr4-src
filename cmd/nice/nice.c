/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)nice:nice.c	1.7"
/*
**	nice
*/


#include	<stdio.h>
#include	<ctype.h>
#include	<sys/errno.h>

main(argc, argv)
int argc;
char *argv[];
{
	int	nicarg = 10;
	extern	errno;
	extern	char *sys_errlist[];

	if(argc > 1 && argv[1][0] == '-') {
		register char	*p = argv[1];

		if(*++p != '-') {
			--p;
		}
		while(*++p)
			if(!isdigit(*p)) {
				fprintf(stderr, "nice: argument must be numeric.\n");
				exit(2);
			}
		nicarg = atoi(&argv[1][1]);
		argc--;
		argv++;
	}
	if(argc < 2) {
		fprintf(stderr, "nice: usage: nice [-num] command\n");
		exit(2);
	}
	errno = 0;
	if (nice(nicarg) == -1) {
		/*
		 * Could be an error or a legitimate return value.
		 * The only error we care about is EINVAL, which will
		 * be returned if we are not in the time sharing
		 * scheduling class.  For any error other than EINVAL
		 * we will go ahead and exec the command even though
		 * the priority change failed.
		 */
		if (errno == EINVAL) {
			fprintf(stderr, "nice: invalid operation; \
not a time sharing process\n");
			exit(2);
		}
	}
	execvp(argv[1], &argv[1]);
	fprintf(stderr, "%s: %s\n", sys_errlist[errno], argv[1]);
	exit(2);
}
