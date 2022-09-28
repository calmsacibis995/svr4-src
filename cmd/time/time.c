/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)time:time.c	1.7"
/*
**	Time a command
*/

#include	<stdio.h>
#include	<signal.h>
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/times.h>
#include	<sys/param.h>		/* HZ defined here */
#include	<unistd.h>

main(argc, argv)
char **argv;
{
	struct tms buffer;
	register pid_t p;
	extern	errno;
	extern	char	*sys_errlist[];
	int	status;
	clock_t	before, after;

	before = times(&buffer);
	if(argc<=1)
		exit(0);
	p = fork();
	if(p == (pid_t)-1) {
		fprintf(stderr,"time: cannot fork -- try again.\n");
		exit(2);
	}
	if(p == (pid_t)0) {
/*		close(1);	lem commented this out	*/
		execvp(argv[1], &argv[1]);
	        fprintf(stderr, "%s: %s\n", sys_errlist[errno], argv[1]);
		exit(2);
	}
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	while(wait(&status) != p);
	if((status & 0377) != '\0')
		fprintf(stderr,"time: command terminated abnormally.\n");
	after = times(&buffer);
	fprintf(stderr,"\n");
	printt("real", (after-before));
	printt("user", buffer.tms_cutime);
	printt("sys ", buffer.tms_cstime);
	exit(status >> 8);
}

/*
The following use of HZ/10 will work correctly only if HZ is a multiple
of 10.  However the only values for HZ now in use are 100 for the 3B
and 60 for other machines.
*/
char quant[] = { HZ/10, 10, 10, 6, 10, 6, 10, 10, 10 };
char *pad  = "000      ";
char *sep  = "\0\0.\0:\0:\0\0";
char *nsep = "\0\0.\0 \0 \0\0";

printt(s, a)
char *s;
clock_t a;
{
	register i;
	char	digit[9];
	char	c;
	int	nonzero;

	for(i=0; i<9; i++) {
		digit[i] = a % quant[i];
		a /= (clock_t)quant[i];
	}
	fprintf(stderr,s);
	nonzero = 0;
	while(--i>0) {
		c = digit[i]!=0 ? digit[i]+'0':
		    nonzero ? '0':
		    pad[i];
		if (c != '\0')
			putc (c, stderr);
		nonzero |= digit[i];
		c = nonzero?sep[i]:nsep[i];
		if (c != '\0')
			putc (c, stderr);
	}
	fprintf(stderr,"\n");
}
