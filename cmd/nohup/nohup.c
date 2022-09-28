/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)nohup:nohup.c	1.6"

#include	<stdio.h>

char	nout[100] = "nohup.out";
char	*getenv();
extern	char	*sys_errlist[];
extern	int	errno;

main(argc, argv)
char **argv;
{
	char	*home;
	FILE *temp;
	int	err;

	if(argc < 2) {
		fprintf(stderr,"usage: nohup command arg ...\n");
		exit(1);
	}
	argv[argc] = 0;
	signal(1, 1);
	signal(3, 1);
	if(isatty(1)) {
		if( (temp = fopen(nout, "a")) == NULL) {
			if((home=getenv("HOME")) == NULL) {
				fprintf(stderr,"nohup: cannot open/create nohup.out\n");
				exit(1);
			}
			strcpy(nout,home);
			strcat(nout,"/nohup.out");
			if(freopen(nout, "a", stdout) == NULL) {
				fprintf(stderr,"nohup: cannot open/create nohup.out\n");
				exit(1);
			}
		}
		else {
			fclose(temp);
			freopen(nout, "a", stdout);
		}
		fprintf(stderr, "Sending output to %s\n", nout);
	}
	if(isatty(2)) {
		close(2);
		dup(1);
	}
	execvp(argv[1], &argv[1]);
	err = errno;

/* It failed, so print an error */
	freopen("/dev/tty", "w", stderr);
	fprintf(stderr,"%s: %s: %s\n", argv[0], argv[1], sys_errlist[err]);
	exit(1);
}
