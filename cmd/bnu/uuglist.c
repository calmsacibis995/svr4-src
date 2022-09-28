/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:uuglist.c	1.6.4.1"

#include	"uucp.h"

#define MAXLENGTH 256
#define C_MAX	  512

static void insert();
void rproc(), uproc();

static char  Nnament[MAXLENGTH][NAMESIZE];
static char *Nptr[MAXLENGTH];
static short Nnames = 0;

main(argc, argv)
int argc;
char **argv;
{
	int c, i, uopt = 0;
	char prev[2 * NAMESIZE];

	if (eaccess(GRADES, 04) == -1) {
		(void) fprintf(stderr, "No administrator defined service grades available on this machine, use single letter/number only\n");
		exit(0);
	}

	while ((c = getopt(argc, argv, "x:u")) != EOF)
		switch(c) {
		case 'u':
			uopt++;
			break;
		case 'x':
			Debug = atoi(optarg);
			if (Debug < 0)
				Debug = 1;
			break;
		default:
			(void) fprintf(stderr, "usage: uuglist [-u] [-xLEVEL]\n");
			exit(-1);
		}

	if (uopt) {
		Uid = getuid();

		if (Uid == 0)
			(void) setuid(UUCPUID);

		(void) guinfo(Uid, User);

		uproc();
	} else
		rproc();

	for (i = 0; i < Nnames; i++) {

		if (EQUALS(Nptr[i], prev))
			continue;

		puts(Nptr[i]);
		(void) strcpy(prev, Nptr[i]);
	}
	exit(0);
}
static void
insert(name)
char *name;
{
	register i,j;
	register char *p;

	DEBUG(7, "insert(%s) ", name);

	for (i = Nnames; i > 0; i--)
		if (strcmp(name, Nptr[i-1]) > 0)
			break;

	if (i == MAXLENGTH)
		return;

	if (Nnames == MAXLENGTH)
		p = strcpy(Nptr[--Nnames], name);
	else
		p = strcpy(Nnament[Nnames], name);

	for (j = Nnames; j > i; j--)
		Nptr[j] = Nptr[j-1];

	DEBUG(7, "insert %s ", p);
	DEBUG(7, "at %d\n", i);
	Nptr[i] = p;
	Nnames++;
	return;
}

void
rproc()
{
	FILE *cfd;
	char line[BUFSIZ];
	char *carray[C_MAX];
	int na;

	cfd = fopen(GRADES, "r");

	while (rdfulline(cfd, line, BUFSIZ) != 0) {

		na = getargs(line, carray, C_MAX);
		insert(carray[0]);
	}

	(void) fclose(cfd);
	return;
}

void
uproc()
{
	FILE *cfd;
	char line[BUFSIZ];
	char *carray[C_MAX];
	int na;

	cfd = fopen(GRADES, "r");

	while (rdfulline(cfd, line, BUFSIZ) != 0) {

		na = getargs(line, carray, C_MAX);
		
		if (upermit(carray, na) != FAIL)
			insert(carray[0]);
	}

	(void) fclose(cfd);
	return;
}

int Dfileused = FALSE;
wfcommit() {}
void cleanup() {}
gnamef() {}
gdirf() {}
cklock() {}

/*VARARGS*/
/*ARGSUSED*/
void
assert (s1, s2, i1, s3, i2)
char *s1, *s2, *s3;
int i1, i2;
{ }		/* for ASSERT in gnamef.c */

/*VARARGS*/
/*ARGSUSED*/
void
errent(s1, s2, i1, file, line)
char *s1, *s2, *file;
{ }
