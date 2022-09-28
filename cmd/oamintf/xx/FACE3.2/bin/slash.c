/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:xx/FACE3.2/bin/slash.c	1.1"
#include <stdio.h>

extern char *optarg;
extern int optind;

main(argc, argv)
int argc;
char *argv[];
{
	register int c, fromstr;
	register char *sptr;
	FILE *fp, *fopen();
	char *ifile;
	int errflag;

	errflag = 0;	
	ifile = NULL;
	while((c = getopt(argc, argv, "f:")) != EOF) {
		switch(c) {
		case 'f':
			ifile = optarg;
			break;
		case '?':
			errflag++;
			break;
		}
	}

	if (errflag)
		exit(1);
	if (ifile) {				/* from a file */
		if ((fp = fopen(ifile, "r")) == NULL)
			exit(1);
		fromstr = 0;
	}
	else if (optind == argc) { 		/* from stdin */
		fp = stdin;
		fromstr = 0;
	}
	else {					/* from a string */
		fromstr = 1;
		sptr = argv[1];
	}

	for (; ;) {
		if (fromstr)
			c = *sptr++;
		else
			c = fgetc(fp);
		switch(c) {
		case '\\':
			printf("\\\\");
			break;
		case '$':
			printf("\\$");
			break;
		case '`':
			printf("\\`");
			break;
		case '\'':
			printf("\\'");
			break;
		case '"':
			printf("\\\"");
			break;
		case '&':
			printf("\\&");
			break;
		case '[':
			printf("\\[");
			break;
		case ']':
			printf("\\]");
			break;
		case '<':
			printf("\\<");
			break;
		case '>':
			printf("\\>");
			break;
		case ';':
			printf("\\;");
			break;
		case '\0':
		case EOF:
			exit(0);
		default:
			putchar(c);
			break;
		}
	}
}

