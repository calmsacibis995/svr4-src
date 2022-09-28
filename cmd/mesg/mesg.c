/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)mesg:mesg.c	1.6"
/*
 * mesg -- set current tty to accept or
 *	forbid write permission.
 *
 *	mesg [-y] [-n]
 *		y allow messages
 *		n forbid messages
 *	return codes
 *		0 if messages are ON or turned ON
 *		1 if messages are OFF or turned OFF
 *		2 if usage error
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

struct stat sbuf;

char *tty;
char *ttyname();

main(argc, argv)
char *argv[];
{
	int i, c, r=0, errflag=0;
	extern int optind;

	for(i = 0; i <= 2; i++) {
		if ((tty = ttyname(i)) != NULL)
			break;
	}
	if (stat(tty, &sbuf) < 0)
		error("cannot stat");
	if (argc < 2) {
		if (sbuf.st_mode & (S_IWGRP|S_IWOTH))
			printf("is y\n");
		else  {
			r = 1;
			printf("is n\n");
		}
	}
	while ((c = getopt(argc, argv, "yn")) != EOF) {
		switch (c){
		case 'y':
			newmode(S_IRUSR|S_IWUSR|S_IWGRP);
			break;
		case 'n':
			newmode(S_IRUSR|S_IWUSR);
			r = 1;
			break;
		case '?':
			errflag++;
		}
	}

	if (errflag /*  || (argc > optind) */ )
		error("usage: mesg [-y] [-n]");

/* added for temporary compat. */
	if(argc > optind) switch(*argv[optind]) {
		case 'y':
			newmode(S_IRUSR|S_IWUSR|S_IWGRP);
			break;
		case 'n':
			newmode(S_IRUSR|S_IWUSR);
			r = 1;
			break;
		default:
			errflag++;
		}

	if (errflag)
		error("usage: mesg [-y] [-n]");
/* added to here */
	exit(r);
}

error(s)
char *s;
{
	fprintf(stderr, "mesg: %s\n", s);
	exit(2);
}

newmode(m)
mode_t m;
{
	if (chmod(tty, m) < 0)
		error("cannot change mode");
}
