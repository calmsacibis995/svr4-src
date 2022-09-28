/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:smtp/src/fromsmtp.c	1.4.3.1"
#include <stdio.h>
#include <ctype.h>
#include "xmail.h"
#include "s_string.h"
#include "header.h"
#include "aux.h"

/* imports */
extern char *strcpy();
extern FILE *popen();
extern char *lowercase();
extern void exit();

/* predeclared */
char *lowercase();

main(ac, av)
int ac;
char *av[];
{
	string *cmd = s_new();
	FILE *fp;
	string *cva = s_new();
	int c, debug = 0;
	char *host = 0;
	char *defaultsender = NULL;
	extern char *optarg;
	extern int optind;

	while ((c = getopt(ac, av, "s:dh:")) != EOF) {
		switch (c) {
		case 'h':
			host=optarg;
			break;
		case 's':
			defaultsender=optarg;
			break;
		case 'd':
			debug++;
			break;
		default:
			usage();
		}
	}
	if (optind >= ac) {
		int i;
		for(i=0; i<ac; i++)
			fprintf(stderr, "%s ", av[i]);
		fprintf(stderr, "\n");
		usage();
	}

	s_append(cva, lowercase(av[optind++]));
	for (; optind < ac; optind++) {
		s_append(cva, " ");
		s_append(cva, lowercase(av[optind]));
	}
	/* start up the mailer and pipe mail into it */
#ifdef SVR4
	if (debug)
		s_append(cmd, "/usr/bin/echo ");
	s_append(cmd, "exec /usr/bin/rmail ");
	s_append(cmd, s_to_c(cva));
	if (debug)
		s_append(cmd, ";/usr/bin/cat ");
#else
	if (debug)
		s_append(cmd, "/bin/echo ");
	s_append(cmd, "exec /bin/rmail ");
	s_append(cmd, s_to_c(cva));
	if (debug)
		s_append(cmd, ";/bin/cat ");
#endif
	fp = (FILE *)popen(s_to_c(cmd), "w");
	if (fp == NULL)
		exit(1);
	from822(host, fgets, stdin, fp, defaultsender);	

	/* return any errors */
	if (pclose(fp))
		return 1;
	return 0;
}

/*
 *	Convert a string to lower case.
 */
char *
lowercase(sp)
char *sp;
{
	register char *lp = sp;

	while(*lp) {
		if(isupper(*lp))
			*lp = tolower(*lp);
		lp++;
	}
	return sp;
}

usage()
{
	fputs("usage: fromsmtp [-h host or net] [-s sender] address-list\n", stderr);
	exit(1);
}
