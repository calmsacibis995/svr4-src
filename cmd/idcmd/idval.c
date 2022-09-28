/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)idcmd:idval.c	1.1"

/* This program supports the idtune command. The idtune shell script
 * used to rely on 'expr' to compare the new tunable parameter value
 * against its lower and upper bound. Both decimal and hexadecimal
 * values can be specified but expr would give incorrect return value 
 * when comparing a decimal with a hex value. Therefore, this supporting
 * C program was written to do the appropriate conversions.
 */
#include <stdio.h>

#define	USAGE	"Usage: compare -l | -g value1 value2\n"
#define VALUES	"compare: must specify two values to compare\n"

char gflag = 0;		/* -g flag specified, is first value greater than second? */
char lflag = 0;		/* -l flag specified, is first value less than second? */
char value1[128];	/* char string for the first value */
char value2[128];	/* char string for the second value */

void error();
int lessthan();		/* is value1 less than lower bound (value2)? */
int greaterthan();	/* is value1 greater than upper bound (value2)? */

/* exit codes as follows:
 *		1: TRUE		(value1 is greater (or less) than value2.
 *		0: FALSE	(value1 is within upper (or lower) bound.
 *		-1: failure
 */

main(argc, argv)
int argc;
char **argv;
{
	int m;
	int ret;	/* returned value from comparisons */
	long lvalue1,
	     lvalue2;	/* returned values from strtol */
	extern int optind;
	long strtol();  /* does string to long integer conversion */

	while ((m = getopt(argc, argv, "?gl")) != EOF)
		switch(m) {

			case 'g':
				gflag++;
				break;

			case 'l':
				lflag++;
				break;

			case '?':
				error(USAGE);
		}

		if (lflag && gflag) {
			fprintf(stderr, " compare: must have one of -g or -l options\n");
			error(USAGE);
		}

		if (argc == optind) {
			fprintf(stderr, "compare: must specify two values to compare\n");
			error (USAGE);
		}

		sprintf(value1, "%s", argv[optind]);

		if (*value1 == '\0') {
			fprintf(stderr, VALUES);
			error(USAGE);
		}

		sprintf(value2, "%s", argv[++optind]);

		if (*value2 == '\0') {
			fprintf(stderr, VALUES);
			error(USAGE);
		}
		lvalue1 = strtol(value1, (char **)NULL, 0);
		lvalue2 = strtol(value2, (char **)NULL, 0);

		if (lflag)
			ret = lessthan(lvalue1, lvalue2);

		if (gflag)
			ret = greaterthan(lvalue1, lvalue2);
		exit(ret);
}


void error(s)
char *s;
{
	fprintf(stderr, "%s", s);
	exit(-1);
}

lessthan(val1, val2)
long val1, val2;
{
	if (val1 < val2)
		return(1);
	else
		return(0);
}


greaterthan(val1, val2)
long val1, val2;
{
	if (val1 > val2)
		return(1);
	else
		return(0);
}
