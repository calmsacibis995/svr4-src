/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*	Copyright (c) 1988, Sun Microsystems, Inc.	*/
/*	All Rights Reserved.				*/

#ident	"@(#)bnu:uudecode.c	1.2.3.1"
/*
 * uudecode [input]
 *
 * create the specified file, decoding as you go.
 * used with uuencode.
 */
#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

/* single character decode */
#define DEC(c)	(((c) - ' ') & 077)

static void decode(), outdec();
static char *index();

main(argc, argv)
char **argv;
{
	FILE *in, *out;
	mode_t mode;
	char dest[128];
	char buf[80];

	/* optional input arg */
	if (argc > 1) {
		if ((in = fopen(argv[1], "r")) == NULL) {
			perror(argv[1]);
			exit(1);
		}
		argv++; argc--;
	} else
		in = stdin;

	if (argc != 1) {
		(void)printf("Usage: uudecode [infile]\n");
		exit(2);
	}

	/* search for header line */
	for (;;) {
		if (fgets(buf, sizeof buf, in) == NULL) {
			(void)fprintf(stderr, "No begin line\n");
			exit(3);
		}
		if (strncmp(buf, "begin ", 6) == 0)
			break;
	}
	(void)sscanf(buf, "begin %lo %s", &mode, dest);

	/* handle ~user/file format */
	if (dest[0] == '~') {
		char *sl;
		struct passwd *getpwnam();
		struct passwd *user;
		char dnbuf[100];

		sl = index(dest, '/');
		if (sl == NULL) {
			(void)fprintf(stderr, "Illegal ~user\n");
			exit(3);
		}
		*sl++ = 0;
		user = getpwnam(dest+1);
		if (user == NULL) {
			(void)fprintf(stderr, "No such user as %s\n", dest);
			exit(4);
		}
		(void)strcpy(dnbuf, user->pw_dir);
		(void)strcat(dnbuf, "/");
		(void)strcat(dnbuf, sl);
		(void)strcpy(dest, dnbuf);
	}

	/* create output file */
	out = fopen(dest, "w");
	if (out == NULL) {
		perror(dest);
		exit(4);
	}
	(void)chmod(dest, mode);

	decode(in, out);

	if (fgets(buf, sizeof buf, in) == NULL || strcmp(buf, "end\n")) {
		(void)fprintf(stderr, "No end line\n");
		exit(5);
	}
	exit(0);
	/* NOTREACHED */
}

/*
 * copy from in to out, decoding as you go along.
 */
static void
decode(in, out)
FILE *in;
FILE *out;
{
	char buf[80];
	char *bp;
	int n;

	for (;;) {
		/* for each input line */
		if (fgets(buf, sizeof buf, in) == NULL) {
			(void)printf("Short file\n");
			exit(10);
		}
		n = DEC(buf[0]);
		if (n <= 0)
			break;

		bp = &buf[1];
		while (n > 0) {
			outdec(bp, out, n);
			bp += 4;
			n -= 3;
		}
	}
	return;
}

/*
 * output a group of 3 bytes (4 input characters).
 * the input chars are pointed to by p, they are to
 * be output to file f.  n is used to tell us not to
 * output all of them at the end of the file.
 */
static void
outdec(p, f, n)
char *p;
FILE *f;
{
	int c1, c2, c3;

	c1 = DEC(*p) << 2 | DEC(p[1]) >> 4;
	c2 = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
	c3 = DEC(p[2]) << 6 | DEC(p[3]);
	if (n >= 1)
		(void)putc(c1, f);
	if (n >= 2)
		(void)putc(c2, f);
	if (n >= 3)
		(void)putc(c3, f);
	return;
}


/* fr: like read but stdio */
int
fr(fd, buf, cnt)
FILE *fd;
char *buf;
int cnt;
{
	int c, i;

	for (i=0; i<cnt; i++) {
		c = getc(fd);
		if (c == EOF)
			return(i);
		buf[i] = c;
	}
	return (cnt);
}

/*
 * Return the ptr in sp at which the character c appears;
 * NULL if not found
 */

#define	NULL	0

static char *
index(sp, c)
register char *sp, c;
{
	do {
		if (*sp == c)
			return(sp);
	} while (*sp++);
	return(NULL);
}
