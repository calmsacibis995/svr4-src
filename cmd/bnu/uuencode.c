/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)bnu:uuencode.c	1.3.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
 * uuencode [input] output
 *
 * Encode a file so it can be mailed to a remote system.
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

/* ENC is the basic 1 character encoding function to make a char printing */
#define ENC(c) (((c) & 077) + ' ')

static void encode(), outdec();

main(argc, argv)
char **argv;
{
	FILE *in;
	struct stat sbuf;
	mode_t mode = 0;

	/* optional 1st argument */
	if (argc > 2) {
		if ((in = fopen(argv[1], "r")) == NULL) {
			perror(argv[1]);
			exit(1);
		}
		argv++; argc--;
	} else {
		in = stdin;
		mode = 0777;
	}

	if (argc != 2) {
		(void)printf("Usage: uuencode [infile] remotefile\n");
		exit(2);
	}

	/* figure out the input file mode */
	(void)fstat(fileno(in), &sbuf);
	if (mode != 0777) {
		mode = sbuf.st_mode & 0777;
	}
	(void)printf("begin %lo %s\n", (long) mode, argv[1]);

	encode(in, stdout);

	(void)printf("end\n");
	exit(0);
	/* NOTREACHED */
}

/*
 * copy from in to out, encoding as you go along.
 */
static void
encode(in, out)
FILE *in;
FILE *out;
{
	char buf[80];
	int i, n;

	for (;;) {
		/* 1 (up to) 45 character line */
		n = fr(in, buf, 45);
		(void)putc(ENC(n), out);

		for (i=0; i<n; i += 3)
			outdec(&buf[i], out);

		(void)putc('\n', out);
		if (n <= 0)
			break;
	}
	return;
}

/*
 * output one group of 3 bytes, pointed at by p, on file f.
 */
static void
outdec(p, f)
char *p;
FILE *f;
{
	int c1, c2, c3, c4;

	c1 = *p >> 2;
	c2 = (*p << 4) & 060 | (p[1] >> 4) & 017;
	c3 = (p[1] << 2) & 074 | (p[2] >> 6) & 03;
	c4 = p[2] & 077;
	(void)putc(ENC(c1), f);
	(void)putc(ENC(c2), f);
	(void)putc(ENC(c3), f);
	(void)putc(ENC(c4), f);
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
