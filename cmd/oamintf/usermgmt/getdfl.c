/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:usermgmt/getdfl.c	1.1.2.2"
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <deflt.h>
#include <stdlib.h>


int	mindate, maxdate, warndate, idledate;
int	nflg, xflg, wflg, iflg, tflg;
main(argc, argv)
int argc;
char **argv;
{
	extern char *optarg;
	extern int optind;
	char *char_p;
	register int c, flag = 0;

	char_p = NULL;
	while ((c = getopt(argc, argv, "xnwit")) != EOF) {
		switch (c) {
		case 'x':		/* delet the password */
			xflg = 1;
			break;
		case 'n':
			nflg = 1;
			break;
		case 'w':
			wflg = 1;
			break;
		case 'i':
			iflg = 1;
			break;
		case 't':
			tflg = 1;
			break;
		}
	}
	argv = &argv[optind];
        if( (defopen(*argv)) != 0) {
        	defopen(NULL);                  /* close defaults file */
		exit;
	}
	if(xflg) {
		/*
		 * char_p == NULL when MAXWEEKS note in the file and
		 * *char_p == '\0' when no value assigned to MAXWEEKS.
		 */
		if( (char_p=defread("MAXWEEKS=")) != NULL && *char_p != '\0') {
			if ((maxdate = atoi(char_p)) == -1) {
				maxdate = -1;
			} else {
				maxdate *= 7;
			}
			printf("%d",maxdate);
		}
	} else if (nflg) {
		if( (char_p=defread("MINWEEKS=")) != NULL && *char_p != '\0') {
			mindate = 7 * atoi(char_p);
			if (mindate >= 0)
				printf("%d",mindate);
		}
	} else if (wflg) {
		if( (char_p=defread("WARNWEEKS=")) != NULL && *char_p != '\0') {
			warndate = 7 * atoi(char_p);
			if (warndate >= 0)
				printf("%d",warndate);
		}
	} else if (iflg) {
		if( (char_p=defread("IDLEWEEKS=")) != NULL && *char_p != '\0') {
			idledate = 7 * atoi(char_p);
			if (idledate >= 0)
				printf("%d",idledate);
		}
	} else if (tflg) {
		if( (char_p=defread("TIMEZONE=")) != NULL && *char_p != '\0') 
			printf("%s",char_p);
	}
        defopen(NULL);                  /* close defaults file */
}
